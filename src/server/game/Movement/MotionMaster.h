
#ifndef TRINITY_MOTIONMASTER_H
#define TRINITY_MOTIONMASTER_H

#include "Common.h"
#include "SharedDefines.h"
#include "Object.h"
#include "ObjectDefines.h"
#include "Spline/MoveSpline.h"
#include "MovementDefines.h"
#include <vector>

struct Position;
class MovementGenerator;
class Unit;
class PathGenerator;
struct WaypointPath;
class SplineHandler;

class TC_GAME_API MotionMaster
{
    friend SplineHandler;
    private:
        typedef MovementGenerator* _Ty;

        void pop();

        bool NeedInitTop() const;
        void InitTop();
    public:

        explicit MotionMaster(Unit* unit) : _top(-1), _owner(unit), _cleanFlag(MOTIONMMASTER_CLEANFLAG_NONE)
        {
            for (uint8 i = 0; i < MAX_MOTION_SLOT; ++i)
            {
                _slot[i] = nullptr;
                _needInit[i] = true;
            }
        }
        ~MotionMaster();

        void Initialize();
        void InitDefault();

        bool empty() const { return (_top < 0); }
        int size() const { return _top + 1; }
        MovementGenerator* top() const;

        MovementGeneratorType GetCurrentMovementGeneratorType() const;
        MovementGeneratorType GetMotionSlotType(MovementSlot slot) const;
        MovementGenerator* GetMotionSlot(MovementSlot slot) const;

        void DirectDelete(_Ty curr, bool premature = false);
        void DelayedDelete(_Ty curr, bool premature = false);
		void ClearExpireList();

        void UpdateMotion(uint32 diff);
		void Clear(bool reset = true);
		void MovementExpired(bool reset = true);

        void MoveIdle();
        void MoveTargetedHome();
        void MoveRandom(float spawndist = 0.0f);
        void MoveFollow(Unit* target, float dist, ChaseAngle angle, MovementSlot slot = MOTION_SLOT_ACTIVE);
        void MoveChase(Unit* target, Optional<ChaseRange> dist = {}, Optional<ChaseAngle> angle = {});
        void MoveChase(Unit* target, float dist, float angle = 0.0f) { MoveChase(target, Optional<ChaseRange>(dist), Optional<ChaseAngle>(angle)); }
        void MoveConfused();
        void MoveFleeing(Unit* enemy, uint32 time = 0);
        void MovePoint(uint32 id, Position const& pos, bool generatePath = true, Optional<float> finalOrient = {}, bool forceDestination = true);
        void MovePoint(uint32 id, float x, float y, float z, bool generatePath = true, Optional<float> finalOrient = {}, bool forceDestination = true);
      
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
#ifdef LICH_KING
        void MoveJumpTo(float angle, float speedXY, float speedZ);
        void MoveJump(Position const& pos, float speedXY, float speedZ, uint32 id = EVENT_JUMP, bool hasOrientation = false);
        void MoveJump(float x, float y, float z, float o, float speedXY, float speedZ, uint32 id = EVENT_JUMP, bool hasOrientation = false);
#endif
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

        uint32 GetCurrentSplineId() const; // sunwell: Escort system

        void PropagateSpeedChange();
        void ReinitializeMovement();

        bool GetDestination(float &x, float &y, float &z);
    private:
        typedef std::vector<MovementGenerator*> MovementList;

        bool Mutate(MovementGenerator *m, MovementSlot slot);                  // use Move* functions instead

        void DirectClean(bool reset);
        void DelayedClean();

        void DirectExpire(bool reset, bool premature = true);
        void DelayedExpire(bool premature = true);

        MovementList _expireList;
        MovementGenerator* _slot[MAX_MOTION_SLOT];
        int _top;
        Unit* _owner;
        bool _needInit[MAX_MOTION_SLOT];
        uint8 _cleanFlag;
};
#endif
