#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"
#include "SpellHistory.h"

#define SOUL_SHARD 6265

class SuppressionTest : public TestCaseScript
{
public:
    SuppressionTest() : TestCaseScript("talents warlock suppression") { }

    // "Reduces the chance for enemies to resist your Affliction spells by 10%."
    class SuppressionTestImpt : public TestCase
    {
    public:
        SuppressionTestImpt() : TestCase(STATUS_PASSING) { } 
        // Known bug: Curse of Doom is not affected, should be.
        // WIP: replace creature 12 by a test specific creature

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            Creature* dummy = SpawnCreature(12); // Boss

            LearnTalent(warlock, Talents::Warlock::SUPPRESSION_RNK_5);
            float const talentHitPct = 10.0f;
            float const hitChance = 16.0f - talentHitPct;

            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_EXHAUSTION_RNK_1, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::DEATH_COIL_RNK_4, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::DRAIN_MANA_RNK_6, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::FEAR_RNK_3, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, hitChance, SPELL_MISS_RESIST);
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, hitChance, SPELL_MISS_RESIST);
            //not affected
            TEST_SPELL_HIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, 16.0f, SPELL_MISS_RESIST);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SuppressionTestImpt>();
    }
};

class ImprovedCorruptionTest : public TestCaseScript
{
public:
    ImprovedCorruptionTest() : TestCaseScript("talents warlock improved_corruption") { }

    // "Reduces the casting time of your Corruption spell by 2 sec." (max rank)
    class ImprovedCorruptionTestImpt : public TestCase
    {
    public:
        ImprovedCorruptionTestImpt() : TestCase(STATUS_PASSING) { }

        void TestCorruptionCastTime(TestPlayer* caster, uint32 talentSpellId, uint32 talentFactor)
        {
            LearnTalent(caster, talentSpellId);
            uint32 const expectedCastTime = 2000 - talentFactor;
            TEST_CAST_TIME(caster, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCastTime);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            uint32 const castTimeReducedPerTalentPoint = 400;

            TestCorruptionCastTime(warlock, Talents::Warlock::IMPROVED_CORRUPTION_RNK_1, 1 * castTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::IMPROVED_CORRUPTION_RNK_2, 2 * castTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::IMPROVED_CORRUPTION_RNK_3, 3 * castTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::IMPROVED_CORRUPTION_RNK_4, 4 * castTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::IMPROVED_CORRUPTION_RNK_5, 5 * castTimeReducedPerTalentPoint);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedCorruptionTestImpt>();
    }
};

class ImprovedCurseOfWeaknessTest : public TestCaseScript
{
public:
    ImprovedCurseOfWeaknessTest() : TestCaseScript("talents warlock improved_curse_of_weakness") { }

    // "Increases the effect of your Curse of Weakness by 20 %" (max rank)
    class ImprovedCurseOfWeaknessTestImpt : public TestCase
    {
    public:
        ImprovedCurseOfWeaknessTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            EQUIP_NEW_ITEM(rogue, 34188); // Leggins of Immortal Night - 124 AP

            LearnTalent(warlock, Talents::Warlock::IMPROVED_CURSE_OF_WEAKNESS_RNK_2);
            float const talentFactor = 1.2f;
            
            uint32 const curseOfWeaknessAPMalus = ClassSpellsDamage::Warlock::CURSE_OF_WEAKNESS_RNK_7 * talentFactor;
            float const expectedRogueAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK) - curseOfWeaknessAPMalus;

            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8);
            float const actualRogueAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            ASSERT_INFO("Rogue has %f AP, should have %f.", actualRogueAP, expectedRogueAP);
            TEST_ASSERT(Between(actualRogueAP, expectedRogueAP - 1.0f, expectedRogueAP + 1.0f));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedCurseOfWeaknessTestImpt>();
    }
};

class ImprovedDrainSoulTest : public TestCaseScript
{
public:
    ImprovedDrainSoulTest() : TestCaseScript("talents warlock improved_drain_soul") { }

    // "Returns 15% of your maximum mana if the target is killed by you while you drain its soul. In addition, your Affliction spells generate 10% less threat." (max rank)
    class ImprovedDrainSoulTestImpt : public TestCase
    {
    public:
        ImprovedDrainSoulTestImpt() : TestCase(STATUS_PASSING) { } 

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            _location.MoveInFront(_location, 10.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            warlock->DisableRegeneration(true);
            rogue->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::IMPROVED_DRAIN_SOUL_RNK_2);
            float const talentManaFactor = 0.15f;
            float const talentThreatFactor = 1.0f - 0.1f;

            // Mana returned
            uint32 const drainSoulManaCost = 360;
            warlock->SetPower(POWER_MANA, drainSoulManaCost);

            uint32 const expectedManaReturn = warlock->GetMaxPower(POWER_MANA) * talentManaFactor;
            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::DRAIN_SOUL_RNK_5);
            rogue->SetHealth(1); // Less than a Drain Soul tick
            WaitNextUpdate();
            Wait(4000);
            TEST_ASSERT(rogue->IsDead());
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == expectedManaReturn);

            // Only get mana if the drain soul kills the target
            rogue->ResurrectPlayer(1.0f);
            warlock->SetPower(POWER_MANA, drainSoulManaCost);
            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::DRAIN_SOUL_RNK_5);
            Wait(1000);
            rogue->KillSelf();
            Wait(2000);
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == 0); //no mana returned

            // Threat reduced by 10% for all affliction spells
            Creature* dummy = SpawnCreature();
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, talentThreatFactor, false);
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, talentThreatFactor, false);
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, talentThreatFactor, false);
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::DEATH_COIL_RNK_4, talentThreatFactor, false); //probably not working with the heal
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, talentThreatFactor, false);
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, talentThreatFactor, false);
            TEST_THREAT(warlock, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, talentThreatFactor, false);
            //okay now... this is dirty. We cant just use TEST_THREAT because damage are not done on target but on enemies around him. So we're using the callback to cast another one on the warlock at each loop that will hit enemies
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            auto dirtyCallback = [](Unit* caster, Unit* target) {
                caster->CastSpell(caster, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, true);
            };
            TEST_THREAT_CALLBACK(warlock, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, talentThreatFactor, false, dirtyCallback);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedDrainSoulTestImpt>();
    }
};

class ImprovedLifeTapTest : public TestCaseScript
{
public:
	ImprovedLifeTapTest() : TestCaseScript("talents warlock improved_life_tap") { }
    
    //"Increases the amount of Mana awarded by your Life Tap spell by 20 %" (max rank)
	class ImprovedLifeTapTestImpt : public TestCase
	{
	public:
		ImprovedLifeTapTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            warlock->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::IMPROVED_LIFE_TAP_RNK_2);
            float const improvedLifeTapFactor = 1.2f;

            uint32 const spellLevel = 68;
            uint32 const perLevelPoint = 1;
            uint32 const perLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * perLevelPoint;
            uint32 const expectedManaGained = (ClassSpellsDamage::Warlock::LIFE_TAP_RNK_7 + perLevelGain) * improvedLifeTapFactor;

            warlock->SetPower(POWER_MANA, 0);
			warlock->CastSpell(warlock, ClassSpells::Warlock::LIFE_TAP_RNK_7);
			TEST_ASSERT(warlock->GetPower(POWER_MANA) == expectedManaGained);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedLifeTapTestImpt>();
	}
};

class SoulSiphonTest : public TestCaseScript
{
public:
    SoulSiphonTest() : TestCaseScript("talents warlock soul_siphon") { }

    //Increases the amount drained by your Drain Life spell by an additional 4 % for each Affliction effect on the target, up to a maximum of 60 % additional effect.
    class SoulSiphonTestImpt : public TestCase
    {
    public:
        SoulSiphonTestImpt() : TestCase(STATUS_PASSING) { }

        void Add4AfflictionSpell(TestPlayer* warlock, Creature* dummy)
        {
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
        }

        void IsAffectedByTalent(TestPlayer* warlock, TestPlayer* warlock2, Creature* dummy, uint32 spellId, float talentFactor)
        {
            warlock->SetHealth(1);
            FORCE_CAST(warlock2, dummy, spellId, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED));
            uint32 warlockExpectedHealth = 1 + 5.0f * floor(ClassSpellsDamage::Warlock::DRAIN_LIFE_RNK_8_TICK * (1.0f + talentFactor));
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8);
            WaitNextUpdate(); //Wait for channel to start
            ASSERT_INFO("After Drain Life started, target didnt have aura %u.", spellId);
            TEST_HAS_AURA(dummy, spellId);
            Wait(6000); //Wait for drain life to finish
            ASSERT_INFO("After spell %u, Warlock has %u HP but should have %u HP.", spellId, warlock->GetHealth(), warlockExpectedHealth);
            TEST_ASSERT(warlock->GetHealth() == warlockExpectedHealth);
            dummy->RemoveAurasDueToSpell(spellId);
            warlock->SetPower(POWER_MANA, warlock->GetMaxPower(POWER_MANA));
            warlock2->SetPower(POWER_MANA, warlock2->GetMaxPower(POWER_MANA));
        }

        uint32 GetHarmfulAurasCount(Creature* dummy)
        {
            uint32 harmfulAuraCount = 0;
            auto& auras = dummy->GetAppliedAuras();
            for (const auto & i : auras)
            {
                if (!i.second->IsPositive())
                    harmfulAuraCount++;
            }
            return harmfulAuraCount;
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            warlock->DisableRegeneration(true);

            TestPlayer* warlock2 = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();
            dummy->_disableSpellBreakChance = true; //needed for fears

            LearnTalent(warlock, Talents::Warlock::SOUL_SIPHON_RNK_2);
            float const bonusPerSpell = 0.04f;

            // Only Drain Life test - Damage is already amplified by itself?
            warlock->SetHealth(1);
            uint32 warlockExpectedHealth = 1 + 5.0f * floor(ClassSpellsDamage::Warlock::DRAIN_LIFE_RNK_8_TICK * (1 + bonusPerSpell));
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8);
            Wait(5500);
            ASSERT_INFO("After Drain Life, Warlock has %u HP but should have %u HP.", warlock->GetHealth(), warlockExpectedHealth);
            TEST_ASSERT(warlock->GetHealth() == warlockExpectedHealth);

            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_EXHAUSTION_RNK_1, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, 2 * bonusPerSpell + 0.1f); // +10% from CoE
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8, 2 * bonusPerSpell);
            //Patch 2.3.0 (2007 - 11 - 13) : Rank 2 changed to 4 % increase per affliction effect from 5 % .It no longer affects Drain Mana.
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::DRAIN_MANA_RNK_6, 1 * bonusPerSpell); //1 instead of 2
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::FEAR_RNK_3, 2 * bonusPerSpell);
            dummy->NearTeleportTo(_location);
            WaitNextUpdate();
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, 2 * bonusPerSpell);
            dummy->NearTeleportTo(_location);
            WaitNextUpdate();
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, 2 * bonusPerSpell);
            IsAffectedByTalent(warlock, warlock2, dummy, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, 2 * bonusPerSpell);

            // Test Max bonus: +60%
            TestPlayer* warlock3 = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* warlock4 = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* warlock5 = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* warlock6 = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);

            Add4AfflictionSpell(warlock2, dummy);
            Add4AfflictionSpell(warlock3, dummy);
            Add4AfflictionSpell(warlock4, dummy);
            Add4AfflictionSpell(warlock5, dummy);
            Add4AfflictionSpell(warlock6, dummy);
            uint32 const expectedHarmfulAuraCount = 20;

            // Dummy should be affected by expectedHarmfulAuraCount Affliction spells
            // If 60% is not enforced, drain should take 80% more dmg
            warlock->SetHealth(1);
            uint32 const actualHarmfulAuraCount = GetHarmfulAurasCount(dummy);
            ASSERT_INFO("Dummy has %u harmful auras instead of %u.", actualHarmfulAuraCount, expectedHarmfulAuraCount);
            TEST_ASSERT(actualHarmfulAuraCount == expectedHarmfulAuraCount);

            float const expectedDrainLifeBonus = std::min(1.0f + actualHarmfulAuraCount * bonusPerSpell, 1.60f);
            warlockExpectedHealth = 1 + 5.0f * floor(ClassSpellsDamage::Warlock::DRAIN_LIFE_RNK_8_TICK * expectedDrainLifeBonus);
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8);
            WaitNextUpdate(); //Wait for channel to start
            Wait(6000); //6s cast time
            uint32 const newActualHarmfulAuraCount = GetHarmfulAurasCount(dummy);
            ASSERT_INFO("Dummy has %u harmful auras instead of %u (after drain life cast).", newActualHarmfulAuraCount, expectedHarmfulAuraCount);
            TEST_ASSERT(newActualHarmfulAuraCount == expectedHarmfulAuraCount);
            ASSERT_INFO("Warlock has %u HP but should have %u HP.", warlock->GetHealth(), warlockExpectedHealth);
            TEST_ASSERT(warlock->GetHealth() == warlockExpectedHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SoulSiphonTestImpt>();
    }
};

class AmplifyCurseTest : public TestCaseScript
{
public:

    AmplifyCurseTest() : TestCaseScript("talents warlock amplify_curse") { }

    //"Increases the effect of your next Curse of Doom or Curse of Agony by 50 % , or your next Curse of Exhaustion by an additional 20 % .Lasts 30sec."
    class AmplifyCurseTestImpt : public TestCase
    {
    public:
        AmplifyCurseTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::AMPLIFY_CURSE_RNK_1);
            float const codAndCoABoostFactor = 1.5f;
            float const coeBoostFactor = 0.5f;

