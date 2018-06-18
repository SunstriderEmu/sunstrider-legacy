#include "TestCase.h"
#include "MapManager.h"
#include "TestThread.h"
#include "RandomPlayerbotMgr.h"
#include "TestPlayer.h"
#include "RandomPlayerbotFactory.h"
#include "PlayerbotFactory.h"
#include "CharacterCache.h"
#include "SpellHistory.h"
#include "SpellAuraEffects.h"
//#include "ClassSpells.h" //I'm avoiding including this for now since it is changed often and will break script rebuild it is modified and TestCase.cpp has to be rebuilt too

#include <algorithm>

void TestCase::_TestStacksCount(TestPlayer* caster, Unit* target, uint32 castSpellID, uint32 testSpell, uint32 requireCount)
{
    //TODO: cast!
    INTERNAL_TEST_ASSERT(false);
    /*
	uint32 auraCount = target->GetAuraCount(testSpell);
	INTERNAL_TEST_ASSERT(auraCount == requireCount);
    */
}

void TestCase::_TestPowerCost(TestPlayer* caster, uint32 castSpellID, Powers powerType, uint32 expectedPowerCost)
{
    SpellInfo const* spellInfo = _GetSpellInfo(castSpellID);

    INTERNAL_ASSERT_INFO("Spell %u has wrong power type %u (instead of %u)", castSpellID, uint32(spellInfo->PowerType), uint32(powerType));
    INTERNAL_TEST_ASSERT(spellInfo->PowerType == powerType);

    Spell* spell = new Spell(caster, spellInfo, TRIGGERED_NONE);
    uint32 actualCost = spellInfo->CalcPowerCost(caster, spellInfo->GetSchoolMask(), spell);
    delete spell;
    spell = nullptr;

    INTERNAL_ASSERT_INFO("Spell %u has cost %u power instead of %u", castSpellID, actualCost, expectedPowerCost);
    if (expectedPowerCost != 0)
    {
        INTERNAL_TEST_ASSERT(Between(actualCost, expectedPowerCost - 1, expectedPowerCost + 1));
    }
    else
    {
        INTERNAL_TEST_ASSERT(actualCost == expectedPowerCost);
    }
}

void TestCase::_TestCooldown(Unit* caster, Unit* target, uint32 castSpellID, uint32 cooldownMS)
{
    SpellInfo const* spellInfo = _GetSpellInfo(castSpellID);

    //special case for channeled spell, spell system currently does not allow casting them instant
    if (spellInfo->IsChanneled())
    {
        //For now we can't test CD in this case, channel and CD are only starting at next update and when testing CD we already get different values
        return;
    }

    SpellMissInfo const previousForceHitResult = caster->_forceHitResult;
    caster->ForceSpellHitResult(SPELL_MISS_NONE);
    caster->GetSpellHistory()->ResetCooldown(castSpellID);
    _TestCast(caster, target, castSpellID, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
    caster->ForceSpellHitResult(previousForceHitResult);

    //all setup, proceed to test CD
    _TestHasCooldown(caster, castSpellID, cooldownMS);

    //Cleaning up
    caster->GetSpellHistory()->ResetCooldown(castSpellID);
    caster->GetSpellHistory()->ResetGlobalCooldown();
}

void TestCase::_TestCast(Unit* caster, Unit* victim, uint32 spellID, SpellCastResult expectedCode, TriggerCastFlags triggeredFlags)
{
    uint32 res = caster->CastSpell(victim, spellID, triggeredFlags);
    if (expectedCode == SPELL_CAST_OK)
    {
        INTERNAL_ASSERT_INFO("Caster couldn't cast %u, error %s", spellID, StringifySpellCastResult(res).c_str());
    }
    else
    {
        INTERNAL_ASSERT_INFO("Caster cast %u returned unexpected result %s", spellID, StringifySpellCastResult(res).c_str());
    }

	INTERNAL_TEST_ASSERT(res == uint32(expectedCode));
}

void TestCase::_ForceCast(Unit* caster, Unit* victim, uint32 spellID, SpellMissInfo forcedMissInfo, TriggerCastFlags triggeredFlags)
{
    SpellMissInfo const previousForceHitResult = caster->_forceHitResult;
    caster->ForceSpellHitResult(forcedMissInfo);
    uint32 res = caster->CastSpell(victim, spellID, triggeredFlags);
    caster->ForceSpellHitResult(previousForceHitResult);
    INTERNAL_ASSERT_INFO("Caster couldn't cast %u, error %s", spellID, StringifySpellCastResult(res).c_str());
    INTERNAL_TEST_ASSERT(res == uint32(SPELL_CAST_OK));
}

void TestCase::_TestDirectValue(Unit* caster, Unit* target, uint32 spellID, uint32 expectedMin, uint32 expectedMax, bool crit, bool damage, Optional<TestCallback> callback) //if !damage, then use healing
{
    INTERNAL_TEST_ASSERT(expectedMax >= expectedMin);
    Player* _casterOwner = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
    TestPlayer* casterOwner = dynamic_cast<TestPlayer*>(_casterOwner);
    INTERNAL_ASSERT_INFO("Caster in not a testing bot (or a pet/summon of testing bot)");
    INTERNAL_TEST_ASSERT(casterOwner != nullptr);
    auto AI = caster->ToPlayer()->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();
    _MaxHealth(caster);
    _MaxHealth(target);

    auto[sampleSize, maxPredictionError] = _GetApproximationParams(expectedMin, expectedMax);

	EnableCriticals(caster, crit);

    SpellMissInfo const previousForceHitResult = caster->_forceHitResult;
    for (uint32 i = 0; i < sampleSize; i++)
    {
        if (callback)
            callback.get()(caster, target);

        _ForceCast(caster, target, spellID, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_LOS));

        HandleThreadPause();
        HandleSpellsCleanup(caster);
    }

    uint32 dealtMin;
    uint32 dealtMax;
    if(damage)
        std::tie(dealtMin, dealtMax) = GetDamagePerSpellsTo(casterOwner, target, spellID, crit, sampleSize);
    else 
        std::tie(dealtMin, dealtMax) = GetHealingPerSpellsTo(casterOwner, target, spellID, crit, sampleSize);

    TC_LOG_TRACE("test.unit_test", "spellId: %u -> dealtMin: %u - dealtMax %u - expectedMin: %u - expectedMax: %u - sampleSize: %u", spellID, dealtMin, dealtMax, expectedMin, expectedMax, sampleSize);

    uint32 allowedMin = expectedMin > maxPredictionError ? expectedMin - maxPredictionError : 0; //protect against underflow
    uint32 allowedMax = expectedMax + maxPredictionError;

    INTERNAL_ASSERT_INFO("Enforcing high result for spell %u. allowedMax: %u, dealtMax: %u", spellID, allowedMax, dealtMax);
    INTERNAL_TEST_ASSERT(dealtMax <= allowedMax);
    INTERNAL_ASSERT_INFO("Enforcing low result for spell %u. allowedMin: %u, dealtMin: %u", spellID, allowedMin, dealtMin);
    INTERNAL_TEST_ASSERT(dealtMin >= allowedMin);

    //Restoring
    RestoreCriticals(caster);
    _RestoreUnitState(caster);
    _RestoreUnitState(target);
}

