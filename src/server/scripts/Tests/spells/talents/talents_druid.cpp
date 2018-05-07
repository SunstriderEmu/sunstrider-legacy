#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"
#include "SpellHistory.h"

// Helpers to get shapeshift base damage
// http://wowwiki.wikia.com/wiki/Attack_power?oldid=1715193

uint32 GetMoonkinBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startStr = player->GetStat(STAT_STRENGTH);


	uint32 const moonkinBonus = level * 1.5f; // From tooltips
	uint32 const moonkinLevelBonus = level * 1.5f;
	uint32 const moonkinMeleeBase = startStr * 2 + moonkinLevelBonus - 20;

	return moonkinMeleeBase + moonkinBonus;
}

uint32 GetCatBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startAgi = player->GetStat(STAT_AGILITY);
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const catBonus = 40 + startAgi; // From tooltip
	uint32 const catLevelBonus = level * 2.0f;
	uint32 const catMeleeBase = startStr * 2 + startAgi + catLevelBonus - 20;

	return catMeleeBase + catBonus;
}

uint32 GetBearBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const bearBonus = 30; // From tooltip
	uint32 const bearLevelBonus = level * 3.0f;
	uint32 const bearMeleeBase = startStr * 2 + bearLevelBonus - 20;

	return bearMeleeBase + bearBonus;
}

uint32 GetDireBearBaseAP(TestPlayer* player, uint32 level = 70)
{
	uint32 const startStr = player->GetStat(STAT_STRENGTH);

	uint32 const direBearBonus = 120; // From tooltips
	uint32 const bearLevelBonus = level * 3.0f;
	uint32 const bearMeleeBase = startStr * 2 + bearLevelBonus - 20;

	return bearMeleeBase + direBearBonus;
}

class StarlightWrathTest : public TestCaseScript
{
public:

	StarlightWrathTest() : TestCaseScript("talents druid starlight_wrath") { }

	class StarlightWrathTestImpt : public TestCase
	{
	public:
		StarlightWrathTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummy = SpawnCreature();

			LearnTalent(player, Talents::Druid::STARLIGHT_WRATH_RNK_5);

			uint32 res = player->CastSpell(dummy, ClassSpells::Druid::WRATH_RNK_10);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Spell* spell = player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
			TEST_ASSERT(spell != nullptr);
			ASSERT_INFO("Spell %u has a cast time > 1.5s", spell->GetSpellInfo()->Id);
			TEST_ASSERT(spell->GetCastTime() == 1500);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<StarlightWrathTestImpt>();
	}
};

class ImprovedNaturesGraspTest : public TestCaseScript
{
public:

	ImprovedNaturesGraspTest() : TestCaseScript("talents druid improved_natures_grasp") { }

	class ImprovedNaturesGraspTestImpt : public TestCase
	{
	public:
		ImprovedNaturesGraspTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, -3.0f);
			TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_HUMAN, 70, spawnPosition); // Spawn behind the druid

			Wait(5000);

			LearnTalent(player, Talents::Druid::NATURES_GRASP_RNK_1);
			LearnTalent(player, Talents::Druid::IMPROVED_NATURES_GRASP_RNK_4);

			uint32 res = player->CastSpell(player, ClassSpells::Druid::NATURES_GRASP_RNK_7);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(5000);
			ASSERT_INFO("Druid has not Nature's Grasp aura");
            TEST_HAS_AURA(player, ClassSpells::Druid::NATURES_GRASP_RNK_7);
			warrior->Attack(player, true);
			Wait(1500);
			ASSERT_INFO("Druid still has aura");
            TEST_HAS_NOT_AURA(player, ClassSpells::Druid::NATURES_GRASP_RNK_7);
			ASSERT_INFO("Warrior isnt root");
            TEST_HAS_AURA(warrior, ClassSpells::Druid::NATURES_GRASP_RNK_7_PROC);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedNaturesGraspTestImpt>();
	}
};

class InsectSwarmTest : public TestCaseScript
{
public:
	InsectSwarmTest() : TestCaseScript("talents druid insect_swarm") { }

	class InsectSwarmTestImpt : public TestCase
	{
	public:
		InsectSwarmTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedInsectSwarmMana = 175;
			TEST_POWER_COST(druid, ClassSpells::Druid::INSECT_SWARM_RNK_6, POWER_MANA, expectedInsectSwarmMana);

			// Spell coefficient
			float const wrathSpellCoeff = 12.0f / 15.0f;
			uint32 const wrathBonusSP = wrathSpellCoeff * staffSP;

			// Damage
			uint32 const expectedInsectSwarmTotalDmg = ClassSpellsDamage::Druid::INSECT_SWARM_RNK_6_TOTAL + wrathBonusSP;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::INSECT_SWARM_RNK_6, expectedInsectSwarmTotalDmg, false);

			// TODO: -2% hit chance
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<InsectSwarmTestImpt>();
	}
};

class NaturesReachTest : public TestCaseScript
{
public:

	NaturesReachTest() : TestCaseScript("talents druid natures_reach") { }

	class NaturesReachTestImpt : public TestCase
	{
	public:
		NaturesReachTestImpt() : TestCase(STATUS_PASSING) { }

		void TestSpell(TestPlayer* player, Unit* target, uint32 spellId, uint32 castTime = 1500)
		{
			uint32 res = player->CastSpell(target, spellId);
			ASSERT_INFO("Druid couldn't cast %u", spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(castTime);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			player->SetMaxPower(POWER_MANA, 10000);
			player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
			Wait(1000);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 48.0f);
			Creature* range48 = SpawnCreatureWithPosition(spawnPosition, 26); // Beast for Soothe Animal - 48m

			Position spawnPosition2(_location);
			spawnPosition2.MoveInFront(_location, 36.0f);
			Creature* range36 = SpawnCreatureWithPosition(spawnPosition2, 20); // Draconid for Hibernate - 36m
			TestPlayer* player36 = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, 70, spawnPosition2); // Player for Innervate - 36m

			Position spawnPosition3(_location);
			spawnPosition3.MoveInFront(_location, 24.0f);
			TestPlayer* range24 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition3); // 24m

			LearnTalent(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			LearnTalent(player, Talents::Druid::FAERIE_FIRE_FERAL_RNK_1);
			LearnTalent(player, Talents::Druid::NATURES_REACH_RNK_2);

			TestSpell(player, range24, ClassSpells::Druid::CYCLONE_RNK_1, 1500);
			TestSpell(player, range36, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, 1500);
			TestSpell(player, range36, ClassSpells::Druid::FAERIE_FIRE_RNK_5);
			TestSpell(player, range36, ClassSpells::Druid::HURRICANE_RNK_4, 10000);
			TestSpell(player, player36, ClassSpells::Druid::INNERVATE_RNK_1);
			TestSpell(player, range36, ClassSpells::Druid::MOONFIRE_RNK_12);
			TestSpell(player, range48, ClassSpells::Druid::SOOTHE_ANIMAL_RNK_4);
			TestSpell(player, range36, ClassSpells::Druid::STARFIRE_RNK_8, 3000);
			TestSpell(player, range36, ClassSpells::Druid::WRATH_RNK_10, 2000);
			TestSpell(player, player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			TestSpell(player, range36, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<NaturesReachTestImpt>();
	}
};

class LunarGuidanceTest : public TestCaseScript
{
public:
	LunarGuidanceTest() : TestCaseScript("talents druid lunar_guidance") { }

	class LunarGuidanceTestImpt : public TestCase
	{
	public:
		LunarGuidanceTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			uint32 const intellect = player->GetStat(STAT_INTELLECT);
			uint32 const startHealing = player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL);
			uint32 const startDamage = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL);

			uint32 const expectedHealing = startHealing + intellect * 0.25f;
			uint32 const expectedDamage = startDamage + intellect * 0.25f;

			// Test improved
			LearnTalent(player, Talents::Druid::LUNAR_GUIDANCE_RNK_3);
			TEST_ASSERT(Between<float>(player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedHealing - 1, expectedHealing + 1));
			TEST_ASSERT(Between<float>(player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL), expectedDamage - 1, expectedDamage + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<LunarGuidanceTestImpt>();
	}
};

class MoonglowTest : public TestCaseScript
{
public:
	MoonglowTest() : TestCaseScript("talents druid moonglow") { }

