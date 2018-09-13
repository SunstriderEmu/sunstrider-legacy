
#ifndef TRINITY_MOTIONMASTER_H
#define TRINITY_MOTIONMASTER_H

#include "Common.h"
#include "ObjectGuid.h"
#include "SharedDefines.h"
#include "MovementGenerator.h"
#include "Object.h"
#include "ObjectDefines.h"
#include "Spline/MoveSpline.h"
#include "MovementDefines.h"
#include <deque>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

struct Position;
class MovementGenerator;
class Unit;
class PathGenerator;
class Unit;
struct WaypointPath;
class SplineHandler;
namespace Movement
{
    class MoveSplineInit;
};

enum MotionMasterFlags : uint8
{
    MOTIONMASTER_FLAG_NONE                          = 0x0,
    MOTIONMASTER_FLAG_UPDATE                        = 0x1, // Update in progress
    MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING = 0x2
};

enum MotionMasterDelayedActionType : uint8
{
    MOTIONMASTER_DELAYED_CLEAR = 0,
    MOTIONMASTER_DELAYED_CLEAR_SLOT,
    MOTIONMASTER_DELAYED_CLEAR_MODE,
    MOTIONMASTER_DELAYED_CLEAR_PRIORITY,
    MOTIONMASTER_DELAYED_ADD,
    MOTIONMASTER_DELAYED_REMOVE,
    MOTIONMASTER_DELAYED_REMOVE_TYPE,
    MOTIONMASTER_DELAYED_INITIALIZE
};

struct MovementGeneratorDeleter
{
    void operator()(MovementGenerator* a);
};

struct MovementGeneratorComparator
{
public:
    bool operator()(MovementGenerator const* a, MovementGenerator const* b) const;
};

struct MovementGeneratorInformation
{
    MovementGeneratorInformation(MovementGeneratorType type, ObjectGuid targetGUID, std::string const& targetName);

    MovementGeneratorType Type;
    ObjectGuid TargetGUID;
    std::string TargetName;
};

class MotionMasterDelayedAction
{
public:
    explicit MotionMasterDelayedAction(std::function<void()>&& action, MotionMasterDelayedActionType type) : Action(std::move(action)), Type(type) { }
    ~MotionMasterDelayedAction() { }

    void Resolve() { Action(); }

    std::function<void()> Action;
    uint8 Type;
};

class TC_GAME_API MotionMaster
{
    friend SplineHandler;
    public:
        explicit MotionMaster(Unit* unit);
        ~MotionMaster();

        void Initialize();
        void InitializeDefault();

        bool Empty() const;
        uint32 Size() const;

        std::vector<MovementGeneratorInformation> GetMovementGeneratorsInformation() const;
        MovementSlot GetCurrentSlot() const;
        MovementGenerator* GetCurrentMovementGenerator() const;
        MovementGeneratorType GetCurrentMovementGeneratorType() const;
        MovementGeneratorType GetCurrentMovementGeneratorType(MovementSlot slot) const;
        MovementGenerator* GetCurrentMovementGenerator(MovementSlot slot) const;
        // Returns first found MovementGenerator that matches the given criteria
        MovementGenerator* GetMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot = MOTION_SLOT_ACTIVE) const;
        bool HasMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot = MOTION_SLOT_ACTIVE) const;

  //      void DirectDelete(_Ty curr, bool premature = false);
  //      void DelayedDelete(_Ty curr, bool premature = false);
		//void ClearExpireList();

        void Update(uint32 diff);
        void Add(MovementGenerator* movement, MovementSlot slot = MOTION_SLOT_ACTIVE);
        // NOTE: MOTION_SLOT_DEFAULT will be autofilled with IDLE_MOTION_TYPE
        void Remove(MovementGenerator* movement, MovementSlot slot = MOTION_SLOT_ACTIVE);
        // Removes first found movement
        // NOTE: MOTION_SLOT_DEFAULT will be autofilled with IDLE_MOTION_TYPE
        void Remove(MovementGeneratorType type, MovementSlot slot = MOTION_SLOT_ACTIVE);
        // NOTE: NOTE: MOTION_SLOT_DEFAULT wont be affected
        void Clear();
        // Removes all movements for the given MovementSlot
        // NOTE: MOTION_SLOT_DEFAULT will be autofilled with IDLE_MOTION_TYPE
        void Clear(MovementSlot slot);
        // Removes all movements with the given MovementGeneratorMode
        // NOTE: MOTION_SLOT_DEFAULT wont be affected
        void Clear(MovementGeneratorMode mode);
        // Removes all movements with the given MovementGeneratorPriority
        // NOTE: MOTION_SLOT_DEFAULT wont be affected
        void Clear(MovementGeneratorPriority priority);

        void PropagateSpeedChange();
        //sunwell? reset all movement generators
        void ReinitializeMovement();
        bool GetDestination(float &x, float &y, float &z);

        void MoveIdle();
        void MoveTargetedHome();
        void MoveRandom(float spawndist = 0.0f);
        void MoveFollow(Unit* target, float dist, ChaseAngle angle, MovementSlot slot = MOTION_SLOT_ACTIVE);
        void MoveChase(Unit* target, Optional<ChaseRange> dist = {}, Optional<ChaseAngle> angle = {});
        void MoveChase(Unit* target, float dist, float angle = 0.0f);
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
        void MoveDistract(uint32 time, float orientation);
        //see WaypointMovementGenerator for info about smoothSpline
        void MovePath(uint32 path_id, bool repeatable = true, bool smoothSpline = false);
        //see WaypointMovementGenerator for info about smoothSpline
        void MovePath(WaypointPath& path, bool repeatable = true, bool smoothSpline = false);
        void MoveRotate(uint32 id, uint32 time, RotateDirection direction);
        /** Look towards the target for given time */
        void MoveStealthAlert(Unit const* target, uint32 time);

        uint32 GetCurrentSplineId() const; // sunwell: Escort system

        void LaunchMoveSpline(Movement::MoveSplineInit&& init, uint32 id = 0, MovementGeneratorPriority priority = MOTION_PRIORITY_NORMAL, MovementGeneratorType type = EFFECT_MOTION_TYPE);
    private:
        typedef std::unique_ptr<MovementGenerator, MovementGeneratorDeleter> MovementGeneratorPointer;
        typedef std::multiset<MovementGenerator*, MovementGeneratorComparator> MotionMasterContainer;
        typedef std::unordered_multimap<uint32, MovementGenerator const*> MotionMasterUnitStatesContainer;

        void AddFlag(uint8 const flag) { _flags |= flag; }
        bool HasFlag(uint8 const flag) const { return (_flags & flag) != 0; }
        void RemoveFlag(uint8 const flag) { _flags &= ~flag; }

        void Pop(bool active, bool movementInform);
        void DirectInitialize();
        void DirectClear();
        void DirectClearDefault();
        void DirectClear(std::function<bool(MovementGenerator*)> const& filter);
        void DirectAdd(MovementGenerator* movement, MovementSlot slot);

        void Delete(MovementGenerator* movement, bool active, bool movementInform);
        void DeleteDefault(bool active, bool movementInform);
        void AddBaseUnitState(MovementGenerator const* movement);
        void ClearBaseUnitState(MovementGenerator const* movement);
        void ClearBaseUnitStates();

        Unit* _owner;
        MovementGeneratorPointer _defaultGenerator;
        MotionMasterContainer _generators;
        MotionMasterUnitStatesContainer _baseUnitStatesMap;
        std::deque<MotionMasterDelayedAction> _delayedActions;
        uint8 _flags;
};
#endif
