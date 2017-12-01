#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

class StarlightWrathTest : public TestCaseScript
{
public:

	StarlightWrathTest() : TestCaseScript("talents druid starlight_wrath") { }

	class StarlightWrathTestImpt : public TestCase
	{
	public:
		StarlightWrathTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummy = SpawnCreature();

			LearnTalent(player, Talents::Druid::STARLIGHT_WRATH_RNK_5);

			uint32 res = player->CastSpell(dummy, ClassSpells::Druid::WRATH_RNK_10);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(1750);
			res = player->CastSpell(dummy, ClassSpells::Druid::WRATH_RNK_10);
			ASSERT_INFO("Not able to cast after 1.75s");
			TEST_ASSERT(res == SPELL_CAST_OK);
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
		ImprovedNaturesGraspTestImpt() : TestCase(true) { }

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
			TEST_ASSERT(player->HasAura(ClassSpells::Druid::NATURES_GRASP_RNK_7));
			warrior->Attack(player, true);
			Wait(1500);
			ASSERT_INFO("Druid still has aura");
			TEST_ASSERT(!player->HasAura(ClassSpells::Druid::NATURES_GRASP_RNK_7));
			ASSERT_INFO("Warrior isnt root");
			TEST_ASSERT(warrior->HasAura(ClassSpells::Druid::NATURES_GRASP_RNK_7_PROC));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedNaturesGraspTestImpt>();
	}
};

class NaturesReachTest : public TestCaseScript
{
public:

	NaturesReachTest() : TestCaseScript("talents druid natures_reach") { }

	class NaturesReachTestImpt : public TestCase
	{
	public:
		NaturesReachTestImpt() : TestCase(true) { }

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
		LunarGuidanceTestImpt() : TestCase(true) { }

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
		MoonglowTestImpt() : TestCase(true) { }

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
		MoonfuryTestImpt() : TestCase(true) { }

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
			//Test regular damage

			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::STARFIRE_RNK_8, starfireMinDamage, starfireMaxDamage);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::MOONFIRE_RNK_12, moonfireMinDamage, moonfireMaxDamage);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::WRATH_RNK_10, wrathMinDamage, wrathMaxDamage);

			//Test improved damage 5%
			LearnTalent(player, Talents::Druid::MOONFURY_RNK_5);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::STARFIRE_RNK_8, starfireMinDamage * 1.1f, starfireMaxDamage * 1.1f);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::MOONFIRE_RNK_12, moonfireMinDamage * 1.1f, moonfireMaxDamage * 1.1f);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::WRATH_RNK_10, wrathMinDamage * 1.1f, wrathMaxDamage * 1.1f);
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
		DreamstateTestImpt() : TestCase(true) { }

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
		WrathOfCenariusTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummyTarget = SpawnCreature();

			LearnTalent(player, Talents::Druid::WRATH_OF_CENARIUS_RNK_5);

			EquipItem(player, 34182); // Grand Magister's Staff of Torrents - 266 SP

			float const starfireFactor	= 5 * 0.04f;
			float const wrathFactor		= 5 * 0.02f;

			float const arcaneSP = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ARCANE);
			float const natureSP = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE);

			uint32 const expectedStarfireMinDamage = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN + arcaneSP * (ClassSpellsCoeff::Druid::STARFIRE + starfireFactor);
			uint32 const expectedStarfireMaxDamage = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX + arcaneSP * (ClassSpellsCoeff::Druid::STARFIRE + starfireFactor);

			uint32 const expectedWrathMinDamage = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN + natureSP * (ClassSpellsCoeff::Druid::WRATH + wrathFactor);
			uint32 const expectedWrathMaxDamage = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX + natureSP * (ClassSpellsCoeff::Druid::WRATH + wrathFactor);

			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDamage, expectedStarfireMaxDamage);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDamage, expectedWrathMaxDamage);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<WrathOfCenariusTestImpt>();
	}
};

class ImprovedMarkOfTheWildTest : public TestCaseScript
{
public:
	ImprovedMarkOfTheWildTest() : TestCaseScript("talents druid improved_mark_of_the_wild") { }

