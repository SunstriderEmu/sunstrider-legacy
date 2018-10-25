
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
#include "BattleGround.h"
#include "FlightPathMovementGenerator.h"
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Pet.h"
#include "Chat.h"
#include "PlayerAntiCheat.h"
#include "GameTime.h"
#include "Map.h"
#include "MovementPacketSender.h"
#include "RBAC.h"
#include "Language.h"

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket & /*recvData*/)
{
   // TC_LOG_DEBUG("network", "WORLD: got MSG_MOVE_WORLDPORT_ACK.");
    HandleMoveWorldportAck();
}

void WorldSession::HandleMoveWorldportAck()
{
    Player* player = GetPlayer();
    // ignore unexpected far teleports
    if (!player->IsBeingTeleportedFar())
        return;

    player->SetSemaphoreTeleportFar(false);

    // get the teleport destination
    WorldLocation &loc = player->GetTeleportDest();

    // possible errors in the coordinate validity check
    if (!MapManager::IsValidMapCoord(loc.m_mapId, loc.m_positionX, loc.m_positionY, loc.m_positionZ, loc.m_orientation))
    {
        LogoutPlayer(false);
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStore.LookupEntry(loc.m_mapId);
    InstanceTemplate const* mInstance = sObjectMgr->GetInstanceTemplate(loc.m_mapId);

    // reset instance validity, except if going to an instance inside an instance
    if(player->m_InstanceValid == false && !mInstance)
        player->m_InstanceValid = true;

    Map* oldMap = player->GetMap();
    Map* newMap = sMapMgr->CreateMap(loc.GetMapId(), GetPlayer());
    player->SetTeleportingToTest(0);

    if (player->IsInWorld())
    {
        TC_LOG_ERROR("network", "Player %s (%u) is still in world when teleported from map %s (%u) to new map %s (%u)", GetPlayer()->GetName().c_str(), player->GetGUID().GetCounter(), oldMap->GetMapName(), oldMap->GetId(), newMap ? newMap->GetMapName() : "Unknown", loc.GetMapId());
        oldMap->RemovePlayerFromMap(player, false);
    }

    // relocate the player to the teleport destination
    // the CannotEnter checks are done in TeleporTo but conditions may change
    // while the player is in transit, for example the map may get full
    if (!newMap || newMap->CannotEnter(player))
    {
        TC_LOG_ERROR("network", "Map %d (%s) could not be created for player %d (%s), porting player to homebind", loc.GetMapId(), newMap ? newMap->GetMapName() : "Unknown", player->GetGUID().GetCounter(), player->GetName().c_str());
        player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
        return;
    }

    float z = loc.GetPositionZ() + player->GetHoverOffset();
    player->Relocate(loc.m_positionX, loc.m_positionY, z, loc.m_orientation);
    player->SetFallInformation(0, player->GetPositionZ());

    player->ResetMap();
    player->SetMap(newMap);

    // check this before Map::AddPlayerToMap(player), because that will create the instance save!
    bool reset_notify = (player->GetBoundInstance(player->GetMapId(), player->GetDifficulty()) == NULL);

    player->SendInitialPacketsBeforeAddToMap();
    // the CanEnter checks are done in TeleportTo but conditions may change
    // while the player is in transit, for example the map may get full
    if(!player->GetMap()->AddPlayerToMap(player))
    {
        TC_LOG_ERROR("network", "WORLD: failed to teleport player %s (%d) to map %d (%s) because of unknown reason!",
            player->GetName().c_str(), player->GetGUID().GetCounter(), loc.GetMapId(), newMap ? newMap->GetMapName() : "Unknown");
        player->ResetMap();
        player->SetMap(oldMap);

        // teleport the player home
        if(!player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation()))
        {
            // the player must always be able to teleport home
            TC_LOG_ERROR("network","WORLD: failed to teleport player %s (%d) to homebind location %d,%f,%f,%f,%f!", player->GetName().c_str(), player->GetGUID().GetCounter(), player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
            DEBUG_ASSERT(false);
        }
        return;
    }

    //this will set player's team ... so IT MUST BE CALLED BEFORE SendInitialPacketsAfterAddToMap()
    // battleground state prepare (in case join to BG), at relogin/tele player not invited
    // only add to bg group and object, if the player was invited (else he entered through command)
    if(player->InBattleground())
    {
        // cleanup seting if outdated
        if(!mEntry->IsBattlegroundOrArena())
        {
            // Do next only if found in battleground
            player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE);  // We're not in BG.
            // reset destination bg team
            player->SetBGTeam(0);
        }
        // join to bg case
        else if(Battleground *bg = player->GetBattleground())
        {
            if(player->IsInvitedForBattlegroundInstance(player->GetBattlegroundId()))
                bg->AddPlayer(player);

            if (bg->isSpectator(player->GetGUID()))
                bg->onAddSpectator(player);
        }
    }

    player->SendInitialPacketsAfterAddToMap();

    // flight fast teleport case
    if (player->IsInFlight())
    {
        if(!player->InBattleground())
        {
            // short preparations to continue flight
            FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(player->GetMotionMaster()->GetCurrentMovementGenerator());
            flight->Initialize(player);
            return;
        }

        // battleground state prepare, stop flight
        player->FinishTaxiFlight();
    }

    if (!player->IsAlive() && player->GetTeleportOptions() & TELE_REVIVE_AT_TELEPORT)
        player->ResurrectPlayer(0.5f);

    // resurrect character at enter into instance where his corpse exist after add to map
    if (mEntry->IsDungeon() && !player->IsAlive())
    {
        if (player->GetCorpseLocation().GetMapId() == mEntry->MapID)
        {
            player->ResurrectPlayer(0.5f);
            player->SpawnCorpseBones();
            player->SaveToDB();
        }
    }

    if(mInstance)
    {
        if(reset_notify && mEntry->IsRaid())
        {
#ifdef LICH_KING
            FIXME; //LK has this message for dungeon as well
#else
            uint32 timeleft = sInstanceSaveMgr->GetResetTimeFor(player->GetMapId(), RAID_DIFFICULTY_NORMAL) - time(NULL);
            player->SendInstanceResetWarning(player->GetMapId(), timeleft); // greeting at the entrance of the resort raid instance
#endif
        }

        // check if instance is valid
        if (!player->CheckInstanceValidity(false))
            player->m_InstanceValid = false;
    }

    // mount allow check
    if(!mEntry->IsMountAllowed())
        player->RemoveAurasByType(SPELL_AURA_MOUNTED);

    // update zone immediately, otherwise leave channel will cause crash in mtmap
    uint32 newzone, newarea;
    player->GetZoneAndAreaId(newzone, newarea);
    player->UpdateZone(newzone, newarea);

    // honorless target
    if(player->pvpInfo.IsHostile)
        player->CastSpell(player, 2479, true);
    // in friendly area
    else if (player->IsPvP() && !player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
        player->UpdatePvP(false, false);

    // resummon pet
    player->ResummonPetTemporaryUnSummonedIfAny();

    //lets process all delayed operations on successful teleport
    player->ProcessDelayedOperations();
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recvData)
{
    //TC_LOG_DEBUG("network", "MSG_MOVE_TELEPORT_ACK");
    /* extract packet */
    ObjectGuid guid;
#ifdef LICH_KING
    recvData >> guid.ReadAsPacked();
#else
    recvData >> guid;
#endif

    uint32 movementCounter, time;
    recvData >> movementCounter >> time;

    // verify that indeed the client is replying with the changes that were send to him
    if (!HasPendingMovementChange(movementCounter, recvData.GetOpcode(), guid))
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleMoveTeleportAck: Player %s from account id %u kicked because no movement change ack was expected from this player",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }
    
    if (!IsAuthorizedToMove(guid, true))
        return;

    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);
    if (!mover)
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleMoveTeleportAck: Player %s (account %u) tried to teleport unit not found %s",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId(), guid.ToString().c_str());
        return;
    }

    PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
    MovementInfo info = mover->GetMovementInfo();
    pendingChange.Resolve(pendingChange, this, mover, &info); //GetMovementInfo wont be used here
}

