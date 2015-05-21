#include "DistractMovementGenerator.h"
#include "Creature.h"

// DistractMovementGenerator

DistractMovementGenerator::DistractMovementGenerator(Unit const* owner, float targetOrientation, uint32 timer) :
    m_timer(timer),
    originalOrientation(owner->GetOrientation()),
    targetOrientation(targetOrientation)
{ }

void DistractMovementGenerator::Initialize(Unit* owner)
{
    owner->AddUnitState(UNIT_STATE_DISTRACTED);
    owner->SetFacingTo(targetOrientation);
}

void DistractMovementGenerator::Finalize(Unit* owner, bool premature)
{
    if(!premature)
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

void AssistanceDistractMovementGenerator::Initialize(Unit* owner)
{
    owner->AddUnitState(UNIT_STATE_DISTRACTED);
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
