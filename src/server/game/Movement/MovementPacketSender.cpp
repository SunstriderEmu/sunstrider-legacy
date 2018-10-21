
#include "MovementPacketSender.h"
#include "Player.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#ifdef LICH_KING
void MovementPacketSender::SendHeightChangeToMover(Unit* unit, float newHeight)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendHeightChangeToMover: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    PlayerMovementPendingChange pendingChange;
    pendingChange.guid = unit->GetGUID();
    pendingChange.movementChangeType = SET_COLLISION_HGT;
    pendingChange.newValue = newHeight;

    uint32 mCounter = unit->PushPendingMovementChange(pendingChange);

    WorldPacket data(SMSG_MOVE_SET_COLLISION_HGT, unit->GetPackGUID().size() + 4 + 4);
    data << unit->GetPackGUID();
    data << mCounter;
    data << newHeight;
    session->SendPacket(&data);

    TC_LOG_TRACE("movement", "Sent teleport ack to player %s for unit %s (%s)",
        session->GetPlayer()->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str());
}

void MovementPacketSender::SendHeightChangeToObservers(Unit* unit, float newHeight)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    Player* mover = session ? session->GetPlayer() : nullptr;
    if (!mover)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendHeightChangeToObservers: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    WorldPacket data(MSG_MOVE_SET_COLLISION_HGT, 8 + 30 + 4);
    unit->GetMovementInfo().WriteContentIntoPacket(&data, true);
    data << newHeight;
    unit->SendMessageToSet(&data, mover);
}
#endif

void MovementPacketSender::SendTeleportAckPacket(Unit* unit, MovementInfo const& movementInfo)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendTeleportAckPacket: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    PlayerMovementPendingChange pendingChange(session->GetPlayer()->GetMap()->GetGameTimeMS());
    pendingChange.guid = unit->GetGUID();
    pendingChange.movementChangeType = TELEPORT;

    uint32 mCounter = session->PushPendingMovementChange(pendingChange);

    WorldPacket data(MSG_MOVE_TELEPORT_ACK, 41);
    data << unit->GetPackGUID();
    data << mCounter;
    movementInfo.WriteContentIntoPacket(&data);
    session->SendPacket(&data);

    TC_LOG_TRACE("movement", "Sent teleport ack to player %s for unit %s (%s)",
        session->GetPlayer()->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str());
}

void MovementPacketSender::SendTeleportPacket(Unit* unit, MovementInfo const& movementInfo)
{
    // MSG_MOVE_TELEPORT is used in 2 different cases: 
    // 1) to teleport NPCs controlled by the server.
    // 2) to inform of the (acknowledged) teleport of a player-controlled unit
    // 3) teleport from an NPC casting blink?
    // that's why here, unit->GetPlayerMovingMe() CAN be NULL. In that case, we are in scenario 1.
    WorldSession* session = unit->GetPlayerMovingMe();

    WorldPacket data(MSG_MOVE_TELEPORT, 38);
    data << unit->GetPackGUID();
    movementInfo.WriteContentIntoPacket(&data);
    unit->SendMessageToSet(&data, session ? session->GetPlayer() : nullptr);

    if (session)
    {
        TC_LOG_TRACE("movement", "Sent teleport to set to player %s for unit %s (%s)",
            session->GetPlayer()->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str());
    }
    else 
    {
        TC_LOG_TRACE("movement", "Sent teleport for npc unit %s (%s)",
            unit->GetName().c_str(), unit->GetGUID().ToString().c_str());
    }
}

// Spline packets are for units controlled by the server. "Force speed change" (wrongly named opcodes) and "move set speed" packets are for units controlled by a player.
Opcodes const MovementPacketSender::moveTypeToOpcode[MAX_MOVE_TYPE][3] =
{
    { SMSG_SPLINE_SET_WALK_SPEED,        SMSG_FORCE_WALK_SPEED_CHANGE,           MSG_MOVE_SET_WALK_SPEED },
    { SMSG_SPLINE_SET_RUN_SPEED,         SMSG_FORCE_RUN_SPEED_CHANGE,            MSG_MOVE_SET_RUN_SPEED },
    { SMSG_SPLINE_SET_RUN_BACK_SPEED,    SMSG_FORCE_RUN_BACK_SPEED_CHANGE,       MSG_MOVE_SET_RUN_BACK_SPEED },
    { SMSG_SPLINE_SET_SWIM_SPEED,        SMSG_FORCE_SWIM_SPEED_CHANGE,           MSG_MOVE_SET_SWIM_SPEED },
    { SMSG_SPLINE_SET_SWIM_BACK_SPEED,   SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,      MSG_MOVE_SET_SWIM_BACK_SPEED },
    { SMSG_SPLINE_SET_TURN_RATE,         SMSG_FORCE_TURN_RATE_CHANGE,            MSG_MOVE_SET_TURN_RATE },
    { SMSG_SPLINE_SET_FLIGHT_SPEED,      SMSG_FORCE_FLIGHT_SPEED_CHANGE,         MSG_MOVE_SET_FLIGHT_SPEED },
    { SMSG_SPLINE_SET_FLIGHT_BACK_SPEED, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE,    MSG_MOVE_SET_FLIGHT_BACK_SPEED },
#ifdef LICH_KING
    { SMSG_SPLINE_SET_PITCH_RATE,        SMSG_FORCE_PITCH_RATE_CHANGE,           MSG_MOVE_SET_PITCH_RATE },
#endif
};

