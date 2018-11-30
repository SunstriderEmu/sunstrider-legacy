#include "ClientControl.h"
#include "MovementPacketSender.h"
#include "UpdateTime.h"

ClientControl::ClientControl(WorldSession* session) :
    _owner(session),
    _movementCounter(0),
    lastMoveClientTimestamp(0),
    lastMoveServerTimestamp(0),
    _pendingActiveMoverSplineId(0),
    _activeMover(),
    _serverActiveMover()
{
    _this = std::shared_ptr<ClientControl>(this, [](ClientControl*) {});
}

void ClientControl::Update(uint32 diff)
{
    CheckPendingMovementAcks();
}

void ClientControl::ResolveAllPendingChanges()
{
    if (_activeMover.expired())
        return;

    TC_LOG_TRACE("movement", "Resolve all pending change for player %s (%u) (for all controlled units)",
        _owner->GetPlayer()->GetName().c_str(), _owner->GetPlayer()->GetGUID().GetCounter());

    while (HasPendingMovementChange())
    {
        PlayerMovementPendingChange pendingChange = PopPendingMovementChange().second;
        pendingChange.Resolve(pendingChange, _owner, nullptr, nullptr);
    }
}

bool ClientControl::IsAllowedToTakeControl(ObjectGuid guid)
{
    return _allowedClientControl.find(guid) != _allowedClientControl.end();
}

bool ClientControl::IsAllowedToMove(ObjectGuid guid, bool log /*= true*/)
{
    bool authorized = _allowedClientMove.find(guid) != _allowedClientMove.end();
    if (!authorized && log)
        TC_LOG_DEBUG("movement", "player %s (%u) tried to move with non allowed unit %s",
            _owner->GetPlayer()->GetName().c_str(), _owner->GetPlayer()->GetGUID().GetCounter(), guid.ToString().c_str());

    return authorized;
}

Unit* ClientControl::GetAllowedActiveMover() const
{
    Unit* activeMover = _activeMover.lock().get();
    if (activeMover && _allowedClientMove.find(activeMover->GetGUID()) != _allowedClientMove.end() && !activeMover->IsMoverSuppressed())
        return activeMover;

    return nullptr;
}

void ClientControl::InitActiveMover(Unit* activeMover)
{
    _activeMover.reset();
    _serverActiveMover.unit = activeMover->_this;
    activeMover->m_playerMovingMe = _this;
    _allowedClientControl.insert(activeMover->GetGUID());
    //Client will send CMSG_SET_ACTIVE_MOVER when joining a map
}

void ClientControl::AllowMover(Unit* mover)
{
    _allowedClientMove.insert(mover->GetGUID());
    _pendingActiveMoverSplineId = 0;
}

void ClientControl::DisallowMover(Unit* mover)
{
    _allowedClientMove.erase(mover->GetGUID());
    ResolveAllPendingChanges();
}

void ClientControl::UpdateTakeControl(Unit* target, bool allowMove)
{
    bool const targetSuppressed = target->IsMoverSuppressed();
    if (allowMove)
    {
        if (_serverActiveMover.unit.lock().get() == target)
            return; //we already have control

        _serverActiveMover = ServerActiveMover(target->_this, targetSuppressed);
        target->m_playerMovingMe = _this;
        if (targetSuppressed)
            return; //don't send control update, we need to delay it in this case
    }
    else if (/*!allowMove &&*/ _serverActiveMover.unit.lock().get() == target)
    {
        _serverActiveMover.Reset();
        target->m_playerMovingMe.reset();
    }

    SetClientControl(target, allowMove);
}

void ClientControl::SetClientControl(Unit* target, bool allowMove)
{
    std::string action = allowMove ? "Sending" : "Removing";
    std::string to = allowMove ? "to" : "from";
    TC_LOG_TRACE("movement", "SMSG_CLIENT_CONTROL_UPDATE: %s control of unit %s %s client of %s",
        action.c_str(), target->GetName().c_str(), to.c_str(), _owner->GetPlayer()->GetName().c_str());

    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, target->GetPackGUID().size() + 1);
    data << target->GetPackGUID();
    data << uint8(allowMove);
    _owner->SendPacket(&data);

    //sun: We told the client to change active mover but there will be some time before he respond
    //We delay setting active mover until the client actually tells us he takes control with CMSG_SET_ACTIVE_MOVER
}

ClientControl::PendingChangePair ClientControl::PopPendingMovementChange()
{
    auto result = _pendingMovementChanges.front();
    _pendingMovementChanges.pop_front();
    return result;
}

