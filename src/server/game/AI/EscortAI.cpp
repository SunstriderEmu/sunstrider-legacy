/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

/* ScriptData
SDName: Npc_EscortAI
SD%Complete: 100
SDComment:
SDCategory: Npc
EndScriptData */


#include "EscortAI.h"

#define WP_LAST_POINT   0xFFFFFF

npc_escortAI::npc_escortAI(Creature *c) : ScriptedAI(c),
PlayerTimer(1000), MaxPlayerDistance(DEFAULT_MAX_PLAYER_DISTANCE), CanMelee(true),
DespawnAtEnd(true), DespawnAtFar(true), m_uiEscortState(STATE_ESCORT_NONE)
{
	me->GetPosition(LastPos.x, LastPos.y, LastPos.z);
}

void npc_escortAI::AttackStart(Unit *who) 
{
    if (!who)
        return;

    if (HasEscortState(STATE_ESCORT_ESCORTING) && !Defend)
        return;

    if ( me->Attack(who, true) )
    {
        MovementGeneratorType type = me->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_ACTIVE);
        if (type == WAYPOINT_MOTION_TYPE || type == POINT_MOTION_TYPE)
        {
            me->GetMotionMaster()->MovementExpired();
            //me->DisableSpline();
            me->StopMoving();
        }

        if (!me->IsInCombat())
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                //Store last position
                me->GetPosition(LastPos.x, LastPos.y, LastPos.z);
            }
        }

        me->GetMotionMaster()->MovementExpired();
        if (IsCombatMovementAllowed())
            me->GetMotionMaster()->MoveChase(who);
    }
}

void npc_escortAI::MoveInLineOfSight(Unit *who)
{
    if (HasEscortState(STATE_ESCORT_ESCORTING) && AssistPlayerInCombat(who))
        return;

    if (HasEscortState(STATE_ESCORT_ESCORTING) && !Attack)
        return;

    if(me->GetVictim() || me->CanAggro(who) != CAN_ATTACK_RESULT_OK)
        return;

    AttackStart(who);
}

void npc_escortAI::JustRespawned()
{
    RemoveEscortState(STATE_ESCORT_ESCORTING);
    IsOnHold = false;

    //Re-Enable questgiver flag
    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

    Reset();
}

void npc_escortAI::EnterEvadeMode(EvadeReason /* why */)
{
    me->RemoveAllAuras();
    me->GetThreatManager().ClearAllThreat();
    me->CombatStop();
    me->SetLootRecipient(nullptr);

    if (HasEscortState(STATE_ESCORT_ESCORTING) || !DespawnAtEnd)
    {
        AddEscortState(STATE_ESCORT_RETURNING);
        me->GetMotionMaster()->MovementExpired();
        me->GetMotionMaster()->MovePoint(WP_LAST_POINT, LastPos.x, LastPos.y, LastPos.z);

    }else
    {
        me->GetMotionMaster()->MovementExpired();
        me->GetMotionMaster()->MoveTargetedHome();
    }
    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

    Reset();
}

