#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

class MageWandSpecializationTest : public TestCaseScript
{
public:
    MageWandSpecializationTest() : TestCaseScript("talents mage wand_specialization") { }

	class WandSpecializationTestImpt : public TestCase
	{
	public:
		WandSpecializationTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

			LearnTalent(mage, Talents::Mage::WAND_SPECIALIZATION_RNK_2);
            float const talentFactor = 1.25f;

			EQUIP_NEW_ITEM(mage, 28783); //Eredar Wand of Obliteration, 177 - 330 Shadow Damage
			uint32 const expectedWandMinDamage = 177 * talentFactor;
			uint32 const expectedWandMaxDamage = 330 * talentFactor;

			//Test improved damage 25%
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::WAND, expectedWandMinDamage, expectedWandMaxDamage, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<WandSpecializationTestImpt>();
	}
};

class MagicAbsorptionTest : public TestCaseScript
{
public:
	MagicAbsorptionTest() : TestCaseScript("talents mage magic_absorption") { }

	class MagicAbsorptionTestImpt : public TestCase
	{
	public:
        /*
        Bugs:
            - Fully resisted spells to restore 5% of total mana
        */
		MagicAbsorptionTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void AssertManaRestored(TestPlayer* mage, Unit* dummy, uint32 spellId, uint32 expectedManaRestored)
        {
            TriggerCastFlags flags = TriggerCastFlags(TRIGGERED_IGNORE_POWER_AND_REAGENT_COST | TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_SPEED | TRIGGERED_TREAT_AS_NON_TRIGGERED);
            FORCE_CAST(mage, dummy, spellId, SPELL_MISS_RESIST, flags);
            ASSERT_INFO("%s didn't return 0.05 of total mana.", _SpellString(spellId).c_str());
            TEST_ASSERT(mage->GetPower(POWER_MANA) == expectedManaRestored);
        }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
            mage->DisableRegeneration(true);
            mage->SetPower(POWER_MANA, 0);

            uint32 const talentResistBoost = 10;
            float const talentResistManaBack = 0.05f;

            // Resistance
			uint32 const expectedResArcane = mage->GetResistance(SPELL_SCHOOL_ARCANE) + talentResistBoost;
			uint32 const expectedResFire = mage->GetResistance(SPELL_SCHOOL_FIRE) + talentResistBoost;
			uint32 const expectedResFrost = mage->GetResistance(SPELL_SCHOOL_FROST) + talentResistBoost;
			uint32 const expectedResNature = mage->GetResistance(SPELL_SCHOOL_NATURE) + talentResistBoost;
			uint32 const expectedResShadow = mage->GetResistance(SPELL_SCHOOL_SHADOW) + talentResistBoost;

			LearnTalent(mage, Talents::Mage::MAGIC_ABSORPTION_RNK_5);
			TEST_ASSERT(mage->GetResistance(SPELL_SCHOOL_ARCANE) == expectedResArcane);
			TEST_ASSERT(mage->GetResistance(SPELL_SCHOOL_FIRE) == expectedResFire);
			TEST_ASSERT(mage->GetResistance(SPELL_SCHOOL_FROST) == expectedResFrost);
			TEST_ASSERT(mage->GetResistance(SPELL_SCHOOL_NATURE) == expectedResNature);
			TEST_ASSERT(mage->GetResistance(SPELL_SCHOOL_SHADOW) == expectedResShadow);

			// Fully resisted spells to restore 5% of total mana
            Creature* dummy = SpawnCreature();
            uint32 const expectedManaRestored = mage->GetMaxPower(POWER_MANA) * talentResistManaBack;
            AssertManaRestored(mage, dummy, ClassSpells::Mage::ARCANE_BLAST_RNK_1, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::COUNTERSPELL_RNK_1, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::POLYMORPH_RNK_4, expectedManaRestored);
            mage->AddAura(ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4, mage);
            AssertManaRestored(mage, mage, ClassSpells::Mage::REMOVE_LESSER_CURSE_RNK_1, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::SLOW__RNK_1, expectedManaRestored);
            dummy->AddAura(ClassSpells::Priest::POWER_WORD_FORTITUDE_RNK_7, dummy);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::SPELLSTEAL_RNK_1, expectedManaRestored);

