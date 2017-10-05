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

CreatureAI::CreatureAI(Creature *c) : UnitAI((Unit*)c), me(c), m_MoveInLineOfSight_locked(false)
{

}

//Disable CreatureAI when charmed
void CreatureAI::OnCharmed(Unit* charmer, bool apply)
{
    //me->IsAIEnabled = !apply;*/
    me->NeedChangeAI = true;
    me->IsAIEnabled = false;
}

void CreatureAI::OnPossess(Unit* charmer, bool apply) {}

void CreatureAI::Talk(uint8 id, WorldObject const* whisperTarget /*= nullptr*/)
{
    sCreatureTextMgr->SendChat(me, id, whisperTarget);
}

bool CreatureAI::AssistPlayerInCombatAgainst(Unit* who)
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
      /*  if (!(me->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_CAN_ASSIST))
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

// scripts does not take care about MoveInLineOfSight loops
// MoveInLineOfSight can be called inside another MoveInLineOfSight and cause stack overflow
void CreatureAI::MoveInLineOfSight_Safe(Unit* who)
{
    if (m_MoveInLineOfSight_locked == true)
        return;
    m_MoveInLineOfSight_locked = true;
    MoveInLineOfSight(who);
    m_MoveInLineOfSight_locked = false;
}

void CreatureAI::MoveInLineOfSight(Unit* who)
{
    //if has just respawned and not a summon, wait a bit before reacting
    if (me->HasJustRespawned() && !me->GetSummonerGUID())
        return;

    if (me->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET) // non-combat pets should just stand there and look good;)
        return;

    if (!me->HasReactState(REACT_AGGRESSIVE))
        return;

    //attack target if no current victim, else just enter combat with it
    if (!me->GetVictim())
    {
        if (AssistPlayerInCombatAgainst(who))
            return;

        CanAttackResult result = me->CanAggro(who, false);
        if (result != CAN_ATTACK_RESULT_OK)
            return;

        if (me->HasUnitState(UNIT_STATE_DISTRACTED))
        {
            me->ClearUnitState(UNIT_STATE_DISTRACTED);
            me->GetMotionMaster()->Clear();
        }

        me->ClearUnitState(UNIT_STATE_EVADE);
        who->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
        AttackStart(who);
    } else {
        if(!me->IsInCombatWith(who))
        {
            who->SetInCombatWith(me);
            me->AddThreat(who, 0.0f);
        }
    }
}

void CreatureAI::_OnOwnerCombatInteraction(Unit* target)
{
    if (!target || !me->IsAlive())
        return;

    if (!me->HasReactState(REACT_PASSIVE) && /*me->CanStartAttack(target, true)*/ me->CanAttack(target, true))
    {
        if (me->IsInCombat())
            me->AddThreat(target, 0.0f);
        else
            AttackStart(target);
    }
}

bool CreatureAI::UpdateVictim(bool evade)
{
    if(!me->IsInCombat())
        return false;

    if (!me->HasReactState(REACT_PASSIVE))
    {
        if (Unit* victim = me->SelectVictim(evade))
            if (!me->IsFocusing(nullptr, true) && victim != me->GetVictim())
                AttackStart(victim);

        return me->GetVictim() != nullptr;
    } else if (me->GetThreatManager().isThreatListEmpty())
    {
        EnterEvadeMode(EVADE_REASON_NO_HOSTILES);
        return false;
    }

    return true;
}

void CreatureAI::EnterEvadeMode(EvadeReason why)
{
    if (!_EnterEvadeMode(why))
        return;

    if (!me->IsAlive())
        return;

    //TC_LOG_DEBUG("entities.unit", "Creature %u enters evade mode.", me->GetEntry());

#ifdef LICH_KING
    if (!me->GetVehicle()) // otherwise me will be in evade mode forever
    {
#endif
        if (Unit* owner = me->GetCharmerOrOwner())
        {
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle(), MOTION_SLOT_ACTIVE);
        }
        else
        {
            me->GetMotionMaster()->MoveTargetedHome();
        }
#ifdef LICH_KING
    }
#endif

    Reset();

#ifdef LICH_KING
    if (me->IsVehicle()) // use the same sequence of addtoworld, aireset may remove all summons!
        me->GetVehicleKit()->Reset(true);
#endif
}

bool CreatureAI::_EnterEvadeMode(EvadeReason /*why*/)
{
    me->RemoveAurasOnEvade();
    me->DeleteThreatList();
    me->CombatStop();
    me->InitCreatureAddon();
    me->SetLootRecipient(nullptr);
    me->ResetPlayerDamageReq();
    me->SetLastDamagedTime(0);
    me->SetCannotReachTarget(false);

    if (me->IsInEvadeMode())
        return false;

    return true;
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
        for (const auto & i : PlayerList)
        {
            if (me->IsWithinMeleeRange(i.GetSource()))
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
        if (!force && pUnit->GetThreatManager().isThreatListEmpty())
        {
            error_log("TSCR: DoZoneInCombat called for creature that either cannot have threat list or has empty threat list (pUnit entry = %d)", pUnit->GetTypeId() == TYPEID_UNIT ? (pUnit->ToCreature())->GetEntry() : 0);

            return;
        }
    }

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for (const auto & i : PlayerList)
    {
        if (Player* i_pl = i.GetSource())
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
