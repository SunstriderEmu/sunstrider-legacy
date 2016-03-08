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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CreatureAI.h"
#include "Creature.h"
#include "Player.h"
#include "Pet.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "World.h"
#include "CreatureTextMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

//Enable PlayerAI when charmed
void PlayerAI::OnCharmed(Unit* charmer, bool apply) { me->IsAIEnabled = apply; }

//Disable CreatureAI when charmed
void CreatureAI::OnCharmed(Unit* charmer, bool apply)
{
    //me->IsAIEnabled = !apply;*/
    me->NeedChangeAI = true;
    me->IsAIEnabled = false;
}

void PlayerAI::OnPossess(Unit* charmer, bool apply) {}
void CreatureAI::OnPossess(Unit* charmer, bool apply) {}

void CreatureAI::Talk(uint8 id, WorldObject const* whisperTarget /*= nullptr*/)
{
    sCreatureTextMgr->SendChat(me, id, whisperTarget);
}

bool CreatureAI::AssistPlayerInCombat(Unit* who)
{
    if (!who)
        return false;
    
    //not a player
    if (!who->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    //only help friendly
    if(!me->IsFriendlyTo(who))
        return false;

    if(!me->IsWithinDistInMap(who, sWorld->getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS)))
        return false;

    for(auto itr : who->GetAttackers())
    {
        //experimental (unknown) flag not present
      /*  if (!(me->GetCreatureTemplate()->type_flags & 0x001000)) // CREATURE_TYPEFLAGS_AID_PLAYERS
            return false; */

        //contested guards don't assists if victim is not in combat (hacky)
        if (me->GetScriptName() == "guard_contested") {
            if (!itr->IsInCombat())
                continue;
        }

        //too far away from player, can aggro target ?
        if (me->CanAggro(itr, true) == CAN_ATTACK_RESULT_OK)
        {
            //already fighting someone?
            if (!me->GetVictim())
                AttackStart(itr);
            else
            {
                itr->SetInCombatWith(me);
                me->AddThreat(itr, 0.0f);
            }
            return true;
        }
    }

    return false;
}

void CreatureAI::MoveInLineOfSight(Unit* who)
{
    //if has just respawned and not a summon, wait a bit before reacting
    if (me->HasJustRespawned() && !me->GetSummonerGUID())
        return;

    if(AssistPlayerInCombat(who))
        return;

    CanAttackResult result = me->CanAggro(who, false);
    if(   result == CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH_WARN_RANGE
       && me->CanDoSuspiciousLook(who))
    {
        me->StartSuspiciousLook(who);
    }

    if(result != CAN_ATTACK_RESULT_OK) 
        return;

    //attack target if no current victim, else just enter combat with it
    if (!me->GetVictim())
    {
        who->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);

        if (me->HasUnitState(UNIT_STATE_DISTRACTED))
        {
            me->ClearUnitState(UNIT_STATE_DISTRACTED);
            me->GetMotionMaster()->Clear();
        }

        AttackStart(who);
    } else {
        if(!me->IsInCombatWith(who))
        {
            who->SetInCombatWith(me);
            me->AddThreat(who, 0.0f);
        }
    }
}

bool CreatureAI::UpdateVictim(bool evade)
{
    if(!me->IsInCombat())
        return false;

    if(Unit *victim = me->SelectVictim(evade)) 
        AttackStart(victim);

    return me->GetVictim();
}

void CreatureAI::EnterEvadeMode()
{
    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->CombatStop();
    me->InitCreatureAddon();
    me->SetLootRecipient(NULL);
    me->ResetPlayerDamageReq();

    if(me->IsAlive())
    {
        me->AddUnitState(UNIT_STATE_EVADE);
        me->GetMotionMaster()->MoveTargetedHome();
    }
    
    me->SetLastDamagedTime(0);
}

bool CreatureAI::IsInMeleeRange() const
{
    Unit* victim = me->GetVictim();
    if (!victim)
        return false;

    //check victim first to speed up most cases
    if (me->IsWithinMeleeRange(victim))
        return true;

    Map* pMap = me->GetMap();
    Map::PlayerList const &PlayerList = pMap->GetPlayers();
    if (!PlayerList.isEmpty())
    {
        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            if (me->IsWithinMeleeRange(i->GetSource()))
                return true;
        }
    }
    return false;
}