	class MoonglowTestImpt : public TestCase
	{
	public:
		MoonglowTestImpt() : TestCase(STATUS_PASSING) { }

		void TestMana(TestPlayer* player, Unit* target, uint32 spellId, uint32 expectedManaCost, uint32 castTime = 1500)
		{
			player->SetPower(POWER_MANA, expectedManaCost);

			uint32 res = player->CastSpell(target, spellId);
			ASSERT_INFO("Issue casting %u", spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(castTime + 1000);
			ASSERT_INFO("Some mana remained after casting %u", spellId);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 0);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::MOONGLOW_RNK_3);

			float const moonglowFactor = 1 - 3 * 0.03f;
			uint32 const expectedMoonfireMana = 495 * moonglowFactor;
			uint32 const expectedStarfireMana = 370 * moonglowFactor;
			uint32 const expectedWrathMana = 255 * moonglowFactor;
			uint32 const expectedHealingTouchMana = 935 * moonglowFactor;
			uint32 const expectedRegrowthMana = 675 * moonglowFactor;
			uint32 const expectedRejuvenationMana = 415 * moonglowFactor;

			TestMana(player, creature, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireMana);
			TestMana(player, creature, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMana, 3000);
			TestMana(player, creature, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMana, 2000);
			TestMana(player, player, ClassSpells::Druid::HEALING_TOUCH_RNK_13, expectedHealingTouchMana, 3500);
			TestMana(player, player, ClassSpells::Druid::REGROWTH_RNK_10, expectedRegrowthMana, 2000);
			TestMana(player, player, ClassSpells::Druid::REJUVENATION_RNK_13, expectedRejuvenationMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MoonglowTestImpt>();
	}
};

class MoonfuryTest : public TestCaseScript
{
public:

	MoonfuryTest() : TestCaseScript("talents druid Moonfury") { }

	class MoonfuryTestImpt : public TestCase
	{
	public:
		MoonfuryTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			// Starfire rank 8
			uint32 const starfireMinDamage = 550;
			uint32 const starfireMaxDamage = 647;

			// Moonfire rank 12
			uint32 const moonfireMinDamage = 305;
			uint32 const moonfireMaxDamage = 357;

			// Wrath rank 10
			uint32 const wrathMinDamage = 383;
			uint32 const wrathMaxDamage = 432;

			Creature* dummyTarget = SpawnCreature();

			//Test improved damage 5%
			LearnTalent(player, Talents::Druid::MOONFURY_RNK_5);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::STARFIRE_RNK_8, starfireMinDamage * 1.1f, starfireMaxDamage * 1.1f, false);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::MOONFIRE_RNK_12, moonfireMinDamage * 1.1f, moonfireMaxDamage * 1.1f, false);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::WRATH_RNK_10, wrathMinDamage * 1.1f, wrathMaxDamage * 1.1f, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MoonfuryTestImpt>();
	}
};

class DreamstateTest : public TestCaseScript
{
public:
	DreamstateTest() : TestCaseScript("talents druid dreamstate") { }

	class DreamstateTestImpt : public TestCase
	{
	public:
		DreamstateTestImpt() : TestCase(STATUS_PASSING) { }

		void TestRegen(TestPlayer* player, uint32 expectedMana)
		{
			uint32 currentMana = player->GetPower(POWER_MANA);
			ASSERT_INFO("Druid has %u mana, expected: %u", currentMana, expectedMana);
			TEST_ASSERT(currentMana == expectedMana);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			const float regenTick = sqrt(player->GetStat(STAT_INTELLECT)) * player->OCTRegenMPPerSpirit() * 2;
			uint32 const expectedRegenTick = player->GetStat(STAT_INTELLECT) * 0.1f / 5 * 2;

			LearnTalent(player, Talents::Druid::DREAMSTATE_RNK_3);

			player->SetMaxPower(POWER_MANA, 675 + 415); // Regrowth + Rejuvenation
			uint32 res = player->CastSpell(player, ClassSpells::Druid::REGROWTH_RNK_10);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2500);
			res = player->CastSpell(player, ClassSpells::Druid::REJUVENATION_RNK_13);
			TEST_ASSERT(res == SPELL_CAST_OK);
			uint32 expectedMana = expectedRegenTick;
			TestRegen(player, expectedMana);

			Wait(2000);
			expectedMana = 2 * expectedRegenTick;
			TestRegen(player, expectedMana);

			Wait(2000);
			expectedMana = 3 * expectedRegenTick;
			TestRegen(player, expectedMana);

			Wait(2000);
			expectedMana = 4 * expectedRegenTick + regenTick; // Out of FSR
			TestRegen(player, expectedMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DreamstateTestImpt>();
	}
};

class WrathOfCenariusTest : public TestCaseScript
{
public:

	WrathOfCenariusTest() : TestCaseScript("talents druid wrath_of_cenarius") { }

	class WrathOfCenariusTestImpt : public TestCase
	{
	public:
		WrathOfCenariusTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummyTarget = SpawnCreature();

			LearnTalent(player, Talents::Druid::WRATH_OF_CENARIUS_RNK_5);

			EQUIP_NEW_ITEM(player, 34182); // Grand Magister's Staff of Torrents - 266 SP

			float const starfireFactor	= 5 * 0.04f;
			float const wrathFactor		= 5 * 0.02f;

			float const arcaneSP = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ARCANE);
			float const natureSP = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE);

			uint32 const expectedStarfireMinDamage = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN + arcaneSP * (ClassSpellsCoeff::Druid::STARFIRE + starfireFactor);
			uint32 const expectedStarfireMaxDamage = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX + arcaneSP * (ClassSpellsCoeff::Druid::STARFIRE + starfireFactor);

			uint32 const expectedWrathMinDamage = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN + natureSP * (ClassSpellsCoeff::Druid::WRATH + wrathFactor);
			uint32 const expectedWrathMaxDamage = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX + natureSP * (ClassSpellsCoeff::Druid::WRATH + wrathFactor);

			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDamage, expectedStarfireMaxDamage, false);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDamage, expectedWrathMaxDamage, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<WrathOfCenariusTestImpt>();
	}
};

class FerocityTest : public TestCaseScript
{
public:

	FerocityTest() : TestCaseScript("talents druid ferocity") { }

	class FerocityTestImpt : public TestCase
	{
	public:
		FerocityTestImpt() : TestCase(STATUS_PASSING) { }

		void TestPower(TestPlayer* player, Unit* victim, uint32 spellId, uint32 expectedPower, Powers power)
		{
			player->SetPower(power, expectedPower);
			Wait(500);
			uint32 res = player->CastSpell(victim, spellId);
			ASSERT_INFO("Druid couldnt cast %u, result: %s", spellId, StringifySpellCastResult(res).c_str());
			TEST_ASSERT(res == SPELL_CAST_OK);
			ASSERT_INFO("Druid had some %u remaining power after %u", player->GetPower(power), spellId);
			TEST_ASSERT(player->GetPower(power) == 0);
			Wait(2000); // GCD
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* creature = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);

			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::FEROCITY_RNK_5);

			int32 const ferocityReduction = 5;
			uint32 const expectedMaulRage	= (15 - ferocityReduction) * 10;
			uint32 const expectedSwipeRage	= (20 - ferocityReduction) * 10;
			uint32 const expectedMangleRage = (20 - ferocityReduction) * 10;

			uint32 const expectedClawEnergy		= (45 - ferocityReduction) * 10;
			uint32 const expectedRakeEnergy		= (40 - ferocityReduction) * 10;
			uint32 const expectedMangleEnergy	= (45 - ferocityReduction) * 10;