/*
These packets are sent by the client in order to transmit the movement of the character currently controlled by the client.
This character is usually the player but it can be a creature (in case of a possess (eg priest MC)) or a vehicle. Later in this handler explaination,
'player' will be used when 'player controlled unit' should have been used.
The server then retransmits these packets to the other clients, which will extrapolate the unit's motion.
All the server has to do with all these packets is:
1) validate & update the data received: position, orientation, fall data and movement flags (this list should be exhaustive. please update if there is something missing).
2) transmit this packet to the other players nearby
Handles:
MSG_MOVE_START_FORWARD
MSG_MOVE_START_BACKWARD
MSG_MOVE_STOP
MSG_MOVE_START_STRAFE_LEFT
MSG_MOVE_START_STRAFE_RIGHT
MSG_MOVE_STOP_STRAFE
MSG_MOVE_JUMP
MSG_MOVE_START_TURN_LEFT
MSG_MOVE_START_TURN_RIGHT
MSG_MOVE_STOP_TURN
MSG_MOVE_START_PITCH_UP
MSG_MOVE_START_PITCH_DOWN
MSG_MOVE_STOP_PITCH
MSG_MOVE_SET_RUN_MODE
MSG_MOVE_SET_WALK_MODE
MSG_MOVE_FALL_LAND
MSG_MOVE_START_SWIM
MSG_MOVE_STOP_SWIM
MSG_MOVE_SET_FACING
MSG_MOVE_SET_PITCH
MSG_MOVE_HEARTBEAT -- packet sent every 0.5 s when the player is moving.
MSG_MOVE_START_ASCEND
MSG_MOVE_STOP_ASCEND
MSG_MOVE_START_DESCEND
CMSG_MOVE_FALL_RESET -- the player has encounter an object while failing, thus modifing the trajectory of his fall. this packet gives info regarding the new trajectory. !!!! @todo: needs to be processed in a different handler and this opcode shouldn'nt be sent to other clients !!!!
CMSG_MOVE_SET_FLY -- the player has started or stopped to fly (toggle effect). !!!! @todo: needs to be processed in a different handler and this opcode shouldn'nt be sent to other clients !!!!
CMSG_MOVE_CHNG_TRANSPORT !!!! @todo: needs to be processed in a different handler and this opcode shouldn'nt be sent to other clients !!!!
*/
void WorldSession::HandleMovementOpcodes(WorldPacket& recvData)
{
    uint16 opcode = recvData.GetOpcode();
    /* extract packet */
    MovementInfo movementInfo;
#ifdef LICH_KING
    movementInfo.FillContentFromPacket(&recvData, true);
    recvData.rfinish();                         // prevent warnings spam

    if (!IsAuthorizedToMove(guid, true))
        return;

    Unit* mover = ObjectAccessor::GetUnit(*_player, movementInfo.guid);
#else
    movementInfo.FillContentFromPacket(&recvData, false);
    recvData.rfinish();                         // prevent warnings spam
    if (!_activeMover)
    {
        TC_LOG_DEBUG("movement", "Player sent a move packet (%u) but is not currently moving any unit", recvData.GetOpcode());
        return;
    }
    if (!IsAuthorizedToMove(_activeMover->GetGUID(), true))
        return;

    ASSERT(_activeMover->m_playerMovingMe == this);

    Unit* mover = _activeMover;
#endif

    Player* plrMover = mover->ToPlayer();

    TC_LOG_TRACE("movement", "%s: Player %s, moving unit %s", GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), mover->GetName().c_str());

    // ignore movement packets if the player is getting far teleported (change of map). keep processing movement packets when the unit is only doing a near teleport.
    if (plrMover && plrMover->IsBeingTeleportedFar())
    {
        recvData.rfinish();                     // prevent warnings spam
        return;
    }


    if (!movementInfo.pos.IsPositionValid())
        return;

    /* validate new movement packet */
    mover->ValidateMovementInfo(&movementInfo);

    /* handle special cases */
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT)) // @todo LK: move this stuff. CMSG_MOVE_CHNG_TRANSPORT should be handled elsewhere than here.
    {
        // We were teleported, skip packets that were broadcast before teleport
        if (movementInfo.pos.GetExactDist2d(mover) > SIZE_OF_GRIDS)
        {
            recvData.rfinish();                 // prevent warnings spam
            return;
        }

#ifdef LICH_KING
        // T_POS ON VEHICLES!
        if (mover->GetVehicle())
            movementInfo.transport.pos = mover->m_movementInfo.transport.pos;
#endif

        // transports size limited
        // (also received at zeppelin leave by some reason with t_* as absolute in continent coordinates, can be safely skipped)
        if (fabs(movementInfo.transport.pos.GetPositionX()) > 75.0f || fabs(movementInfo.transport.pos.GetPositionY()) > 75.0f || fabs(movementInfo.transport.pos.GetPositionZ()) > 75.0f )
        {
            recvData.rfinish();                   // prevent warnings spam
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
                {
                    plrMover->m_transport = transport;
                    transport->AddPassenger(plrMover);
                }
            }
            else if (plrMover->GetTransport()->GetGUID() != movementInfo.transport.guid)
            {
                bool foundNewTransport = false;
                plrMover->m_transport->RemovePassenger(plrMover);
                if (Transport* transport = plrMover->GetMap()->GetTransport(movementInfo.transport.guid))
                {
                    foundNewTransport = true;
                    plrMover->m_transport = transport;
                    transport->AddPassenger(plrMover);
                }

                if (!foundNewTransport)
                {
                    plrMover->m_transport = NULL;
                    movementInfo.transport.Reset();
                }
            }
        }

        if (!mover->GetTransport()
#ifdef LICH_KING
            && !mover->GetVehicle()
#endif
            )
            movementInfo.flags &= ~MOVEMENTFLAG_ONTRANSPORT;
    }
    else if (plrMover && plrMover->GetTransport())                // if we were on a transport, leave
    {
        plrMover->m_transport->RemovePassenger(plrMover);
        plrMover->m_transport = nullptr;
        movementInfo.transport.Reset();
    }

    if (plrMover)
    {
        //sunstrider: Client also send SWIMMING while flying so we can't just update InWater when client stops sending it. A player swimming then flying upward will be still considered in water
        // To fix this: It seems the client does not set the PLAYER_FLYING flag while swimming. But I'm not 100% sure there is no case it could happen. If this is false and we should check for Map::IsUnderWater as well
        if (((movementInfo.flags & MOVEMENTFLAG_PLAYER_FLYING) != 0) && plrMover->IsInWater())
        {
            plrMover->SetInWater(false);
        }
        else if (((movementInfo.flags & MOVEMENTFLAG_SWIMMING) != 0) != plrMover->IsInWater())
        {
            // now client not include swimming flag in case jumping under water
            plrMover->SetInWater(!plrMover->IsInWater() || plrMover->GetBaseMap()->IsUnderWater(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY(), movementInfo.pos.GetPositionZ()));
        }
    }

    // sunwell: do not allow to move with UNIT_FLAG_REMOVE_CLIENT_CONTROL
    if (mover->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
    {
        // skip moving packets
        if (movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING))
            return;
    }

    anticheat->OnPlayerMoved(mover, movementInfo, OpcodeClient(opcode));

    /* process position-change */
    mover->UpdateMovementInfo(movementInfo);

    // as strange as it may be, retail servers actually use MSG_MOVE_START_SWIM_CHEAT & MSG_MOVE_STOP_SWIM_CHEAT to respectively set and unset the 'Flying' movement flag. 
    // The only thing left to do is to move the handling of CMSG_MOVE_SET_FLY into a different handler
    if (opcode == CMSG_MOVE_SET_FLY)
        opcode = movementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING) ? MSG_MOVE_START_SWIM_CHEAT : MSG_MOVE_STOP_SWIM_CHEAT;

    WorldPacket data(opcode, recvData.size());
    mover->GetMovementInfo().WriteContentIntoPacket(&data, true);  //this contains the server time, not the time provided by client
    mover->SendMessageToSet(&data, _player);

    if (!mover->IsStandState() && (movementInfo.flags & (MOVEMENTFLAG_MASK_MOVING | MOVEMENTFLAG_MASK_TURNING)))
        mover->SetStandState(UNIT_STAND_STATE_STAND);

    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (opcode == MSG_MOVE_FALL_LAND && plrMover && !plrMover->IsInFlight() && (!plrMover->GetTransport() || plrMover->GetTransport()->IsStaticTransport()))
        plrMover->HandleFall(movementInfo);

