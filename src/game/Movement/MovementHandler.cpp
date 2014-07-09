/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "Corpse.h"
#include "Player.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Transport.h"
#include "Battleground.h"
#include "WaypointMovementGenerator.h"
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"
#include "World.h"
#include "IRCMgr.h"

#define MOVEMENT_PACKET_TIME_DELAY 0

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket & /*recvData*/)
{
    TC_LOG_DEBUG("network", "WORLD: got MSG_MOVE_WORLDPORT_ACK.");
    HandleMoveWorldportAckOpcode();
}

void WorldSession::HandleMoveWorldportAckOpcode()
{
    // ignore unexpected far teleports
    if (!GetPlayer()->IsBeingTeleportedFar())
        return;

    GetPlayer()->SetSemaphoreTeleportFar(false);

    // get the teleport destination
    WorldLocation &loc = GetPlayer()->GetTeleportDest();

    // possible errors in the coordinate validity check
    if(!MapManager::IsValidMapCoord(loc.m_mapId,loc.m_positionX,loc.m_positionY,loc.m_positionZ,loc.m_orientation))
    {
        LogoutPlayer(false);
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStore.LookupEntry(loc.m_mapId);
    InstanceTemplate const* mInstance = objmgr.GetInstanceTemplate(loc.m_mapId);

    // reset instance validity, except if going to an instance inside an instance
    if(GetPlayer()->m_InstanceValid == false && !mInstance)
        GetPlayer()->m_InstanceValid = true;

    // relocate the player to the teleport destination
    GetPlayer()->SetMapId(loc.m_mapId);
    GetPlayer()->Relocate(loc.m_positionX, loc.m_positionY, loc.m_positionZ, loc.m_orientation);

    // since the MapId is set before the GetInstance call, the InstanceId must be set to 0
    // to let GetInstance() determine the proper InstanceId based on the player's binds
    GetPlayer()->SetInstanceId(0);

    // check this before Map::Add(player), because that will create the instance save!
    bool reset_notify = (GetPlayer()->GetBoundInstance(GetPlayer()->GetMapId(), GetPlayer()->GetDifficulty()) == NULL);

     if(!GetPlayer()->GetMap())
    {
        sLog.outError("WorldSession::HandleMoveWorldportAckOpcode : couldn't get map, kicking player");
        KickPlayer();
        return;
    }
    GetPlayer()->SendInitialPacketsBeforeAddToMap();
    // the CanEnter checks are done in TeleporTo but conditions may change
    // while the player is in transit, for example the map may get full
    if(!GetPlayer()->GetMap()->Add(GetPlayer()))
    {
        // teleport the player home
        GetPlayer()->SetDontMove(false);
        if(!GetPlayer()->TeleportTo(GetPlayer()->m_homebindMapId, GetPlayer()->m_homebindX, GetPlayer()->m_homebindY, GetPlayer()->m_homebindZ, GetPlayer()->GetOrientation()))
        {
            // the player must always be able to teleport home
            sLog.outError("WORLD: failed to teleport player %s (%d) to homebind location %d,%f,%f,%f,%f!", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow(), GetPlayer()->m_homebindMapId, GetPlayer()->m_homebindX, GetPlayer()->m_homebindY, GetPlayer()->m_homebindZ, GetPlayer()->GetOrientation());
            assert(false);
        }
        return;
    }

    //this will set player's team ... so IT MUST BE CALLED BEFORE SendInitialPacketsAfterAddToMap()
    // battleground state prepare (in case join to BG), at relogin/tele player not invited
    // only add to bg group and object, if the player was invited (else he entered through command)
    if(_player->InBattleground())
    {
        // cleanup seting if outdated
        if(!mEntry->IsBattlegroundOrArena())
        {
            // Do next only if found in battleground
            _player->SetBattlegroundId(0);                          // We're not in BG.
            // reset destination bg team
            _player->SetBGTeam(0);
        }
        // join to bg case
        else if(Battleground *bg = _player->GetBattleground())
        {
            if(_player->IsInvitedForBattlegroundInstance(_player->GetBattlegroundId()))
                bg->AddPlayer(_player);

            if (bg->isSpectator(_player->GetGUID()))
                bg->onAddSpectator(_player);
        }
    }

    GetPlayer()->SendInitialPacketsAfterAddToMap();

    // flight fast teleport case
    if(GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType()==FLIGHT_MOTION_TYPE)
    {
        if(!_player->InBattleground())
        {
            // short preparations to continue flight
            FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetPlayer()->GetMotionMaster()->top());
            flight->Initialize(GetPlayer());
            return;
        }

        // battleground state prepare, stop flight
        GetPlayer()->GetMotionMaster()->MovementExpired();
        GetPlayer()->CleanupAfterTaxiFlight();
    }

    // resurrect character at enter into instance where his corpse exist after add to map
    Corpse *corpse = GetPlayer()->GetCorpse();
    if (corpse && corpse->GetType() != CORPSE_BONES && corpse->GetMapId() == GetPlayer()->GetMapId())
    {
        if( mEntry->IsDungeon() )
        {
            GetPlayer()->ResurrectPlayer(0.5f,false);
            GetPlayer()->SpawnCorpseBones();
            GetPlayer()->SaveToDB();
        }
    }

    if(mEntry->IsRaid() && mInstance)
    {
        if(reset_notify)
        {
            uint32 timeleft = sInstanceSaveManager.GetResetTimeFor(GetPlayer()->GetMapId()) - time(NULL);
            GetPlayer()->SendInstanceResetWarning(GetPlayer()->GetMapId(), timeleft); // greeting at the entrance of the resort raid instance
        }
    }

    // mount allow check
    if(!mEntry->IsMountAllowed())
        _player->RemoveAurasByType(SPELL_AURA_MOUNTED);

    // honorless target
    if(GetPlayer()->pvpInfo.inHostileArea)
        GetPlayer()->CastSpell(GetPlayer(), 2479, true);

    // resummon pet
    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();

    GetPlayer()->SetDontMove(false);
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recvData)
{
    _player->SetSemaphoreTeleportNear(false);
    
    TC_LOG_DEBUG("network", "MSG_MOVE_TELEPORT_ACK");
    uint64 guid;
    recvData >> guid;
    uint32 flags, time;
    recvData >> flags >> time;
    TC_LOG_DEBUG("network", "Guid " UI64FMTD, guid);
    TC_LOG_DEBUG("network", "Flags %u, time %u", flags, time/IN_MILLISECONDS);

    Player* plMover = _player->m_mover->ToPlayer();
    if (guid != plMover->GetGUID())
        return;

    if (!plMover || !plMover->IsBeingTeleportedNear())
        return;

    plMover->SetSemaphoreTeleportNear(false);

    // resummon pet
    GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();
}

