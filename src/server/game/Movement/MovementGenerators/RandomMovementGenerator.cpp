
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
    i_nextMoveTime(0), wander_distance(spawn_dist) { }

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
        return;
    }

    float respX, respY, respZ, respO, destX, destY, destZ, travelDistZ;
    owner->GetHomePosition(respX, respY, respZ, respO);
    Map const* map = owner->GetBaseMap();

    // For 2D/3D system selection
    //bool is_land_ok  = creature.CanWalk();                // not used?
    //bool is_water_ok = creature.CanSwim();                // not used?
    bool is_air_ok = owner->CanFly();

    const float angle = float(rand_norm()) * static_cast<float>(M_PI*2.0f);
    const float range = float(rand_norm()) * wander_distance;
    const float distanceX = range * std::cos(angle);
    const float distanceY = range * std::sin(angle);

    destX = respX + distanceX;
    destY = respY + distanceY;

    // prevent invalid coordinates generation
    Trinity::NormalizeMapCoord(destX);
    Trinity::NormalizeMapCoord(destY);

    travelDistZ = distanceX*distanceX + distanceY*distanceY;

    if (is_air_ok)                                          // 3D system above ground and above water (flying mode)
    {
        // Limit height change
        const float distanceZ = float(rand_norm()) * sqrtf(travelDistZ)/2.0f;
        destZ = respZ + distanceZ;
        float levelZ = map->GetWaterOrGroundLevel(owner->GetPhaseMask(), destX, destY, destZ-2.0f);

        float diff = destZ - levelZ;
        // Problem here, we must fly above the ground and water, not under. Let's try on next tick
        if (diff < 0)
            return;
        else if (diff < 2.5f)
            destZ += 2.5f - diff; //just go a little higher, we don't want to brush the ground with our wings
    }
    //else if (is_water_ok)                                 // 3D system under water and above ground (swimming mode)
    else                                                    // 2D only
    {
        // 10.0 is the max that vmap high can check (MAX_CAN_FALL_DISTANCE)
        travelDistZ = travelDistZ >= 100.0f ? 10.0f : sqrtf(travelDistZ);

        // The fastest way to get an accurate result 90% of the time.
        // Better result can be obtained like 99% accuracy with a ray light, but the cost is too high and the code is too long.
        destZ = map->GetHeight(owner->GetPhaseMask(), destX, destY, respZ+travelDistZ-2.0f, false);

        if (fabs(destZ - respZ) > travelDistZ)              // Map check
        {
            // Vmap Horizontal or above
            destZ = map->GetHeight(owner->GetPhaseMask(), destX, destY, respZ - 2.0f, true);

            if (fabs(destZ - respZ) > travelDistZ)
            {
                // Vmap Higher
                destZ = map->GetHeight(owner->GetPhaseMask(), destX, destY, respZ+travelDistZ-2.0f, true);

                // let's forget this bad coords where a z cannot be find and retry at next tick
                if (fabs(destZ - respZ) > travelDistZ)
                    return;
            }
        }
    }

    if (!is_air_ok && roll_chance_i(50))
        i_nextMoveTime.Reset(urand(5000, 10000));
    else
        i_nextMoveTime.Reset(urand(50, 400));
    
    owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Movement::MoveSplineInit init(owner);
    init.MoveTo(destX, destY, destZ);
    init.SetWalk(true);
    if(is_air_ok)
        init.SetFly();
    init.Launch();

    //Call for creature group update
    owner->SignalFormationMovement(Position(destX, destY, destZ));
}

template<>
bool RandomMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED | MOVEMENTGENERATOR_FLAG_TRANSITORY);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->IsAlive())
        return false;

    if (!wander_distance)
        wander_distance = owner->GetRespawnRadius();

    owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);
    SetRandomLocation(owner);
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
        i_nextMoveTime.Reset(0);  // Expire the timer
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }
    else
        RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

    i_nextMoveTime.Update(diff);
    if ((HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (i_nextMoveTime.Passed() && owner->movespline->Finalized()))
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