            TEST_CAST(warlock, warlock, ClassSpells::Warlock::AMPLIFY_CURSE_RNK_1);
            TEST_AURA_MAX_DURATION(warlock, ClassSpells::Warlock::AMPLIFY_CURSE_RNK_1, Seconds(30));

            // Curse of Exhaustion
            float const expectedSpeed = warrior->GetSpeed(MOVE_RUN) * coeBoostFactor;
            FORCE_CAST(warlock, warrior, ClassSpells::Warlock::CURSE_OF_EXHAUSTION_RNK_1);
            ASSERT_INFO("Warrior has %f speed, %f was expected.", warrior->GetSpeed(MOVE_RUN), expectedSpeed);
            TEST_ASSERT(Between<float>(warrior->GetSpeed(MOVE_RUN), expectedSpeed - 0.1f, expectedSpeed + 0.1f));
            TEST_HAS_NOT_AURA(warlock, ClassSpells::Warlock::AMPLIFY_CURSE_RNK_1);
            TEST_HAS_COOLDOWN(warlock, ClassSpells::Warlock::AMPLIFY_CURSE_RNK_1, Minutes(3)); // Cooldown starts when consuming a spell

            // Curse of Agony
            float const expectedCoAMaxDamage = ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL * codAndCoABoostFactor;
            uint32 const expectedCoADamage = (4 * expectedCoAMaxDamage / 24.0f) + (4 * expectedCoAMaxDamage / 12.0f) + (4 * expectedCoAMaxDamage / 8.0f);
            TEST_CAST(warlock, warlock, ClassSpells::Warlock::AMPLIFY_CURSE_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage, false);

            // Curse of Doom
            float const expectedCoDDamage = ClassSpellsDamage::Warlock::CURSE_OF_DOOM_RNK_2 * codAndCoABoostFactor;
            TEST_CAST(warlock, warlock, ClassSpells::Warlock::AMPLIFY_CURSE_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, expectedCoDDamage, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AmplifyCurseTestImpt>();
    }
};

class ImprovedCurseOfAgonyTest : public TestCaseScript
{
public:

	ImprovedCurseOfAgonyTest() : TestCaseScript("talents warlock improved_curse_of_agony") { }

    //"Increases the damage done by your Curse of Agony by 10 %" (max rank)
	class ImprovedCurseOfAgonyTestImpt : public TestCase
	{
	public:
		ImprovedCurseOfAgonyTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::IMPROVED_CURSE_OF_AGONY_RNK_2);
            float const improvedCoAFactor = 1.1f;

			float const expectedCoAMaxDamage = ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL * improvedCoAFactor;
            uint32 const expectedCoADamage = (4 * expectedCoAMaxDamage / 24.0f) + (4 * expectedCoAMaxDamage / 12.0f) + (4 * expectedCoAMaxDamage / 8.0f);

            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedCurseOfAgonyTestImpt>();
	}
};

class FelConcentrationTest : public TestCaseScript
{
public:

    FelConcentrationTest() : TestCaseScript("talents warlock fel_concentration") { }

    //"Gives you a 70% chance to avoid interruption caused by damage while channeling the Drain Life, Drain Mana, or Drain Soul spell." (max rank)
    class FelConcentrationTestImpt : public TestCase
    {
    public:
        FelConcentrationTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* paladin = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN);

            LearnTalent(warlock, Talents::Warlock::FEL_CONCENTRATION_RNK_5);
            float const resistPushBackChance = 70.f;

            TEST_PUSHBACK_RESIST_CHANCE(warlock, paladin, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, paladin, ClassSpells::Warlock::DRAIN_MANA_RNK_6, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, paladin, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, resistPushBackChance);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FelConcentrationTestImpt>();
    }
};

class GrimReachTest : public TestCaseScript
{
public:
    GrimReachTest() : TestCaseScript("talents warlock grim_reach") { }

    // "Increases the range of your Affliction spells by 20%." (max rank)
    class GrimReachTestImpt : public TestCase
    {
    public:
        GrimReachTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Position spawn36(_location);
            spawn36.MoveInFront(_location, 36.0f);
            Creature* dummy36 = SpawnCreatureWithPosition(spawn36);

            Position spawn24(_location);
            spawn24.MoveInFront(_location, 24.0f);
            Creature* dummy24 = SpawnCreatureWithPosition(spawn24);

            // Out of Range
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CORRUPTION_RNK_8, SPELL_FAILED_OUT_OF_RANGE);

            LearnTalent(warlock, Talents::Warlock::GRIM_REACH_RNK_2);

            // 36m
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CORRUPTION_RNK_8, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy24, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::DEATH_COIL_RNK_4, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::DRAIN_MANA_RNK_6, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy24, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, SPELL_CAST_OK, TRIGGERED_FULL_DEBUG_MASK);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<GrimReachTestImpt>();
    }
};

class NightfallTest : public TestCaseScript
{
public:

    NightfallTest() : TestCaseScript("talents warlock nightfall") { }

    //"Gives your Corruption and Drain Life spells a 4% chance to cause you to enter a Shadow Trance state after damaging the opponent. The Shadow Trance state reduces the casting time of your next Shadow Bolt spell by 100%." (max rank)
    class NightfallTestImpt : public TestCase
    {
    public:
        NightfallTestImpt() : TestCase(STATUS_WIP) { } // Proc doesnt proc!

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::NIGHTFALL_RNK_2);
            LearnTalent(warlock, Talents::Warlock::IMPROVED_CORRUPTION_RNK_5);
            float const procChance = 4.f;
            uint32 const procSpellId = 17941;

            // Proc chance
            auto callback = [&](Unit* caster, Unit* target) {
                bool hasAura = caster->HasAura(procSpellId);
                caster->RemoveAllAuras();
                return hasAura;
            };
            Wait(5000);
            TEST_AURA_TICK_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, EFFECT_0, procChance, callback);
            TEST_AURA_TICK_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, EFFECT_0, procChance, callback);

            // Instant shadow bolt
            warlock->AddAura(procSpellId, warlock);
            TEST_CAST_TIME(warlock, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, uint32(0));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<NightfallTestImpt>();
    }
};

class EmpoweredCorruptionTest : public TestCaseScript
{
public:

	EmpoweredCorruptionTest() : TestCaseScript("talents warlock empowered_corruption") { }

    //"Your Corruption spell gains an additional 36 % of your bonus spell damage effects."
	class EmpoweredCorruptionTestImpt : public TestCase
	{
	public:
		EmpoweredCorruptionTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::EMPOWERED_CORRUPTION_RNK_3);
            float const empoweredCorruptionFactor = 0.36f;

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

			float const corruptionSpellCoeff = ClassSpellsCoeff::Warlock::CORRUPTION;
			float const expectedCorruptionDamage = ClassSpellsDamage::Warlock::CORRUPTION_RNK_8_TOTAL + (corruptionSpellCoeff + empoweredCorruptionFactor) * spellPower;

            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCorruptionDamage, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EmpoweredCorruptionTestImpt>();
	}
};

class ShadowEmbraceTest : public TestCaseScript
{
public:
    ShadowEmbraceTest() : TestCaseScript("talents warlock shadow_embrace") { }

    class ShadowEmbraceTestImpt : public TestCase
    {
    public:
        //See hack in AuraEffect::CleanupTriggeredSpells for spell cleanup handling
        //"Your Corruption, Curse of Agony, Siphon Life and Seed of Corruption spells also cause the Shadow Embrace effect, which reduces physical damage caused by 5 %" (max rank)
        ShadowEmbraceTestImpt() : TestCase(STATUS_PASSING) { }

        void SpellAppliesAndRemovesShadowEmbrace(TestPlayer* warlock, TestPlayer* victim, uint32 spellId, Seconds dotTime, uint32 shadowEmbraceId)
        {
            victim->SetFullHealth();
            FORCE_CAST(warlock, victim, spellId, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_PROC_AS_NON_TRIGGERED));
            ASSERT_INFO("After spell %u, warlock doesn't have Shadow Embrace.", spellId);
            TEST_HAS_AURA(victim, shadowEmbraceId);
            Wait(dotTime);
            ASSERT_INFO("After spell %u, warlock still has Shadow Embrace.", spellId);
            TEST_HAS_NOT_AURA(victim, shadowEmbraceId);
        }

        void ApplyShadowEmbrace(TestPlayer* rogue, uint32 shadowEmbraceId)
        {
            if (!rogue->HasAura(shadowEmbraceId))
                rogue->AddAura(shadowEmbraceId, rogue);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            LearnTalent(warlock, Talents::Warlock::SHADOW_EMBRACE_RNK_5);
            float const shadowEmbraceFactor = 1 - 0.05f;

            uint32 const SHADOW_EMBRACE_AURA_RNK_5 = 32391;

            SpellAppliesAndRemovesShadowEmbrace(warlock, rogue, ClassSpells::Warlock::CORRUPTION_RNK_8, Seconds(18), SHADOW_EMBRACE_AURA_RNK_5);
            SpellAppliesAndRemovesShadowEmbrace(warlock, rogue, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, Seconds(24), SHADOW_EMBRACE_AURA_RNK_5);
            SpellAppliesAndRemovesShadowEmbrace(warlock, rogue, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, Seconds(18), SHADOW_EMBRACE_AURA_RNK_5);
            SpellAppliesAndRemovesShadowEmbrace(warlock, rogue, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, Seconds(30), SHADOW_EMBRACE_AURA_RNK_5);

            // Physical damage reduced by 5%
            Creature* dummy = SpawnCreature();
            EQUIP_NEW_ITEM(rogue, 32837); // Warglaive of Azzinoth MH
            Wait(1500);
            EQUIP_NEW_ITEM(rogue, 32838); // Warglaive of Azzinoth OH
            WaitNextUpdate();
            int const sinisterStrikeBonus = 98;
            float const normalizedSwordSpeed = 2.4f;
            float const AP = rogue->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (dummy->GetArmor() / (dummy->GetArmor() + 10557.5f));
            // Sinister strike
            uint32 const weaponMinDamage = 214 + (AP / 14 * normalizedSwordSpeed);
            uint32 const weaponMaxDamage = 398 + (AP / 14 * normalizedSwordSpeed);
            uint32 const expectedSinisterStrikeMin = (weaponMinDamage + sinisterStrikeBonus) * armorFactor * shadowEmbraceFactor;
            uint32 const expectedSinisterStrikeMax = (weaponMaxDamage + sinisterStrikeBonus) * armorFactor * shadowEmbraceFactor;
            ApplyShadowEmbrace(rogue, SHADOW_EMBRACE_AURA_RNK_5);
            TEST_DIRECT_SPELL_DAMAGE(rogue, dummy, ClassSpells::Rogue::SINISTER_STRIKE_RNK_10, expectedSinisterStrikeMin, expectedSinisterStrikeMax, false);
            // Melee
            uint32 const mhMinDamage = (214 + (AP / 14 * 2.8f)) * armorFactor * shadowEmbraceFactor;
            uint32 const mhMaxDamage = (398 + (AP / 14 * 2.8f)) * armorFactor * shadowEmbraceFactor;
            uint32 const ohMinDamage = (107 + (AP / 14 * 1.4f)) / 2.0f * armorFactor * shadowEmbraceFactor;
            uint32 const ohMaxDamage = (199 + (AP / 14 * 1.4f)) / 2.0f * armorFactor * shadowEmbraceFactor;
            ApplyShadowEmbrace(rogue, SHADOW_EMBRACE_AURA_RNK_5);
            TEST_MELEE_DAMAGE(rogue, dummy, BASE_ATTACK, mhMinDamage, mhMaxDamage, false);
            ApplyShadowEmbrace(rogue, SHADOW_EMBRACE_AURA_RNK_5);
            TEST_MELEE_DAMAGE(rogue, dummy, OFF_ATTACK, ohMinDamage, ohMaxDamage, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShadowEmbraceTestImpt>();
    }
};

class SiphonLifeTest : public TestCaseScript
{
public:
    SiphonLifeTest() : TestCaseScript("talents warlock siphon_life") { }

    //Spell from talent. "Transfers 63 health from the target to the caster every 3 sec.Lasts 30sec." (max rank)
    class SiphonLifeTestImpt : public TestCase
    {
    public:
        SiphonLifeTestImpt() : TestCase(STATUS_PASSING) { }

        void TestCorruptionCastTime(TestPlayer* caster, Creature* victim, uint32 talentSpellId, uint32 talentFactor)
        {
            LearnTalent(caster, talentSpellId);
            uint32 const expectedCastTime = 2000 - talentFactor;
            TEST_CAST_TIME(caster, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCastTime);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            Creature* dummy = SpawnCreature();

            warlock->DisableRegeneration(true);

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedSiphonLifeManaCost = 410;
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, POWER_MANA, expectedSiphonLifeManaCost);

            // Damage
            float const spellCoefficient = ClassSpellsCoeff::Warlock::SIPHON_LIFE; // DrDamage & WoWWiki coeff
            float const tickAmount = 10.0f;
            uint32 const expectedSLTick = ClassSpellsDamage::Warlock::SIHPON_LIFE_RNK_6_TICK + spellPower * spellCoefficient / tickAmount;
            uint32 const expectedSLTotal = expectedSLTick * tickAmount;
            warlock->SetHealth(1);
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, expectedSLTotal, true);
            TEST_ASSERT(warlock->GetHealth() == 1 + expectedSLTotal);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SiphonLifeTestImpt>();
    }
};