uint32 ClientControl::PushPendingMovementChange(PlayerMovementPendingChange newChange)
{
    //Apparently retail has the counter on unit rather than per session. I don't think this makes a difference.
    _pendingMovementChanges.emplace_back(std::make_pair(++_movementCounter, std::move(newChange)));
    return _movementCounter;
}

bool ClientControl::HasPendingMovementChange(uint32 counter, uint16 opcode, ObjectGuid guid, bool apply) const
{
    if (_pendingMovementChanges.empty())
        return false;

    MovementChangeType type = INVALID;
    switch (opcode)
    {
    case MSG_MOVE_TELEPORT_ACK:                        type = TELEPORT;                         break;
    case CMSG_MOVE_KNOCK_BACK_ACK:                     type = KNOCK_BACK;                       break;
    case CMSG_FORCE_MOVE_ROOT_ACK:                     type = ROOT;                             break;
    case CMSG_FORCE_MOVE_UNROOT_ACK:                   type = ROOT;                             break;
    case CMSG_MOVE_WATER_WALK_ACK:                     type = WATER_WALK;                       break;
    case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:             type = SPEED_CHANGE_WALK;                break;
    case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:              type = SPEED_CHANGE_RUN;                 break;
    case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:         type = SPEED_CHANGE_RUN_BACK;            break;
    case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:             type = SPEED_CHANGE_SWIM;                break;
    case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:        type = SPEED_CHANGE_SWIM_BACK;           break;
    case CMSG_FORCE_TURN_RATE_CHANGE_ACK:              type = RATE_CHANGE_TURN;                 break;
    case CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK:           type = SPEED_CHANGE_FLIGHT_SPEED;        break;
    case CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:      type = SPEED_CHANGE_FLIGHT_BACK_SPEED;   break;
    case CMSG_MOVE_HOVER_ACK:                          type = SET_HOVER;                        break;
    case CMSG_MOVE_SET_CAN_FLY_ACK:                    type = SET_CAN_FLY;                      break;
    case CMSG_MOVE_FEATHER_FALL_ACK:                   type = FEATHER_FALL;                     break;
#ifdef LICH_KING
    case LK_CMSG_FORCE_PITCH_RATE_CHANGE_ACK:          type = RATE_CHANGE_PITCH;                break;
    case LK_CMSG_MOVE_GRAVITY_DISABLE_ACK:             type = GRAVITY;                          break;
    case LK_CMSG_MOVE_GRAVITY_ENABLE_ACK:              type = GRAVITY;                          break;
    case LK_CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK: type = SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY; break;
    case LK_CMSG_MOVE_SET_COLLISION_HGT_ACK:           type = SET_COLLISION_HGT;                break;
#endif
    default:
        return false;
    }

    if (_pendingMovementChanges.front().first != counter)
        return false;

    PlayerMovementPendingChange const& change = _pendingMovementChanges.front().second;
    bool found = change.movementChangeType == type
        && change.guid == guid
        && change.apply == apply;

    if (!found)
    {
        TC_LOG_ERROR("movement", "Received ack with existing counter %u but incorrect type %u or guid %s or apply %u (expected type %u and guid %s and apply %u)",
            counter, type, guid.ToString().c_str(), uint8(apply), change.movementChangeType, change.guid.ToString().c_str(), uint8(change.apply));
    }
    return found;
}

bool ClientControl::HasPendingMovementChange(MovementChangeType changeType) const
{
    return std::find_if(_pendingMovementChanges.begin(), _pendingMovementChanges.end(),
        [changeType](ClientControl::PendingChangePair const& pendingChange)
    {
        return pendingChange.second.movementChangeType == changeType;
    }) != _pendingMovementChanges.end();
}

void ClientControl::CheckPendingMovementAcks()
{
    if (!HasPendingMovementChange())
        return;

    if (!_owner->GetPlayer())
        return; //is this possible?

    Map* map = _owner->GetPlayer()->FindMap();
    if (!map)
        return;

    PlayerMovementPendingChange const& oldestChangeToAck = _pendingMovementChanges.front().second;
    if (map->GetGameTimeMS() > oldestChangeToAck.time + sWorld->getIntConfig(CONFIG_PENDING_MOVE_CHANGES_TIMEOUT) + sWorldUpdateTime.GetLastUpdateTime()) //added update time to compenstate for big server lag or debugger usage
    {
        /*
        when players are teleported from one corner of a map to an other (example: from Dragonblight to the entrance of Naxxramas, both in the same map: Northend),
        is it done through what is called a 'near' teleport. A near teleport always involve teleporting a player from one point to an other in the same map, even if
        the distance is huge. When that distance is big enough, a loading screen appears on the client side. During that time, the client loads the surrounding zone
        of the new location (and everything it contains). The problem is that, as long as the client hasn't finished loading the new zone, it will NOT ack the near
        teleport. So if the server sends a near teleport order at a certain time and the client takes 20s to load the new zone (let's imagine a very slow computer),
        even with zero latency, the server will receive an ack from the client only after 20s.
        For this reason and because the current implementation is simple (you dear reader, feel free to improve it if you can), we will just ignore checking for
        near teleport acks (for now. @todo).
        */
        if (oldestChangeToAck.movementChangeType == TELEPORT)
            return;

        TC_LOG_INFO("cheat", "Unit::CheckPendingMovementAcks: Player %s (GUID: %u) took too long to acknowledge a movement change. He was therefore kicked.", _owner->GetPlayer()->GetName().c_str(), _owner->GetPlayer()->GetGUID().GetCounter());
        _owner->KickPlayer();
    }
}