void TestCase::_TestMeleeDamage(Unit* caster, Unit* target, WeaponAttackType attackType, uint32 expectedMin, uint32 expectedMax, bool crit, Optional<TestCallback> callback)
{
    auto AI = _GetCasterAI(caster);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();
    _MaxHealth(target);

    auto[sampleSize, maxPredictionError] = _GetApproximationParams(expectedMin, expectedMax);

    MeleeHitOutcome previousForceMeleeResult = caster->_forceMeleeResult;
    caster->ForceMeleeHitResult(crit ? MELEE_HIT_CRIT : MELEE_HIT_NORMAL);
    for (uint32 i = 0; i < sampleSize; i++)
    {
        target->SetFullHealth();

        if (callback)
            callback.get()(caster, target);

        if (attackType != RANGED_ATTACK)
            caster->AttackerStateUpdate(target, attackType);
        else
        {
            caster->CastSpell(target, 75, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_LOS)); //shoot
            HandleSpellsCleanup(caster);
        }

        HandleThreadPause();
    }
    caster->ForceMeleeHitResult(previousForceMeleeResult);

    auto [dealtMin, dealtMax] = GetWhiteDamageDoneTo(caster, target, attackType, crit, sampleSize);

    //TC_LOG_DEBUG("test.unit_test", "attackType: %u - crit %u -> dealtMin: %u - dealtMax %u - expectedMin: %u - expectedMax: %u - sampleSize: %u", uint32(attackType), uint32(crit), dealtMin, dealtMax, expectedMin, expectedMax, sampleSize);

    uint32 allowedMin = expectedMin > maxPredictionError ? expectedMin - maxPredictionError : 0; //protect against underflow
    uint32 allowedMax = expectedMax + maxPredictionError;

    INTERNAL_ASSERT_INFO("Enforcing high result for attackType: %u - crit %u. allowedMax: %u, dealtMax: %u", uint32(attackType), uint32(crit), allowedMax, dealtMax);
    INTERNAL_TEST_ASSERT(dealtMax <= allowedMax);
    INTERNAL_ASSERT_INFO("Enforcing low result for attackType: %u - crit %u. allowedMin: %u, dealtMin: %u", uint32(attackType), uint32(crit), allowedMin, dealtMin);
    INTERNAL_TEST_ASSERT(dealtMin >= allowedMin);

    //restoring
    _RestoreUnitState(target);
}

int32 TestCase::_CastDotAndWait(Unit* caster, Unit* target, uint32 spellID, bool crit)
{
    auto AI = _GetCasterAI(caster);
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    EnableCriticals(caster, crit);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    _ForceCast(caster, target, spellID, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED));

    //Currently, channeled spells start at the next update so we need to wait for it to be applied.
    //Remove this line if spell system has changed and this is not true
    if (spellInfo->IsChanneled())
        WaitNextUpdate();

    Aura* aura = target->GetAura(spellID, caster->GetGUID());
    INTERNAL_ASSERT_INFO("Target has not %u aura with caster %u after spell successfully casted", spellID, caster->GetGUID().GetCounter());
    INTERNAL_TEST_ASSERT(aura != nullptr);
    int32 auraAmount = 0;
    if(aura->GetEffect(EFFECT_0))
        auraAmount = aura->GetEffect(EFFECT_0)->GetAmount();

    //spell did hit, let's wait for dot duration
    uint32 waitTime = aura->GetDuration();
    Wait(waitTime);
    //aura may be deleted at this point, do not use anymore
    aura = nullptr;

    //make sure aura expired
    INTERNAL_ASSERT_INFO("Target still has %u aura after %u ms", spellID, waitTime);
    INTERNAL_TEST_ASSERT(!target->HasAura(spellID, caster->GetGUID()));

    //Restoring
    RestoreCriticals(caster);
    return auraAmount;
}

