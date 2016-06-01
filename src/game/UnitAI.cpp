#include "UnitAI.h"

void UnitAI::AttackStart(Unit *victim)
{
    if(!victim)
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

void UnitAI::AttackStartCaster(Unit* victim, float dist)
{
    SetCombatDistance(dist);
    AttackStart(victim);
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
    bool changed = m_combatDistance != dist;
    m_combatDistance = dist;
     //create new targeted movement gen
    if(changed && me->GetVictim())
    {
        me->AttackStop();
        AttackStart(me->GetVictim()); 
    }
};

void UnitAI::SetCombatMovementAllowed(bool allow)
{
    bool changed = m_allowCombatMovement != allow;
    m_allowCombatMovement = allow;
    //re create new movement gen
    if(changed && me->GetVictim())
    {
        me->AttackStop();
        AttackStart(me->GetVictim()); 
    }
}

void UnitAI::SetRestoreCombatMovementOnOOM(bool set)
{
    m_restoreCombatMovementOnOOM = set;
    //Movement will be restored at next oom cast
}

bool UnitAI::GetRestoreCombatMovementOnOOM()
{
    return m_restoreCombatMovementOnOOM;
}

Unit* UnitAI::SelectTarget(SelectAggroTarget target, uint32 position)
{
    std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator i = m_threatlist.begin();
    std::list<HostileReference*>::reverse_iterator r = m_threatlist.rbegin();

    if (position >= m_threatlist.size() || !m_threatlist.size())
        return NULL;

    switch (target)
    {
    case SELECT_TARGET_RANDOM:
        advance(i, position + (rand() % (m_threatlist.size() - position)));
        return ObjectAccessor::GetUnit((*me), (*i)->getUnitGuid());
    case SELECT_TARGET_TOPAGGRO:
        advance(i, position);
        return ObjectAccessor::GetUnit((*me), (*i)->getUnitGuid());
    case SELECT_TARGET_BOTTOMAGGRO:
        advance(r, position);
        return ObjectAccessor::GetUnit((*me), (*r)->getUnitGuid());
    default:
        break;
    }

    return NULL;
}

Unit* UnitAI::SelectTarget(SelectAggroTarget targetType, uint32 position, float radius, bool playersOnly, bool noTank)
{
    std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
    if (position >= threatlist.size())
        return NULL;

    std::list<Unit*> targetList;
    for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius, noTank))
            targetList.push_back((*itr)->getTarget());

    if (position >= targetList.size())
        return NULL;

    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
        targetList.sort(Trinity::ObjectDistanceOrderPred(me));

    switch (targetType)
    {
    case SELECT_TARGET_NEAREST:
    case SELECT_TARGET_TOPAGGRO:
    {
        std::list<Unit*>::iterator itr = targetList.begin();
        std::advance(itr, position);
        return *itr;
    }
    case SELECT_TARGET_FARTHEST:
    case SELECT_TARGET_BOTTOMAGGRO:
    {
        std::list<Unit*>::reverse_iterator ritr = targetList.rbegin();
        std::advance(ritr, position);
        return *ritr;
    }
    case SELECT_TARGET_RANDOM:
    {
        std::list<Unit*>::iterator itr = targetList.begin();
        std::advance(itr, urand(position, targetList.size() - 1));
        return *itr;
    }
    default:
        break;
    }

    return NULL;
}

struct TargetDistanceOrder : public std::binary_function<const Unit, const Unit, bool>
{
    const Unit* me;
    TargetDistanceOrder(const Unit* Target) : me(Target) {};
    // functor for operator ">"
    bool operator()(const Unit* _Left, const Unit* _Right) const
    {
        return (me->GetDistance(_Left) < me->GetDistance(_Right));
    }
};

