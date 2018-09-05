
#include "Creature.h"
#include "CreatureAI.h"
#include "MapManager.h"
#include "FleeingMovementGenerator.h"
#include "PathGenerator.h"
#include "ObjectAccessor.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"
#include "Management/VMapFactory.h"

#define MIN_QUIET_DISTANCE 28.0f
#define MAX_QUIET_DISTANCE 43.0f

template<class T>
FleeingMovementGenerator<T>::FleeingMovementGenerator(ObjectGuid fleeTargetGUID) :
    MovementGeneratorMedium<T, FleeingMovementGenerator<T>>(MOTION_MODE_DEFAULT, MOTION_PRIORITY_HIGHEST, UNIT_STATE_FLEEING),
    _fleeTargetGUID(fleeTargetGUID), i_nextCheckTime(0) 
{ }

template<class T>
MovementGeneratorType FleeingMovementGenerator<T>::GetMovementGeneratorType() const
{
    return FLEEING_MOTION_TYPE;
}

template<class T>
void FleeingMovementGenerator<T>::SetTargetLocation(T* owner)
{
    if (!owner)
        return;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        owner->StopMoving();
        _path = nullptr;
        return;
    }

    owner->AddUnitState(UNIT_STATE_FLEEING_MOVE);

    Position destination = owner->GetPosition();
    GetPoint(owner, destination);

    // Add LOS check for target point
    if (!owner->IsWithinLOS(destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ()))
    {
        i_nextCheckTime.Reset(200);
        return;
    }

    if (!_path)
    {
        _path = std::make_unique<PathGenerator>(owner);
        _path->SetPathLengthLimit(30.0f);
        _path->ExcludeSteepSlopes();
    }
    Transport* ownerTransport = owner->GetTransport();
    _path->SetTransport(ownerTransport);

    bool result = _path->CalculatePath(destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());
    if (!result || (_path->GetPathType() & PATHFIND_NOPATH))
    {
        i_nextCheckTime.Reset(100);
        return;
    }

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(_path->GetPath(), 0, ownerTransport);
    init.SetWalk(false);
    int32 traveltime = init.Launch();
    i_nextCheckTime.Reset(traveltime + urand(800, 1500));

    //TC_LOG_TRACE("misc", "FleeingMovementGenerator<T>::SetTargetLocation pos (%f %f %f)", destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ());
}

template<class T>
void FleeingMovementGenerator<T>::GetPoint(T* owner, Position &position)
{
    float casterDistance, casterAngle;
    if (Unit* fleeTarget = ObjectAccessor::GetUnit(*owner, _fleeTargetGUID))
    {
        casterDistance = fleeTarget->GetDistance(owner);
        if (casterDistance > 0.2f)
            casterAngle = fleeTarget->GetAbsoluteAngle(owner);
        else
            casterAngle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
    }
    else
    {
        casterDistance = 0.0f;
        casterAngle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
    }

    float distance, angle;
    if (casterDistance < MIN_QUIET_DISTANCE)
    {
        distance = frand(0.4f, 1.3f)*(MIN_QUIET_DISTANCE - casterDistance);
        angle = casterAngle + frand(-static_cast<float>(M_PI) / 8.0f, static_cast<float>(M_PI) / 8.0f);
    }
    else if (casterDistance > MAX_QUIET_DISTANCE)
    {
        distance = frand(0.4f, 1.0f)*(MAX_QUIET_DISTANCE - MIN_QUIET_DISTANCE);
        angle = -casterAngle + frand(-static_cast<float>(M_PI) / 4.0f, static_cast<float>(M_PI) / 4.0f);
    }
    else    // we are inside quiet range
    {
        distance = frand(0.6f, 1.2f)*(MAX_QUIET_DISTANCE - MIN_QUIET_DISTANCE);
        angle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
    }

    position = owner->GetFirstWalkableCollisionPosition(distance, angle);
}