void TestCase::_TestDotDamage(Unit* caster, Unit* target, uint32 spellID, int32 expectedTotalAmount, bool crit /* = false*/)
{
    auto AI = _GetCasterAI(caster);
    AI->ResetSpellCounters();
    bool heal = expectedTotalAmount < 0;
    _MaxHealth(target, heal);
    
    _CastDotAndWait(caster, target, spellID, crit);

    auto [dotDamageToTarget, tickCount] = AI->GetDotDamage(target, spellID);
	TC_LOG_TRACE("test.unit_test", "spellId: %u -> dotDamageToTarget: %i - expectedTotalAmount: %i", spellID, dotDamageToTarget, expectedTotalAmount);
    INTERNAL_ASSERT_INFO("Enforcing dot damage. dotDamageToTarget: %i, expectedTotalAmount: %i", dotDamageToTarget, expectedTotalAmount);
    INTERNAL_TEST_ASSERT(dotDamageToTarget >= (expectedTotalAmount - tickCount) && dotDamageToTarget <= (expectedTotalAmount + tickCount)); //dots have greater error since they got their damage divided in several ticks

    _RestoreUnitState(target);
}

void TestCase::_TestThreat(Unit* caster, Creature* target, uint32 spellID, float expectedThreatFactor, bool heal, Optional<TestCallback> callback)
{
    // + It'd be nice to deduce heal arg from spell but I don't see any sure way to do it atm

    auto AI = _GetCasterAI(caster);
    //TestPlayer* playerCaster = static_cast<TestPlayer*>(caster); //_GetCasterAI guarantees the caster is a TestPlayer
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    ResetSpellCast(caster);
    ResetSpellCast(target);
    AI->ResetSpellCounters();

    uint32 const startThreat = target->GetThreatManager().GetThreat(caster);
    bool applyAura = spellInfo->HasAnyAura();
    Unit* spellTarget = nullptr;

    //make sure the target is in combat with our caster so that threat is generated
    target->EngageWithTarget(caster);
    INTERNAL_ASSERT_INFO("Caster is not in combat with spell target.");
    INTERNAL_TEST_ASSERT(target->IsInCombatWith(caster));

    if (heal)
        spellTarget = caster;
    else
        spellTarget = target;

    _MaxHealth(spellTarget, heal);
    uint32 const spellTargetStartingHealth = spellTarget->GetHealth();
    uint32 const casterStartingHealth = caster->GetHealth();

    spellTarget->RemoveArenaAuras(false); //may help with already present hot and dots breaking the results

    if (callback)
        callback.get()(caster, target);

    uint32 auraAmount = 0;
    if (applyAura)
        auraAmount = _CastDotAndWait(caster, spellTarget, spellID, false);
    else
    {
        _ForceCast(caster, spellTarget, spellID, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_TARGET_AURASTATE | TRIGGERED_IGNORE_LOS));
        if (spellInfo->IsChanneled())
        {
            WaitNextUpdate();
            uint32 baseDurationTime = spellInfo->GetDuration(); 
            Wait(baseDurationTime); //reason we do this is that currently we can't instantly cast a channeled spell with our spell system
        }
    }

    //just make sure target is still in combat with caster
    INTERNAL_ASSERT_INFO("Caster is not in combat with spell target.");
    INTERNAL_TEST_ASSERT(target->IsInCombatWith(caster));
    int32 totalDamage = 0;
    if (spellInfo->HasAuraEffect(SPELL_AURA_MANA_SHIELD) || spellInfo->HasAuraEffect(SPELL_AURA_SCHOOL_ABSORB))
        totalDamage = auraAmount;
    else
    {
        totalDamage = spellTargetStartingHealth - spellTarget->GetHealth();
        INTERNAL_ASSERT_INFO("No damage or heal done to target");
        INTERNAL_TEST_ASSERT(totalDamage != 0);
    }
    
    if (!heal) //also add health leech if any
    {
        int32 const diff = caster->GetHealth() - casterStartingHealth;
        if (diff > 0)
            totalDamage += diff * 0.5f;
    }

    float const actualThreatDone = target->GetThreatManager().GetThreat(caster) - startThreat;
    float const expectedTotalThreat = std::abs(totalDamage) * expectedThreatFactor;

    INTERNAL_ASSERT_INFO("Enforcing threat for overtime spell %u. Creature should have %f threat but has %f.", spellID, expectedTotalThreat, actualThreatDone);
    INTERNAL_TEST_ASSERT(Between<float>(actualThreatDone, expectedTotalThreat - 1.f, expectedTotalThreat + 1.f));

    //Cleanup
    target->GetThreatManager().ResetThreat(caster);
    target->GetThreatManager().AddThreat(caster, startThreat);
    caster->RemoveAurasDueToSpell(spellID);
    target->RemoveAurasDueToSpell(spellID);
    _RestoreUnitState(spellTarget);
}

