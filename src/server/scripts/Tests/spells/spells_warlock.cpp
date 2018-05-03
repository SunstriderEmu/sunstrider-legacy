#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "SpellHistory.h"

#define SOUL_SHARD 6265

class CorruptionTest : public TestCaseScript
{
public:
    CorruptionTest() : TestCaseScript("spells warlock corruption") { }

    class CorruptionTestImpt : public TestCase
    {
    public:
        CorruptionTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedCorruptionManaCost = 370;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, POWER_MANA, expectedCorruptionManaCost);

            // Damage -- something's wrong with the spell coeff
            float const spellCoefficient = ClassSpellsCoeff::Warlock::CORRUPTION;
            float const tickAmount = 6.0f;
            uint32 const expectedCorruptionTick = ClassSpellsDamage::Warlock::CORRUPTION_RNK_8_TICK + spellPower * spellCoefficient / tickAmount;
            uint32 const expectedCorruptionTotal = expectedCorruptionTick * tickAmount;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCorruptionTotal, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CorruptionTestImpt>();
    }
};

class CurseOfAgonyTest : public TestCaseScript
{
public:
    CurseOfAgonyTest() : TestCaseScript("spells warlock curse_of_agony") { }

    class CurseOfAgonyTestImpt : public TestCase
    {
    public:
        CurseOfAgonyTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedCurseOfAgonyManaCost = 265;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, POWER_MANA, expectedCurseOfAgonyManaCost);

            // Damage -- something's wrong with the spell coeff
            float const spellCoefficient = ClassSpellsCoeff::Warlock::CURSE_OF_AGONY;
            float const expectedCoAMaxDamage = ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL + spellPower * spellCoefficient;
            uint32 const expectedCoADamage = (4 * expectedCoAMaxDamage / 24.0f) + (4 * expectedCoAMaxDamage / 12.0f) + (4 * expectedCoAMaxDamage / 8.0f);
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfAgonyTestImpt>();
    }
};

class CurseOfDoomTest : public TestCaseScript
{
public:
    CurseOfDoomTest() : TestCaseScript("spells warlock curse_of_doom") { }

    class CurseOfDoomTestImpt : public TestCase
    {
    public:
        CurseOfDoomTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* player = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            // Cannot be cast on players
            TEST_CAST(warlock, player, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, SPELL_FAILED_TARGET_IS_PLAYER);

            uint32 const expectedCurseOfDoomManaCost = 380;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, POWER_MANA, expectedCurseOfDoomManaCost);

            TEST_CAST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2);
            TEST_AURA_MAX_DURATION(dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, Minutes(1));
            TEST_HAS_COOLDOWN(warlock, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, Minutes(1));

            // Damage -- something's wrong with the spell coeff
            float const spellCoefficient = ClassSpellsCoeff::Warlock::CURSE_OF_DOOM;
            float const expectedCoDDamage = ClassSpellsDamage::Warlock::CURSE_OF_DOOM_RNK_2 + spellPower * spellCoefficient;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, expectedCoDDamage, true);

            // TODO: test Doomguard summon
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfDoomTestImpt>();
    }
};

class CurseOfRecklessnessTest : public TestCaseScript
{
public:
    CurseOfRecklessnessTest() : TestCaseScript("spells warlock curse_of_recklessness") { }

    class CurseOfRecklessnessTestImpt : public TestCase
    {
    public:
        CurseOfRecklessnessTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);

            // Fear the rogue
            uint32 const expectedRogueAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK) + 135;
            uint32 expectedRogueArmor = int32(rogue->GetArmor() - 800) > 0 ? rogue->GetArmor() - 800 : 0;
            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::FEAR_RNK_3, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_HAS_AURA(rogue, ClassSpells::Warlock::FEAR_RNK_3);
            // Cast curse on the rogue
            uint32 const expectedCurseOfRecklessnessManaCost = 160;
            TEST_POWER_COST(warlock, rogue, ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5, POWER_MANA, expectedCurseOfRecklessnessManaCost);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5, Minutes(2));
            // Fear should be removed by the curse
            TEST_HAS_NOT_AURA(rogue, ClassSpells::Warlock::FEAR_RNK_3);
            // Check armor and AP is reduced as intended
            ASSERT_INFO("Armor: %u - Expected: %u", rogue->GetArmor(), expectedRogueArmor);
            TEST_ASSERT(rogue->GetArmor() == expectedRogueArmor);
            TEST_ASSERT(rogue->GetTotalAttackPowerValue(BASE_ATTACK) == expectedRogueAP);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfRecklessnessTestImpt>();
    }
};

class CurseOfElementsTest : public TestCaseScript
{
public:
    CurseOfElementsTest() : TestCaseScript("spells warlock curse_of_elements") { }

    class CurseOfElementsTestImpt : public TestCase
    {
    public:
        CurseOfElementsTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            // Apply CoE
            uint32 const expectedCurseOfElementsManaCost = 260;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, POWER_MANA, expectedCurseOfElementsManaCost);
            TEST_AURA_MAX_DURATION(dummy, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, Minutes(5));

            // Increase damage taken by Shadow, Fire, Arcane and Frost by 10%
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * 1.1f, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * 1.1f, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * 1.1f * 1.5f, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * 1.1f * 1.5f, true);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * 1.1f, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * 1.1f, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * 1.1f * 1.5f, ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * 1.1f * 1.5f, true);
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN * 1.1f, ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX * 1.1f, false);
            TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::STARFIRE_RNK_8, ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN * 1.1f * 1.5f, ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX * 1.1f * 1.5f, true);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN * 1.1f, ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX * 1.1f, false);
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN * 1.1f * 1.5f, ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX * 1.1f * 1.5f, true);

            // Reduce Shadow, Fire, Arcane and Frost resistances by 88
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            uint32 const curseOfElementsResistanceMalus = 88;
            rogue->AddAura(ClassSpells::Priest::SHADOW_PROTECTION_RNK_4, rogue);
            EQUIP_ITEM(rogue, 32420);
            uint32 const expectedShadowResistance = 70 + 40 - curseOfElementsResistanceMalus;
            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, Minutes(2)); // Bug here, is still 5min
            TEST_ASSERT(rogue->GetResistance(SPELL_SCHOOL_SHADOW) == expectedShadowResistance);
            TEST_ASSERT(rogue->GetResistance(SPELL_SCHOOL_ARCANE) == 0);
            TEST_ASSERT(rogue->GetResistance(SPELL_SCHOOL_FIRE) == 0);
            TEST_ASSERT(rogue->GetResistance(SPELL_SCHOOL_FROST) == 0);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfElementsTestImpt>();
    }
};

