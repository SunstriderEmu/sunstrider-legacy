
#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "WorldPacket.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "G3DPosition.hpp"


template<class T>
HomeMovementGenerator<T>::HomeMovementGenerator() : 
    MovementGeneratorMedium<T, HomeMovementGenerator<T>>(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING)
{
}

template HomeMovementGenerator<Creature>::HomeMovementGenerator();

template<class T>
MovementGeneratorType HomeMovementGenerator<T>::GetMovementGeneratorType() const
{
    return HOME_MOTION_TYPE;
}

template MovementGeneratorType HomeMovementGenerator<Creature>::GetMovementGeneratorType() const;

template<>
void HomeMovementGenerator<Creature>::SetTargetLocation(Creature* owner)
{
    // if we are ROOT/STUNNED/DISTRACTED even after aura clear, finalize on next update - otherwise we would get stuck in evade
    if (owner->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        return;
    }

    owner->ClearUnitState(UNIT_STATE_ALL_ERASABLE & ~UNIT_STATE_EVADE);
    owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Position destination = owner->GetHomePosition();
    Movement::MoveSplineInit init(owner);
    // TODO: maybe this never worked, who knows, top is always this generator, so this code calls GetResetPosition on itself
    //float x, y, z, o;
    //// at apply we can select more nice return points base at current movegen
    //if (owner->GetMotionMaster()->empty() || !owner->GetMotionMaster()->top()->GetResetPosition(owner, x, y, z))
    //{
    //    owner->GetHomePosition(x, y, z, o);
    //    init.SetFacing(o);
    //}

    //sun: keep z if its really close... that way we can keep creatures at their exact DB position if it didn't changed. Else there are some imprecisions that could lead to feets in the ground, etc.
    float oldZ = destination.m_positionZ;
    owner->UpdateAllowedPositionZ(destination.m_positionX, destination.m_positionY, destination.m_positionZ);
    if (oldZ - destination.m_positionZ < 2.0f)
        destination.m_positionZ = oldZ;
    init.MoveTo(PositionToVector3(destination));
    init.SetFacing(destination.GetOrientation());
    init.SetWalk(false);
    init.Launch();
}

template<>
bool HomeMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    SetTargetLocation(owner);
    return true;
}

template<>
void HomeMovementGenerator<Creature>::DoReset(Creature* owner)
{ 
    RemoveFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    DoInitialize(owner);
}

template<>
bool HomeMovementGenerator<Creature>::DoUpdate(Creature* owner, const uint32 /*time_diff*/)
{
    if (HasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED) || owner->movespline->Finalized())
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }
    return true;
}

template<class T>
void HomeMovementGenerator<T>::DoDeactivate(T*) { }

template<>
void HomeMovementGenerator<Creature>::DoDeactivate(Creature* owner)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void HomeMovementGenerator<T>::DoFinalize(T*, bool, bool) { }

template<>
void HomeMovementGenerator<Creature>::DoFinalize(Creature* owner, bool active, bool movementInform)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE | UNIT_STATE_EVADE);
    
    if (movementInform && HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
    {
        owner->SetWalk(true);
        owner->InitCreatureAddon(true);
        owner->SetSpawnHealth();
        owner->AI()->JustReachedHome();
    }
}

template<class T>
void HomeMovementGenerator<T>::SetTargetLocation(T*) { }

template<class T>
bool HomeMovementGenerator<T>::DoUpdate(T*, uint32)
{
    return false;
}