Unit* UnitAI::SelectTarget(SelectAggroTarget targetType, uint32 position, float distNear, float distFar, bool playerOnly)
{
    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
    {
        std::list<HostileReference*> &m_threatlist = me->getThreatManager().getThreatList();
        if (m_threatlist.empty()) return NULL;
        std::list<Unit*> targetList;
        std::list<HostileReference*>::iterator itr = m_threatlist.begin();
        for (; itr != m_threatlist.end(); ++itr)
        {
            Unit *target = ObjectAccessor::GetUnit(*me, (*itr)->getUnitGuid());
            if (!target
                || (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
                || (target->GetTransForm() == FORM_SPIRITOFREDEMPTION)
                || (distNear && me->IsWithinCombatRange(target, distNear))
                || (distFar && !me->IsWithinCombatRange(target, distFar))
                )
            {
                continue;
            }
            targetList.push_back(target);
        }
        if (position >= targetList.size())
            return NULL;
        targetList.sort(TargetDistanceOrder(me));
        if (targetType == SELECT_TARGET_NEAREST)
        {
            std::list<Unit*>::iterator i = targetList.begin();
            advance(i, position);
            return *i;
        }
        else
        {
            std::list<Unit*>::reverse_iterator i = targetList.rbegin();
            advance(i, position);
            return *i;
        }
    }
    else
    {
        std::list<HostileReference*> m_threatlist = me->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator i;
        Unit *target;
        while (position < m_threatlist.size())
        {
            if (targetType == SELECT_TARGET_BOTTOMAGGRO)
            {
                i = m_threatlist.end();
                advance(i, -(int32)position - 1);
            }
            else
            {
                i = m_threatlist.begin();
                if (targetType == SELECT_TARGET_TOPAGGRO)
                    advance(i, position);
                else // random
                    advance(i, position + rand() % (m_threatlist.size() - position));
            }

            target = ObjectAccessor::GetUnit(*me, (*i)->getUnitGuid());
            if (!target
                || (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
                || (target->GetTransForm() == FORM_SPIRITOFREDEMPTION)
                || (distNear && me->IsWithinCombatRange(target, distNear))
                || (distFar && !me->IsWithinCombatRange(target, distFar))
                )
            {
                m_threatlist.erase(i);
            }
            else
            {
                return target;
            }
        }
    }

    return NULL;
}

// selects random unit not having aura
Unit* UnitAI::SelectTarget(uint32 position, float distNear, float distFar, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex)
{
    std::list<HostileReference*> m_threatlist = me->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator i;
    Unit *target;
    while (position < m_threatlist.size())
    {
        i = m_threatlist.begin();
        advance(i, position + rand() % (m_threatlist.size() - position));

        target = ObjectAccessor::GetUnit(*me, (*i)->getUnitGuid());
        if (!target
            || !target->IsAlive()
            || (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
            || (distNear && me->IsWithinCombatRange(target, distNear))
            || (distFar && !me->IsWithinCombatRange(target, distFar))
            || (auraCheck && target->HasAuraEffect(spellId, effIndex))
            || (exceptPossesed && target->IsPossessed())
            || (exceptPossesed && target->IsPossessing())
            )
        {
            m_threatlist.erase(i);
        }
        else
        {
            return target;
        }
    }
    return NULL;
}

void UnitAI::SelectUnitList(std::list<Unit*> &targetList, uint32 maxTargets, SelectAggroTarget targetType, float radius, bool playersOnly, uint32 notHavingAuraId, uint8 effIndex)
{
    std::list<HostileReference*> const& threatlist = me->getThreatManager().getThreatList();
    if (threatlist.empty())
        return;

    for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius)
            && (!notHavingAuraId || !((*itr)->getTarget()->HasAuraEffect(notHavingAuraId, effIndex))))
            targetList.push_back((*itr)->getTarget());

    if (targetList.size() < maxTargets)
        return;

    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
        targetList.sort(Trinity::ObjectDistanceOrderPred(me));

    if (targetType == SELECT_TARGET_FARTHEST || targetType == SELECT_TARGET_BOTTOMAGGRO)
        targetList.reverse();

    if (targetType == SELECT_TARGET_RANDOM)
        Trinity::Containers::RandomResizeList(targetList, maxTargets);
    else
        targetList.resize(maxTargets);
}

bool UnitAI::checkTarget(Unit* target, bool playersOnly, float radius, bool noTank)
{
    if (!me)
        return false;

    if (!target)
        return false;

    if (!target->IsAlive())
        return false;

    if (noTank && target == me->GetVictim())
        return false;

    if (playersOnly && (target->GetTypeId() != TYPEID_PLAYER))
        return false;

    if (radius > 0.0f && !me->IsWithinCombatRange(target, radius))
        return false;

    if (radius < 0.0f && me->IsWithinCombatRange(target, -radius))
        return false;

    return true;
}