class CurseOfTonguesTest : public TestCaseScript
{
public:
    CurseOfTonguesTest() : TestCaseScript("spells warlock curse_of_tongues") { }

    class CurseOfTonguesTestImpt : public TestCase
    {
    public:
        CurseOfTonguesTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);
            Creature* dummy = SpawnCreature();

            // PvE
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, SPELL_MISS_NONE);
            TEST_AURA_MAX_DURATION(dummy, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, Seconds(30));
            // TODO: test increased casting time

            // PvP
            uint32 const expectedCurseOfTonguesManaCost = 110;
            TEST_POWER_COST(warlock, mage, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, POWER_MANA, expectedCurseOfTonguesManaCost);
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Warlock::CURSE_OF_TONGUES_RNK_2, Seconds(12)); // bug here, it's 10 but as of patch 2.1, it should be 12s
            // Test increased casting time: Frostbolt cast time is 3.0s, with the curse it should be 4.8s
            FORCE_CAST(mage, warlock, ClassSpells::Mage::FROSTBOLT_RNK_13, SPELL_MISS_NONE);
            Wait(4000);
            TEST_HAS_NOT_AURA(warlock, ClassSpells::Mage::FROSTBOLT_RNK_13);
            Wait(1000);
            TEST_HAS_AURA(warlock, ClassSpells::Mage::FROSTBOLT_RNK_13);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfTonguesTestImpt>();
    }
};

class CurseOfWeaknessTest : public TestCaseScript
{
public:
    CurseOfWeaknessTest() : TestCaseScript("spells warlock curse_of_weakness") { } 

    class CurseOfWeaknessTestImpt : public TestCase
    {
    public:
        CurseOfWeaknessTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);

            uint32 const curseOfWeaknessAPMalus = 350;
            uint32 const expectedRogueAP = rogue->GetTotalAttackPowerValue(BASE_ATTACK) - curseOfWeaknessAPMalus;

            uint32 const expectedCurseOfWeaknessManaCost = 265;
            TEST_POWER_COST(warlock, rogue, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8, POWER_MANA, expectedCurseOfWeaknessManaCost);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8, Minutes(2));
            TEST_ASSERT(rogue->GetTotalAttackPowerValue(BASE_ATTACK) == expectedRogueAP);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurseOfWeaknessTestImpt>();
    }
};

class DeathCoilTest : public TestCaseScript
{
public:
    DeathCoilTest() : TestCaseScript("spells warlock death_coil") { }

    class DeathCoilTestImpt : public TestCase
    {
    public:
        DeathCoilTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP
            warlock->SetHealth(1);
            warlock->DisableRegeneration(true);
            rogue->DisableRegeneration(true);

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            // Caster heals 100% of damage
            uint32 const rogueStartHealth = rogue->GetHealth();
            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::DEATH_COIL_RNK_4, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_SPEED));
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warlock::DEATH_COIL_RNK_4, Seconds(3));
            uint32 const expectedWarlockHealth = 1 + rogueStartHealth - rogue->GetHealth();
            TEST_ASSERT(warlock->GetHealth() == expectedWarlockHealth);

            // Damage
            float const castTime = 1.5f;
            float const spellCoefficient = castTime / 3.5f / 2.0f;

            uint32 const spellLevel = 68;
            float const dmgPerLevel = 3.4f;
            float const dmgPerLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;

            uint32 const expectedDeathCoilDmg = ClassSpellsDamage::Warlock::DEATH_COIL_RNK_4 + dmgPerLevelGain + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::DEATH_COIL_RNK_4, expectedDeathCoilDmg, expectedDeathCoilDmg, false);

            uint32 const expectedDeathCoilManaCost = 600;
            TEST_POWER_COST(warlock, rogue, ClassSpells::Warlock::DEATH_COIL_RNK_4, POWER_MANA, expectedDeathCoilManaCost);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DeathCoilTestImpt>();
    }
};

class DrainLifeTest : public TestCaseScript
{
public:
    DrainLifeTest() : TestCaseScript("spells warlock drain_life") { }

    class DrainLifeTestImpt : public TestCase
    {
    public:
        DrainLifeTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34337); // Golden Staff of the Sin'dorei - 183 SP
            warlock->DisableRegeneration(true);
            dummy->DisableRegeneration(true);

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 183);

            uint32 const expectedDrainLifeManaCost = 425;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, POWER_MANA, expectedDrainLifeManaCost);

            // Damage & Heal
            float const duration = 5.0f;
            float const spellCoeff = duration / 3.5f / 2.0f;
            uint32 const totalDrainLife = 5.0f * ClassSpellsDamage::Warlock::DRAIN_LIFE_RNK_8_TICK + spellPower * spellCoeff;
            uint32 const expectedTickAmount = totalDrainLife / 5.0f;

            warlock->SetHealth(1);
            uint32 warlockExpectedHealth = 1 + 5.0f * expectedTickAmount;
            uint32 dummyExpectedHealth = dummy->GetHealth() - 5 * expectedTickAmount;
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_GCD));
            Wait(5500);
            TEST_ASSERT(dummy->GetHealth() == dummyExpectedHealth);
            TEST_ASSERT(warlock->GetHealth() == warlockExpectedHealth);

            // Mortal Strike: reduces heal, not the damage
            warlock->SetHealth(1);
            warlock->AddAura(ClassSpells::Warrior::MORTAL_STRIKE_RNK_6, warlock);
            warlockExpectedHealth = 1 + 5.0f * expectedTickAmount / 2.0f;
            dummyExpectedHealth = dummy->GetHealth() - 5 * expectedTickAmount;
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            Wait(5500);
            TEST_ASSERT(dummy->GetHealth() == dummyExpectedHealth);
            TEST_ASSERT(warlock->GetHealth() == warlockExpectedHealth);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DrainLifeTestImpt>();
    }
};

class DrainManaTest : public TestCaseScript
{
public:
    DrainManaTest() : TestCaseScript("spells warlock drain_mana") { }

    class DrainManaTestImpt : public TestCase
    {
    public:
        DrainManaTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP
            warlock->DisableRegeneration(true);

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedDrainManaManaCost = 455;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::DRAIN_MANA_RNK_6, POWER_MANA, expectedDrainManaManaCost);