class CurseOfExhaustionTest : public TestCaseScript
{
public:
    CurseOfExhaustionTest() : TestCaseScript("talents warlock curse_of_exhaustion") { }

    //Spell from talent. "Reduces the target's movement speed by 30% for 12sec. Only one Curse per Warlock can be active on any one target." (max rank)
    class CurseOfExhaustionTestImpt : public TestCase
    {
    public:
        CurseOfExhaustionTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN);

            LearnTalent(warlock, Talents::Warlock::CURSE_OF_EXHAUSTION_RNK_1);
            float const speedMalusFactor = 1 - 0.3f;

            float const warriorSpeed = warrior->GetSpeed(MOVE_RUN);
            float const expectedSpeed = warrior->GetSpeed(MOVE_RUN) * speedMalusFactor;
            FORCE_CAST(warlock, warrior, ClassSpells::Warlock::CURSE_OF_EXHAUSTION_RNK_1);
            ASSERT_INFO("Warrior has %f speed, %f was expected.", warrior->GetSpeed(MOVE_RUN), expectedSpeed);
            TEST_ASSERT(Between<float>(warrior->GetSpeed(MOVE_RUN), expectedSpeed - 0.1f, expectedSpeed + 0.1f));
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Warlock::CURSE_OF_EXHAUSTION_RNK_1, Seconds(12));

            TEST_POWER_COST(warlock, ClassSpells::Warlock::CURSE_OF_EXHAUSTION_RNK_1, POWER_MANA, 156);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfExhaustionTestImpt>();
    }
};

class ShadowMasteryTest : public TestCaseScript
{
public:
	ShadowMasteryTest() : TestCaseScript("talents warlock shadow_mastery") { }

	class ShadowMasteryTestImpt : public TestCase
	{
	public:
        //"Increases the damage dealt or life drained by your Shadow spells by 10%." (max rank)
		ShadowMasteryTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            Creature* dummy = SpawnCreature();

            dummy->DisableRegeneration(true);
            warlock->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::SHADOW_MASTERY_RNK_5);
            float const shadowMasteryFactor = 1.1f;

            // Corruption
			float const expectedCorruptionDamage = ClassSpellsDamage::Warlock::CORRUPTION_RNK_8_TOTAL * shadowMasteryFactor;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCorruptionDamage, false);

            // Curse of Agony
			float const expectedCoaDamage = ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL * shadowMasteryFactor;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoaDamage, false);

            // Curse of Doom should not be affected by Shadow Mastery cf Leulier's DPS Spreadsheet v1.12 (http://www.leulier.fr/spreadsheet/) and WoWWiki also mentions it
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, ClassSpellsDamage::Warlock::CURSE_OF_DOOM_RNK_2, false);

            // Death Coil
            uint32 const spellLevel = 68;
            float const dmgPerLevel = 3.4f;
            float const dmgPerLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;
			float const expectedDeathCoilDamage = (ClassSpellsDamage::Warlock::DEATH_COIL_RNK_4 + dmgPerLevelGain) * shadowMasteryFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::DEATH_COIL_RNK_4, expectedDeathCoilDamage, expectedDeathCoilDamage, false);

            // Drain Life
            float const drainLifeTickAmount = 5.0f;
            uint32 const expectedDrainLifeTick = ClassSpellsDamage::Warlock::DRAIN_LIFE_RNK_8_TICK * shadowMasteryFactor;
            uint32 const expectedDrainLifeTotal = drainLifeTickAmount * expectedDrainLifeTick;
            uint32 const dummyExpectedHealth = dummy->GetHealth() - expectedDrainLifeTotal;
            warlock->SetHealth(1);
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8);
            Wait(6000);
            WaitNextUpdate();
            ASSERT_INFO("Dummy has %u HP but %u was expected.", dummy->GetHealth(), dummyExpectedHealth);
            TEST_ASSERT(dummy->GetHealth() == dummyExpectedHealth);
            ASSERT_INFO("Warlock has %u HP but %u was expected.", warlock->GetHealth(), 1 + expectedDrainLifeTotal);
            TEST_ASSERT(warlock->GetHealth() == 1 + expectedDrainLifeTotal);

            // Drain Soul
            float const drainSoulTickAmount = 5.0f;
            uint32 const expectedDrainSoulTick = ClassSpellsDamage::Warlock::DRAIN_SOUL_RNK_5_TICK * shadowMasteryFactor;
            TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, drainSoulTickAmount, expectedDrainSoulTick);

            // Seed of Corruption
            uint32 const seedOfCorruptionTickAmount = 6;
            uint32 const expectedSoCTick = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_TICK * shadowMasteryFactor;
            uint32 const expectedSoCTotalAmount = seedOfCorruptionTickAmount * expectedSoCTick;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, expectedSoCTotalAmount, true);

            // Siphon Life 
            uint32 const siphonLifetickAmount = 10;
            uint32 const expectedSLTick = ClassSpellsDamage::Warlock::SIHPON_LIFE_RNK_6_TICK * siphonLifetickAmount;
            uint32 const expectedSLTotalAmount = expectedSLTick * shadowMasteryFactor;
            warlock->SetHealth(1);
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, expectedSLTotalAmount, true);
            ASSERT_INFO("Warlock has %u HP but %u was expected.", warlock->GetHealth(), 1 + expectedSLTotalAmount);
            TEST_ASSERT(warlock->GetHealth() == 1 + expectedSLTotalAmount);

            // Shadow Bolt
            uint32 const expectedShadowBoltMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * shadowMasteryFactor;
            uint32 const expectedShadowBoltMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * shadowMasteryFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedShadowBoltMin, expectedShadowBoltMax, false);

            // Shadowburn
            uint32 const expectedShadowBurnMin = ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MIN * shadowMasteryFactor;
            uint32 const expectedShadowBurnMax = ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MAX * shadowMasteryFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, expectedShadowBurnMin, expectedShadowBurnMax, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ShadowMasteryTestImpt>();
	}
};

class ContagionTest : public TestCaseScript
{
public:

	ContagionTest() : TestCaseScript("talents warlock contagion") { }

    // "Increases the damage of Curse of Agony, Corruption and Seed of Corruption by 5% and reduces the chance your Affliction spells will be dispelled by an additional 30%." (max rank)
	class ContagionTestImpt : public TestCase
	{
	public:
		ContagionTestImpt() : TestCase(STATUS_WIP) { } // SoC logic is now fixed in here. New status?

		void Test() override
		{
			TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::CONTAGION_RNK_5);
            float const contagionFactor = 1.05f;

            // "Increases the damage of Curse of Agony, Corruption and Seed of Corruption by 5 %"

            // Corruption
			uint32 const expectedCorruptionDamage = ClassSpellsDamage::Warlock::CORRUPTION_RNK_8_TOTAL * contagionFactor;
            //TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCorruptionDamage, false);

            // Curse of Agony
			uint32 const expectedCoADamage = ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL * contagionFactor;
            //TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage, false);

            // Seed of Corruption
			uint32 const expectedSoCDamage = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_TICK * contagionFactor;
            //TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, expectedSoCDamage, false);

            Creature* dummy2 = SpawnCreature();
            uint32 const expectedDetonationMin = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MIN * contagionFactor;
            uint32 const expectedDetonationMax = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MAX * contagionFactor;
            //okay now... this is dirty. We cant just use TEST_DIRECT_SPELL_DAMAGE because damage are not done on target but on enemies around him. So we're using the callback to cast another one on the warlock at each loop that will hit enemies
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            auto dirtyCallback = [](Unit* caster, Unit* target) {
                caster->CastSpell(caster, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, true);
            };
            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(warlock, dummy2, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, expectedDetonationMin, expectedDetonationMax, false, dirtyCallback);
            dummy2->DespawnOrUnsummon();

            // Reduces the chance your Affliction spells wtill be dispelled by 30% (5/5)
            const float dispelTalentFactor = 30.f;
            float const expectedResist = dispelTalentFactor;

            _location.MoveInFront(_location, 5.f);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            WaitNextUpdate();
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);

            ASSERT_INFO("Corruption");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedResist);
            ASSERT_INFO("Death Coil");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::DEATH_COIL_RNK_4, expectedResist);
            ASSERT_INFO("Drain Life");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, expectedResist);
            ASSERT_INFO("Drain Mana");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::DRAIN_MANA_RNK_6, expectedResist);
            ASSERT_INFO("Fear");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::FEAR_RNK_3, expectedResist);
            ASSERT_INFO("Howl of Terror");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, expectedResist);
            ASSERT_INFO("Seed of Corruption");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, expectedResist);
            ASSERT_INFO("Siphon Life");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::SIPHON_LIFE_RNK_6, expectedResist);
            ASSERT_INFO("Unstable Affliction");
            TEST_DISPEL_RESIST_CHANCE(warlock, priest, priest, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, expectedResist);

            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);
            ASSERT_INFO("Curse of Agony");
            TEST_DISPEL_RESIST_CHANCE(warlock, druid, druid, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedResist);
            ASSERT_INFO("Curse of Recklessness");
            TEST_DISPEL_RESIST_CHANCE(warlock, druid, druid, ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5, expectedResist);
            ASSERT_INFO("Curse of the elements");
            TEST_DISPEL_RESIST_CHANCE(warlock, druid, druid, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, expectedResist);
            ASSERT_INFO("Curse of Tongues");
            TEST_DISPEL_RESIST_CHANCE(warlock, druid, druid, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, expectedResist);
            ASSERT_INFO("Curse of Weakness");
            TEST_DISPEL_RESIST_CHANCE(warlock, druid, druid, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8, expectedResist);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ContagionTestImpt>();
	}
};

class DarkPactTest : public TestCaseScript
{
public:
    DarkPactTest() : TestCaseScript("talents warlock dark_pact") { }

    //"Drains 700 of your pet's Mana, returning 100% to you." (max rank)
    class DarkPactTestImpt : public TestCase
    {
    public:
        DarkPactTestImpt() : TestCase(STATUS_PASSING) { } // Not affected by Spell Coefficient

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            warlock->DisableRegeneration(true);

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            TEST_CAST(warlock, warlock, ClassSpells::Warlock::DARK_PACT_RNK_4, SPELL_FAILED_NO_PET);

            // Summon Voidwalker
            TEST_CAST(warlock, warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
            Guardian* voidwalker = warlock->GetGuardianPet();
            TEST_ASSERT(voidwalker != nullptr);
            voidwalker->DisableRegeneration(true);

            uint32 expectedManaDrained = ClassSpellsDamage::Warlock::DARK_PACT_RNK_4 + spellPower * ClassSpellsCoeff::Warlock::DARK_PACT;
            uint32 expectedPetMana = std::max(voidwalker->GetPower(POWER_MANA) - expectedManaDrained, uint32(0));
            warlock->SetPower(POWER_MANA, 0);
            TEST_CAST(warlock, warlock, ClassSpells::Warlock::DARK_PACT_RNK_4);
            ASSERT_INFO("Voidwalker has %u MP but should have %u MP.", voidwalker->GetPower(POWER_MANA), expectedPetMana);
            TEST_ASSERT(voidwalker->GetPower(POWER_MANA) == expectedPetMana);
            ASSERT_INFO("Warlock has %u MP but should have %u MP.", warlock->GetPower(POWER_MANA), expectedManaDrained);
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == expectedManaDrained);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DarkPactTestImpt>();
    }
};

class ImprovedHowOfTerrorTest : public TestCaseScript
{
public:
    ImprovedHowOfTerrorTest() : TestCaseScript("talents warlock improved_howl_of_terror") { }

    //"Reduces the casting time of your Howl of Terror spell by 1.5 sec." (max rank)
    class ImprovedHowOfTerrorTestImpt : public TestCase
    {
    public:
        ImprovedHowOfTerrorTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            uint32 baseCastTime = 1500;
            uint32 reduc_rank_1 = 800;
            uint32 reduc_rank_2 = 1500;
            LearnTalent(warlock, Talents::Warlock::IMPROVED_HOWL_OF_TERROR_RNK_1);
            TEST_CAST_TIME(warlock, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, baseCastTime - reduc_rank_1);
            LearnTalent(warlock, Talents::Warlock::IMPROVED_HOWL_OF_TERROR_RNK_2);
            TEST_CAST_TIME(warlock, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, baseCastTime - reduc_rank_2);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedHowOfTerrorTestImpt>();
    }
};

class MaledictionTest : public TestCaseScript
{
public:
    MaledictionTest() : TestCaseScript("talents warlock malediction") { }

    //"Increases the damage bonus effect of your Curse of Shadows and Curse of the Elements spells by an additional 3%" (max rank)
    class MaledictionTestImpt : public TestCase
    {
    public:
        MaledictionTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::MALEDICTION_RNK_3);
            float const talentBonus = 0.03f;
            float const damageFactor = 1.1f + talentBonus; //1.1 = CoE factor

            // Apply CoE
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4);

            // Increase damage taken by Shadow, Fire, Arcane and Frost by 10%
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * damageFactor, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * damageFactor, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * damageFactor, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * damageFactor, false);
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN * damageFactor, ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX * damageFactor, false);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN * damageFactor, ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX * damageFactor, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MaledictionTestImpt>();
    }
};

