#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"
#include "ClassSpells.h"

class WandSpecializationTest : public TestCaseScript
{
public:
	WandSpecializationTest() : TestCaseScript("talents mage wand_specialization") { }

	class WandSpecializationTestImpt : public TestCase
	{
	public:
		WandSpecializationTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);
			EquipItem(player, 28783); //Eredar Wand of Obliteration, 177 - 330 Shadow Damage
			uint32 const wandMinDamage = 177;
			uint32 const wandMaxDamage = 330;

			Creature* dummyTarget = SpawnCreature();
			//Test regular damage
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::WAND, wandMinDamage, wandMaxDamage);

			//Test improved damage 25%
			LearnTalent(player, Talents::Mage::WAND_SPECIALIZATION_RNK_2);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::WAND, wandMinDamage * 1.25f, wandMaxDamage * 1.25f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<WandSpecializationTestImpt>();
	}
};

class MagicAbsorptionTest : public TestCaseScript
{
public:
	MagicAbsorptionTest() : TestCaseScript("talents mage magic_absorption") { }

	class MagicAbsorptionTestImpt : public TestCase
	{
	public:
		MagicAbsorptionTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			uint32 const startResArcane = player->GetResistance(SPELL_SCHOOL_ARCANE);
			uint32 const startResFire = player->GetResistance(SPELL_SCHOOL_FIRE);
			uint32 const startResFrost = player->GetResistance(SPELL_SCHOOL_FROST);
			uint32 const startResNature = player->GetResistance(SPELL_SCHOOL_NATURE);
			uint32 const startResShadow = player->GetResistance(SPELL_SCHOOL_SHADOW);

			uint32 const expectedResArcane = startResArcane + 10;
			uint32 const expectedResFire = startResFire + 10;
			uint32 const expectedResFrost = startResFrost + 10;
			uint32 const expectedResNature = startResNature + 10;
			uint32 const expectedResShadow = startResShadow + 10;

			LearnTalent(player, Talents::Mage::MAGIC_ABSORPTION_RNK_5);
			TEST_ASSERT(player->GetResistance(SPELL_SCHOOL_ARCANE) == expectedResArcane);
			TEST_ASSERT(player->GetResistance(SPELL_SCHOOL_FIRE) == expectedResFire);
			TEST_ASSERT(player->GetResistance(SPELL_SCHOOL_FROST) == expectedResFrost);
			TEST_ASSERT(player->GetResistance(SPELL_SCHOOL_NATURE) == expectedResNature);
			TEST_ASSERT(player->GetResistance(SPELL_SCHOOL_SHADOW) == expectedResShadow);

			// TODO: test fully resisted spells to restore 5% of total mana
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MagicAbsorptionTestImpt>();
	}
};

class MagicAttunementTest : public TestCaseScript
{
public:
	MagicAttunementTest() : TestCaseScript("talents mage magic_attunement") { }

	class MagicAttunementTestImpt : public TestCase
	{
	public:
		MagicAttunementTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			TestPlayer* shaman = SpawnRandomPlayer(CLASS_SHAMAN);

			float const coeffHW = 85.71f;

			uint32 const minHW = 2134;
			uint32 const maxHW = 2436;

			uint32 const expectedMinHW = 2134 + 360 * coeffHW;
			uint32 const expectedMaxHW = 2436 + 360 * coeffHW;

			// Test regular
			//TestHeal(shaman, mage, ClassSpells::Shaman::HEALING_WAVE_RNK_12, minHW, maxHW);

			// Test improved
			mage->CastSpell(mage, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6);
			LearnTalent(mage, Talents::Mage::MAGIC_ATTUNEMENT_RNK_2);
			//TestHeal(shaman, mage, ClassSpells::Shaman::HEALING_WAVE_RNK_12, expectedMinHW, expectedMaxHW);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MagicAttunementTestImpt>();
	}
};

class ArcaneFortitudeTest : public TestCaseScript
{
public:
	ArcaneFortitudeTest() : TestCaseScript("talents mage arcane_fortitude") { }

	class ArcaneFortitudeTestImpt : public TestCase
	{
	public:
		ArcaneFortitudeTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			uint32 const startArmor = player->GetArmor();
			uint32 const expectedArmor = player->GetArmor() + player->GetStat(STAT_INTELLECT);

			LearnTalent(player, Talents::Mage::ARCANE_FORTITUDE_RNK_1);
			TEST_ASSERT(player->GetArmor() == expectedArmor);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ArcaneFortitudeTestImpt>();
	}
};

class ImprovedCounterspellTest : public TestCaseScript
{
public:
	ImprovedCounterspellTest() : TestCaseScript("talents mage improved_counterspell") { }

	class ImprovedCounterspellTestImpt : public TestCase
	{
	public:
		ImprovedCounterspellTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* mage = SpawnRandomPlayer(CLASS_MAGE);
			Creature* dummyTarget = SpawnCreature();

			LearnTalent(mage, Talents::Mage::IMPROVED_COUNTERSPELL_RNK_2);
			mage->CastSpell(dummyTarget, ClassSpells::Mage::COUNTERSPELL_RNK_1);
			Wait(3500);
			TEST_ASSERT(dummyTarget->HasAura(18469));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedCounterspellTestImpt>();
	}
};

class ArcaneMindTest : public TestCaseScript
{
public:
	ArcaneMindTest() : TestCaseScript("talents mage arcane_mind") { }