#ifdef LICH_KING
    //  interrupt parachutes upon falling or landing in water
    if (opcode == MSG_MOVE_FALL_LAND || opcode == MSG_MOVE_START_SWIM)
        mover->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_LANDING); // Parachutes
#endif

    if (plrMover && plrMover->GetMap()) // Nothing is charmed, or player charmed
    {
        plrMover->UpdateFallInformationIfNeed(movementInfo, opcode);

        // Used to handle spell interrupts on move (client does not always does it by itself)
        if (plrMover->isMoving())
            plrMover->SetHasMovedInUpdate(true);

        // Anti Undermap
        float minHeight = plrMover->GetMap()->GetMinHeight(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY());
        if (movementInfo.pos.GetPositionZ() < minHeight)
        {
            // Is there any ground existing above min height? If there is a ground at this position, we very probably fell undermap, try to recover player.
            // Else, just kill him
            float gridMapHeight = plrMover->GetMap()->GetGridMapHeight(movementInfo.pos.GetPositionX(), movementInfo.pos.GetPositionY());
            if(gridMapHeight > minHeight)
                plrMover->UndermapRecall(); // Port player back to last safe position
            else
            {
                plrMover->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_IS_OUT_OF_BOUNDS);
                if (plrMover->IsAlive()) // Still alive while falling
                {
                    plrMover->EnvironmentalDamage(DAMAGE_FALL_TO_VOID, plrMover->GetMaxHealth());
                    // player can be alive if GM/etc
                    // Change the death state to CORPSE to prevent the death timer from
                    // Starting in the next player update
                    if (plrMover->IsAlive())
                        plrMover->KillPlayer();
                }
            }
        }
        else if (plrMover->CanFreeMove() && !movementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING)) // If player is able to move and not falling or jumping..
        {
            plrMover->SaveSafePosition(movementInfo.pos); // Save current position for UndermapRecall()
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
   // TC_LOG_DEBUG("network", "WORLD: Recvd %s (%u, 0x%X) opcode", GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), opcode, opcode);

    ObjectGuid guid;

#ifdef LICH_KING
    recvData >> guid.ReadAsPacked();
#else
    recvData >> guid;
#endif

    if (!IsAuthorizedToMove(guid, true))
        return;

    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);
    if (!mover)
    {
        TC_LOG_ERROR("movement", "%s: Player %s failed to get unit %s", 
            GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), guid.ToString().c_str());
        return;
    }

    uint32 movementCounter;
    float  speedReceived;

    MovementInfo movementInfo;
    movementInfo.guid = guid;

    recvData >> movementCounter;
    movementInfo.FillContentFromPacket(&recvData, false);
    recvData >> speedReceived;

    // verify that indeed the client is replying with the changes that were send to him
    if (!HasPendingMovementChange(movementCounter, recvData.GetOpcode(), guid))
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleForceSpeedChangeAck: Player %s from account id %u kicked because no movement change ack was expected from this player",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();

        return;
    }

    PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
    float speedSent = pendingChange.newValue;
    
    if (std::fabs(speedSent - speedReceived) > 0.01f)
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleForceSpeedChangeAck: Player %s from account id %u kicked for incorrect data returned in an ack",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }

    TC_LOG_TRACE("movement", "%s: Player %s, moving unit %s",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), mover->GetName().c_str());

    /* the client data has been verified. let's do the actual change now */
    pendingChange.Resolve(pendingChange, this, mover, &movementInfo);
}

