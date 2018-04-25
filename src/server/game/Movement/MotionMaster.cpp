
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
#include "FollowMovementGenerator.h"
#include "GenericMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "RotateMovementGenerator.h"
#include "StealthAlertMovementGenerator.h"
#include "WaypointMovementGenerator.h"

inline MovementGenerator* GetIdleMovementGenerator()
{
    return sMovementGeneratorRegistry->GetRegistryItem(IDLE_MOTION_TYPE)->Create();
}

inline bool isStatic(MovementGenerator* movement)
{
    return (movement == GetIdleMovementGenerator());
}

MovementGenerator* MotionMaster::top() const
{
    ASSERT(!empty());

    return _slot[_top];
}

void MotionMaster::Initialize()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator *movement = top();
        pop();
        if (movement)
            DirectDelete(movement, true);
    }

    while (!_expireList.empty())
    {
        delete _expireList.back();
        _expireList.pop_back();
    }

    InitDefault();
}

// set new default movement generator
void MotionMaster::InitDefault()
{
    Mutate(FactorySelector::SelectMovementGenerator(_owner), MOTION_SLOT_IDLE);
}

MotionMaster::~MotionMaster()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator *curr = top();
        pop();
        if (curr && !isStatic(curr))
            delete curr;    // Skip finalizing on delete, it might launch new movement
    }
}

void MotionMaster::UpdateMotion(uint32 diff)
{
    if (!_owner)
        return;

    ASSERT(!empty());

    _cleanFlag |= MOTIONMMASTER_CLEANFLAG_UPDATE;
    if (!top()->Update(_owner, diff))
    {
        _cleanFlag &= ~MOTIONMMASTER_CLEANFLAG_UPDATE;
        /* sunstrider: tweaked this, a movement generator in the idle telling us it's done can only mean two things atm :
        - Waypoint generator has an invalid path 
        - Waypoint generator has finished its path (not repeating case). (TC handle this case by having the generator stalling)
        In both case, we want to end the generator. In both case restarting the waypoint generator does not make sense.
        */
        if (_top != MOTION_SLOT_IDLE)
            MovementExpired();
        else
            MoveIdle();
    } else
        _cleanFlag &= ~MOTIONMMASTER_CLEANFLAG_UPDATE;

    if (!_expireList.empty())
        ClearExpireList();
}

void MotionMaster::Clear(bool reset /* = true*/)
{
    if (_cleanFlag & MOTIONMMASTER_CLEANFLAG_UPDATE)
    {
        if (reset)
            _cleanFlag |= MOTIONMMASTER_CLEANFLAG_RESET;
        else
            _cleanFlag &= ~MOTIONMMASTER_CLEANFLAG_RESET;
        DelayedClean();
    }
    else
        DirectClean(reset);
}

void MotionMaster::MovementExpired(bool reset /* = true*/)
{
    if (_cleanFlag & MOTIONMMASTER_CLEANFLAG_UPDATE)
    {
        if (reset)
            _cleanFlag |= MOTIONMMASTER_CLEANFLAG_RESET;
        else
            _cleanFlag &= ~MOTIONMMASTER_CLEANFLAG_RESET;
        DelayedExpire(false);
    }
    else
        DirectExpire(reset, false);
}

void MotionMaster::ClearExpireList()
{
    for (auto itr : _expireList)
        DirectDelete(itr);

    _expireList.clear();

    if (empty())
        Initialize();
    else if (NeedInitTop())
        InitTop();
    else if (_cleanFlag & MOTIONMMASTER_CLEANFLAG_RESET)
        top()->Reset(_owner);

    _cleanFlag &= ~MOTIONMMASTER_CLEANFLAG_RESET;
}

void MotionMaster::DirectClean(bool reset)
{
    while (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        if (curr) DirectDelete(curr, true);
    }

    if (empty())
        return;

    if (NeedInitTop())
        InitTop();
    else if (reset)
        top()->Reset(_owner);
}

void MotionMaster::DelayedClean()
{
    while (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        if (curr)
            DelayedDelete(curr);
    }
}