/*
MSG_MOVE_START_FORWARD MSG_MOVE_START_BACKWARD MSG_MOVE_STOP MSG_MOVE_START_STRAFE_LEFT MSG_MOVE_START_STRAFE_RIGHT
MSG_MOVE_STOP_STRAFE MSG_MOVE_JUMP MSG_MOVE_START_TURN_LEFT MSG_MOVE_START_TURN_RIGHT MSG_MOVE_STOP_TURN 
MSG_MOVE_START_PITCH_UP MSG_MOVE_START_PITCH_DOWN MSG_MOVE_STOP_PITCH MSG_MOVE_SET_RUN_MODE MSG_MOVE_SET_WALK_MODE
MSG_MOVE_FALL_LAND MSG_MOVE_START_SWIM MSG_MOVE_STOP_SWIM MSG_MOVE_SET_FACING MSG_MOVE_SET_PITCH MSG_MOVE_HEARTBEAT
CMSG_MOVE_FALL_RESET CMSG_MOVE_SET_FLY MSG_MOVE_START_ASCEND MSG_MOVE_STOP_ASCEND CMSG_MOVE_CHNG_TRANSPORT
MSG_MOVE_START_DESCEND 
*/
void WorldSession::HandleMovementOpcodes(WorldPacket& recvData)
{
    uint16 opcode = recvData.GetOpcode();

    Unit* mover = _player->m_mover;

    ASSERT(mover != NULL);                      // there must always be a mover

    Player* plrMover = mover->ToPlayer(); // The unit we're currently moving

    // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
    if (plrMover && plrMover->IsBeingTeleported())
    {
        recvData.rfinish();                     // prevent warnings spam
        return;
    } 

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    recvData.rfinish();                         // prevent warnings spam

    if (!movementInfo.pos.IsPositionValid())
        return;

    /* handle special cases */
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
    {
        // transports size limited
        // (also received at zeppelin leave by some reason with t_* as absolute in continent coordinates, can be safely skipped)
        if (movementInfo.transport.pos.GetPositionX() > 50 || movementInfo.transport.pos.GetPositionY() > 50 || movementInfo.transport.pos.GetPositionZ() > 50)
        {
            recvData.rfinish();                 // prevent warnings spam
            return;
        }

        if (!Trinity::IsValidMapCoord(movementInfo.pos.GetPositionX() + movementInfo.transport.pos.GetPositionX(), movementInfo.pos.GetPositionY() + movementInfo.transport.pos.GetPositionY(),
            movementInfo.pos.GetPositionZ() + movementInfo.transport.pos.GetPositionZ(), movementInfo.pos.GetOrientation() + movementInfo.transport.pos.GetOrientation()))
        {
            recvData.rfinish();                 // prevent warnings spam
            return;
        }

        // if we boarded a transport, add us to it
        if (plrMover)
        {
            if (!plrMover->GetTransport())
            {
                if (Transport* transport = plrMover->GetMap()->GetTransport(movementInfo.transport.guid))
                    transport->AddPassenger(plrMover);
            }
            else if (plrMover->GetTransport()->GetGUID() != movementInfo.transport.guid)
            {
                plrMover->GetTransport()->RemovePassenger(plrMover);
                if (Transport* transport = plrMover->GetMap()->GetTransport(movementInfo.transport.guid))
                {
                    transport->AddPassenger(plrMover);
                    plrMover->m_anti_transportGUID = transport->GetGUID();
                } else
                    movementInfo.transport.Reset();
            }
        }

        if (!mover->GetTransport())
        {
            GameObject* go = mover->GetMap()->GetGameObject(movementInfo.transport.guid);
            if (!go || go->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
                movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        }
    }
    else if (plrMover && plrMover->GetTransport())                // if we were on a transport, leave
    {
        plrMover->m_anti_transportGUID = 0;
        plrMover->GetTransport()->RemovePassenger(plrMover);
        movementInfo.transport.Reset();
    }

    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (opcode == MSG_MOVE_FALL_LAND && plrMover && !plrMover->IsInFlight())
        plrMover->HandleFall(movementInfo);

    if (plrMover && ((movementInfo.flags & MOVEMENTFLAG_SWIMMING) != 0) != plrMover->IsInWater())
    {
        // now client not include swimming flag in case jumping under water
        plrMover->SetInWater(!plrMover->IsInWater() || plrMover->GetBaseMap()->IsUnderWater(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY(), movementInfo.pos.GetPositionZ()));
    }

    // ---- anti-cheat features -->>>
    if(plrMover && _player->GetGUID() == plrMover->GetGUID()) //disabled for charmed
    {
        if (opcode == MSG_MOVE_FALL_LAND && plrMover && !plrMover->IsInFlight())
        {
            //alternate falltime calculation
            if (plrMover->m_anti_beginfalltime != 0) 
            {
                uint32 ServerFallTime = getMSTime() - plrMover->m_anti_beginfalltime;
                if (movementInfo.fallTime < ServerFallTime && (time(NULL) - plrMover->m_anti_TeleTime) > 15) {
                    movementInfo.fallTime = ServerFallTime;
                }
                plrMover->m_anti_beginfalltime = 0;
            }

            if ((plrMover->m_anti_transportGUID == 0) && sWorld.GetMvAnticheatEnable() &&
                plrMover->GetSession()->GetSecurity() <= sWorld.GetMvAnticheatGmLevel() &&
                plrMover->GetSession()->GetGroupId() == 0 && //ignore gm in groups
                plrMover->GetMotionMaster()->GetCurrentMovementGeneratorType()!=FLIGHT_MOTION_TYPE &&
                (time(NULL) - plrMover->m_anti_TeleTime) > 15)
            {
                const uint32 CurTime=getMSTime();
                if(GetMSTimeDiff(plrMover->m_anti_lastalarmtime,CurTime) > sWorld.GetMvAnticheatAlarmPeriod())
                {
                    plrMover->m_anti_alarmcount = 0;
                }
                /* I really don't care about movement-type yet (todo)
                UnitMoveType move_type;

                if (MovementFlags & MOVEMENTFLAG_FLYING) move_type = MOVE_FLY;
                else if (MovementFlags & MOVEMENTFLAG_SWIMMING) move_type = MOVE_SWIM;
                else if (MovementFlags & MOVEMENTFLAG_WALK_MODE) move_type = MOVE_WALK;
                else move_type = MOVE_RUN;*/

                float delta_x = plrMover->GetPositionX() - movementInfo.pos.GetPositionX();
                float delta_y = plrMover->GetPositionY() - movementInfo.pos.GetPositionY();
                float delta_z = plrMover->GetPositionZ() - movementInfo.pos.GetPositionZ();
                float delta = sqrt(delta_x * delta_x + delta_y * delta_y); // Len of movement-vector via Pythagoras (a^2+b^2=Len)
                float tg_z = 0.0f; //tangens
                float delta_t = GetMSTimeDiff(GetPlayer()->m_anti_lastmovetime,CurTime);
                plrMover->m_anti_lastmovetime = CurTime;
                plrMover->m_anti_MovedLen += delta;

                if (delta_t > 15000.0f)
                    delta_t = 15000.0f;

                //antiOFF fall-damage, MOVEMENTFLAG_UNK4 seted by client if player try movement when falling and unset in this case the MOVEMENTFLAG_FALLING flag.
                if ((plrMover->m_anti_beginfalltime == 0) &&
                        (movementInfo.GetMovementFlags() & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR)) != 0) {
                    plrMover->m_anti_beginfalltime = CurTime;
                } else if (plrMover->m_anti_beginfalltime != 0 &&
                        ((movementInfo.GetMovementFlags() & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR)) == 0) &&
                        (movementInfo.GetMovementFlags() & MOVEMENTFLAG_SWIMMING) != 0) {
                    plrMover->m_anti_beginfalltime = 0;
                }

                if (plrMover->m_anti_NextLenCheck <= CurTime) {
                    // Check every 500ms is a lot more advisable then 1000ms, because normal movment packet arrives every 500ms
                    uint32 FH__Tmp1 = plrMover->m_anti_NextLenCheck;
                    float delta_xyt = plrMover->m_anti_MovedLen / (CurTime - (plrMover->m_anti_NextLenCheck - 500));
                    plrMover->m_anti_NextLenCheck = CurTime + 500;
                    plrMover->m_anti_MovedLen = 0.0f;
                    if (delta_xyt > 0.04f && delta <= 80.0f) {
                        Anti__CheatOccurred(CurTime, "Speed hack", delta_xyt, LookupOpcodeName(opcode),
                                (float) (plrMover->GetMotionMaster()->GetCurrentMovementGeneratorType()),
                                (float) (CurTime - (FH__Tmp1 - 500)), &movementInfo);
                    }
                }
        
                if (delta > 80.0f) {
                    Anti__ReportCheat("Tele hack", delta, LookupOpcodeName(opcode));
                }

                // Check for waterwalking
                if (((movementInfo.GetMovementFlags() & MOVEMENTFLAG_WATERWALKING) != 0) &&
                        ((movementInfo.GetMovementFlags() ^ MOVEMENTFLAG_WATERWALKING) != 0) && // Client sometimes set waterwalk where it shouldn't do that...
                        ((movementInfo.GetMovementFlags() & MOVEMENTFLAG_FALLING) == 0) &&
                        plrMover->GetBaseMap()->IsUnderWater(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY(), movementInfo.pos.GetPositionZ() - 6.0f) &&
                        !(plrMover->HasAuraType(SPELL_AURA_WATER_WALK) || plrMover->HasAuraType(SPELL_AURA_GHOST))) {
                    Anti__CheatOccurred(CurTime, "Water walking", 0.0f, NULL, 0.0f, (uint32)(movementInfo.GetMovementFlags()));
                }

                float Anti__GroundZ = plrMover->GetMap()->GetHeight(plrMover->GetPositionX(),plrMover->GetPositionY(),MAX_HEIGHT);
                float Anti__FloorZ = plrMover->GetMap()->GetHeight(plrMover->GetPositionX(),plrMover->GetPositionY(),plrMover->GetPositionZ());
                float Anti__MapZ = (Anti__FloorZ < -199900.0f) ? Anti__GroundZ : Anti__FloorZ;

                if(!plrMover->CanFly() &&
                    ((movementInfo.GetMovementFlags() & (MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING /*| MOVEMENTFLAG_FLYING2 | MOVEMENTFLAG_FLY_UP*/)) != 0) &&
                    !plrMover->GetBaseMap()->IsUnderWater(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY(), movementInfo.pos.GetPositionZ()-5.0f) &&
                    (Anti__MapZ==INVALID_HEIGHT || Anti__MapZ+5.0f < plrMover->GetPositionZ()) && Anti__MapZ >= -199900.0f)
                {
                    Anti__CheatOccurred(CurTime,"Fly hack",
                        ((uint8)(plrMover->HasAuraType(SPELL_AURA_FLY))) +
                        ((uint8)(plrMover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED))*2),
                        NULL,plrMover->GetPositionZ()-(Anti__MapZ+5.0f));
                }
            }
        }
    }
    // <<---- anti-cheat features

    uint32 mstime = getMSTime();
    /*----------------------*/
    if (m_clientTimeDelay == 0)
        m_clientTimeDelay = mstime - movementInfo.time;

    /* process position-change */
    WorldPacket data(opcode, recvData.size());
    movementInfo.time = movementInfo.time + m_clientTimeDelay + MOVEMENT_PACKET_TIME_DELAY;

    data.appendPackGUID(mover->GetGUID());
    WriteMovementInfo(&data, &movementInfo);
    mover->SendMessageToSet(&data, _player);
    mover->m_movementInfo = movementInfo;
    
    mover->UpdatePosition(movementInfo.pos);

    if (!mover->IsStandState() && (movementInfo.flags & (MOVEMENTFLAG_MASK_MOVING | MOVEMENTFLAG_MASK_TURNING)))
        mover->SetStandState(UNIT_STAND_STATE_STAND);

    if (plrMover)                                            // nothing is charmed, or player charmed
    {
        plrMover->UpdateFallInformationIfNeed(movementInfo, opcode);

        if (movementInfo.pos.GetPositionZ() < -500.0f)
        {
            if (!(plrMover->GetBattleground() && plrMover->GetBattleground()->HandlePlayerUnderMap(_player)))
            {
                // NOTE: this is actually called many times while falling
                // even after the player has been teleported away
                /// @todo discard movement packets after the player is rooted
                if (plrMover->IsAlive())
                {
                    plrMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, GetPlayer()->GetMaxHealth());
                    // player can be alive if GM/etc
                    // change the death state to CORPSE to prevent the death timer from
                    // starting in the next player update
                    if (!plrMover->IsAlive())
                        plrMover->KillPlayer();
                }
            }
        }
    }
}