void npc_escortAI::UpdateAI(const uint32 diff)
{
    //Waypoint Updating
    if (HasEscortState(STATE_ESCORT_ESCORTING) && !me->IsInCombat() && WaitTimer && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (WaitTimer <= diff)
        {
            if (ReconnectWP)
            {
                //Correct movement speed
                if (Run)
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                else
                    me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);

                //Continue with waypoints
                if( !IsOnHold )
                {
                    me->GetMotionMaster()->MovePoint(CurrentWP->id, CurrentWP->x, CurrentWP->y, CurrentWP->z );
                    WaitTimer = 0;
                    ReconnectWP = false;
                    return;
                }
            }

            //End of the line, Despawn self then immediatly respawn
            if (CurrentWP == WaypointList.end())
            {
                RemoveEscortState(STATE_ESCORT_ESCORTING);
                if(DespawnAtEnd)
                {
                    me->SetDeathState(JUST_DIED);
                    me->SetHealth(0);
                    me->CombatStop();
                    me->GetThreatManager().ClearAllThreat();
                    me->Respawn();
                    me->GetMotionMaster()->Clear(true);

                    //Re-Enable gossip
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                    WaitTimer = 0;
                    return;
                }else{
                    WaitTimer = 0;
                    return;
                }
            }

            if( !IsOnHold )
            {
                me->GetMotionMaster()->MovePoint(CurrentWP->id, CurrentWP->x, CurrentWP->y, CurrentWP->z );
                WaitTimer = 0;
            }
        }else WaitTimer -= diff;
    }

    //Check if player is within range if he's a player
    Player* player = ObjectAccessor::GetPlayer(*me, PlayerGUID);
    if (HasEscortState(STATE_ESCORT_ESCORTING) && !me->IsInCombat() && PlayerGUID && player && player->GetSession()->GetSecurity() <= SEC_PLAYER)
    {
        if (PlayerTimer < diff)
        {
            Unit* p = ObjectAccessor::GetUnit(*me, PlayerGUID);

            if (DespawnAtFar && (!p || me->GetDistance(p) > GetMaxPlayerDistance()))
            {
                JustDied(me);
                RemoveEscortState(STATE_ESCORT_ESCORTING);

                me->SetDeathState(JUST_DIED);
                me->SetHealth(0);
                me->CombatStop();
                me->GetThreatManager().ClearAllThreat();
                me->Respawn();
                me->GetMotionMaster()->Clear(true);

                //Re-Enable gossip
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }

            PlayerTimer = 1000;
        }else PlayerTimer -= diff;
    }

    if(CanMelee)
    {
        //Check if we have a current target
        if( me->IsAlive() && UpdateVictim())
            DoMeleeAttackIfReady();
    }
}

void npc_escortAI::MovementInform(uint32 type, uint32 id)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    if (type == POINT_MOTION_TYPE)
    {
        //Original position reached, continue waypoint movement
        if (id == WP_LAST_POINT)
        {
            ReconnectWP = true;
            WaitTimer = 1;
            RemoveEscortState(STATE_ESCORT_RETURNING);
        }
        else if (CurrentWP != WaypointList.end())
        {
            //Make sure that we are still on the right waypoint
            if (CurrentWP->id != id)
            {
                return;
            }

            //Call WP function
            WaypointReached(CurrentWP->id);

            WaitTimer = CurrentWP->WaitTimeMs + 1;

            ++CurrentWP;
        }
    }
    else if (type == WAYPOINT_MOTION_TYPE)
    {
        if (WaitTimer <= 1 && /* !HasEscortState(STATE_ESCORT_PAUSED) && */ CurrentWP != WaypointList.end())
        {
            //Call WP function
            me->UpdatePosition(CurrentWP->x, CurrentWP->y, CurrentWP->z, me->GetOrientation());
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            WaypointReached(CurrentWP->id);

            WaitTimer = CurrentWP->WaitTimeMs + 1;

            ++CurrentWP;

            if (WaitTimer > 1 || HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (me->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_ACTIVE) == WAYPOINT_MOTION_TYPE)
                    me->GetMotionMaster()->MovementExpired();
                me->StopMovingOnCurrentPos();
                me->GetMotionMaster()->MoveIdle();
            }
        }
    }
}

Player* npc_escortAI::GetPlayerForEscort() 
{ 
    return ObjectAccessor::GetPlayer(*me, PlayerGUID);
}

void npc_escortAI::OnPossess(Unit* charmer, bool apply)
{
    // We got possessed in the middle of being escorted, store the point
    // where we left off to come back to when possess is removed
    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        if (apply)
            me->GetPosition(LastPos.x, LastPos.y, LastPos.z);
        else
        {
            AddEscortState(STATE_ESCORT_RETURNING);
            me->GetMotionMaster()->MovementExpired();
            me->GetMotionMaster()->MovePoint(WP_LAST_POINT, LastPos.x, LastPos.y, LastPos.z);
        }
    }
}

