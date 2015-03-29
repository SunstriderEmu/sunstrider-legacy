/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "ScriptedCreature.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"

// Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary {
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} *SpellSummary;

void SummonList::Despawn(Creature *summon)
{
    uint64 guid = summon->GetGUID();
    for(iterator i = begin(); i != end(); ++i)
    {
        if(*i == guid)
        {
            erase(i);
            return;
        }
    }
}

void SummonList::DespawnEntry(uint32 entry)
{
    for(auto itr = begin(); itr != end();)
    {
        if(Creature *summon = Unit::GetCreature(*m_creature, *itr))
        {
            if(summon->GetEntry() == entry)
            {
                summon->RemoveFromWorld();
                summon->SetDeathState(JUST_DIED);
                summon->RemoveCorpse();
                itr = erase(itr);
                continue;
            }
        }
        itr++;
    }
}

void SummonList::DespawnAll(bool withoutWorldBoss)
{
    for(iterator i = begin(); i != end(); ++i)
    {
        if(Creature *summon = Unit::GetCreature(*m_creature, *i))
        {
            if (withoutWorldBoss && summon->IsWorldBoss())
                continue;

            summon->AddObjectToRemoveList();
        }
    }
    clear();
}

bool SummonList::IsEmpty()
{
    return empty();
}

void BumpHelper::Update(const uint32 diff)
{
    for(auto itr = begin(); itr != end();)
    {
        if(itr->second < diff) //okay to erase
            itr = erase(itr);
        else //just decrease time left
        {
            itr->second -= diff;
            itr++;
        }
    }
}

//return true if not yet present in list
bool BumpHelper::AddCooldown(Unit* p, uint32 customValue)
{
    std::map<uint64,uint32>::iterator found = find(p->GetGUID());
    if(found != end())
        return false;

    insert(std::make_pair(p->GetGUID(),customValue?customValue:m_cooldown)); //3s before being knockable again
    return true;
}

void ScriptedAI::AttackStart(Unit* who, bool melee)
{
    if (!who)
        return;

    if (m_creature->Attack(who, melee))
    {
        m_creature->AddThreat(who, 0.0f);

        if (!InCombat)
        {
            InCombat = true;
            EnterCombat(who);
        }

        if(melee)
            DoStartMovement(who);
        else
            DoStartNoMovement(who);
    }
}

void ScriptedAI::AttackStart(Unit* who)
{
    if (!who)
        return;
    
    bool melee = (GetCombatDistance() > ATTACK_DISTANCE) ? me->GetDistance(who) <= ATTACK_DISTANCE : true;
    if (m_creature->Attack(who, melee))
    {
        m_creature->AddThreat(who, 0.0f);
        if (!InCombat)
        {
            InCombat = true;
            EnterCombat(who);
        }

        if(IsCombatMovementAllowed())
        {
            DoStartMovement(who);
        } else {
            DoStartNoMovement(who);
        }
    }
}

void ScriptedAI::UpdateAI(const uint32 diff)
{
    //Check if we have a current target
    if (m_creature->IsAlive() && UpdateVictim())
    {
        if (m_creature->IsAttackReady() )
        {
            //If we are within range melee the target
            if (m_creature->IsWithinMeleeRange(m_creature->GetVictim()))
            {
                m_creature->AttackerStateUpdate(m_creature->GetVictim());
                m_creature->ResetAttackTimer();
            }
        }
    }
}