/*
CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK
CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK
CMSG_FORCE_RUN_SPEED_CHANGE_ACK
CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK
CMSG_FORCE_SWIM_SPEED_CHANGE_ACK
CMSG_FORCE_WALK_SPEED_CHANGE_ACK
CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK
CMSG_FORCE_TURN_RATE_CHANGE_ACK
*/
void WorldSession::HandleForceSpeedChangeAck(WorldPacket &recvData)
{
    uint32 opcode = recvData.GetOpcode();
    TC_LOG_DEBUG("network", "WORLD: Recvd %s (%u, 0x%X) opcode", LookupOpcodeName(opcode), opcode, opcode);

    /* extract packet */
    uint64 guid;
    uint32 unk1;
    float  newspeed;

    recvData >> guid;

    // now can skip not our packet
    if (_player->GetGUID() != guid)
    {
        recvData.rfinish();                   // prevent warnings spam
        return;
    }

    // continue parse packet

    recvData >> unk1;                                      // counter or moveEvent

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    recvData >> newspeed;
    /*----------------*/

    // client ACK send one packet for mounted/run case and need skip all except last from its
    // in other cases anti-cheat check can be fail in false case
    UnitMoveType move_type;
    UnitMoveType force_move_type;

    static char const* move_type_name[MAX_MOVE_TYPE] = {  "Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate", "Flight", "FlightBack" };

    switch (opcode)
    {
        case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:          move_type = MOVE_WALK;          force_move_type = MOVE_WALK;        break;
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:           move_type = MOVE_RUN;           force_move_type = MOVE_RUN;         break;
        case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:      move_type = MOVE_RUN_BACK;      force_move_type = MOVE_RUN_BACK;    break;
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:          move_type = MOVE_SWIM;          force_move_type = MOVE_SWIM;        break;
        case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:     move_type = MOVE_SWIM_BACK;     force_move_type = MOVE_SWIM_BACK;   break;
        case CMSG_FORCE_TURN_RATE_CHANGE_ACK:           move_type = MOVE_TURN_RATE;     force_move_type = MOVE_TURN_RATE;   break;
        case CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK:        move_type = MOVE_FLIGHT;        force_move_type = MOVE_FLIGHT;      break;
        case CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:   move_type = MOVE_FLIGHT_BACK;   force_move_type = MOVE_FLIGHT_BACK; break;
        default:
            TC_LOG_ERROR("network", "WorldSession::HandleForceSpeedChangeAck: Unknown move type opcode: %u", opcode);
            return;
    }

    // skip all forced speed changes except last and unexpected
    // in run/mounted case used one ACK and it must be skipped.m_forced_speed_changes[MOVE_RUN} store both.
    if (_player->m_forced_speed_changes[force_move_type] > 0)
    {
        --_player->m_forced_speed_changes[force_move_type];
        if (_player->m_forced_speed_changes[force_move_type] > 0)
            return;
    }

    if (!_player->GetTransport() && fabs(_player->GetSpeed(move_type) - newspeed) > 0.01f)
    {
        if (_player->GetSpeed(move_type) > newspeed)         // must be greater - just correct
        {
            TC_LOG_ERROR("network", "%sSpeedChange player %s is NOT correct (must be %f instead %f), force set to correct value",
                move_type_name[move_type], _player->GetName().c_str(), _player->GetSpeed(move_type), newspeed);
            _player->SetSpeed(move_type, _player->GetSpeedRate(move_type), true);
        }
        else                                                // must be lesser - cheating
        {
            TC_LOG_DEBUG("misc", "Player %s from account id %u kicked for incorrect speed (must be %f instead %f)",
                _player->GetName().c_str(), _player->GetSession()->GetAccountId(), _player->GetSpeed(move_type), newspeed);
            _player->GetSession()->KickPlayer();
        }
    }
}