void npc_escortAI::AddWaypoint(uint32 id, float x, float y, float z, uint32 WaitTimeMs)
{
    Escort_Waypoint t(id, x, y, z, WaitTimeMs);

    WaypointList.push_back(t);
}

void npc_escortAI::SetRun(bool bRun)
{
    if (bRun)
    {
        if (!Run)
            me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
        else
            TC_LOG_ERROR("scripts","EscortAI attempt to set run mode for creature %u, but is already running.", me->GetEntry());
    }
    else
    {
        if (Run)
            me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
        else
            TC_LOG_ERROR("scripts","EscortAI attempt to set walk mode for creature %u, but is already walking.", me->GetEntry());
    }
    Run = bRun;
}

void npc_escortAI::GetWaypointListFromDB(uint32 entry)
{
    if (entry == 0)
        return; //entry = 0 means the script use old waypoint system, with AddWaypoint in the script constructor
    QueryResult result = WorldDatabase.PQuery("SELECT pointid, location_x, location_y, location_z, waittime FROM escort_waypoints WHERE entry = '%u' ORDER BY pointid", entry);
    
    if (!result)
    {
        TC_LOG_ERROR("scripts","SD2 ERROR: EscortAI: Attempt to GetWaypointListFromDB for creature entry %u, but no record found in DB !", entry);
        return;
    }
    
    uint16 pointid;
    float location_x, location_y, location_z;
    uint32 waittime;
    
    do
    {
        Field* fields = result->Fetch();
        pointid = fields[0].GetUInt16();
        location_x = fields[1].GetFloat();
        location_y = fields[2].GetFloat();
        location_z = fields[3].GetFloat();
        waittime = fields[4].GetUInt32();
        
        AddWaypoint(pointid, location_x, location_y, location_z, waittime);        
    }while (result->NextRow());
}

void npc_escortAI::Start(bool bAttack, bool bDefend, bool bRun, uint64 pGUID, uint32 entry)
{
    if (me->IsInCombat())
        return;

    GetWaypointListFromDB(entry);

    if (WaypointList.empty()) {
        TC_LOG_ERROR("scripts","Call to escortAI::Start with 0 waypoints for creature %u", me->GetEntry());
        return;
    }

    Attack = bAttack;
    Defend = bDefend;
    Run = bRun;
    PlayerGUID = pGUID;

    CurrentWP = WaypointList.begin();

    //Set initial speed
    if (Run)
        me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
    else me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);

    //Start WP
    me->GetMotionMaster()->MovePoint(CurrentWP->id, CurrentWP->x, CurrentWP->y, CurrentWP->z );
    AddEscortState(STATE_ESCORT_ESCORTING);
    ReconnectWP = false;
    RemoveEscortState(STATE_ESCORT_RETURNING);
    IsOnHold = false;

    //Disable questgiver flag
    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    
}

void npc_escortAI::SetEscortPaused(bool bPaused)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;
        
    IsOnHold = bPaused;

    if (bPaused)
        AddEscortState(STATE_ESCORT_PAUSED);
    else
        RemoveEscortState(STATE_ESCORT_PAUSED);
}

bool npc_escortAI::AssistPlayerInCombat(Unit* who)
{
    if (!who || !who->GetVictim())
        return false;

    //experimental (unknown) flag not present
    if (!(me->GetCreatureTemplate()->type_flags & 0x001000/*CREATURE_TYPEFLAGS_AID_PLAYERS*/))
        return false;

    //not a player
    if (!who->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    //never attack friendly
    if (me->IsFriendlyTo(who))
        return false;

    //too far away and no free sight?
    if (me->IsWithinDistInMap(who, 25.0f) && me->IsWithinLOSInMap(who, VMAP::ModelIgnoreFlags::M2))
    {
        //already fighting someone?
        if (!me->GetVictim())
        {
            EnterCombat(who);
            AttackStart(who);
            return true;
        }
        else
        {
            me->GetThreatManager().AddThreat(who, 0.0f);
            return true;
        }
    }

    return false;
}