void TestCase::_TestChannelDamage(Unit* caster, Unit* target, uint32 spellID, uint32 testedSpell, uint32 expectedTickCount, int32 expectedTickAmount, bool healing /* = false*/)
{
    auto AI = _GetCasterAI(caster);
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    uint32 baseDurationTime = spellInfo->GetDuration();
    SpellMissInfo const previousForceHitResult = caster->_forceHitResult;

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    EnableCriticals(caster, false);
    _ForceCast(caster, target, spellID, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_TARGET_AURASTATE));

    _StartUnitChannels(caster); //remove if spell system allow to cast channel instantly
    Wait(baseDurationTime); //reason we do this is that currently we can't instantly cast a channeled spell with our spell system
    Spell* currentSpell = caster->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    INTERNAL_ASSERT_INFO("caster is still casting channel after baseDurationTime %u", baseDurationTime);
    INTERNAL_TEST_ASSERT(currentSpell == nullptr);

    uint32 totalChannelDmg = 0; 
    if (healing)
        totalChannelDmg = GetChannelHealingTo(caster, target, testedSpell, expectedTickCount, {});
    else
        totalChannelDmg = GetChannelDamageTo(caster, target, testedSpell, expectedTickCount, {});

    INTERNAL_ASSERT_INFO("Target still has aura %u", testedSpell);
    INTERNAL_TEST_ASSERT(!target->HasAura(testedSpell));

    INTERNAL_ASSERT_INFO("Caster still has aura %u", testedSpell);
    INTERNAL_TEST_ASSERT(!caster->HasAura(spellID));

    uint32 actualTickAmount = totalChannelDmg / expectedTickCount;
    INTERNAL_ASSERT_INFO("Channel damage: resultTickAmount: %i, expectedTickAmount: %i", actualTickAmount, expectedTickAmount);
    INTERNAL_TEST_ASSERT(actualTickAmount >= (expectedTickAmount - 2) && actualTickAmount <= (expectedTickAmount + 2)); //channels have greater error since they got their damage divided in several ticks

    //Restoring
    RestoreCriticals(caster);
}

void TestCase::_TestSpellHitChance(Unit* caster, Unit* victim, uint32 spellID, float expectedResultPercent, SpellMissInfo missInfo, Optional<TestCallback> callback)
{
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    auto AI = _GetCasterAI(caster);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    _EnsureAlive(caster, victim);
    _MaxHealth(victim);

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedResultPercent / 100.0f);

    for (uint32 i = 0; i < sampleSize; i++)
    {
        if (callback)
            callback.get()(caster, victim);

        victim->SetFullHealth();
        _TestCast(caster, victim, spellID, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_LOS));
        if (spellInfo->IsChanneled())
            _StartUnitChannels(caster);

        HandleThreadPause();
        HandleSpellsCleanup(caster);
    }

    _TestSpellOutcomePercentage(caster, victim, spellID, missInfo, expectedResultPercent, resultingAbsoluteTolerance * 100, sampleSize);

    //Restore
    ResetSpellCast(caster); // some procs may have occured and may still be in flight, remove them
    _RestoreUnitState(victim);
}

void TestCase::_TestAuraTickProcChance(Unit* caster, Unit* target, uint32 spellID, SpellEffIndex index, float chance, uint32 procSpellId, bool checkSelf)
{
    auto callback = [checkSelf, procSpellId](Unit* caster, Unit* target) { 
        return (checkSelf ? caster : target)->HasAura(procSpellId); 
    };
    _TestAuraTickProcChanceCallback(caster, target, spellID, index, chance, procSpellId, callback);
}

void TestCase::_TestAuraTickProcChanceCallback(Unit* caster, Unit* target, uint32 spellID, SpellEffIndex effIndex, float expectedResultPercent, uint32 procSpellId, TestCallbackResult callback)
{
    _EnsureAlive(caster, target);

    Aura* aura = caster->AddAura(spellID, target);
    INTERNAL_ASSERT_INFO("_TestAuraTickProcChance failed to add aura %u on victim", spellID);
    INTERNAL_TEST_ASSERT(aura != nullptr);
    AuraEffect* effect = aura->GetEffect(effIndex);
    INTERNAL_ASSERT_INFO("_TestAuraTickProcChance failed to get aura %u effect %u on victim", spellID, uint32(effIndex));
    INTERNAL_TEST_ASSERT(effect != nullptr);

    _MaxHealth(target);
    _MaxHealth(caster);

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedResultPercent / 100.0f);

    uint32 successCount = 0;
    for (uint32 i = 0; i < sampleSize; i++)
    {
        effect->PeriodicTick(caster);
        successCount += uint32(callback(caster, target));

        caster->RemoveAurasDueToSpell(procSpellId);
        target->RemoveAurasDueToSpell(procSpellId);

        caster->SetFullHealth();
        target->SetFullHealth();
        caster->ClearDiminishings();
        target->ClearDiminishings();

        HandleThreadPause();
    }

    ResetSpellCast(caster); // some procs may have occured and may still be in flight, remove them

    float actualSuccessPercent = 100 * (successCount / float(sampleSize));
    INTERNAL_ASSERT_INFO("_TestAuraTickProcChance on spell %u: expected result: %f, result: %f", spellID, expectedResultPercent, actualSuccessPercent);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResultPercent, actualSuccessPercent - resultingAbsoluteTolerance * 100, actualSuccessPercent + resultingAbsoluteTolerance * 100));

    //Restoring
    _RestoreUnitState(caster);
    _RestoreUnitState(target);
}