#ifdef LICH_KING
void WorldSession::HandleCollisionHeightChangeAck(WorldPacket &recvData)
{
    /* extract packet */
    ObjectGuid guid;
    uint32 movementCounter;
    MovementInfo movementInfo;
    float heightReceived;

    recvData >> guid;
    if (!IsAuthorizedToMove(guid, true))
        return;

    movementInfo.guid = guid;
    recvData >> movementCounter;
    movementInfo.FillContentFromPacket(&recvData, false);
    recvData >> heightReceived;

    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);

    // verify that indeed the client is replying with the changes that were send to him
    if (!HasPendingMovementChange(movementCounter, recvData.GetOpcode(), guid))
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleCollisionHeightChangeAck: Player %s from account id %u kicked because no movement change ack was expected from this player",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }

    PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
    float heightSent = pendingChange.newValue;

    if (std::fabs(heightSent - heightReceived) > 0.01f)
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleCollisionHeightChangeAck: Player %s from account id %u kicked for incorrect data returned in an ack",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }

    mover->ValidateMovementInfo(&movementInfo);
    /* the client data has been verified. let's do the actual change now */
    mover->UpdateMovementInfo(movementInfo);
    mover->SetCollisionHeightReal(heightSent);
    MovementPacketSender::SendHeightChangeToObservers(mover, heightSent);
}
#endif

Unit* WorldSession::GetAllowedActiveMover() const
{
    if (_activeMover && _allowedClientMove.find(_activeMover->GetGUID()) != _allowedClientMove.end())
        return _activeMover;
    else
        return nullptr;
}

void WorldSession::ResetActiveMover(bool onDelete /*= false*/)
{
    Unit* previousMover = _activeMover;
    if (_activeMover)
        if (_activeMover->m_playerMovingMe == this)
        {
            _activeMover->m_playerMovingMe = nullptr;
            _activeMover->RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING); // + send heartbeat?
        }

    _activeMover = nullptr;

    if (onDelete)
        return;
    
    //Only code not on deletion here
    anticheat->OnPlayerMoverChanged(previousMover, nullptr);
}

void WorldSession::InitActiveMover(Unit* activeMover)
{
    _activeMover = nullptr; //May have one from a previous session
    _allowedClientControl.insert(activeMover->GetGUID());
    //Client will send CMSG_SET_ACTIVE_MOVER when joining a map
}

void WorldSession::AllowMover(Unit* mover)
{
    _allowedClientMove.insert(mover->GetGUID());
    _pendingActiveMoverSplineId = 0;
    mover->m_playerMovingMe = this;
}

void WorldSession::DisallowMover(Unit* mover)
{
    _allowedClientMove.erase(mover->GetGUID());
    if (mover->m_playerMovingMe == this)
        mover->m_playerMovingMe = nullptr;
    ResolveAllPendingChanges();
}

//Here is the active mover sequence:
// - Server tell the unit of his new active mover with SMSG_CLIENT_CONTROL_UPDATE 
// - Client respond with CMSG_SET_ACTIVE_MOVER to enable this mover
// - We send a spline on current unit position to transfer correct movement info
// - Client respond CMSG_MOVE_SPLINE_DONE, transfer is complete
// The spline step is necessary because when mover changed, it's possible that the client controlling it has not yet received 
// the move flags of the mover that were set just before he took control. This is not possible when we keep the same mover since
// those changes must be acked by client. Also, same problem is present with speed changes.
void WorldSession::SetActiveMover(Unit* activeMover)
{
    //Resolve all pending changes for this unit before taking control
    if (WorldSession* session = activeMover->GetPlayerMovingMe())
        session->DisallowMover(activeMover);

    DisallowMover(activeMover);

    _activeMover = activeMover;

#ifdef TODOMOV
    _pendingActiveMoverSplineId = activeMover->StopMovingOnCurrentPos(); //Send spline movement before allowing move
    if (_pendingActiveMoverSplineId == 0)
    {
        TC_LOG_FATAL("movement", "WorldSession::SetActiveMover: player %s with pending mover %s, FAILED to start spline movement",
            _player->GetName().c_str(), _activeMover->GetGUID().ToString().c_str());

        //Unit will get stuck. Shouldn't ever happen in current impl.
        DEBUG_ASSERT(false);
    }

    TC_LOG_TRACE("movement", "Received CMSG_SET_ACTIVE_MOVER for player %s with pending mover %s (%s), now sending the spline movement (id %u)",
        _player->GetName().c_str(), _activeMover->GetName().c_str(), _activeMover->GetGUID().ToString().c_str(), _pendingActiveMoverSplineId);
#else
    if (IsAuthorizedToTakeControl(_activeMover->GetGUID()))
    {
        AllowMover(_activeMover);
        TC_LOG_TRACE("movement", "WorldSession::SetActiveMover: Enabling move of unit %s (%s) to player %s (%s)",
            _activeMover->GetName().c_str(), _activeMover->GetGUID().ToString().c_str(), _player->GetName().c_str(), _player->GetName().c_str());
    }
    else
    {
        TC_LOG_ERROR("movement", "WorldSession::SetActiveMover: Failed enabling move of unit %s (%s) to player %s (%s), pending spline id is correct but player is not allowed to take control anymore",
            _activeMover->GetName().c_str(), _activeMover->GetGUID().ToString().c_str(), _player->GetName().c_str(), _player->GetName().c_str());
    }
#endif
}

