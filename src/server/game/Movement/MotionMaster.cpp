
#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"
#include "G3DPosition.hpp"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include <cassert>

#include "ChaseMovementGenerator.h"
#include "ConfusedMovementGenerator.h"
#include "DistractMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "FlightPathMovementGenerator.h"
#include "FollowMovementGenerator.h"
#include "FormationMovementGenerator.h"
#include "GenericMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "RotateMovementGenerator.h"
#include "WaypointMovementGenerator.h"

inline MovementGenerator* GetIdleMovementGenerator()
{
    return sMovementGeneratorRegistry->GetRegistryItem(IDLE_MOTION_TYPE)->Create();
}

inline bool IsStatic(MovementGenerator* movement)
{
    return (movement == GetIdleMovementGenerator());
}

inline void MovementGeneratorPointerDeleter(MovementGenerator* a)
{
    if (a != nullptr && !IsStatic(a))
        delete a;
}

void MovementGeneratorDeleter::operator()(MovementGenerator * a)
{
    MovementGeneratorPointerDeleter(a);
}

bool MovementGeneratorComparator::operator()(MovementGenerator const* a, MovementGenerator const* b) const
{
    if (a->Mode > b->Mode)
        return true;
    else if (a->Mode == b->Mode)
        return a->Priority > b->Priority;

    return false;
}

MovementGeneratorInformation::MovementGeneratorInformation(MovementGeneratorType type, ObjectGuid targetGUID, std::string const& targetName) : Type(type), TargetGUID(targetGUID), TargetName(targetName) { }

MotionMaster::MotionMaster(Unit* unit) : _owner(unit), _defaultGenerator(nullptr), _flags(MOTIONMASTER_FLAG_NONE) { }

MotionMaster::~MotionMaster()
{
    _delayedActions.clear();

    for (auto itr = _generators.begin(); itr != _generators.end(); itr = _generators.erase(itr))
        MovementGeneratorPointerDeleter(*itr);
}

void MotionMaster::Initialize()
{
    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this]()
        {
            Initialize();
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_INITIALIZE);
        return;
    }

    DirectInitialize();
}

// set new default movement generator
void MotionMaster::InitializeDefault()
{
    Add(FactorySelector::SelectMovementGenerator(_owner), MOTION_SLOT_DEFAULT);
}

bool MotionMaster::Empty() const
{
    return !_defaultGenerator && _generators.empty();
}

uint32 MotionMaster::Size() const
{
    return _defaultGenerator ? 1 : 0 + uint32(_generators.size());
}


std::vector<MovementGeneratorInformation> MotionMaster::GetMovementGeneratorsInformation() const
{
    std::vector<MovementGeneratorInformation> list;

    if (_defaultGenerator)
        list.emplace_back(_defaultGenerator->GetMovementGeneratorType(), ObjectGuid::Empty, std::string());

    for (auto itr = _generators.begin(); itr != _generators.end(); ++itr)
    {
        MovementGenerator* movement = *itr;
        MovementGeneratorType const type = movement->GetMovementGeneratorType();
        switch (type)
        {
        case CHASE_MOTION_TYPE:
        case FOLLOW_MOTION_TYPE:
            if (AbstractFollower* followInformation = dynamic_cast<AbstractFollower*>(movement))
            {
                if (Unit* target = followInformation->GetTarget())
                    list.emplace_back(type, target->GetGUID(), target->GetName());
                else
                    list.emplace_back(type, ObjectGuid::Empty, std::string());
            }
            else
                list.emplace_back(type, ObjectGuid::Empty, std::string());
            break;
        default:
            list.emplace_back(type, ObjectGuid::Empty, std::string());
            break;
        }
    }

    return list;
}

MovementSlot MotionMaster::GetCurrentSlot() const
{
    if (!_generators.empty())
        return MOTION_SLOT_ACTIVE;

    if (_defaultGenerator)
        return MOTION_SLOT_DEFAULT;

    return MAX_MOTION_SLOT;
}