void MovementPacketSender::SendSpeedChangeToMover(Unit* unit, UnitMoveType mtype, float newRate)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendSpeedChangeToMover: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    float newSpeedFlat = newRate * baseMoveSpeed[mtype];
    PlayerMovementPendingChange pendingChange(unit->GetMap()->GetGameTimeMS());
    pendingChange.guid = unit->GetGUID();
    pendingChange.newValue = newSpeedFlat;
    pendingChange.movementChangeType = MovementPacketSender::GetChangeTypeByMoveType(mtype);
    uint32 mCounter = session->PushPendingMovementChange(pendingChange);

    WorldPacket data;
    data.Initialize(moveTypeToOpcode[mtype][1], mtype != MOVE_RUN ? 8 + 4 + 4 : 8 + 4 + 1 + 4);
    data << unit->GetPackGUID();
    data << mCounter;
    if (mtype == MOVE_RUN)
        data << uint8(1);                               // unknown byte added in 2.1.0
    data << newSpeedFlat;
    session->SendPacket(&data);

    TC_LOG_TRACE("movement", "Sent new speed %f to player %s for unit %s (%s), counter %u",
        newSpeedFlat, session->GetPlayer()->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str(), mCounter);
}

MovementChangeType MovementPacketSender::GetChangeTypeByMoveType(UnitMoveType moveType)
{
    switch (moveType)
    {
        case MOVE_WALK:         return SPEED_CHANGE_WALK;
        case MOVE_RUN:          return SPEED_CHANGE_RUN;
        case MOVE_RUN_BACK:     return SPEED_CHANGE_RUN_BACK;
        case MOVE_SWIM:         return SPEED_CHANGE_SWIM;
        case MOVE_SWIM_BACK:    return SPEED_CHANGE_SWIM_BACK;
        case MOVE_TURN_RATE:    return RATE_CHANGE_TURN;
        case MOVE_FLIGHT:       return SPEED_CHANGE_FLIGHT_SPEED;
        case MOVE_FLIGHT_BACK:  return SPEED_CHANGE_FLIGHT_BACK_SPEED;
#ifdef LICH_KING
        case MOVE_PITCH_RATE:   return RATE_CHANGE_PITCH;
#endif
        default:
            ASSERT(false, "MovementPacketSender::SendSpeedChangeToMover Unsupported UnitMoveType");
    }
}

void MovementPacketSender::SendSpeedChangeToObservers(Unit* unit, UnitMoveType mtype, float newRate)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendSpeedChangeToObservers: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    WorldPacket data;
    data.Initialize(moveTypeToOpcode[mtype][2], 8 + 30 + 4);
    unit->GetMovementInfo().WriteContentIntoPacket(&data, true);
    data << newRate;
    unit->SendMessageToSet(&data, session->GetPlayer());
}

void MovementPacketSender::SendSpeedChangeToAll(Unit* unit, UnitMoveType mtype, float newRate)
{
    WorldPacket data;
    data.Initialize(moveTypeToOpcode[mtype][0], 8 + 4);
    data << unit->GetPackGUID();
    data << newRate;
    unit->SendMessageToSet(&data, true);
}

void MovementPacketSender::SendKnockBackToMover(Unit* unit, float vcos, float vsin, float speedXY, float speedZ)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendKnockBackToMover: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    PlayerMovementPendingChange pendingChange(unit->GetMap()->GetGameTimeMS());
    pendingChange.guid = unit->GetGUID();
    pendingChange.movementChangeType = KNOCK_BACK;
    pendingChange.knockbackInfo.vcos = vcos;
    pendingChange.knockbackInfo.vsin = vsin;
    pendingChange.knockbackInfo.speedXY = speedXY;
    pendingChange.knockbackInfo.speedZ = speedZ;

    uint32 mCounter = session->PushPendingMovementChange(pendingChange);

    WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8 + 4 + 4 + 4 + 4 + 4));
    data << unit->GetPackGUID();
    data << mCounter;
    data << float(vcos);                                    // x direction
    data << float(vsin);                                    // y direction
    data << float(speedXY);                                 // Horizontal speed
    data << float(speedZ);                                 // Z Movement speed (vertical)
    session->SendPacket(&data);

    TC_LOG_TRACE("movement", "Sent knockback to player %s for unit %s (%s), counter %u",
        session->GetPlayer()->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str(), mCounter);
}

