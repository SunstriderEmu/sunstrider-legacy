/*
 * Copyright (C) 2009 - 2011 Windrunner
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

#include "precompiled.h"
#include "Creature.h"
#include "CreatureAINew.h"
#include "World.h"
#include "Log.h"
#include "CreatureTextMgr.h"
#include "Spell.h"

CreatureAINew::~CreatureAINew()
{
    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++)
        delete itr->second;
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

void CreatureAINew::update(const uint32 diff)
{
    if (me->isAlive() && updateVictim()) {
        if (me->isAttackReady()) {
            if (me->IsWithinMeleeRange(me->getVictim())) {
                me->AttackerStateUpdate(me->getVictim());
                me->resetAttackTimer();
            }
        }
    }
}

void CreatureAINew::attackStart(Unit* victim)
{
    if (!victim)
        return;
    if (me->Attack(victim, true)) {
        if (me->isPet()) {
            if (victim->getVictim() && victim->getVictim()->GetGUID() != me->GetGUID())
                me->GetMotionMaster()->MoveChase(victim, CONTACT_DISTANCE, M_PI);
            else
                me->GetMotionMaster()->MoveChase(victim);
        }
        else
            me->GetMotionMaster()->MoveChase(victim);

        if (!aiInCombat()) {
            setAICombat(true);
            onCombatStart(victim);
        }
    }
}

void Creature_NoMovementAINew::attackStart(Unit* victim)
{
    if (!victim)
        return;

    if (me->Attack(victim, false)) {
        me->GetMotionMaster()->MoveIdle();

        if (!aiInCombat()) {
            setAICombat(true);
            onCombatStart(victim);
        }
    }
}

void CreatureAINew::evade()
{
    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->CombatStop();
    me->LoadCreaturesAddon();
    me->SetLootRecipient(NULL);
    me->ResetPlayerDamageReq();

    if (me->isAlive())
        me->GetMotionMaster()->MoveTargetedHome();

    setAICombat(false);
    onReset(false);
}

bool CreatureAINew::updateVictim(bool evade)
{
    if (!me->isInCombat())
        return false;

    if (Unit *victim = me->SelectVictim(evade))
        attackStart(victim);

    return me->getVictim();
}

void CreatureAINew::onMoveInLoS(Unit* who)
{
    if (me->getVictim())
        return;

    if (me->canStartAttack(who))
        attackStart(who);
    else if (who->getVictim() && me->IsFriendlyTo(who)
        && me->IsWithinDistInMap(who, sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS))
        && me->canAttack(who->getVictim()))
        attackStart(who->getVictim());
}

void CreatureAINew::doMeleeAttackIfReady()
{
    if (me->isAttackReady() && !me->hasUnitState(UNIT_STAT_CASTING)) {
        if (me->IsWithinMeleeRange(me->getVictim())) {
            me->AttackerStateUpdate(me->getVictim());
            me->resetAttackTimer();
        }
    }

    if (me->haveOffhandWeapon() && me->isAttackReady(OFF_ATTACK) && !me->hasUnitState(UNIT_STAT_CASTING)) {
        if (me->IsWithinMeleeRange(me->getVictim())) {
            me->AttackerStateUpdate(me->getVictim(), OFF_ATTACK);
            me->resetAttackTimer(OFF_ATTACK);
        }
    }
}

void CreatureAINew::addEvent(uint8 id, uint32 minTimer, uint32 maxTimer, uint32 flags, bool activeByDefault, uint32 phaseMask)
{
    m_events[id] = new AIEvent(id, minTimer, maxTimer, flags, activeByDefault, phaseMask);
}

void CreatureAINew::scheduleEvent(uint8 id, uint32 minTimer, uint32 maxTimer)
{
    if (id >= EVENT_MAX_ID) {
        sLog.outError("CreatureAINew::schedule: event has too high id (%u) for creature: %u (entry: %u).", id, me->GetDBTableGUIDLow(), me->GetEntry());
        return;
    }

    if (minTimer > maxTimer) {
        sLog.outError("CreatureAINew::schedule: event %u has minTimer > maxTimer for creature: %u (entry: %u), swapping timers.", id, me->GetDBTableGUIDLow(), me->GetEntry());
        std::swap(minTimer, maxTimer);
    }
    
    EventMap::iterator itr = m_events.find(id);
    if (itr != m_events.end())
        itr->second->calcTimer(minTimer, maxTimer);
    else
        sLog.outError("CreatureAINew::schedule: Event %u is not set for creature %u (entry; %u).", id, me->GetDBTableGUIDLow(), me->GetEntry());
}

void CreatureAINew::disableEvent(uint8 id)
{
    EventMap::iterator itr = m_events.find(id);
    if (itr != m_events.end())
        itr->second->active = false;
    else
        sLog.outError("CreatureAINew::disableEvent: Event %u is not set for creature %u (entry: %u).", id, me->GetDBTableGUIDLow(), me->GetEntry());
}

void CreatureAINew::enableEvent(uint8 id)
{
    EventMap::iterator itr = m_events.find(id);
    if (itr != m_events.end())
        itr->second->active = true;
    else
        sLog.outError("CreatureAINew::enableEvent: Event %u is not set for creature %u (entry: %u).", id, me->GetDBTableGUIDLow(), me->GetEntry());
}

void CreatureAINew::delayEvent(uint8 id, uint32 delay)
{
    EventMap::iterator itr = m_events.find(id);
    if (itr == m_events.end()) {
        sLog.outError("CreatureAINew::delay: Event %u is not set for creature %u (entry: %u).", id, me->GetDBTableGUIDLow(), me->GetEntry());
        return;
    }
    
    itr->second->timer += delay;
}

void CreatureAINew::delayAllEvents(uint32 delay)
{
    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++)
        itr->second->timer += delay;
}

bool CreatureAINew::executeEvent(uint32 const diff, uint8& id)
{
    uint8 exec = EVENT_MAX_ID;
    uint32 minTimer = 0xFFFFFFFF;
    AIEvent* selected = NULL;

    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++) {
        AIEvent* evt = itr->second;
        if (!evt->active)
            continue;
  
        if (!evt->isActiveInPhase(m_phase))
            continue;

        if (evt->timer <= diff && evt->timer < minTimer) {
            if (me->IsNonMeleeSpellCasted(false) && (evt->flags & EVENT_FLAG_DELAY_IF_CASTING)) {
                evt->timer = 1; // Delay to next tick with high priority
                continue;
            }
            else {
                exec = itr->first;
                minTimer = evt->timer;
                selected = evt;
            }
        }
    }
    
    if (exec < EVENT_MAX_ID) {
        id = exec;
        return true;
    }
    
    return false;
}

void CreatureAINew::updateEvents(uint32 const diff)
{
    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++) {
        if (!itr->second->active)
            continue;
            
        if (!itr->second->isActiveInPhase(m_phase))
            continue;

        if (itr->second->timer > diff)
            itr->second->timer -= diff;
    }
}

void CreatureAINew::doCast(Unit* victim, uint32 spellId, bool triggered, bool interrupt)
{
    if (me->hasUnitState(UNIT_STAT_CASTING) && !triggered && !interrupt)
        return;

    if (interrupt && me->IsNonMeleeSpellCasted(false))
        me->InterruptNonMeleeSpells(false);

    if (victim)
        me->CastSpell(victim, spellId, triggered);
    else
        me->CastSpell((Unit*)NULL, spellId, triggered);
}

Unit* CreatureAINew::selectUnit(SelectedTarget target, uint32 position)
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

bool CreatureAINew::checkTarget(Unit* target, bool playersOnly, float radius)
{
    if (!me)
        return false;

    if (!target)
        return false;

    if (playersOnly && (target->GetTypeId() != TYPEID_PLAYER))
        return false;

    if (radius > 0.0f && !me->IsWithinCombatRange(target, radius))
        return false;

    if (radius < 0.0f && me->IsWithinCombatRange(target, -radius))
        return false;

    return true;
}

Unit* CreatureAINew::selectUnit(SelectedTarget targetType, uint32 position, float radius, bool playersOnly)
{
    std::list<HostilReference*>& threatlist = me->getThreatManager().getThreatList();
    if (position >= threatlist.size())
        return NULL;

    std::list<Unit*> targetList;
    for (std::list<HostilReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius))
            targetList.push_back((*itr)->getTarget());

    if (position >= targetList.size())
        return NULL;

    if (targetType == TARGET_NEAREST || targetType == TARGET_FARTHEST)
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

void CreatureAINew::selectUnitList(std::list<Unit*>& targetList, uint32 maxTargets, SelectedTarget targetType, float radius, bool playersOnly)
{
    std::list<HostilReference*> const& threatlist = me->getThreatManager().getThreatList();
        if (threatlist.empty())
            return;

    for (std::list<HostilReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius))
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

void CreatureAINew::getAllPlayersInRange(std::list<Player*>& players, float range)
{
    CellPair pair(Trinity::ComputeCellPair(me->GetPositionX(), me->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();
    
    Trinity::AllPlayersInRange check(me, range);
    Trinity::PlayerListSearcher<Trinity::AllPlayersInRange> searcher(players, check);
    TypeContainerVisitor<Trinity::PlayerListSearcher<Trinity::AllPlayersInRange>, GridTypeMapContainer> visitor(searcher);
    cell.Visit(pair, visitor, *me->GetMap(), *me, range);
}

void CreatureAINew::setZoneInCombat()
{
    Map* map = me->GetMap();

    if (!map->IsDungeon()) {
        sLog.outError("CreatureAI::setZoneInCombat called on a map that is not an instance (creature entry = %u)", me->GetEntry());
        return;
    }

    if (!me->CanHaveThreatList() || me->getThreatManager().isThreatListEmpty()) {
        error_log("CreatureAI::setZoneInCombat called for a creature that either cannot have a threat list or has empty threat list (creature entry = %u)", me->GetEntry());
        return;
    }

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i) {
        if (Player* i_pl = i->getSource()) {
            if (i_pl->isAlive()) {
                me->SetInCombatWith(i_pl);
                i_pl->SetInCombatWith(me);
                me->AddThreat(i_pl, 0.0f);
            }
        }
    }
}

uint32 CreatureAINew::talk(uint8 groupid, uint64 targetGUID)
{
    return sCreatureTextMgr.SendChat(me, groupid, targetGUID);
}

void CreatureAINew::deleteFromThreatList(uint64 guid)
{
    for (std::list<HostilReference*>::iterator itr = me->getThreatManager().getThreatList().begin(); itr != me->getThreatManager().getThreatList().end(); ++itr) {
        if((*itr)->getUnitGuid() == guid) {
            (*itr)->removeReference();
            break;
        }
    }
}

void CreatureAINew::deleteFromThreatList(Unit* target)
{
    if (!target)
        return;
        
    deleteFromThreatList(target->GetGUID());
}

void CreatureAINew::doTeleportTo(float x, float y, float z, uint32 time)
{
    me->Relocate(x,y,z);
    me->SendMonsterMove(x, y, z, time);
}

void CreatureAINew::doResetThreat()
{
    if (!me->CanHaveThreatList() || me->getThreatManager().isThreatListEmpty())
        return;

    std::list<HostilReference*>& m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostilReference*>::iterator itr;

    for(itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr) {
        Unit* pUnit = NULL;
        pUnit = Unit::GetUnit((*me), (*itr)->getUnitGuid());
        if (pUnit && doGetThreat(pUnit))
            doModifyThreatPercent(pUnit, -100);
    }
}

float CreatureAINew::doGetThreat(Unit* unit)
{
    if (!unit)
        return 0.0f;

    return me->getThreatManager().getThreat(unit);
}

void CreatureAINew::doModifyThreatPercent(Unit* unit, int32 pct)
{
    if (!unit)
        return;

    me->getThreatManager().modifyThreatPercent(unit, pct);
}

void CreatureAINew::doModifyThreat(Unit* unit, float threat)
{
    if (!unit)
        return;
        
    me->AddThreat(unit, threat);
}