class UnstableAfflictionTest : public TestCaseScript
{
public:
    UnstableAfflictionTest() : TestCaseScript("talents warlock unstable_affliction") { }

    //"Shadow energy slowly destroys the target, causing 1050 damage over 18sec. In addition, if the Unstable Affliction is dispelled it will cause 1575 damage to the dispeller and silence them for 5sec." (max rank)
    class UnstableAfflictionTestImpt : public TestCase
    {
    public:
        UnstableAfflictionTestImpt() : TestCase(STATUS_PASSING) { } 

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedUnstableAfflictionManaCost = 400;
            TEST_POWER_COST(warlock, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, POWER_MANA, expectedUnstableAfflictionManaCost);

            // DoT
            float const spellCoefficient = ClassSpellsCoeff::Warlock::UNSTABLE_AFFLICTION_DOT;
            float const tickAmount = 6.0f;
            uint32 const expectedUnstableAfflictionTick = ClassSpellsDamage::Warlock::UNSTABLE_AFFLICTION_RNK_3_TICK + spellPower * spellCoefficient / tickAmount;
            uint32 const expectedUnstableAfflictionTotal = expectedUnstableAfflictionTick * tickAmount;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, expectedUnstableAfflictionTotal, false);

            // Dispell & damage
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            priest->SetMaxHealth(10000);
            priest->SetFullHealth();
            priest->DisableRegeneration(true);
            uint32 const expectedUADamage = ClassSpellsDamage::Warlock::UNSTABLE_AFFLICTION_RNK_3_DISPELLED + spellPower * ClassSpellsCoeff::Warlock::UNSTABLE_AFFLICTION;
            FORCE_CAST(warlock, priest, ClassSpells::Warlock::UNSTABLE_AFFLICTION_RNK_3, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Warlock::UNSTABLE_AFFLICTION_SILENCE, Seconds(5));
            TEST_CAST(priest, priest, ClassSpells::Priest::RENEW_RNK_12, SPELL_FAILED_SILENCED);
            //But can this spell crit?
            auto[dealtMin, dealtMax] = GetDamagePerSpellsTo(warlock, priest, ClassSpells::Warlock::UNSTABLE_AFFLICTION_SILENCE, false, 1);
            uint32 actualDamageDone = dealtMin;
            ASSERT_INFO("actualDamageDone %u, expectedDamageDone %u", actualDamageDone, expectedUADamage);
            TEST_ASSERT(Between(actualDamageDone, expectedUADamage - 3, expectedUADamage + 3)); //larger tolerance due to error related to tick approximation
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<UnstableAfflictionTestImpt>();
    }
};

class ImprovedHealthstoneTest : public TestCaseScript
{
public:
	ImprovedHealthstoneTest() : TestCaseScript("talents warlock improved_healthstone") { }

    //"Increases the amount of Health restored by your Healthstone by 20 %" (max rank)
	class ImprovedHealthstoneTestImpt : public TestCase
	{
	public:
		ImprovedHealthstoneTestImpt() : TestCase(STATUS_PASSING) { }

        void TestImprovedHealthstone(TestPlayer* caster, float talentFactor, uint32 healthstoneSpellId, uint32 healthstoneItemId, uint32 healthRestored)
        {
            caster->SetHealth(1);
            uint32 const expectedHealthRestored = 1 + healthRestored * talentFactor;
            uint32 const expectedHealthRestoredCrit = 1 + healthRestored * talentFactor * 1.5f;
            TEST_CAST(caster, caster, healthstoneSpellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_ASSERT(caster->GetItemCount(healthstoneItemId, false) == 1);
            USE_ITEM(caster, caster, healthstoneItemId);
            ASSERT_INFO("After Healthstone %u, Warlock should have %u HP but has %u.", healthstoneItemId, expectedHealthRestored, caster->GetHealth());
            TEST_ASSERT(caster->GetHealth() == expectedHealthRestored || caster->GetHealth() == expectedHealthRestoredCrit);
            caster->GetSpellHistory()->ResetAllCooldowns();
        }

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            warlock->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::IMPROVED_HEALTHSTONE_RNK_1);
            float const improvedHealthStoneFactorRank1 = 1.1f;
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank1, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_1, 19004, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_1_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank1, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_2, 19006, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_2_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank1, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_3, 19008, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_3_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank1, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_4, 19010, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_4_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank1, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_5, 19012, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_5_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank1, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_6, 22104, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_6_HP_RESTORED);

            LearnTalent(warlock, Talents::Warlock::IMPROVED_HEALTHSTONE_RNK_2);
            float const improvedHealthStoneFactorRank2 = 1.2f;
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank2, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_1, 19005, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_1_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank2, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_2, 19007, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_2_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank2, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_3, 19009, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_3_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank2, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_4, 19011, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_4_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank2, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_5, 19013, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_5_HP_RESTORED);
            TestImprovedHealthstone(warlock, improvedHealthStoneFactorRank2, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_6, 22105, ClassSpellsDamage::Warlock::CREATE_HEALTHSTONE_RNK_6_HP_RESTORED);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedHealthstoneTestImpt>();
	}
};

class DemonicEmbraceTest : public TestCaseScript
{
public:
	DemonicEmbraceTest() : TestCaseScript("talents warlock demonic_embrace") { }

    //"Increases your total Stamina by 15% but reduces your total Spirit by 5%" (max rank)
	class DemonicEmbraceTestImpt : public TestCase
	{
	public:
		DemonicEmbraceTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            float const demonicEmbraceStaminaFactor = 1.15f;
            float const demonicEmbraceSpiritFactor = 0.95f;

            float const expectedWarlockStamina = warlock->GetStat(STAT_STAMINA) * demonicEmbraceStaminaFactor;
            float const expectedWarlockSpirit = warlock->GetStat(STAT_SPIRIT) * demonicEmbraceSpiritFactor;

			LearnTalent(warlock, Talents::Warlock::DEMONIC_EMBRACE_RNK_5);
			TEST_ASSERT(Between<float>(warlock->GetStat(STAT_STAMINA), expectedWarlockStamina - 1, expectedWarlockStamina + 1));
			TEST_ASSERT(Between<float>(warlock->GetStat(STAT_SPIRIT), expectedWarlockSpirit - 1, expectedWarlockSpirit + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicEmbraceTestImpt>();
	}
};

class ImprovedHealthFunnelTest : public TestCaseScript
{
public:
	ImprovedHealthFunnelTest() : TestCaseScript("talents warlock improved_health_funnel") { }

    //"Increases the amount of Health transferred by your Health Funnel spell by 20% and reduces the initial health cost by 20%" (max rank)
	class ImprovedHealthFunnelTestImpt : public TestCase
	{
	public:
		ImprovedHealthFunnelTestImpt() : TestCase(STATUS_KNOWN_BUG) { } // Talent is working but spell is not, so it will fail as long as the spell test is failing

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            warlock->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::IMPROVED_HEALTH_FUNNEL_RNK_2);
            float const talentHealthTransferredFactor = 1.2f;
            float const talentInitialCostFactor = 0.8f;

            // Summon Voidwalker (with low health)
            TEST_CAST(warlock, warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
            Guardian* voidwalker = warlock->GetGuardianPet();
            TEST_ASSERT(voidwalker != nullptr);
            voidwalker->DisableRegeneration(true);
            voidwalker->SetHealth(1);

            // Calculate new health restored and initial cost
            float const tickAmount = 10.0f;
            uint32 const totalHealthFunnelHeal = tickAmount * ClassSpellsDamage::Warlock::HEALTH_FUNNEL_RNK_8_HEAL_PER_TICK * talentHealthTransferredFactor;
            uint32 const expectedTickAmount = totalHealthFunnelHeal / tickAmount;
            uint32 const expectedInitialCost = ClassSpellsDamage::Warlock::HEALTH_FUNNEL_RNK_8_HP_COST * talentInitialCostFactor;
            uint32 const totalHealthCost = tickAmount * ClassSpellsDamage::Warlock::HEALTH_FUNNEL_RNK_8_HP_PER_TICK + expectedInitialCost;

            uint32 const expectedWarlockHealth = warlock->GetHealth() - totalHealthCost;
            uint32 const expectedVoidwalkerHealth = voidwalker->GetHealth() + expectedTickAmount * tickAmount;

            Wait(5000);
            TEST_CAST(warlock, voidwalker, ClassSpells::Warlock::HEALTH_FUNNEL_RNK_8);
            Wait(Seconds(11));
            ASSERT_INFO("Warlock has %u HP, %u was expected.", warlock->GetHealth(), expectedWarlockHealth);
            TEST_ASSERT(warlock->GetHealth() == expectedWarlockHealth);
            ASSERT_INFO("Voidwalker has %u HP, %u was expected.", voidwalker->GetHealth(), expectedVoidwalkerHealth);
            TEST_ASSERT(voidwalker->GetHealth() == expectedVoidwalkerHealth);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedHealthFunnelTestImpt>();
	}
};

class FelIntellectTest : public TestCaseScript
{
public:
	FelIntellectTest() : TestCaseScript("talents warlock fel_intellect") { }

    //Increases the Intellect of your Imp, Voidwalker, Succubus, Felhunter and Felguard by 15% and increases your maximum mana by 3%
	class FelIntellectTestImpt : public TestCase
	{
	public:
		FelIntellectTestImpt() : TestCase(STATUS_KNOWN_BUG) { } //Imp has no bonus intellect, the others probably too

		float GetPetIntellect(TestPlayer* caster, uint32 summonPetSpellId)
		{
            TEST_CAST(caster, caster, summonPetSpellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            Guardian* pet = caster->GetGuardianPet();
            TEST_ASSERT(pet != nullptr);
			return pet->GetStat(STAT_INTELLECT);
		}

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
            float const intellectTalentFactor = 1.15f;
            float const manaTalentFactor = 1.03f;

            //Test warlock Mana
            uint32 const expectedMaxMana = warlock->GetMaxPower(POWER_MANA) * manaTalentFactor;

            //Test pets intellect
			float const expectedImpInt          = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1) * intellectTalentFactor;
			float const expectedVoidwalkerInt   = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1) * intellectTalentFactor;
			float const expectedSuccubusInt     = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1) * intellectTalentFactor;
			float const expectedFelhunterInt    = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1) * intellectTalentFactor;
			float const expectedFelguardInt     = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1) * intellectTalentFactor;

            LearnTalent(warlock, Talents::Warlock::FEL_INTELLECT_RNK_3);
            Wait(1000);

            ASSERT_INFO("Warlock has %u max mana, %u expected.", warlock->GetMaxPower(POWER_MANA), expectedMaxMana);
            TEST_ASSERT(Between<uint32>(warlock->GetMaxPower(POWER_MANA), expectedMaxMana - 1, expectedMaxMana + 1));

			float const impNextInt = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
            ASSERT_INFO("Imp has %f Intellect, %f expected.", impNextInt, expectedImpInt);
			TEST_ASSERT(Between<float>(impNextInt, expectedImpInt - 1.0f, expectedImpInt + 1.0f));

			float const voidwalkerNextInt = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
            ASSERT_INFO("Voidwalker has %f Intellect, %f expected.", voidwalkerNextInt, expectedVoidwalkerInt);
			TEST_ASSERT(Between<float>(voidwalkerNextInt, expectedVoidwalkerInt - 1.0f, expectedVoidwalkerInt + 1.0f));

			float const succubusNextInt = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
            ASSERT_INFO("Succubus has %f Intellect, %f expected.", succubusNextInt, expectedSuccubusInt);
			TEST_ASSERT(Between<float>(succubusNextInt, expectedSuccubusInt - 1.0f, expectedSuccubusInt + 1.0f));

			float const felhunterNextInt = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
            ASSERT_INFO("Felhunter has %f Intellect, %f expected.", felhunterNextInt, expectedFelhunterInt);
			TEST_ASSERT(Between<float>(felhunterNextInt, expectedFelhunterInt - 1.0f, expectedFelhunterInt + 1.0f));

			float const felguardNextInt = GetPetIntellect(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);
            ASSERT_INFO("Felguard has %f Intellect, %f expected.", felguardNextInt, expectedFelguardInt);
			TEST_ASSERT(Between<float>(felguardNextInt, expectedFelguardInt - 1.0f, expectedFelguardInt + 1.0f));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FelIntellectTestImpt>();
	}
};

class FelDominationTest : public TestCaseScript
{
public:
	FelDominationTest() : TestCaseScript("talents warlock fel_domination") { }

    //"Your next Imp, Voidwalker, Succubus, Felhunter or Felguard Summon spell has its casting time reduced by 5.5 sec and its Mana cost reduced by 50%."
	class FelDominationTestImpt : public TestCase
	{
	public:
		FelDominationTestImpt() : TestCase(STATUS_PASSING) { }

