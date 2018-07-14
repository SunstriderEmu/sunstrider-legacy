#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class AmplifyMagicTest : public TestCaseScript
{
public:
    AmplifyMagicTest() : TestCaseScript("spells mage amplify_magic") { }

    /*Amplifies magic used against the targeted party member, increasing damage taken from
    spells by up to 120 and healing spells by up to 240. Lasts 10min.*/
    class AmplifyMagicTestImpt : public TestCase
    {
    public:
        AmplifyMagicTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_TROLL);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            EQUIP_NEW_ITEM(enemy, 34182); // Grand Magister's Staff of Torrents - 266 SP
            EQUIP_NEW_ITEM(priest, 34182); // Grand Magister's Staff of Torrents - 266 SP

            // Can only cast on group member
            TEST_CAST(mage, priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, SPELL_FAILED_BAD_TARGETS);
            GroupPlayer(mage, priest);
            TEST_CAST(mage, priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6);
            TEST_AURA_MAX_DURATION(priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, Minutes(10));

            // Spell damage taken and healing boosts
            uint32 const spellDamageTakenBoost = enemy->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) + 120;
            float const spellCoefficient = ClassSpellsCoeff::Warlock::SHADOW_BOLT;
            uint32 const minSB = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + spellDamageTakenBoost * spellCoefficient;
            uint32 const maxSB = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + spellDamageTakenBoost * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(enemy, priest, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, minSB, maxSB, false);

            uint32 const healingBoost = priest->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) + 240;
            float const greaterHealCoeff = ClassSpellsCoeff::Priest::GREATER_HEAL;
            uint32 const minGH = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MIN + healingBoost * greaterHealCoeff;
            uint32 const maxGH = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MAX + healingBoost * greaterHealCoeff;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, minGH, maxGH, false);

            // Test mana cost
            uint32 const expectedAmplifyMagicMana = 600;
            TEST_POWER_COST(priest, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, POWER_MANA, expectedAmplifyMagicMana);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<AmplifyMagicTestImpt>();
    }
};

class ArcaneBlastTest : public TestCaseScript
{
public:
    ArcaneBlastTest() : TestCaseScript("spells mage arcane_blast") { }

    /*Blasts the target with energy, dealing 648 to 753 Arcane damage. 
    Each time you cast Arcane Blast, the casting time is reduced while
        mana cost is increased. Effect stacks up to 3 times and lasts 8sec.*/
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

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ArcaneBlastTestImpt>();
    }
};

class ArcaneBrillianceTest : public TestCaseScript
{
public:
    ArcaneBrillianceTest() : TestCaseScript("spells mage arcane_brilliance") { }

    //Infuses the target's party with brilliance, increasing their Intellect by 40 for 1h.
    class ArcaneBrillianceTestImpt : public TestCase
    {
    public:
        ArcaneBrillianceTestImpt() : TestCase(STATUS_PASSING) { }

        void AssertArcaneBrillianceWorks(TestPlayer* caster, TestPlayer* victim, uint32 spellId, uint32 reagentId, uint32 manaCost, uint8 intBonus)
        {
            float const expectedCasterInt = caster->GetStat(STAT_INTELLECT) + intBonus;
            float const expectedVictimInt = victim->GetStat(STAT_INTELLECT) + intBonus;

            // Mana cost
            TEST_POWER_COST(caster, spellId, POWER_MANA, manaCost);

            caster->AddItem(reagentId, 1);
            TEST_ASSERT(caster->HasItemCount(reagentId, 1, false));
            TEST_CAST(caster, victim, spellId, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
            TEST_ASSERT(caster->GetItemCount(reagentId, false) == 0);

            // Aura duration
            TEST_AURA_MAX_DURATION(caster, spellId, Hours(1));
            TEST_AURA_MAX_DURATION(victim, spellId, Hours(1));

            // Stat
            TEST_ASSERT(caster->GetStat(STAT_INTELLECT) == expectedCasterInt);
            TEST_ASSERT(victim->GetStat(STAT_INTELLECT) == expectedVictimInt);

            // Reset for next test
            caster->RemoveAurasDueToSpell(spellId);
            victim->RemoveAurasDueToSpell(spellId);
        }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_TROLL);

            GroupPlayer(mage, priest);

            uint32 const ARCANE_POWDER = 17020;