			uint32 res = player->CastSpell(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000); // GCD
            TC_LOG_TRACE("test.unit_test", "power: %u", player->GetPower(POWER_RAGE));
			TestPower(player, creature, ClassSpells::Druid::MAUL_RNK_8, expectedMaulRage, POWER_RAGE);
			TestPower(player, creature, ClassSpells::Druid::SWIPE_RNK_6, expectedSwipeRage, POWER_RAGE);
			TestPower(player, creature, ClassSpells::Druid::MANGLE_BEAR_RNK_3, expectedMangleRage, POWER_RAGE);
			player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);

			res = player->CastSpell(player, ClassSpells::Druid::CAT_FORM_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TestPower(player, player, ClassSpells::Druid::MOONKIN_FORM_RNK_1, expectedClawEnergy, POWER_ENERGY);
			TestPower(player, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeEnergy, POWER_ENERGY);
			TestPower(player, creature, ClassSpells::Druid::MANGLE_CAT_RNK_3, expectedMangleEnergy, POWER_ENERGY);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FerocityTestImpt>();
	}
};

class BrutalImpactTest : public TestCaseScript
{
public:

	BrutalImpactTest() : TestCaseScript("talents druid brutal_impact") { }

	class BrutalImpactTestImpt : public TestCase
	{
	public:
		BrutalImpactTestImpt() : TestCase(STATUS_PASSING) { }

		void TestPower(TestPlayer* player, Unit* target, uint32 spellId, int32 auraTime, Powers power)
		{
			player->SetPower(power, player->GetMaxPower(power));
			uint32 res = player->CastSpell(target, spellId);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", spellId, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Aura* aura = target->GetAura(spellId);
			TEST_ASSERT(aura != nullptr);
			ASSERT_INFO("Victim doesnt have aura with duration %u, but: %u", auraTime, aura->GetDuration());
			TEST_ASSERT(aura->GetDuration() == auraTime);
			Wait(2000); // GCD
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);

			LearnTalent(player, Talents::Druid::BRUTAL_IMPACT_RNK_2);

			// Bash
			uint32 res = player->CastSpell(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", ClassSpells::Druid::BEAR_FORM_RNK_1, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000); // GCD
			TestPower(player, enemy, ClassSpells::Druid::BASH_RNK_3, 5000, POWER_RAGE);
			player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(4000); // Get Out Of Combat

			// Pounce
			res = player->CastSpell(player, ClassSpells::Druid::CAT_FORM_RNK_1);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", ClassSpells::Druid::CAT_FORM_RNK_1, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000); // GCD
			res = player->CastSpell(player, ClassSpells::Druid::PROWL_RNK_3);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", ClassSpells::Druid::PROWL_RNK_3, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000); // GCD
			TestPower(player, enemy, ClassSpells::Druid::POUNCE_RNK_4, 2000, POWER_ENERGY); // Diminishing returns makes it from 4000 to 2000
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<BrutalImpactTestImpt>();
	}
};

class ThickHideTest : public TestCaseScript
{
public:
	ThickHideTest() : TestCaseScript("talents druid thick_hide") { }

	class ThickHideTestImpt : public TestCase
	{
	public:
		ThickHideTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			RemoveAllEquipedItems(player);

			uint32 const startingArmorViaAgi = player->GetStat(STAT_AGILITY) * 2;
			uint32 const startingArmor = player->GetArmor() - startingArmorViaAgi;

			EQUIP_NEW_ITEM(player, 34392); // Demontooth Shoulderpads - 514 Armor (140 hidden) - 38 Agi

			uint32 const itemArmor = 514 + 140;
			uint32 const itemAgi = 38 * 2;
			uint32 expectedArmor = startingArmor + startingArmorViaAgi + itemArmor + itemAgi;
			ASSERT_INFO("Druid armor: %u armor, expected: %u", player->GetArmor(), expectedArmor);
			TEST_ASSERT(player->GetArmor() == expectedArmor);

			LearnTalent(player, Talents::Druid::THICK_HIDE_RNK_3);
			expectedArmor = (startingArmor + itemArmor) * 1.1f + startingArmorViaAgi + itemAgi;
			ASSERT_INFO("Druid armor: %u armor, expected: %u", player->GetArmor(), expectedArmor);
			TEST_ASSERT(player->GetArmor() == expectedArmor);

			player->AddAura(33079, player); // Scroll of Protection V - 300 armor
			ASSERT_INFO("Druid armor: %u armor, expected: %u", player->GetArmor(), expectedArmor);
			TEST_ASSERT(player->GetArmor() == expectedArmor);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ThickHideTestImpt>();
	}
};

class ShreddingAttacksTest : public TestCaseScript
{
public:

	ShreddingAttacksTest() : TestCaseScript("talents druid shredding_attacks") { }

	class ShreddingAttacksTestImpt : public TestCase
	{
	public:
		ShreddingAttacksTestImpt() : TestCase(STATUS_PASSING) { }

		void TestPower(TestPlayer* player, Unit* victim, uint32 spellId, uint32 expectedPower, Powers power)
		{
			player->SetPower(power, expectedPower);
			uint32 res = player->CastSpell(victim, spellId);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", spellId, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			ASSERT_INFO("Druid had %u remaining power after %u", player->GetPower(power), spellId);
			TEST_ASSERT(player->GetPower(power) == 0);
			Wait(2000); // GCD
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);

			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::SHREDDING_ATTACKS_RNK_2);

			uint32 const expectedLacerateRage = (15 - 2) * 10;
			uint32 const expectedShredEnergy = (60 - 18) * 10;

			uint32 res = player->CastSpell(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000);
			TestPower(player, enemy, ClassSpells::Druid::LACERATE_RNK_1, expectedLacerateRage, POWER_RAGE);
			player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);

			res = player->CastSpell(player, ClassSpells::Druid::CAT_FORM_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000);
			TestPower(player, enemy, ClassSpells::Druid::SHRED_RNK_7, expectedShredEnergy, POWER_ENERGY);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ShreddingAttacksTestImpt>();
	}
};

class PredatoryStrikesTest : public TestCaseScript
{
public:

	PredatoryStrikesTest() : TestCaseScript("talents druid predatory_strikes") { }

	class PredatoryStrikesTestImpt : public TestCase
	{
	public:
		PredatoryStrikesTestImpt() : TestCase(STATUS_PASSING) { }

		void TestAP(TestPlayer* player, uint32 spellId, uint32 expectedAP)
		{
			uint32 res = player->CastSpell(player, spellId);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", spellId, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000);
			ASSERT_INFO("Druid had %f AP after %u, %u expected", player->GetTotalAttackPowerValue(BASE_ATTACK), spellId, expectedAP);
			TEST_ASSERT(player->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP);
			player->RemoveAurasDueToSpell(spellId);
			Wait(2000); // GCD
		}

		void Test() override
		{
			uint32 const level = 70;
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, level);
			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::PREDATORY_STRIKES_RNK_3);
			uint32 const predatoryStrikesBonus = level * 1.5f;

			uint32 const expectedMoonkinAP	= GetMoonkinBaseAP(player, level)  + predatoryStrikesBonus;
			uint32 const expectedCatAP		= GetCatBaseAP(player, level)	   + predatoryStrikesBonus;
			uint32 const expectedBearAP		= GetBearBaseAP(player, level)	   + predatoryStrikesBonus;
			uint32 const expectedDireBearAP = GetDireBearBaseAP(player, level) + predatoryStrikesBonus;

			TestAP(player, ClassSpells::Druid::MOONKIN_FORM_RNK_1, expectedMoonkinAP); // OK
			TestAP(player, ClassSpells::Druid::CAT_FORM_RNK_1, expectedCatAP);
			TestAP(player, ClassSpells::Druid::BEAR_FORM_RNK_1, expectedBearAP);
			TestAP(player, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, expectedDireBearAP);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<PredatoryStrikesTestImpt>();
	}
};

class SavageFuryTest : public TestCaseScript
{
public:

	SavageFuryTest() : TestCaseScript("talents druid savage_fury") { }

	class SavageFuryTestImpt : public TestCase
	{
	public:
		SavageFuryTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();
			creature->DisableRegeneration(true);

			EQUIP_NEW_ITEM(player, 30883); // Stanchion of Primal Instinct - 136-293 damage
			LearnTalent(player, Talents::Druid::SAVAGE_FURY_RNK_2);

			uint32 res = player->CastSpell(player, ClassSpells::Druid::CAT_FORM_RNK_1);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", ClassSpells::Druid::CAT_FORM_RNK_1, res);
			TEST_ASSERT(res == SPELL_CAST_OK);

