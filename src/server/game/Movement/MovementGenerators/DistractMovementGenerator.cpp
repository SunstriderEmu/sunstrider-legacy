#include "DistractMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MovementDefines.h"

// DistractMovementGenerator

DistractMovementGenerator::DistractMovementGenerator(Unit const* owner, float targetOrientation, uint32 timer) :
    MovementGenerator(MOTION_MODE_DEFAULT, MOTION_PRIORITY_HIGHEST, UNIT_STATE_DISTRACTED),
    _timer(timer),
    originalOrientation(owner->GetOrientation()),
    targetOrientation(targetOrientation)
{ }

bool DistractMovementGenerator::Initialize(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    // Distracted creatures stand up if not standing
    if (!owner->IsStandState())
        owner->SetStandState(UNIT_STAND_STATE_STAND);

    owner->SetFacingTo(targetOrientation);
    return true;
}

void DistractMovementGenerator::Reset(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    Initialize(owner);
}

bool DistractMovementGenerator::Update(Unit* owner, uint32 diff)
{
    if (!owner)
        return false;

    if (diff > _timer)
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }

    _timer -= diff;
    return true;
}

void DistractMovementGenerator::Deactivate(Unit*)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
}

void DistractMovementGenerator::Finalize(Unit* owner, bool/* active*/, bool movementInform)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    // TODO: This code should be handled somewhere else
    // If this is a creature, then return orientation to original position (for idle movement creatures)
    if (movementInform && HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && owner->GetTypeId() == TYPEID_UNIT)
        owner->SetFacingTo(originalOrientation);
}

MovementGeneratorType DistractMovementGenerator::GetMovementGeneratorType() const
{
    return DISTRACT_MOTION_TYPE;
}

// AssistanceDistractMovementGenerator

AssistanceDistractMovementGenerator::AssistanceDistractMovementGenerator(Unit const* owner, float orientation, uint32 timer) :
    DistractMovementGenerator(owner, orientation, timer)
{ 
    Priority = MOTION_PRIORITY_NORMAL;
}

void AssistanceDistractMovementGenerator::Finalize(Unit* owner, bool/* active*/, bool movementInform)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (movementInform && HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && owner->GetTypeId() == TYPEID_UNIT)
        owner->ToCreature()->SetReactState(REACT_AGGRESSIVE);
}

MovementGeneratorType AssistanceDistractMovementGenerator::GetMovementGeneratorType() const
{
    return ASSISTANCE_DISTRACT_MOTION_TYPE;
}