//delete top movement generator if any, and re init top
void MotionMaster::DirectExpire(bool reset, bool premature)
{
    if (size() > 1)
    {
        int previousTop = _top;
        MovementGenerator *curr = top();
        ASSERT(curr != nullptr);
        pop();
        DirectDelete(curr, premature);

        /* sunstrider: There is one case when whe shouldn't reset: if a new movement generator was created (in scripts) during the delete,
        it was just initialized and we don't want to reset it. We can detect that's the case if _top has not decreased.
        Any case this may be wrong?    */
        if (reset && _top >= previousTop)
            reset = false;
    }

    //set the new top
    while (!empty() && !top())
        --_top;

    if (empty())
        Initialize();
    else if (NeedInitTop())
        InitTop();
    else if (reset)
    {
        top()->Reset(_owner);
    }
}

void MotionMaster::DelayedExpire(bool premature)
{
    if (size() > 1)
    {
        MovementGenerator *curr = top();
        ASSERT(curr != nullptr);
        pop();
        DelayedDelete(curr, premature);
    }

    while (!empty() && !top())
        --_top;
}

void MotionMaster::MoveIdle()
{
    if(!empty())
        MovementExpired(false);
    //! Should be preceded by MovementExpired or Clear if there's an overlying movementgenerator active
    if (empty() || !isStatic(top()))
        Mutate(GetIdleMovementGenerator(), MOTION_SLOT_IDLE);
}

void MotionMaster::MoveRandom(float spawndist)
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveRandom: '%s', started random movement (spawnDist: %f)", _owner->GetGUID().ToString().c_str(), spawndist);
        Mutate(new RandomMovementGenerator<Creature>(spawndist), MOTION_SLOT_IDLE);
    }
}