template<class T>
bool FleeingMovementGenerator<T>::DoInitialize(T* owner)
{
    MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED | MOVEMENTGENERATOR_FLAG_TRANSITORY);
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->IsAlive())
        return false;

    // TODO: UNIT_FIELD_FLAGS should not be handled by generators
    owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
    SetTargetLocation(owner);
    _path = nullptr;
    return true;
}

template<class T>
void FleeingMovementGenerator<T>::DoReset(T* owner)
{
    MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    DoInitialize(owner);
}

template<class T>
bool FleeingMovementGenerator<T>::DoUpdate(T* owner, uint32 time_diff)
{
    if (!owner || !owner->IsAlive())
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        _path = nullptr;
        return true;
    }
    else
        MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

    i_nextCheckTime.Update(time_diff);
    if ((MovementGenerator::HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (i_nextCheckTime.Passed() && owner->movespline->Finalized()))
    {
        MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);
        SetTargetLocation(owner);
    }

    return true;
}

template<class T>
void FleeingMovementGenerator<T>::DoDeactivate(T* owner)
{
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_FLEEING_MOVE);
}

template<class T>
void FleeingMovementGenerator<T>::DoFinalize(T*, bool, bool)
{
}

template<>
void FleeingMovementGenerator<Player>::DoFinalize(Player* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
        owner->ClearUnitState(UNIT_STATE_FLEEING_MOVE);
        owner->StopMoving();
    }
}

template<>
void FleeingMovementGenerator<Creature>::DoFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
        owner->ClearUnitState(UNIT_STATE_FLEEING_MOVE);
        if (owner->GetVictim())
            owner->SetTarget(owner->EnsureVictim()->GetGUID());
    }
}

template FleeingMovementGenerator<Player>::FleeingMovementGenerator(ObjectGuid);
template FleeingMovementGenerator<Creature>::FleeingMovementGenerator(ObjectGuid);
template MovementGeneratorType FleeingMovementGenerator<Player>::GetMovementGeneratorType() const;
template MovementGeneratorType FleeingMovementGenerator<Creature>::GetMovementGeneratorType() const;
template bool FleeingMovementGenerator<Player>::DoInitialize(Player*);
template bool FleeingMovementGenerator<Creature>::DoInitialize(Creature*);
template void FleeingMovementGenerator<Player>::GetPoint(Player*, Position&);
template void FleeingMovementGenerator<Creature>::GetPoint(Creature*, Position&);
template void FleeingMovementGenerator<Player>::SetTargetLocation(Player*);
template void FleeingMovementGenerator<Creature>::SetTargetLocation(Creature*);
template void FleeingMovementGenerator<Player>::DoReset(Player*);
template void FleeingMovementGenerator<Creature>::DoReset(Creature*);
template bool FleeingMovementGenerator<Player>::DoUpdate(Player*, uint32);
template bool FleeingMovementGenerator<Creature>::DoUpdate(Creature*, uint32);
template void FleeingMovementGenerator<Player>::DoDeactivate(Player*);
template void FleeingMovementGenerator<Creature>::DoDeactivate(Creature*);

//---- TimedFleeingMovementGenerator

bool TimedFleeingMovementGenerator::Update(Unit* owner, uint32 diff)
{
    if (!owner || !owner->IsAlive())
        return false;

    _totalFleeTime.Update(diff);
    if (_totalFleeTime.Passed())
        return false;

    return FleeingMovementGenerator<Creature>::DoUpdate(owner->ToCreature(), diff);
}

void TimedFleeingMovementGenerator::Finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (!active)
        return;

    owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
    owner->ClearUnitState(UNIT_STATE_FLEEING | UNIT_STATE_FLEEING_MOVE);
    owner->StopMoving();

    if (Unit* victim = owner->GetVictim())
    {
        if (owner->IsAlive())
        {
            owner->AttackStop();
            owner->ToCreature()->AI()->AttackStart(victim);
        }
    }
}

MovementGeneratorType TimedFleeingMovementGenerator::GetMovementGeneratorType() const
{
    return TIMED_FLEEING_MOTION_TYPE;
}
