#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

#define SOUL_SHARD 6265

class CorruptionTest : public TestCaseScript
{
public:
    CorruptionTest() : TestCaseScript("spells warlock corruption") { }

    class CorruptionTestImpt : public TestCase
    {
    public:
        CorruptionTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

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
        CurseOfAgonyTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

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

class RainOfFireTest : public TestCaseScript
{
public:
    RainOfFireTest() : TestCaseScript("spells warlock rain_of_fire") { }

    class RainOfFireTestImpt : public TestCase
    {
    public:
        RainOfFireTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedRainOfFireManaCost = 1480;
            TEST_POWER_COST(warlock, warlock, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, POWER_MANA, expectedRainOfFireManaCost);

            // Damage -- seems to have a wrong coeff, DrDamage agrees with the following
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
        HellfireTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

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

            // No durability damage on suicide
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
        ImmolateTestImpt() : TestCase(STATUS_KNOWN_BUG, true) { }

        void Test() override
        {
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_ITEM(warlock, 34336); // Sunflare - 292 SP

            uint32 const spellPower = warlock->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedImmolateManaCost = 445;
            TEST_POWER_COST(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, POWER_MANA, expectedImmolateManaCost);

            // Damage -- bug with direct damage
            float const castTime = 2.0f;
            float const duration = 15.0f;
            float const dotPortion = (duration / 15.0f) / ((duration / 15.0f) + (castTime / 3.5f));
            float const directPortion = 1 - dotPortion;
            float const dotSpellCoefficient = (duration / 15.0f) * dotPortion;
            float const directSpellCoefficient = (castTime / 3.5f) * directPortion;

            uint32 const spellLevel = 69;
            float const dmgPerLevel = 4.3f;
            float const dmgPerLevelGain = std::max(warlock->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;

            uint32 const expectedImmolateDirect = ClassSpellsDamage::Warlock::IMMOLATE_RNK_9 + dmgPerLevelGain + spellPower * directSpellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedImmolateDirect, expectedImmolateDirect, false);
            TEST_DIRECT_SPELL_DAMAGE(warlock, dummy, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedImmolateDirect * 1.5f, expectedImmolateDirect * 1.5f, true);

            // Bonus with Immolate
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
        IncinerateTestImpt() : TestCase(STATUS_PASSING, true) { }

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
        SearingPainTestImpt() : TestCase(STATUS_PASSING, true) { }

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
        ShadowBoltTestImpt() : TestCase(STATUS_PASSING, true) { }

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
        SoulFireTestImpt() : TestCase(STATUS_PARTIAL, true) { }

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
    // Destruction: 7/7
    new HellfireTest();
    new ImmolateTest();
    new IncinerateTest();
    new RainOfFireTest();
    new SearingPainTest();
    new ShadowBoltTest();
    new SoulFireTest();
}