// sent by client when gaining control of a unit
void WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_SET_ACTIVE_MOVER");

    uint64 guid;
    recvData >> guid; //Client started controlling this unit

    Unit* movedUnit = ObjectAccessor::GetUnit(*_player, guid);
    if(movedUnit)
        _player->SetMover(movedUnit);
    else
        sLog.outError("Player %u - WorldSession::HandleSetActiveMoverOpcode could not find player with guid %u",_player->GetGUID(),guid);
}

//CMSG_MOVE_NOT_ACTIVE_MOVER
//sent by client when loosing control of a unit
void WorldSession::HandleMoveNotActiveMover(WorldPacket &recvData)
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_MOVE_NOT_ACTIVE_MOVER");

    uint64 old_mover_guid;
    recvData >> old_mover_guid;

    MovementInfo mi;
    ReadMovementInfo(recvData, &mi);

    //mi.guid = old_mover_guid;

    _player->m_movementInfo = mi;
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket& /*recvData*/)
{
    WorldPacket data(SMSG_MOUNTSPECIAL_ANIM, 8);
    data << uint64(GetPlayer()->GetGUID());

    GetPlayer()->SendMessageToSet(&data, false);
}

// CMSG_MOVE_KNOCK_BACK_ACK
//Le format a l'air correct mais le client renvoie sa position au milieu du bump ce qui ruine l'anim
void WorldSession::HandleMoveKnockBackAck(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_MOVE_KNOCK_BACK_ACK");

    uint64 guid;
    recvData >> guid;

    if (_player->m_mover->GetGUID() != guid)
        return;

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    _player->m_movementInfo = movementInfo;

    WorldPacket data(MSG_MOVE_KNOCK_BACK, 73);
    recvData << guid;
    _player->BuildMovementPacket(&data);

    // knockback specific info
    data << movementInfo.jump.sinAngle;
    data << movementInfo.jump.cosAngle;
    data << movementInfo.jump.xyspeed;
    data << movementInfo.jump.zspeed;

    _player->SendMessageToSet(&data, false);
}