//Following logic is partially disabled with #ifdef TODOMOV:
// Need to fix: https://github.com/ValorenWoW/sunstrider-core/pull/67
//
//Here is the active mover sequence:
// - Server tell the unit of his new active mover with SMSG_CLIENT_CONTROL_UPDATE 
// - Client respond with CMSG_SET_ACTIVE_MOVER to enable this mover
// - We send a spline on current unit position to transfer correct movement info
// - Client respond CMSG_MOVE_SPLINE_DONE, transfer is complete
// The spline step is necessary because when mover changed, it's possible that the client controlling it has not yet received 
// the move flags of the mover that were set just before he took control. This is not possible when we keep the same mover since
// those changes must be acked by client. Also, same problem is present with speed changes.
void ClientControl::SetActiveMover(Unit* activeMover)
{
    //Resolve all pending changes for this unit before taking control
    if (ClientControl* session = activeMover->GetPlayerMovingMe())
        session->DisallowMover(activeMover);

    DisallowMover(activeMover);

    _activeMover = activeMover->_this;

#ifdef TODOMOV
    _pendingActiveMoverSplineId = activeMover->StopMovingOnCurrentPos(); //Send spline movement before allowing move
    if (_pendingActiveMoverSplineId == 0)
    {
        TC_LOG_FATAL("movement", "WorldSession::SetActiveMover: player %s with pending mover %s, FAILED to start spline movement",
            _owner->GetPlayer()->GetName().c_str(), _activeMover->GetGUID().ToString().c_str());

        //Unit will get stuck. Shouldn't ever happen in current impl.
        DEBUG_ASSERT(false);
    }

    TC_LOG_TRACE("movement", "Received CMSG_SET_ACTIVE_MOVER for player %s with pending mover %s (%s), now sending the spline movement (id %u)",
        _owner->GetPlayer()->GetName().c_str(), _activeMover->GetName().c_str(), _activeMover->GetGUID().ToString().c_str(), _pendingActiveMoverSplineId);
#else
    if (IsAllowedToTakeControl(activeMover->GetGUID()))
    {
        AllowMover(activeMover);
        TC_LOG_TRACE("movement", "WorldSession::SetActiveMover: Enabling move of unit %s (%s) to player %s (%s)",
            activeMover->GetName().c_str(), activeMover->GetGUID().ToString().c_str(), _owner->GetPlayer()->GetName().c_str(), _owner->GetPlayer()->GetName().c_str());
    }
    else
    {
        TC_LOG_ERROR("movement", "WorldSession::SetActiveMover: Failed enabling move of unit %s (%s) to player %s (%s), pending spline id is correct but player is not allowed to take control anymore",
            activeMover->GetName().c_str(), activeMover->GetGUID().ToString().c_str(), _owner->GetPlayer()->GetName().c_str(), _owner->GetPlayer()->GetName().c_str());
    }
#endif
}

void ClientControl::PlayerDisconnect()
{
    if (Unit* u = _serverActiveMover.unit.lock().get())
    {
        ASSERT(u->m_playerMovingMe.lock().get() == this);
        u->m_playerMovingMe.reset();
        _serverActiveMover.Reset();
    }
}

void ClientControl::ResetActiveMover()
{
    Unit* previousMover = _activeMover.lock().get();
    _activeMover.reset();
    _owner->anticheat.OnPlayerMoverChanged(previousMover, nullptr);
}