            AssertArcaneBrillianceWorks(mage, priest, ClassSpells::Mage::ARCANE_BRILLIANCE_RNK_1, ARCANE_POWDER, 1500, 31);
            AssertArcaneBrillianceWorks(mage, priest, ClassSpells::Mage::ARCANE_BRILLIANCE_RNK_2, ARCANE_POWDER, 1800, 40);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ArcaneBrillianceTestImpt>();
    }
};

class ArcaneExplosionTest : public TestCaseScript
{
public:
    ArcaneExplosionTest() : TestCaseScript("spells mage arcane_explosion") { }

    //Causes an explosion of arcane magic around the caster, causing 377 to 408 Arcane damage to all targets within 10 yards.
    class ArcaneExplosionTestImpt : public TestCase
    {
    public:
        ArcaneExplosionTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            uint32 const expectedArcaneExplosionManaCost = 545;
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_EXPLOSION_RNK_8, POWER_MANA, expectedArcaneExplosionManaCost);

            // Damage
            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP
            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            float const castTime = 1.5f;
            float const spellCoeff = castTime / 3.5f / 2.f;

            uint32 const minArcaneExplosion = ClassSpellsDamage::Mage::ARCANE_EXPLOSION_RNK_8_MIN + spellPower * spellCoeff;
            uint32 const maxArcaneExplosion = ClassSpellsDamage::Mage::ARCANE_EXPLOSION_RNK_8_MAX + spellPower * spellCoeff;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ARCANE_EXPLOSION_RNK_8, minArcaneExplosion, maxArcaneExplosion, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ArcaneExplosionTestImpt>();
    }
};

class ArcaneIntellectTest : public TestCaseScript
{
public:
    ArcaneIntellectTest() : TestCaseScript("spells mage arcane_intellect") { }

    //"Increases the target's Intellect by 40 for 30min."
    class ArcaneIntellectTestImpt : public TestCase
    {
    public:
        ArcaneIntellectTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);

            uint32 const intBonus = 40;

            float const expectedInt = mage->GetStat(STAT_INTELLECT) + intBonus;

            uint32 const expectedArcaneIntellectManaCost = 700;
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_INTELLECT_RNK_6, POWER_MANA, expectedArcaneIntellectManaCost);

            TEST_CAST(mage, mage, ClassSpells::Mage::ARCANE_INTELLECT_RNK_6);
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Mage::ARCANE_INTELLECT_RNK_6, Minutes(30));

            TEST_ASSERT(mage->GetStat(STAT_INTELLECT) == expectedInt);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ArcaneIntellectTestImpt>();
    }
};

class ArcaneMissilesTest : public TestCaseScript
{
public:
    ArcaneMissilesTest() : TestCaseScript("spells mage arcane_missiles") { }

    //"Launches Arcane Missiles at the enemy, causing 240 Arcane damage every 1 sec for 5sec."
    class ArcaneMissilesTestImpt : public TestCase
    {
    public:
        ArcaneMissilesTestImpt() : TestCase(STATUS_PASSING) { } 

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN);
            Creature* dummy = SpawnCreature();

            uint32 const expectedArcaneMissilesManaCost = 740;
            TEST_POWER_COST(mage, ClassSpells::Mage::ARCANE_MISSILES_RNK_10, POWER_MANA, expectedArcaneMissilesManaCost);

            // Damage
            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP
            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            TEST_ASSERT(spellPower == 292);

            float const castTime = 5.f;
            float const spellCoeff = castTime / 3.5f;

            uint32 const spellLevel = 64;
            uint32 const spellMaxLevel = 68;
            float const perLevelPoint = 1.2f;
            uint32 const perLevelGain = (spellMaxLevel - spellLevel) * perLevelPoint;

            uint32 const tickAmount = 5;
            uint32 const spellPowerPerTick = floor(spellPower * spellCoeff) / tickAmount;
            uint32 const arcaneMissilesDmgPerTick = ClassSpellsDamage::Mage::ARCANE_MISSILES_RNK_10_TICK + perLevelGain + spellPowerPerTick;

            TEST_CHANNEL_DAMAGE(mage, dummy, ClassSpells::Mage::ARCANE_MISSILES_RNK_10, tickAmount, arcaneMissilesDmgPerTick, ClassSpells::Mage::ARCANE_MISSILES_RNK_10_PROC);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ArcaneMissilesTestImpt>();
    }
};

class BlinkTest : public TestCaseScript
{
public:
    BlinkTest() : TestCaseScript("spells mage blink") { }

