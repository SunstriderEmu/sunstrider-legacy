#include "UnitAI.h"

void UnitAI::AttackStart(Unit *victim)
{
    if(!victim)
        return;
        
    bool melee = (me->GetCombatDistance() > ATTACK_DISTANCE) ? me->GetDistance(victim) <= ATTACK_DISTANCE : true; //visual part
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
    me->SetCombatDistance(dist);
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
            me->CastSpell(me->GetVictim(), spell, TRIGGERED_NONE);
            me->ResetAttackTimer();
        }
        else
            return false;
    }
    
    return true;
}

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
    std::list<HostileReference*>& m_threatlist = me->GetThreatManager().getThreatList();
    auto i = m_threatlist.begin();
    auto r = m_threatlist.rbegin();

    if (position >= m_threatlist.size() || !m_threatlist.size())
        return nullptr;

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

    return nullptr;
}

Unit* UnitAI::SelectTarget(SelectAggroTarget targetType, uint32 position, float radius, bool playersOnly, bool noTank)
{
    std::list<HostileReference*>& threatlist = me->GetThreatManager().getThreatList();
    if (position >= threatlist.size())
        return nullptr;

    std::list<Unit*> targetList;
    for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        if (checkTarget((*itr)->getTarget(), playersOnly, radius, noTank))
            targetList.push_back((*itr)->getTarget());

    if (position >= targetList.size())
        return nullptr;

    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
        targetList.sort(Trinity::ObjectDistanceOrderPred(me));

    switch (targetType)
    {
    case SELECT_TARGET_NEAREST:
    case SELECT_TARGET_TOPAGGRO:
    {
        auto itr = targetList.begin();
        std::advance(itr, position);
        return *itr;
    }
    case SELECT_TARGET_FARTHEST:
    case SELECT_TARGET_BOTTOMAGGRO:
    {
        auto ritr = targetList.rbegin();
        std::advance(ritr, position);
        return *ritr;
    }
    case SELECT_TARGET_RANDOM:
    {
        auto itr = targetList.begin();
        std::advance(itr, urand(position, targetList.size() - 1));
        return *itr;
    }
    default:
        break;
    }

    return nullptr;
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
        std::list<HostileReference*> &m_threatlist = me->GetThreatManager().getThreatList();
        if (m_threatlist.empty()) return nullptr;
        std::list<Unit*> targetList;
        auto itr = m_threatlist.begin();
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
            return nullptr;
        targetList.sort(TargetDistanceOrder(me));
        if (targetType == SELECT_TARGET_NEAREST)
        {
            auto i = targetList.begin();
            advance(i, position);
            return *i;
        }
        else
        {
            auto i = targetList.rbegin();
            advance(i, position);
            return *i;
        }
    }
    else
    {
        std::list<HostileReference*> m_threatlist = me->GetThreatManager().getThreatList();
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

    return nullptr;
}

// selects random unit not having aura
Unit* UnitAI::SelectTarget(uint32 position, float distNear, float distFar, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex)
{
    std::list<HostileReference*> m_threatlist = me->GetThreatManager().getThreatList();
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
    return nullptr;
}

