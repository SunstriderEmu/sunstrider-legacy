
#include "NullCreatureAI.h"
#include "Creature.h"

void PassiveAI::UpdateAI(const uint32)
{
    if(me->IsInCombat())
        EnterEvadeMode();
}

void PossessedAI::AttackStart(Unit *target)
{
    me->Attack(target, true);
}

void PossessedAI::UpdateAI(const uint32 diff)
{
    if(me->GetVictim())
    {
        if(me->_CanCreatureAttack(me->GetVictim()) != CAN_ATTACK_RESULT_OK)
            me->AttackStop();
        else
            DoMeleeAttackIfReady();
    }
}

void PossessedAI::JustDied(Unit *u)
{
    // We died while possessed, disable our loot. Disabled because : WHY ?
  //  me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void PossessedAI::KilledUnit(Unit* victim)
{
    // We killed a creature, disable victim's loot
   /* Disabled because : WHY ?
    if (victim->GetTypeId() == TYPEID_UNIT)
        victim->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE); */
}

void CritterAI::DamageTaken(Unit *done_by, uint32 &)
{
    //force fleeing on creature taking damage
    if(!me->HasUnitState(UNIT_STATE_FLEEING))
        me->SetControlled(true, UNIT_STATE_FLEEING);
}

void CritterAI::EnterEvadeMode(EvadeReason why)
{
    if (me->IsPolymorphed())
        return;
    if(me->HasUnitState(UNIT_STATE_FLEEING))
        me->SetControlled(false, UNIT_STATE_FLEEING);
    CreatureAI::EnterEvadeMode(why);
}

int32 CritterAI::Permissible(Creature const* creature)
{
    if (creature->IsCritter() && !creature->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

int32 NullCreatureAI::Permissible(Creature const* creature)
{
    if (creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
        return PERMIT_BASE_PROACTIVE + 50;

    if (creature->IsTrigger())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_IDLE;
}