            // Doesn't drain mana if there isnt mana (!)
            warlock->SetPower(POWER_MANA, 0);
            FORCE_CAST(warlock, dummy, ClassSpells::Warlock::DRAIN_MANA_RNK_6, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_GCD));
            Wait(5500);
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == 0);

            // Bug here -- successfully drains the mage but doesn't return the mana to the warlock
            Position spawnPos;
            spawnPos.MoveInFront(_location, 10.0f);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL, 70, spawnPos);
            mage->DisableRegeneration(true);
            uint32 warlockExpectedMana = 5.0f * ClassSpellsDamage::Warlock::DRAIN_MANA_RNK_6_TICK;
            uint32 mageExpectedMana = mage->GetPower(POWER_MANA) - 5.0f * ClassSpellsDamage::Warlock::DRAIN_MANA_RNK_6_TICK;
            FORCE_CAST(warlock, mage, ClassSpells::Warlock::DRAIN_MANA_RNK_6, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            Wait(5500);
            TEST_ASSERT(mage->GetPower(POWER_MANA) == mageExpectedMana);
            ASSERT_INFO("Mana: %u - Expected: %u", warlock->GetPower(POWER_MANA), warlockExpectedMana);
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == warlockExpectedMana);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DrainManaTestImpt>();
    }
};

class DrainSoulTest : public TestCaseScript
{
public:
    DrainSoulTest() : TestCaseScript("spells warlock drain_soul") { }

    class DrainSoulTestImpt : public TestCase
    {
    public:
        DrainSoulTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            _location.MoveInFront(_location, 10.0f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            rogue->DisableRegeneration(true);

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);
            // Damage -- 
            // DrDamage coef: 2.143
            float const spellCoeff = ClassSpellsCoeff::Warlock::DRAIN_SOUL;
            uint32 const totalDrainSoul = 5.0f * ClassSpellsDamage::Warlock::DRAIN_SOUL_RNK_5_TICK + spellPower * spellCoeff;
            uint32 const expectedTickAmount = totalDrainSoul / 5.0f;
            TEST_CHANNEL_DAMAGE(warlock, rogue, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, 5, expectedTickAmount);

            Wait(2000);
            rogue->SetHealth(1);
            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            Wait(3000);
            Wait(1);
            // Gain soulshard on target's death
            TEST_ASSERT(rogue->IsDead());
            TEST_ASSERT(warlock->HasItemCount(SOUL_SHARD, 1));

            rogue->ResurrectPlayer(1.0f);
            uint32 const expectedDrainSoulManaCost = 360;
            TEST_POWER_COST(warlock, rogue, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, POWER_MANA, expectedDrainSoulManaCost);

            // TODO: drain on creture that doesnt yield experience or honor for a player
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DrainSoulTestImpt>();
    }
};

class FearTest : public TestCaseScript
{
public:
    FearTest() : TestCaseScript("spells warlock fear") { }

    class FearTestImpt : public TestCase
    {
    public:
        FearTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);

            uint32 const expectedFearManaCost = 313;
            TEST_POWER_COST(warlock, rogue, ClassSpells::Warlock::FEAR_RNK_3, POWER_MANA, expectedFearManaCost);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warlock::FEAR_RNK_3, Seconds(10));

            // Only 1 fear is active per warlock
            FORCE_CAST(warlock, mage, ClassSpells::Warlock::FEAR_RNK_3, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_GCD));
            TEST_HAS_NOT_AURA(rogue, ClassSpells::Warlock::FEAR_RNK_3);
            Wait(2000);
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Warlock::FEAR_RNK_3, Seconds(10));

            FORCE_CAST(warlock, rogue, ClassSpells::Warlock::FEAR_RNK_3, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_IGNORE_GCD));
            TEST_HAS_NOT_AURA(mage, ClassSpells::Warlock::FEAR_RNK_3);
            Wait(2000);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Warlock::FEAR_RNK_3, Seconds(5));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FearTestImpt>();
    }
};

class HowlOfTerrorTest : public TestCaseScript
{
public:
    HowlOfTerrorTest() : TestCaseScript("spells warlock howl_of_terror") { }

    class HowlOfTerrorTestImpt : public TestCase
    {
    public:
        HowlOfTerrorTestImpt() : TestCase(STATUS_PASSING) { }