MovementGenerator* MotionMaster::GetCurrentMovementGenerator() const
{
    if (!_generators.empty())
        return *_generators.begin();

    if (_defaultGenerator)
        return _defaultGenerator.get();

    return nullptr;
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
    if (Empty())
        return MAX_MOTION_TYPE;

    MovementGenerator const* movement = GetCurrentMovementGenerator();
    if (!movement)
        return MAX_MOTION_TYPE;

    return movement->GetMovementGeneratorType();
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType(MovementSlot slot) const
{
    if (Empty() || IsInvalidMovementSlot(slot))
        return MAX_MOTION_TYPE;

    if (slot == MOTION_SLOT_ACTIVE && !_generators.empty())
        return (*_generators.begin())->GetMovementGeneratorType();

    if (slot == MOTION_SLOT_DEFAULT && _defaultGenerator)
        return _defaultGenerator->GetMovementGeneratorType();

    return MAX_MOTION_TYPE;
}

MovementGenerator* MotionMaster::GetCurrentMovementGenerator(MovementSlot slot) const
{
    if (Empty() || IsInvalidMovementSlot(slot))
        return nullptr;

    if (slot == MOTION_SLOT_ACTIVE && !_generators.empty())
        return *_generators.begin();

    if (slot == MOTION_SLOT_DEFAULT && _defaultGenerator)
        return _defaultGenerator.get();

    return nullptr;
}

MovementGenerator* MotionMaster::GetMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot) const
{
    if (Empty() || IsInvalidMovementSlot(slot))
        return nullptr;

    MovementGenerator* movement = nullptr;
    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && filter(_defaultGenerator.get()))
            movement = _defaultGenerator.get();
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = std::find_if(_generators.begin(), _generators.end(), std::ref(filter));
            if (itr != _generators.end())
                movement = *itr;
        }
        break;
    default:
        break;
    }

    return movement;
}

bool MotionMaster::HasMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot) const
{
    if (Empty() || IsInvalidMovementSlot(slot))
        return false;

    bool value = false;
    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && filter(_defaultGenerator.get()))
            value = true;
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = std::find_if(_generators.begin(), _generators.end(), std::ref(filter));
            value = itr != _generators.end();
        }
        break;
    default:
        break;
    }

    return value;
}

void MotionMaster::Update(uint32 diff)
{
    if (!_owner)
        return;

    ASSERT(!Empty(), "MotionMaster:Update: update called without Initializing! (%s)", _owner->GetGUID().ToString().c_str());

    AddFlag(MOTIONMASTER_FLAG_UPDATE);

    MovementGenerator* top = GetCurrentMovementGenerator();
    if (HasFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING) && IsStatic(top))
    {
        RemoveFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING);
        top->Initialize(_owner);
    }
    if (top->HasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING))
        top->Initialize(_owner);
    if (top->HasFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED))
        top->Reset(_owner);

    ASSERT(!top->HasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED), "MotionMaster:Update: update called on an uninitialized top! (%s) (type: %u, flags: %u)", _owner->GetGUID().ToString().c_str(), top->GetMovementGeneratorType(), top->Flags);

    if (!top->Update(_owner, diff))
    {
        ASSERT(top == GetCurrentMovementGenerator(), "MotionMaster::Update: top was modified while updating! (%s)", _owner->GetGUID().ToString().c_str());

        //sun: In current TC implementation, we can't return false in update for generators with MOTION_MODE_DEFAULT else we would crash here
        //little edit here to fix that, because our WaypointMovementGenerator CAN return false on failure
        if (GetCurrentSlot() == MOTION_SLOT_ACTIVE)
            Pop(true, true); // Natural, and only, call to MovementInform
        else
            MoveIdle();

        /* TC
        // Since all the actions that modify any slot are delayed, this movement is guaranteed to be top
        //Pop(true, true); // Natural, and only, call to MovementInform
        */
    }

    RemoveFlag(MOTIONMASTER_FLAG_UPDATE);

    while (!_delayedActions.empty())
    {
        _delayedActions.front().Resolve();
        _delayedActions.pop_front();
    }
}

void MotionMaster::Add(MovementGenerator* movement, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    if (!movement)
        return;

    if (IsInvalidMovementSlot(slot))
    {
        delete movement;
        return;
    }

    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this, movement, slot]()
        {
            Add(movement, slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_ADD);
    }
    else
        DirectAdd(movement, slot);
}

void MotionMaster::Remove(MovementGenerator* movement, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    if (!movement || IsInvalidMovementSlot(slot))
        return;

    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this, movement, slot]()
        {
            Remove(movement, slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_REMOVE);
        return;
    }

    if (Empty())
        return;

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && _defaultGenerator.get() == movement)
            DirectClearDefault();
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = _generators.find(movement);
            if (itr != _generators.end())
            {
                MovementGenerator* pointer = *itr;
                bool const top = GetCurrentMovementGenerator() == pointer;
                _generators.erase(pointer);
                Delete(pointer, top, false);
            }
        }
        break;
    default:
        break;
    }
}