void TestCase::_TestMeleeProcChance(Unit* attacker, Unit* victim, uint32 procSpellID, bool selfProc, float expectedChancePercent, MeleeHitOutcome meleeHitOutcome, WeaponAttackType attackType, Optional<TestCallback> callback)
{
    MeleeHitOutcome previousForceOutcome = attacker->_forceMeleeResult;
    attacker->ForceMeleeHitResult(meleeHitOutcome);

    auto launchCallback = [&](Unit* caster, Unit* victim) {
        attacker->AttackerStateUpdate(victim, attackType);
    };
    auto[actualSuccessPercent, resultingAbsoluteTolerance] = _TestProcChance(attacker, victim, procSpellID, selfProc, expectedChancePercent, launchCallback, callback);

    INTERNAL_ASSERT_INFO("Spell %u proc'd %f instead of expected %f", procSpellID, actualSuccessPercent, expectedChancePercent);
    INTERNAL_TEST_ASSERT(Between<float>(expectedChancePercent, actualSuccessPercent - resultingAbsoluteTolerance * 100, actualSuccessPercent + resultingAbsoluteTolerance * 100));

    attacker->AttackStop();
    attacker->ForceMeleeHitResult(previousForceOutcome);
}

void TestCase::_TestSpellProcChance(Unit* caster, Unit* victim, uint32 spellID, uint32 procSpellID, bool selfProc, float expectedChancePercent, SpellMissInfo missInfo, bool crit, Optional<TestCallback> callback)
{
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);
    EnableCriticals(caster, crit);

    auto launchCallback = [&](Unit* caster, Unit* victim) {
        _ForceCast(caster, victim, spellID, missInfo, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_PROC_AS_NON_TRIGGERED | TRIGGERED_IGNORE_LOS));
        if (spellInfo->IsChanneled())
            _StartUnitChannels(caster);
    };
    auto[actualSuccessPercent, resultingAbsoluteTolerance] = _TestProcChance(caster, victim, procSpellID, selfProc, expectedChancePercent, launchCallback, callback);

    INTERNAL_ASSERT_INFO("Spell %u proc'd %f instead of expected %f (by spell %u)", procSpellID, actualSuccessPercent, expectedChancePercent, spellID);
    INTERNAL_TEST_ASSERT(Between<float>(expectedChancePercent, actualSuccessPercent - resultingAbsoluteTolerance * 100, actualSuccessPercent + resultingAbsoluteTolerance * 100));

    RestoreCriticals(caster);
}

std::pair<float /*procChance*/, float /*absoluteTolerance*/> TestCase::_TestProcChance(Unit* caster, Unit* victim, uint32 procSpellID, bool selfProc, float expectedChancePercent, TestCallback launchCallback, Optional<TestCallback> callback)
{
    auto casterAI = _GetCasterAI(caster, false);
    auto victimAI = _GetCasterAI(victim, false);
    INTERNAL_ASSERT_INFO("Could not find Testing AI for neither caster or victim");
    INTERNAL_TEST_ASSERT(casterAI != nullptr && victimAI != nullptr);

    _EnsureAlive(caster, victim);
    /*SpellInfo const* procSpellInfo =*/ _GetSpellInfo(procSpellID);

    ResetSpellCast(caster);
    if(casterAI)
        casterAI->ResetSpellCounters();
    if(victimAI)
        victimAI->ResetSpellCounters();
    _MaxHealth(caster);
    _MaxHealth(victim);

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedChancePercent / 100.0f);

    Unit* checkTarget = selfProc ? caster : victim;
    auto hasProcced = [checkTarget, procSpellID](PlayerbotTestingAI* AI) {
        auto damageToTarget = AI->GetSpellDamageDoneInfo(checkTarget);
        if (!damageToTarget)
            return false;

        for (auto itr : *damageToTarget)
        {
            if (itr.damageInfo.SpellID != procSpellID)
                continue;

            return true;
        }
        return false;
    };

    uint32 procCount = 0;
    for (uint32 i = 0; i < sampleSize; i++)
    {
        victim->SetFullHealth();
        if (callback)
            callback.get()(caster, victim);

        launchCallback(caster, victim);

        //max 1 proc counted per loop
        if (((casterAI ? hasProcced(casterAI) : false)) || (victimAI ? hasProcced(victimAI) : false))
            procCount++;
        if (casterAI)
            casterAI->ResetSpellCounters();
        if (victimAI)
            victimAI->ResetSpellCounters();

        caster->RemoveAurasDueToSpell(procSpellID);
        victim->RemoveAurasDueToSpell(procSpellID);
        caster->SetFullHealth();
        victim->SetFullHealth();
        caster->ClearDiminishings();
        victim->ClearDiminishings();

        HandleThreadPause();
        HandleSpellsCleanup(caster);
    }

    float actualSuccessPercent = 100 * (procCount / float(sampleSize));
    
    //Restore
    ResetSpellCast(caster); // some procs may have occured and may still be in flight, remove them
    _RestoreUnitState(victim);
    _RestoreUnitState(caster);

    return std::make_pair(actualSuccessPercent, resultingAbsoluteTolerance);
}

