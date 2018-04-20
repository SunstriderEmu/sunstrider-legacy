
#include "RotateMovementGenerator.h"
#include "CreatureAI.h"
#include "Creature.h"

bool RotateMovementGenerator::Initialize(Unit* owner)
{
    if (!owner->IsStopped())
        owner->StopMoving();

    if (owner->GetVictim())
        owner->SetInFront(owner->GetVictim());

    owner->AddUnitState(UNIT_STATE_ROTATING);

    owner->AttackStop();
    return true;
}

bool RotateMovementGenerator::Update(Unit* owner, uint32 diff)
{
    float angle = owner->GetOrientation();
    if (m_direction == ROTATE_DIRECTION_LEFT)
    {
        angle += (float)diff * static_cast<float>(M_PI * 2.0f) / m_maxDuration;
        while (angle >= static_cast<float>(M_PI * 2.0f)) 
            angle -= static_cast<float>(M_PI * 2);
    }
    else
    {
        angle -= (float)diff * static_cast<float>(M_PI * 2.0f) / m_maxDuration;
        while (angle < 0) 
            angle += static_cast<float>(M_PI * 2.0f);
    }

    owner->SetFacingTo(angle);

    if (m_duration > diff)
        m_duration -= diff;
    else
        return false;

    return true;
}

void RotateMovementGenerator::Finalize(Unit* owner, bool /* premature */)
{
    owner->ClearUnitState(UNIT_STATE_ROTATING);
    if (owner->GetTypeId() == TYPEID_UNIT)
        owner->ToCreature()->AI()->MovementInform(ROTATE_MOTION_TYPE, 0);
}

MovementGeneratorType RotateMovementGenerator::GetMovementGeneratorType() const
{
    return ROTATE_MOTION_TYPE;
}