			float const AP = player->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const weaponMinDamage = 136 + (AP / 14);
			float const weaponMaxDamage = 293 + (AP / 14);
			float const savageFuryFactor = 1.2f;

			// Mangle (Cat)
			uint32 const expectedMangleMinDmg = (weaponMinDamage * 1.6f + 264) * armorFactor * savageFuryFactor;
			uint32 const expectedMangleMaxDmg = (weaponMaxDamage * 1.6f + 264) * armorFactor * savageFuryFactor;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Druid::MANGLE_CAT_RNK_3, expectedMangleMinDmg, expectedMangleMaxDmg, false);

			// Rake bleed
			uint32 const expectedRakeBleed = floor(AP * 0.06f + 108) * savageFuryFactor;
			TEST_DOT_DAMAGE(player, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeBleed, false);

			// Rake initial damage
			uint32 const expectedRakeDmg = floor(AP / 100 + 78) * savageFuryFactor;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeDmg, expectedRakeDmg, false);

			// Claw
			uint32 const expectedClawMin = ClassSpellsDamage::Druid::CLAW_RNK_6 + weaponMinDamage * armorFactor * savageFuryFactor;
			uint32 const expectedClawMax = ClassSpellsDamage::Druid::CLAW_RNK_6 + weaponMaxDamage * armorFactor * savageFuryFactor;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Druid::CLAW_RNK_6, expectedClawMin, expectedClawMax, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SavageFuryTestImpt>();
	}
};

class FaerieFireFeralTest : public TestCaseScript
{
public:

	FaerieFireFeralTest() : TestCaseScript("talents druid faerie_fire_feral") { }

	class FaerieFireFeralTestImpt : public TestCase
	{
	public:
		FaerieFireFeralTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);
			TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawnPosition);

			EQUIP_NEW_ITEM(rogue, 34211); // S4 Chest
			int32 const expectedRogueArmor = rogue->GetArmor() - 610;

			// Only cast in Bear or Cat form
            TEST_CAST(player, player, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5, SPELL_FAILED_ONLY_SHAPESHIFT);

			// Bear Form
            TEST_CAST(player, player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(2000); // GCD

			// Faerie Fire Feral
            TEST_CAST(player, rogue, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5);
			Aura* aura = rogue->GetAura(ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5);
			TEST_ASSERT(aura != nullptr);
			ASSERT_INFO("Rogue has %u armor, expected: %i", rogue->GetArmor(), expectedRogueArmor);
			TEST_ASSERT(int32(rogue->GetArmor()) == expectedRogueArmor);
			TEST_ASSERT(aura->GetDuration() == 40 * SECOND * IN_MILLISECONDS);
            TEST_HAS_COOLDOWN(player, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5, Seconds(6));
			Wait(2000);

			// Rogue can't stealth
            TEST_CAST(rogue, rogue, ClassSpells::Rogue::STEALTH_RNK_4, SPELL_FAILED_CASTER_AURASTATE);

			// Cat form
			player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_CAST(player, player, ClassSpells::Druid::CAT_FORM_RNK_1);
			Wait(2000); // GCD

			// Mage can't invisible
            player->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(player, mage, ClassSpells::Druid::FAERIE_FIRE_FERAL_RNK_5);
            TEST_CAST(mage, mage, ClassSpells::Mage::INVISIBILITY_RNK_1, SPELL_FAILED_CASTER_AURASTATE);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FaerieFireFeralTestImpt>();
	}
};

class HeartOfTheWildTest : public TestCaseScript
{
public:
	HeartOfTheWildTest() : TestCaseScript("talents druid heart_of_the_wild") { }

	class HeartOfTheWildTestImpt : public TestCase
	{
	public:
		HeartOfTheWildTestImpt() : TestCase(STATUS_PASSING) { }

		void TestStat(TestPlayer* player, uint32 formSpellId, uint32 expectedStat, uint32 stat = 0)
		{
            TEST_CAST(player, player, formSpellId);
			if (stat == STAT_STAMINA)
			{
				TEST_ASSERT(player->GetStat(STAT_STAMINA) == expectedStat);
			}
			else
			{
				float const currentAP = player->GetTotalAttackPowerValue(BASE_ATTACK);
				ASSERT_INFO("Druid has %f AP, %u expected", currentAP, expectedStat);
				TEST_ASSERT(currentAP == expectedStat);
			}
			player->RemoveAurasDueToSpell(formSpellId);
			Wait(2000); // GCD
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			uint32 const expectedInt = player->GetStat(STAT_INTELLECT) * 1.2f;
			uint32 const expectedSta = player->GetStat(STAT_STAMINA) * 1.25f * 1.2f;
			uint32 const expectedAP  = GetCatBaseAP(player, 70) * 1.1f;

			LearnTalent(player, Talents::Druid::HEART_OF_THE_WILD_RNK_5);
			TEST_ASSERT(player->GetStat(STAT_INTELLECT) == expectedInt);
			TestStat(player, ClassSpells::Druid::BEAR_FORM_RNK_1, expectedSta, STAT_STAMINA);
			TestStat(player, ClassSpells::Druid::CAT_FORM_RNK_1, expectedAP);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<HeartOfTheWildTestImpt>();
	}
};

class SurvivalOfTheFittestTest : public TestCaseScript
{
public:
	SurvivalOfTheFittestTest() : TestCaseScript("talents druid survival_of_the_fittest") { }

	class SurvivalOfTheFittestTestImpt : public TestCase
	{
	public:
		SurvivalOfTheFittestTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

		void TestStat(TestPlayer* player, Stats stat, uint32 expectedStat)
		{
			ASSERT_INFO("Druid stat %u is: %f, expected: %u", stat, player->GetStat(stat), expectedStat);
			TEST_ASSERT(player->GetStat(stat) == expectedStat);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			uint32 const expectedAgi = player->GetStat(STAT_AGILITY) * 1.03f;
			uint32 const expectedInt = player->GetStat(STAT_INTELLECT) * 1.03f;
			uint32 const expectedSpi = player->GetStat(STAT_SPIRIT) * 1.03f;
			uint32 const expectedSta = player->GetStat(STAT_STAMINA) * 1.03f;
			uint32 const expectedStr = player->GetStat(STAT_STRENGTH) * 1.03f;

			LearnTalent(player, Talents::Druid::SURVIVAL_OF_THE_FITTEST_RNK_3);
			TestStat(player, STAT_AGILITY, expectedAgi);
			TestStat(player, STAT_INTELLECT, expectedInt);
			TestStat(player, STAT_SPIRIT, expectedSpi);
			TestStat(player, STAT_STAMINA, expectedSta);
			TestStat(player, STAT_STRENGTH, expectedStr);

			// TODO: -3% chance to be critically hit by melee
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SurvivalOfTheFittestTestImpt>();
	}
};

class MangleTest : public TestCaseScript
{
public:
	MangleTest() : TestCaseScript("talents druid mangle") { }

	class MangleTestImpt : public TestCase
	{
	public:
		MangleTestImpt() : TestCase(STATUS_PASSING) { }

		void MorphTo(TestPlayer* druid, uint32 formSpellId)
		{
			uint32 res = druid->CastSpell(druid, formSpellId);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", formSpellId, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
		}