void MotionMaster::Remove(MovementGeneratorType type, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    if (IsInvalidMovementGeneratorType(type) || IsInvalidMovementSlot(slot))
        return;

    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this, type, slot]()
        {
            Remove(type, slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_REMOVE_TYPE);
        return;
    }

    if (Empty())
        return;

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && _defaultGenerator->GetMovementGeneratorType() == type)
            DirectClearDefault();
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = std::find_if(_generators.begin(), _generators.end(), [type](MovementGenerator const* a) -> bool
            {
                return a->GetMovementGeneratorType() == type;
            });

            if (itr != _generators.end())
            {
                MovementGenerator* pointer = *itr;
                bool const top = GetCurrentMovementGenerator() == pointer;
                _generators.erase(pointer);
                Delete(pointer, top, false);
            }
        }
        break;
    default:
        break;
    }
}

void MotionMaster::Clear()
{
    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this]()
        {
            Clear();
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR);
        return;
    }

    if (!Empty())
        DirectClear();
}

void MotionMaster::Clear(MovementSlot slot)
{
    if (IsInvalidMovementSlot(slot))
        return;

    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this, slot]()
        {
            Clear(slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR_SLOT);
        return;
    }

    if (Empty())
        return;

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        DirectClearDefault();
        break;
    case MOTION_SLOT_ACTIVE:
        DirectClear();
        break;
    default:
        break;
    }
}

void MotionMaster::Clear(MovementGeneratorMode mode)
{
    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this, mode]()
        {
            Clear(mode);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR_MODE);
        return;
    }

    if (Empty())
        return;

    std::function<bool(MovementGenerator*)> criteria = [mode](MovementGenerator* a) -> bool
    {
        return a->Mode == mode;
    };
    DirectClear(criteria);
}

void MotionMaster::Clear(MovementGeneratorPriority priority)
{
    if (HasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        std::function<void()> action = [this, priority]()
        {
            Clear(priority);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR_PRIORITY);
        return;
    }

    if (Empty())
        return;

    std::function<bool(MovementGenerator*)> criteria = [priority](MovementGenerator* a) -> bool
    {
        return a->Priority == priority;
    };
    DirectClear(criteria);
}

void MotionMaster::PropagateSpeedChange()
{
    if (Empty())
        return;

    MovementGenerator* movement = GetCurrentMovementGenerator();
    if (!movement)
        return;

    movement->UnitSpeedChanged();
}

void MotionMaster::ReinitializeMovement()
{
    if (Empty())
        return;

    MovementGenerator* movement = GetCurrentMovementGenerator();
    if (!movement)
        return;

    movement->Reset(_owner);
}

bool MotionMaster::GetDestination(float &x, float &y, float &z)
{
    if (_owner->movespline->Finalized())
        return false;

    G3D::Vector3 const& dest = _owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}

void MotionMaster::MoveIdle()
{
    Add(GetIdleMovementGenerator(), MOTION_SLOT_DEFAULT);
}

void MotionMaster::MoveTargetedHome()
{
    Creature* owner = _owner->ToCreature();
    if (!owner)
    {
        TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveTargetedHome: '%s', attempted to move towards target home.", _owner->GetGUID().ToString().c_str());
        return;
    }

    Clear();

    Unit* target = owner->GetCharmerOrOwner();
    if (!target)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTargetedHome: '%s', targeted home.", _owner->GetGUID().ToString().c_str());
        Add(new HomeMovementGenerator<Creature>());
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTargetedHome: '%s', starts following '%s'", _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
        Add(new FollowMovementGenerator(target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE));
    }
}

void MotionMaster::MoveRandom(float spawndist)
{
    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveRandom: '%s', started random movement (spawnDist: %f)", _owner->GetGUID().ToString().c_str(), spawndist);
        Add(new RandomMovementGenerator<Creature>(spawndist), MOTION_SLOT_DEFAULT);
    }
}