            AssertManaRestored(mage, dummy, ClassSpells::Mage::BLAST_WAVE_RNK_7, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::DRAGONS_BREATH_RNK_4, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::FIRE_BLAST_RNK_9, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::FIREBALL_RNK_13, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::PYROBLAST_RNK_10, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::SCORCH_RNK_9, expectedManaRestored);

            AssertManaRestored(mage, dummy, ClassSpells::Mage::CONE_OF_COLD_RNK_6, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::FROST_NOVA_RNK_5, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, expectedManaRestored);
            AssertManaRestored(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, expectedManaRestored);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MagicAbsorptionTestImpt>();
	}
};

class MagicAttunementTest : public TestCaseScript
{
public:
	MagicAttunementTest() : TestCaseScript("talents mage magic_attunement") { }

	class MagicAttunementTestImpt : public TestCase
	{
	public:
        /*
        Bugs:
            - Should increase the effect of Amplify Magic & Dampen Magic by 50%
        */
		MagicAttunementTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

		void Test() override
		{
            TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);
            TestPlayer* priest = SpawnPlayer(CLASS_PRIEST, RACE_TROLL);
            TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            GroupPlayer(mage, priest);
            
            LearnTalent(mage, Talents::Mage::MAGIC_ATTUNEMENT_RNK_2);
            float const talentFactor = 1.5f;

            EQUIP_NEW_ITEM(enemy, 34182); // Grand Magister's Staff of Torrents - 266 SP
            EQUIP_NEW_ITEM(priest, 34335); // Hammer of Sanctification - 550 BH

            // Amplify Magic
            priest->AddAura(ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6, priest);

            uint32 const spellDamageTakenBoost = enemy->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) + 120 * talentFactor;
            float const spellCoefficient = ClassSpellsCoeff::Warlock::SHADOW_BOLT;
            uint32 const minSBBoost = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + spellDamageTakenBoost * spellCoefficient;
            uint32 const maxSBBoost = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + spellDamageTakenBoost * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(enemy, priest, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, minSBBoost, maxSBBoost, false);

            uint32 const healingBoost = priest->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) + 240 * talentFactor;
            float const greaterHealCoeff = ClassSpellsCoeff::Priest::GREATER_HEAL;
            uint32 const minGHBoost = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MIN + healingBoost * greaterHealCoeff;
            uint32 const maxGHBoost = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MAX + healingBoost * greaterHealCoeff;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, minGHBoost, maxGHBoost, false);
            priest->RemoveAurasDueToSpell(ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6);

            // Dampen Magic
            priest->AddAura(ClassSpells::Mage::DAMPEN_MAGIC_RNK_6, priest);

            uint32 const spellDamageTakenMalus = enemy->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) - 120 * talentFactor;
            uint32 const minSBMalus = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN + spellDamageTakenMalus * spellCoefficient;
            uint32 const maxSBMalus = ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX + spellDamageTakenMalus * spellCoefficient;
            TEST_DIRECT_SPELL_DAMAGE(enemy, priest, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, minSBMalus, maxSBMalus, false);

            uint32 const healingMalus = priest->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) - 240 * talentFactor;
            uint32 const minGHMalus = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MIN + healingMalus * greaterHealCoeff;
            uint32 const maxGHMalus = ClassSpellsDamage::Priest::GREATER_HEAL_RNK_7_MAX + healingMalus * greaterHealCoeff;
            TEST_DIRECT_HEAL(priest, priest, ClassSpells::Priest::GREATER_HEAL_RNK_7, minGHMalus, maxGHMalus, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MagicAttunementTestImpt>();
	}
};

class ArcaneFortitudeTest : public TestCaseScript
{
public:
	ArcaneFortitudeTest() : TestCaseScript("talents mage arcane_fortitude") { }

