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

void CreatureAI::Talk(uint8 id, uint64 WhisperGuid)
{
    sCreatureTextMgr->SendChat(me, id, WhisperGuid);
}

void CreatureAI::MoveInLineOfSight(Unit *who)
{
    if(me->GetVictim())
        return;
        
    if (me->getAI())
        return;

    if (me->HasJustRespawned() && !me->GetSummonerGUID())
        return;

    if(me->canStartAttack(who))
        AttackStart(who);
    else if(who->GetVictim() && me->IsFriendlyTo(who)
        && me->IsWithinDistInMap(who, sWorld->getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS))
        && me->CanCallAssistance()
        && who->GetVictim()->GetTypeId() != CREATURE_TYPE_CRITTER
        && me->CanAttack(who->GetVictim())) {
        if (who->GetTypeId() != TYPEID_UNIT || who->ToCreature()->CanCallAssistance()) {
            if (me->GetScriptName() == "guard_contested") {
                if (who->GetVictim() && !who->GetVictim()->IsInCombat())
                    return;
            }
            
            me->SetNoCallAssistance(true);
            AttackStart(who->GetVictim());
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

void CreatureAI::AttackStartIfCan(Unit* victim)
{
    if(!victim)
        return;

    //Merge conflict : set CanStartAttack
    if(me->CanAttack(victim))
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
    if(!target || !charmer->CanAttack(target))
        AttackStart(charmer->SelectNearestTarget());
}