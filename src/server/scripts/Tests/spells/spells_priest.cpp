#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"

class DispelMagicTest : public TestCaseScript
{
public:
    DispelMagicTest() : TestCaseScript("spells priest dispel_magic") { }

    class DispelMagicTestImpt : public TestCase
    {
    public:
        DispelMagicTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* enemy2 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

            // Fail -- Bug here
            TEST_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_FAILED_NOTHING_TO_DISPEL, TRIGGERED_IGNORE_GCD);
            TEST_CAST(priest, enemy1, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, SPELL_FAILED_NOTHING_TO_DISPEL, TRIGGERED_IGNORE_GCD);

            // Setup
            TEST_CAST(enemy1, enemy1, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy1, enemy1, ClassSpells::Priest::FEAR_WARD_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy1, enemy1, ClassSpells::Priest::INNER_FIRE_RNK_7, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy1, priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy2, priest, ClassSpells::Druid::MOONFIRE_RNK_12, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_CAST(enemy2, priest, ClassSpells::Druid::INSECT_SWARM_RNK_6, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);

            // Offensive dispel
            uint32 const expectedDispelMagicMana = 366;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2, POWER_MANA, expectedDispelMagicMana);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Druid::INSECT_SWARM_RNK_6);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Druid::MOONFIRE_RNK_12);
            TEST_HAS_AURA(priest, ClassSpells::Priest::SHADOW_WORD_PAIN_RNK_10);
            // Defensive dispel
            TEST_CAST(priest, priest, ClassSpells::Priest::DISPEL_MAGIC_RNK_2);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::INNER_FIRE_RNK_7);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_HAS_AURA(priest, ClassSpells::Priest::DIVINE_SPIRIT_RNK_5);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DispelMagicTestImpt>();
    }
};

class FearWardTest : public TestCaseScript
{
public:
    FearWardTest() : TestCaseScript("spells priest fear_ward") { }

    class FearWardTestImpt : public TestCase
    {
    public:
        FearWardTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            // Mana cost
            uint32 const expectedFearWardMana = 78;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::FEAR_WARD_RNK_1, POWER_MANA, expectedFearWardMana);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, EFFECT_0, 3 * MINUTE * IN_MILLISECONDS);

            // Cooldown
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::FEAR_WARD_RNK_1, 3 * MINUTE);

            // Fear
            TEST_CAST(warlock, priest, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::FEAR_WARD_RNK_1);
            TEST_CAST(warlock, priest, ClassSpells::Warlock::FEAR_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_AURA(priest, ClassSpells::Warlock::FEAR_RNK_3);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FearWardTestImpt>();
    }
};

class InnerFireTest : public TestCaseScript
{
public:
    InnerFireTest() : TestCaseScript("spells priest inner_fire") { }

    class InnerFireTestImpt : public TestCase
    {
    public:
        InnerFireTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            priest->DisableRegeneration(true);

            uint32 innerFireBonus = 1580;
            uint32 expectedArmor = priest->GetArmor() + innerFireBonus;

            // Mana cost
            uint32 const expectedInnerFireMana = 375;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::INNER_FIRE_RNK_7, POWER_MANA, expectedInnerFireMana);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, EFFECT_0, 10 * MINUTE * IN_MILLISECONDS);

            // Charges
            TEST_AURA_CHARGE(priest, ClassSpells::Priest::INNER_FIRE_RNK_7, EFFECT_0, 20);

            // Armor
            TEST_ASSERT(priest->GetArmor() == expectedArmor);

            // Lose charge
            auto AI = rogue->GetTestingPlayerbotAI();
            int hits = 0;
            for (uint32 i = 0; i < 150; i++)
            {
                rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                auto damageToTarget = AI->GetWhiteDamageDoneInfo(priest);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();
                if (data.damageInfo.hitOutCome != MELEE_HIT_NORMAL && data.damageInfo.hitOutCome != MELEE_HIT_CRIT)
                    continue;
                hits++;
                Aura* aura = priest->GetAura(ClassSpells::Priest::INNER_FIRE_RNK_7, EFFECT_0);
                if (hits < 20)
                {
                    TEST_ASSERT(aura != nullptr);
                    int currentCharges = aura->GetCharges();
                    TEST_ASSERT(currentCharges == 20 - hits);
                }
                else
                {
                    TEST_ASSERT(aura == nullptr);
                    TEST_ASSERT(!priest->HasAura(ClassSpells::Priest::INNER_FIRE_RNK_7));
                    break;
                }
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<InnerFireTestImpt>();
    }
};