void ScriptedAI::EnterEvadeMode()
{
    //m_creature->InterruptNonMeleeSpells(true);
    m_creature->RemoveAllAuras();
    m_creature->DeleteThreatList();
    m_creature->CombatStop();
    m_creature->InitCreatureAddon();
    m_creature->SetLootRecipient(NULL);

    if(m_creature->IsAlive())
    {
        if(Unit* owner = m_creature->GetOwner())
        {
            if(owner->IsAlive())
                m_creature->GetMotionMaster()->MoveFollow(owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
        }
        else
            m_creature->GetMotionMaster()->MoveTargetedHome();
    }

    InCombat = false;
    m_creature->ResetAllowedToLootList();
    Reset();
}

void ScriptedAI::JustRespawned()
{
    InCombat = false;
    Reset();
}

void ScriptedAI::DoStartMovement(Unit* victim, float distance, float angle)
{
    if (!victim)
        return;

    m_creature->GetMotionMaster()->MoveChase(victim, distance, angle);
}

void ScriptedAI::DoStartNoMovement(Unit* victim)
{
    if (!victim)
        return;

    m_creature->GetMotionMaster()->MoveIdle();
}

void ScriptedAI::DoStopAttack()
{
    if (m_creature->GetVictim() != NULL)
    {
        m_creature->AttackStop();
    }
}

uint32 ScriptedAI::DoCast(Unit* victim, uint32 spellId, bool triggered)
{
    //remove this?
    if (m_creature->HasUnitState(UNIT_STATE_CASTING) && !triggered)
        return SPELL_FAILED_SPELL_IN_PROGRESS;

    uint32 reason = m_creature->CastSpell(victim, spellId, triggered);

    //restore combat movement on out of mana
    if(reason == SPELL_FAILED_NO_POWER && GetRestoreCombatMovementOnOOM() && !IsCombatMovementAllowed())
        SetCombatMovementAllowed(true);

    return reason;
}

uint32 ScriptedAI::DoCastAOE(uint32 spellId, bool triggered)
{
    return DoCast((Unit*)NULL, spellId, triggered);
}

uint32 ScriptedAI::DoCastSpell(Unit* who,SpellEntry const *spellInfo, bool triggered)
{
    //remove this?
    if (!who || m_creature->IsNonMeleeSpellCast(false))
        return SPELL_FAILED_SPELL_IN_PROGRESS;

    m_creature->StopMoving();
    return m_creature->CastSpell(who, spellInfo, triggered);
}

void ScriptedAI::DoSay(const char* text, uint32 language, Unit* target, bool SayEmote)
{
    if (target)
    {
        m_creature->Say(text, language, target->GetGUID());
        if(SayEmote)
            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
    }
    else m_creature->Say(text, language, 0);
}

void ScriptedAI::DoYell(const char* text, uint32 language, Unit* target)
{
    if (target) m_creature->Yell(text, language, target->GetGUID());
    else m_creature->Yell(text, language, 0);
}

void ScriptedAI::DoTextEmote(const char* text, Unit* target, bool IsBossEmote)
{
    if (target) m_creature->TextEmote(text, target->GetGUID(), IsBossEmote);
    else m_creature->TextEmote(text, 0, IsBossEmote);
}

void ScriptedAI::DoWhisper(const char* text, Unit* reciever, bool IsBossWhisper)
{
    if (!reciever || reciever->GetTypeId() != TYPEID_PLAYER)
        return;

    m_creature->Whisper(text, reciever->GetGUID(), IsBossWhisper);
}

void ScriptedAI::DoPlaySoundToSet(Unit* unit, uint32 sound)
{
    if (!unit)
        return;

    if (!GetSoundEntriesStore()->LookupEntry(sound))
    {
        error_log("TSCR: Invalid soundId %u used in DoPlaySoundToSet (by unit TypeId %u, guid %u)", sound, unit->GetTypeId(), unit->GetGUID());
        return;
    }

    WorldPacket data(4);
    data.SetOpcode(SMSG_PLAY_SOUND);
    data << uint32(sound);
    unit->SendMessageToSet(&data,false);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 id, float x, float y, float z, float angle, uint32 type, uint32 despawntime)
{
    return m_creature->SummonCreature(id,m_creature->GetPositionX() + x,m_creature->GetPositionY() + y,m_creature->GetPositionZ() + z, angle, (TempSummonType)type, despawntime);
}

Unit* ScriptedAI::SelectUnit(SelectAggroTarget target, uint32 position)
{
    std::list<HostilReference*>& m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostilReference*>::iterator i = m_threatlist.begin();
    std::list<HostilReference*>::reverse_iterator r = m_threatlist.rbegin();

    if (position >= m_threatlist.size() || !m_threatlist.size())
        return NULL;

    switch (target)
    {
        case SELECT_TARGET_RANDOM:
            advance(i, position + (rand()%(m_threatlist.size() - position)));
            return Unit::GetUnit((*me), (*i)->getUnitGuid());
        case SELECT_TARGET_TOPAGGRO:
            advance(i, position);
            return Unit::GetUnit((*me), (*i)->getUnitGuid());
        case SELECT_TARGET_BOTTOMAGGRO:
            advance(r, position);
            return Unit::GetUnit((*me),(*r)->getUnitGuid());
        default:
            break;
    }

    return NULL;
}

bool ScriptedAI::checkTarget(Unit* target, bool playersOnly, float radius, bool noTank)
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

Unit* ScriptedAI::SelectUnit(SelectAggroTarget targetType, uint32 position, float radius, bool playersOnly, bool noTank)
{
    std::list<HostilReference*>& threatlist = me->getThreatManager().getThreatList();
    if (position >= threatlist.size())
        return NULL;

    std::list<Unit*> targetList;
    for (std::list<HostilReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
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

Unit* ScriptedAI::SelectUnit(SelectAggroTarget targetType, uint32 position, float distNear, float distFar, bool playerOnly)
{
    if(targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
    {
        std::list<HostilReference*> &m_threatlist = m_creature->getThreatManager().getThreatList();
        if(m_threatlist.empty()) return NULL;
        std::list<Unit*> targetList;
        std::list<HostilReference*>::iterator itr = m_threatlist.begin();
        for(; itr!= m_threatlist.end(); ++itr)
        {
            Unit *target = Unit::GetUnit(*m_creature, (*itr)->getUnitGuid());
            if(!target
                || playerOnly && target->GetTypeId() != TYPEID_PLAYER
                || target->GetTransForm() == FORM_SPIRITOFREDEMPTION
                || distNear && m_creature->IsWithinCombatRange(target, distNear) 
                || distFar && !m_creature->IsWithinCombatRange(target, distFar)
              )
            {
                continue;
            }
            targetList.push_back(target);
        }
        if(position >= targetList.size())
            return NULL;
        targetList.sort(TargetDistanceOrder(m_creature));
        if(targetType == SELECT_TARGET_NEAREST)
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
        std::list<HostilReference*> m_threatlist = m_creature->getThreatManager().getThreatList();
        std::list<HostilReference*>::iterator i;
        Unit *target;
        while(position < m_threatlist.size())
        {
            if(targetType == SELECT_TARGET_BOTTOMAGGRO)
            {
                i = m_threatlist.end();
                advance(i, - (int32)position - 1);
            }
            else
            {
                i = m_threatlist.begin();
                if(targetType == SELECT_TARGET_TOPAGGRO)
                    advance(i, position);
                else // random
                    advance(i, position + rand()%(m_threatlist.size() - position));
            }

            target = Unit::GetUnit(*m_creature,(*i)->getUnitGuid());
            if(!target
                || playerOnly && target->GetTypeId() != TYPEID_PLAYER
                || target->GetTransForm() == FORM_SPIRITOFREDEMPTION
                || distNear && m_creature->IsWithinCombatRange(target, distNear) 
                || distFar && !m_creature->IsWithinCombatRange(target, distFar)
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
Unit* ScriptedAI::SelectUnit( uint32 position, float distNear, float distFar, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex)
{
    std::list<HostilReference*> m_threatlist = m_creature->getThreatManager().getThreatList();
    std::list<HostilReference*>::iterator i;
    Unit *target;
    while(position < m_threatlist.size())
    {
        i = m_threatlist.begin();
        advance(i, position + rand()%(m_threatlist.size() - position));
 
        target = Unit::GetUnit(*m_creature,(*i)->getUnitGuid());
        if(!target
            || !target->IsAlive()
            || playerOnly && target->GetTypeId() != TYPEID_PLAYER
            || distNear && m_creature->IsWithinCombatRange(target, distNear)
            || distFar && !m_creature->IsWithinCombatRange(target, distFar)
            || auraCheck && target->HasAura(spellId, effIndex)
            || exceptPossesed && target->IsPossessed()
            || exceptPossesed && target->IsPossessing()
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

void ScriptedAI::SelectUnitList(std::list<Unit*> &targetList, uint32 maxTargets, SelectAggroTarget targetType, float radius, bool playersOnly, uint32 notHavingAuraId, uint8 effIndex)
{
    std::list<HostilReference*> const& threatlist = me->getThreatManager().getThreatList();
    if (threatlist.empty())
        return;

    for (std::list<HostilReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius)
            && (!notHavingAuraId || !((*itr)->getTarget()->HasAura(notHavingAuraId, effIndex))) )
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

SpellEntry const* ScriptedAI::SelectSpell(Unit* Target, int32 School, int32 Mechanic, SelectTarget Targets, uint32 PowerCostMin, uint32 PowerCostMax, float RangeMin, float RangeMax, SelectEffect Effects)
{
    //No target so we can't cast
    if (!Target)
        return false;

    //Silenced so we can't cast
    if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    //Using the extended script system we first create a list of viable spells
    SpellEntry const* Spell[4];
    Spell[0] = 0;
    Spell[1] = 0;
    Spell[2] = 0;
    Spell[3] = 0;

    uint32 SpellCount = 0;

    SpellEntry const* TempSpell;
    SpellRangeEntry const* TempRange;

    //Check if each spell is viable(set it to null if not)
    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        TempSpell = sSpellMgr->GetSpellInfo(m_creature->m_spells[i]);

        //This spell doesn't exist
        if (!TempSpell)
            continue;

        // Targets and Effects checked first as most used restrictions
        //Check the spell targets if specified
        if ( Targets && !(SpellSummary[m_creature->m_spells[i]].Targets & (1 << (Targets-1))) )
            continue;

        //Check the type of spell if we are looking for a specific spell type
        if ( Effects && !(SpellSummary[m_creature->m_spells[i]].Effects & (1 << (Effects-1))) )
            continue;

        //Check for school if specified
        if (School >= 0 && TempSpell->SchoolMask & School)
            continue;

        //Check for spell mechanic if specified
        if (Mechanic >= 0 && TempSpell->Mechanic != Mechanic)
            continue;

        //Make sure that the spell uses the requested amount of power
        if (PowerCostMin &&  TempSpell->manaCost < PowerCostMin)
            continue;

        if (PowerCostMax && TempSpell->manaCost > PowerCostMax)
            continue;

        //Continue if we don't have the mana to actually cast this spell
        if (TempSpell->manaCost > m_creature->GetPower((Powers)TempSpell->powerType))
            continue;

        //Get the Range
        TempRange = GetSpellRangeStore()->LookupEntry(TempSpell->rangeIndex);

        //Spell has invalid range store so we can't use it
        if (!TempRange)
            continue;

        //Check if the spell meets our range requirements
        if (RangeMin && TempRange->maxRange < RangeMin)
            continue;
        if (RangeMax && TempRange->maxRange > RangeMax)
            continue;

        //Check if our target is in range
        if (m_creature->IsWithinDistInMap(Target, TempRange->minRange) || !m_creature->IsWithinDistInMap(Target, TempRange->maxRange))
            continue;

        //All good so lets add it to the spell list
        Spell[SpellCount] = TempSpell;
        SpellCount++;
    }

    //We got our usable spells so now lets randomly pick one
    if (!SpellCount)
        return NULL;

    return Spell[rand()%SpellCount];
}

bool ScriptedAI::CanCast(Unit* Target, SpellEntry const *Spell, bool Triggered)
{
    //No target so we can't cast
    if (!Target || !Spell)
        return false;

    //Silenced so we can't cast
    if (!Triggered && m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    //Check for power
    if (!Triggered && m_creature->GetPower((Powers)Spell->powerType) < Spell->manaCost)
        return false;

    SpellRangeEntry const *TempRange = NULL;

    TempRange = GetSpellRangeStore()->LookupEntry(Spell->rangeIndex);

    //Spell has invalid range store so we can't use it
    if (!TempRange)
        return false;

    //Unit is out of range of this spell
    if (m_creature->GetDistance(Target) > TempRange->maxRange || m_creature->GetDistance(Target) < TempRange->minRange)
        return false;

    return true;
}

void ScriptedAI::SetEquipmentSlots(bool bLoadDefault, int32 uiMainHand, int32 uiOffHand, int32 uiRanged)
{
    if (bLoadDefault)
    {
        if (CreatureTemplate const* pInfo = GetCreatureTemplateStore(m_creature->GetEntry()))
            m_creature->LoadEquipment(pInfo->equipmentId,true);

        return;
    }

    if (uiMainHand >= 0)
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(uiMainHand));

    if (uiOffHand >= 0)
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, uint32(uiOffHand));

    if (uiRanged >= 0)
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, uint32(uiRanged));
}

float GetSpellMaxRange(uint32 id)
{
    SpellEntry const *spellInfo = sSpellMgr->GetSpellInfo(id);
    if(!spellInfo) return 0;
    SpellRangeEntry const *range = GetSpellRangeStore()->LookupEntry(spellInfo->rangeIndex);
    if(!range) return 0;
    return range->maxRange;
}

void FillSpellSummary()
{
    SpellSummary = new TSpellSummary[sObjectMgr->GetMaxSpellId()+1];

    SpellEntry const* TempSpell;

    //for (int i=0; i < GetSpellStore()->GetNumRows(); i++ )
    for (std::map<uint32, SpellEntry*>::iterator itr = sObjectMgr->GetSpellStore()->begin(); itr != sObjectMgr->GetSpellStore()->end(); itr++)
    {
        int i = itr->first;
        SpellSummary[i].Effects = 0;
        SpellSummary[i].Targets = 0;

        TempSpell = sSpellMgr->GetSpellInfo(i);
        //This spell doesn't exist
        if (!TempSpell)
            continue;

        for (int j=0; j<3; j++)
        {
            //Spell targets self
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SELF-1);

            //Spell targets a single enemy
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ENEMY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DEST_TARGET_ENEMY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_ENEMY-1);

            //Spell targets AoE at enemy
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_SRC ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_DST ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DEST_DYNOBJ_ENEMY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_ENEMY-1);

            //Spell targets an enemy
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ENEMY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DEST_TARGET_ENEMY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_SRC ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_DST ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DEST_DYNOBJ_ENEMY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_ENEMY-1);

            //Spell targets a single friend(or self)
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ALLY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_PARTY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_FRIEND-1);

            //Spell targets aoe friends
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_TARGET ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_FRIEND-1);

            //Spell targets any friend(or self)
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ALLY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_PARTY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_TARGET ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_FRIEND-1);

            //Make sure that this spell includes a damage effect
            if ( TempSpell->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE ||
                TempSpell->Effect[j] == SPELL_EFFECT_INSTAKILL ||
                TempSpell->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
                TempSpell->Effect[j] == SPELL_EFFECT_HEALTH_LEECH )
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_DAMAGE-1);

            //Make sure that this spell includes a healing effect (or an apply aura with a periodic heal)
            if ( TempSpell->Effect[j] == SPELL_EFFECT_HEAL ||
                TempSpell->Effect[j] == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                TempSpell->Effect[j] == SPELL_EFFECT_HEAL_MECHANICAL ||
                (TempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA  && TempSpell->EffectApplyAuraName[j]== 8 ))
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_HEALING-1);

            //Make sure that this spell applies an aura
            if ( TempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA )
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_AURA-1);
        }
    }
}