void ClientControl::ClientSetActiveMover(ObjectGuid guid)
{
    Unit* activeMover = _activeMover.lock().get();
    if (activeMover && activeMover->GetGUID() == guid)
        return; //we already control this unit, ignore

    if (!IsAllowedToTakeControl(guid))
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleSetActiveMoverOpcode: The client of player %s doesn't have the permission to control this unit (GUID: %s)!", _owner->GetPlayer()->GetName().c_str(), guid.ToString().c_str());
        return;
    }

    Unit* mover = ObjectAccessor::GetUnit(*_owner->GetPlayer(), guid);
    if (!mover)
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleSetActiveMoverOpcode: The client provided an invalid %s (unit may also have just been removed for world)", guid.ToString().c_str());
        return;
    }

    TC_LOG_TRACE("movement", "CMSG_SET_ACTIVE_MOVER: Player %s, setting unit %s as active mover",
        _owner->GetPlayer()->GetName().c_str(), mover->GetName().c_str());

    ResetActiveMover();
    SetActiveMover(mover);
}

void ClientControl::ClientSetNotActiveMover(MovementInfo& movementInfo)
{
    Unit* mover = _activeMover.lock().get();
    if (!mover)
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleMoveNotActiveMover: The client doesn't control any unit right now");
        return;
    }
    ResetActiveMover();

    if (mover->GetGUID() != movementInfo.guid)
    {
        TC_LOG_ERROR("movement", "WorldSession::HandleMoveNotActiveMover: The client is trying to desactive an unit which is not the active mover?");
        return;
    }

    TC_LOG_TRACE("movement", "CMSG_MOVE_NOT_ACTIVE_MOVER: Player %s, removing unit %s from active mover",
        _owner->GetPlayer()->GetName().c_str(), mover->GetName().c_str());

    //It may be that another player already took control, we don't want to update mover movement info in that case
    if (IsAllowedToMove(movementInfo.guid, false))
    {
        mover->ValidateMovementInfo(&movementInfo);
        movementInfo.flags &= ~MOVEMENTFLAG_MASK_MOVING; //remove any moving flag, else we'll have unit rubber banding until someone else takes control of it
        mover->UpdateMovementInfo(movementInfo);
        //send move (not sure which opcode is sent for this)
        WorldPacket data(MSG_MOVE_STOP, sizeof(MovementInfo));
        mover->GetMovementInfo().WriteContentIntoPacket(&data, true);  //this contains the server time, not the time provided by client
        mover->SendMessageToSet(&data, _owner->GetPlayer());
    }
}

void ClientControl::AllowTakeControl(Unit* target, bool charm)
{
    // Allow this session to take control of the unit with CMSG_SET_ACTIVE_MOVER
    _allowedClientControl.insert(target->GetGUID());
    //from this point, client is allowed to take control of the unit using CMSG_SET_ACTIVE_MOVER, but is not yet allowed to move it

    bool takeControlImmediately = charm || _serverActiveMover.unit.expired();
    if (takeControlImmediately)
    {
        // Set viewpoint
        if (target == _owner->GetPlayer())
            _owner->GetPlayer()->SetGuidValue(PLAYER_FARSIGHT, ObjectGuid::Empty);
        else
            _owner->GetPlayer()->SetViewpoint(target, true); //Setting PLAYER_FARSIGHT will trigger CMSG_FAR_SIGHT from client (which will have no effect here since we already set vision in SetViewpoint)

        UpdateTakeControl(target, true);
    }
}

void ClientControl::DisallowTakeControl(Unit* target)
{
    _allowedClientControl.erase(target->GetGUID());
    //from this point, client not allowed to take the unit anymore using CMSG_SET_ACTIVE_MOVER, but may still be able to move/ack it until another player actives the unit as mover

    if (Unit* mover = _serverActiveMover.unit.lock().get())
    {
        // if we had this target as active mover, send remove control
        if (mover == target)
        {
            UpdateTakeControl(target, false);

            // Also restore control to first other unit we can control
            if (!_allowedClientControl.empty())
            {
                if (Unit* mover = ObjectAccessor::GetUnit(*_owner->GetPlayer(), *(_allowedClientControl.begin())))
                    UpdateTakeControl(mover, true);
            }
        }
    }

    if (_owner->GetPlayer()->GetGuidValue(PLAYER_FARSIGHT) == target->GetGUID())
        _owner->GetPlayer()->SetViewpoint(target, false);
}

void ClientControl::UpdateSuppressedMover(Unit* target)
{
    ASSERT(_serverActiveMover.unit.lock().get() == target);
    ASSERT(target->m_playerMovingMe.lock().get() == this);
    bool suppressed = target->IsMoverSuppressed();
    if (_serverActiveMover.suppressed != suppressed) //if suppressed state changed, update client control
    {
        SetClientControl(target, !suppressed);
        _serverActiveMover.suppressed = suppressed;
    }
}