		void TestSummonManaCost(TestPlayer* warlock, uint32 summonSpellId, uint32 expectedManaCost)
		{
            warlock->SetPower(POWER_MANA, expectedManaCost);
			warlock->GetSpellHistory()->ResetAllCooldowns();

            TEST_CAST(warlock, warlock, ClassSpells::Warlock::FEL_DOMINATION_RNK_1);
            TEST_HAS_COOLDOWN(warlock, ClassSpells::Warlock::FEL_DOMINATION_RNK_1, Minutes(15));
            TEST_AURA_MAX_DURATION(warlock, ClassSpells::Warlock::FEL_DOMINATION_RNK_1, Seconds(15));

            uint32 const castTimeReduc = 5500;
            TEST_CAST_TIME(warlock, summonSpellId, 10000 - castTimeReduc);
            TEST_POWER_COST(warlock, summonSpellId, POWER_MANA, expectedManaCost);
            TEST_CAST(warlock, warlock, summonSpellId, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
		}

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            warlock->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
            LearnTalent(warlock, Talents::Warlock::FEL_DOMINATION_RNK_1);
            float const talentFactor = 0.5f;

            uint32 const baseMana = warlock->GetMaxPower(POWER_MANA) - warlock->GetManaBonusFromIntellect();
            uint32 const expectedImpManaCost        = baseMana * 0.64f * talentFactor;
            uint32 const expectedFelguardManaCost   = baseMana * 0.80f * talentFactor;
            uint32 const expectedFelhunterManaCost  = baseMana * 0.80f * talentFactor;
            uint32 const expectedSuccubusManaCost   = baseMana * 0.80f * talentFactor;
            uint32 const expectedVoidwalkerManaCost = baseMana * 0.80f * talentFactor;

            warlock->AddItem(SOUL_SHARD, 4);
			TEST_ASSERT(warlock->HasItemCount(6265, 4, false));

			TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1, expectedImpManaCost);
			TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, expectedFelguardManaCost);
			TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, expectedFelhunterManaCost);
			TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, expectedSuccubusManaCost);
			TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, expectedVoidwalkerManaCost);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FelDominationTestImpt>();
	}
};

class FelStaminaTest : public TestCaseScript
{
public:
	FelStaminaTest() : TestCaseScript("talents warlock fel_stamina") { }

    //"Increases the Stamina of your Imp, Voidwalker, Succubus, Felhunter and Felguard by 15% and increases your maximum health by 3%"
	class FelStaminaTestImpt : public TestCase
	{
	public:
		FelStaminaTestImpt() : TestCase(STATUS_PASSING) { }

		float GetPetStamina(TestPlayer* warlock, uint32 summonSpellId)
		{
            TEST_CAST(warlock, warlock, summonSpellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
			Pet* pet = warlock->GetPet();
			TEST_ASSERT(pet != nullptr);
			return pet->GetStat(STAT_STAMINA);
		}

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
            float const talentStaminaFactor = 1.15f;
            float const talentHealthFactor = 1.03f;

			uint32 const expectedWarlockHealth = warlock->GetHealth() * talentHealthFactor;

            warlock->AddItem(SOUL_SHARD, 8);
			TEST_ASSERT(warlock->HasItemCount(6265, 8));

			float const expectedImpStamina          = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			float const expectedVoidwalkerStamina   = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			float const expectedSuccubusStamina     = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			float const expectedFelhunterStamina    = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
			float const expectedFelguardStamina     = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);

			LearnTalent(warlock, Talents::Warlock::FEL_STAMINA_RNK_3);
			TEST_ASSERT(Between<uint32>(warlock->GetHealth(), expectedWarlockHealth - 1, expectedWarlockHealth + 1));

			float const impNextInt = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			TEST_ASSERT(Between<float>(impNextInt, expectedImpStamina - 1.0f, expectedImpStamina + 1.0f));

			float const voidwalkerNextInt = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			TEST_ASSERT(Between<float>(voidwalkerNextInt, expectedVoidwalkerStamina - 1.0f, expectedVoidwalkerStamina + 1.0f));

			float const succubusNextInt = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			TEST_ASSERT(Between<float>(succubusNextInt, expectedSuccubusStamina - 1.0f, expectedSuccubusStamina + 1.0f));

			float const felhunterNextInt = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
			TEST_ASSERT(Between<float>(felhunterNextInt, expectedFelhunterStamina - 1.0f, expectedFelhunterStamina + 1.0f));

			float const felguardNextInt = GetPetStamina(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);
			TEST_ASSERT(Between<float>(felguardNextInt, expectedFelguardStamina - 1.0f, expectedFelguardStamina + 1.0f));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FelStaminaTestImpt>();
	}
};

class DemonicAegisTest : public TestCaseScript
{
public:
	DemonicAegisTest() : TestCaseScript("talents warlock demonic_aegis") { }

    //Increases the effectiveness of your Demon Armor and Fel Armor spells by 30% (max rank)
	class DemonicAegisTestImpt : public TestCase
	{
	public:
		DemonicAegisTestImpt() : TestCase(STATUS_KNOWN_BUG) { } // Talent is OK, Demon Armor spell is not

        void TestDemonArmorBonuses(TestPlayer* caster, Creature* victim, float talentFactor, uint32 demonArmorSpellId, uint32 armorBonus, uint32 shadowResBonus, uint32 healthRestore)
        {
            TEST_ASSERT(caster->IsInCombatWith(victim));

            caster->SetHealth(1);
            uint32 const expectedArmor = caster->GetArmor() + armorBonus * talentFactor;
            uint32 const expectedShadowRes = caster->GetResistance(SPELL_SCHOOL_SHADOW) + shadowResBonus * talentFactor;

            TEST_CAST(caster, caster, demonArmorSpellId);
            TEST_ASSERT(caster->GetArmor() == expectedArmor);
            TEST_ASSERT(caster->GetResistance(SPELL_SCHOOL_SHADOW) == expectedShadowRes);

            uint32 const regenTick = floor(healthRestore * talentFactor / 2.5f);
            uint32 const beforeWaitTime = GameTime::GetGameTimeMS();
            Wait(2000);
            uint32 const afterWaitTime = GameTime::GetGameTimeMS();
            uint32 const elapsedTimeInSeconds = floor((afterWaitTime - beforeWaitTime) / 1000.0f);
            uint32 const expectedTickAmount = floor(elapsedTimeInSeconds / 2.0f);
            uint32 const expectedHealth = 1 + regenTick * expectedTickAmount;
            ASSERT_INFO("%u, Health: %u, expected: %u", demonArmorSpellId, caster->GetHealth(), expectedHealth);
            TEST_ASSERT(caster->GetHealth() == expectedHealth);
        }

        void TestFelArmorBonuses(TestPlayer* caster, TestPlayer* healer, float talentFactor, uint32 felArmorSpellId, uint32 spellDamageBonus)
        {
            // Spell power
            uint32 const expectedSpellDamage = caster->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) + spellDamageBonus * talentFactor;
            TEST_CAST(caster, caster, felArmorSpellId);
            TEST_ASSERT(uint32(caster->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW)) == expectedSpellDamage);

            // Healing
            float const felArmorFactor = 1.0f + (0.2f * 1.3f); //Base 20% heal boost * talent
            uint32 const expectedHTMin = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MIN * felArmorFactor;
            uint32 const expectedHTMax = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MAX * felArmorFactor;
            TEST_DIRECT_HEAL(healer, caster, ClassSpells::Druid::HEALING_TOUCH_RNK_13, expectedHTMin, expectedHTMax, false);

            caster->RemoveAurasDueToSpell(felArmorSpellId);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);
            Creature* dummy = SpawnCreature();

            warlock->AttackerStateUpdate(dummy, BASE_ATTACK);

            LearnTalent(warlock, Talents::Warlock::DEMONIC_AEGIS_RNK_3);
            float const talentFactor = 1.3f;

            TestDemonArmorBonuses(warlock, dummy, talentFactor, ClassSpells::Warlock::DEMON_ARMOR_RNK_1, 210, 3, 7);
            TestDemonArmorBonuses(warlock, dummy, talentFactor, ClassSpells::Warlock::DEMON_ARMOR_RNK_2, 300, 6, 9);
            TestDemonArmorBonuses(warlock, dummy, talentFactor, ClassSpells::Warlock::DEMON_ARMOR_RNK_3, 390, 9, 11);
            TestDemonArmorBonuses(warlock, dummy, talentFactor, ClassSpells::Warlock::DEMON_ARMOR_RNK_4, 480, 12, 13);
            TestDemonArmorBonuses(warlock, dummy, talentFactor, ClassSpells::Warlock::DEMON_ARMOR_RNK_5, 570, 15, 15);
            TestDemonArmorBonuses(warlock, dummy, talentFactor, ClassSpells::Warlock::DEMON_ARMOR_RNK_6, 660, 18, 18);

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            TestFelArmorBonuses(warlock, druid, talentFactor, ClassSpells::Warlock::FEL_ARMOR_RNK_1, 50);
            TestFelArmorBonuses(warlock, druid, talentFactor, ClassSpells::Warlock::FEL_ARMOR_RNK_2, 100);
        }
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicAegisTestImpt>();
	}
};

class MasterSummonerTest : public TestCaseScript
{
public:
	MasterSummonerTest() : TestCaseScript("talents warlock master_summoner") { }

    //Reduces the casting time of your Imp, Voidwalker, Succubus, Felhunter and Fel Guard Summoning spells by 4 sec and the Mana cost by 40 % .
	class MasterSummonerTestImpt : public TestCase
	{
	public:
		MasterSummonerTestImpt() : TestCase(STATUS_PASSING) { }

        void TestSummonManaCost(TestPlayer* warlock, uint32 summonSpellId, uint32 expectedManaCost, bool useFelDomination = false)
        {
            warlock->GetSpellHistory()->ResetAllCooldowns();

            if (useFelDomination)
                TEST_CAST(warlock, warlock, ClassSpells::Warlock::FEL_DOMINATION_RNK_1);

            uint32 const masterSummonerReduc = 4000;
            uint32 const felDominationReduc = 5500;
            uint32 const baseCastTime = 10000;
            int32 expectedCastTime = baseCastTime - masterSummonerReduc;
            if (useFelDomination)
                expectedCastTime -= felDominationReduc;
            if (expectedCastTime < 0)
                expectedCastTime = 0;
            TEST_CAST_TIME(warlock, summonSpellId, uint32(expectedCastTime));

            TEST_POWER_COST(warlock, summonSpellId, POWER_MANA, expectedManaCost);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            warlock->DisableRegeneration(true);

            LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
            LearnTalent(warlock, Talents::Warlock::FEL_DOMINATION_RNK_1);
            LearnTalent(warlock, Talents::Warlock::MASTER_SUMMONER_RNK_2);
            float const masterSummonerManaCostFactor = 1 - 0.4f;
            float const felDominationManaCostFactor = 0.5f;

            uint32 const baseMana = warlock->GetMaxPower(POWER_MANA) - warlock->GetManaBonusFromIntellect();
            uint32 const expectedImpManaCost = baseMana * 0.64f;
            uint32 const expectedFelguardManaCost = baseMana * 0.80f;
            uint32 const expectedFelhunterManaCost = baseMana * 0.80f;
            uint32 const expectedSuccubusManaCost = baseMana * 0.80f;
            uint32 const expectedVoidwalkerManaCost = baseMana * 0.80f;

            warlock->AddItem(SOUL_SHARD, 8);
            TEST_ASSERT(warlock->HasItemCount(6265, 8, false));

            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1, expectedImpManaCost * masterSummonerManaCostFactor);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, expectedFelguardManaCost * masterSummonerManaCostFactor);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, expectedFelhunterManaCost * masterSummonerManaCostFactor);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, expectedSuccubusManaCost * masterSummonerManaCostFactor);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, expectedVoidwalkerManaCost * masterSummonerManaCostFactor);

            float talentsFactor = 1 - (felDominationManaCostFactor + 1 - masterSummonerManaCostFactor);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1, expectedImpManaCost * talentsFactor, true);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, expectedFelguardManaCost * talentsFactor, true);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, expectedFelhunterManaCost * talentsFactor, true);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, expectedSuccubusManaCost * talentsFactor, true);
            TestSummonManaCost(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, expectedVoidwalkerManaCost * talentsFactor, true);
        }
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MasterSummonerTestImpt>();
	}
};

class DemonicSacrificeTest : public TestCaseScript
{
public:
	DemonicSacrificeTest() : TestCaseScript("talents warlock demonic_sacrifice") { }

	class DemonicSacrificeTestImpt : public TestCase
	{
	public:
		DemonicSacrificeTestImpt() : TestCase(STATUS_WIP) { } // What's the new status now that SoC logic is fixed in here?

		void SacrificePet(TestPlayer* warlock, uint32 summonSpellId, uint32 aura, uint32 previousAura = 0)
		{
			TEST_CAST(warlock, warlock, summonSpellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
			Pet* pet = warlock->GetPet();
			TEST_ASSERT(pet != nullptr);
			if(previousAura != 0)
                TEST_HAS_NOT_AURA(warlock, previousAura);

			TEST_CAST(warlock, warlock, ClassSpells::Warlock::DEMONIC_SACRIFICE_RNK_1);
            TEST_HAS_AURA(warlock, aura);
		}

		void TestImpSacrifice(TestPlayer* warlock, Creature* dummy, float sacrificeFactor)
		{
			// Immolate
            uint32 const immolateSpellLevel = 69; //db values
            float const immolateDmgPerLevel = 4.3f; //db values
            float const immolateDmgPerLevelGain = std::max(warlock->GetLevel() - immolateSpellLevel, uint32(0)) * immolateDmgPerLevel;
			uint32 expectedDirectImmolate = (ClassSpellsDamage::Warlock::IMMOLATE_RNK_9 + immolateDmgPerLevelGain) * sacrificeFactor;
			uint32 expectedDotImmolate = ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_DOT * sacrificeFactor;
			TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedDirectImmolate, expectedDirectImmolate, false);
			TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedDotImmolate, false);