void TestCase::_TestPushBackResistChance(Unit* caster, Unit* target, uint32 spellID, float expectedResultPercent)
{
    _EnsureAlive(caster, target);
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    uint32 startingHealth = caster->GetHealth();

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedResultPercent / 100.0f);

    Creature* attackingUnit = SpawnCreature(0, false);
    attackingUnit->ForceMeleeHitResult(MELEE_HIT_NORMAL);

    bool channeled = spellInfo->IsChanneled();
    TriggerCastFlags castFlags = TRIGGERED_FULL_MASK;
    if (!channeled)
        castFlags = TriggerCastFlags(castFlags & ~TRIGGERED_CAST_DIRECTLY); //if channeled, we want finish the initial cast directly to start channeling

    _TestCast(caster, target, spellID, SPELL_CAST_OK, castFlags);
    if (channeled)
        WaitNextUpdate(); //currently we can't start a channel before next update
    Spell* spell = caster->GetCurrentSpell(channeled ? CURRENT_CHANNELED_SPELL:  CURRENT_GENERIC_SPELL);
    INTERNAL_ASSERT_INFO("_TestPushBackResistChance: Failed to get spell %u", spellID);
    INTERNAL_TEST_ASSERT(spell != nullptr);

    uint32 pushbackCount = 0;
    for (uint32 i = 0; i < sampleSize; i++)
    {
        caster->SetFullHealth();

        //timer should be increased for cast, descreased for channels
        uint32 const startChannelTime = 10000;
        uint32 const startCastTime = 1;
        spell->m_timer = channeled ? startChannelTime : startCastTime;
        uint32 healthBefore = caster->GetHealth();

        attackingUnit->AttackerStateUpdate(caster, BASE_ATTACK);
        //health check is just here to ensure integrity
        INTERNAL_ASSERT_INFO("Caster has not lost hp, did melee hit failed?");
        INTERNAL_TEST_ASSERT(caster->GetHealth() < healthBefore);

        if (channeled ? spell->m_timer < startChannelTime : spell->m_timer > startCastTime)
            pushbackCount++;

        HandleThreadPause();
    }

    float actualResistPercent = 100.0f * (1 - (pushbackCount / float(sampleSize)));
    INTERNAL_ASSERT_INFO("_TestPushBackResistChance on spell %u: expected result: %f, result: %f", spellID, expectedResultPercent, actualResistPercent);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResultPercent, actualResistPercent - resultingAbsoluteTolerance * 100, actualResistPercent + resultingAbsoluteTolerance * 100));

    //restoring
    attackingUnit->DisappearAndDie();
    caster->SetHealth(startingHealth);
}

void TestCase::_TestSpellDispelResist(Unit* caster, Unit* target, Unit* dispeler, uint32 spellID, float expectedResultPercent, Optional<TestCallback> callback)
{
    //Dispel resist chance is not related to hit chance but is a separate roll
    //https://wow.gamepedia.com/index.php?title=Dispel&oldid=1432725

    _EnsureAlive(caster, target);
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);
    uint32 dispelMask = spellInfo->GetDispelMask();
    uint32 dispelSpellId = 0;
    if (dispelMask & (1 << DISPEL_MAGIC))
        dispelSpellId = 527; //priest dispel
    else if (dispelMask & (1 << DISPEL_CURSE))
        dispelSpellId = 2782; //druid remove curse
    else if (dispelMask & (1 << DISPEL_DISEASE))
        dispelSpellId = 528; //priest curse disease
    else if (dispelMask & (1 << DISPEL_POISON))
        dispelSpellId = 8946;  //Druid Curse Poison
    else
    {
        INTERNAL_ASSERT_INFO("Dispel mask %u not handled", dispelSpellId);
        INTERNAL_TEST_ASSERT(false);
    }

    _MaxHealth(target);

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedResultPercent / 100.0f);

    uint32 resistedCount = 0;
    for (uint32 i = 0; i < sampleSize; i++)
    {
        target->SetFullHealth();

        if (callback)
            callback.get()(caster, target);

        _ForceCast(caster, target, spellID, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_LOS));
        if (spellInfo->IsChanneled())
            _StartUnitChannels(caster);
        INTERNAL_ASSERT_INFO("TestCase::_TestSpellDispelResist target has not aura of %u after cast", spellID);
        INTERNAL_TEST_ASSERT(target->HasAura(spellID));
        _ForceCast(dispeler, target, dispelSpellId, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
        resistedCount += uint32(target->HasAura(spellID));

        target->ClearDiminishings();
        target->RemoveAurasDueToSpell(spellID);

        HandleThreadPause();
        HandleSpellsCleanup(caster);
        HandleSpellsCleanup(dispeler);
    }

    float actualResistPercent = 100.0f * (resistedCount / float(sampleSize));
    INTERNAL_ASSERT_INFO("_TestSpellDispelResist on spell %u: expected result: %f, result: %f", spellID, expectedResultPercent, actualResistPercent);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResultPercent, actualResistPercent - resultingAbsoluteTolerance * 100, actualResistPercent + resultingAbsoluteTolerance * 100));

    //Restore
    _RestoreUnitState(target);
}

