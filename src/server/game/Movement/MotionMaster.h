
#ifndef TRINITY_MOTIONMASTER_H
#define TRINITY_MOTIONMASTER_H

#include <vector>
#include "SharedDefines.h"
#include "Object.h"
#include "Spline/MoveSpline.h"
#include "Position.h"

class MovementGenerator;
class Unit;
class PathGenerator;
struct WaypointPath;
class SplineHandler;

// Creature Entry ID used for waypoints show, visible only for GMs
#define VISUAL_WAYPOINT 1

// values 0 ... MAX_DB_MOTION_TYPE-1 used in DB
enum MovementGeneratorType
{
    IDLE_MOTION_TYPE          = 0,                              // IdleMovementGenerator.h
    RANDOM_MOTION_TYPE        = 1,                              // RandomMovementGenerator.h
    WAYPOINT_MOTION_TYPE      = 2,                              // WaypointMovementGenerator.h
    MAX_DB_MOTION_TYPE        = 3,                              // *** this and below motion types can't be set in DB.
    ANIMAL_RANDOM_MOTION_TYPE = 3,                              // AnimalRandomMovementGenerator.h
    CONFUSED_MOTION_TYPE      = 4,                              // ConfusedMovementGenerator.h
    CHASE_MOTION_TYPE         = 5,                              // TargetedMovementGenerator.h
    HOME_MOTION_TYPE          = 6,                              // HomeMovementGenerator.h
    FLIGHT_MOTION_TYPE        = 7,                              // WaypointMovementGenerator.h
    POINT_MOTION_TYPE         = 8,                              // PointMovementGenerator.h
    FLEEING_MOTION_TYPE       = 9,                              // FleeingMovementGenerator.h
    DISTRACT_MOTION_TYPE      = 10,                             // IdleMovementGenerator.h
    ASSISTANCE_MOTION_TYPE    = 11,                             // PointMovementGenerator.h (first part of flee for assistance)
    ASSISTANCE_DISTRACT_MOTION_TYPE = 12,                       // IdleMovementGenerator.h (second part of flee for assistance)
    TIMED_FLEEING_MOTION_TYPE = 13,                             // FleeingMovementGenerator.h (alt.second part of flee for assistance)
    FOLLOW_MOTION_TYPE        = 14,
    ROTATE_MOTION_TYPE        = 15,
    EFFECT_MOTION_TYPE        = 16,
    STEALTH_WARN_MOTION_TYPE  = 17,                             // StealthWarnMovementGenerator.h TODO, précédence sur les autres générateurs ?
    NULL_MOTION_TYPE          = 19
};

//this determines priority between movement generators
enum MovementSlot
{
    MOTION_SLOT_IDLE, //Default movement, is never deleted unless replaced by another
    MOTION_SLOT_ACTIVE,
    MOTION_SLOT_CONTROLLED, 
    MAX_MOTION_SLOT
};

enum MMCleanFlag
{
    MMCF_NONE   = 0,
    MMCF_UPDATE = 1, // Clear or Expire called from update
    MMCF_RESET  = 2  // Flag if need top()->Reset()
};

enum RotateDirection
{
    ROTATE_DIRECTION_LEFT,
    ROTATE_DIRECTION_RIGHT
};

// assume it is 25 yard per 0.6 second
#define SPEED_CHARGE    42.0f

class TC_GAME_API MotionMaster
{
    friend SplineHandler;
    private:
        //typedef std::stack<MovementGenerator *> Impl;
        typedef MovementGenerator* _Ty;

        void pop()
        {
            if (empty())
                return;

            Impl[_top] = nullptr;
            while (!empty() && !top())
                --_top;
        }
        void push(_Ty _Val) { ++_top; Impl[_top] = _Val; }

        bool needInitTop() const
        {
            if (empty())
                return false;
            return _needInit[_top];
        }
        void InitTop();
    public:

        explicit MotionMaster(Unit* unit) : _top(-1), _owner(unit), _cleanFlag(MMCF_NONE)
        {
            for (uint8 i = 0; i < MAX_MOTION_SLOT; ++i)
            {
                Impl[i] = nullptr;
                _needInit[i] = true;
            }
        }
        ~MotionMaster();

        void Initialize();
        void InitDefault();