			// Rain of Fire
			uint32 expectedRoFTick = ClassSpellsDamage::Warlock::RAIN_OF_FIRE_RNK_5_TOTAL * sacrificeFactor / 4.0f;
			TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5_PROC, 4, expectedRoFTick);

			// Incinerate
			uint32 expectedIncinerateMin = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * sacrificeFactor;
			uint32 expectedIncinerateMax = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * sacrificeFactor;
			TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin, expectedIncinerateMax, false);

			// Searing Pain
			uint32 expectedSearingPainMin = ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MIN * sacrificeFactor;
			uint32 expectedSearingPainMax = ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MAX * sacrificeFactor;
			TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, expectedSearingPainMin, expectedSearingPainMax, false);

			// Soul Fire
			uint32 expectedSoulFireMin = ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MIN * sacrificeFactor;
			uint32 expectedSoulFireMax = ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MAX * sacrificeFactor;
			TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, expectedSoulFireMin, expectedSoulFireMax, false);

			// Hellfire
            warlock->SetMaxHealth(uint32(10000000));
            warlock->SetFullHealth();

            uint32 const hellfireSpellLevel = 68;
            float const hellfireDmgPerLevel = 0.8f;
            float const hellfireDmgPerLevelGain = std::max(warlock->GetLevel() - hellfireSpellLevel, uint32(0)) * hellfireDmgPerLevel;
			uint32 totalHellfire = 15.0f * floor((ClassSpellsDamage::Warlock::HELLFIRE_RNK_4_TICK + hellfireDmgPerLevelGain) * sacrificeFactor);
            uint32 hellfireTickAmount = totalHellfire / 15.0f;
            TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::HELLFIRE_RNK_4, ClassSpells::Warlock::HELLFIRE_RNK_4_TRIGGER, 15, hellfireTickAmount);

            // Self damage
            TEST_DOT_DAMAGE(warlock, warlock, ClassSpells::Warlock::HELLFIRE_RNK_4, totalHellfire, false);
		}

		void TestVoidwalkerSacrifice(TestPlayer* warlock, Creature* dummy)
		{
            warlock->AttackerStateUpdate(dummy, BASE_ATTACK);
			warlock->DisableRegeneration(false);
			warlock->SetHealth(1);
            // Restores 2% every 4 sec.
            uint32 const regenTick = warlock->GetMaxHealth() * 0.02f;

            uint32 const beforeWaitTime = GameTime::GetGameTimeMS();
            Wait(5000);
            uint32 const afterWaitTime = GameTime::GetGameTimeMS();
            uint32 const elapsedTimeInSeconds = floor((afterWaitTime - beforeWaitTime) / 1000.0f);
            uint32 const expectedTickAmount = floor(elapsedTimeInSeconds / 4.0f);
            uint32 const expectedHealth = 1 + regenTick * expectedTickAmount;
            ASSERT_INFO("Warlock should have %u HP but has %u HP.", expectedHealth, warlock->GetHealth());
            TEST_ASSERT(warlock->GetHealth() == expectedHealth);
		}

		void TestSuccubusSacrifice(TestPlayer* warlock, Creature* dummy, float sacrificeFactor)
		{
			// Death Coil
            uint32 const deathCoilSpellLevel = 68;
            float const deathCoilDmgPerLevel = 3.4f;
            float const deathCoilDmgPerLevelGain = std::max(warlock->GetLevel() - deathCoilSpellLevel, uint32(0)) * deathCoilDmgPerLevel;
			uint32 const expectedDeathCoil = (ClassSpellsDamage::Warlock::DEATH_COIL_RNK_4 + deathCoilDmgPerLevelGain) * sacrificeFactor;
			TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::DEATH_COIL_RNK_4, expectedDeathCoil, expectedDeathCoil, false);

			// Shadow Bolt
			uint32 const expectedShadowBoltMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * sacrificeFactor;
			uint32 const expectedShadowBoltMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * sacrificeFactor;
			TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedShadowBoltMin, expectedShadowBoltMax, false);

            // Shadowburn
            uint32 const expectedShadowburnMin = ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MIN * sacrificeFactor;
            uint32 const expectedShadowburnMax = ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MAX * sacrificeFactor;
		    TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, expectedShadowburnMin, expectedShadowburnMax, false);

			// Corruption
            float const corruptionTickAmount = 6.0f;
			uint32 const expectedCorruption = ClassSpellsDamage::Warlock::CORRUPTION_RNK_8_TICK * sacrificeFactor * corruptionTickAmount;
			TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCorruption, false);

			// Curse of Agony
            uint32 const expectedCoAMaxDamage = ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL * sacrificeFactor;
            uint32 const expectedCoADamage = (4.0f * expectedCoAMaxDamage / 24.0f) + (4.0f * expectedCoAMaxDamage / 12.0f) + (4.0f * expectedCoAMaxDamage / 8.0f);
			TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage, false);

			// Curse of Doom
			uint32 const expectedCoD = ClassSpellsDamage::Warlock::CURSE_OF_DOOM_RNK_2 * sacrificeFactor;
			TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, expectedCoD, false);

            // Seed of Corruption
            Creature* dummy2 = SpawnCreature(); // spawn second dummy to take SoC detonation

            float const socTickAmount = 6.0f;
            uint32 const seedOfCorruptionTick = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_TICK * sacrificeFactor;
            uint32 const expectedSoCTotalAmount = socTickAmount * seedOfCorruptionTick;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, expectedSoCTotalAmount, true);

            uint32 const expectedDetonationMin = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MIN * sacrificeFactor;
            uint32 const expectedDetonationMax = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MAX * sacrificeFactor;
            //okay now... this is dirty. We cant just use TEST_DIRECT_SPELL_DAMAGE because damage are not done on target but on enemies around him. So we're using the callback to cast another one on the warlock at each loop that will hit enemies
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            auto dirtyCallback = [](Unit* caster, Unit* target) {
                caster->CastSpell(caster, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, true);
            };
            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(warlock, dummy2, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, expectedDetonationMin, expectedDetonationMax, false, dirtyCallback);
            dummy2->DespawnOrUnsummon();
		}

		void TestFelhunterSacrifice(TestPlayer* warlock, Creature* dummy, float sacrificeFactor)
		{
            warlock->AttackerStateUpdate(dummy, BASE_ATTACK);
            warlock->DisableRegeneration(true);
            warlock->SetPower(POWER_MANA, 0);
            // Restores x% of total mana every 4 sec.
            uint32 const regenTick = warlock->GetMaxPower(POWER_MANA) * sacrificeFactor;

            uint32 const beforeWaitTime = GameTime::GetGameTimeMS();
            Wait(5000);
            uint32 const afterWaitTime = GameTime::GetGameTimeMS();
            uint32 const elapsedTimeInSeconds = floor((afterWaitTime - beforeWaitTime) / 1000.0f);
            uint32 const expectedTickAmount = floor(elapsedTimeInSeconds / 4.0f);
            uint32 const expectedMana = regenTick * expectedTickAmount;
            ASSERT_INFO("Warlock should have %u MP but has %u MP.", expectedMana, warlock->GetPower(POWER_MANA));
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == expectedMana);
		}

		void TestFelguardSacrifice(TestPlayer* warlock, Creature* dummy)
		{
			TestSuccubusSacrifice(warlock, dummy, 1.1f);
			TestFelhunterSacrifice(warlock, dummy, 0.02f);
		}

		void Test() override
		{
			TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
            LearnTalent(warlock, Talents::Warlock::DEMONIC_SACRIFICE_RNK_1);

			SacrificePet(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1, 18789); // Burning Wish
			TestImpSacrifice(warlock, dummy, 1.15f);
			SacrificePet(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, 18790, 18789); // Fel Stamina
			TestVoidwalkerSacrifice(warlock, dummy);
			SacrificePet(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, 18791, 18790); // Touch of Shadow
			TestSuccubusSacrifice(warlock, dummy, 1.15f);
			SacrificePet(warlock, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, 18792, 18791); // Fel Energy
			TestFelhunterSacrifice(warlock, dummy, 0.03f);
			SacrificePet(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, 35701, 18792); // Touch of Shadow
			TestFelguardSacrifice(warlock, dummy);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicSacrificeTestImpt>();
	}
};

class ManaFeedTest : public TestCaseScript
{
public:
	ManaFeedTest() : TestCaseScript("talents warlock mana_feed") { }

    //When you gain mana from Drain Mana or Life Tap spells, your pet gains 100 % of the mana you gain. (max rank)
	class ManaFeedTestImpt : public TestCase
	{
	public:
		ManaFeedTestImpt() : TestCase(STATUS_KNOWN_BUG) { } //0 mana given to pet

		void AssertManaFeed(TestPlayer* warlock, TestPlayer* enemy, uint32 summonSpell)
		{
            TEST_CAST(warlock, warlock, summonSpell, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
			Pet* pet = warlock->GetPet();
			TEST_ASSERT(pet != nullptr);

			pet->DisableRegeneration(true);
            warlock->DisableRegeneration(true);
			pet->SetPower(POWER_MANA, 0);
            uint32 baseMana = 2000;
            warlock->SetPower(POWER_MANA, baseMana);
            enemy->SetPower(POWER_MANA, baseMana);

			// Drain Mana -- bug here
			FORCE_CAST(warlock, enemy, ClassSpells::Warlock::DRAIN_MANA_RNK_6, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            WaitNextUpdate();
			Wait(5000); //Drain mana has 5s cast time
            ASSERT_INFO("warlock did not gain mana?");
            TEST_ASSERT(warlock->GetPower(POWER_MANA) > baseMana);
            uint32 const warlockManaGain = warlock->GetPower(POWER_MANA) - baseMana;
            ASSERT_INFO("Drain Mana, Pet invoqued with %u, should have %u MP but has %u MP.", summonSpell, warlockManaGain, pet->GetPower(POWER_MANA));
			TEST_ASSERT(pet->GetPower(POWER_MANA) == warlockManaGain);

			// Life Tap
            pet->SetPower(POWER_MANA, 0);
            uint32 const spellLevel = 68;
            uint32 const perLevelPoint = 1;
            uint32 const perLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * perLevelPoint;
            uint32 const expectedManaGained = ClassSpellsDamage::Warlock::LIFE_TAP_RNK_7 + perLevelGain;
			TEST_CAST(warlock, warlock, ClassSpells::Warlock::LIFE_TAP_RNK_7, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            ASSERT_INFO("Life Tap Pet invoqued with %u, should have %u MP but has %u MP.", summonSpell, expectedManaGained, pet->GetPower(POWER_MANA));
			TEST_ASSERT(pet->GetPower(POWER_MANA) == expectedManaGained);
		}

		void Test() override
		{
			TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 10.0f);
			TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawnPosition);

			LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(warlock, Talents::Warlock::MANA_FEED_RNK_3);
			AssertManaFeed(warlock, enemy, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			AssertManaFeed(warlock, enemy, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			AssertManaFeed(warlock, enemy, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			AssertManaFeed(warlock, enemy, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1);
			AssertManaFeed(warlock, enemy, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ManaFeedTestImpt>();
	}
};

class DemonicKnowledgeTest : public TestCaseScript
{
public:
	DemonicKnowledgeTest() : TestCaseScript("talents warlock demonic_knowledge") { }

    //"Increases your spell damage by an amount equal to 12% of the total of your active demon's Stamina plus Intellect." (max rank)
	class DemonicKnowledgeTestImpt : public TestCase
	{
	public:
		DemonicKnowledgeTestImpt() : TestCase(STATUS_KNOWN_BUG) { } //No SP given

		void AssertDemonicKnowledge(TestPlayer* warlock, uint32 summonSpellId, float spellPower, float talentFactor)
		{
			TEST_CAST(warlock, warlock, summonSpellId, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
			Pet* pet = warlock->GetPet();
			TEST_ASSERT(pet != nullptr);

            Wait(1); //dunno if needed

			float const expectedSP = spellPower + (pet->GetStat(STAT_STAMINA) + pet->GetStat(STAT_INTELLECT)) * talentFactor;
            ASSERT_INFO("After %u, Warlock should have %f SP but has %f SP.", summonSpellId, expectedSP, warlock->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW));
			TEST_ASSERT(Between<float>(warlock->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW), expectedSP - 0.1f, expectedSP + 0.1f));
		}

		void Test() override
		{
			TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

			float const spellPower = warlock->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            float const talentFactor = 0.12f;

			LearnTalent(warlock, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(warlock, Talents::Warlock::DEMONIC_KNOWLEDGE_RNK_3);

			AssertDemonicKnowledge(warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1, spellPower, talentFactor);
			AssertDemonicKnowledge(warlock, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, spellPower, talentFactor);
			AssertDemonicKnowledge(warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, spellPower, talentFactor);
			AssertDemonicKnowledge(warlock, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, spellPower, talentFactor);
			AssertDemonicKnowledge(warlock, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, spellPower, talentFactor);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicKnowledgeTestImpt>();
	}
};

class ImprovedShadowBoltTest : public TestCaseScript
{
public:
    ImprovedShadowBoltTest() : TestCaseScript("talents warlock improved_shadow_bolt") { }

    //"Your Shadow Bolt critical strikes increase Shadow damage dealt to the target by 20% until 4 non-periodic damage sources are applied. Effect lasts a maximum of 12sec." (max rank)
    class ImprovedShadowBoltTestImpt : public TestCase
    {
    public:
        ImprovedShadowBoltTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { } //Todo: Test if buff is consumed + also consumed by another spell such as priest mind blast

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::IMPROVED_SHADOW_BOLT_RNK_5);
            float const talentFactor = 1.2f;
            EnableCriticals(warlock, true);

            uint32 const IMPROVED_SHADOW_BOLT_RNK_5_PROC = 17800;

            // Buff is applied
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED | TRIGGERED_PROC_AS_NON_TRIGGERED));
            TEST_AURA_MAX_DURATION(dummy, IMPROVED_SHADOW_BOLT_RNK_5_PROC, Seconds(12));

            uint32 expectedMinDamage = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * talentFactor * 1.5f;
            uint32 expectedMaxDamage = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * talentFactor * 1.5f;
            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedMinDamage, expectedMaxDamage, true, [IMPROVED_SHADOW_BOLT_RNK_5_PROC](Unit* caster, Unit* target) {
                if(!target->HasAura(IMPROVED_SHADOW_BOLT_RNK_5_PROC))
                    target->AddAura(IMPROVED_SHADOW_BOLT_RNK_5_PROC, target);
            });
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedShadowBoltTestImpt>();
    }
};

class CataclysmTest : public TestCaseScript
{
public:
    CataclysmTest() : TestCaseScript("talents warlock cataclysm") { }

    //Reduces the Mana cost of your Destruction spells by 5% (max ranks)
    class CataclysmTestImpt : public TestCase
    {
    public:
        CataclysmTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            LearnTalent(warlock, Talents::Warlock::CATACLYSM_RNK_5);
            float const talentFactor = 1 - 0.05f;

            TEST_POWER_COST(warlock, ClassSpells::Warlock::CONFLAGRATE_RNK_6, POWER_MANA, 305 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::HELLFIRE_RNK_4, POWER_MANA, 1665 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::IMMOLATE_RNK_9, POWER_MANA, 445 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::INCINERATE_RNK_2, POWER_MANA, 355 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, POWER_MANA, 1480 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SEARING_PAIN_RNK_8, POWER_MANA, 205 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, POWER_MANA, 420 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SHADOWBURN_RNK_8, POWER_MANA, 515 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SHADOWFURY_RNK_3, POWER_MANA, 710 * talentFactor);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SOUL_FIRE_RNK_4, POWER_MANA, 250 * talentFactor);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CataclysmTestImpt>();
    }
};

class BaneTest : public TestCaseScript
{
public:
    BaneTest() : TestCaseScript("talents warlock bane") { }

