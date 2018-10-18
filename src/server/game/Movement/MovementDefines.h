/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#ifndef TRINITY_MOVEMENTDEFINES_H
#define TRINITY_MOVEMENTDEFINES_H

#include "Common.h"
#include <memory>
#include <queue>

enum UnitMoveType : uint8
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
    MOVE_FLIGHT         = 6,
    MOVE_FLIGHT_BACK    = 7,

    MAX_MOVE_TYPE,
};

// Creature Entry ID used for waypoints show, visible only for GMs
#define VISUAL_WAYPOINT 1
// assume it is 25 yard per 0.6 second
#define SPEED_CHARGE    42.0f

TC_GAME_API extern float baseMoveSpeed[MAX_MOVE_TYPE];
//sun: removed playerBaseMoveSpeed, use baseMoveSpeed instead. Speeds are the same, this was just a way to handle the retired "Rate.MoveSpeed" config

enum MovementChangeType
{
    INVALID,

    ROOT,
    WATER_WALK,
    SET_HOVER,
    SET_CAN_FLY,
#ifdef LICH_KING
    SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,
    RATE_CHANGE_PITCH,
    SET_COLLISION_HGT,
    GRAVITY,
#endif
    FEATHER_FALL,

    SPEED_CHANGE_WALK,
    SPEED_CHANGE_RUN,
    SPEED_CHANGE_RUN_BACK,
    SPEED_CHANGE_SWIM,
    SPEED_CHANGE_SWIM_BACK,
    RATE_CHANGE_TURN,
    SPEED_CHANGE_FLIGHT_SPEED,
    SPEED_CHANGE_FLIGHT_BACK_SPEED,

    TELEPORT,
    KNOCK_BACK
};

class PlayerMovementPendingChange
{
public:
    ObjectGuid guid;
    MovementChangeType movementChangeType = INVALID; 
    uint32 time = 0;

    float newValue = 0.0f; // used if speed or height change
    bool apply = false; // used if movement flag change
    struct KnockbackInfo
    {
        float vcos = 0.0f;
        float vsin = 0.0f;
        float speedXY = 0.0f;
        float speedZ = 0.0f;
    } knockbackInfo; // used if knockback

    PlayerMovementPendingChange(uint32 time);

    // If using this to resolve from a client ack, you must provide mover and movementInfo. Mover guid must match the guid in the pending change.
    // Else, to resolve from the server info only (mover and movementInfo should be null)
    void Resolve(PlayerMovementPendingChange const& change, WorldSession* session, Unit* mover = nullptr, MovementInfo* movementInfo = nullptr);
private:
    void _HandleMoveKnockBackAck(WorldSession* session, Unit* mover, MovementInfo& movementInfo, bool validate);
    void _HandleMoveTeleportAck(WorldSession* session, Unit* mover, MovementInfo& movementInfo);
    void _HandleMovementFlagChangeToggleAck(WorldSession* session, Unit* mover, MovementInfo& movementInfo, bool validate);
    void _HandleForceSpeedChangeAck(WorldSession* session, Unit* mover, MovementInfo& movementInfo);
};