void TestCase::_TestMeleeHitChance(Unit* caster, Unit* victim, WeaponAttackType weaponAttackType, float expectedResultPercent, MeleeHitOutcome meleeHitOutcome, Optional<TestCallback> callback)
{
    _EnsureAlive(caster, victim);
    INTERNAL_ASSERT_INFO("_TestMeleeHitChance can only be used with BASE_ATTACK and OFF_ATTACK");
    INTERNAL_TEST_ASSERT(weaponAttackType <= OFF_ATTACK);

    _MaxHealth(victim);

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedResultPercent / 100.0f);

    for (uint32 i = 0; i < sampleSize; i++)
    {
        victim->SetFullHealth();

        if (callback)
            callback.get()(caster, victim);

        caster->AttackerStateUpdate(victim, weaponAttackType);
        HandleThreadPause();
    }

    _TestMeleeOutcomePercentage(caster, victim, weaponAttackType, meleeHitOutcome, expectedResultPercent, resultingAbsoluteTolerance * 100, sampleSize);

    //Restore
    ResetSpellCast(caster); // some procs may have occured and may still be in flight, remove them
    _RestoreUnitState(victim);
}

void TestCase::_TestMeleeOutcomePercentage(Unit* attacker, Unit* victim, WeaponAttackType weaponAttackType, MeleeHitOutcome meleeHitOutcome, float expectedResult, float allowedError, uint32 sampleSize /*= 0*/)
{
    auto AI = _GetCasterAI(attacker);

    auto damageToTarget = AI->GetMeleeDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("_TestMeleeOutcomePercentage found no data for this victim (%s)", victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());

    uint32 success = 0;
    uint32 attacks = 0; //total attacks with correct type
    //uint32 total = 0;
    for (auto itr : *damageToTarget)
    {
      /*  total++;
        float mean = (success / float(total))* 100.f;
        if(total % 100)
            TC_LOG_ERROR("test.unit_test", "%u error %f \t\t mean %f \t\t %f", total, std::abs(mean - expectedResult), mean, expectedResult);*/

        if (itr.damageInfo.AttackType != weaponAttackType)
            continue;

        attacks++;

        if (itr.damageInfo.HitOutCome != meleeHitOutcome)
            continue;

        success++;
    }

    if (sampleSize)
    {
        INTERNAL_ASSERT_INFO("_TestMeleeOutcomePercentage found %u results instead of expected sample size %u", attacks, sampleSize);
        INTERNAL_TEST_ASSERT(attacks == sampleSize)
    }

    float const result = success / float(damageToTarget->size()) * 100;;
    INTERNAL_ASSERT_INFO("_TestMeleeOutcomePercentage: expected result: %f, result: %f", expectedResult, result);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResult, result - allowedError, result + allowedError));
}

void TestCase::_TestSpellOutcomePercentage(Unit* caster, Unit* victim, uint32 spellID, SpellMissInfo missInfo, float expectedResult, float allowedError, uint32 expectedSampleSize /*= 0*/)
{
    auto AI = _GetCasterAI(caster);

    auto damageToTarget = AI->GetSpellDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("_TestSpellOutcomePercentage found no data of %u for this victim (%s)", spellID, victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());
    
    /*SpellInfo const* spellInfo =*/ _GetSpellInfo(spellID);

    uint32 actualDesiredOutcomeCount = 0;
    uint32 actualSampleCount = 0;
    for (auto itr : *damageToTarget)
    {
        if (itr.damageInfo.SpellID != spellID)
            continue;

        actualSampleCount++;

        if (itr.missInfo != missInfo)
            continue;

        actualDesiredOutcomeCount++;
    }

    if (expectedSampleSize)
    {
        INTERNAL_ASSERT_INFO("_TestSpellOutcomePercentage found %u results instead of expected sample size %u for spell %u", actualSampleCount, expectedSampleSize, spellID);
        INTERNAL_TEST_ASSERT(actualSampleCount == expectedSampleSize)
    }

    float const result = (actualDesiredOutcomeCount / float(actualSampleCount)) * 100.0f;
    INTERNAL_ASSERT_INFO("TestSpellOutcomePercentage on spell %u: expected result: %f, result: %f", spellID, expectedResult, result);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResult, result - allowedError, result + allowedError));
}

void TestCase::_TestSpellCritPercentage(Unit* caster, Unit* victim, uint32 spellId, float expectedResult, float allowedError, uint32 sampleSize)
{
    auto AI = _GetCasterAI(caster);

    auto damageToTarget = AI->GetSpellDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("_TestSpellCritPercentage found no data of spell %u for this victim (%s)", spellId, victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());
    
    /*SpellInfo const* spellInfo =*/ _GetSpellInfo(spellId);

    uint32 critCount = 0;
    uint32 foundCount = 0;
    for (auto itr : *damageToTarget)
    {
        if (itr.damageInfo.SpellID != spellId)
            continue;

        foundCount++;

        if (itr.crit)
            critCount++;
    }

    if (sampleSize)
    {
        INTERNAL_ASSERT_INFO("_TestSpellCritPercentage found %u results instead of expected sample size %u for spell %u", uint32(damageToTarget->size()), sampleSize, spellId);
        INTERNAL_TEST_ASSERT(foundCount == sampleSize)
    }

    float const result = (critCount / float(foundCount)) * 100.0f;
    INTERNAL_ASSERT_INFO("_TestSpellCritPercentage on spell %u: expected result: %f, result: %f", spellId, expectedResult, result);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResult, result - allowedError, result + allowedError));
}