void MotionMaster::MoveFollow(Unit* target, float dist, ChaseAngle angle, MovementSlot slot)
{
    // Ignore movement request if target not exist
    if (!target || target == _owner)
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveFollow: '%s', starts following '%s'", _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
    Add(new FollowMovementGenerator(target, dist, angle), slot);
}

void MotionMaster::MoveChase(Unit* target, Optional<ChaseRange> dist, Optional<ChaseAngle> angle, bool run /* = true*/)
{
    // Ignore movement request if target not exist
    if (!target || target == _owner)
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveChase: '%s', starts chasing '%s'", _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
    Add(new ChaseMovementGenerator(target, dist, angle, run));
}

void MotionMaster::MoveConfused()
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveConfused: '%s', started confused movement.", _owner->GetGUID().ToString().c_str());
        Add(new ConfusedMovementGenerator<Player>());
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveConfused: '%s', started confused movement.", _owner->GetGUID().ToString().c_str());
        Add(new ConfusedMovementGenerator<Creature>());
    }
}

void MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveFleeing: '%s', flees from '%s' (time: %u)", _owner->GetGUID().ToString().c_str(), enemy->GetGUID().ToString().c_str(), time);
    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        if (time)
            Add(new TimedFleeingMovementGenerator(enemy->GetGUID(), time));
        else
            Add(new FleeingMovementGenerator<Creature>(enemy->GetGUID()));
    }
    else
        Add(new FleeingMovementGenerator<Player>(enemy->GetGUID()));
}

void MotionMaster::MovePoint(uint32 id, Position const& pos, bool generatePath/* = true*/, Optional<float> finalOrient/* = {}*/, bool forceDestination /*= true*/)
{
    MovePoint(id, pos.m_positionX, pos.m_positionY, pos.m_positionZ, generatePath, finalOrient, forceDestination);
}

void MotionMaster::MovePoint(uint32 id, float x, float y, float z, bool generatePath, Optional<float> finalOrient, bool forceDestination)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePoint: '%s', targeted point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        Add(new PointMovementGenerator<Player>(id, x, y, z, generatePath, 0.0f, finalOrient), MOTION_SLOT_ACTIVE);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePoint: '%s', targeted point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        Add(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, 0.0f, finalOrient), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveCloserAndStop(uint32 id, Unit* target, float distance)
{
    float distanceToTravel = _owner->GetExactDist2d(target) - distance;
    if (distanceToTravel > 0.0f)
    {
        float angle = _owner->GetAbsoluteAngle(target);
        float destx = _owner->GetPositionX() + distanceToTravel * std::cos(angle);
        float desty = _owner->GetPositionY() + distanceToTravel * std::sin(angle);
        MovePoint(id, destx, desty, target->GetPositionZ());
    } else
    {
        // we are already close enough. We just need to turn toward the target without changing position.
        Movement::MoveSplineInit init(_owner);
        init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ());
        init.SetFacing(target);
        Add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id));
    }
}

void MotionMaster::MoveLand(uint32 id, Position const& pos)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveLand: '%s', landing point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(PositionToVector3(pos));
#ifdef LICH_KING
    init.SetAnimation(Movement::ToGround);
#endif
    Add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id));
}

void MotionMaster::MoveTakeoff(uint32 id, Position const& pos)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTakeoff: '%s', landing point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(PositionToVector3(pos));
#ifdef LICH_KING
    init.SetAnimation(Movement::ToFly);
#endif
    Add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id));
}

void MotionMaster::MoveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ)
{
//BC creatures can't be knocked back
#ifdef LICH_KING
    // this function may make players fall below map
    if (_owner->GetTypeId() == TYPEID_PLAYER)
        return;

    if (speedXY <= 0.1f)
        return;

    float x, y, z;
    float moveTimeHalf = speedZ / Movement::gravity;
    float dist = 2 * moveTimeHalf * speedXY;
    float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);

    _owner->GetNearPoint(_owner, x, y, z, _owner->GetCombatReach(), dist, _owner->GetAbsoluteAngle(srcX, srcY) + M_PI);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
    init.SetParabolic(max_height, 0);
    init.SetOrientationFixed(true);
    init.SetVelocity(speedXY);

    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, 0);
    movement->Priority = MOTION_PRIORITY_HIGHEST;
    Add(movement);
#endif
}

