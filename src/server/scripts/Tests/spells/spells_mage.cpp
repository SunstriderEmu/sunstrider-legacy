#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class IceLanceTest : public TestCaseScript
{
public:
    IceLanceTest() : TestCaseScript("spells mage ice_lance") { }

    class IceLanceTestImpt : public TestCase
    {
    public:
        IceLanceTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP

            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            uint32 const expectedIceLanceManaCost = 150;
            TEST_POWER_COST(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, POWER_MANA, expectedIceLanceManaCost);

            // Damage -- Failure here, something's wrong with spell coeff. 
            // Spell coeff, like damage, should triple on frozen target
            float const castTime = 1.5f;
            float const spellCoeff = castTime / 3.5f / 3.0f;
            uint32 const iceLanceMin = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN + spellPower * spellCoeff;
            uint32 const iceLanceMax = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX + spellPower * spellCoeff;
            float const frozenFactor = 3.0f;
            uint32 const iceLanceMinFrozen = iceLanceMin * frozenFactor;
            uint32 const iceLanceMaxFrozen = iceLanceMax * frozenFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMin, iceLanceMax, false);
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMin * 1.5f, iceLanceMax * 1.5f, true);

            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMinFrozen, iceLanceMaxFrozen, false, [](Unit* caster, Unit* target) {
                target->AddAura(ClassSpells::Mage::FROST_NOVA_RNK_1, target);
            });

            TEST_DIRECT_SPELL_DAMAGE_CALLBACK(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMinFrozen * 1.5f, iceLanceMaxFrozen * 1.5f, true, [](Unit* caster, Unit* target) {
                target->AddAura(ClassSpells::Mage::FROST_NOVA_RNK_1, target);
            });
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<IceLanceTestImpt>();
    }
};

class FrostboltTest : public TestCaseScript
{
public:
    FrostboltTest() : TestCaseScript("spells mage frostbolt") { }

    class FrostboltTestImpt : public TestCase
    {
    public:
        FrostboltTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_ORC);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP

            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            float const expectedSpeed = rogue->GetSpeed(MOVE_RUN) * 0.6f;
            uint32 const expectedFrostboltManaCost = 330;
            TEST_POWER_COST(mage, rogue, ClassSpells::Mage::FROSTBOLT_RNK_13, POWER_MANA, expectedFrostboltManaCost);
            Wait(1000);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Mage::FROSTBOLT_RNK_13, Seconds(9));
            ASSERT_INFO("Speed: %f - Expected: %f", rogue->GetSpeed(MOVE_RUN), expectedSpeed);
            TEST_ASSERT(Between(rogue->GetSpeed(MOVE_RUN), expectedSpeed - 0.1f, expectedSpeed + 0.1f));

            // Damage -- something's wrong with spell coeff, seems that the penalty for extra effect is not taken into account
            float const castTime = 3.0f;
            float const penaltyForExtraEffect = 0.95f;
            float const spellCoeff = castTime / 3.5f * penaltyForExtraEffect;

            uint32 const spellLevel = 69;
            float const dmgPerLevel = 3.5f;
            float const dmgPerLevelGain = std::max(mage->GetLevel() - spellLevel, uint32(0)) * dmgPerLevel;

            uint32 const frostboltMin = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_MIN + dmgPerLevelGain + spellPower * spellCoeff;
            uint32 const frostboltMax = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_MAX + dmgPerLevelGain + spellPower * spellCoeff;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMin, frostboltMax, false);
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMin * 1.5f, frostboltMax * 1.5f, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FrostboltTestImpt>();
    }
};

void AddSC_test_spells_mage()
{
    new IceLanceTest();
    new FrostboltTest();
}
