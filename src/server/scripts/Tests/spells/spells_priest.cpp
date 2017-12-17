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

class MassDispelTest : public TestCaseScript
{
public:
    MassDispelTest() : TestCaseScript("spells priest mass_dispel") { }

    class MassDispelTestImpt : public TestCase
    {
    public:
        MassDispelTestImpt() : TestCase(STATUS_INCOMPLETE, true) { }

        TestPlayer* SpawnShaman(Races race, Position spawn)
        {
            TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, race, 70, spawn);
            EQUIP_ITEM(shaman, 28523); // Totem
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::MANA_SPRING_TOTEM_RNK_5, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::GRACE_OF_AIR_TOTEM_RNK_3, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::STRENGTH_OF_EARTH_TOTEM_RNK_6, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(shaman, shaman, ClassSpells::Shaman::FROST_RESISTANCE_TOTEM_RNK_4, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            return shaman;
        }

        TestPlayer* SpawnHunter(Races race, Position spawn)
        {
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, race, 70, spawn);
            hunter->SummonPet(20673, hunter->GetPositionX(), hunter->GetPositionY(), hunter->GetPositionZ(), 0.0f, HUNTER_PET, 0); // Wind Serpent
            Pet* pet = hunter->GetPet();
            TEST_ASSERT(pet != nullptr);
            return hunter;
        }

        void Test() override
        {
            // Creature
            Creature* creature = SpawnCreature(6);

            // Priest
            TestPlayer* priestA = SpawnPlayer(CLASS_PRIEST, RACE_HUMAN);
            TestPlayer* priestH = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);

            // Setup
            Position spawn(_location);
            // Paladin
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* paladinA = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN, 70, spawn);
            TestPlayer* paladinH = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF, 70, spawn);
            // Mage
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* mageA = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawn);
            TestPlayer* mageH = SpawnPlayer(CLASS_MAGE, RACE_BLOODELF, 70, spawn);
            // Warlock
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* warlockA = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawn);
            TestPlayer* warlockH = SpawnPlayer(CLASS_WARLOCK, RACE_BLOODELF, 70, spawn);
            // Shaman
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* shamanA = SpawnShaman(RACE_DRAENEI, spawn);
            TestPlayer* shamanH = SpawnShaman(RACE_ORC, spawn);
            // Hunter
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* hunterA = SpawnHunter(RACE_DWARF, spawn);
            TestPlayer* hunterH = SpawnHunter(RACE_ORC, spawn);
            // Warrior
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* warriorA = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawn);
            TestPlayer* warriorH = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN, 70, spawn);
            // Druid
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* druidA = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawn);
            TestPlayer* druidH = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, 70, spawn);
            // Rogue
            spawn.MoveInFront(spawn, 3.0f);
            TestPlayer* rogueA = SpawnPlayer(CLASS_ROGUE, RACE_NIGHTELF, 70, spawn);
            TestPlayer* rogueH = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 70, spawn);

            Wait(5000);

            creature->Attack(priestA, true);
            TEST_CAST(creature, creature, 45855, SPELL_CAST_OK, TRIGGERED_FULL_MASK); // Gas Nova, Felmyst spell
            //TEST_HAS_AURA(priestH, 45855);
            TEST_HAS_AURA(priestA, 45855);
            TEST_HAS_AURA(shamanA, 45855);

            // from the center
            // affects pets, not totems
            // 1 dispel per target
            // up to 10 friendly player and 10 enemy
            // Remove Ice Block, Divine Shield, some banish?
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MassDispelTestImpt>();
    }
};

class PowerWordFortitudeTest : public TestCaseScript
{
public:
    PowerWordFortitudeTest() : TestCaseScript("spells priest power_word_fortitude") { }

    class PowerWordFortitudeTestImpt : public TestCase
    {
    public:
        PowerWordFortitudeTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            uint32 powerWordFortitudeBonus = 79;
            uint32 expectedHealth = warrior->GetMaxHealth() + powerWordFortitudeBonus * 10;

            // Mana cost
            uint32 const expectedPowerWordFortitudeMana = 700;
            TEST_POWER_COST(priest, warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, POWER_MANA, expectedPowerWordFortitudeMana);

            // Health
            ASSERT_INFO("Health: %u, expected: %u", warrior->GetHealth(), expectedHealth);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedHealth);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, EFFECT_0, 30 * MINUTE * IN_MILLISECONDS);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PowerWordFortitudeTestImpt>();
    }
};

class PowerWordShieldTest : public TestCaseScript
{
public:
    PowerWordShieldTest() : TestCaseScript("spells priest power_word_shield") { }

    class PowerWordShieldTestImpt : public TestCase
    {
    public:
        PowerWordShieldTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

            EQUIP_ITEM(priest, 34335); // Hammer of Sanctification -- 550 BH

            uint32 expectedAbsorb = ClassSpellsDamage::Priest::POWER_WORD_SHIELD_RNK_12 + ClassSpellsCoeff::Priest::POWER_WORD_SHIELD * 550;

            // Mana cost
            uint32 const expectedPowerWordShieldMana = 600;
            TEST_POWER_COST(priest, priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, POWER_MANA, expectedPowerWordShieldMana);

            // Aura
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, EFFECT_0, 30 * SECOND * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(priest, 6788, EFFECT_0, 15 * SECOND * IN_MILLISECONDS); // Weakened Aura

            // Cooldown
            TEST_HAS_COOLDOWN(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, 4 * SECOND);

