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
 
#include "CreatureScript.h"

class Creature;

bool CreatureScript::updateVictim(bool evade)
{
    if(!me->isInCombat())
        return false;

    if (Unit *victim = me->SelectVictim(evade))
        attackStart(victim);

    return me->getVictim();
}

void CreatureScript::doMeleeAttackIfReady()
{
    // Make sure our attack is ready and we aren't currently casting before checking distance
    if (me->isAttackReady() && !me->hasUnitState(UNIT_STAT_CASTING)) {
        // If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim())) {
            me->AttackerStateUpdate(me->getVictim());
            me->resetAttackTimer();
        }
    }
    
    if (me->haveOffhandWeapon() && me->isAttackReady(OFF_ATTACK) && !me->hasUnitState(UNIT_STAT_CASTING)) {
        // If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim())) {
            me->AttackerStateUpdate(me->getVictim(), OFF_ATTACK);
            me->resetAttackTimer(OFF_ATTACK);
        }
    }
}

void CreatureScript::attackStart(Unit* victim)
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
    }
}

void CreatureScript::onMoveInLoS(Unit* who)
{
    if (me->getVictim())
        return;

    if (me->canStartAttack(who))
        AttackStart(who);
    else if (who->getVictim() && me->IsFriendlyTo(who)
        && me->IsWithinDistInMap(who, sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS))
        && me->canAttack(who->getVictim()))
        attackStart(who->getVictim());
}

void CreatureScript::evade()
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