    //"Reduces the casting time of your Shadow Bolt and Immolate spells by 0.5 sec and your Soul Fire spell by 2 sec."
    class BaneTestImpt : public TestCase
    {
    public:
        BaneTestImpt() : TestCase(STATUS_PASSING) { }

        void TestCorruptionCastTime(TestPlayer* caster, uint32 talentSpellId, uint32 talentPoint, uint32 shadowbBoltImmolateFactor, uint32 soulFireFactor)
        {
            LearnTalent(caster, talentSpellId);
            uint32 const shadowBolt = 3000 - talentPoint * shadowbBoltImmolateFactor;
            TEST_CAST_TIME(caster, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, shadowBolt);
            uint32 const immolate = 2000 - talentPoint * shadowbBoltImmolateFactor;
            TEST_CAST_TIME(caster, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, shadowBolt);
            uint32 const soulFire = 6000 - talentPoint * soulFireFactor;
            TEST_CAST_TIME(caster, ClassSpells::Warlock::SOUL_FIRE_RNK_4, soulFire);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnRandomPlayer(CLASS_WARLOCK);

            uint32 const shadowbBoltCastTimeReducedPerTalentPoint = 100;
            uint32 const soulFireCastTimeReducedPerTalentPoint = 400;

            TestCorruptionCastTime(warlock, Talents::Warlock::BANE_RNK_1, 1, shadowbBoltCastTimeReducedPerTalentPoint, soulFireCastTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::BANE_RNK_2, 2, shadowbBoltCastTimeReducedPerTalentPoint, soulFireCastTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::BANE_RNK_3, 3, shadowbBoltCastTimeReducedPerTalentPoint, soulFireCastTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::BANE_RNK_4, 4, shadowbBoltCastTimeReducedPerTalentPoint, soulFireCastTimeReducedPerTalentPoint);
            TestCorruptionCastTime(warlock, Talents::Warlock::BANE_RNK_5, 5, shadowbBoltCastTimeReducedPerTalentPoint, soulFireCastTimeReducedPerTalentPoint);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BaneTestImpt>();
    }
};

class AftermathTest : public TestCaseScript
{
public:
    AftermathTest() : TestCaseScript("talents warlock aftermath") { } // Never procs

    class AftermathTestImpt : public TestCase
    {
    public:
        AftermathTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::AFTERMATH_RNK_5);
            uint32 const spellProcId = 18118;
            float const procChance = 10.0f;

            TEST_SPELL_PROC_CHANCE_CALLBACK(warlock, dummy, ClassSpells::Warlock::CONFLAGRATE_RNK_6, spellProcId, false, procChance, SPELL_MISS_NONE, false, [](Unit* caster, Unit* target) {
                caster->CastSpell(target, ClassSpells::Warlock::IMMOLATE_RNK_9, TRIGGERED_FULL_MASK);
            });
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOWFURY_RNK_3, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, spellProcId, false, procChance, SPELL_MISS_NONE, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AftermathTestImpt>();
    }
};

class ImprovedFireboltTest : public TestCaseScript
{
public:
    ImprovedFireboltTest() : TestCaseScript("talents warlock improved_firebolt") { }

    class ImprovedFireboltTestImpt : public TestCase
    {
    public:
        ImprovedFireboltTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);

            LearnTalent(warlock, Talents::Warlock::IMPROVED_FIREBOLT_RNK_2);

            TEST_CAST(warlock, warlock, ClassSpells::Warlock::SUMMON_IMP_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
            Guardian* imp = warlock->GetGuardianPet();
            TEST_ASSERT(imp != nullptr);

            TEST_CAST_TIME(imp, ClassSpells::Warlock::IMP_FIREBOLT_RNK_8, 2500);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedFireboltTestImpt>();
    }
};

class ImprovedLashOfPainTest : public TestCaseScript
{
public:
    ImprovedLashOfPainTest() : TestCaseScript("talents warlock improved_lash_of_pain") { }

    class ImprovedLashOfPainTestImpt : public TestCase
    {
    public:
        ImprovedLashOfPainTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::IMPROVED_FIREBOLT_RNK_2);

            TEST_CAST(warlock, warlock, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            WaitNextUpdate();
            Guardian* succubus = warlock->GetGuardianPet();
            TEST_ASSERT(succubus != nullptr);

            TEST_COOLDOWN(succubus, dummy, ClassSpells::Warlock::SUCCUBUS_LASH_OF_PAIN_RNK_7, Seconds(6));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedLashOfPainTestImpt>();
    }
};

class DevastationTest : public TestCaseScript
{
public:
    DevastationTest() : TestCaseScript("talents warlock devastation") { }

    class DevastationTestImpt : public TestCase
    {
    public:
        DevastationTestImpt() : TestCase(STATUS_WIP) { } // Bugs on Immolate, Incinerate, Searing Pain, Shadow Bolt, Shadowfury

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::DEVASTATION_RNK_5);
            float const talentFactor = 5.0f;
            float const expectedSpellCritChance = warlock->GetFloatValue(PLAYER_CRIT_PERCENTAGE) + talentFactor;

            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_SPELL_CRIT_CHANCE_CALLBACK(warlock, dummy, ClassSpells::Warlock::CONFLAGRATE_RNK_6, expectedSpellCritChance, [](Unit* caster, Unit* target) {
                caster->CastSpell(target, ClassSpells::Warlock::IMMOLATE_RNK_9, TRIGGERED_FULL_MASK);
            });
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedSpellCritChance);
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedSpellCritChance);
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, expectedSpellCritChance);
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSpellCritChance);
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, expectedSpellCritChance);
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SHADOWFURY_RNK_3, expectedSpellCritChance);
            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, expectedSpellCritChance);
            warlock->ResetForceSpellHitResult();
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DevastationTestImpt>();
    }
};

class ShadowburnTest : public TestCaseScript
{
public:
    ShadowburnTest() : TestCaseScript("talents warlock shadowburn") { }

    class ShadowburnTestImpt : public TestCase
    {
    public:
        ShadowburnTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedShadowburnManaCost = 515;
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SHADOWBURN_RNK_8, POWER_MANA, expectedShadowburnManaCost);
            
            warlock->AddItem(SOUL_SHARD, 1);
            TEST_CAST(warlock, enemy, ClassSpells::Warlock::SHADOWBURN_RNK_8, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            // Consumes a Soul Shard
            TEST_ASSERT(warlock->GetItemCount(SOUL_SHARD, false) == 0);
            TEST_HAS_COOLDOWN(warlock, ClassSpells::Warlock::SHADOWBURN_RNK_8, Seconds(15));
            TEST_AURA_MAX_DURATION(enemy, ClassSpells::Warlock::SHADOWBURN_RNK_8_TRIGGER, Seconds(5));
            Wait(Seconds(2));
            enemy->KillSelf();
            // Gives shard if affected target dies within 5sec
            TEST_ASSERT(warlock->GetItemCount(SOUL_SHARD, false) == 1);

            // Damage
            float const spellCoefficient = ClassSpellsCoeff::Warlock::SHADOWBURN;
            uint32 const expectedShadowburnMin = ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MIN + spellPower * spellCoefficient;
            uint32 const expectedShadowburnMax = ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MAX + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, expectedShadowburnMin, expectedShadowburnMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, expectedShadowburnMin * 1.5f, expectedShadowburnMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShadowburnTestImpt>();
    }
};

class WarlockIntensityTest : public TestCaseScript
{
public:

    WarlockIntensityTest() : TestCaseScript("talents warlock intensity") { }

    class WarlockIntensityTestImpt : public TestCase
    {
    public:
        WarlockIntensityTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            LearnTalent(warlock, Talents::Warlock::INTENSITY_RNK_2);
            float const resistPushBackChance = 70.f;

            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::HELLFIRE_RNK_4, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::IMMOLATE_RNK_9, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::INCINERATE_RNK_2, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::SEARING_PAIN_RNK_8, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, resistPushBackChance);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::SHADOWFURY_RNK_3, resistPushBackChance);
            warlock->AddItem(SOUL_SHARD, 1);
            TEST_PUSHBACK_RESIST_CHANCE(warlock, rogue, ClassSpells::Warlock::SOUL_FIRE_RNK_4, resistPushBackChance);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<WarlockIntensityTestImpt>();
    }
};

class DestructiveReachTest : public TestCaseScript
{
public:
    DestructiveReachTest() : TestCaseScript("talents warlock destructive_reach") { }

    class DestructiveReachTestImpt : public TestCase
    {
    public:
        /*
        Bug:
            - Talent is working, Searing Pain spell deals too much threat.
        */
        DestructiveReachTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Position spawn36(_location);
            spawn36.MoveInFront(_location, 36.0f);
            Creature* dummy36 = SpawnCreatureWithPosition(spawn36);

            Position spawn24(_location);
            spawn24.MoveInFront(_location, 24.0f);
            Creature* dummy24 = SpawnCreatureWithPosition(spawn24);

            // Out of Range
            TEST_CAST(warlock, dummy36, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_FAILED_OUT_OF_RANGE);

            LearnTalent(warlock, Talents::Warlock::DESTRUCTIVE_REACH_RNK_2);
            float const threatFactor = 1 - 0.1f;

            // 36m + Threat
            TEST_THREAT_CALLBACK(warlock, dummy36, ClassSpells::Warlock::CONFLAGRATE_RNK_6, threatFactor, false, [](Unit* caster, Unit* target) {
                caster->AddAura(ClassSpells::Warlock::IMMOLATE_RNK_9, target);
            });
            TEST_THREAT(warlock, dummy36, ClassSpells::Warlock::IMMOLATE_RNK_9, threatFactor, false);
            TEST_THREAT(warlock, dummy36, ClassSpells::Warlock::INCINERATE_RNK_2, threatFactor, false);
            TEST_THREAT(warlock, dummy36, ClassSpells::Warlock::SEARING_PAIN_RNK_8, 2.f * threatFactor, false);
            TEST_THREAT(warlock, dummy36, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, threatFactor, false);
            TEST_THREAT(warlock, dummy24, ClassSpells::Warlock::SHADOWBURN_RNK_8, threatFactor, false);
            TEST_THREAT(warlock, dummy36, ClassSpells::Warlock::SHADOWFURY_RNK_3, threatFactor, false);
            TEST_THREAT(warlock, dummy36, ClassSpells::Warlock::SOUL_FIRE_RNK_4, threatFactor, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DestructiveReachTestImpt>();
    }
};

class ImprovedSearingPainTest : public TestCaseScript
{
public:
    ImprovedSearingPainTest() : TestCaseScript("talents warlock improved_searing_pain") { }

    class ImprovedSearingPainTestImpt : public TestCase
    {
    public:
        ImprovedSearingPainTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::IMPROVED_SEARING_PAIN_RNK_3);
            float const talentFactor = 10.0f;
            float const expectedSpellCritChance = warlock->GetFloatValue(PLAYER_CRIT_PERCENTAGE) + talentFactor;

