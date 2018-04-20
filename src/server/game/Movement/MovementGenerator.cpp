
#include "Creature.h"
#include "MovementDefines.h"
#include "MovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "WaypointMovementGenerator.h"

MovementGenerator::~MovementGenerator() { }

IdleMovementFactory::IdleMovementFactory() : MovementGeneratorCreator(IDLE_MOTION_TYPE) { }

MovementGenerator* IdleMovementFactory::Create(Unit* /*object*/) const
{
    static IdleMovementGenerator instance;
    return &instance;
}

RandomMovementFactory::RandomMovementFactory() : MovementGeneratorCreator(RANDOM_MOTION_TYPE) { }

MovementGenerator* RandomMovementFactory::Create(Unit* /*object*/) const
{
    return new RandomMovementGenerator<Creature>();
}

WaypointMovementFactory::WaypointMovementFactory() : MovementGeneratorCreator(WAYPOINT_MOTION_TYPE) { }

MovementGenerator* WaypointMovementFactory::Create(Unit* /*object*/) const
{
    return new WaypointMovementGenerator<Creature>();
}
