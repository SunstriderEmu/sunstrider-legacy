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

/* TC
uint32 UnitAI::DoCast(uint32 spellId)
{
    Unit* target = nullptr;

    switch (AISpellInfo[spellId].target)
    {
    default:
    case AITARGET_SELF:
        target = me;
        break;
    case AITARGET_VICTIM:
        target = me->GetVictim();
        break;
    case AITARGET_ENEMY:
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
        {
            bool playerOnly = spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS);
            target = SelectTarget(SELECT_TARGET_RANDOM, 0, spellInfo->GetMaxRange(false), playerOnly);
        }
        break;
    }
    case AITARGET_ALLY:
        target = me;
        break;
    case AITARGET_BUFF:
        target = me;
        break;
    case AITARGET_DEBUFF:
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
        {
            bool playerOnly = spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS);
            float range = spellInfo->GetMaxRange(false);

            DefaultTargetSelector targetSelector(me, range, playerOnly, true, -(int32)spellId);
            if (!(spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_VICTIM)
                && targetSelector(me->GetVictim()))
                target = me->GetVictim();
            else
                target = SelectTarget(SELECT_TARGET_RANDOM, 0, targetSelector);
        }
        break;
    }
    }

    if (target)
        me->CastSpell(target, spellId, false);
}
*/

uint32 UnitAI::DoCast(Unit* victim, uint32 spellId, CastSpellExtraArgs const& args)
{
    if (me->HasUnitState(UNIT_STATE_CASTING) && !(args.TriggerFlags & TRIGGERED_IGNORE_CAST_IN_PROGRESS))
        return SPELL_FAILED_UNKNOWN;

    uint32 reason = me->CastSpell(victim, spellId, args);

    //restore combat movement on out of mana
    if (reason == SPELL_FAILED_NO_POWER && GetRestoreCombatMovementOnOOM() && !IsCombatMovementAllowed())
        SetCombatMovementAllowed(true);

    return reason;
}

uint32 UnitAI::DoCastVictim(uint32 spellId, CastSpellExtraArgs const& args)
{
    if (Unit* victim = me->GetVictim())
        return DoCast(victim, spellId, args);

    return SPELL_FAILED_BAD_TARGETS;
}

Unit* UnitAI::SelectTarget(SelectAggroTarget targetType, uint32 position, float dist, bool playerOnly, bool withTank, int32 aura)
{
    return SelectTarget(targetType, position, DefaultTargetSelector(me, dist, playerOnly, withTank, aura));
}

void UnitAI::SelectTargetList(std::list<Unit*>& targetList, uint32 num, SelectAggroTarget targetType, uint32 offset, float dist, bool playerOnly, bool withTank, int32 aura)
{
    SelectTargetList(targetList, num, targetType, offset, DefaultTargetSelector(me, dist, playerOnly, withTank, aura));
}

ThreatManager& UnitAI::GetThreatManager()
{
    return me->GetThreatManager();
}

void UnitAI::SortByDistance(std::list<Unit*> list, bool ascending)
{
    list.sort(Trinity::ObjectDistanceOrderPred(me, ascending));
}