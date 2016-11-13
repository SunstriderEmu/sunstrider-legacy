
#include "ByteBuffer.h"
#include "ReactorAI.h"
#include "Errors.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAIImpl.h"

int ReactorAI::Permissible(const Creature* creature)
{
    if (creature->IsCivilian() || creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;

    return PERMIT_BASE_NO;
}

void ReactorAI::UpdateAI(uint32 /*diff*/)
{
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}
