/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "PetAI.h"
#include "Errors.h"
#include "Pet.h"
#include "Player.h"
#include "DBCStores.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "SpellMgr.h"
#include "Creature.h"
#include "World.h"
#include "Util.h"

int PetAI::Permissible(const Creature *creature)
{
    if( creature->IsPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature *c) : CreatureAI(c), i_pet(*c), i_tracker(TIME_INTERVAL_LOOK), distanceCheckTimer(3000)
{
    m_AllySet.clear();
    UpdateAllies();
}

void PetAI::EnterEvadeMode()
{
}

bool PetAI::_needToStop() const
{
    // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
    if(i_pet.IsCharmed() && i_pet.GetVictim() == i_pet.GetCharmer())
        return true;

    if (i_pet.GetOwner() && i_pet.GetOwner()->ToPlayer() && i_pet.ToPet() && i_pet.ToPet()->isControlled() && i_pet.GetVictim()->IsJustCCed() && i_pet.GetVictim()->GetEntry() != 10) // Training dummy exception
        return true;

    if (i_pet.IsNonMeleeSpellCast(false))
        return true;

    return i_pet.CanAttack(i_pet.GetVictim()) != CAN_ATTACK_RESULT_OK;
}

void PetAI::ResetMovement()
{
    Unit* owner = i_pet.GetCharmerOrOwner();

    if(owner && i_pet.IsAlive() && i_pet.GetCharmInfo() && i_pet.GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        i_pet.GetMotionMaster()->MoveFollow(owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
    }
    else
    {
        i_pet.ClearUnitState(UNIT_STATE_FOLLOW);
        i_pet.GetMotionMaster()->Clear();
        i_pet.GetMotionMaster()->MoveIdle();
    }
}
void PetAI::_stopAttack()
{
    if( !i_pet.IsAlive() )
        i_pet.GetHostileRefManager().deleteReferences();

    ResetMovement();
    i_pet.CombatStop();
}

void PetAI::UpdateAI(const uint32 diff)
{
    if (!i_pet.IsAlive())
        return;

    Unit* owner = i_pet.GetCharmerOrOwner();

    if(m_updateAlliesTimer <= diff)
        // UpdateAllies self set update timer
        UpdateAllies();
    else
        m_updateAlliesTimer -= diff;

    // i_pet.GetVictim() can't be used for check in case stop fighting, i_pet.GetVictim() clear at Unit death etc.
    if( i_pet.GetVictim() )
    {
        if( _needToStop() )
        {
            //TC_LOG_DEBUG("entities.pet","Pet AI stopped attacking [guid=%u]", i_pet.GetGUIDLow());
            _stopAttack();
            return;
        }

        DoMeleeAttackIfReady();
    }
    else
    {
        if(!i_pet.IsNonMeleeSpellCast(false))
        {
            if(i_pet.IsInCombat() && i_pet.GetAttackers().empty())
            {
               _stopAttack();
            } else if(owner && i_pet.GetCharmInfo()) //no victim
            {
                if(owner->IsInCombat() && !(i_pet.HasReactState(REACT_PASSIVE) || i_pet.GetCharmInfo()->HasCommandState(COMMAND_STAY))) {
                    AttackStart(owner->GetAttackerForHelper());
                }
                else if(i_pet.GetCharmInfo()->HasCommandState(COMMAND_FOLLOW) && !i_pet.HasUnitState(UNIT_STATE_FOLLOW))
                    i_pet.GetMotionMaster()->MoveFollow(owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
            }
        }
    }

    if(!me->GetCharmInfo())
        return;

    if (i_pet.GetGlobalCooldown() == 0 && !i_pet.HasUnitState(UNIT_STATE_CASTING))
    {
        bool inCombat = me->GetVictim();

        //Autocast
        for (uint8 i = 0; i < i_pet.GetPetAutoSpellSize(); i++)
        {
            uint32 spellID = i_pet.GetPetAutoSpellOnPos(i);
            if (!spellID)
                continue;

            SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellID);
            if (!spellInfo)
                continue;

            // ignore some combinations of combat state and combat/noncombat spells
            if (!inCombat)
            {
                if (!spellInfo->IsPositive())
                    continue;
            }
            else
            {
                if (!spellInfo->CanBeUsedInCombat())
                    continue;
            }

            Spell *spell = new Spell(&i_pet, spellInfo, false, 0);

            if(inCombat && !i_pet.HasUnitState(UNIT_STATE_FOLLOW) && spell->CanAutoCast(i_pet.GetVictim()))
            {
                //m_targetSpellStore.push_back(std::make_pair<Unit*, Spell*>(i_pet.GetVictim(), spell));
                m_targetSpellStore.push_back(std::make_pair(i_pet.GetVictim(), spell));
                continue;
            }
            else
            {
                bool spellUsed = false;
                for(std::set<uint64>::iterator tar = m_AllySet.begin(); tar != m_AllySet.end(); ++tar)
                {
                    Unit* Target = ObjectAccessor::GetUnit(i_pet,*tar);

                    //only buff targets that are in combat, unless the spell can only be cast while out of combat
                    if(!Target)
                        continue;

                    if(spell->CanAutoCast(Target))
                    {
                        //m_targetSpellStore.push_back(std::make_pair<Unit*, Spell*>(Target, spell));
                        m_targetSpellStore.push_back(std::make_pair(Target, spell));
                        spellUsed = true;
                        break;
                    }
                }
                if (!spellUsed)
                    delete spell;
            }
        }

        //found units to cast on to
        if(!m_targetSpellStore.empty())
        {
            uint32 index = urand(0, m_targetSpellStore.size() - 1);

            Spell* spell  = m_targetSpellStore[index].second;
            Unit*  target = m_targetSpellStore[index].first;

            m_targetSpellStore.erase(m_targetSpellStore.begin() + index);

            SpellCastTargets targets;
            targets.SetUnitTarget( target );

            if( !i_pet.HasInArc(M_PI, target) )
            {
                i_pet.SetInFront(target);
                if( target->GetTypeId() == TYPEID_PLAYER )
                    i_pet.SendUpdateToPlayer( target->ToPlayer() );

                if(owner && owner->GetTypeId() == TYPEID_PLAYER)
                    i_pet.SendUpdateToPlayer( owner->ToPlayer() );
            }

            if(i_pet.IsPet())
                ((Pet*)&i_pet)->CheckLearning(spell->m_spellInfo->Id);

            spell->prepare(&targets);
        }
        while (!m_targetSpellStore.empty())
        {
            delete m_targetSpellStore.begin()->second;
            m_targetSpellStore.erase(m_targetSpellStore.begin());
        }

        if(i_pet.IsPet() && ((Pet*)&i_pet)->getPetType() == MINI_PET)
        {
            Minipet_DistanceCheck(diff);
        }
    }
}

void PetAI::UpdateAllies()
{
    Unit* owner = i_pet.GetCharmerOrOwner();
    Group *pGroup = NULL;

    m_updateAlliesTimer = 10000;                            //update friendly targets every 10 seconds, lesser checks increase performance

    if(!owner)
        return;
    else if(owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = (owner->ToPlayer())->GetGroup();

    //only pet and owner/not in group->ok
    if(m_AllySet.size() == 2 && !pGroup)
        return;
    //owner is in group; group members filled in already (no raid -> subgroupcount = whole count)
    if(pGroup && !pGroup->isRaidGroup() && m_AllySet.size() == (pGroup->GetMembersCount() + 2))
        return;

    m_AllySet.clear();
    m_AllySet.insert(i_pet.GetGUID());
    if(pGroup)                                              //add group
    {
        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* Target = itr->GetSource();
            if(!Target || !pGroup->SameSubGroup(owner->ToPlayer(), Target))
                continue;

            if(Target->GetGUID() == owner->GetGUID())
                continue;

            m_AllySet.insert(Target->GetGUID());
        }
    }
    else                                                    //remove group
        m_AllySet.insert(owner->GetGUID());
}

void PetAI::Minipet_DistanceCheck(uint32 diff)
{
    Unit* owner = me->GetOwner();
    if (!owner)
        return;
    if (distanceCheckTimer <= diff)
    {
        distanceCheckTimer = 2000;
        float masterSpeed = owner->GetSpeed(MOVE_RUN);
        float masterSpeedRate = masterSpeed / baseMoveSpeed[MOVE_RUN];
        float masterDistance = me->GetDistance(owner);
        if(masterDistance >= 20)
        {
            me->SetSpeedRate(MOVE_RUN, masterSpeedRate * (masterDistance / 15.f));
        } else if (me->GetSpeed(MOVE_RUN) > masterSpeed) {
            me->SetSpeedRate(MOVE_RUN, masterSpeedRate);
        }
    } else distanceCheckTimer -= diff;
}