//CMSG_SET_ACTIVE_MOVER
// sent by client when gaining control of a unit
void WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recvData)
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_SET_ACTIVE_MOVER");

    //LK OK
    ObjectGuid guid;
    recvData >> guid; //Client started controlling this unit

    if (_activeMover && _activeMover->GetGUID() == guid)
        return; //we already control this unit, ignore

    if (!IsAuthorizedToTakeControl(guid))
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleSetActiveMoverOpcode: The client of player %s doesn't have the permission to control this unit (GUID: %s)!", _player->GetName().c_str(), guid.ToString().c_str());
        return;
    }

    _releaseMoverTimeout = 0;

    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);
    if (!mover)
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleSetActiveMoverOpcode: The client provided an invalid %s (unit may also have just been removed for world)", guid.ToString().c_str());
        return;
    }

    TC_LOG_TRACE("movement", "%s: Player %s, setting unit %s as active mover",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), mover->GetName().c_str());

#ifndef LICH_KING
    //On LK, client may have multiple active movers
    ResetActiveMover();
#endif
    SetActiveMover(mover);
 }

//CMSG_MOVE_NOT_ACTIVE_MOVER
//sent by client when loosing control of a unit.
//Client may send CMSG_SET_ACTIVE_MOVER to another unit instead
void WorldSession::HandleMoveNotActiveMover(WorldPacket &recvData)
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_MOVE_NOT_ACTIVE_MOVER");

    MovementInfo movementInfo;
    movementInfo.FillContentFromPacket(&recvData, true);

    _releaseMoverTimeout = 0;
    if (!_activeMover)
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleMoveNotActiveMover: The client doesn't control any unit right now");
        return;
    }

    if (_activeMover->GetGUID() != movementInfo.guid)
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleMoveNotActiveMover: The client is trying to desactive an unit which is not the active mover?");
        return;
    }

    TC_LOG_TRACE("movement", "%s: Player %s, removing unit %s from active mover",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), _activeMover->GetName().c_str());

    //It may be that another player already took control, we don't want to update mover movement info in that case
    if (IsAuthorizedToMove(movementInfo.guid, false))
    {
        if (Unit* mover = ObjectAccessor::GetUnit(*_player, movementInfo.guid))
        {
            mover->ValidateMovementInfo(&movementInfo);
            movementInfo.flags &= ~MOVEMENTFLAG_MASK_MOVING; //remove any moving flag, else we'll have unit rubber banding until someone else takes control of it
            mover->UpdateMovementInfo(movementInfo);
        }
        else
            TC_LOG_ERROR("entities.unit", "WorldSession::HandleMoveNotActiveMover: Did not find active mover %s",
                movementInfo.guid.ToString().c_str());
    }

    ResetActiveMover();
}

void WorldSession::SetClientControl(Unit* target, bool allowMove)
{
    // still affected by some aura that shouldn't allow control, only allow on last such aura to be removed
    if (allowMove && target->HasUnitState(UNIT_STATE_CANT_CLIENT_CONTROL))
        return;

    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, target->GetPackGUID().size() + 1);
    data << target->GetPackGUID();
    data << uint8(allowMove);
    SendPacket(&data);

    //sun: We told the client to change active mover but there will be some time before he respond
    //We delay setting active mover until the client actually tells us he takes control with CMSG_SET_ACTIVE_MOVER

    std::string action = allowMove ? "Sending" : "Removing";
    std::string to = allowMove ? "to" : "from";
    TC_LOG_TRACE("movement", "SMSG_CLIENT_CONTROL_UPDATE: %s control of unit %s %s client of %s",
        action.c_str(), target->GetName().c_str(), to.c_str(), _player->GetName().c_str());

    if (allowMove)
    {
        if (target == _player)
            _player->SetGuidValue(PLAYER_FARSIGHT, ObjectGuid::Empty);
        else
            _player->SetViewpoint(target, true); //Setting PLAYER_FARSIGHT will trigger CMSG_FAR_SIGHT from client, which will have no effect here since we already set vision at this point
    }
    else if (_player->GetGuidValue(PLAYER_FARSIGHT))
        _player->SetViewpoint(target, false);

    // Allow this session to take control of the unit with CMSG_SET_ACTIVE_MOVER
    if (allowMove)
    {
        _allowedClientControl.insert(target->GetGUID());
        //from this point, client is allowed to take control of the unit using CMSG_SET_ACTIVE_MOVER, but is not yet allowed to move it
    }
    else
    {
        _allowedClientControl.erase(target->GetGUID());
        if(target == _activeMover)
            _releaseMoverTimeout = sWorld->getIntConfig(CONFIG_PENDING_MOVE_CHANGES_TIMEOUT);
        //from this point, client not allowed to take the unit anymore using CMSG_SET_ACTIVE_MOVER, but may still be able to move/ack it until another player actives the unit as mover
    }
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket& /*recvData*/)
{
    WorldPacket data(SMSG_MOUNTSPECIAL_ANIM, 8);
    data << uint64(GetPlayer()->GetGUID());

    GetPlayer()->SendMessageToSet(&data, false);
}