	class ArcaneFortitudeTestImpt : public TestCase
	{
	public:
		ArcaneFortitudeTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);

			uint32 const expectedArmor = mage->GetArmor() + mage->GetStat(STAT_INTELLECT);

			LearnTalent(mage, Talents::Mage::ARCANE_FORTITUDE_RNK_1);
			TEST_ASSERT(mage->GetArmor() == expectedArmor);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ArcaneFortitudeTestImpt>();
	}
};

class ImprovedCounterspellTest : public TestCaseScript
{
public:
	ImprovedCounterspellTest() : TestCaseScript("talents mage improved_counterspell") { }

	class ImprovedCounterspellTestImpt : public TestCase
	{
	public:
		ImprovedCounterspellTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

			LearnTalent(mage, Talents::Mage::IMPROVED_COUNTERSPELL_RNK_2);
            FORCE_CAST(mage, dummy, ClassSpells::Mage::COUNTERSPELL_RNK_1);
            WaitNextUpdate();
			TEST_AURA_MAX_DURATION(dummy, 18469, Seconds(4)); //"Counterspell - Silenced", 4s
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ImprovedCounterspellTestImpt>();
	}
};

class ArcaneMindTest : public TestCaseScript
{
public:
	ArcaneMindTest() : TestCaseScript("talents mage arcane_mind") { }

	class ArcaneMindTestImpt : public TestCase
	{
	public:
		ArcaneMindTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);

            float const talentFactor = 1.15f;
			float const expectedInt = mage->GetStat(STAT_INTELLECT) * talentFactor;

			LearnTalent(mage, Talents::Mage::ARCANE_MIND_RNK_5);
            ASSERT_INFO("Mage has %f Int but %f was expected.", mage->GetStat(STAT_INTELLECT), expectedInt);
			TEST_ASSERT(Between<float>(mage->GetStat(STAT_INTELLECT), expectedInt - 1.f, expectedInt + 1.f));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ArcaneMindTestImpt>();
	}
};

class ArcaneInstabilityTest : public TestCaseScript
{
public:
	ArcaneInstabilityTest() : TestCaseScript("talents mage arcane_instability") { }

	class ArcaneInstabilityTestImpt : public TestCase
	{
	public:
		ArcaneInstabilityTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			EQUIP_NEW_ITEM(mage, 34182); // Grand Magister's Staff of Torrents - 266 spell power

            float const talentSpelLPowerFactor = 1.03f;
            float const talentSpellCritFactor = 3.0f;

			int32 const expectedSP = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
			float const expectedSC = mage->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1);

			LearnTalent(mage, Talents::Mage::ARCANE_INSTABILITY_RNK_3);
			TEST_ASSERT(Between<int32>(mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE), expectedSP - 1, expectedSP + 1));
			TEST_ASSERT(Between<float>(mage->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1), expectedSC - 0.1f, expectedSC + 0.1f));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ArcaneInstabilityTestImpt>();
	}
};

class EmpoweredArcaneMissilesTest : public TestCaseScript
{
public:
	EmpoweredArcaneMissilesTest() : TestCaseScript("talents mage empowered_arcane_missiles") { }

	class EmpoweredArcaneMissilesTestImpt : public TestCase
	{
	public:
		EmpoweredArcaneMissilesTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

            float const talentSpellCoeffFactor = 0.45f;
		
            EQUIP_NEW_ITEM(mage, 34182); // Grand Magister's Staff of Torrents - 266 spell power
            uint32 const arcaneSpellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE);
			float const expectedCoeff = ClassSpellsCoeff::Mage::ARCANE_MISSILES + talentSpellCoeffFactor;

            uint32 const tickAmount = 5;
            uint32 const spellPowerPerTick = floor(arcaneSpellPower * expectedCoeff) / tickAmount;
			uint32 const expectedTickDmg = ClassSpellsDamage::Mage::ARCANE_MISSILES_RNK_10_LVL_70_TICK + spellPowerPerTick;