            TEST_SPELL_CRIT_CHANCE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, expectedSpellCritChance);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedSearingPainTestImpt>();
    }
};

class PyroclasmTest : public TestCaseScript
{
public:
    PyroclasmTest() : TestCaseScript("talents warlock pyroclasm") { }

    class PyroclasmTestImpt : public TestCase
    {
    public:
        PyroclasmTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::PYROCLASM_RNK_2);
            uint32 const spellProcId = 18093;
            float const procChance = 26.f;
            
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            // WoWWiki: Since Rain of Fire and Hellfire do damage over time, the chance to stun is distributed over all the ticks of the spell, not 13%/26% per tick.
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::HELLFIRE_RNK_4_TRIGGER, spellProcId, false, procChance / 15.f, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(warlock, dummy, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5_PROC, spellProcId, false, procChance / 4.f, SPELL_MISS_NONE, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PyroclasmTestImpt>();
    }
};

class ImprovedImmolateTest : public TestCaseScript
{
public:
    ImprovedImmolateTest() : TestCaseScript("talents warlock improved_immolate") { }

    class ImprovedImmolateTestImpt : public TestCase
    {
    public:
        ImprovedImmolateTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::IMPROVED_IMMOLATE_RNK_5);
            float const talentFactor = 1.25f;

            // Direct damage
            uint32 const expectedImprovedImmolateDirect = ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_LVL_70 * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedImprovedImmolateDirect, expectedImprovedImmolateDirect, false);
            // Make sure DoT isnt boosted
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_DOT, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImprovedImmolateTestImpt>();
    }
};

class RuinTest : public TestCaseScript
{
public:
    RuinTest() : TestCaseScript("talents warlock ruin") { }

    class RuinTestImpt : public TestCase
    {
    public:
        RuinTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::RUIN_RNK_1);

            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(warlock, dummy, ClassSpells::Warlock::CONFLAGRATE_RNK_6, ClassSpellsDamage::Warlock::CONFLAGRATE_RNK_6_MIN * 2.f, ClassSpellsDamage::Warlock::CONFLAGRATE_RNK_6_MAX * 2.f, true, [](Unit* caster, Unit* target) {
                caster->CastSpell(target, ClassSpells::Warlock::IMMOLATE_RNK_9, TRIGGERED_FULL_MASK);
            });
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_LVL_70 * 2.f, ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_LVL_70 * 2.f, true);
            dummy->RemoveAllAuras(); // Removing Immolate that would fail Incinerate test
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * 2.f, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * 2.f, true);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MIN * 2.f, ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MAX * 2.f, true);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * 2.f, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * 2.f, true);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWBURN_RNK_8, ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MIN * 2.f, ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MAX * 2.f, true);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWFURY_RNK_3, ClassSpellsDamage::Warlock::SHADOWFURY_RNK_3_MIN * 2.f, ClassSpellsDamage::Warlock::SHADOWFURY_RNK_3_MAX * 2.f, true);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MIN * 2.f, ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MAX * 2.f, true);
            warlock->ResetForceSpellHitResult();
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RuinTestImpt>();
    }
};

class NetherProtectionTest : public TestCaseScript
{
public:
    NetherProtectionTest() : TestCaseScript("talents warlock nether_protection") { }

    class NetherProtectionTestImpt : public TestCase
    {
    public:
        NetherProtectionTestImpt() : TestCase(STATUS_KNOWN_BUG) { } // Never procs

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            LearnTalent(warlock, Talents::Warlock::NETHER_PROTECTION_RNK_3);
            uint32 const spellProcId = 30300;
            float const procChance = 30.0f;

            // Immune to Fire and Shadow for 4sec
            warlock->AddAura(spellProcId, warlock);
            TEST_AURA_MAX_DURATION(warlock, spellProcId, Seconds(4));
            uint32 warlockHealth = warlock->GetHealth();
            FORCE_CAST(enemy, warlock, ClassSpells::Warlock::INCINERATE_RNK_2, SPELL_MISS_NONE, TRIGGERED_FULL_DEBUG_MASK);
            FORCE_CAST(enemy, warlock, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_MISS_NONE, TRIGGERED_FULL_DEBUG_MASK);
            TEST_ASSERT(warlock->GetHealth() == warlockHealth);

            // Proc chance
            TEST_SPELL_PROC_CHANCE(enemy, warlock, ClassSpells::Warlock::INCINERATE_RNK_2, spellProcId, false, procChance, SPELL_MISS_NONE, false);
            TEST_SPELL_PROC_CHANCE(enemy, warlock, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, spellProcId, false, procChance, SPELL_MISS_NONE, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<NetherProtectionTestImpt>();
    }
};

class EmberstormTest : public TestCaseScript
{
public:
    EmberstormTest() : TestCaseScript("talents warlock emberstorm") { }

    class EmberstormTestImpt : public TestCase
    {
    public:
        EmberstormTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::EMBERSTORM_RNK_5);
            float const damageFactor = 1.1f;
            warlock->SetMaxHealth(uint32(1000000));
            warlock->SetFullHealth();

            // Incinerate cast time reduces by 10%
            TEST_CAST_TIME(warlock, ClassSpells::Warlock::INCINERATE_RNK_2, 2500 * 0.9f);

            // Fire damage increased by 10%
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(warlock, dummy, ClassSpells::Warlock::CONFLAGRATE_RNK_6, ClassSpellsDamage::Warlock::CONFLAGRATE_RNK_6_MIN * damageFactor, ClassSpellsDamage::Warlock::CONFLAGRATE_RNK_6_MAX * damageFactor, false, [](Unit* caster, Unit* target) {
                caster->CastSpell(target, ClassSpells::Warlock::IMMOLATE_RNK_9, TRIGGERED_FULL_MASK);
            });
            warlock->ResetForceSpellHitResult();
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_LVL_70 * damageFactor, ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_LVL_70 * damageFactor, false);
            dummy->RemoveAllAuras(); // Removing Immolate that would fail Incinerate test
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * damageFactor, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * damageFactor, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MIN * damageFactor, ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MAX * damageFactor, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MIN * damageFactor, ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MAX * damageFactor, false);
            // Helfire
            TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::HELLFIRE_RNK_4, ClassSpells::Warlock::HELLFIRE_RNK_4_TRIGGER, 15, ClassSpellsDamage::Warlock::HELLFIRE_RNK_4_TICK_LVL_70 * damageFactor);
            TEST_DOT_DAMAGE(warlock, warlock, ClassSpells::Warlock::HELLFIRE_RNK_4, ClassSpellsDamage::Warlock::HELLFIRE_RNK_4_TICK_LVL_70 * damageFactor * 15.f, false);
            TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5_PROC, 4, ClassSpellsDamage::Warlock::RAIN_OF_FIRE_RNK_5_TICK_LVL_70 * damageFactor);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<EmberstormTestImpt>();
    }
};

class BacklashTest : public TestCaseScript
{
public:
    BacklashTest() : TestCaseScript("talents warlock backlash") { }

    class BacklashTestImpt : public TestCase
    {
    public:
        BacklashTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            float const expectedSpellCritChance = warlock->GetFloatValue(PLAYER_CRIT_PERCENTAGE) + 3.0f;

            LearnTalent(warlock, Talents::Warlock::BACKLASH_RNK_3);
            float const talentFactor = 5.0f;

            uint32 const backlashSpellProcId = 34936;

            // +3% spell crit
            TEST_ASSERT(warlock->GetFloatValue(PLAYER_CRIT_PERCENTAGE) == expectedSpellCritChance);

            // Provides instant Shadow Bolt or Incinerate
            warlock->AddAura(backlashSpellProcId, warlock);
            TEST_CAST_TIME(warlock, ClassSpells::Warlock::INCINERATE_RNK_2, uint32(0));
            TEST_CAST_TIME(warlock, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, uint32(0));

            // Proc
            // Rogue attack
            // after each attack:
            // if aura auraCount++ + remove aura
            // totalcount++
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BacklashTestImpt>();
    }
};

class ConflagrateTest : public TestCaseScript
{
public:
    ConflagrateTest() : TestCaseScript("talents warlock conflagrate") { }

    class ConflagrateTestImpt : public TestCase
    {
    public:
        ConflagrateTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedConflagrateManaCost = 305;
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_POWER_COST(warlock, ClassSpells::Warlock::CONFLAGRATE_RNK_6, POWER_MANA, expectedConflagrateManaCost);

            // Damage
            float const spellBonus = spellPower * ClassSpellsCoeff::Warlock::CONFLAGRATE;
            uint32 const expectedConflagrateMin = ClassSpellsDamage::Warlock::CONFLAGRATE_RNK_6_MIN + spellBonus;
            uint32 const expectedConflagrateMax = ClassSpellsDamage::Warlock::CONFLAGRATE_RNK_6_MAX + spellBonus;
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(warlock, dummy, ClassSpells::Warlock::CONFLAGRATE_RNK_6, expectedConflagrateMin, expectedConflagrateMax, false, [](Unit* caster, Unit* target) {
                caster->CastSpell(target, ClassSpells::Warlock::IMMOLATE_RNK_9, TRIGGERED_FULL_MASK);
            });
            warlock->ResetForceSpellHitResult();
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ConflagrateTestImpt>();
    }
};

class ShadowAndFlamesTest : public TestCaseScript
{
public:
    ShadowAndFlamesTest() : TestCaseScript("talents warlock shadow_and_flames") { }

    class ShadowAndFlamesTestImpt : public TestCase
    {
    public:
        ShadowAndFlamesTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            LearnTalent(warlock, Talents::Warlock::SHADOW_AND_FLAME_RNK_5);
            float const talentFactor = 0.2f;

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            // Incinerate
            float const castTime = 2.5f;
            float const spellCoefficient = castTime / 3.5f + talentFactor;
            uint32 const expectedIncinerateMin = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN + spellPower * spellCoefficient;
            uint32 const expectedIncinerateMax = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin, expectedIncinerateMax, false);

            // Shadow Bolt
            float const sbCastTime = 3.f;
            float const sbSpellCoefficient = sbCastTime / 3.5f + talentFactor;
            uint32 const expectedSBMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + spellPower * sbSpellCoefficient;
            uint32 const expectedSBMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + spellPower * sbSpellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSBMin, expectedSBMax, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShadowAndFlamesTestImpt>();
    }
};

class ShadowfuryTest : public TestCaseScript
{
public:
    ShadowfuryTest() : TestCaseScript("talents warlock shadowfury") { }

    class ShadowfuryTestImpt : public TestCase
    {
    public:
        ShadowfuryTestImpt() : TestCase(STATUS_KNOWN_BUG) { } // Spell Coeff seems slightly off

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedShadowfuryManaCost = 710;
            TEST_POWER_COST(warlock, ClassSpells::Warlock::SHADOWFURY_RNK_3, POWER_MANA, expectedShadowfuryManaCost);

            // Damage
            float const spellBonus = spellPower * ClassSpellsCoeff::Warlock::SHADOWFURY; // DrDamage
            uint32 const expectedShadowfuryMin = ClassSpellsDamage::Warlock::SHADOWFURY_RNK_3_MIN + spellBonus;
            uint32 const expectedShadowfuryMax = ClassSpellsDamage::Warlock::SHADOWFURY_RNK_3_MAX + spellBonus;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOWFURY_RNK_3, expectedShadowfuryMin, expectedShadowfuryMax, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShadowfuryTestImpt>();
    }
};

void AddSC_test_talents_warlock()
{
	// Affliction
    new SuppressionTest();
    new ImprovedCorruptionTest();
    new ImprovedCurseOfWeaknessTest();
    new ImprovedDrainSoulTest();
	new ImprovedLifeTapTest();
    new SoulSiphonTest();
    new ImprovedCurseOfAgonyTest();
    new FelConcentrationTest();
    new AmplifyCurseTest();
    new GrimReachTest();
    new NightfallTest();
	new EmpoweredCorruptionTest();
    new ShadowEmbraceTest();
    new SiphonLifeTest();
    new CurseOfExhaustionTest();
	new ShadowMasteryTest();
	new ContagionTest();
    new DarkPactTest();
    new ImprovedHowOfTerrorTest();
    new MaledictionTest();
    new UnstableAfflictionTest();
	// Demonology
	new ImprovedHealthstoneTest();
	new DemonicEmbraceTest();
	new ImprovedHealthFunnelTest();
	new FelIntellectTest();
	new FelDominationTest();
	new FelStaminaTest();
	new DemonicAegisTest();
	new MasterSummonerTest();
	new DemonicSacrificeTest();
	new DemonicKnowledgeTest();
	new ManaFeedTest();
    // Destruction
    new ImprovedShadowBoltTest();
    new CataclysmTest();
    new BaneTest();
    new AftermathTest();
    new ImprovedFireboltTest();
    new ImprovedLashOfPainTest();
    new DevastationTest();
    new ShadowburnTest();
    new WarlockIntensityTest();
    new DestructiveReachTest();
    new ImprovedSearingPainTest();
    new PyroclasmTest();
    new ImprovedImmolateTest();
    new RuinTest();
    new NetherProtectionTest();
    new EmberstormTest();
    // TODO: Backlash
    new ConflagrateTest();
    // TODO: Soul Leech
    new ShadowAndFlamesTest();
    new ShadowfuryTest();
}