// CMSG_MOVE_KNOCK_BACK_ACK
void WorldSession::HandleMoveKnockBackAck(WorldPacket& recvData)
{
    ObjectGuid guid;
    uint32 movementCounter;
    MovementInfo movementInfo;

#ifdef LICH_KING
    recvData >> guid.ReadAsPacked();
#else
    recvData >> guid;
#endif
    if (!IsAuthorizedToMove(guid, true))
        return;

    movementInfo.guid = guid;
    recvData >> movementCounter;
    movementInfo.FillContentFromPacket(&recvData, false);

    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);

    // verify that indeed the client is replying with the changes that were send to him
    if (!HasPendingMovementChange(movementCounter, recvData.GetOpcode(), guid))
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleMoveKnockBackAck: Player %s from account id %u kicked because no movement change ack was expected from this player",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }

    PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
    if (   std::fabs(pendingChange.knockbackInfo.speedXY - movementInfo.jump.xyspeed) > 0.01f
        || std::fabs(pendingChange.knockbackInfo.speedZ - movementInfo.jump.zspeed) > 0.01f
        || std::fabs(pendingChange.knockbackInfo.vcos - movementInfo.jump.cosAngle) > 0.01f
        || std::fabs(pendingChange.knockbackInfo.vsin - movementInfo.jump.sinAngle) > 0.01f)
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleMoveKnockBackAck: Player %s from account id %u kicked for incorrect data returned in an ack",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());

        _player->GetSession()->KickPlayer();
        return;
    }

    TC_LOG_TRACE("movement", "WorldSession::HandleMoveKnockBackAck: Received CMSG_MOVE_KNOCK_BACK_ACK from player %s for unit %s (%s), counter %u",
        _player->GetName().c_str(), mover->GetName().c_str(), mover->GetGUID().ToString().c_str(), movementCounter);

    pendingChange.Resolve(pendingChange, this, mover, &movementInfo);
}

/*
handles those packets:

APPLY:

CMSG_FORCE_MOVE_ROOT_ACK
CMSG_MOVE_GRAVITY_DISABLE_ACK

UNAPPLY:

CMSG_FORCE_MOVE_UNROOT_ACK
CMSG_MOVE_GRAVITY_ENABLE_ACK
*/
void WorldSession::HandleMovementFlagChangeAck(WorldPacket& recvData)
{
    /* extract packet */
    ObjectGuid guid;
    uint32 movementCounter;
    MovementInfo movementInfo;

#ifdef LICH_KING
    recvData >> guid.ReadAsPacked();
#else
    recvData >> guid;
#endif
    if (!IsAuthorizedToMove(guid, true))
        return;

    movementInfo.guid = guid;
    recvData >> movementCounter;
    movementInfo.FillContentFromPacket(&recvData);

    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);

    bool applyReceived;
    switch (recvData.GetOpcode())
    {
    case CMSG_FORCE_MOVE_ROOT_ACK:      applyReceived = true;  break;
    case CMSG_FORCE_MOVE_UNROOT_ACK:    applyReceived = false; break;
#ifdef LICH_KING
    case CMSG_MOVE_GRAVITY_DISABLE_ACK: applyReceived = true;  break;
    case CMSG_MOVE_GRAVITY_ENABLE_ACK:  applyReceived = false; break;
#endif
    default:
        TC_LOG_ERROR("movement", "WorldSession::HandleMovementFlagChangeAck: Unknown move type opcode: %s",
            GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str());
        return;
    }

    // verify that indeed the client is replying with the changes that were send to him
    if (!HasPendingMovementChange(movementCounter, recvData.GetOpcode(), guid, applyReceived))
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleMovementFlagChangeAck: Player %s from account id %u kicked because no movement change ack was expected from this player",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }

    TC_LOG_TRACE("movement", "Received %s from player %s for unit %s (%s), counter %u",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), mover->GetName().c_str(), mover->GetGUID().ToString().c_str(), movementCounter);

    PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
    pendingChange.Resolve(pendingChange, this, mover, &movementInfo);
}

/*
handles those packets:

CMSG_MOVE_WATER_WALK_ACK
CMSG_MOVE_HOVER_ACK
CMSG_MOVE_SET_CAN_FLY_ACK
CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK
CMSG_MOVE_FEATHER_FALL_ACK
*/
void WorldSession::HandleMovementFlagChangeToggleAck(WorldPacket& recvData)
{
    /* extract packet */
    ObjectGuid guid;
    uint32 movementCounter;
    MovementInfo movementInfo;
    uint32 applyInt;
    bool applyReceived;

#ifdef LICH_KING
    recvData >> guid.ReadAsPacked();
#else
    recvData >> guid;
#endif
    movementInfo.guid = guid;

    if (!IsAuthorizedToMove(guid, true))
        return; 

    recvData >> movementCounter;
    movementInfo.FillContentFromPacket(&recvData);
    recvData >> applyInt;
    applyReceived = applyInt == 0u ? false : true;
    
    Unit* mover = ObjectAccessor::GetUnit(*_player, guid);

    // verify that indeed the client is replying with the changes that were send to him
    if (!HasPendingMovementChange(movementCounter, recvData.GetOpcode(), guid, applyReceived))
    {
        TC_LOG_INFO("cheat", "WorldSession::HandleMovementFlagChangeToggleAck: Player %s from account id %u kicked because no movement change ack was expected from this player",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
        _player->GetSession()->KickPlayer();
        return;
    }

    TC_LOG_TRACE("movement", "Received %s from player %s for unit %s (%s), counter %u",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), mover->GetName().c_str(), mover->GetGUID().ToString().c_str(), movementCounter);

    PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
    pendingChange.Resolve(pendingChange, this, mover, &movementInfo);
}

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket& recvData)
{
    //Sun: Tentative implementation based on mangos logic
    //A comment regarding the role of CMSG_MOVE_TIME_SKIPPED: https://github.com/TrinityCore/TrinityCore/pull/18189#issuecomment-300423962

    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_MOVE_TIME_SKIPPED");
    ObjectGuid guid;
    uint32 timeSkipped;

#ifdef LICH_KING
    recvData >> guid.ReadAsPacked();
#else
    recvData >> guid;
#endif
    recvData >> timeSkipped;

    if (!IsAuthorizedToMove(guid, true))
        return;

    TC_LOG_TRACE("movement", "%s: Player %s, receiving time skip %u for active mover %s", 
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(recvData.GetOpcode())).c_str(), _player->GetName().c_str(), timeSkipped, _activeMover->GetName().c_str());

    WorldPacket data(MSG_MOVE_TIME_SKIPPED, 16);
    data << guid.WriteAsPacked();
    data << timeSkipped;
    _player->SendMessageToSet(&data, false);
}