void MotionMaster::MoveTargetedHome()
{
    Creature* owner = _owner->ToCreature();
    if (!owner)
    {
        TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveTargetedHome: '%s', attempted to move towards target home.", _owner->GetGUID().ToString().c_str());
        return;
    }

    Clear(false);

    Unit* target = owner->GetCharmerOrOwner();
    if (!target)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTargetedHome: '%s', targeted home.", _owner->GetGUID().ToString().c_str());
        Mutate(new HomeMovementGenerator<Creature>(), MOTION_SLOT_ACTIVE);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTargetedHome: '%s', starts following '%s'", _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
        Mutate(new FollowMovementGenerator(target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveConfused()
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveConfused: '%s', started confused movement.", _owner->GetGUID().ToString().c_str());
        Mutate(new ConfusedMovementGenerator<Player>(), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveConfused: '%s', started confused movement.", _owner->GetGUID().ToString().c_str());
        Mutate(new ConfusedMovementGenerator<Creature>(), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveChase(Unit* target, Optional<ChaseRange> dist, Optional<ChaseAngle> angle)
{
    // ignore movement request if target not exist
    if (!target || target == _owner || _owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveChase: '%s', starts chasing '%s'", _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
    Mutate(new ChaseMovementGenerator(target, dist, angle), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveFollow(Unit* target, float dist, ChaseAngle angle, MovementSlot slot)
{
    // ignore movement request if target not exist
    if (!target || target == _owner || _owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveFollow: '%s', starts following '%s'", _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
    Mutate(new FollowMovementGenerator(target, dist, angle), slot);
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
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath, 0.0f, finalOrient), MOTION_SLOT_ACTIVE);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePoint: '%s', targeted point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, 0.0f, finalOrient), MOTION_SLOT_ACTIVE);
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
        Mutate(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id), MOTION_SLOT_ACTIVE);
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
    Mutate(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveTakeoff(uint32 id, Position const& pos)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTakeoff: '%s', landing point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(PositionToVector3(pos));
#ifdef LICH_KING
    init.SetAnimation(Movement::ToFly);
#endif
    Mutate(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id), MOTION_SLOT_ACTIVE);
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

    Mutate(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, 0), MOTION_SLOT_CONTROLLED);
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
    Mutate(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveCharge(float x, float y, float z, float speed, uint32 id, bool generatePath)
{
    if (_slot[MOTION_SLOT_CONTROLLED] && _slot[MOTION_SLOT_CONTROLLED]->GetMovementGeneratorType() != DISTRACT_MOTION_TYPE)
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveCharge: '%s', charging point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath, speed), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveCharge: '%s', charging point Id: %u (X: %f, Y: %f, Z: %f)", _owner->GetGUID().ToString().c_str(), id, x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, speed), MOTION_SLOT_CONTROLLED);
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
    if(target)
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
        Mutate(new AssistanceMovementGenerator(x, y, z), MOTION_SLOT_ACTIVE);
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
        Mutate(new AssistanceDistractMovementGenerator(time), MOTION_SLOT_ACTIVE);
    }
    else
    {
        TC_LOG_ERROR("movement.motionmaster", "MotionMaster::MoveSeekAssistanceDistract: '%s', attempted to call distract assistance.", _owner->GetGUID().ToString().c_str());
    }
}

void MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    if (_owner->HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveFleeing: '%s', flees from '%s' (time: %u)", _owner->GetGUID().ToString().c_str(), enemy->GetGUID().ToString().c_str(), time);
    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        if (time)
            Mutate(new TimedFleeingMovementGenerator(enemy->GetGUID(), time), MOTION_SLOT_CONTROLLED);
        else
            Mutate(new FleeingMovementGenerator<Creature>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
    else
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveTaxiFlight(uint32 path, uint32 pathnode)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (path < sTaxiPathNodesByPath.size())
        {
            TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveTaxiFlight: '%s', taxi to path Id: %u (node %u)", _owner->GetGUID().ToString().c_str(), path, pathnode);
            FlightPathMovementGenerator* movement = new FlightPathMovementGenerator(pathnode);
            movement->LoadPath(_owner->ToPlayer());
            Mutate(movement, MOTION_SLOT_CONTROLLED);
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
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    TC_LOG_TRACE("movement.motionmaster", "MotionMaster::MoveStealthAlert: Creature (Entry: %u GUID: '%s') almost detected target '%s' (timer: %u)",
        _owner->GetEntry(), _owner->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str(), timer);

    Mutate(new StealthAlertMovementGenerator(_owner, target, timer), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveDistract(float x, float y, uint32 timer)
{
    //return if already a generator in MOTION_SLOT_CONTROLLED
    if (_slot[MOTION_SLOT_CONTROLLED])
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveDistract: '%s', distracted (timer: %u)", _owner->GetGUID().ToString().c_str(), timer);
    }
    else
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveDistract: '%s', distracted (timer: %u)", _owner->GetGUID().ToString().c_str(), timer);
    }

    float angle = _owner->GetAbsoluteAngle(x,y);
    Mutate(new DistractMovementGenerator(_owner, angle, timer), MOTION_SLOT_CONTROLLED);
}

bool MotionMaster::Mutate(MovementGenerator *m, MovementSlot slot)
{
    //remove last generator in this slot if any
    if (MovementGenerator *curr = _slot[slot])
    {
        bool delayed = (_top == slot && (_cleanFlag & MOTIONMMASTER_CLEANFLAG_UPDATE));

        _slot[slot] = nullptr; // in case a new one is generated in this slot during directdelete
        //kelno: clear slot AND decrease top immediately to avoid crashes when referencing null top in DirectDelete
        while (!empty() && !top())
            --_top;

        if (delayed)
            DelayedDelete(curr, true);
        else
            DirectDelete(curr, true);
    }

    //set top to new top if needed
    if (_top < slot)
        _top = slot;

    _slot[slot] = m;
    ASSERT(_slot[slot] != nullptr);
    //if generator is new top, init it immediately, else mark as need init
    if (_top > slot)
        _needInit[slot] = true;
    else
    {
        _needInit[slot] = false;
        return m->Initialize(_owner);
    }
    return true;
}

void MotionMaster::MovePath(uint32 pathId, bool repeatable, bool smoothSpline)
{
    if (!pathId)
        return;
    //We set waypoint movement as new default movement generator
    Mutate(new WaypointMovementGenerator<Creature>(pathId, repeatable, smoothSpline), MOTION_SLOT_IDLE);

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePath: '%s', starts moving over path Id: %u (repeatable: %s, smooth: %u)", _owner->GetGUID().ToString().c_str(), pathId, repeatable ? "YES" : "NO", uint32(smoothSpline));
}

void MotionMaster::MovePath(WaypointPath& path, bool repeatable, bool smoothSpline)
{
    //We set waypoint movement as new default movement generator. If non repeating case, the generator will trigger a MoveIdle itself at the end
    Mutate(new WaypointMovementGenerator<Creature>(path, repeatable, smoothSpline), MOTION_SLOT_IDLE);

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MovePath: '%s', starts moving over path Id: %u (repeatable: %s, smooth: %u)", _owner->GetGUID().ToString().c_str(), path.id, repeatable ? "YES" : "NO", uint32(smoothSpline));
}

void MotionMaster::MoveRotate(uint32 time, RotateDirection direction)
{
    if (!time)
        return;

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::MoveRotate: '%s', starts rotate (time: %u, direction: %u)", _owner->GetGUID().ToString().c_str(), time, direction);
    Mutate(new RotateMovementGenerator(time, direction), MOTION_SLOT_ACTIVE);
}

void MotionMaster::LaunchMoveSpline(Movement::MoveSplineInit&& init, uint32 id/*= 0*/, MovementSlot slot/*= MOTION_SLOT_ACTIVE*/, MovementGeneratorType type/*= EFFECT_MOTION_TYPE*/)
{
    if (IsInvalidMovementGeneratorType(type))
    {
        TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::LaunchMoveSpline: '%s', tried to launch a spline with an invalid MovementGeneratorType: %u (Id: %u, Slot: %u)", _owner->GetGUID().ToString().c_str(), type, id, slot);
        return;
    }

    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::LaunchMoveSpline: '%s', initiates spline Id: %u (Type: %u, Slot: %u)", _owner->GetGUID().ToString().c_str(), id, type, slot);
    Mutate(new GenericMovementGenerator(std::move(init), type, id), slot);
}

void MotionMaster::PropagateSpeedChange()
{
    for (int i = 0; i <= _top; ++i)
    {
        if (_slot[i])
            _slot[i]->UnitSpeedChanged();
    }
}

void MotionMaster::ReinitializeMovement()
{
    for (int i = 0; i <= _top; ++i)
    {
        if (_slot[i])
            _slot[i]->Reset(_owner);
    }
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
   if (empty())
       return MAX_MOTION_TYPE;

   MovementGenerator* movement = top();
   if (!movement)
       return MAX_MOTION_TYPE;

   return movement->GetMovementGeneratorType();
}

MovementGeneratorType MotionMaster::GetMotionSlotType(MovementSlot slot) const
{
    if (empty() || IsInvalidMovementSlot(slot) || !_slot[slot])
        return MAX_MOTION_TYPE;
    else
        return _slot[slot]->GetMovementGeneratorType();
}

MovementGenerator* MotionMaster::GetMotionSlot(MovementSlot slot) const
{
    if (empty() || IsInvalidMovementSlot(slot) || !_slot[slot])
        return nullptr;

    return _slot[slot];
}

// sunwell: Escort system
uint32 MotionMaster::GetCurrentSplineId() const
{
    if (empty())
        return 0;

    return top()->GetSplineId();
}

void MotionMaster::InitTop()
{
    top()->Initialize(_owner);
    _needInit[_top] = false;
}

void MotionMaster::DirectDelete(_Ty curr, bool premature)
{
    if (isStatic(curr))
        return;

    curr->Finalize(_owner, premature);
    delete curr;
}

void MotionMaster::DelayedDelete(_Ty curr, bool premature)
{
    TC_LOG_DEBUG("movement.motionmaster", "MotionMaster::DelayedDelete: '%s', delayed deleting movement generator (type: %u)", _owner->GetGUID().ToString().c_str(), curr->GetMovementGeneratorType());
    if (isStatic(curr))
        return;

    _expireList.push_back(curr);
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

void MotionMaster::pop()
{
    if (empty())
        return;

    _slot[_top] = nullptr;
    while (!empty() && !top())
        --_top;
}

bool MotionMaster::NeedInitTop() const
{
    if (empty())
        return false;
    return _needInit[_top];
}