// values 0 ... MAX_DB_MOTION_TYPE-1 used in DB
enum MovementGeneratorType : uint8
{
    IDLE_MOTION_TYPE                = 0,     // IdleMovementGenerator.h
    RANDOM_MOTION_TYPE              = 1,     // RandomMovementGenerator.h
    WAYPOINT_MOTION_TYPE            = 2,     // WaypointMovementGenerator.h
    MAX_DB_MOTION_TYPE              = 3,     // *** this and below motion types can't be set in DB.
    ANIMAL_RANDOM_MOTION_TYPE       = 3,     // AnimalRandomMovementGenerator.h
    CONFUSED_MOTION_TYPE            = 4,     // ConfusedMovementGenerator.h
    CHASE_MOTION_TYPE               = 5,     // ChaseMovementGenerator.h
    HOME_MOTION_TYPE                = 6,     // HomeMovementGenerator.h
    FLIGHT_MOTION_TYPE              = 7,     // WaypointMovementGenerator.h
    POINT_MOTION_TYPE               = 8,     // PointMovementGenerator.h
    FLEEING_MOTION_TYPE             = 9,     // FleeingMovementGenerator.h
    DISTRACT_MOTION_TYPE            = 10,    // IdleMovementGenerator.h
    ASSISTANCE_MOTION_TYPE          = 11,    // PointMovementGenerator.h
    ASSISTANCE_DISTRACT_MOTION_TYPE = 12,    // IdleMovementGenerator.h 
    TIMED_FLEEING_MOTION_TYPE       = 13,    // FleeingMovementGenerator.h 
    FOLLOW_MOTION_TYPE              = 14,    // FollowMovementGenerator.h
    ROTATE_MOTION_TYPE              = 15,    // IdleMovementGenerator.h
    EFFECT_MOTION_TYPE              = 16,
    //TC SPLINE_CHAIN_MOTION_TYPE        = 17,    // SplineChainMovementGenerator.h
    FORMATION_MOTION_TYPE           = 18,    // FormationMovementGenerator.h

    MAX_MOTION_TYPE                          // limit
};

//this determines priority between movement generators
enum MovementGeneratorMode : uint8
{
    MOTION_MODE_DEFAULT = 0,
    MOTION_MODE_OVERRIDE
};

enum MovementGeneratorPriority
{
    MOTION_PRIORITY_NONE = 0,
    MOTION_PRIORITY_NORMAL,
    MOTION_PRIORITY_HIGHEST
};

enum MovementSlot : uint8
{
    MOTION_SLOT_DEFAULT = 0,
    MOTION_SLOT_ACTIVE,
    MAX_MOTION_SLOT
};

enum RotateDirection : uint8
{
    ROTATE_DIRECTION_LEFT          = 0,
    ROTATE_DIRECTION_RIGHT
};

struct TC_GAME_API ChaseRange
{
    //ranges here will be added to combat reach
    ChaseRange(float range);
    ChaseRange(float _minRange, float _maxRange);
    ChaseRange(float _minRange, float _minTolerance, float _maxTolerance, float _maxRange);

    // this contains info that informs how we should path!
    float MinRange;     // we have to move if we are within this range...    (min. attack range)
    float MinTolerance; // ...and if we are, we will move this far away
    float MaxRange;     // we have to move if we are outside this range...   (max. attack range)
    float MaxTolerance; // ...and if we are, we will move into this range

    bool operator==(ChaseRange const& other) const
    {
        return other.MinRange == MinRange && other.MinTolerance == MinTolerance && other.MaxRange == MaxRange && other.MaxTolerance == MaxTolerance;
    }
    bool operator!=(ChaseRange const& other) const
    {
        return !(*this == other);
    }
};

struct TC_GAME_API ChaseAngle
{
    ChaseAngle(float angle, float _tolerance = M_PI_4);

    float RelativeAngle; // we want to be at this angle relative to the target (0 = front, M_PI = back)
    float Tolerance;     // but we'll tolerate anything within +- this much

    float UpperBound() const;
    float LowerBound() const;
    bool IsAngleOkay(float relativeAngle) const;
};

inline bool IsInvalidMovementGeneratorType(uint8 const type) { return type == MAX_DB_MOTION_TYPE || type >= MAX_MOTION_TYPE; }
inline bool IsInvalidMovementSlot(uint8 const slot) { return slot >= MAX_MOTION_SLOT; }

struct FormationMoveSegment
{
    FormationMoveSegment(Position start, Position dest, uint32 moveType, bool hasDestOrientation, float followAngle, float followDist) :
        start(start), dest(dest), moveType(moveType), hasDestOrientation(hasDestOrientation), followAngle(followAngle), followDist(followDist) {}

    Position start;
    Position dest;
    uint32 moveType;
    bool hasDestOrientation;
    float followAngle;
    float followDist;
};

#endif