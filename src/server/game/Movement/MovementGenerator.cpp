
#include "MovementGenerator.h"
#include "IdleMovementGenerator.h"

MovementGenerator::~MovementGenerator() { }

MovementGenerator* IdleMovementFactory::Create(Unit* /*object*/) const
{
    static IdleMovementGenerator instance;
    return &instance;
}
