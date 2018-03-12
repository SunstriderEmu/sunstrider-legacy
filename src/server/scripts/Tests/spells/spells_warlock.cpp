#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

#define SOUL_SHARD 6265

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
    new ImmolateTest();
    new IncinerateTest();
    new SearingPainTest();
    new ShadowBoltTest();
    new SoulFireTest();
}