class LevitateTest : public TestCaseScript
{
public:
    LevitateTest() : TestCaseScript("spells priest levitate") { }

    class LevitateTestImpt : public TestCase
    {
    public:
        LevitateTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            priest->DisableRegeneration(true);

            // Mana cost
            uint32 const expectedLevitateMana = 100;
            priest->AddItem(17056, 1); // Reagent Light Feather
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::LEVITATE_RNK_1, POWER_MANA, expectedLevitateMana);
            TEST_ASSERT(priest->GetItemCount(17056, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::LEVITATE_RNK_1, EFFECT_0, 2 * MINUTE * IN_MILLISECONDS);

            // Cancel on damage taken
            rogue->Attack(priest, true);
            Wait(500);
            rogue->AttackStop();
            TEST_ASSERT(!priest->HasAura(ClassSpells::Priest::LEVITATE_RNK_1));

            // TODO: over water, slow fall
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<LevitateTestImpt>();
    }
};

class ManaBurnTest : public TestCaseScript
{
public:
    ManaBurnTest() : TestCaseScript("spells priest mana_burn") { }

    class ManaBurnTestImpt : public TestCase
    {
    public:
        ManaBurnTestImpt() : TestCase(STATUS_PASSING, true) { }

        void TestManaBurn(TestPlayer* priest, Unit* victim, uint32 expectedMin, uint32 expectedMax)
        {
            uint32 sampleSize;
            uint32 maxPredictionError;
            _GetApproximationParams(sampleSize, maxPredictionError, expectedMin, expectedMax);

            EnableCriticals(priest, true);

            uint32 victimMaxHealth = victim->GetMaxHealth();
            uint32 victimMaxMana = victim->GetMaxPower(POWER_MANA);

            uint32 minBurn = std::numeric_limits<uint32>::max();
            uint32 maxBurn = 0;

            priest->ForceSpellHitResult(SPELL_MISS_NONE);
            for (uint32 i = 0; i < sampleSize; i++)
            {
                TEST_CAST(priest, victim, ClassSpells::Priest::MANA_BURN_RNK_7, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

                uint32 manaBurnt = victimMaxMana - victim->GetPower(POWER_MANA);
                minBurn = std::min(minBurn, manaBurnt);
                maxBurn = std::max(maxBurn, manaBurnt);

                uint32 expectedHealth = victimMaxHealth - manaBurnt / 2;
                ASSERT_INFO("Health: %u, expected: %u", victim->GetHealth(), expectedHealth);
                TEST_ASSERT(victim->GetHealth() == expectedHealth);

                // Reset
                victim->SetFullHealth();
                victim->SetPower(POWER_MANA, victim->GetMaxPower(POWER_MANA));
                Wait(10);
            }
            priest->ResetForceSpellHitResult();
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            EQUIP_ITEM(priest, 34336); // Sunflare - 292 SP
            priest->DisableRegeneration(true);
            mage->DisableRegeneration(true);
            Wait(5000);

            // Fail
            TEST_CAST(priest, rogue, ClassSpells::Priest::MANA_BURN_RNK_7, SPELL_FAILED_BAD_TARGETS);

            // Mana cost
            uint32 const expectedManaBurnMana = 355;
            TEST_POWER_COST(priest, mage, ClassSpells::Priest::MANA_BURN_RNK_7, POWER_MANA, expectedManaBurnMana);

            // Nothing on rogue
            TEST_ASSERT(rogue->GetHealth() == rogue->GetMaxHealth());

            TestManaBurn(priest, mage, ClassSpellsDamage::Priest::MANA_BURN_RNK_7_MIN, ClassSpellsDamage::Priest::MANA_BURN_RNK_7_MAX);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ManaBurnTestImpt>();
    }
};

void AddSC_test_spells_priest()
{
    // Discipline: 5/12
    new DispelMagicTest();
    new FearWardTest();
    new InnerFireTest();
    new LevitateTest();
    new ManaBurnTest();
}