void UnitAI::SelectUnitList(std::list<Unit*> &targetList, uint32 maxTargets, SelectAggroTarget targetType, float radius, bool playersOnly, uint32 notHavingAuraId, uint8 effIndex)
{
    std::list<HostileReference*> const& threatlist = me->GetThreatManager().getThreatList();
    if (threatlist.empty())
        return;

    for (auto itr : threatlist)
        if (checkTarget(itr->getTarget(), playersOnly, radius)
            && (!notHavingAuraId || !(itr->getTarget()->HasAuraEffect(notHavingAuraId, effIndex))))
            targetList.push_back(itr->getTarget());

    if (targetList.size() < maxTargets)
        return;

    if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
        targetList.sort(Trinity::ObjectDistanceOrderPred(me));

    if (targetType == SELECT_TARGET_FARTHEST || targetType == SELECT_TARGET_BOTTOMAGGRO)
        targetList.reverse();

    if (targetType == SELECT_TARGET_RANDOM)
        Trinity::Containers::RandomResize(targetList, maxTargets);
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


DefaultTargetSelector::DefaultTargetSelector(Unit const* unit, float dist, bool playerOnly, bool withTank, int32 aura)
    : me(unit), m_dist(dist), m_playerOnly(playerOnly), except(!withTank ? unit->GetThreatManager().GetCurrentVictim() : nullptr), m_aura(aura)
{
}

bool DefaultTargetSelector::operator()(Unit const* target) const
{
    if (!me)
        return false;

    if (!target)
        return false;

    if (except && target == except)
        return false;

    if (m_playerOnly && (target->GetTypeId() != TYPEID_PLAYER))
        return false;

    if (m_dist > 0.0f && !me->IsWithinCombatRange(target, m_dist))
        return false;

    if (m_dist < 0.0f && me->IsWithinCombatRange(target, -m_dist))
        return false;

    if (m_aura)
    {
        if (m_aura > 0)
        {
            if (!target->HasAura(m_aura))
                return false;
        }
        else
        {
            if (target->HasAura(-m_aura))
                return false;
        }
    }

    return true;
}

SpellTargetSelector::SpellTargetSelector(Unit* caster, uint32 spellId) :
    _caster(caster), _spellInfo(sSpellMgr->GetSpellForDifficultyFromSpell(sSpellMgr->GetSpellInfo(spellId), caster))
{
    ASSERT(_spellInfo);
}

bool SpellTargetSelector::operator()(Unit const* target) const
{
    if (!target || _spellInfo->CheckTarget(_caster, target) != SPELL_CAST_OK)
        return false;

    // copypasta from Spell::CheckRange
    float minRange = 0.0f;
    float maxRange = 0.0f;
    float rangeMod = 0.0f;
    if (_spellInfo->RangeEntry)
    {
        if (_spellInfo->RangeEntry->type & SPELL_RANGE_MELEE)
        {
            rangeMod = _caster->GetCombatReach() + 4.0f / 3.0f;
            rangeMod += target->GetCombatReach();

            rangeMod = std::max(rangeMod, NOMINAL_MELEE_RANGE);
        }
        else
        {
            float meleeRange = 0.0f;
            if (_spellInfo->RangeEntry->type & SPELL_RANGE_RANGED)
            {
                meleeRange = _caster->GetCombatReach() + 4.0f / 3.0f;
                meleeRange += target->GetCombatReach();

                meleeRange = std::max(meleeRange, NOMINAL_MELEE_RANGE);
            }

            minRange = _caster->GetSpellMinRangeForTarget(target, _spellInfo) + meleeRange;
            maxRange = _caster->GetSpellMaxRangeForTarget(target, _spellInfo);

            rangeMod = _caster->GetCombatReach();
            rangeMod += target->GetCombatReach();

            if (minRange > 0.0f && !(_spellInfo->RangeEntry->type & SPELL_RANGE_RANGED))
                minRange += rangeMod;
        }

        if (_caster->isMoving() && target->isMoving() && !_caster->IsWalking() && !target->IsWalking() &&
            (_spellInfo->RangeEntry->type & SPELL_RANGE_MELEE || target->GetTypeId() == TYPEID_PLAYER))
            rangeMod += 8.0f / 3.0f;
    }

    maxRange += rangeMod;

    minRange *= minRange;
    maxRange *= maxRange;

    if (target != _caster)
    {
        if (_caster->GetExactDistSq(target) > maxRange)
            return false;

        if (minRange > 0.0f && _caster->GetExactDistSq(target) < minRange)
            return false;
    }

    return true;
}

bool NonTankTargetSelector::operator()(Unit const* target) const
{
    if (!target)
        return false;

    if (_playerOnly && target->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (Unit* currentVictim = _source->GetThreatManager().GetCurrentVictim())
        return target != currentVictim;

    return target != _source->GetVictim();
}


bool PowerUsersSelector::operator()(Unit const* target) const
{
    if (!_me || !target)
        return false;

    if (target->GetPowerType() != _power)
        return false;

    if (_playerOnly && target->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (_dist > 0.0f && !_me->IsWithinCombatRange(target, _dist))
        return false;

    if (_dist < 0.0f && _me->IsWithinCombatRange(target, -_dist))
        return false;

    return true;
}

bool FarthestTargetSelector::operator()(Unit const* target) const
{
    if (!_me || !target)
        return false;

    if (_playerOnly && target->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (_dist > 0.0f && !_me->IsWithinCombatRange(target, _dist))
        return false;

    if (_inLos && !_me->IsWithinLOSInMap(target))
        return false;

    return true;
}

uint32 UnitAI::DoCast(Unit* victim, uint32 spellId, bool triggered)
{
    if (!victim || (me->HasUnitState(UNIT_STATE_CASTING) && !triggered))
        return SPELL_FAILED_UNKNOWN;

    uint32 reason = me->CastSpell(victim, spellId, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);

    //restore combat movement on out of mana
    if (reason == SPELL_FAILED_NO_POWER && GetRestoreCombatMovementOnOOM() && !IsCombatMovementAllowed())
        SetCombatMovementAllowed(true);

    return reason;
}

uint32 UnitAI::DoCastAOE(uint32 spellId, bool triggered)
{
    if (!triggered && me->HasUnitState(UNIT_STATE_CASTING))
        return SPELL_FAILED_UNKNOWN;

    return me->CastSpell(nullptr, spellId, triggered);
}

uint32 UnitAI::DoCastSpell(Unit* who, SpellInfo const *spellInfo, bool triggered)
{
    if (!triggered && me->HasUnitState(UNIT_STATE_CASTING))
        return SPELL_FAILED_UNKNOWN;

    return me->CastSpell(who, spellInfo, triggered);
}

uint32 UnitAI::DoCastVictim(uint32 spellId, bool triggered)
{
    if (!me->GetVictim() || (me->HasUnitState(UNIT_STATE_CASTING) && !triggered))
        return SPELL_FAILED_UNKNOWN;

    return me->CastSpell(me->GetVictim(), spellId, triggered);
}