void MotionMaster::MoveFall(uint32 id /*=0*/)
{
    // use larger distance for vmap height search than in most other cases
    float tz = _owner->GetMapHeight(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (tz <= INVALID_HEIGHT)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveFall: '%s', unable to retrieve a proper height at map Id: %u (X: %f, Y: %f, Z: %f)",
            _owner->GetGUID().ToString().c_str(), _owner->GetMap()->GetId(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ());
        return;
    }

    // Abort too if the ground is very near
    if (fabs(_owner->GetPositionZ() - tz) < 0.1f)
        return;

    _owner->AddUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING);
    _owner->m_movementInfo.SetFallTime(0);

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        _owner->ToPlayer()->SetFallInformation(0, _owner->GetPositionZ());
        return;
    }

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), tz, false);
    init.SetFall();


    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id);
    movement->Priority = MOTION_PRIORITY_HIGHEST;
    Add(movement);
}

void MotionMaster::MoveCharge(float x, float y, float z, float speed, uint32 id, bool generatePath)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveCharge: '%s', charging point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        PointMovementGenerator<Player>* movement = new PointMovementGenerator<Player>(id, x, y, z, generatePath, speed);
        movement->Priority = MOTION_PRIORITY_HIGHEST;
        movement->BaseUnitState = UNIT_STATE_CHARGING;
        Add(movement);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveCharge: '%s', charging point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        PointMovementGenerator<Creature>* movement = new PointMovementGenerator<Creature>(id, x, y, z, generatePath, speed);
        movement->Priority = MOTION_PRIORITY_HIGHEST;
        movement->BaseUnitState = UNIT_STATE_CHARGING;
        Add(movement);
    }
}

void MotionMaster::MoveCharge(PathGenerator const& path, float speed /*= SPEED_CHARGE*/, Unit* target /* = nullptr */)
{
    G3D::Vector3 dest = path.GetActualEndPosition();

    MoveCharge(dest.x, dest.y, dest.z, speed, EVENT_CHARGE_PREPATH);

    // Charge movement is not started when using EVENT_CHARGE_PREPATH
    Movement::MoveSplineInit init(_owner);
    init.MovebyPath(path.GetPath());
    init.SetVelocity(speed);
    if(target) //sun: charge should also turn player to target (may have taken a not straight path)
        init.SetFacing(target);
    init.Launch();
}

void MotionMaster::MoveSeekAssistance(float x, float y, float z)
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveSeekAssistance: '%s', seeks assistance (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), x, y, z);

        _owner->AttackStop();
        _owner->CastStop();
        _owner->ToCreature()->SetReactState(REACT_PASSIVE);
        Add(new AssistanceMovementGenerator(EVENT_ASSIST_MOVE, x, y, z));
    } 
    else
    {
        TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveSeekAssistance: '%s', attempted to seek assistance.", _owner->GetGUID().ToString().c_str());
    }
}

void MotionMaster::MoveSeekAssistanceDistract(uint32 time)
{
    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveSeekAssistanceDistract: '%s', is distracted after assistance call (Time: %u)", _owner->GetGUID().ToString().c_str(), time);
        Add(new AssistanceDistractMovementGenerator(_owner, _owner->GetOrientation(), time));
    }
    else
    {
        TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveSeekAssistanceDistract: '%s', attempted to call distract assistance.", _owner->GetGUID().ToString().c_str());
    }
}

void MotionMaster::MoveTaxiFlight(uint32 path, uint32 pathnode)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (path < sTaxiPathNodesByPath.size())
        {
            TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTaxiFlight: '%s', taxi to path Id: %u (node %u)", _owner->GetGUID().ToString().c_str(), path, pathnode);

            // Only one FLIGHT_MOTION_TYPE is allowed
            bool hasExisting = HasMovementGenerator([](MovementGenerator const* gen) { return gen->GetMovementGeneratorType() == FLIGHT_MOTION_TYPE; });
            ASSERT(!hasExisting, "Duplicate flight path movement generator");

            FlightPathMovementGenerator* movement = new FlightPathMovementGenerator(pathnode);
            movement->LoadPath(_owner->ToPlayer());
            Add(movement);
        }
        else
        {
            TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveTaxiFlight: '%s', attempted taxi to non-existing path Id: %u (node: %u)", _owner->GetGUID().ToString().c_str(), path, pathnode);
        }
    }
    else
    {
        TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveTaxiFlight: '%s', attempted taxi to path Id: %u (node: %u)", _owner->GetGUID().ToString().c_str(), path, pathnode);
    }
}

