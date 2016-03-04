#include "ArcherAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "SpellMgr.h"
#include "ObjectAccessor.h"
//#include "Management/VMapFactory.h"
#include "World.h"

#include <list>

int ArcherAI::Permissible(const Creature * /*creature*/)
{
    return PERMIT_BASE_NO;
}

ArcherAI::ArcherAI(Creature *c) : CreatureAI(c)
{
    if (!me->m_spells[0]) {
        TC_LOG_ERROR("scripts","ArcherAI set for creature (entry = %u) with spell1=0. AI will do nothing", me->GetEntry());
        return;
    }

    SpellInfo const* spell0 = sSpellMgr->GetSpellInfo(me->m_spells[0]);
    if (!spell0) {
        TC_LOG_ERROR("scripts","ArcherAI set for creature (entry = %u) with spell1=%u (non existent spell). AI will do nothing", me->GetEntry(), me->m_spells[0]);
        return;
    }

    m_minRange = spell0->GetMinRange();
    if (!m_minRange)
        m_minRange = MELEE_RANGE;
    m_CombatDistance = spell0->GetMaxRange(false, me->GetSpellModOwner());
    me->m_SightDistance = m_CombatDistance;
}

void ArcherAI::AttackStart(Unit *who)
{
    if (!who)
        return;

    if (me->IsWithinCombatRange(who, m_minRange))
    {
        if (me->Attack(who, true) && !who->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY))
            me->GetMotionMaster()->MoveChase(who);
    }
    else
    {
        if (me->Attack(who, false) && !who->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY))
            me->GetMotionMaster()->MoveChase(who, m_CombatDistance);
    }

    if (who->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY))
        me->GetMotionMaster()->MoveIdle();
}

void ArcherAI::UpdateAI(const uint32 /*diff*/)
{
    if (!UpdateVictim())
        return;

    if (!me->IsWithinCombatRange(me->GetVictim(), m_minRange))
        DoSpellAttackIfReady(me->m_spells[0]);
    else
        DoMeleeAttackIfReady();
}