void MovementPacketSender::SendKnockBackToObservers(Unit* unit, float vcos, float vsin, float speedXY, float speedZ)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendSpeedChangeToObservers: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    WorldPacket data(MSG_MOVE_KNOCK_BACK, 66);
    unit->GetMovementInfo().WriteContentIntoPacket(&data, true);
    data << vcos;
    data << vsin;
    data << speedXY;
    data << speedZ;
    unit->SendMessageToSet(&data, session->GetPlayer());
}

void MovementPacketSender::SendMovementFlagChangeToMover(Unit* unit, MovementFlags mFlag, bool apply)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    if (!session)
    {
        TC_LOG_ERROR("entities.unit", "MovementPacketSender::SendMovementFlagChangeToMover: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    Opcodes opcode;
    MovementChangeType movementChangeType;
    switch (mFlag)
    {
        case MOVEMENTFLAG_ROOT:                 opcode = apply ? SMSG_FORCE_MOVE_ROOT : SMSG_FORCE_MOVE_UNROOT; movementChangeType = ROOT; break;
        case MOVEMENTFLAG_WATERWALKING:         opcode = apply ? SMSG_MOVE_WATER_WALK : SMSG_MOVE_LAND_WALK; movementChangeType = WATER_WALK; break;
        case MOVEMENTFLAG_HOVER:                opcode = apply ? SMSG_MOVE_SET_HOVER : SMSG_MOVE_UNSET_HOVER; movementChangeType = SET_HOVER; break;
        case MOVEMENTFLAG_CAN_FLY:              opcode = apply ? SMSG_MOVE_SET_CAN_FLY : SMSG_MOVE_UNSET_CAN_FLY; movementChangeType = SET_CAN_FLY; break;
        case MOVEMENTFLAG_FALLING_SLOW:         opcode = apply ? SMSG_MOVE_FEATHER_FALL : SMSG_MOVE_NORMAL_FALL; movementChangeType = FEATHER_FALL; break;
#ifdef LICH_KING
        case MOVEMENTFLAG_DISABLE_GRAVITY:      opcode = apply ? SMSG_MOVE_GRAVITY_DISABLE : SMSG_MOVE_GRAVITY_ENABLE; movementChangeType = GRAVITY_DISABLE; break;
#endif
        default:
            TC_LOG_ERROR("movement", "MovementPacketSender::SendMovementFlagChangeToMover: Unsupported MovementFlag (%d), data not sent to client.", mFlag);
            DEBUG_ASSERT(false);
            return;
    }

    PlayerMovementPendingChange pendingChange(unit->GetMap()->GetGameTimeMS());
    pendingChange.guid = unit->GetGUID();
    pendingChange.movementChangeType = movementChangeType;
    pendingChange.apply = apply;

    uint32 mCounter = session->PushPendingMovementChange(pendingChange);


    WorldPacket data(opcode, 12 + 4); // is the upper bound of unit->GetPackGUID().size() indeed equal to 12?
    data << unit->GetPackGUID();
    data << mCounter;
    session->SendPacket(&data);

    TC_LOG_TRACE("movement", "Sent %s to player %s for unit %s (%s), counter %u",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), session->GetPlayer()->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str(), mCounter);
}

#ifdef LICH_KING
void MovementPacketSender::SendMovementFlagChangeToMover(Unit* unit, MovementFlags2 mFlag, bool apply)
{
    WorldSession* session = unit->GetPlayerMovingMe();
    Player* player = unit->GetPlayerMovingMe();
    if (!mover)
    {
        TC_LOG_ERROR("movement", "MovementPacketSender::SendMovementFlagChangeToMover: Incorrect use of the function. It was called on a unit controlled by the server!");
        return;
    }

    Opcodes opcode;
    MovementChangeType movementChangeType;
    switch (mFlag)
    {
        case MOVEMENTFLAG2_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY: opcode = apply ? SMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY : SMSG_MOVE_UNSET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY; movementChangeType = SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY; break;
        default:
            TC_LOG_ERROR("movement", "MovementPacketSender::SendMovementFlagChangeToMover: Unsupported MovementFlag2 (%d), data not sent to client.", mFlag);
            DEBUG_ASSERT(false);
            return;
    }

    WorldSession* session = mover->GetSession();
    PlayerMovementPendingChange pendingChange;
    pendingChange.guid = unit->GetGUID();
    pendingChange.movementChangeType = movementChangeType;
    pendingChange.apply = apply;

    uint32 mCounter = session->PushPendingMovementChange(pendingChange);

    WorldPacket data(opcode, 12 + 4); // is the upper bound of unit->GetPackGUID().size() indeed equal to 12?
    data << unit->GetPackGUID();
    data << mCounter;
    mover->GetSession()->SendPacket(&data);

    TC_LOG_TRACE("movement", "Sent  %s to player %s for unit %s (%s), counter %u",
        GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), player->GetName().c_str(), unit->GetName().c_str(), unit->GetGUID().ToString().c_str(), mCounter);
}
#endif

