#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class AmplifyMagicTest : public TestCaseScript
{
public:
    AmplifyMagicTest() : TestCaseScript("spells mage amplify_magic") { }

    class AmplifyMagicTestImpt : public TestCase
    {
    public:
        /*
        Bugs:
            - Boosting is a little less than should be for both
        */
        AmplifyMagicTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_TROLL);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            // Can only cast on group member
            TEST_CAST(mage, priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, SPELL_FAILED_BAD_TARGETS);
            GroupPlayer(mage, priest);
            TEST_CAST(mage, priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, Minutes(10));

            // Spell damage taken and healing boosts
            float const spellDamageTakenBoost = 120;
            float const healingBoost = 240;

            uint32 const minSB = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + spellDamageTakenBoost;
            uint32 const maxSB = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + spellDamageTakenBoost;
            TEST_DIRECT_SPELL_DAMAGE(enemy, priest, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, minSB, maxSB, false);

            uint32 const minGH = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MIN + healingBoost;
            uint32 const maxGH = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MAX + healingBoost;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, minGH, maxGH, false);

            // Test mana cost
            uint32 const expectedAmplifyMagicMana = 600;
            TEST_POWER_COST(priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, POWER_MANA, expectedAmplifyMagicMana);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AmplifyMagicTestImpt>();
    }
};

class ArcaneBlastTest : public TestCaseScript
{
public:
    ArcaneBlastTest() : TestCaseScript("spells mage arcane_blast") { }

    class ArcaneBlastTestImpt : public TestCase
    {
    public:
        ArcaneBlastTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            uint32 const expectedArcaneBlastManaCost = 195;
            uint32 const expectedArcaneBlastManaCostStack1 = 341;
            uint32 const expectedArcaneBlastManaCostStack2 = 487;
            uint32 const expectedArcaneBlastManaCostStack3 = 633;

            float const castTimeReductionPerStack = 333.3f;
            uint32 const expectedArcaneBlastCastTime = 2500;
            uint32 const expectedArcaneBlastCastTimeStack1 = expectedArcaneBlastCastTime - castTimeReductionPerStack;
            uint32 const expectedArcaneBlastCastTimeStack2 = expectedArcaneBlastCastTimeStack1 - castTimeReductionPerStack;
            uint32 const expectedArcaneBlastCastTimeStack3 = expectedArcaneBlastCastTimeStack2 - castTimeReductionPerStack;

            // Stacks, spell cast time, mana cost
            TEST_CAST_TIME(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, expectedArcaneBlastCastTime);
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, POWER_MANA, expectedArcaneBlastManaCost);
            FORCE_CAST(mage, dummy, ClassSpells::Mage::ARCANE_BLAST_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            // Stack 1
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1_SELF_DEBUFF, Seconds(8));
            TEST_CAST_TIME(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, expectedArcaneBlastCastTimeStack1);
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, POWER_MANA, expectedArcaneBlastManaCostStack1);
            FORCE_CAST(mage, dummy, ClassSpells::Mage::ARCANE_BLAST_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            // Stack 2
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1_SELF_DEBUFF, Seconds(8));
            TEST_CAST_TIME(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, expectedArcaneBlastCastTimeStack2);
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, POWER_MANA, expectedArcaneBlastManaCostStack2);
            FORCE_CAST(mage, dummy, ClassSpells::Mage::ARCANE_BLAST_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            // Stack 3
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1_SELF_DEBUFF, Seconds(8));
            TEST_CAST_TIME(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, expectedArcaneBlastCastTimeStack3);
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_BLAST_RNK_1, POWER_MANA, expectedArcaneBlastManaCostStack3);
            // Max stack = 3
            FORCE_CAST(mage, dummy, ClassSpells::Mage::ARCANE_BLAST_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            Aura* arcaneBlast = mage->GetAura(ClassSpells::Mage::ARCANE_BLAST_RNK_1_SELF_DEBUFF);
            TEST_ASSERT(arcaneBlast != nullptr);
            TEST_ASSERT(arcaneBlast->GetStackAmount() == 3);

            // Damage
            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP
            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            float const castTime = 2.5f;
            float const spellCoeff = castTime / 3.5f;

            uint32 const spellLevel = 64;
            uint32 const spellMaxLevel = 68;
            uint32 const perLevelPoint = 5;
            uint32 const perLevelGain = (spellMaxLevel - spellLevel) * perLevelPoint;

            uint32 const minArcaneBlast = ClassSpellsDamage::Mage::ARCANE_BLAST_RNK_1_MIN + perLevelGain + spellPower * spellCoeff;
            uint32 const maxArcaneBlast = ClassSpellsDamage::Mage::ARCANE_BLAST_RNK_1_MAX + perLevelGain + spellPower * spellCoeff;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ARCANE_BLAST_RNK_1, minArcaneBlast, maxArcaneBlast, false);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ArcaneBlastTestImpt>();
    }
};

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
            TEST_POWER_COST(mage, ClassSpells::Mage::ICE_LANCE_RNK_1, POWER_MANA, expectedIceLanceManaCost);

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
            TEST_POWER_COST(mage, ClassSpells::Mage::FROSTBOLT_RNK_13, POWER_MANA, expectedFrostboltManaCost);

            FORCE_CAST(mage, rogue, ClassSpells::Mage::FROSTBOLT_RNK_13, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_SPEED));
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
    // Arcane
    new AmplifyMagicTest();
    new ArcaneBlastTest();
    // Fire
    // Frost
    new IceLanceTest();
    new FrostboltTest();
}