		void TestA(TestPlayer* caster, Unit* target, uint32 spellID)
		{
			// Cast a spell
			// Check if resist miss etc
			// continue if hit
			Player* _casterOwner = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
			TestPlayer* casterOwner = dynamic_cast<TestPlayer*>(_casterOwner);
			ASSERT_INFO("Caster in not a testing bot (or a pet/summon of testing bot)");
			TEST_ASSERT(casterOwner != nullptr);
			auto AI = caster->ToPlayer()->GetTestingPlayerbotAI();
			ASSERT_INFO("Caster in not a testing bot");
			TEST_ASSERT(AI != nullptr);
			AI->ResetSpellCounters();

			uint32 sampleSize = 500;

			for (uint32 i = 0; i < sampleSize; i++)
			{
				uint32 result = caster->CastSpell(target, spellID, TRIGGERED_FULL_MASK);
				ASSERT_INFO("Spell casting failed with reason %u", result);
				TEST_ASSERT(result == SPELL_CAST_OK);
				Aura* aura = target->GetAura(spellID);
				TEST_ASSERT(aura != nullptr);
				TEST_ASSERT(aura->GetDuration() == 12 * SECOND * IN_MILLISECONDS);
				Wait(1000);
				auto damageToTarget = AI->GetSpellDamageDoneInfo(target);

				if (damageToTarget->empty())
					continue;

				uint64 totalDamage = 0;
				for (auto itr : *damageToTarget)
				{
					if (itr.spellID != spellID)
						continue;

					//use only spells that hit target
					if (itr.missInfo != SPELL_MISS_NONE)
						continue;

					if (itr.crit)
						continue; //ignore crit... damage crits are affected by a whole lot of other factors so best just using regulars hit

					uint32 damage = itr.damageInfo.damage;
					damage += itr.damageInfo.resist;
					damage += itr.damageInfo.blocked;
					damage += itr.damageInfo.absorb;
					//resilience not taken into account...

					totalDamage += damage;
				}
                TC_LOG_TRACE("test.unit_test", "damage:" UI64FMTD, totalDamage);
				TEST_ASSERT(totalDamage != 0);
				break;
			}
		}

		void TestMangle(TestPlayer* druidWithMangle, TestPlayer* druidTestDamage, Unit* creature, uint32 mangleSpellId)
		{
			Wait(5000);
			// Calcul base
			float const AP = druidTestDamage->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const weaponMinDamage = 136 + (AP / 14);
			float const weaponMaxDamage = 293 + (AP / 14);
			float const mangleFactor = 1.3;

			// Initialize Mangle (Cat) on dummy
			TestA(druidWithMangle, creature, mangleSpellId);
			//uint32 comboTarget = druidWithMangle->GetComboTarget();
			TEST_ASSERT(druidWithMangle->GetComboTarget() == creature->GetGUID());
			TEST_ASSERT(druidWithMangle->GetComboPoints() == 1);

			// Shred
			float const expectedShredMinDmg = (weaponMinDamage * 2.25f + 405) * armorFactor * mangleFactor;
			float const expectedShredMaxDmg = (weaponMaxDamage * 2.25f + 405) * armorFactor * mangleFactor;
			TEST_DIRECT_SPELL_DAMAGE(druidTestDamage, creature, ClassSpells::Druid::SHRED_RNK_7, expectedShredMinDmg, expectedShredMaxDmg, false);

			// Mangle (Cat)
			float const expectedMangleMinDmg = weaponMinDamage * 1.6f * armorFactor + 264;
			float const expectedMangleMaxDmg = weaponMaxDamage * 1.6f * armorFactor + 264;
			TEST_DIRECT_SPELL_DAMAGE(druidTestDamage, creature, mangleSpellId, expectedMangleMinDmg, expectedMangleMaxDmg, false);


			// Bleed: Rake
			//	Initial damage
            TEST_CAST(druidWithMangle, creature, mangleSpellId);
			float const expectedRakeDmg = (AP / 100 + 78) * mangleFactor;
			TEST_DIRECT_SPELL_DAMAGE(druidTestDamage, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeDmg, expectedRakeDmg, false);

			//	Rake bleed
            TEST_CAST(druidWithMangle, creature, mangleSpellId);
			float const expectedRakeBleed = (AP * 0.06f + 108) * mangleFactor;
            TEST_CAST(druidTestDamage, creature, ClassSpells::Druid::RAKE_RNK_5);
			Wait(500);
            TEST_HAS_AURA(creature, ClassSpells::Druid::RAKE_RNK_5);
			uint32 const startHealth = creature->GetHealth();
			Wait(9500);
			TEST_ASSERT(creature->GetHealth() == startHealth - expectedRakeBleed);
			Wait(3000);
		}

		void Test() override
		{
			// Initialize 2 druids, same stats
			TestPlayer* druidWithMangle = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* druidTestDamage = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition, 17); // 0 armor

			EQUIP_NEW_ITEM(druidWithMangle, 30883); // Pillar of Ferocity - 136-293 damage
			EQUIP_NEW_ITEM(druidTestDamage, 30883); // Pillar of Ferocity - 136-293 damage

			LearnTalent(druidWithMangle, Talents::Druid::MANGLE_RNK_1);
			TEST_ASSERT(druidWithMangle->HasSpell(ClassSpells::Druid::MANGLE_BEAR_RNK_1));
			TEST_ASSERT(druidWithMangle->HasSpell(ClassSpells::Druid::MANGLE_CAT_RNK_1));
			TEST_ASSERT(druidWithMangle->GetTotalAttackPowerValue(BASE_ATTACK) == druidTestDamage->GetTotalAttackPowerValue(BASE_ATTACK));

			// Morph both druids in cat form
			MorphTo(druidWithMangle, ClassSpells::Druid::CAT_FORM_RNK_1);
			MorphTo(druidTestDamage, ClassSpells::Druid::CAT_FORM_RNK_1);
			druidWithMangle->SetPower(POWER_ENERGY, druidWithMangle->GetMaxPower(POWER_ENERGY));
			druidTestDamage->SetPower(POWER_ENERGY, druidTestDamage->GetMaxPower(POWER_ENERGY));
			Wait(2000);
			TestMangle(druidWithMangle, druidTestDamage, creature, ClassSpells::Druid::MANGLE_CAT_RNK_3);

			druidWithMangle->RemoveAurasDueToSpell(ClassSpells::Druid::CAT_FORM_RNK_1);
			MorphTo(druidWithMangle, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2);
			druidWithMangle->SetPower(POWER_RAGE, druidWithMangle->GetMaxPower(POWER_RAGE));
			Wait(2000);
			TestMangle(druidWithMangle, druidTestDamage, creature, ClassSpells::Druid::MANGLE_BEAR_RNK_3);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MangleTestImpt>();
	}
};

class ImprovedMarkOfTheWildTest : public TestCaseScript
{
public:
	ImprovedMarkOfTheWildTest() : TestCaseScript("talents druid improved_mark_of_the_wild") { }

	class ImprovedMarkOfTheWildTestImpt : public TestCase
	{
	public:
		ImprovedMarkOfTheWildTestImpt() : TestCase(STATUS_PASSING) { }

		void TestSpellOfTheWild(TestPlayer* player, uint32 spell, TestPlayer* victim)
		{
			LearnTalent(player, Talents::Druid::IMPROVED_MARK_OF_THE_WILD_RNK_5);

			uint32 const startArmor = victim->GetArmor();
			uint32 const startAgi = victim->GetStat(STAT_AGILITY);
			uint32 const startInt = victim->GetStat(STAT_INTELLECT);
			uint32 const startSpi = victim->GetStat(STAT_SPIRIT);
			uint32 const startSta = victim->GetStat(STAT_STAMINA);
			uint32 const startStr = victim->GetStat(STAT_STRENGTH);
			uint32 const startResArcane = victim->GetResistance(SPELL_SCHOOL_ARCANE);
			uint32 const startResFire = victim->GetResistance(SPELL_SCHOOL_FIRE);
			uint32 const startResFrost = victim->GetResistance(SPELL_SCHOOL_FROST);
			uint32 const startResNature = victim->GetResistance(SPELL_SCHOOL_NATURE);
			uint32 const startResShadow = victim->GetResistance(SPELL_SCHOOL_SHADOW);

            uint32 const expectedArmor = startArmor + 340 * 1.35f + 14 * 1.35f * 2; //also add armor related to agility increase
			uint32 const expectedAgi = startAgi + 14 * 1.35f;
			uint32 const expectedInt = startInt + 14 * 1.35f;
			uint32 const expectedSpi = startSpi + 14 * 1.35f;
			uint32 const expectedSta = startSta + 14 * 1.35f;
			uint32 const expectedStr = startStr + 14 * 1.35f;
			uint32 const expectedResArcane = startResArcane + 25 * 1.35f;
			uint32 const expectedResFire = startResFire + 25 * 1.35f;
			uint32 const expectedResFrost = startResFrost + 25 * 1.35f;
			uint32 const expectedResNature = startResNature + 25 * 1.35f;
			uint32 const expectedResShadow = startResShadow + 25 * 1.35f;

            TEST_CAST(player, victim, spell);
            TEST_HAS_AURA(victim, spell);

			TEST_ASSERT(Between<uint32>(victim->GetArmor(), expectedArmor - 1, expectedArmor + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_AGILITY), expectedAgi - 1, expectedAgi + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_INTELLECT), expectedInt - 1, expectedInt + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_SPIRIT), expectedSpi - 1, expectedSpi + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_STAMINA), expectedSta - 1, expectedSta + 1));
			TEST_ASSERT(Between<uint32>(victim->GetStat(STAT_STRENGTH), expectedStr - 1, expectedStr + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_ARCANE), expectedResArcane - 1, expectedResArcane + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_FIRE), expectedResFire - 1, expectedResFire + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_FROST), expectedResFrost - 1, expectedResFrost + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_NATURE), expectedResNature - 1, expectedResNature + 1));
			TEST_ASSERT(Between<uint32>(victim->GetResistance(SPELL_SCHOOL_SHADOW), expectedResShadow - 1, expectedResShadow + 1));
			Wait(2000);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			TestSpellOfTheWild(player, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8, player);

			TestPlayer* player2 = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* target = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
			player2->AddItem(22148, 1); // Wild Quillvine, regeant for Git of the Wild
			TestSpellOfTheWild(player2, ClassSpells::Druid::GIFT_OF_THE_WILD_RNK_3, target);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedMarkOfTheWildTestImpt>();
	}
};