        bool isFeared(Unit* victim)
        {
            return victim->HasAura(ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            TestPlayer* enemy1 = SpawnPlayer(CLASS_ROGUE, RACE_ORC);

            FORCE_CAST(warlock, warlock, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            TEST_AURA_MAX_DURATION(enemy1, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, Seconds(8));
            TEST_HAS_COOLDOWN(warlock, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, Seconds(40));
            enemy1->RemoveAurasDueToSpell(ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2);

            // Fear 5 at max
            TestPlayer* enemy2 = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            TestPlayer* enemy3 = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            TestPlayer* enemy4 = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            TestPlayer* enemy5 = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            TestPlayer* enemy6 = SpawnPlayer(CLASS_ROGUE, RACE_ORC);

            FORCE_CAST(warlock, warlock, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            uint32 count = 0;
            count += uint32(isFeared(enemy1));
            count += uint32(isFeared(enemy2));
            count += uint32(isFeared(enemy3));
            count += uint32(isFeared(enemy4));
            count += uint32(isFeared(enemy5));
            count += uint32(isFeared(enemy6));
            TEST_ASSERT(count == 5);

            uint32 const expectedHowlOfTerrorManaCost = 200;
            TEST_POWER_COST(warlock, warlock, ClassSpells::Warlock::HOWL_OF_TERROR_RNK_2, POWER_MANA, expectedHowlOfTerrorManaCost);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HowlOfTerrorTestImpt>();
    }
};

class LifeTapTest : public TestCaseScript
{
public:
    LifeTapTest() : TestCaseScript("spells warlock life_tap") { }

    class LifeTapTestImpt : public TestCase
    {
    public:
        LifeTapTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const spellLevel = 68;
            uint32 const perLevelPoint = 1;
            uint32 const perLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * perLevelPoint;
            float const spellCoeff = ClassSpellsCoeff::Warlock::LIFE_TAP; //DrDamage value
            uint32 const expectedManaGained = ClassSpellsDamage::Warlock::LIFE_TAP_RNK_7 + perLevelGain + spellPower * spellCoeff;

            warlock->DisableRegeneration(true);
            warlock->SetHealth(expectedManaGained + 1);
            warlock->SetPower(POWER_MANA, 0);

            TEST_CAST(warlock, warlock, ClassSpells::Warlock::LIFE_TAP_RNK_7);
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == expectedManaGained);
            TEST_ASSERT(warlock->GetHealth() == 1);
            TEST_ASSERT(warlock->GetPower(POWER_MANA) == expectedManaGained);
            // Can't suicide with Life Tap (http://www.wowhead.com/spell=1454/life-tap#comments:id=19418)
            TEST_CAST(warlock, warlock, ClassSpells::Warlock::LIFE_TAP_RNK_7, SPELL_FAILED_FIZZLE, TRIGGERED_IGNORE_GCD);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<LifeTapTestImpt>();
    }
};

class SeedOfCorruptionTest : public TestCaseScript
{
public:
    SeedOfCorruptionTest() : TestCaseScript("spells warlock seed_of_corruption") { }

    class SeedOfCorruptionTestImpt : public TestCase
    {
    public:
        SeedOfCorruptionTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void ResetDummiesHealth(Creature* dummy1, Creature* dummy2, Creature* dummy3) {
            dummy1->SetFullHealth();
            dummy2->SetFullHealth();
            dummy3->SetFullHealth();
            uint32 const maxHealth = dummy1->GetHealth();
            TEST_ASSERT(dummy2->GetHealth() == maxHealth && dummy3->GetHealth() == maxHealth);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy1 = SpawnCreature();
            Creature* dummy2 = SpawnCreature();
            Creature* dummy3 = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedSeedOfCorruptionManaCost = 882;
            TEST_POWER_COST(warlock, dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, POWER_MANA, expectedSeedOfCorruptionManaCost);
            Wait(500);
            TEST_AURA_MAX_DURATION(dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, Seconds(18));

            // Damage -- wrong spell coefficients
            float const tickAmount = 6.0f;
            // Coefficients were taken from the Warlock's Den, not WoW Wiki, as they were the most recent
            float const dotCoeff = ClassSpellsCoeff::Warlock::SEED_OF_CORRUPTION_DOT;
            float const directCoeff = ClassSpellsCoeff::Warlock::SEED_OF_CORRUPTION;
            
            uint32 const seedOfCorruptionTick = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_TICK + spellPower * dotCoeff / tickAmount;
            uint32 const expectedTotalAmount = tickAmount * seedOfCorruptionTick;
            TEST_DOT_DAMAGE(warlock, dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, expectedTotalAmount, true);

            float const expectedDetonationMin = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MIN + spellPower * directCoeff;
            float const expectedDetonationMax = ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MAX + spellPower * directCoeff;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, expectedDetonationMin, expectedDetonationMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1_DETONATION, expectedDetonationMin * 1.5f, expectedDetonationMax * 1.5f, true);

            // SoC detonates upon its victim taking 1044 damage
            ResetDummiesHealth(dummy1, dummy2, dummy3);
            uint32 const maxHealth = dummy1->GetHealth();
            FORCE_CAST(warlock, dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            Wait(18100);
            TEST_ASSERT(dummy1->GetHealth() == maxHealth - expectedTotalAmount);
            TEST_ASSERT(dummy2->GetHealth() < maxHealth);
            TEST_ASSERT(dummy3->GetHealth() < maxHealth);

            // SoC detonates upon its victim's death
            ResetDummiesHealth(dummy1, dummy2, dummy3);
            FORCE_CAST(warlock, dummy1, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(warlock, dummy2, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            dummy1->KillSelf(); // TODO: that doesnt detonate its SoC, should just damage to death
            TEST_ASSERT(dummy2->GetHealth() < maxHealth && dummy3->GetHealth() < maxHealth);
            // SoC detonation does not detonate other SoC
            TEST_HAS_AURA(dummy2, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SeedOfCorruptionTestImpt>();
    }
};

class BanishTest : public TestCaseScript
{
public:
    BanishTest() : TestCaseScript("spells warlock banish") { }

    class BanishTestImpt : public TestCase
    {
    public:
        BanishTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();
            Creature* demon = SpawnCreature(21, true); // Demon
            Creature* elemental = SpawnCreature(22, true); // Elemental

            // Should fail on classic dummy 
            TEST_CAST(warlock, dummy, ClassSpells::Warlock::BANISH_RNK_2, SPELL_FAILED_BAD_TARGETS);

            // Should succeed on Demon & Elemental
            FORCE_CAST(warlock, demon, ClassSpells::Warlock::BANISH_RNK_2, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            TEST_AURA_MAX_DURATION(demon, ClassSpells::Warlock::BANISH_RNK_2, Seconds(30));
            uint32 banishManaCost = 200;
            TEST_POWER_COST(warlock, elemental, ClassSpells::Warlock::BANISH_RNK_2, POWER_MANA, banishManaCost);

            // Only 1 active banish per warlock
            TEST_HAS_NOT_AURA(demon, ClassSpells::Warlock::BANISH_RNK_2);
            TEST_HAS_AURA(elemental, ClassSpells::Warlock::BANISH_RNK_2);

            // Banished is invulnerable
            {
                uint32 elemHealth = elemental->GetHealth();
                //to spell damage
                FORCE_CAST(warlock, elemental, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
                Wait(2000);
                TEST_ASSERT(elemental->GetHealth() == elemHealth);

                //to melee damage
                warlock->ForceMeleeHitResult(MELEE_HIT_NORMAL);
                warlock->AttackerStateUpdate(elemental, BASE_ATTACK);
                Wait(1);
                TEST_ASSERT(elemental->GetHealth() == elemHealth);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<BanishTestImpt>();
    }
};

class CreateFirestoneTest : public TestCaseScript
{
public:
    CreateFirestoneTest() : TestCaseScript("spells warlock create_firestone") { }

    class CreateFirestoneTestImpt : public TestCase
    {
    public:
        CreateFirestoneTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

        void CreateFirestone(TestPlayer* caster, uint32 firestoneSpellId, uint32 firestone, uint32 expectedManaCost)
        {
            caster->AddItem(SOUL_SHARD, 1);
            TEST_POWER_COST(caster, caster, firestoneSpellId, POWER_MANA, expectedManaCost);
            TEST_ASSERT(caster->GetItemCount(SOUL_SHARD, false) == 0);
            TEST_ASSERT(caster->GetItemCount(firestone, false) == 1);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            const uint32 LESSER_FIRESTONE   = 1254;
            const uint32 FIRESTONE          = 13699;
            const uint32 GREATER_FIRESTONE  = 13700;
            const uint32 MAJOR_FIRESTONE    = 13701;
            const uint32 MASTER_FIRESTONE   = 22128;

            // Assert for each that firestone is created + assert power costs
            CreateFirestone(warlock, ClassSpells::Warlock::CREATE_FIRESTONE_RNK_1, LESSER_FIRESTONE, 500);
            CreateFirestone(warlock, ClassSpells::Warlock::CREATE_FIRESTONE_RNK_2, FIRESTONE, 700);
            CreateFirestone(warlock, ClassSpells::Warlock::CREATE_FIRESTONE_RNK_3, GREATER_FIRESTONE, 900);
            CreateFirestone(warlock, ClassSpells::Warlock::CREATE_FIRESTONE_RNK_4, MAJOR_FIRESTONE, 1100);
            warlock->DestroyConjuredItems(true);

            // Spell damage
            {
                warlock->DestroyItemCount(MASTER_FIRESTONE, 1, true); //destroy before adding a new one in next macro
                EQUIP_ITEM(warlock, MASTER_FIRESTONE);
                Wait(1000);

                // Increase fire spell damage by 30
                const uint32 majorFirestoneFireBonus = 30;
                const uint32 expectedIncinerateMin = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN + majorFirestoneFireBonus;
                const uint32 expectedIncinerateMax = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX + majorFirestoneFireBonus;
                TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin, expectedIncinerateMax, false);
                TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin * 1.5f, expectedIncinerateMax *1.5f, true);

                // Should only increase fire damage
                TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX, false);
                TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * 1.5f, ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * 1.5f, true);
            }

            // TODO: melee proc chance of passive (ClassSpells::Warlock::CREATE_FIRESTONE_PASSIVE_RNK_5)
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CreateFirestoneTestImpt>();
    }
};

class CreateHealthstoneTest : public TestCaseScript
{
public:
    CreateHealthstoneTest() : TestCaseScript("spells warlock create_healthstone") { }

    class CreateHealthstoneTestImpt : public TestCase
    {
    public:
        CreateHealthstoneTestImpt() : TestCase(STATUS_PASSING) { }

        void CreateHealthstone(TestPlayer* caster, uint32 healthstoneSpellId, uint32 healthstone, uint32 expectedManaCost, uint32 healthRestored)
        {
            caster->SetHealth(1);
            caster->AddItem(SOUL_SHARD, 2);
            TEST_CAST(caster, caster, healthstoneSpellId);
            Wait(3500);
            TEST_ASSERT(caster->GetItemCount(SOUL_SHARD, false) == 1);
            TEST_ASSERT(caster->GetItemCount(healthstone, false) == 1);
            Wait(1);
            USE_ITEM(caster, caster, healthstone);
            Wait(Seconds(1));
            TEST_ASSERT(caster->GetItemCount(healthstone, false) == 0);
            TEST_ASSERT(caster->GetHealth() == 1 + healthRestored);
            caster->GetSpellHistory()->ResetAllCooldowns();

            TEST_POWER_COST(caster, caster, healthstoneSpellId, POWER_MANA, expectedManaCost);
            TEST_ASSERT(caster->GetItemCount(SOUL_SHARD, false) == 0);
            caster->SetFullPower(POWER_MANA);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            warlock->DisableRegeneration(true);
            EnableCriticals(warlock, false); // disable Healthstone crit

            const uint32 MINOR_HEALTHSTONE      = 5512;
            const uint32 LESSER_HEALTHSTONE     = 5511;
            const uint32 HEALTHSTONE            = 5509;
            const uint32 GREATER_HEALTHSTONE    = 5510;
            const uint32 MAJOR_HEALTHSTONE      = 9421;
            const uint32 MASTER_HEALTHSTONE     = 22103;

            // Assert for each that healthstone is created + assert power costs + health restored
            CreateHealthstone(warlock, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_1, MINOR_HEALTHSTONE, 95, 100);
            CreateHealthstone(warlock, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_2, LESSER_HEALTHSTONE, 240, 250);
            CreateHealthstone(warlock, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_3, HEALTHSTONE, 475, 500);
            CreateHealthstone(warlock, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_4, GREATER_HEALTHSTONE, 750, 800);
            CreateHealthstone(warlock, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_5, MAJOR_HEALTHSTONE, 1120, 1200);
            CreateHealthstone(warlock, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_6, MASTER_HEALTHSTONE, 1390, 2080);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CreateHealthstoneTestImpt>();
    }
};

class CreateSoulstoneTest : public TestCaseScript
{
public:
    CreateSoulstoneTest() : TestCaseScript("spells warlock create_soulstone") { }

    class CreateSoulstoneTestImpt : public TestCase
    {
    public:
        CreateSoulstoneTestImpt() : TestCase(STATUS_PASSING) { }

        void CreateSoulstone(TestPlayer* caster, uint32 soulstoneSpellId, uint32 soulstoneItemSpellId, uint32 soulstone, uint32 healthRestored, uint32 manaRestored)
        {
            caster->AddItem(SOUL_SHARD, 1);
            TEST_CAST(caster, caster, soulstoneSpellId, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            ASSERT_INFO("cb fdp: %u", caster->GetItemCount(SOUL_SHARD, false));
            TEST_ASSERT(caster->GetItemCount(SOUL_SHARD, false) == 0);
            TEST_ASSERT(caster->GetItemCount(soulstone, false) == 1);

            USE_ITEM(caster, caster, soulstone);
            Wait(Seconds(3));
            TEST_ASSERT(caster->GetItemCount(soulstone, false) == 0);
            // Spell has 30min cooldown. Impossible to check directly, so we test superior to 29 min.
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(soulstoneItemSpellId);
            TEST_ASSERT(spellInfo != nullptr);
            ASSERT_INFO("Spell: %u, cd: %u", soulstoneItemSpellId, caster->GetSpellHistory()->GetRemainingCooldown(spellInfo));
            TEST_ASSERT(caster->GetSpellHistory()->GetRemainingCooldown(spellInfo) > (29 * MINUTE * IN_MILLISECONDS));
            TEST_AURA_MAX_DURATION(caster, soulstoneItemSpellId, Minutes(30));

            caster->KillSelf();
            Wait(1000);
            WorldPacket fakeClientResponse(CMSG_SELF_RES);
            caster->GetSession()->HandleSelfResOpcode(fakeClientResponse);
            Wait(1);
            TEST_ASSERT(caster->GetHealth() == healthRestored);
            TEST_ASSERT(caster->GetPower(POWER_MANA) == manaRestored);
            caster->GetSpellHistory()->ResetAllCooldowns();
            caster->RemoveAurasDueToSpell(soulstoneItemSpellId);

            caster->AddItem(SOUL_SHARD, 1);
            TEST_POWER_COST(caster, caster, soulstoneSpellId, POWER_MANA, 1778);
            TEST_ASSERT(caster->GetItemCount(SOUL_SHARD, false) == 0);
            caster->SetFullPower(POWER_MANA);
            caster->GetSpellHistory()->ResetAllCooldowns();
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            warlock->DisableRegeneration(true);
            
            const uint32 MINOR_SOULSTONE    = 5232;
            const uint32 LESSER_SOULSTONE   = 16892;
            const uint32 SOULSTONE          = 16893;
            const uint32 GREATER_SOULSTONE  = 16895;
            const uint32 MAJOR_SOULSTONE    = 16896;
            const uint32 MASTER_SOULSTONE   = 22116;

            // Assert for each that soulstone is created + assert power costs + resurrected stats
            CreateSoulstone(warlock, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_1, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_1_ITEM, MINOR_SOULSTONE, 400, 700);
            CreateSoulstone(warlock, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_2, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_2_ITEM, LESSER_SOULSTONE, 750, 1200);
            CreateSoulstone(warlock, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_3, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_3_ITEM, SOULSTONE, 1100, 1700);
            CreateSoulstone(warlock, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_4, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_4_ITEM, GREATER_SOULSTONE, 1600, 2200);
            CreateSoulstone(warlock, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_5, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_5_ITEM, MAJOR_SOULSTONE, 2200, 2800);
            CreateSoulstone(warlock, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_6, ClassSpells::Warlock::CREATE_SOULSTONE_RNK_6_ITEM, MASTER_SOULSTONE, 2900, 3300);

            // TODO: 2.1: Soulstones can no longer be used on targets not in your party or raid
            // TODO: 2.1: Soulstone buff will now be removed if the target or caster leaves the party or raid
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CreateSoulstoneTestImpt>();
    }
};

class CreateSpellstoneTest : public TestCaseScript
{
public:
    CreateSpellstoneTest() : TestCaseScript("spells warlock create_spellstone") { }

    class CreateSpellstoneTestImpt : public TestCase
    {
    public:
        CreateSpellstoneTestImpt() : TestCase(STATUS_WIP) { }

        void CreateSpellstone(TestPlayer* caster, uint32 spellstoneSpellId, uint32 spellstone, uint32 expectedManaCost, uint32 criticalStrikeRatingBonus)
        {
            const uint32 expectedSpellCritScore = caster->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_SPELL) + criticalStrikeRatingBonus;
            caster->AddItem(SOUL_SHARD, 1);
            TEST_POWER_COST(caster, caster, spellstoneSpellId, POWER_MANA, expectedManaCost);
            TEST_ASSERT(caster->GetItemCount(SOUL_SHARD, false) == 0);
            TEST_ASSERT(caster->GetItemCount(spellstone, false) == 1);
            // Equip item
            caster->GetSpellHistory()->ResetAllCooldowns();
            caster->AddAura(5760, caster); // Poison
            caster->AddAura(35760, caster); // Disease
            caster->AddAura(ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, caster); // Curse
            caster->AddAura(ClassSpells::Warrior::HAMSTRING_RNK_4, caster); // Debuff
            caster->AddAura(ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, caster); // Magic harmless
            caster->AddAura(ClassSpells::Mage::FROSTBOLT_RNK_13, caster); // Magic harmful
            caster->GetSpellHistory()->ResetAllCooldowns();
            // Use item
            USE_ITEM(caster, caster, spellstone);
            Wait(1);
            ASSERT_INFO("Crit: %u, expected: %u", caster->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_SPELL), expectedSpellCritScore);
            TEST_ASSERT(caster->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_SPELL) == expectedSpellCritScore);
            TEST_HAS_AURA(caster, 5760);
            TEST_HAS_AURA(caster, 35760);
            TEST_HAS_AURA(caster, ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4);
            TEST_HAS_AURA(caster, ClassSpells::Warrior::HAMSTRING_RNK_4);
            TEST_HAS_AURA(caster, ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7);
            TEST_HAS_NOT_AURA(caster, ClassSpells::Mage::FROSTBOLT_RNK_13);
            caster->RemoveAurasDueToSpell(5760);
            caster->RemoveAurasDueToSpell(35760);
            caster->RemoveAurasDueToSpell(ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4);
            caster->RemoveAurasDueToSpell(ClassSpells::Warrior::HAMSTRING_RNK_4);
            caster->RemoveAurasDueToSpell(ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            const uint32 SPELLSTONE         = 5522;
            const uint32 GREATER_SPELLSTONE = 13602;
            const uint32 MAJOR_SPELLSTONE   = 13603;
            const uint32 MASTER_SPELLSTONE  = 22646;

            // Assert for each that spellstone is created + assert power costs
            CreateSpellstone(warlock, ClassSpells::Warlock::CREATE_SPELLSTONE_RNK_1, SPELLSTONE, 500, 8);
            CreateSpellstone(warlock, ClassSpells::Warlock::CREATE_SPELLSTONE_RNK_2, GREATER_SPELLSTONE, 750, 11);
            CreateSpellstone(warlock, ClassSpells::Warlock::CREATE_SPELLSTONE_RNK_3, MAJOR_SPELLSTONE, 1000, 14);
            CreateSpellstone(warlock, ClassSpells::Warlock::CREATE_SPELLSTONE_RNK_4, MASTER_SPELLSTONE, 1150, 20);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CreateSpellstoneTestImpt>();
    }
};

class DemonArmorTest : public TestCaseScript
{
public:
    DemonArmorTest() : TestCaseScript("spells warlock demon_armor") { }

    class DemonArmorTestImpt : public TestCase
    {
    public:
        DemonArmorTestImpt() : TestCase(STATUS_WIP) { }

        void TestDemonArmorBonuses(TestPlayer* caster, uint32 demonArmorSpellId, uint32 expectedManaCost, uint32 armorBonus, uint32 shadowResBonus, uint32 healthRestore)
        {
            caster->SetHealth(1);
            uint32 const expectedArmor = caster->GetArmor() + armorBonus;
            uint32 const expectedShadowRes = caster->GetResistance(SPELL_SCHOOL_SHADOW) + shadowResBonus;
            TEST_POWER_COST(caster, caster, demonArmorSpellId, POWER_MANA, expectedManaCost);
            TEST_ASSERT(caster->GetArmor() == expectedArmor);
            TEST_ASSERT(caster->GetResistance(SPELL_SCHOOL_SHADOW) == expectedShadowRes);
            uint32 const regenTick = healthRestore / 2.5f;
            Wait(2000);
            ASSERT_INFO("Health: %u, expected: %u", caster->GetHealth(), 1 + regenTick);
            TEST_ASSERT(caster->GetHealth() == 1 + regenTick);
            caster->RemoveAurasDueToSpell(demonArmorSpellId);
        }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            warlock->AttackerStateUpdate(dummy, BASE_ATTACK);

            TestDemonArmorBonuses(warlock, ClassSpells::Warlock::DEMON_ARMOR_RNK_1, 110, 210, 3, 7);
            TestDemonArmorBonuses(warlock, ClassSpells::Warlock::DEMON_ARMOR_RNK_2, 208, 300, 6, 9);
            TestDemonArmorBonuses(warlock, ClassSpells::Warlock::DEMON_ARMOR_RNK_3, 320, 390, 9, 11);
            TestDemonArmorBonuses(warlock, ClassSpells::Warlock::DEMON_ARMOR_RNK_4, 460, 480, 12, 13);
            TestDemonArmorBonuses(warlock, ClassSpells::Warlock::DEMON_ARMOR_RNK_5, 632, 570, 15, 15);
            TestDemonArmorBonuses(warlock, ClassSpells::Warlock::DEMON_ARMOR_RNK_6, 820, 660, 18, 18);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<DemonArmorTestImpt>();
    }
};

class RainOfFireTest : public TestCaseScript
{
public:
    RainOfFireTest() : TestCaseScript("spells warlock rain_of_fire") { }

    class RainOfFireTestImpt : public TestCase
    {
    public:
        RainOfFireTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedRainOfFireManaCost = 1480;
            TEST_POWER_COST(warlock, warlock, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, POWER_MANA, expectedRainOfFireManaCost);

            // Damage -- 
            // DrDamage has coef 1.146
            // WoWWiki has coef 0.952 (http://wowwiki.wikia.com/wiki/Spell_power_coefficient?oldid=1336186)
            // Sunstrider uses with DrDamage value
            float const duration = 8.0f;
            float const spellCoeff = duration / 3.5f / 2.0f;

            uint32 const spellLevel = 69;
            float const dmgPerLevel = 0.8f;
            float const dmgPerLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;

            uint32 const totalRainOfFire = 4.0f * (ClassSpellsDamage::Warlock::RAIN_OF_FIRE_RNK_5_TICK + dmgPerLevelGain) + spellPower * spellCoeff;
            uint32 const expectedTickAmount = totalRainOfFire / 4.0f;
            TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5_PROC, 4, expectedTickAmount);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RainOfFireTestImpt>();
    }
};

class HellfireTest : public TestCaseScript
{
public:
    HellfireTest() : TestCaseScript("spells warlock hellfire") { }

    class HellfireTestImpt : public TestCase
    {
    public:
        HellfireTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP
            warlock->SetMaxHealth(uint32(10000000));
            warlock->SetFullHealth();
            warlock->DisableRegeneration(true);

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            // Sometimes the spell doesnt seem to be cast, resulting in test failing here
            uint32 const expectedHellfireManaCost = 1665;
            TEST_POWER_COST(warlock, warlock, ClassSpells::Warlock::HELLFIRE_RNK_4, POWER_MANA, expectedHellfireManaCost);

            // Damage
            float const duration = 15.0f;
            float const spellCoeff = duration / 3.5f / 2.0f;
            
            uint32 const spellLevel = 68;
            float const dmgPerLevel = 0.8f;
            float const dmgPerLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;

            
            uint32 const totalHellfire = 15.0f * (ClassSpellsDamage::Warlock::HELLFIRE_RNK_4_TICK + dmgPerLevelGain) + spellPower * spellCoeff;
            uint32 const expectedTickAmount = totalHellfire / 15.0f;
            TEST_CHANNEL_DAMAGE(warlock, dummy, ClassSpells::Warlock::HELLFIRE_RNK_4, ClassSpells::Warlock::HELLFIRE_RNK_4_TRIGGER, 15, expectedTickAmount);

            // Self damage -- bug here
            uint32 const warlockStartHealth = warlock->GetHealth();
            FORCE_CAST(warlock, warlock, ClassSpells::Warlock::HELLFIRE_RNK_4, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            Wait(15500);
            uint32 const expectedWarlockHealth = warlockStartHealth - 15.0f * expectedTickAmount;
            ASSERT_INFO("Start Health: %u, Current: %u - Expected: %u - Expected Tick: %u, Actual Tick: %u", warlockStartHealth, warlock->GetHealth(), expectedWarlockHealth, expectedTickAmount, uint32((warlockStartHealth - warlock->GetHealth()) / 15.0f));
            TEST_ASSERT(warlock->GetHealth() == expectedWarlockHealth);

            // Should have no durability damage on suicide
            warlock->SetMaxHealth(uint32(100));
            FORCE_CAST(warlock, warlock, ClassSpells::Warlock::HELLFIRE_RNK_4, SPELL_MISS_NONE, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
            Wait(1500);
            TEST_ASSERT(warlock->IsDead());
            Item* sunflare = warlock->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            TEST_ASSERT(sunflare != nullptr);
            uint32 sunflareDurability = sunflare->GetUInt32Value(ITEM_FIELD_DURABILITY);
            uint32 sunflareMaxDurability = sunflare->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            TEST_ASSERT(sunflareDurability == sunflareMaxDurability);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HellfireTestImpt>();
    }
};

class ImmolateTest : public TestCaseScript
{
public:
    ImmolateTest() : TestCaseScript("spells warlock immolate") { }

    class ImmolateTestImpt : public TestCase
    {
    public:
        ImmolateTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedImmolateManaCost = 445;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, POWER_MANA, expectedImmolateManaCost);

            // Direct Damage -- https://wow.gamepedia.com/Talk:Spell_power#Hybrid_Formula
            float const castTime = 2.0f;
            float const duration = 15.0f;
            float const dotPortion = (duration / 15.0f) / ((duration / 15.0f) + (castTime / 3.5f));
            float const directPortion = 1 - dotPortion;
            float const dotSpellCoefficient = (duration / 15.0f) * dotPortion;
            float const directSpellCoefficient = (castTime / 3.5f) * directPortion;

            uint32 const spellLevel = 69; //db values
            float const dmgPerLevel = 4.3f; //db values
            float const dmgPerLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;

            uint32 const expectedImmolateDirect = ClassSpellsDamage::Warlock::IMMOLATE_RNK_9 + dmgPerLevelGain + spellPower * directSpellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedImmolateDirect, expectedImmolateDirect, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedImmolateDirect * 1.5f, expectedImmolateDirect * 1.5f, true);

            // DoT
            uint32 const expectedImmolateTick = (ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_DOT + spellPower * dotSpellCoefficient) / 5.0f;
            uint32 const expectedImmolateTotal = 5.0f * expectedImmolateTick;
            TEST_DOT_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedImmolateTotal, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ImmolateTestImpt>();
    }
};

class IncinerateTest : public TestCaseScript
{
public:
    IncinerateTest() : TestCaseScript("spells warlock incinerate") { }

    class IncinerateTestImpt : public TestCase
    {
    public:
        IncinerateTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedIncinerateManaCost = 355;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, POWER_MANA, expectedIncinerateManaCost);

            // Damage
            float const castTime = 2.5f;
            float const spellCoefficient = castTime / 3.5f;
            uint32 const expectedIncinerateMin = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN + spellPower * spellCoefficient;
            uint32 const expectedIncinerateMax = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin, expectedIncinerateMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin * 1.5f, expectedIncinerateMax * 1.5f, true);

            // Bonus with Immolate
            uint32 const expectedIncinerateBoostedMin = expectedIncinerateMin + ClassSpellsDamage::Warlock::INCINERATE_RNK_2_IMMOLATE_BONUS_MIN;
            uint32 const expectedIncinerateBoostedMax = expectedIncinerateMax + ClassSpellsDamage::Warlock::INCINERATE_RNK_2_IMMOLATE_BONUS_MAX;
            dummy->AddAura(ClassSpells::Warlock::IMMOLATE_RNK_9, dummy);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateBoostedMin, expectedIncinerateBoostedMax, false);
            dummy->AddAura(ClassSpells::Warlock::IMMOLATE_RNK_9, dummy);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateBoostedMin * 1.5f, expectedIncinerateBoostedMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<IncinerateTestImpt>();
    }
};

class SearingPainTest : public TestCaseScript
{
public:
    SearingPainTest() : TestCaseScript("spells warlock searing_pain") { }

    class SearingPainTestImpt : public TestCase
    {
    public:
        SearingPainTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedSearingPainManaCost = 205;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, POWER_MANA, expectedSearingPainManaCost);

            // Damage
            float const castTime = 1.5f;
            float const spellCoefficient = castTime / 3.5f;
            uint32 const expectedSRMin = ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MIN + spellPower * spellCoefficient;
            uint32 const expectedSRMax = ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MAX + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, expectedSRMin, expectedSRMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SEARING_PAIN_RNK_8, expectedSRMin * 1.5f, expectedSRMax * 1.5f, true);

            // TODO: High threat?
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SearingPainTestImpt>();
    }
};

class ShadowBoltTest : public TestCaseScript
{
public:
    ShadowBoltTest() : TestCaseScript("spells warlock shadow_bolt") { }

    class ShadowBoltTestImpt : public TestCase
    {
    public:
        ShadowBoltTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedShadowBoltManaCost = 420;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, POWER_MANA, expectedShadowBoltManaCost);

            // Damage
            float const castTime = 3.0f;
            float const spellCoefficient = castTime / 3.5f;
            uint32 const expectedSBMin = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + spellPower * spellCoefficient;
            uint32 const expectedSBMax = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSBMin, expectedSBMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedSBMin * 1.5f, expectedSBMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShadowBoltTestImpt>();
    }
};

class SoulFireTest : public TestCaseScript
{
public:
    SoulFireTest() : TestCaseScript("spells warlock soul_fire") { }