            // Absorb
            priest->SetFullHealth();
            auto AI = rogue->GetTestingPlayerbotAI();
            int totalDamage = 0;
            for (uint32 i = 0; i < 150; i++)
            {
                rogue->AttackerStateUpdate(priest, BASE_ATTACK);
                auto damageToTarget = AI->GetWhiteDamageDoneInfo(priest);

                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();

                if (data.damageInfo.hitOutCome != MELEE_HIT_NORMAL && data.damageInfo.hitOutCome != MELEE_HIT_CRIT)
                    continue;

                totalDamage += data.damageInfo.damage;
                totalDamage += data.damageInfo.resist;
                totalDamage += data.damageInfo.blocked_amount;
                totalDamage += data.damageInfo.absorb;

                if (totalDamage < expectedAbsorb)
                {
                    TEST_HAS_AURA(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, EFFECT_0);
                    continue;
                }

                TEST_HAS_NOT_AURA(priest, ClassSpells::Priest::POWER_WORD_SHIELD_RNK_12, EFFECT_0);
                uint32 expectedHealth = priest->GetMaxHealth() - (totalDamage - expectedAbsorb);
                TEST_ASSERT(priest->GetHealth() == expectedHealth);
                break;
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PowerWordShieldTestImpt>();
    }
};

class PrayerOfFortitudeTest : public TestCaseScript
{
public:
    PrayerOfFortitudeTest() : TestCaseScript("spells priest prayer_of_fortitude") { }

    class PrayerOfFortitudeTestImpt : public TestCase
    {
    public:
        PrayerOfFortitudeTestImpt() : TestCase(STATUS_PASSING, true) { }

        void TestPrayerOfFortitude(TestPlayer* priest, Unit* warrior, uint32 spellId, uint32 reagentId, uint32 manaCost, uint32 staminaBonus, uint32 priestStartHealth, uint32 warriorStartHealth)
        {
            uint32 expectedPriestHealth = priestStartHealth + staminaBonus * 10;
            uint32 expectedWarriorHealth = warriorStartHealth + staminaBonus * 10;

            // Mana cost
            priest->AddItem(reagentId, 1); // Reagent
            TEST_POWER_COST(priest, warrior, spellId, POWER_MANA, manaCost);
            TEST_ASSERT(priest->GetItemCount(reagentId, false) == 0);

            // Aura
            TEST_AURA_MAX_DURATION(warrior, spellId, EFFECT_0, 1 * HOUR * IN_MILLISECONDS);
            TEST_AURA_MAX_DURATION(priest, spellId, EFFECT_0, 1 * HOUR * IN_MILLISECONDS);

            // Health
            TEST_ASSERT(priest->GetMaxHealth() == expectedPriestHealth);
            TEST_ASSERT(warrior->GetMaxHealth() == expectedWarriorHealth);
        }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_ORC);

            GroupPlayer(priest, warrior);

            uint32 const startPriestHealth = priest->GetMaxHealth();
            uint32 const startWarriorHealth = warrior->GetMaxHealth();

            uint32 const HOLY_CANDLE = 17028;
            uint32 const SACRED_CANDLE = 17029;

            TestPrayerOfFortitude(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_1, HOLY_CANDLE, 1200, 43, startPriestHealth, startWarriorHealth);
            TestPrayerOfFortitude(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_2, SACRED_CANDLE, 1500, 54, startPriestHealth, startWarriorHealth);
            TestPrayerOfFortitude(priest, warrior, ClassSpells::Priest::PRAYER_OF_FORTITUDE_RNK_3, SACRED_CANDLE, 1800, 79, startPriestHealth, startWarriorHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<PrayerOfFortitudeTestImpt>();
    }
};

class ShackleUndeadTest : public TestCaseScript
{
public:
    ShackleUndeadTest() : TestCaseScript("spells priest shackle_undead") { }

    class ShackleUndeadTestImpt : public TestCase
    {
    public:
        ShackleUndeadTestImpt() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_BLOODELF);
            Creature* creature0 = SpawnCreature(6, true);

            Position spawn(_location);
            spawn.MoveInFront(spawn, 20.0f);
            Creature* creature1 = SpawnCreatureWithPosition(spawn, 16525); // Undead
            Creature* creature2 = SpawnCreatureWithPosition(spawn, 16525); // Undead

            // Fail
            TEST_CAST(priest, creature0, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_FAILED_BAD_TARGETS);

            Wait(3000);

            // Mana cost
            uint32 const expectedShackleUndeadMana = 150;
            TEST_POWER_COST(priest, creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, POWER_MANA, expectedShackleUndeadMana);

            // Aura
            TEST_AURA_MAX_DURATION(creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, EFFECT_0, 50 * SECOND * IN_MILLISECONDS);

            creature2->Attack(priest, false);
            for (int count = 0; count < 10; count++)
            {
                if (!creature2->IsMovementPreventedByCasting())
                {
                    Wait(1000);
                    continue;
                }
                TEST_CAST(priest, creature2, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
                TEST_HAS_AURA(creature2, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, EFFECT_0);
                TEST_HAS_NOT_AURA(creature1, ClassSpells::Priest::SHACKLE_UNDEAD_RNK_3, EFFECT_0);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShackleUndeadTestImpt>();
    }
};

void AddSC_test_spells_priest()
{
    // Discipline: 10/10
    new DispelMagicTest();
    new FearWardTest();
    new InnerFireTest();
    new LevitateTest();
    new ManaBurnTest();
    new MassDispelTest();
    new PowerWordFortitudeTest();
    new PowerWordShieldTest();
    new PrayerOfFortitudeTest();
    new ShackleUndeadTest();
}