void TestCase::_EnsureHasAura(Unit* target, int32 spellID)
{
    bool checkHasAura = spellID > 0;
    spellID = std::abs(spellID);
    bool hasAura = target->HasAura(spellID);
    if (checkHasAura)
    {
        INTERNAL_ASSERT_INFO("Target %u (%s) does not have aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
        INTERNAL_TEST_ASSERT(hasAura);
    }
    else 
    {
        INTERNAL_ASSERT_INFO("Target %u (%s) has aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
        INTERNAL_TEST_ASSERT(!hasAura);
    }
}

void TestCase::_TestHasCooldown(Unit* caster, uint32 castSpellID, uint32 cooldownMs)
{
    SpellInfo const* spellInfo = _GetSpellInfo(castSpellID);
    uint32 cooldown = caster->GetSpellHistory()->GetRemainingCooldown(spellInfo);
    INTERNAL_ASSERT_INFO("Caster %s has cooldown %u for spell %u instead of expected %u", caster->GetName().c_str(), cooldown, castSpellID, cooldownMs);
    INTERNAL_TEST_ASSERT(cooldown == cooldownMs);
}

void TestCase::_TestAuraMaxDuration(Unit* target, uint32 spellID, uint32 durationMS)
{
    INTERNAL_ASSERT_INFO("Target %u (%s) is not alive", target->GetGUID().GetCounter(), target->GetName().c_str());
    INTERNAL_TEST_ASSERT(target->IsAlive());

    Aura* aura = target->GetAura(spellID);
    INTERNAL_ASSERT_INFO("Target %u (%s) does not have aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(aura != nullptr);

    uint32 auraDuration = aura->GetMaxDuration();
    INTERNAL_ASSERT_INFO("Target %u (%s) has aura (%u) with duration %u instead of %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID, auraDuration, durationMS);
    INTERNAL_TEST_ASSERT(auraDuration == durationMS);
}

void TestCase::_TestAuraStack(Unit* target, uint32 spellID, uint32 stacks, bool stack)
{
    Aura* aura = target->GetAura(spellID);
    INTERNAL_ASSERT_INFO("Target %u (%s) does not have aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(aura != nullptr);

    uint32 auraStacks = 0;
    std::string type = "stacks";
    if (stack)
        auraStacks = aura->GetStackAmount();
    else
    {
        auraStacks = aura->GetCharges();
        type = "charges";
    }
    INTERNAL_ASSERT_INFO("Target %u (%s) has aura (%u) with %u %s instead of %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID, auraStacks, type.c_str(), stacks);
    INTERNAL_TEST_ASSERT(auraStacks == stacks);
}

void TestCase::_TestUseItem(TestPlayer* caster, Unit* target, uint32 itemId)
{
    Item* firstItem = caster->GetFirstItem(itemId);
    INTERNAL_ASSERT_INFO("_TestUseItem failed to find any item with id %u", itemId);
    INTERNAL_TEST_ASSERT_NOCOUNT(firstItem != nullptr);

    SpellCastTargets targets;
    targets.SetUnitTarget(target);
    bool result = caster->GetSession()->_HandleUseItemOpcode(firstItem->GetBagSlot(), firstItem->GetSlot(), 1, 1, firstItem->GetGUID(), targets);
    INTERNAL_ASSERT_INFO("_TestUseItem failed to use item with id %u", itemId);
    INTERNAL_TEST_ASSERT_NOCOUNT(result);
}

void TestCase::_TestSpellCritChance(Unit* caster, Unit* victim, uint32 spellID, float expectedResultPercent, Optional<TestCallback> callback)
{
    auto AI = _GetCasterAI(caster);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    _EnsureAlive(caster, victim);

    _MaxHealth(victim);

    auto[sampleSize, resultingAbsoluteTolerance] = _GetPercentApproximationParams(expectedResultPercent / 100.0f);

    for (uint32 i = 0; i < sampleSize; i++)
    {
        if (callback)
            callback.get()(caster, victim);

        victim->SetFullHealth();
        _ForceCast(caster, victim, spellID, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_IGNORE_LOS));

        HandleThreadPause();
        HandleSpellsCleanup(caster);
    }

    _TestSpellCritPercentage(caster, victim, spellID, expectedResultPercent, resultingAbsoluteTolerance * 100, sampleSize);

    //Restore
    ResetSpellCast(caster); // some procs may have occured and may still be in flight, remove them
    _RestoreUnitState(victim);
}

void TestCase::_TestSpellCastTime(Unit* caster, uint32 spellID, uint32 expectedCastTimeMS)
{
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    Spell* spell = new Spell(caster, spellInfo, TRIGGERED_NONE);
    uint32 const actualCastTime = spellInfo->CalcCastTime(spell);
    delete spell;
    spell = nullptr;

    ASSERT_INFO("Cast time did not match: Expected %u - Actual %u", expectedCastTimeMS, actualCastTime);
    TEST_ASSERT(actualCastTime == expectedCastTimeMS);
}
