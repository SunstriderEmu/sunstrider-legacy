
#include "RotateMovementGenerator.h"
#include "CreatureAI.h"
#include "Creature.h"

RotateMovementGenerator::RotateMovementGenerator(uint32 id, uint32 time, RotateDirection direction) :
    MovementGenerator(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROTATING),
    _id(id), _duration(time), _maxDuration(time), _direction(direction)
{ }

bool RotateMovementGenerator::Initialize(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner->IsStopped())
        owner->StopMoving();

    if (owner->GetVictim())
        owner->SetInFront(owner->GetVictim());

    owner->AttackStop();
    return true;
}

void RotateMovementGenerator::Reset(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    Initialize(owner);
}

bool RotateMovementGenerator::Update(Unit* owner, uint32 diff)
{
    if (!owner)
        return false;

    float angle = owner->GetOrientation();
    if (_direction == ROTATE_DIRECTION_LEFT)
    {
        angle += (float)diff * static_cast<float>(M_PI * 2.0f) / _maxDuration;
        while (angle >= static_cast<float>(M_PI * 2.0f)) 
            angle -= static_cast<float>(M_PI * 2);
    }
    else
    {
        angle -= (float)diff * static_cast<float>(M_PI * 2.0f) / _maxDuration;
        while (angle < 0) 
            angle += static_cast<float>(M_PI * 2.0f);
    }

    owner->SetFacingTo(angle);

    if (_duration > diff)
        _duration -= diff;
    else
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }

    return true;
}

void RotateMovementGenerator::Deactivate(Unit*)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
}

void RotateMovementGenerator::Finalize(Unit* owner, bool/* active*/, bool movementInform)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (owner->GetTypeId() == TYPEID_UNIT)
        owner->ToCreature()->AI()->MovementInform(ROTATE_MOTION_TYPE, _id);
}

MovementGeneratorType RotateMovementGenerator::GetMovementGeneratorType() const
{
    return ROTATE_MOTION_TYPE;
}