//CMSG_MOVE_HOVER_ACK
//BC structure confirmed
void WorldSession::HandleMoveHoverAck(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_MOVE_HOVER_ACK");

    uint64 guid;                                            // guid - unused
    recvData >> guid;

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    recvData.read_skip<uint32>();                          // unk2
}

//CMSG_MOVE_WATER_WALK_ACK
//BC structure confirmed
void WorldSession::HandleMoveWaterWalkAck(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network", "CMSG_MOVE_WATER_WALK_ACK");

    uint64 guid;                                            // guid - unused
    recvData >> guid;

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);

    recvData.read_skip<uint32>();                          // unk2
}

void WorldSession::HandleSummonResponseOpcode(WorldPacket& recvData)
{
    if (!_player->IsAlive() || _player->IsInCombat())
        return;

    uint64 summoner_guid;
    bool agree;
    recvData >> summoner_guid;
    recvData >> agree;

    _player->SummonIfPossible(agree);
}

bool WorldSession::Anti__CheatOccurred(uint32 CurTime,const char* Reason,float Speed,const char* Op,
                                       float Val1,uint32 Val2,MovementInfo* MvInfo)
{
    if(!Reason)
    {
        sLog.outError("Anti__CheatOccurred: Missing Ply or Reason paremeter!");
        return false;
    }

    GetPlayer()->m_anti_lastalarmtime = CurTime;
    GetPlayer()->m_anti_alarmcount = GetPlayer()->m_anti_alarmcount + 1;

    if (GetPlayer()->m_anti_alarmcount > sWorld.GetMvAnticheatAlarmCount())
    {
        Anti__ReportCheat(Reason,Speed,Op,Val1,Val2,MvInfo);
        return true;
    }
    return false;
}