class FurorTest : public TestCaseScript
{
public:
	FurorTest() : TestCaseScript("talents druid furor") { }

	class FurorTestImpt : public TestCase
	{
	public:
		FurorTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(player, Talents::Druid::FUROR_RNK_5);
            TEST_CAST(player, player, ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_HAS_AURA(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			TEST_ASSERT(player->GetPower(POWER_RAGE) == 100); // = 10 rage
            player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1); //we need to dispell it manually, clients send CMSG_CANCEL_AURA when switching form
            Wait(2000); //Wait GCD
            TEST_CAST(player, player, ClassSpells::Druid::CAT_FORM_RNK_1);
            TEST_HAS_AURA(player, ClassSpells::Druid::CAT_FORM_RNK_1);
			TEST_ASSERT(player->GetPower(POWER_ENERGY) == 40);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FurorTestImpt>();
	}
};

class NaturalistTest : public TestCaseScript
{
public:
	NaturalistTest() : TestCaseScript("talents druid naturalist") { }

	class NaturalistTestImpt : public TestCase
	{
	public:
		NaturalistTestImpt() : TestCase(STATUS_PASSING_INCOMPLETE) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(player, Talents::Druid::NATURALIST_RNK_5);

			uint32 res = player->CastSpell(player, ClassSpells::Druid::HEALING_TOUCH_RNK_13);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Spell* spell = player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
			TEST_ASSERT(spell != nullptr);
			ASSERT_INFO("Spell %u has a cast time > 3s", spell->GetSpellInfo()->Id);
			TEST_ASSERT(spell->GetCastTime() == 3 * SECOND * IN_MILLISECONDS);

			// TODO: increases physical damage dealt in all forms by 10%
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<NaturalistTestImpt>();
	}
};

class NaturalShapeshifterTest : public TestCaseScript
{
public:
	NaturalShapeshifterTest() : TestCaseScript("talents druid natural_shapeshifter") { }

	class NaturalShapeshifterTestImpt : public TestCase
	{
	public:
		NaturalShapeshifterTestImpt() : TestCase(STATUS_PASSING) { }

		void TestMana(TestPlayer* player, uint32 spellId, uint32 expectedMana)
		{
			player->SetPower(POWER_MANA, expectedMana);
            TEST_CAST(player, player, spellId);
            TEST_HAS_AURA(player, spellId);
			ASSERT_INFO("Druid had some remaining mana after %u", spellId);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 0);
			player->RemoveAurasDueToSpell(spellId); // we need to dispell it manually, clients send CMSG_CANCEL_AURA when switching form
			Wait(2000); // GCD
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			player->DisableRegeneration(true);

			Wait(5000);
			LearnTalent(player, Talents::Druid::NATURAL_SHAPESHIFTER_RNK_3);

			uint32 const expectedBearMana	= 829 * 0.7f;
			uint32 const expectedCatMana	= 829 * 0.7f;
			uint32 const expectedTravelMana = 308 * 0.7f;
			uint32 const expectedAquaMana	= 308 * 0.7f;
			uint32 const expectedMoonMana	= 521 * 0.7f;
			uint32 const expectedTreeMana	= 663 * 0.7f;

			TestMana(player, ClassSpells::Druid::BEAR_FORM_RNK_1, expectedBearMana);
			TestMana(player, ClassSpells::Druid::CAT_FORM_RNK_1, expectedCatMana);
			TestMana(player, ClassSpells::Druid::TRAVEL_FORM_RNK_1, expectedTravelMana);
			//TestMana(player, ClassSpells::Druid::AQUATIC_FORM_RNK_1, expectedAquaMana);
			TestMana(player, ClassSpells::Druid::MOONKIN_FORM_RNK_1, expectedMoonMana);
			TestMana(player, ClassSpells::Druid::TREE_OF_LIFE_RNK_1, expectedTreeMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<NaturalShapeshifterTestImpt>();
	}
};

class IntensityTest : public TestCaseScript
{
public:
	IntensityTest() : TestCaseScript("talents druid intensity") { }

	class IntensityTestImpt : public TestCase
	{
	public:
		IntensityTestImpt() : TestCase(STATUS_PASSING) { }

		void TestRegen(TestPlayer* player, uint32 expectedMana)
		{
			uint32 currentMana = player->GetPower(POWER_MANA);
			ASSERT_INFO("Druid has %u mana, expected: %u", currentMana, expectedMana);
			TEST_ASSERT(currentMana == expectedMana);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(player, Talents::Druid::INTENSITY_RNK_3);

			// 30% mana regen while casting
			uint32 const regenTick = sqrt(player->GetStat(STAT_INTELLECT)) * player->OCTRegenMPPerSpirit() * 2;
			uint32 const expectedRegenTick = regenTick * 0.30f;

			player->SetMaxPower(POWER_MANA, 675 + 415); // Regrowth + Rejuvenation
			uint32 res = player->CastSpell(player, ClassSpells::Druid::REGROWTH_RNK_10);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2500);
			res = player->CastSpell(player, ClassSpells::Druid::REJUVENATION_RNK_13);
			TEST_ASSERT(res == SPELL_CAST_OK);
			uint32 expectedMana = expectedRegenTick;
			TestRegen(player, expectedMana);

			Wait(2000);
			expectedMana = 2 * expectedRegenTick;
			TestRegen(player, expectedMana);

			Wait(2000);
			expectedMana = 3 * expectedRegenTick;
			TestRegen(player, expectedMana);

			Wait(2000);
			expectedMana = 3 * expectedRegenTick + regenTick; // Out of FSR
			TestRegen(player, expectedMana);

			// 10 rage when using Enrage
			player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
			res = player->CastSpell(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", ClassSpells::Druid::BEAR_FORM_RNK_1, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000);
			res = player->CastSpell(player, ClassSpells::Druid::ENRAGE_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(500);
			ASSERT_INFO("Druid doesnt have 10 rage but %u", player->GetPower(POWER_RAGE));
			TEST_ASSERT(player->GetPower(POWER_RAGE) / 10 == 10);
			player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<IntensityTestImpt>();
	}
};

class TranquilSpiritTest : public TestCaseScript
{
public:
	TranquilSpiritTest() : TestCaseScript("talents druid tranquil_spirit") { }

	class TranquilSpiritTestImpt : public TestCase
	{
	public:
		TranquilSpiritTestImpt() : TestCase(STATUS_PASSING) { }

		void TestMana(TestPlayer* player, uint32 spellId, uint32 expectedMana, uint32 castingTime)
		{
			player->SetPower(POWER_MANA, expectedMana);
			uint32 res = player->CastSpell(player, spellId);
			ASSERT_INFO("Druid couldnt cast %u, result: %u", spellId, res);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(castingTime + 100);
			ASSERT_INFO("Druid had some remaining mana after %u", spellId);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 0);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::TRANQUIL_SPIRIT_RNK_5);