void ScriptedAI::DoZoneInCombat(Unit* pUnit, bool force)
{
    if (!pUnit)
        pUnit = m_creature;

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
    for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
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

void ScriptedAI::DoResetThreat()
{
    if (!m_creature->CanHaveThreatList() || m_creature->getThreatManager().isThreatListEmpty())
    {
        error_log("TSCR: DoResetThreat called for creature that either cannot have threat list or has empty threat list (m_creature entry = %d)", m_creature->GetEntry());

        return;
    }

    std::list<HostilReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
    std::list<HostilReference*>::iterator itr;

    for(itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
    {
        Unit* pUnit = NULL;
        pUnit = Unit::GetUnit((*m_creature), (*itr)->getUnitGuid());
        if(pUnit && DoGetThreat(pUnit))
            DoModifyThreatPercent(pUnit, -100);
    }
}

float ScriptedAI::DoGetThreat(Unit* pUnit)
{
    if(!pUnit) return 0.0f;
    return m_creature->getThreatManager().getThreat(pUnit);
}

void ScriptedAI::DoModifyThreatPercent(Unit *pUnit, int32 pct)
{
    if(!pUnit) return;
    m_creature->getThreatManager().modifyThreatPercent(pUnit, pct);
}

void ScriptedAI::DoTeleportTo(float x, float y, float z, uint32 time)
{
    me->Relocate(x, y, z);
    float speed = me->GetDistance(x, y, z) / ((float)time * 0.001f);
    me->MonsterMoveWithSpeed(x, y, z, speed);
}

void ScriptedAI::DoTeleportPlayer(Unit* pUnit, float x, float y, float z, float o)
{
    if(!pUnit || pUnit->GetTypeId() != TYPEID_PLAYER)
    {
        if(pUnit)
            error_log("TSCR: Creature %u (Entry: %u) Tried to teleport non-player unit (Type: %u GUID: %u) to x: %f y:%f z: %f o: %f. Aborted.", m_creature->GetGUID(), m_creature->GetEntry(), pUnit->GetTypeId(), pUnit->GetGUID(), x, y, z, o);
        return;
    }

    (pUnit->ToPlayer())->TeleportTo(pUnit->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

void ScriptedAI::DoTeleportAll(float x, float y, float z, float o)
{
    Map *map = m_creature->GetMap();
    if (!map->IsDungeon())
        return;

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        if (Player* i_pl = i->GetSource())
            if (i_pl->IsAlive())
                i_pl->TeleportTo(m_creature->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

Unit* FindCreature(uint32 entry, float range, Unit* Finder)
{
    if(!Finder)
        return NULL;
    Creature* target = NULL;

    CellPair pair(Trinity::ComputeCellPair(Finder->GetPositionX(), Finder->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::AllCreaturesOfEntryInRange check(Finder, entry, range);
    Trinity::CreatureSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(target, check);
    TypeContainerVisitor<Trinity::CreatureSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);
    cell.Visit(pair, visitor, *Finder->GetMap(), *Finder, range);
    
    return target;
}

void FindCreatures(std::list<Creature*>& list, uint32 entry, float range, Unit* Finder)
{
    CellPair pair(Trinity::ComputeCellPair(Finder->GetPositionX(), Finder->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::AllCreaturesOfEntryInRange check(Finder, entry, range);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(list, check);
    TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *Finder->GetMap());
}

GameObject* FindGameObject(uint32 entry, float range, Unit* Finder)
{
    if(!Finder)
        return NULL;
    GameObject* target = NULL;

    CellPair pair(Trinity::ComputeCellPair(Finder->GetPositionX(), Finder->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::AllGameObjectsWithEntryInGrid go_check(entry);
    Trinity::GameObjectSearcher<Trinity::AllGameObjectsWithEntryInGrid> searcher(target, go_check);
    TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::AllGameObjectsWithEntryInGrid>, GridTypeMapContainer> visitor(searcher);
    cell.Visit(pair, visitor, *Finder->GetMap(), *Finder, range);
    return target;
}

Unit* ScriptedAI::DoSelectLowestHpFriendly(float range, uint32 MinHPDiff)
{
    Unit* pUnit = NULL;
    Trinity::MostHPMissingInRange u_check(m_creature, range, MinHPDiff);
    Trinity::UnitLastSearcher<Trinity::MostHPMissingInRange> searcher(pUnit, u_check);
    m_creature->VisitNearbyObject(range, searcher);
    return pUnit;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyCC(float range)
{
    std::list<Creature*> pList;
    Trinity::FriendlyCCedInRange u_check(m_creature, range);
    Trinity::CreatureListSearcher<Trinity::FriendlyCCedInRange> searcher(pList, u_check);
    m_creature->VisitNearbyObject(range, searcher);
    return pList;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyMissingBuff(float range, uint32 spellid)
{
    std::list<Creature*> pList;
    Trinity::FriendlyMissingBuffInRange u_check(m_creature, range, spellid);
    Trinity::CreatureListSearcher<Trinity::FriendlyMissingBuffInRange> searcher(pList, u_check);
    m_creature->VisitNearbyObject(range, searcher);
    return pList;
}

/*void Scripted_NoMovementAI::MoveInLineOfSight(Unit *who)
{
    if( !m_creature->GetVictim() && m_creature->CanAttack(who) && ( m_creature->IsHostileTo( who )) && who->isInAccessiblePlaceFor(m_creature) )
    {
        if (!m_creature->CanFly() && m_creature->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
            return;

        float attackRadius = m_creature->GetAttackDistance(who);
        if( m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->IsWithinLOSInMap(who) )
        {
            who->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
            AttackStart(who);
        }
    }
}*/

void LoadOverridenSQLData()
{
    GameObjectTemplate *goInfo;

    // Sunwell Plateau : Kalecgos : Spectral Rift
    goInfo = const_cast<GameObjectTemplate*>(sObjectMgr->GetGameObjectTemplate(187055));
    if(goInfo && goInfo->type == GAMEOBJECT_TYPE_GOOBER)
        goInfo->goober.lockId = 57; // need LOCKTYPE_QUICK_OPEN
}

void LoadOverridenDBCData()
{
    SpellEntry *spellInfo;

    // Black Temple : Illidan : Parasitic Shadowfiend Passive
    spellInfo = const_cast<SpellEntry*>(sSpellMgr->GetSpellInfo(41913));
    if(spellInfo)
        spellInfo->EffectApplyAuraName[0] = 4; // proc debuff, and summon infinite fiends
}