bool WorldSession::Anti__ReportCheat(const char* Reason,float Speed,const char* Op,float Val1,uint32 Val2,MovementInfo* MvInfo)
{
    if(!Reason)
    {
        sLog.outError("Anti__ReportCheat: Missing Player or Reason paremeter!");
        return false;
    }
    const char* player=GetPlayer()->GetName();
    uint32 Acc=GetPlayer()->GetSession()->GetAccountId();
    uint32 Map=GetPlayer()->GetMapId();
    if(!player)
    {
        sLog.outError("Anti__ReportCheat: Player with no name?!?");
        return false;
    }

    if(sWorld.GetMvAnticheatWarn())
    {
        if(lastCheatWarn + 120 < time(NULL)) //2m cooldown
        {
            lastCheatWarn = time(NULL);
            std::stringstream msg;
            msg << "Nouvelle entree anticheat pour le joueur " << player << " (guid : " << GetPlayer()->GetGUIDLow() << ").";

            ChatHandler(GetPlayer()).SendGlobalGMSysMessage(msg.str().c_str());
        }
    }

    QueryResult *Res=CharacterDatabase.PQuery("SELECT speed,Val1,Val2 FROM cheaters WHERE player='%s' AND reason LIKE '%s' AND Map='%u' AND last_date >= NOW()-300",player,Reason,Map);
    if(Res)
    {
        Field* Fields = Res->Fetch();

        std::stringstream Query;
        Query << "UPDATE cheaters SET count=count+1,last_date=NOW()";
        Query.precision(5);
        if(Speed>0.0f && Speed > Fields[0].GetFloat())
        {
            Query << ",speed='";
            Query << std::fixed << Speed;
            Query << "'";
        }

        if(Val1>0.0f && Val1 > Fields[1].GetFloat())
        {
            Query << ",Val1='";
            Query << std::fixed << Val1;
            Query << "'";
        }

        Query << " WHERE player='" << player << "' AND reason='" << Reason << "' AND Map='" << Map << "' AND last_date >= NOW()-300 ORDER BY entry LIMIT 1";

        CharacterDatabase.Execute(Query.str().c_str());
        delete Res;
    }
    else
    {
        if(!Op)
        { Op=""; }
        std::stringstream Pos;
        Pos << "OldPos: " << GetPlayer()->GetPositionX() << " " << GetPlayer()->GetPositionY() << " "
            << GetPlayer()->GetPositionZ();
        if(MvInfo)
        {
            Pos << "\nNew: " << MvInfo->pos.GetPositionX() << " " << MvInfo->pos.GetPositionY() << " " << MvInfo->pos.GetPositionZ() << "\n"
                << "t_guid: " << MvInfo->transport.guid << " falltime: " << MvInfo->fallTime;
        }
        CharacterDatabase.PExecute("INSERT INTO cheaters (player,acctid,reason,speed,count,first_date,last_date,`Op`,Val1,Val2,Map,Pos,Level) "
            "VALUES ('%s','%u','%s','%f','1',NOW(),NOW(),'%s','%f','%u','%u','%s','%u')",
            player,Acc,Reason,Speed,Op,Val1,Val2,Map,
            Pos.str().c_str(),GetPlayer()->GetLevel());
    }

    if(sWorld.GetMvAnticheatKill() && GetPlayer()->IsAlive())
    {
        GetPlayer()->DealDamage(GetPlayer(), GetPlayer()->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
    }
    if(sWorld.GetMvAnticheatKick())
    {
        GetPlayer()->GetSession()->KickPlayer();
    }
    if(sWorld.GetMvAnticheatBan() & 1)
    {
        sWorld.BanAccount(BAN_CHARACTER,player,sWorld.GetMvAnticheatBanTime(),"Cheat","Anticheat");
    }
    if(sWorld.GetMvAnticheatBan() & 2)
    {
        QueryResult *result = LoginDatabase.PQuery("SELECT last_ip FROM account WHERE id=%u", Acc);
        if(result)
        {

            Field *fields = result->Fetch();
            std::string LastIP = fields[0].GetCppString();
            if(!LastIP.empty())
            {
                sWorld.BanAccount(BAN_IP,LastIP,sWorld.GetMvAnticheatBanTime(),"Cheat","Anticheat");
            }
            delete result;
        }
    }
    return true;
}