/*
Handles CMSG_WORLD_TELEPORT. That packet is sent by the client when the user types a special build-in command restricted to GMs.
cf http://wow.gamepedia.com/Console_variables#GM_Commands
The usage of this packet is therefore restricted to GMs and will never be used by normal players.
*/
void WorldSession::HandleWorldTeleportOpcode(WorldPacket& recvData)
{
    uint32 time;
    uint32 mapid;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recvData >> time;                                      // time in m.sec.
    recvData >> mapid;
    recvData >> PositionX;
    recvData >> PositionY;
    recvData >> PositionZ;
    recvData >> Orientation;                               // o (3.141593 = 180 degrees)

    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_WORLD_TELEPORT");

    if (GetPlayer()->IsInFlight())
    {
        TC_LOG_DEBUG("movement", "Player '%s' (GUID: %u) in flight, ignore worldport command.",
            GetPlayer()->GetName().c_str(), GetPlayer()->GetGUID().GetCounter());
        return;
    }

    TC_LOG_TRACE("movement", "CMSG_WORLD_TELEPORT: Player = %s, Time = %u, map = %u, x = %f, y = %f, z = %f, o = %f",
        GetPlayer()->GetName().c_str(), time, mapid, PositionX, PositionY, PositionZ, Orientation);

    if (GetSecurity() >= SEC_GAMEMASTER3)
    //if (HasPermission(rbac::RBAC_PERM_OPCODE_WORLD_TELEPORT))
        GetPlayer()->TeleportTo(mapid, PositionX, PositionY, PositionZ, Orientation);
    else
    {
        //no effect //SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        TC_LOG_INFO("cheat", "WorldSession::HandleWorldTeleportOpcode: Non GM player %s (Account %u) tried to use gm teleport",
            _player->GetName().c_str(), _player->GetSession()->GetAccountId());
    }
}

void WorldSession::HandleSummonResponseOpcode(WorldPacket& recvData)
{
    if (!_player->IsAlive() || _player->IsInCombat())
        return;

    ObjectGuid summoner_guid;
    bool agree;
    recvData >> summoner_guid;
    recvData >> agree;

    _player->SummonIfPossible(agree);
}

void WorldSession::ResolveAllPendingChanges()
{
    if (!_activeMover)
        return;

    TC_LOG_TRACE("movement", "Resolve all pending change for player %s (%u) (for all controlled units)",
       _player->GetName().c_str(), _player->GetGUID().GetCounter());

    while (HasPendingMovementChange())
    {
        PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
        pendingChange.Resolve(pendingChange, this, nullptr, nullptr);
    }
}

void PlayerMovementPendingChange::_HandleMoveTeleportAck(WorldSession* session, Unit* mover, MovementInfo& /*movementInfo*/)
{
    Player* plMover = mover->ToPlayer();
    if (!plMover || !plMover->IsBeingTeleportedNear())
        return;

    plMover->SetSemaphoreTeleportNear(false);

    WorldLocation const& dest = plMover->GetTeleportDest();
    // now that it has been acknowledge, we can inform the observers of that teleport
    MovementInfo movementInfo = plMover->GetMovementInfo();
    movementInfo.pos.Relocate(dest);
    if (TransportBase* transportBase = plMover->GetDirectTransport())
    {
        float x, y, z, o;
        dest.GetPosition(x, y, z, o);
        transportBase->CalculatePassengerOffset(x, y, z, &o);
        movementInfo.transport.pos.Relocate(x, y, z, o);
    }
    MovementPacketSender::SendTeleportPacket(plMover, movementInfo);
    uint32 old_zone = plMover->GetZoneId();

    plMover->UpdatePosition(dest, true);
    plMover->SetFallInformation(0, session->GetPlayer()->GetPositionZ());

    uint32 newzone, newarea;
    plMover->GetZoneAndAreaId(newzone, newarea);
    plMover->UpdateZone(newzone, newarea);

    // new zone
    if (old_zone != newzone)
    {
        // honorless target
        if (plMover->pvpInfo.IsHostile)
            plMover->CastSpell(plMover, 2479, true);

        // in friendly area
        else if (plMover->IsPvP() && !plMover->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
            plMover->UpdatePvP(false, false);
    }

    // teleport pets if they are not unsummoned
    if (Pet* pet = plMover->GetPet())
    {
        if (!pet->IsWithinDist3d(plMover, plMover->GetMap()->GetVisibilityRange() - 5.0f))
            pet->NearTeleportTo(plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), pet->GetOrientation());
    }

    // resummon pet
    session->GetPlayer()->ResummonPetTemporaryUnSummonedIfAny();

    //lets process all delayed operations on successful teleport
    session->GetPlayer()->ProcessDelayedOperations();

    plMover->GetMotionMaster()->ReinitializeMovement();
}

void PlayerMovementPendingChange::Resolve(PlayerMovementPendingChange const& change, WorldSession* session, Unit* mover /*= nullptr*/, MovementInfo* movementInfo /*= nullptr*/)
{
    //if we're doing a serverside resolve, we're using the current movement info from the server, so no need to validate it
    bool serverSideResolve = mover == nullptr;
    MovementInfo serverMovementInfo = mover->GetMovementInfo();
    if (serverSideResolve)
    {
        ASSERT(movementInfo == nullptr); //else, wrong usage of this function
        mover = ObjectAccessor::GetUnit(*session->GetPlayer(), guid);
        if (!mover)
        {
            TC_LOG_ERROR("movement", "PlayerMovementPendingChange::Resolve: Failed to get unit %s in change (from player %s %s)",
                guid.ToString().c_str(), session->GetPlayer()->GetName().c_str(), session->GetPlayer()->GetGUID().ToString().c_str());
            return;
        }
        movementInfo = &serverMovementInfo;
    }
    else 
    {
        ASSERT(mover->GetGUID() == change.guid);
    }

    switch (movementChangeType)
    {
        case ROOT: // CMSG_FORCE_MOVE_ROOT_ACK && CMSG_FORCE_MOVE_UNROOT_ACK
        case WATER_WALK: // CMSG_MOVE_WATER_WALK_ACK
        case SET_HOVER:  // CMSG_MOVE_HOVER_ACK
        case SET_CAN_FLY: // CMSG_MOVE_SET_CAN_FLY_ACK
        case FEATHER_FALL: // CMSG_MOVE_FEATHER_FALL_ACK
    #ifdef LICH_KING
        case GRAVITY: // CMSG_MOVE_GRAVITY_DISABLE_ACK && CMSG_MOVE_GRAVITY_ENABLE_ACK?
        case SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY: // CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK
    #endif
            _HandleMovementFlagChangeToggleAck(session, mover, *movementInfo, !serverSideResolve);
            break;
    #ifdef LICH_KING
        case RATE_CHANGE_PITCH:
        case SET_COLLISION_HGT:
            TODO; //split the opcode handlers so that we can use them here
            break;
    #endif
        case KNOCK_BACK:
            _HandleMoveKnockBackAck(session, mover, *movementInfo, !serverSideResolve);
            break;
        case SPEED_CHANGE_WALK:
        case SPEED_CHANGE_RUN:
        case SPEED_CHANGE_RUN_BACK:
        case SPEED_CHANGE_SWIM:
        case SPEED_CHANGE_SWIM_BACK:
        case RATE_CHANGE_TURN:
        case SPEED_CHANGE_FLIGHT_SPEED:
        case SPEED_CHANGE_FLIGHT_BACK_SPEED:
            _HandleForceSpeedChangeAck(session, mover, *movementInfo);
            break;
        case TELEPORT:
            _HandleMoveTeleportAck(session, mover, *movementInfo);
            break;
        case INVALID:
        default:
            ASSERT(false);
    }
}