	class ArcaneMindTestImpt : public TestCase
	{
	public:
		ArcaneMindTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			uint32 const startInt = player->GetStat(STAT_INTELLECT);
			uint32 const expectedInt = startInt * 1.15f;
			LearnTalent(player, Talents::Mage::ARCANE_MIND_RNK_5);
			TEST_ASSERT(Between<float>(player->GetStat(STAT_INTELLECT), expectedInt - 1, expectedInt + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ArcaneMindTestImpt>();
	}
};

class ArcaneInstabilityTest : public TestCaseScript
{
public:
	ArcaneInstabilityTest() : TestCaseScript("talents mage arcane_instability") { }

	class ArcaneInstabilityTestImpt : public TestCase
	{
	public:
		ArcaneInstabilityTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);
			EquipItem(player, 34182); // Grand Magister's Staff of Torrents - 266 spell power

			float const startSP = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
			float const startSC = player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE);

			float const expectedSP = startSP * 1.03f;
			float const expectedSC = startSC + 3;

			LearnTalent(player, Talents::Mage::ARCANE_INSTABILITY_RNK_3);
			TEST_ASSERT(Between<float>(player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE), expectedSP - 1, expectedSP + 1));
			TEST_ASSERT(player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE) == expectedSC);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ArcaneInstabilityTestImpt>();
	}
};

class EmpoweredArcaneMissilesTest : public TestCaseScript
{
public:
	EmpoweredArcaneMissilesTest() : TestCaseScript("talents mage empowered_arcane_missiles") { }

	class EmpoweredArcaneMissilesTestImpt : public TestCase
	{
	public:
		EmpoweredArcaneMissilesTestImpt() : TestCase(true) { }

		void Test() override
		{
			Creature* dummyTarget = SpawnCreature();
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);
			//EquipItem(player, 34182); // Grand Magister's Staff of Torrents - 266 spell power

			float const startCoeff = 142.86;
			float const expectedCoeff = startCoeff + (3 * 15);

			uint32 const tickAM = 260;

			float const expectedTickAM = tickAM + player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE) * expectedCoeff;

			LearnTalent(player, Talents::Mage::EMPOWERED_ARCANE_MISSILES_RNK_3);
			//TestChannelDamage(player, dummyTarget, ClassSpells::Mage::ARCANE_MISSILES_RNK_10, expectedTickAM);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EmpoweredArcaneMissilesTestImpt>();
	}
};

class MindMasteryTest : public TestCaseScript
{
public:
	MindMasteryTest() : TestCaseScript("talents mage mind_mastery") { }

	class MindMasteryTestImpt : public TestCase
	{
	public:
		MindMasteryTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			uint32 const startSP = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE);
			uint32 const expectedSP = startSP + player->GetStat(STAT_INTELLECT) * 0.25f;

			LearnTalent(player, Talents::Mage::MIND_MASTERY_RNK_5);
			TEST_ASSERT(Between<float>(player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE), expectedSP - 1, expectedSP + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MindMasteryTestImpt>();
	}
};

class CriticalMassTest : public TestCaseScript
{
public:
	CriticalMassTest() : TestCaseScript("talents mage critical_mass") { }

	class CriticalMassTestImpt : public TestCase
	{
	public:
		CriticalMassTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			float const startSC = player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE);

			float const expectedSC = startSC + 6;

			LearnTalent(player, Talents::Mage::CRITICAL_MASS_RNK_3);
			TEST_ASSERT(player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE) == expectedSC);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<CriticalMassTestImpt>();
	}
};

class FirePowerTest : public TestCaseScript
{
public:

	FirePowerTest() : TestCaseScript("talents mage fire_power") { }

	class FirePowerTestImpt : public TestCase
	{
	public:
		FirePowerTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			// Fireblast rank 9
			uint32 const fireblastMinDamage = 664;
			uint32 const fireblastMaxDamage = 786;

			// Fireball rank 13
			uint32 const fireballMinDamage = 649;
			uint32 const fireballMaxDamage = 821;

			// Flamestrike rank 5
			uint32 const flamestrikeMinDamage = 480;
			uint32 const flamestrikeMaxDamage = 585;

			// Molten armor rank 1
			uint32 const moltenArmorDamage = 75;

			// Scorch rank 9
			uint32 const scorchMinDamage = 305;
			uint32 const scorchMaxDamage = 361;

			Creature* dummyTarget = SpawnCreature();
			//Test regular damage
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::FIRE_BLAST_RNK_9, fireblastMinDamage, fireblastMaxDamage);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage, fireballMaxDamage);
			//TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::FLAMESTRIKE_RNK_7, flamestrikeMinDamage, flamestrikeMaxDamage);
			//TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::MOLTEN_ARMOR_RNK_1, moltenArmorDamage);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::SCORCH_RNK_9, scorchMinDamage, scorchMaxDamage);

			//Test improved damage 10%
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::FIRE_BLAST_RNK_9, fireblastMinDamage * 1.1f, fireblastMaxDamage * 1.1f);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage * 1.1f, fireballMaxDamage * 1.1f);
			//TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::FLAMESTRIKE_RNK_7, flamestrikeMinDamage * 1.1f, flamestrikeMaxDamage * 1.1f);
			//TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::MOLTEN_ARMOR_RNK_1, moltenArmorDamage * 1.1f);
			TestDirectSpellDamage(player, dummyTarget, ClassSpells::Mage::SCORCH_RNK_9, scorchMinDamage * 1.1f, scorchMaxDamage * 1.1f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FirePowerTestImpt>();
	}
};

void AddSC_test_talents_mage()
{
	new WandSpecializationTest();
	new MagicAbsorptionTest();
	new MagicAttunementTest();
	new ArcaneFortitudeTest();
	new ImprovedCounterspellTest();
	new ArcaneMindTest();
	new ArcaneInstabilityTest();
	new EmpoweredArcaneMissilesTest();
	new MindMasteryTest();
	new CriticalMassTest();
	new FirePowerTest();
}