			uint32 const expectedHealingTouchMana = 935 * 0.9f;
			uint32 const expectedTranquilityMana = 1650 * 0.9f;

			TestMana(player, ClassSpells::Druid::HEALING_TOUCH_RNK_13, expectedHealingTouchMana, 3500);
			TestMana(player, ClassSpells::Druid::TRANQUILITY_RNK_5, expectedTranquilityMana, 8000);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<TranquilSpiritTestImpt>();
	}
};

class ImprovedRejuvenationTest : public TestCaseScript
{
public:
	ImprovedRejuvenationTest() : TestCaseScript("talents druid improved_rejuvenation") { }

	class ImprovedRejuvenationTestImpt : public TestCase
	{
	public:
		ImprovedRejuvenationTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			player->DisableRegeneration(true);
			player->SetHealth(1);

			LearnTalent(player, Talents::Druid::IMPROVED_REJUVENATION_RNK_3);

			uint32 const rejuvenationTick = floor(ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TOTAL * 1.15f / 4);
			uint32 const expectedHealth = 1 + 4 * rejuvenationTick;

			uint32 res = player->CastSpell(player, ClassSpells::Druid::REJUVENATION_RNK_13);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(13000);
			ASSERT_INFO("Druid doesnt have health %u but %u", expectedHealth, player->GetHealth());
			TEST_ASSERT(player->GetHealth() == expectedHealth);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedRejuvenationTestImpt>();
	}
};

class GiftOfNatureTest : public TestCaseScript
{
public:
	GiftOfNatureTest() : TestCaseScript("talents druid gift_of_nature") { }

	class GiftOfNatureTestImpt : public TestCase
	{
	public:
		GiftOfNatureTestImpt() : TestCase(STATUS_PASSING) { }

		void TestHeal(TestPlayer* player, uint32 spellId, uint32 HotTime, uint32 expectedHealth, uint32 crit = 0)
		{
			player->SetHealth(1);
			uint32 res = player->CastSpell(player, spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(HotTime + 1000);
			if (crit > 0)
			{
                TC_LOG_TRACE("test.unit_test", "health: %u, expected: %u", player->GetHealth(), expectedHealth);
				if (player->GetHealth() == expectedHealth)
				{
					ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, expectedHealth, player->GetHealth());
					TEST_ASSERT(player->GetHealth() == expectedHealth);
				}
				else
				{
					ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, crit, player->GetHealth());
					TEST_ASSERT(player->GetHealth() == crit);
				}
			}
			else
			{
				ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, expectedHealth, player->GetHealth());
				TEST_ASSERT(player->GetHealth() == expectedHealth);
			}
		}

		void TestRegrowth(TestPlayer* player, uint32 spellId, uint32 HotTime, uint32 expectedHealth)
		{
			player->SetHealth(1);
			uint32 res = player->CastSpell(player, spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2500);
			uint32 regrowthStart = player->GetHealth();
			Wait(HotTime );
			ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, regrowthStart + expectedHealth, player->GetHealth());
			TEST_ASSERT(player->GetHealth() == regrowthStart + expectedHealth);
		}

		void TestSwiftmend(TestPlayer* player, uint32 spellId, uint32 expectedHealth)
		{
			player->SetHealth(100);
			uint32 res = player->CastSpell(player, spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000);
			uint32 startHealth = player->GetHealth();
			res = player->CastSpell(player, ClassSpells::Druid::SWIFTMEND_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			uint32 currentHealth = player->GetHealth();
			ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, startHealth + expectedHealth, currentHealth);
			TEST_ASSERT(currentHealth == startHealth + expectedHealth);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::GIFT_OF_NATURE_RNK_5);

			float const giftOfNatureFactor = 1 + 5 * 0.02f;

			// Swiftmend
			uint32 const rejuvenationTotal = 4 * floor(ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TOTAL * giftOfNatureFactor / 4);
			uint32 const regrowthTotal = 6 * floor(ClassSpellsDamage::Druid::REGROWTH_RNK_10_TOTAL * giftOfNatureFactor / 7);
			TestSwiftmend(player, ClassSpells::Druid::REJUVENATION_RNK_13, rejuvenationTotal);
			TestSwiftmend(player, ClassSpells::Druid::REGROWTH_RNK_10, regrowthTotal);

			// Rejuvenation
			uint32 const rejuvenationTick = floor(ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TOTAL * giftOfNatureFactor / 4);
			uint32 const expectedRejuvenationHealth = 1 + 4 * rejuvenationTick;
			TestHeal(player, ClassSpells::Druid::REJUVENATION_RNK_13, 12000, expectedRejuvenationHealth);

			// Lifebloom
			uint32 const lifebloomTick = floor(ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_TOTAL * giftOfNatureFactor / 7);
			uint32 const lifebloomBurst = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST * giftOfNatureFactor;
			uint32 const lifebloomBurstCrit = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST * giftOfNatureFactor * 1.5f;
			uint32 const expectedLifebloomHealth	 = 1 + 7 * lifebloomTick + lifebloomBurst;
			uint32 const expectedLifebloomHealthCrit = 1 + 7 * lifebloomTick + lifebloomBurstCrit;
			TestHeal(player, ClassSpells::Druid::LIFEBLOOM_RNK_1, 7000, expectedLifebloomHealth, expectedLifebloomHealthCrit);

			// Regrowth HoTs
			uint32 const regrowthTick = floor(ClassSpellsDamage::Druid::REGROWTH_RNK_10_TOTAL * giftOfNatureFactor / 7);
			uint32 const expectedRegrowthHealth = 7 * regrowthTick;
			TestRegrowth(player, ClassSpells::Druid::REGROWTH_RNK_10, 21000, expectedRegrowthHealth);
			// Regrowth Direct
			uint32 const regrowthMin = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MIN * giftOfNatureFactor;
			uint32 const regrowthMax = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MAX * giftOfNatureFactor;
			//TEST_DIRECT_HEAL(player, player, ClassSpells::Druid::REGROWTH_RNK_10, regrowthMin, regrowthMax);

			// Healing Touch
			uint32 const healingTouchMin = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MIN * giftOfNatureFactor;
			uint32 const healingTouchMax = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MAX * giftOfNatureFactor;
			//TEST_DIRECT_HEAL(player, player, ClassSpells::Druid::HEALING_TOUCH_RNK_13, healingTouchMin, healingTouchMax);

		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<GiftOfNatureTestImpt>();
	}
};

class EmpoweredTouchTest : public TestCaseScript
{
public:
	EmpoweredTouchTest() : TestCaseScript("talents druid empowered_touch") { }

	class EmpoweredTouchTestImpt : public TestCase
	{
	public:
		EmpoweredTouchTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			player->DisableRegeneration(true);

			LearnTalent(player, Talents::Druid::EMPOWERED_TOUCH_RNK_2);

			uint32 const bh = player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL);
			uint32 const healingTouchCoeff = ClassSpellsCoeff::Druid::HEALING_TOUCH + 20;

			uint32 const healingTouchMin = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MIN + bh * healingTouchCoeff;
			uint32 const healingTouchMax = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MAX + bh * healingTouchCoeff;
			//TEST_DIRECT_HEAL(player, player, ClassSpells::Druid::HEALING_TOUCH_RNK_13, healingTouchMin, healingTouchMax);

		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EmpoweredTouchTestImpt>();
	}
};

class LivingSpiritTest : public TestCaseScript
{
public:
	LivingSpiritTest() : TestCaseScript("talents druid living_spirit") { }

	class LivingSpiritTestImpt : public TestCase
	{
	public:
		LivingSpiritTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			uint32 const startSpi = player->GetStat(STAT_SPIRIT);
			uint32 const expectedSpi = startSpi * 1.15f;

			LearnTalent(player, Talents::Druid::LIVING_SPIRIT_RNK_3);
			TEST_ASSERT(Between<float>(player->GetStat(STAT_SPIRIT), expectedSpi - 1, expectedSpi + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<LivingSpiritTestImpt>();
	}
};

class EmpoweredRejuvenationTest : public TestCaseScript
{
public:
	EmpoweredRejuvenationTest() : TestCaseScript("talents druid empowered_rejuvenation") { }