			LearnTalent(mage, Talents::Mage::EMPOWERED_ARCANE_MISSILES_RNK_3);
            TEST_CHANNEL_DAMAGE(mage, dummy, ClassSpells::Mage::ARCANE_MISSILES_RNK_10, tickAmount, expectedTickDmg, ClassSpells::Mage::ARCANE_MISSILES_RNK_10_PROC);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<EmpoweredArcaneMissilesTestImpt>();
	}
};

class MindMasteryTest : public TestCaseScript
{
public:
	MindMasteryTest() : TestCaseScript("talents mage mind_mastery") { }

	class MindMasteryTestImpt : public TestCase
	{
	public:
		MindMasteryTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);

			int32 const expectedSP = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE) + mage->GetStat(STAT_INTELLECT) * 0.25f;

			LearnTalent(mage, Talents::Mage::MIND_MASTERY_RNK_5);
			TEST_ASSERT(mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE) == expectedSP);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MindMasteryTestImpt>();
	}
};

class CriticalMassTest : public TestCaseScript
{
public:
	CriticalMassTest() : TestCaseScript("talents mage critical_mass") { }

	class CriticalMassTestImpt : public TestCase
	{
	public:
		CriticalMassTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);

            float const talentFireCritFactor = 6.0f;
			float const expectedSC = mage->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE) + talentFireCritFactor;

			LearnTalent(mage, Talents::Mage::CRITICAL_MASS_RNK_3);
			TEST_ASSERT(Between<float>(mage->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE), expectedSC - 0.1f, expectedSC + 0.1f));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<CriticalMassTestImpt>();
	}
};

class FirePowerTest : public TestCaseScript
{
public:

	FirePowerTest() : TestCaseScript("talents mage fire_power") { }

	class FirePowerTestImpt : public TestCase
	{
	public:
		FirePowerTestImpt() : TestCase(STATUS_WIP) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

            LearnTalent(mage, Talents::Mage::FIRE_POWER_RNK_5);
            float const talentFactor = 1.1f;

			// Fireblast
			uint32 const fireblastMinDamage = ClassSpellsDamage::Mage::FIREBALL_RNK_13_MIN_LVL_70 * talentFactor;
			uint32 const fireblastMaxDamage = ClassSpellsDamage::Mage::FIREBALL_RNK_13_MAX_LVL_70 * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FIRE_BLAST_RNK_9, fireblastMinDamage, fireblastMaxDamage, false);

			// Fireball
            uint32 const fireballTickAmount = 4;
            uint32 const fireballTick = ClassSpellsDamage::Mage::FIREBALL_RNK_13_TICK * talentFactor;
            uint32 const fireballTotal = fireballTick * fireballTickAmount;
            TEST_DOT_DAMAGE(mage, dummy, ClassSpells::Mage::FIREBALL_RNK_13, fireballTotal, false);
			uint32 const fireballMinDamage = ClassSpellsDamage::Mage::FIREBALL_RNK_13_MIN_LVL_70  * talentFactor;
			uint32 const fireballMaxDamage = ClassSpellsDamage::Mage::FIREBALL_RNK_13_MAX_LVL_70  * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage, fireballMaxDamage, false);

			// Flamestrike
            uint32 const flamestrikeTickAmount = 4;
            uint32 const flamestrikeTick = ClassSpellsDamage::Mage::FLAMESTRIKE_RNK_7_TICK * talentFactor;
            uint32 const flamestrikeTotal = flamestrikeTickAmount * flamestrikeTick;
            TEST_DOT_DAMAGE(mage, dummy, ClassSpells::Mage::FLAMESTRIKE_RNK_7, flamestrikeTotal, false);
			uint32 const flamestrikeMinDamage = ClassSpellsDamage::Mage::FLAMESTRIKE_RNK_7_MIN_LVL_70 * talentFactor;
			uint32 const flamestrikeMaxDamage = ClassSpellsDamage::Mage::FLAMESTRIKE_RNK_7_MAX_LVL_70 * talentFactor;
			TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FLAMESTRIKE_RNK_7, flamestrikeMinDamage, flamestrikeMaxDamage, false);

			// Molten armor rank 1
			uint32 const moltenArmorDamage = ClassSpellsDamage::Mage::MOLTEN_ARMOR_RNK_1 * talentFactor;
            auto callback = [](Unit* caster, Unit* victim) {
                victim->AttackerStateUpdate(caster, BASE_ATTACK);
            };
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::MOLTEN_ARMOR_RNK_1, moltenArmorDamage, moltenArmorDamage, false, callback,  ClassSpells::Mage::MOLTEN_ARMOR_RNK_1_PROC);

			// Scorch rank 9
			uint32 const scorchMinDamage = ClassSpellsDamage::Mage::SCORCH_RNK_9_MIN * talentFactor;
			uint32 const scorchMaxDamage = ClassSpellsDamage::Mage::SCORCH_RNK_9_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::SCORCH_RNK_9, scorchMinDamage, scorchMaxDamage, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<FirePowerTestImpt>();
	}
};