void MotionMaster::MoveStealthAlert(Unit const* target, uint32 timer)
{
    TC_LOG_TRACE("movement.motionmaster", "MotionMaster::MoveStealthAlert: Creature (Entry: %u GUID: '%s') almost detected target '%s' (timer: %u)",
        _owner->GetEntry(), _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str(), timer);

    // Face the unit (stealthed player) and set distracted state for 5 seconds
    MoveDistract(timer, _owner->GetAbsoluteAngle(target));
}

void MotionMaster::MoveDistract(uint32 timer, float orientation)
{
    /*
    if (_slot[MOTION_SLOT_CONTROLLED])
    return;
    */
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveDistract: '%s', distracted (timer: %u, orientation: %f)", _owner->GetGUID().ToString().c_str(), timer, orientation);
    Add(new DistractMovementGenerator(_owner, orientation, timer));
}

void MotionMaster::MovePath(uint32 pathId, bool repeatable, bool smoothSpline)
{
    if (!pathId)
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePath: '%s', starts moving over path Id: %u (repeatable: %s, smooth: %u)", _owner->GetGUID().ToString().c_str(), pathId, repeatable ? "YES" : "NO", uint32(smoothSpline));
    Add(new WaypointMovementGenerator<Creature>(pathId, repeatable, smoothSpline), MOTION_SLOT_DEFAULT);
}

void MotionMaster::MovePath(WaypointPath& path, bool repeatable, bool smoothSpline)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePath: '%s', starts moving over path Id: %u (repeatable: %s, smooth: %u)", _owner->GetGUID().ToString().c_str(), path.id, repeatable ? "YES" : "NO", uint32(smoothSpline));
    //We set waypoint movement as new default movement generator. If non repeating case, the generator will trigger a MoveIdle itself at the end
    Add(new WaypointMovementGenerator<Creature>(path, repeatable, smoothSpline), MOTION_SLOT_DEFAULT);
}

void MotionMaster::MoveRotate(uint32 id, uint32 time, RotateDirection direction)
{
    if (!time)
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveRotate: '%s', starts rotate (time: %u, direction: %u)", _owner->GetGUID().ToString().c_str(), time, direction);
    Add(new RotateMovementGenerator(id, time, direction), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveFormation(uint32 id, FormationMoveSegment path, Creature* leader)
{
    if (_owner->GetTypeId() != TYPEID_UNIT)
        return;

    Add(new FormationMovementGenerator(id, leader->GetGUID(), path));
}

void MotionMaster::LaunchMoveSpline(Movement::MoveSplineInit&& init, uint32 id/*= 0*/, MovementGeneratorPriority priority/* = MOTION_PRIORITY_NORMAL*/, MovementGeneratorType type/*= EFFECT_MOTION_TYPE*/)
{
    if (IsInvalidMovementGeneratorType(type))
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::LaunchMoveSpline: '%s', tried to launch a spline with an invalid MovementGeneratorType: %u (Id: %u, Priority: %u)", _owner->GetGUID().ToString().c_str(), type, id, priority);
        return;
    }

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::LaunchMoveSpline: '%s', initiates spline Id: %u (Type: %u, Priority: %u)", _owner->GetGUID().ToString().c_str(), id, type, priority);

    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), type, id);
    movement->Priority = priority;
    Add(movement);
}

/******************** Private methods ********************/

void MotionMaster::Pop(bool active, bool movementInform)
{
    MovementGenerator* pointer = *_generators.begin();
    _generators.erase(pointer);
    Delete(pointer, active, movementInform);
}

void MotionMaster::DirectInitialize()
{
    // Clear ALL movement generators (including default)
    DirectClearDefault();
    DirectClear();

    InitializeDefault();
}

void MotionMaster::DirectClear()
{
    // First delete Top
    if (!_generators.empty())
        Pop(true, false);

    // Then the rest
    while (!_generators.empty())
        Pop(false, false);

    // Make sure the storage is empty
    ClearBaseUnitStates();
}

void MotionMaster::DirectClearDefault()
{
    if (_defaultGenerator)
        DeleteDefault(_generators.empty(), false);
}

void MotionMaster::DirectClear(std::function<bool(MovementGenerator*)> const& filter)
{
    if (_generators.empty())
        return;

    MovementGenerator const* top = GetCurrentMovementGenerator();
    for (auto itr = _generators.begin(); itr != _generators.end();)
    {
        if (filter(*itr))
        {
            MovementGenerator* movement = *itr;
            itr = _generators.erase(itr);
            Delete(movement, movement == top, false);
        }
        else
            ++itr;
    }
}