void CreatureAI::DoZoneInCombat(Unit* pUnit, bool force)
{
    if (!pUnit)
        pUnit = me;

    Map *map = pUnit->GetMap();

    if (!map->IsDungeon())                                  //use IsDungeon instead of Instanceable, in case battlegrounds will be instantiated
    {
        error_log("TSCR: DoZoneInCombat call for map that isn't an instance (pUnit entry = %d)", pUnit->GetTypeId() == TYPEID_UNIT ? (pUnit->ToCreature())->GetEntry() : 0);
        return;
    }

    if (!pUnit->CanHaveThreatList())
    {
        if (!force && pUnit->getThreatManager().isThreatListEmpty())
        {
            error_log("TSCR: DoZoneInCombat called for creature that either cannot have threat list or has empty threat list (pUnit entry = %d)", pUnit->GetTypeId() == TYPEID_UNIT ? (pUnit->ToCreature())->GetEntry() : 0);

            return;
        }
    }

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
    {
        if (Player* i_pl = i->GetSource())
            if (i_pl->IsAlive()
                && !i_pl->IsGameMaster())
            {
                pUnit->SetInCombatWith(i_pl);
                i_pl->SetInCombatWith(pUnit);
                pUnit->AddThreat(i_pl, 0.0f);
            }
    }
}

void CreatureAI::AttackStartIfCan(Unit* victim)
{
    if(!victim)
        return;

    //Merge conflict : set CanStartAttack
    if(me->CanAttack(victim) == CAN_ATTACK_RESULT_OK)
        AttackStart(victim);
}

void SimpleCharmedAI::UpdateAI(const uint32 /*diff*/)
{
    Creature *charmer = me->GetCharmer()->ToCreature();

    //kill self if charm aura has infinite duration
    if(charmer->IsInEvadeMode())
    {
        Unit::AuraList const& auras = me->GetAurasByType(SPELL_AURA_MOD_CHARM);
        for(Unit::AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
            if((*iter)->GetCasterGUID() == charmer->GetGUID() && (*iter)->IsPermanent())
            {
                charmer->Kill(me);
                return;
            }
    }

    if(!charmer->IsInCombat())
        me->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

    Unit *target = me->GetVictim();
    if(!target || charmer->CanAttack(target) != CAN_ATTACK_RESULT_OK)
        AttackStart(charmer->SelectNearestTarget());
}


Unit* CreatureAI::SelectUnit(SelectAggroTarget target, uint32 position)
{
    std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator i = m_threatlist.begin();
    std::list<HostileReference*>::reverse_iterator r = m_threatlist.rbegin();

    if (position >= m_threatlist.size() || !m_threatlist.size())
        return NULL;

    switch (target)
    {
    case SELECT_TARGET_RANDOM:
        advance(i, position + (rand() % (m_threatlist.size() - position)));
        return ObjectAccessor::GetUnit((*me), (*i)->getUnitGuid());
    case SELECT_TARGET_TOPAGGRO:
        advance(i, position);
        return ObjectAccessor::GetUnit((*me), (*i)->getUnitGuid());
    case SELECT_TARGET_BOTTOMAGGRO:
        advance(r, position);
        return ObjectAccessor::GetUnit((*me), (*r)->getUnitGuid());
    default:
        break;
    }

    return NULL;
}

bool CreatureAI::checkTarget(Unit* target, bool playersOnly, float radius, bool noTank)
{
    if (!me)
        return false;

    if (!target)
        return false;

    if (!target->IsAlive())
        return false;

    if (noTank && target == me->GetVictim())
        return false;

    if (playersOnly && (target->GetTypeId() != TYPEID_PLAYER))
        return false;

    if (radius > 0.0f && !me->IsWithinCombatRange(target, radius))
        return false;

    if (radius < 0.0f && me->IsWithinCombatRange(target, -radius))
        return false;

    return true;
}

struct TargetDistanceOrder : public std::binary_function<const Unit, const Unit, bool>
{
    const Unit* me;
    TargetDistanceOrder(const Unit* Target) : me(Target) {};
    // functor for operator ">"
    bool operator()(const Unit* _Left, const Unit* _Right) const
    {
        return (me->GetDistance(_Left) < me->GetDistance(_Right));
    }
};

Unit* CreatureAI::SelectUnit(SelectAggroTarget targetType, uint32 position, float radius, bool playersOnly, bool noTank)
{
    std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
    if (position >= threatlist.size())
        return NULL;

    std::list<Unit*> targetList;
    for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius, noTank))
            targetList.push_back((*itr)->getTarget());

    if (position >= targetList.size())
        return NULL;

    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
        targetList.sort(Trinity::ObjectDistanceOrderPred(me));

    switch (targetType)
    {
    case SELECT_TARGET_NEAREST:
    case SELECT_TARGET_TOPAGGRO:
    {
        std::list<Unit*>::iterator itr = targetList.begin();
        std::advance(itr, position);
        return *itr;
    }
    case SELECT_TARGET_FARTHEST:
    case SELECT_TARGET_BOTTOMAGGRO:
    {
        std::list<Unit*>::reverse_iterator ritr = targetList.rbegin();
        std::advance(ritr, position);
        return *ritr;
    }
    case SELECT_TARGET_RANDOM:
    {
        std::list<Unit*>::iterator itr = targetList.begin();
        std::advance(itr, urand(position, targetList.size() - 1));
        return *itr;
    }
    default:
        break;
    }

    return NULL;
}