class EmpoweredFireballTest : public TestCaseScript
{
public:

    EmpoweredFireballTest() : TestCaseScript("talents mage empowered_fireball") { }

    class EmpoweredFireballTestImpt : public TestCase
    {
    public:
        EmpoweredFireballTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
            Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP
            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);

            LearnTalent(mage, Talents::Mage::EMPOWERED_FIREBALL_RNK_5);
            float const talentFactor = 0.15f;

            float const fireballSpellCoeff = ClassSpellsCoeff::Mage::FIREBALL + talentFactor;
            uint32 const fireballBonus = spellPower * fireballSpellCoeff;

            uint32 const fireballMinDamage = ClassSpellsDamage::Mage::FIREBALL_RNK_13_MIN_LVL_70 + fireballBonus;
            uint32 const fireballMaxDamage = ClassSpellsDamage::Mage::FIREBALL_RNK_13_MAX_LVL_70 + fireballBonus;

            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage, fireballMaxDamage, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<EmpoweredFireballTestImpt>();
    }
};

class FrostWardingTest : public TestCaseScript
{
public:
	FrostWardingTest() : TestCaseScript("talents mage frost_warding") { }

	class FrostWardingTestImpt : public TestCase
	{
	public:
		FrostWardingTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);

			LearnTalent(mage, Talents::Mage::FROST_WARDING_RNK_2);
            float const talentArmorFactor = 1.3f;
            uint32 const startArmor = mage->GetArmor();

            Wait(3000);

            // Frost Armor
            uint32 const frostArmorBonus = 200;
			uint32 const expectedArmorFA = startArmor + frostArmorBonus * talentArmorFactor;
			mage->CastSpell(mage, ClassSpells::Mage::FROST_ARMOR_RNK_3, true);
			TEST_ASSERT(mage->GetArmor() == expectedArmorFA);

            // Ice Armor
            uint32 const iceArmorBonus = 645;
            uint32 const iceArmorResBonus = 18;
			uint32 const expectedArmorIA = startArmor + iceArmorBonus * talentArmorFactor;
			uint32 const expectedFrostResIA = mage->GetResistance(SPELL_SCHOOL_FROST) + iceArmorResBonus * talentArmorFactor;
            mage->CastSpell(mage, ClassSpells::Mage::ICE_ARMOR_RNK_5, true);
            ASSERT_INFO("Mage has %u armor but %u was expected.", mage->GetArmor(), expectedArmorIA);
			TEST_ASSERT(mage->GetArmor() == expectedArmorIA);
			TEST_ASSERT(mage->GetResistance(SPELL_SCHOOL_FROST) == expectedFrostResIA);

            // TODO: Frost Ward reflects 20% of frost spells
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<FrostWardingTestImpt>();
	}
};

class PiercingIceTest : public TestCaseScript
{
public:

	PiercingIceTest() : TestCaseScript("talents mage piercing_ice") { }

	class PiercingIceTestImpt : public TestCase
	{
	public:
		PiercingIceTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

			LearnTalent(mage, Talents::Mage::PIERCING_ICE_RNK_3);
            float const talentFactor = 1.06f;

			// Blizzard
            uint32 const blizzardTick = ClassSpellsDamage::Mage::BLIZZARD_RNK_7_TICK * talentFactor;
            TEST_CHANNEL_DAMAGE(mage, dummy, ClassSpells::Mage::BLIZZARD_RNK_7, 8, blizzardTick, ClassSpells::Mage::BLIZZARD_RNK_7_PROC);

			// Cone of cold
			uint32 const coneMinDamage = ClassSpellsDamage::Mage::CONE_OF_COLD_RNK_6_MIN * talentFactor;
			uint32 const coneMaxDamage = ClassSpellsDamage::Mage::CONE_OF_COLD_RNK_6_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::CONE_OF_COLD_RNK_6, coneMinDamage, coneMaxDamage, false);

			// Frost nova
			uint32 const novaMinDamage = ClassSpellsDamage::Mage::FROST_NOVA_RNK_5_MIN * talentFactor;
			uint32 const novaMaxDamage = ClassSpellsDamage::Mage::FROST_NOVA_RNK_5_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROST_NOVA_RNK_5, novaMinDamage, novaMaxDamage, false);

			// Frostbolt
			uint32 const frostboltMinDamage = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_LVL_70_MIN * talentFactor;
			uint32 const frostboltMaxDamage = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_LVL_70_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMinDamage, frostboltMaxDamage, false);

			// Ice lance
			uint32 const lanceMinDamage = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN * talentFactor;
			uint32 const lanceMaxDamage = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, lanceMinDamage, lanceMaxDamage, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<PiercingIceTestImpt>();
	}
};

class ImprovedConeOfColdTest : public TestCaseScript
{
public:

	ImprovedConeOfColdTest() : TestCaseScript("talents mage improved_cone_of_cold") { }

	class ImprovedConeOfColdTestImpt : public TestCase
	{
	public:
		ImprovedConeOfColdTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

			LearnTalent(mage, Talents::Mage::IMPROVED_CONE_OF_COLD_RNK_3);
            float const talentFactor = 1.35f;

			uint32 const coneMinDamage = ClassSpellsDamage::Mage::CONE_OF_COLD_RNK_6_MIN * talentFactor;
			uint32 const coneMaxDamage = ClassSpellsDamage::Mage::CONE_OF_COLD_RNK_6_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::CONE_OF_COLD_RNK_6, coneMinDamage, coneMaxDamage, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ImprovedConeOfColdTestImpt>();
	}
};

class ArcticWindsTest : public TestCaseScript
{
public:

	ArcticWindsTest() : TestCaseScript("talents mage arctic_winds") { }

	class ArcticWindsTestImpt : public TestCase
	{
	public:
		ArcticWindsTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

		void Test() override
		{
			TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_TROLL);
			Creature* dummy = SpawnCreature();

			LearnTalent(mage, Talents::Mage::ARCTIC_WINDS_RNK_5);
            float const talentFactor = 1.05f;
            float const talentMissFactor = 5.f;

            // Blizzard
            uint32 const blizzardTick = ClassSpellsDamage::Mage::BLIZZARD_RNK_7_TICK * talentFactor;
            TEST_CHANNEL_DAMAGE(mage, dummy, ClassSpells::Mage::BLIZZARD_RNK_7, 8, blizzardTick, ClassSpells::Mage::BLIZZARD_RNK_7_PROC);

            // Cone of cold
            uint32 const coneMinDamage = ClassSpellsDamage::Mage::CONE_OF_COLD_RNK_6_MIN * talentFactor;
            uint32 const coneMaxDamage = ClassSpellsDamage::Mage::CONE_OF_COLD_RNK_6_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::CONE_OF_COLD_RNK_6, coneMinDamage, coneMaxDamage, false);