void MotionMaster::DirectAdd(MovementGenerator* movement, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    /*
    if (MovementGenerator* curr = _slot[slot])
    {
    _slot[slot] = nullptr; // in case a new one is generated in this slot during directdelete
    if (_top == slot && (_cleanFlag & MOTIONMMASTER_CLEANFLAG_UPDATE))
    DelayedDelete(curr);
    else
    DirectDelete(curr);
    }
    else if (_top < slot)
    {
    _top = slot;
    }

    _slot[slot] = m;
    if (_top > slot)
    _initialize[slot] = true;
    else
    {
    _initialize[slot] = false;
    m->Initialize(_owner);
    }
    */

    /*
    * NOTE: This mimics old behaviour: only one MOTION_SLOT_IDLE, MOTION_SLOT_ACTIVE, MOTION_SLOT_CONTROLLED
    * On future changes support for multiple will be added
    */

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator)
            _defaultGenerator->Finalize(_owner, _generators.empty(), false);

        _defaultGenerator = MovementGeneratorPointer(movement);
        if (IsStatic(movement))
            AddFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING);
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            if (movement->Priority >= (*_generators.begin())->Priority)
            {
                MovementGenerator* pointer = *_generators.begin();
                if (movement->Priority == pointer->Priority)
                {
                    _generators.erase(pointer);
                    Delete(pointer, true, false);
                }
                else
                    pointer->Deactivate(_owner);
            }
            else
            {
                auto itr = std::find_if(_generators.begin(), _generators.end(), [movement](MovementGenerator const* a) -> bool
                {
                    return a->Priority == movement->Priority;
                });

                if (itr != _generators.end())
                {
                    MovementGenerator* pointer = *itr;
                    _generators.erase(pointer);
                    Delete(pointer, false, false);
                }
            }
        }
        else
            _defaultGenerator->Deactivate(_owner);

        _generators.emplace(movement);
        AddBaseUnitState(movement);
        break;
    default:
        break;
    }
}

void MotionMaster::Delete(MovementGenerator* movement, bool active, bool movementInform)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::Delete: deleting generator (Priority: %u, Flags: %u, BaseUnitState: %u, Type: %u), owner: '%s'",
        movement->Priority, movement->Flags, movement->BaseUnitState, movement->GetMovementGeneratorType(), _owner->GetGUID().ToString().c_str());

    movement->Finalize(_owner, active, movementInform);
    ClearBaseUnitState(movement);
    MovementGeneratorPointerDeleter(movement);
}

void MotionMaster::DeleteDefault(bool active, bool movementInform)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::DeleteDefault: deleting generator (Priority: %u, Flags: %u, BaseUnitState: %u, Type: %u), owner: '%s'",
        _defaultGenerator->Priority, _defaultGenerator->Flags, _defaultGenerator->BaseUnitState, _defaultGenerator->GetMovementGeneratorType(), _owner->GetGUID().ToString().c_str());

    _defaultGenerator->Finalize(_owner, active, movementInform);
    _defaultGenerator = MovementGeneratorPointer(GetIdleMovementGenerator());
    AddFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING);
}

void MotionMaster::AddBaseUnitState(MovementGenerator const* movement)
{
    if (!movement || !movement->BaseUnitState)
        return;

    _baseUnitStatesMap.emplace(movement->BaseUnitState, movement);
    _owner->AddUnitState(movement->BaseUnitState);
}

void MotionMaster::ClearBaseUnitState(MovementGenerator const* movement)
{
    if (!movement || !movement->BaseUnitState)
        return;

    Trinity::Containers::MultimapErasePair(_baseUnitStatesMap, movement->BaseUnitState, movement);
    if (_baseUnitStatesMap.count(movement->BaseUnitState) == 0)
        _owner->ClearUnitState(movement->BaseUnitState);
}

void MotionMaster::ClearBaseUnitStates()
{
    uint32 unitState = 0;
    for (auto itr = _baseUnitStatesMap.begin(); itr != _baseUnitStatesMap.end(); ++itr)
        unitState |= itr->first;

    _owner->ClearUnitState(unitState);
    _baseUnitStatesMap.clear();
}
