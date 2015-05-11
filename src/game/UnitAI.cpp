#include "UnitAI.h"

void UnitAI::AttackStart(Unit *victim)
{
    if(!victim)
        return;
        
    if (me->ToCreature() && me->ToCreature()->getAI())
        return;
    
    bool melee = (m_combatDistance > ATTACK_DISTANCE) ? me->GetDistance(victim) <= ATTACK_DISTANCE : true; //visual part
    if(me->Attack(victim, melee))
    {
        if(m_allowCombatMovement)
        {
            //pet attack from behind in melee
            if(me->IsPet() && melee && victim->GetVictim() && victim->GetVictim()->GetGUID() != me->GetGUID())
            {
                me->GetMotionMaster()->MoveChase(victim, CONTACT_DISTANCE, M_PI);
                return;
            }
            
            me->GetMotionMaster()->MoveChase(victim);
        } else {
            me->GetMotionMaster()->MoveIdle();
        }
    }
}

void UnitAI::DoMeleeAttackIfReady()
{
    if (me->HasUnitState(UNIT_STATE_CASTING))
        return;

    Unit* victim = me->GetVictim();

    if (!me->IsWithinMeleeRange(victim))
        return;

    //Make sure our attack is ready and we aren't currently casting before checking distance
    if (me->IsAttackReady())
    {
        me->AttackerStateUpdate(victim);
        me->ResetAttackTimer();
    }

    if (me->HaveOffhandWeapon() && me->IsAttackReady(OFF_ATTACK))
    {
        me->AttackerStateUpdate(victim, OFF_ATTACK);
        me->ResetAttackTimer(OFF_ATTACK);
    }
}

bool UnitAI::DoSpellAttackIfReady(uint32 spell)
{
    if (me->HasUnitState(UNIT_STATE_CASTING) || !me->IsAttackReady())
        return true;
        
    if (!sSpellMgr->GetSpellInfo(spell))
        return true;

    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell))
    {
        if (me->IsWithinCombatRange(me->GetVictim(), spellInfo->GetMaxRange(false, me)))
        {
            me->CastSpell(me->GetVictim(), spell, false);
            me->ResetAttackTimer();
        }
        else
            return false;
    }
    
    return true;
}

void UnitAI::SetCombatDistance(float dist)
{ 
    m_combatDistance = dist;
     //create new targeted movement gen
    if(me->GetVictim())
    {
        me->AttackStop();
        AttackStart(me->GetVictim()); 
    }
};

void UnitAI::SetCombatMovementAllowed(bool allow)
{
    m_allowCombatMovement = allow;
    //create new targeted movement gen
    if(me->GetVictim())
    {
        me->AttackStop();
        AttackStart(me->GetVictim()); 
    }
}

void UnitAI::SetRestoreCombatMovementOnOOM(bool set)
{
    m_restoreCombatMovementOnOOM = set;
}

bool UnitAI::GetRestoreCombatMovementOnOOM()
{
    return m_restoreCombatMovementOnOOM;
}