	class ImprovedMarkOfTheWildTestImpt : public TestCase
	{
	public:
		ImprovedMarkOfTheWildTestImpt() : TestCase(true) { }

		void TestSpellOfTheWild(TestPlayer* player, uint32 spell, TestPlayer* victim)
		{
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

			LearnTalent(player, Talents::Druid::IMPROVED_MARK_OF_THE_WILD_RNK_5);
			uint32 result = player->CastSpell(victim, spell);
            TEST_ASSERT(result == SPELL_CAST_OK);

			TEST_ASSERT(Between<float>(victim->GetArmor(), expectedArmor - 1, expectedArmor + 1));
			TEST_ASSERT(Between<float>(victim->GetStat(STAT_AGILITY), expectedAgi - 1, expectedAgi + 1));
			TEST_ASSERT(Between<float>(victim->GetStat(STAT_INTELLECT), expectedInt - 1, expectedInt + 1));
			TEST_ASSERT(Between<float>(victim->GetStat(STAT_SPIRIT), expectedSpi - 1, expectedSpi + 1));
			TEST_ASSERT(Between<float>(victim->GetStat(STAT_STAMINA), expectedSta - 1, expectedSta + 1));
			TEST_ASSERT(Between<float>(victim->GetStat(STAT_STRENGTH), expectedStr - 1, expectedStr + 1));
			TEST_ASSERT(Between<float>(victim->GetResistance(SPELL_SCHOOL_ARCANE), expectedResArcane - 1, expectedResArcane + 1));
			TEST_ASSERT(Between<float>(victim->GetResistance(SPELL_SCHOOL_FIRE), expectedResFire - 1, expectedResFire + 1));
			TEST_ASSERT(Between<float>(victim->GetResistance(SPELL_SCHOOL_FROST), expectedResFrost - 1, expectedResFrost + 1));
			TEST_ASSERT(Between<float>(victim->GetResistance(SPELL_SCHOOL_NATURE), expectedResNature - 1, expectedResNature + 1));
			TEST_ASSERT(Between<float>(victim->GetResistance(SPELL_SCHOOL_SHADOW), expectedResShadow - 1, expectedResShadow + 1));
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);
            TestSpellOfTheWild(player, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8, player);

            TestPlayer* player2 = SpawnRandomPlayer(CLASS_DRUID);
            TestPlayer* target = SpawnRandomPlayer(CLASS_WARRIOR);
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
		FurorTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_DRUID);

			LearnTalent(player, Talents::Druid::FUROR_RNK_5);
            uint32 result = player->CastSpell(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_ASSERT(result == SPELL_CAST_OK);
            TEST_ASSERT(player->HasAura(ClassSpells::Druid::BEAR_FORM_RNK_1));
			TEST_ASSERT(player->GetPower(POWER_RAGE) == 100); // = 10 rage
            player->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1); //we need to dispell it manually, clients send CMSG_CANCEL_AURA when switching form
            Wait(2000); //Wait GCD
			result = player->CastSpell(player, ClassSpells::Druid::CAT_FORM_RNK_1);
            TEST_ASSERT(result == SPELL_CAST_OK);
            TEST_ASSERT(player->HasAura(ClassSpells::Druid::CAT_FORM_RNK_1));
			TEST_ASSERT(player->GetPower(POWER_ENERGY) == 40);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FurorTestImpt>();
	}
};

class LivingSpiritTest : public TestCaseScript
{
public:
	LivingSpiritTest() : TestCaseScript("talents druid living_spirit") { }

	class LivingSpiritTestImpt : public TestCase
	{
	public:
		LivingSpiritTestImpt() : TestCase(true) { }

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

void AddSC_test_talents_druid()
{
	// Total: 11/62
	// Balance: 8/21
	new StarlightWrathTest();
	new ImprovedNaturesGraspTest();
	new NaturesReachTest();
	new LunarGuidanceTest();
	new MoonglowTest();
	new MoonfuryTest();
	new DreamstateTest();
	new WrathOfCenariusTest();
	// Feral: 0/21
	// Restoration: 3/20
	new ImprovedMarkOfTheWildTest();
	new FurorTest();
	new LivingSpiritTest();
}