void MovementPacketSender::SendMovementFlagChangeToObservers(Unit* unit, MovementFlags mFlag, bool apply)
{
    Opcodes opcode;
    switch (mFlag)
    {
        case MOVEMENTFLAG_ROOT:                 opcode = apply ? MSG_MOVE_ROOT : MSG_MOVE_UNROOT; break;
        case MOVEMENTFLAG_WATERWALKING:         opcode = MSG_MOVE_WATER_WALK; break;
        case MOVEMENTFLAG_HOVER:                opcode = MSG_MOVE_HOVER; break;
        case MOVEMENTFLAG_CAN_FLY:              opcode = MSG_MOVE_UPDATE_CAN_FLY; break;
        case MOVEMENTFLAG_FALLING_SLOW:         opcode = MSG_MOVE_FEATHER_FALL; break;
#ifdef LICH_KING
        case MOVEMENTFLAG_DISABLE_GRAVITY:      opcode = MSG_MOVE_GRAVITY_CHNG; break;
#endif
        default:
            TC_LOG_ERROR("movement", "MovementPacketSender::SendMovementFlagChangeToObservers: Unsupported MovementFlag (%d), data not sent to client.", mFlag);
            DEBUG_ASSERT(false);
            return;
    }

    WorldPacket data(opcode, 64);
    unit->GetMovementInfo().WriteContentIntoPacket(&data, true);
    unit->SendMessageToSet(&data, false);
}

#ifdef LICH_KING
void MovementPacketSender::SendMovementFlagChangeToObservers(Unit* unit, MovementFlags2 mFlag)
{
    Opcodes opcode;
    switch (mFlag)
    {
        case MOVEMENTFLAG2_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY:     opcode = MSG_MOVE_UPDATE_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY; break;
        default:
            TC_LOG_ERROR("movement", "MovementPacketSender::SendMovementFlagChangeToObservers: Unsupported MovementFlag (%d), data not sent to client.", mFlag);
            return;
    }

    WorldPacket data(opcode, 64);
    unit->GetMovementInfo().WriteContentIntoPacket(&data, true);
    unit->SendMessageToSet(&data, false);
}
#endif

void MovementPacketSender::SendMovementFlagChangeToAll(Unit* unit, MovementFlags mFlag, bool apply)
{
    // MOVEMENTFLAG_CAN_FLY & MOVEMENTFLAG2_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY have no equivalent to the player controlled opcodes.
    Opcodes opcode;
    switch (mFlag)
    {
#ifdef LICH_KING
        case MOVEMENTFLAG_DISABLE_GRAVITY:  opcode = apply ? SMSG_SPLINE_MOVE_GRAVITY_DISABLE   : SMSG_SPLINE_MOVE_GRAVITY_ENABLE; break;
#endif
        case MOVEMENTFLAG_ROOT:             opcode = apply ? SMSG_SPLINE_MOVE_ROOT              : SMSG_SPLINE_MOVE_UNROOT; break;
        case MOVEMENTFLAG_WATERWALKING:     opcode = apply ? SMSG_SPLINE_MOVE_WATER_WALK        : SMSG_SPLINE_MOVE_LAND_WALK; break;
        case MOVEMENTFLAG_FALLING_SLOW:     opcode = apply ? SMSG_SPLINE_MOVE_FEATHER_FALL      : SMSG_SPLINE_MOVE_NORMAL_FALL; break;
        case MOVEMENTFLAG_HOVER:            opcode = apply ? SMSG_SPLINE_MOVE_SET_HOVER         : SMSG_SPLINE_MOVE_UNSET_HOVER; break;
        default:
            TC_LOG_ERROR("movement", "MovementPacketSender::SendMovementFlagChangeToAll: Unsupported MovementFlag (%d), data not sent to client.", mFlag);
            DEBUG_ASSERT(false);
            return;
    }

    WorldPacket data;
    data.Initialize(opcode, unit->GetPackGUID().size() + 4);
    data << unit->GetPackGUID();
    unit->SendMessageToSet(&data, true);
}
