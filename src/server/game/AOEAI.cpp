
#include "AOEAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "World.h"

#include <list>

int AOEAI::Permissible(const Creature *creature)
{
    return PERMIT_BASE_NO;
}

AOEAI::AOEAI(Creature *c) : CreatureAI(c)
{
    DEBUG_ASSERT(me->m_spells[0]);
    me->SetVisible(true);
    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);//can't be targeted
    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);//can't be damaged
    me->SetDisplayId(11686);//invisible model,around a size of a player
}

void AOEAI::AttackStart(Unit *who)
{
    
}

void AOEAI::UpdateAI(const uint32 diff)
{
    if(!me->HasAuraEffect(me->m_spells[0], EFFECT_0))
        me->CastSpell(me, me->m_spells[0], TRIGGERED_NONE);
}
