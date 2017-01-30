
#include "NullCreatureAI.h"
#include "Creature.h"

void PassiveAI::UpdateAI(const uint32)
{
    if(me->IsInCombat() && me->GetAttackers().empty())
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
        if(me->CanAttack(me->GetVictim()) != CAN_ATTACK_RESULT_OK)
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
