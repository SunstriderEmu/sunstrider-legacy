
#include "CombatAI.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
//#include "Vehicle.h"
#include "ObjectAccessor.h"
#include "Player.h"

/////////////////
// AggressorAI
/////////////////

int AggressorAI::Permissible(const Creature* creature)
{
    // have some hostile factions, it will be selected by IsHostileTo check at MoveInLineOfSight
    if (!creature->IsCivilian() && !creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;

    return PERMIT_BASE_NO;
}

void AggressorAI::UpdateAI(uint32 /*diff*/)
{
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}