            // Frost nova
            uint32 const novaMinDamage = ClassSpellsDamage::Mage::FROST_NOVA_RNK_5_MIN * talentFactor;
            uint32 const novaMaxDamage = ClassSpellsDamage::Mage::FROST_NOVA_RNK_5_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROST_NOVA_RNK_5, novaMinDamage, novaMaxDamage, false);

            // Frostbolt
            uint32 const frostboltMinDamage = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_LVL_70_MIN * talentFactor;
            uint32 const frostboltMaxDamage = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_LVL_70_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMinDamage, frostboltMaxDamage, false);

            // Ice lance
            uint32 const lanceMinDamage = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MIN * talentFactor;
            uint32 const lanceMaxDamage = ClassSpellsDamage::Mage::ICE_LANCE_RNK_1_MAX * talentFactor;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::ICE_LANCE_RNK_1, lanceMinDamage, lanceMaxDamage, false);

            // Hit rating from melee and ranged attackers is reduced
            TestPlayer* hunter = SpawnPlayer(CLASS_HUNTER, RACE_DWARF);
            float const expectedMissChance = 5.f + talentMissFactor; // Same level players have a 5% miss chance
            TEST_MELEE_HIT_CHANCE(hunter, mage, BASE_ATTACK, expectedMissChance, MELEE_HIT_MISS);
            //TEST_MELEE_HIT_CHANCE(hunter, mage, RANGED_ATTACK, expectedMissChance, MELEE_HIT_MISS);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ArcticWindsTestImpt>();
	}
};

class EmpoweredFrostboltTest : public TestCaseScript
{
public:

	EmpoweredFrostboltTest() : TestCaseScript("talents mage empowered_frostbolt") { }

	class EmpoweredFrostboltTestImpt : public TestCase
	{
	public:
		EmpoweredFrostboltTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummy = SpawnCreature();

            EQUIP_NEW_ITEM(mage, 34336); // Sunflare - 292 SP
            uint32 const spellPower = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST);

			LearnTalent(mage, Talents::Mage::EMPOWERED_FROSTBOLT_RNK_5);
            float const talentSpellCoeffBonus = 0.1f;
            float const talentFrostboltCritBonus = 5.f;

            // Damage
			float const frostboltSpellCoeff = ClassSpellsCoeff::Mage::FROSTBOLT + talentSpellCoeffBonus;
			uint32 const frostboltBonus = mage->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST) * frostboltSpellCoeff;
			uint32 const frostboltMinDamage = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_LVL_70_MIN + frostboltBonus;
			uint32 const frostboltMaxDamage = ClassSpellsDamage::Mage::FROSTBOLT_RNK_13_LVL_70_MAX + frostboltBonus;
            TEST_DIRECT_SPELL_DAMAGE(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMinDamage, frostboltMaxDamage, false);

            // Crit chance
            float const expectedCrit = mage->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FROST) + talentFrostboltCritBonus;
            TEST_SPELL_CRIT_CHANCE(mage, dummy, ClassSpells::Mage::FROSTBOLT_RNK_13, expectedCrit);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<EmpoweredFrostboltTestImpt>();
	}
};

void AddSC_test_talents_mage()
{
	// Arcane
	new MageWandSpecializationTest();
	new MagicAbsorptionTest();
	new MagicAttunementTest();
	new ArcaneFortitudeTest();
	new ImprovedCounterspellTest();
	new ArcaneMindTest();
	new ArcaneInstabilityTest();
	new EmpoweredArcaneMissilesTest();
	new MindMasteryTest();
	// Fire
	new CriticalMassTest();
	new FirePowerTest();
    new EmpoweredFireballTest();
	// Frost
	new FrostWardingTest();
	new PiercingIceTest();
	new ImprovedConeOfColdTest();
	new ArcticWindsTest();
	new EmpoweredFrostboltTest();
}
