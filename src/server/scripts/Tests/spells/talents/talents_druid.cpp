#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"
#include "ClassSpells.h"

class MoonfuryTest : public TestCaseScript
{
public:

	MoonfuryTest() : TestCaseScript("talents shaman Moonfury") { }

	class MoonfuryTestImpt : public TestCase
	{
	public:
		MoonfuryTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_SHAMAN);
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
			TestPlayer* player2 = SpawnRandomPlayer(CLASS_WARRIOR);

			TestSpellOfTheWild(player, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8, player);
			//TestSpellOfTheWild(player, ClassSpells::Druid::GIFT_OF_THE_WILD_RNK_3, player2);
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
			player->CastSpell(player, ClassSpells::Druid::BEAR_FORM_RNK_1);
			TEST_ASSERT(player->GetPower(POWER_RAGE) == 10);
			player->CastSpell(player, ClassSpells::Druid::CAT_FORM_RNK_1);
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
	new MoonfuryTest();
	new LunarGuidanceTest();
	new ImprovedMarkOfTheWildTest();
	new FurorTest();
	new LivingSpiritTest();
}