        bool empty() const { return (_top < 0); }
        int size() const { return _top + 1; }
        _Ty top() const
        {
            ASSERT(!empty());
            return Impl[_top];
        }
        _Ty GetMotionSlot(int slot) const
        {
            ASSERT(slot >= 0);
            return Impl[slot];
        }

        void DirectDelete(_Ty curr, bool premature = false);
        void DelayedDelete(_Ty curr, bool premature = false);
		void ClearExpireList();

        void UpdateMotion(uint32 diff);
		void Clear(bool reset = true);
		void MovementExpired(bool reset = true);

        void MoveIdle();
        void MoveTargetedHome();
        void MoveRandom(float spawndist = 0.0f);
        void MoveFollow(Unit* target, float dist, float angle, MovementSlot slot = MOTION_SLOT_ACTIVE);
        void MoveChase(Unit* target, float dist = 0.0f, float angle = 0.0f);
        void MoveConfused();
        void MoveFleeing(Unit* enemy, uint32 time = 0);
        void MovePoint(uint32 id, Position const& pos, bool generatePath = true, bool forceDestination = true)
            { MovePoint(id, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation, generatePath, forceDestination); }
        //orientation = 0 will be ignored, use near 0 values if you want to do it
        void MovePoint(uint32 id, float x, float y, float z, float o = 0.0f, bool generatePath = true, bool forceDestination = true);
      
        /*  Makes the unit move toward the target until it is at a certain distance from it. The unit then stops.
                   Only works in 2D.
                   This method doesn't account for any movement done by the target. in other words, it only works if the target is stationary.
        */
        void MoveCloserAndStop(uint32 id, Unit* target, float distance);

        // These two movement types should only be used with creatures having landing/takeoff animations
        void MoveLand(uint32 id, Position const& pos);
        void MoveTakeoff(uint32 id, Position const& pos);

        void MoveCharge(float x, float y, float z, float speed = SPEED_CHARGE, uint32 id = EVENT_CHARGE, bool generatePath = false);
        void MoveCharge(PathGenerator const& path, float speed = SPEED_CHARGE, Unit* target = nullptr);
        void MoveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ);
        void MoveJumpTo(float angle, float speedXY, float speedZ);
        void MoveJump(Position const& pos, float speedXY, float speedZ, uint32 id = EVENT_JUMP)
            { MoveJump(pos.m_positionX, pos.m_positionY, pos.m_positionZ, speedXY, speedZ, id); };
        void MoveJump(float x, float y, float z, float speedXY, float speedZ, uint32 id = EVENT_JUMP);
        void MoveFall(uint32 id = 0);

        void MoveSeekAssistance(float x, float y, float z);
        void MoveSeekAssistanceDistract(uint32 timer);
        void MoveTaxiFlight(uint32 path, uint32 pathnode);
        void MoveDistract(float x, float y, uint32 time);
        //see WaypointMovementGenerator for info about smoothSpline
        void MovePath(uint32 path_id, bool repeatable = true, bool smoothSpline = false);
        //see WaypointMovementGenerator for info about smoothSpline
        void MovePath(WaypointPath& path, bool repeatable = true, bool smoothSpline = false);
        void MoveRotate(uint32 time, RotateDirection direction);
        /** Look towards the target for given time */
        void MoveStealthAlert(Unit const* target, uint32 time);

        MovementGeneratorType GetCurrentMovementGeneratorType() const;
        MovementGeneratorType GetMotionSlotType(int slot) const;
        uint32 GetCurrentSplineId() const; // sunwell: Escort system

        void PropagateSpeedChange();
        void ReinitializeMovement();

        bool GetDestination(float &x, float &y, float &z);
    private:
        bool Mutate(MovementGenerator *m, MovementSlot slot);                  // use Move* functions instead

        void DirectClean(bool reset);
        void DelayedClean();

        void DirectExpire(bool reset, bool premature = true);
        void DelayedExpire(bool premature = true);

        typedef std::vector<_Ty> MovementList;
        MovementList _expireList;
        _Ty Impl[MAX_MOTION_SLOT];
        int _top;
        Unit* _owner;
        bool _needInit[MAX_MOTION_SLOT];
        uint8 _cleanFlag;
};
#endif