    class SoulFireTestImpt : public TestCase
    {
    public:
        SoulFireTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedSoulFireManaCost = 250;
            warlock->AddItem(SOUL_SHARD, 1);
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, POWER_MANA, expectedSoulFireManaCost);
            TEST_ASSERT(warlock->GetItemCount(SOUL_SHARD, false) == 0);

            warlock->AddItem(SOUL_SHARD, 1);
            TEST_CAST(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            // Consumes a Soul Shard
            TEST_ASSERT(warlock->GetItemCount(SOUL_SHARD, false) == 0);
            TEST_HAS_COOLDOWN(warlock, ClassSpells::Warlock::SOUL_FIRE_RNK_4, Minutes(1));

            // Damage
            float const spellCoefficient = 1.15f;
            uint32 const expectedSFMin = ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MIN + spellPower * spellCoefficient;
            uint32 const expectedSFMax = ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MAX + spellPower * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, expectedSFMin, expectedSFMax, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::SOUL_FIRE_RNK_4, expectedSFMin * 1.5f, expectedSFMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SoulFireTestImpt>();
    }
};

void AddSC_test_spells_warlock()
{
    // Affliction
    new CorruptionTest();
    new CurseOfAgonyTest();
    new CurseOfDoomTest();
    new CurseOfRecklessnessTest();
    new CurseOfElementsTest();
    new CurseOfTonguesTest();
    new CurseOfWeaknessTest();
    new DeathCoilTest();
    new DrainLifeTest();
    new DrainManaTest();
    new DrainSoulTest();
    new FearTest();
    new HowlOfTerrorTest();
    new LifeTapTest();
    new SeedOfCorruptionTest();
    // Demonology
    new BanishTest();
    new CreateFirestoneTest();
    new CreateHealthstoneTest();
    new CreateSoulstoneTest();
    new CreateSpellstoneTest();
    new DemonArmorTest();
    // Destruction: 7/7
    new HellfireTest();
    new ImmolateTest();
    new IncinerateTest();
    new RainOfFireTest();
    new SearingPainTest();
    new ShadowBoltTest();
    new SoulFireTest();
}
