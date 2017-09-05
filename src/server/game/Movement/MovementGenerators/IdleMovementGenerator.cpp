
#include "IdleMovementGenerator.h"
#include "CreatureAI.h"
#include "Creature.h"

// StopMoving is needed to make unit stop if its last movement generator expires
// But it should not be sent otherwise there are many redundent packets
bool IdleMovementGenerator::Initialize(Unit* owner)
{
    Reset(owner);
    return true;
}

void IdleMovementGenerator::Reset(Unit* owner)
{
    if (!owner->IsStopped())
        owner->StopMoving();
}