void PlayerMovementPendingChange::_HandleMovementFlagChangeToggleAck(WorldSession* session, Unit* mover, MovementInfo& movementInfo, bool validate)
{
    MovementFlags  mFlag = MOVEMENTFLAG_NONE;
    MovementFlags2 mFlag2 = MOVEMENTFLAG2_NONE;

    switch (movementChangeType)
    {
    case WATER_WALK:            mover->SetWaterWalkingReal(apply);   mFlag = MOVEMENTFLAG_WATERWALKING;    break;
    case SET_HOVER:             mover->SetHoverReal(apply);          mFlag = MOVEMENTFLAG_HOVER;           break;
    case SET_CAN_FLY:           mover->SetFlyingReal(apply);         mFlag = MOVEMENTFLAG_CAN_FLY;         break;
    case FEATHER_FALL:          mover->SetFeatherFallReal(apply);    mFlag = MOVEMENTFLAG_FALLING_SLOW;    break;
    case ROOT:                  mover->SetRootedReal(apply);         mFlag = MOVEMENTFLAG_ROOT;            break;
#ifdef LICH_KING 
    case SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY:
        mover->SetCanTransitionBetweenSwimAndFlyReal(change.apply);
        mFlag2 = MOVEMENTFLAG2_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY;
        break;
    case GRAVITY:               mover->SetDisableGravityReal(apply); mFlag = MOVEMENTFLAG_DISABLE_GRAVITY; break;
#endif
    default:
        TC_LOG_ERROR("movement", "WorldSession::HandleMovementFlagChangeToggleAck: Unknown move type %u for mover %s (GUID %s)",
            movementChangeType, mover->GetName().c_str(), mover->GetGUID().ToString().c_str());
        DEBUG_ASSERT(false);
        return;
}

    if(validate)
        mover->ValidateMovementInfo(&movementInfo);
    mover->UpdateMovementInfo(movementInfo);
    if (mFlag != MOVEMENTFLAG_NONE)
        MovementPacketSender::SendMovementFlagChangeToObservers(mover, mFlag, apply);
#ifdef LICH_KING
    else if (mFlag2 != MOVEMENTFLAG2_NONE)
        MovementPacketSender::SendMovementFlagChangeToObservers(mover, mFlag2);
#endif
}

void PlayerMovementPendingChange::_HandleForceSpeedChangeAck(WorldSession* /*session*/, Unit* mover, MovementInfo& movementInfo)
{
    UnitMoveType type;
    switch (movementChangeType)
    {
    case SPEED_CHANGE_WALK:              type = MOVE_WALK;        break;
    case SPEED_CHANGE_RUN:               type = MOVE_RUN;         break;
    case SPEED_CHANGE_RUN_BACK:          type = MOVE_RUN_BACK;    break;
    case SPEED_CHANGE_SWIM:              type = MOVE_SWIM;        break;
    case SPEED_CHANGE_SWIM_BACK:         type = MOVE_SWIM_BACK;   break;
    case RATE_CHANGE_TURN:               type = MOVE_TURN_RATE;   break;
    case SPEED_CHANGE_FLIGHT_SPEED:      type = MOVE_FLIGHT;      break;
    case SPEED_CHANGE_FLIGHT_BACK_SPEED: type = MOVE_FLIGHT_BACK; break;
    default:
        ASSERT(false); //this is server generated data, should never be false
    }

    float const newSpeed = newValue;
    float const newSpeedRate = newSpeed / baseMoveSpeed[type];
    mover->UpdateMovementInfo(movementInfo);
    mover->SetSpeedRateReal(type, newSpeedRate);
    MovementPacketSender::SendSpeedChangeToObservers(mover, type, newSpeed);
}

void PlayerMovementPendingChange::_HandleMoveKnockBackAck(WorldSession* session, Unit* mover, MovementInfo& movementInfo, bool validate)
{
    // knocking a player removes the CanFly flag (the client reacts the same way).
    mover->SetFlyingReal(false);

    if(validate)
        mover->ValidateMovementInfo(&movementInfo);
    /* the client data has been verified. let's do the actual change now */
    mover->UpdateMovementInfo(movementInfo);
    MovementPacketSender::SendKnockBackToObservers(mover, movementInfo.jump.cosAngle, movementInfo.jump.sinAngle, movementInfo.jump.xyspeed, movementInfo.jump.zspeed);

    session->anticheat->OnPlayerKnockBack(mover);
}


bool WorldSession::IsAuthorizedToTakeControl(ObjectGuid guid)
{
    return _allowedClientControl.find(guid) != _allowedClientControl.end();
}

bool WorldSession::IsAuthorizedToMove(ObjectGuid guid, bool log /*= true*/)
{
    bool authorized = _allowedClientMove.find(guid) != _allowedClientMove.end();
    if(!authorized && log)
        TC_LOG_DEBUG("movement", "player %s (%u) tried to move with non allowed unit %s",
            _player->GetName().c_str(), _player->GetGUID().GetCounter(), guid.ToString().c_str());
    
    return authorized;
}