	class EmpoweredRejuvenationTestImpt : public TestCase
	{
	public:
		EmpoweredRejuvenationTestImpt() : TestCase(STATUS_PASSING) { }

		void TestHeal(TestPlayer* player, uint32 spellId, uint32 HotTime, uint32 expectedHealth, uint32 crit = 0)
		{
			player->SetHealth(1);
			player->SetMaxHealth(10000);
			uint32 startHealth = player->GetHealth();
            TEST_CAST(player, player, spellId);
			Wait(HotTime + 1000);
			uint32 currentHealth = player->GetHealth();
            TC_LOG_TRACE("test.unit_test", "health: %u, expected: %u", currentHealth, startHealth + expectedHealth);
			if (crit > 0)
			{
				if (player->GetHealth() == expectedHealth)
				{
					ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, startHealth + expectedHealth, currentHealth);
					TEST_ASSERT(currentHealth == startHealth + expectedHealth);
				}
				else
				{
					ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, crit, currentHealth);
					TEST_ASSERT(currentHealth == crit);
				}
			}
			else
			{
				ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, startHealth + expectedHealth, currentHealth);
				TEST_ASSERT(currentHealth == startHealth + expectedHealth);
			}
		}

		void TestRegrowth(TestPlayer* player, uint32 spellId, uint32 HotTime, uint32 expectedHealth)
		{
			player->SetHealth(1);
			uint32 res = player->CastSpell(player, spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2500);
			uint32 regrowthStart = player->GetHealth();
			Wait(HotTime);
			ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, regrowthStart + expectedHealth, player->GetHealth());
			TEST_ASSERT(player->GetHealth() == regrowthStart + expectedHealth);
		}

		void TestSwiftmend(TestPlayer* player, uint32 spellId, uint32 expectedHealth)
		{
			player->GetSpellHistory()->ResetAllCooldowns();
			player->SetHealth(1);
			uint32 res = player->CastSpell(player, spellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(2000);
			uint32 startHealth = player->GetHealth();
			res = player->CastSpell(player, ClassSpells::Druid::SWIFTMEND_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			ASSERT_INFO("After spell %u, druid doesnt have health %u but %u", spellId, startHealth + expectedHealth, player->GetHealth());
			TEST_ASSERT(player->GetHealth() == startHealth + expectedHealth);
		}

		uint32 CalculateBH(float spellCoeff)
		{
			float const empoweredRejuvenationFactor = 1 + 5 * 0.04f;
			int32 const bh = 550;
			uint32 bonus = bh * spellCoeff * empoweredRejuvenationFactor;
			return bonus;
		}

		float CalculateTranquilityCoeff(int32 tranquilityCastTime)
		{
			// http://wowwiki.wikia.com/wiki/Spell_power?oldid=1576621
			// (castTime / 3.5) / 2 / nbTick;
			return tranquilityCastTime / 3.5f / 2.0f / 4.0f;
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			player->DisableRegeneration(true);

			EQUIP_NEW_ITEM(player, 34335); // Hammer of Sanctification - 550 SP

			LearnTalent(player, Talents::Druid::EMPOWERED_REJUVENATION_RNK_5);

			// Lifebloom
			uint32 const lifebloomTick = floor(ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_TOTAL + CalculateBH(ClassSpellsCoeff::Druid::LIFEBLOOM_HOT) / 7);
			uint32 const lifebloomBurst = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST + CalculateBH(ClassSpellsCoeff::Druid::LIFEBLOOM);
			uint32 const lifebloomBurstCrit = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST + CalculateBH(ClassSpellsCoeff::Druid::LIFEBLOOM) * 1.5f;
			uint32 const expectedLifebloomHealth = 1 + 7 * lifebloomTick + lifebloomBurst;
			uint32 const expectedLifebloomHealthCrit = 1 + 7 * lifebloomTick + lifebloomBurstCrit;
			TC_LOG_TRACE("test.unit_test", "Lifebloom");
			TestHeal(player, ClassSpells::Druid::LIFEBLOOM_RNK_1, 7000, expectedLifebloomHealth, expectedLifebloomHealthCrit);

			// Tranquility
			uint32 const tranquilityTickCoeff = CalculateTranquilityCoeff(8);
			uint32 const tranquilityBH = CalculateBH(tranquilityTickCoeff);
			uint32 const tranquilityTick = ClassSpellsDamage::Druid::TRANQUILITY_RNK_5_TICK + tranquilityBH;
			uint32 const expectedTranquilityHealth = 4 * tranquilityTick;
			TestHeal(player, ClassSpells::Druid::TRANQUILITY_RNK_5, 8000, expectedTranquilityHealth);

			// Regrowth HoT
			uint32 const regrowthTick = floor(ClassSpellsDamage::Druid::REGROWTH_RNK_10_TOTAL + CalculateBH(ClassSpellsCoeff::Druid::REGROWTH_HOT) / 7);
			uint32 const expectedRegrowthHealth = 7 * regrowthTick;
            TC_LOG_TRACE("test.unit_test", "Regrowth HoT");
			TestRegrowth(player, ClassSpells::Druid::REGROWTH_RNK_10, 21000, expectedRegrowthHealth);
			// Regrowth Direct
			uint32 const regrowthMin = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MIN + CalculateBH(ClassSpellsCoeff::Druid::REGROWTH);
			uint32 const regrowthMax = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MAX + CalculateBH(ClassSpellsCoeff::Druid::REGROWTH);
			//TEST_DIRECT_HEAL(player, player, ClassSpells::Druid::REGROWTH_RNK_10, regrowthMin, regrowthMax);

			// Swiftmend
			uint32 const rejuvenationTotal = 4 * floor(ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TOTAL + CalculateBH(ClassSpellsCoeff::Druid::REJUVENATION) / 4);
			uint32 const regrowthTotal = 6 * floor(ClassSpellsDamage::Druid::REGROWTH_RNK_10_TOTAL + CalculateBH(ClassSpellsCoeff::Druid::REGROWTH_HOT) / 7);
            TC_LOG_TRACE("test.unit_test", "Rejuvenation Swift");
			TestSwiftmend(player, ClassSpells::Druid::REJUVENATION_RNK_13, rejuvenationTotal);
            TC_LOG_TRACE("test.unit_test", "Regrowth Swift");
			TestSwiftmend(player, ClassSpells::Druid::REGROWTH_RNK_10, regrowthTotal);

			// Rejuvenation
			uint32 const rejuvenationBH = CalculateBH(ClassSpellsCoeff::Druid::REJUVENATION);
			uint32 const rejuvenationTick = floor((ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TOTAL + rejuvenationBH) / 4);
			uint32 const expectedRejuvenationHealth = 4 * rejuvenationTick;
			TestHeal(player, ClassSpells::Druid::REJUVENATION_RNK_13, 12000, expectedRejuvenationHealth);

		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EmpoweredRejuvenationTestImpt>();
	}
};

void AddSC_test_talents_druid()
{
	// Total: 30/62
	// Balance: 9/21
	new StarlightWrathTest();
	new ImprovedNaturesGraspTest();
	new InsectSwarmTest();
	new NaturesReachTest();
	new LunarGuidanceTest();
	new MoonglowTest();
	new MoonfuryTest();
	new DreamstateTest();
	new WrathOfCenariusTest();
	// Feral: 10/21
	new FerocityTest();
	new BrutalImpactTest();
	new ThickHideTest();
	new ShreddingAttacksTest();
	new PredatoryStrikesTest();
	new SavageFuryTest();
	new FaerieFireFeralTest();
	new HeartOfTheWildTest();
	new SurvivalOfTheFittestTest();
	new MangleTest();
	// Restoration: 11/20
	new ImprovedMarkOfTheWildTest();
	new FurorTest();
	new NaturalistTest();
	new NaturalShapeshifterTest();
	new IntensityTest();
	new TranquilSpiritTest();
	new ImprovedRejuvenationTest();
	new GiftOfNatureTest();
	new EmpoweredTouchTest();
	new LivingSpiritTest();
	new EmpoweredRejuvenationTest();
}