Unit* CreatureAI::SelectUnit(SelectAggroTarget targetType, uint32 position, float distNear, float distFar, bool playerOnly)
{
    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
    {
        std::list<HostileReference*> &m_threatlist = me->getThreatManager().getThreatList();
        if (m_threatlist.empty()) return NULL;
        std::list<Unit*> targetList;
        std::list<HostileReference*>::iterator itr = m_threatlist.begin();
        for (; itr != m_threatlist.end(); ++itr)
        {
            Unit *target = ObjectAccessor::GetUnit(*me, (*itr)->getUnitGuid());
            if (!target
                || (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
                || (target->GetTransForm() == FORM_SPIRITOFREDEMPTION)
                || (distNear && me->IsWithinCombatRange(target, distNear))
                || (distFar && !me->IsWithinCombatRange(target, distFar))
                )
            {
                continue;
            }
            targetList.push_back(target);
        }
        if (position >= targetList.size())
            return NULL;
        targetList.sort(TargetDistanceOrder(me));
        if (targetType == SELECT_TARGET_NEAREST)
        {
            std::list<Unit*>::iterator i = targetList.begin();
            advance(i, position);
            return *i;
        }
        else
        {
            std::list<Unit*>::reverse_iterator i = targetList.rbegin();
            advance(i, position);
            return *i;
        }
    }
    else
    {
        std::list<HostileReference*> m_threatlist = me->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator i;
        Unit *target;
        while (position < m_threatlist.size())
        {
            if (targetType == SELECT_TARGET_BOTTOMAGGRO)
            {
                i = m_threatlist.end();
                advance(i, -(int32)position - 1);
            }
            else
            {
                i = m_threatlist.begin();
                if (targetType == SELECT_TARGET_TOPAGGRO)
                    advance(i, position);
                else // random
                    advance(i, position + rand() % (m_threatlist.size() - position));
            }

            target = ObjectAccessor::GetUnit(*me, (*i)->getUnitGuid());
            if (!target
                || (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
                || (target->GetTransForm() == FORM_SPIRITOFREDEMPTION)
                || (distNear && me->IsWithinCombatRange(target, distNear))
                || (distFar && !me->IsWithinCombatRange(target, distFar))
                )
            {
                m_threatlist.erase(i);
            }
            else
            {
                return target;
            }
        }
    }

    return NULL;
}

// selects random unit not having aura
Unit* CreatureAI::SelectUnit(uint32 position, float distNear, float distFar, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex)
{
    std::list<HostileReference*> m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator i;
    Unit *target;
    while (position < m_threatlist.size())
    {
        i = m_threatlist.begin();
        advance(i, position + rand() % (m_threatlist.size() - position));

        target = ObjectAccessor::GetUnit(*me, (*i)->getUnitGuid());
        if (!target
            || !target->IsAlive()
            || (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
            || (distNear && me->IsWithinCombatRange(target, distNear))
            || (distFar && !me->IsWithinCombatRange(target, distFar))
            || (auraCheck && target->HasAuraEffect(spellId, effIndex))
            || (exceptPossesed && target->IsPossessed())
            || (exceptPossesed && target->IsPossessing())
            )
        {
            m_threatlist.erase(i);
        }
        else
        {
            return target;
        }
    }
    return NULL;
}

void CreatureAI::SelectUnitList(std::list<Unit*> &targetList, uint32 maxTargets, SelectAggroTarget targetType, float radius, bool playersOnly, uint32 notHavingAuraId, uint8 effIndex)
{
    std::list<HostileReference*> const& threatlist = me->getThreatManager().getThreatList();
    if (threatlist.empty())
        return;

    for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius)
            && (!notHavingAuraId || !((*itr)->getTarget()->HasAuraEffect(notHavingAuraId, effIndex))))
            targetList.push_back((*itr)->getTarget());

    if (targetList.size() < maxTargets)
        return;

    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
        targetList.sort(Trinity::ObjectDistanceOrderPred(me));

    if (targetType == SELECT_TARGET_FARTHEST || targetType == SELECT_TARGET_BOTTOMAGGRO)
        targetList.reverse();

    if (targetType == SELECT_TARGET_RANDOM)
        Trinity::Containers::RandomResizeList(targetList, maxTargets);
    else
        targetList.resize(maxTargets);
}
