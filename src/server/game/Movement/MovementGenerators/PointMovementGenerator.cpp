
#include "PointMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"
#include "MotionMaster.h"
#include "MovementDefines.h"
#include "CreatureGroups.h"

template<class T>
PointMovementGenerator<T>::PointMovementGenerator(uint32 _id, float _x, float _y, float _z, bool _generatePath, float _speed /*= 0.0f*/, Optional<float> finalOrient /*= {}*/, bool forceDestination /*= false*/)
    : MovementGeneratorMedium<T, PointMovementGenerator<T>>(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    _movementId(_id),
    _destination(_x, _y, _z),
    _finalOrient(finalOrient),
    _speed(_speed),
    _generatePath(_generatePath),
    _forceDestination(forceDestination)
{ }

template<class T>
void PointMovementGenerator<T>::LaunchMove(T* owner)
{
    Movement::MoveSplineInit init(owner);
    init.MoveTo(G3D::Vector3(_destination.GetPositionX(), _destination.GetPositionY(), _destination.GetPositionZ()), _generatePath, _forceDestination);
    if (_speed > 0.0f)
        init.SetVelocity(_speed);

    if (_finalOrient)
        init.SetFacing(*_finalOrient);

    init.Launch();
}

//----- Point Movement Generator
template<class T>
MovementGeneratorType PointMovementGenerator<T>::GetMovementGeneratorType() const
{
    return POINT_MOTION_TYPE;
}

template<class T>
bool PointMovementGenerator<T>::DoInitialize(T* owner)
{
    MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED | MOVEMENTGENERATOR_FLAG_TRANSITORY);
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    owner->AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);

    if (_movementId == EVENT_CHARGE_PREPATH)
    {
        owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    //if cannot move : init generator but don't move for now
    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        return true;
    }

    LaunchMove(owner);
    return true;
}

template<class T>
bool PointMovementGenerator<T>::DoUpdate(T* owner, uint32 /*diff*/)
{
    if (!owner)
        return false;

    if (_movementId == EVENT_CHARGE_PREPATH)
    {
        if (owner->movespline->Finalized())
        {
            MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
            return false;
        }
        return true;
    }

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        return true;
    }

    if ((MovementGenerator::HasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED) && owner->movespline->Finalized()) || (MovementGenerator::HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()))
    {
        MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED | MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING);
        owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);
        LaunchMove(owner);
    }

    if (owner->movespline->Finalized())
    {
        MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);
        MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
    }

    return true;
}

template<class T>
void PointMovementGenerator<T>::DoDeactivate(T* owner)
{
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void PointMovementGenerator<T>::DoFinalize(T* owner, bool active, bool movementInform)
{
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);

    if (movementInform && MovementGenerator::HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
        MovementInform(owner);
}

template<class T>
void PointMovementGenerator<T>::DoReset(T* unit)
{
    MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    if (!unit->IsStopped())
        unit->StopMoving();

    DoInitialize(owner);
}

template<class T>
void PointMovementGenerator<T>::MovementInform(T* /*unit*/) { }

template <> void PointMovementGenerator<Creature>::MovementInform(Creature* unit)
{
    if (unit->AI())
        unit->AI()->MovementInform(POINT_MOTION_TYPE, _movementId);
}

template PointMovementGenerator<Player>::PointMovementGenerator(uint32, float, float, float, bool, float, Optional<float>, bool);
template PointMovementGenerator<Creature>::PointMovementGenerator(uint32, float, float, float, bool, float, Optional<float>, bool);
template MovementGeneratorType PointMovementGenerator<Player>::GetMovementGeneratorType() const;
template MovementGeneratorType PointMovementGenerator<Creature>::GetMovementGeneratorType() const;
template bool PointMovementGenerator<Player>::DoInitialize(Player*);
template bool PointMovementGenerator<Creature>::DoInitialize(Creature*);
template void PointMovementGenerator<Player>::DoFinalize(Player*, bool, bool);
template void PointMovementGenerator<Creature>::DoFinalize(Creature*, bool, bool);
template void PointMovementGenerator<Player>::DoDeactivate(Player*);
template void PointMovementGenerator<Creature>::DoDeactivate(Creature*);
template void PointMovementGenerator<Player>::DoReset(Player*);
template void PointMovementGenerator<Creature>::DoReset(Creature*);
template bool PointMovementGenerator<Player>::DoUpdate(Player*, uint32);
template bool PointMovementGenerator<Creature>::DoUpdate(Creature*, uint32);

//---- AssistanceMovementGenerator
 	 
MovementGeneratorType AssistanceMovementGenerator::GetMovementGeneratorType() const
{
    return ASSISTANCE_MOTION_TYPE;
}

void AssistanceMovementGenerator::Finalize(Unit* owner, bool active, bool movementInform)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);

    if (movementInform && HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
    {
        Creature* ownerCreature = owner->ToCreature();
        ownerCreature->SetNoCallAssistance(false);
        ownerCreature->CallAssistance();
        if (ownerCreature->IsAlive())
            ownerCreature->GetMotionMaster()->MoveSeekAssistanceDistract(sWorld->getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY));
    }
}
