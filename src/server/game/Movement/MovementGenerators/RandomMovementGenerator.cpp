
#include "Creature.h"
#include "MapManager.h"
#include "RandomMovementGenerator.h"
#include "ObjectAccessor.h"
#include "Map.h"
#include "Util.h"
#include "CreatureGroups.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "MovementDefines.h"

#define RUNNING_CHANCE_RANDOMMV 20                                  //will be "1 / RUNNING_CHANCE_RANDOMMV"

template<class T>
RandomMovementGenerator<T>::RandomMovementGenerator(float spawn_dist /*= 0.0f*/) 
    : MovementGeneratorMedium<T, RandomMovementGenerator<T>>(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    _timer(0), _wanderDistance(spawn_dist), _reference()
{ 
}

template RandomMovementGenerator<Creature>::RandomMovementGenerator(float/* distance*/);

template<class T>
MovementGeneratorType RandomMovementGenerator<T>::GetMovementGeneratorType() const
{
    return RANDOM_MOTION_TYPE;
}

template MovementGeneratorType RandomMovementGenerator<Creature>::GetMovementGeneratorType() const;

template<class T>
void RandomMovementGenerator<T>::SetRandomLocation(T*) { }

template<>
void RandomMovementGenerator<Creature>::SetRandomLocation(Creature* owner)
{
    if (!owner)
        return;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE | UNIT_STATE_LOST_CONTROL) || owner->IsMovementPreventedByCasting())
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        _path = nullptr;
        return;
    }

    Position position(_reference);
    float distance = frand(0.f, 1.f) * _wanderDistance;
    float angle = frand(0.f, 1.f) * float(M_PI) * 2.f;
    owner->MovePositionToFirstCollision(position, distance, angle);

    if (!_path)
    {
        _path = std::make_unique<PathGenerator>(owner);
        _path->ExcludeSteepSlopes();
        _path->SetPathLengthLimit(_wanderDistance * 1.5f);
    }

    bool result = _path->CalculatePath(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());
    if (!result || (_path->GetPathType() & PATHFIND_NOPATH))
    {
        _timer.Reset(100);
        return;
    }
    
    owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(_path->GetPath());
    init.SetWalk(true);
    if(owner->IsFlying())
        init.SetFly();

    uint32 travelTime = init.Launch();
    uint32 resetTimer = roll_chance_i(50) ? urand(5000, 10000) : urand(1000, 2000);
    _timer.Reset(travelTime + resetTimer);

    //Call for creature group update
    owner->SignalFormationMovement(position);
}

template<>
bool RandomMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED | MOVEMENTGENERATOR_FLAG_TRANSITORY);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->IsAlive())
        return false;

    _reference = owner->GetPosition();

    if (!_wanderDistance)
        _wanderDistance = owner->GetRespawnRadius();

    _timer.Reset(0);
    _path = nullptr;
    return true;
}

template<class T>
void RandomMovementGenerator<T>::DoReset(T*) { }

template<>
void RandomMovementGenerator<Creature>::DoReset(Creature* creature)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    DoInitialize(creature);
}

template<class T>
bool RandomMovementGenerator<T>::DoUpdate(T*, uint32)
{
    return false;
}

template<>
bool RandomMovementGenerator<Creature>::DoUpdate(Creature* owner, const uint32 diff)
{
    if (!owner || !owner->IsAlive())
        return true;

    if (owner->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        _timer.Reset(0);  // Expire the timer
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }
    else
        RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

    _timer.Update(diff);
    if ((HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (_timer.Passed() && owner->movespline->Finalized()))
    {
        RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);
        SetRandomLocation(owner);
    }
    return true;
}

template<class T>
void RandomMovementGenerator<T>::DoDeactivate(T*) { }

template<>
void RandomMovementGenerator<Creature>::DoDeactivate(Creature* owner)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void RandomMovementGenerator<T>::DoFinalize(T*, bool, bool) { }

template<>
void RandomMovementGenerator<Creature>::DoFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        owner->StopMoving();

        // TODO: Research if this modification is needed, which most likely isnt
        owner->SetWalk(false);
    }
}