    class BlinkTestImpt : public TestCase
    {
    public:
        BlinkTestImpt() : TestCase(STATUS_WIP) { }

        void Test() override
        {
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);

            Position expectedBlinkPosition;
            expectedBlinkPosition.MoveInFront(_location, 20.f);

            uint32 const expectedBlinkManaCost = 470;
            TEST_POWER_COST(mage, ClassSpells::Mage::BLINK_RNK_1, POWER_MANA, expectedBlinkManaCost);

            TEST_CAST(mage, mage, ClassSpells::Mage::BLINK_RNK_1);
            TEST_AURA_MAX_DURATION(mage, ClassSpells::Mage::BLINK_RNK_1, Seconds(1));
            TEST_HAS_COOLDOWN(mage, ClassSpells::Mage::BLINK_RNK_1, Seconds(15));
            WaitNextUpdate();
            ASSERT_INFO("Distance: %f", mage->GetDistance(expectedBlinkPosition));
            TEST_ASSERT(Between<float>(mage->GetDistance(expectedBlinkPosition), 0.f, 1.f));
            mage->TeleportTo(_location);

            // Spawn rogue behind mage
            expectedBlinkPosition.MoveInFront(_location, -1.f);
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, expectedBlinkPosition);

            auto AI = rogue->GetTestingPlayerbotAI();
            TEST_ASSERT(AI != nullptr);

            for (uint32 i = 0; i < 100; i++)
            {
                if (!mage->HasAura(ClassSpells::Mage::BLINK_RNK_1))
                    mage->AddAura(ClassSpells::Mage::BLINK_RNK_1, mage);
                TEST_HAS_AURA(mage, ClassSpells::Mage::BLINK_RNK_1);

                rogue->AddComboPoints(mage, 5);
                TEST_CAST(rogue, mage, ClassSpells::Rogue::KIDNEY_SHOT_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

                auto damageToTarget = AI->GetSpellDamageDoneInfo(mage);
                TEST_ASSERT(damageToTarget->size() == i + 1);
                auto& data = damageToTarget->back();

                if (data.damageInfo.SpellID != ClassSpells::Rogue::KIDNEY_SHOT_RNK_2)
                    continue;

                TEST_ASSERT(data.damageInfo.HitInfo != SPELL_MISS_NONE);
            }
        }
        // TODO: roots + get out of roots + stuns
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<BlinkTestImpt>();
    }
};

class IceLanceTest : public TestCaseScript
{
public:
    IceLanceTest() : TestCaseScript("spells mage ice_lance") { }

    //Deals 161 to 188 Frost damage to an enemy target. Causes triple damage against Frozen targets.
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

            // Damage
            float const castTime = 1.5f;
            float const spellCoeff = castTime / 3.5f / 3.0f;
            uint32 const iceLanceMin = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN + spellPower * spellCoeff;
            uint32 const iceLanceMax = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX + spellPower * spellCoeff;
            float const frozenFactor = 3.0f;
            uint32 const iceLanceMinFrozen = iceLanceMin * frozenFactor;
            uint32 const iceLanceMaxFrozen = iceLanceMax * frozenFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMin, iceLanceMax, false);
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMin * 1.5f, iceLanceMax * 1.5f, true);

            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMinFrozen, iceLanceMaxFrozen, false, [](Unit* caster, Unit* target) {
                target->AddAura(ClassSpells::Mage::FROST_NOVA_RNK_1, target);
            });

            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, iceLanceMinFrozen * 1.5f, iceLanceMaxFrozen * 1.5f, true, [](Unit* caster, Unit* target) {
                target->AddAura(ClassSpells::Mage::FROST_NOVA_RNK_1, target);
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<IceLanceTestImpt>();
    }
};

class FrostboltTest : public TestCaseScript
{
public:
    FrostboltTest() : TestCaseScript("spells mage frostbolt") { }

    //"Launches a bolt of frost at the enemy, causing 597 to 644 Frost damage and slowing movement speed by 40 % for 9sec."
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

            // Damage
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

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FrostboltTestImpt>();
    }
};

void AddSC_test_spells_mage()
{
    // Arcane
    new AmplifyMagicTest();
    new ArcaneBlastTest();
    new ArcaneBrillianceTest();
    new ArcaneExplosionTest();
    new ArcaneIntellectTest();
    new ArcaneMissilesTest();
    new BlinkTest();
    // Fire
    // Frost
    new IceLanceTest();
    new FrostboltTest();
}
