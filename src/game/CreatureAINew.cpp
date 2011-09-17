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

#include "Creature.h"
#include "CreatureAINew.h"
#include "World.h"
#include "Log.h"

void CreatureAINew::update(const uint32 diff)
{
    sLog.outString("Update");
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
        //DEBUG_LOG("Creature %s tagged a victim to kill [guid=%u]", me->GetName(), victim->GetGUIDLow());
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

void CreatureAINew::schedule(uint8 id, uint32 minTimer, uint32 maxTimer)
{
    if (id >= EVENT_MAX_ID) {
        sLog.outError("CreatureAINew::schedule: event has too high id for creature: %u (entry: %u).", me->GetDBTableGUIDLow(), me->GetEntry());
        return;
    }

    if (minTimer > maxTimer) {
        sLog.outError("CreatureAINew::schedule: event %u has minTimer > maxTimer for creature: %u (entry: %u), swapping timers.", id, me->GetDBTableGUIDLow(), me->GetEntry());
        std::swap(minTimer, maxTimer);
    }
    
    if (minTimer == maxTimer) {
        m_events[id] = minTimer;
        return;
    }
    
    m_events[id] = minTimer + rand()%(maxTimer - minTimer);
}

void CreatureAINew::delay(uint8 id, uint32 delay)
{
    EventMap::iterator itr = m_events.find(id);
    if (itr == m_events.end()) {
        sLog.outError("CreatureAINew::delay: failed attempt to delay non-existant event %u for creature: %u (entry: %u).", id, me->GetDBTableGUIDLow(), me->GetEntry());
        return;
    }
    
    itr->second += delay;
}

void CreatureAINew::delayAll(uint32 delay)
{
    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++)
        itr->second += delay;
}

bool CreatureAINew::executeEvent(uint32 const diff, uint8& id)
{
    uint8 exec = EVENT_MAX_ID;
    uint32 minTimer = 0xFFFFFFFF;

    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++) {
        if (itr->second <= diff && itr->second < minTimer) {
            exec = itr->first;
            minTimer = itr->second;
        }
    }
    
    if (exec < EVENT_MAX_ID) {
        setExecuted(exec);
        id = exec;
        
        return true;
    }
    
    return false;
}

void CreatureAINew::updateEvents(uint32 const diff)
{
    for (EventMap::iterator itr = m_events.begin(); itr != m_events.end(); itr++) {
        if (itr->second > diff)
            itr->second -= diff;
    }
}

void CreatureAINew::doCast(Unit* victim, uint32 spellId, bool triggered, bool interrupt)
{
    if (!victim || me->hasUnitState(UNIT_STAT_CASTING) && !triggered)
        return;

    if (interrupt && me->IsNonMeleeSpellCasted(false))
        me->InterruptNonMeleeSpells(false);

    me->CastSpell(victim, spellId, triggered);
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
