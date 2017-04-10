#include "DistractMovementGenerator.h"
#include "Creature.h"

// DistractMovementGenerator

DistractMovementGenerator::DistractMovementGenerator(Unit const* owner, float targetOrientation, uint32 timer) :
    m_timer(timer),
    originalOrientation(owner->GetOrientation()),
    targetOrientation(targetOrientation)
{ }

bool DistractMovementGenerator::Initialize(Unit* owner)
{
    // Distracted creatures stand up if not standing
    if (!owner->IsStandState())
        owner->SetStandState(UNIT_STAND_STATE_STAND);

    owner->AddUnitState(UNIT_STATE_DISTRACTED);
    owner->SetFacingTo(targetOrientation);
    return true;
}

void DistractMovementGenerator::Finalize(Unit* owner, bool premature)
{
    if (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature() && !premature)
        owner->SetFacingTo(originalOrientation);

    owner->ClearUnitState(UNIT_STATE_DISTRACTED);
}

bool DistractMovementGenerator::Update(Unit* owner, uint32 time_diff)
{
    if(owner->IsInCombat())
        return false;

    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

// AssistanceDistractMovementGenerator

AssistanceDistractMovementGenerator::AssistanceDistractMovementGenerator(uint32 timer) :
    m_timer(timer)
{ }

bool AssistanceDistractMovementGenerator::Initialize(Unit* owner)
{
    owner->AddUnitState(UNIT_STATE_DISTRACTED);
    return true;
}

bool AssistanceDistractMovementGenerator::Update(Unit* /*owner*/, uint32 time_diff)
{
    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

void AssistanceDistractMovementGenerator::Finalize(Unit* unit, bool /* premature */)
{
    unit->ClearUnitState(UNIT_STATE_DISTRACTED);
    unit->ToCreature()->SetReactState(REACT_AGGRESSIVE);
}
