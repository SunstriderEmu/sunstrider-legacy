#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

class MageWandSpecializationTest : public TestCaseScript
{
public:
    MageWandSpecializationTest() : TestCaseScript("talents mage wand_specialization") { }

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
			//TEST_DIRECT_HEAL(shaman, mage, ClassSpells::Shaman::HEALING_WAVE_RNK_12, minHW, maxHW);

			// Test improved
			uint32 result = mage->CastSpell(mage, ClassSpells::Mage::AMPLIFY_MAGIC_RNK_6);
            TEST_ASSERT(result == SPELL_CAST_OK);
			LearnTalent(mage, Talents::Mage::MAGIC_ATTUNEMENT_RNK_2);
			//TEST_DIRECT_HEAL(shaman, mage, ClassSpells::Shaman::HEALING_WAVE_RNK_12, expectedMinHW, expectedMaxHW);
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
			Wait(1000);
			TEST_ASSERT(dummyTarget->HasAura(18469)); //"Counterspell - Silenced", 4s
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
			float const expectedSC = startSC + 3.0f;

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
			EquipItem(player, 34182); // Grand Magister's Staff of Torrents - 266 spell power
            uint32 arcaneSpellPower = player->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_ARCANE);
			float const startCoeff = 1.429;
			float const expectedCoeff = startCoeff + (3 * 0.15);
            uint32 const tickCount = 5;
			uint32 const totalDamage = 260 * tickCount;
            uint32 const expectedTickDmg = (float(totalDamage) + arcaneSpellPower * expectedCoeff) / tickCount;

            Wait(5000);
			LearnTalent(player, Talents::Mage::EMPOWERED_ARCANE_MISSILES_RNK_3);
            TEST_CHANNEL_DAMAGE(player, dummyTarget, ClassSpells::Mage::ARCANE_MISSILES_RNK_10, ClassSpells::Mage::ARCANE_MISSILES_RNK_10_PROC, 5, expectedTickDmg);
            /* 
            Does not work... According to our formula in Unit::CalculateSpellDamage the base damage is 260 + 1.2 per level between 64 and 68 -> 264.8 -> 264
            This is a very fondamental function working for most other spells... Is the tooltip wrong?
            */
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
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FIRE_BLAST_RNK_9, fireblastMinDamage, fireblastMaxDamage);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage, fireballMaxDamage);
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FLAMESTRIKE_RNK_7, flamestrikeMinDamage, flamestrikeMaxDamage);
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::MOLTEN_ARMOR_RNK_1, moltenArmorDamage);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::SCORCH_RNK_9, scorchMinDamage, scorchMaxDamage);

			//Test improved damage 10%
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FIRE_BLAST_RNK_9, fireblastMinDamage * 1.1f, fireblastMaxDamage * 1.1f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage * 1.1f, fireballMaxDamage * 1.1f);
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FLAMESTRIKE_RNK_7, flamestrikeMinDamage * 1.1f, flamestrikeMaxDamage * 1.1f);
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::MOLTEN_ARMOR_RNK_1, moltenArmorDamage * 1.1f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::SCORCH_RNK_9, scorchMinDamage * 1.1f, scorchMaxDamage * 1.1f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FirePowerTestImpt>();
	}
};

class EmpoweredFireballTest : public TestCaseScript
{
public:

    EmpoweredFireballTest() : TestCaseScript("talents mage empowered_fireball") { }

    class EmpoweredFireballTestImpt : public TestCase
    {
    public:
        EmpoweredFireballTestImpt() : TestCase(true) { }

        void Test() override
        {
            TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

            float const fireballSpellCoeff = 100.00;
            float const playerFireSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
            float const fireballMinDamage = 649 + (fireballSpellCoeff + 15) * playerFireSpellPower;
            float const fireballMaxDamage = 821 + (fireballSpellCoeff + 15) * playerFireSpellPower;

            Creature* dummyTarget = SpawnCreature();
            LearnTalent(player, Talents::Mage::EMPOWERED_FIREBALL_RNK_5);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FIREBALL_RNK_13, fireballMinDamage, fireballMaxDamage);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<EmpoweredFireballTestImpt>();
    }
};

class FrostWardingTest : public TestCaseScript
{
public:
	FrostWardingTest() : TestCaseScript("talents mage frost_warding") { }

	class FrostWardingTestImpt : public TestCase
	{
	public:
		FrostWardingTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* mage1 = SpawnRandomPlayer(CLASS_MAGE);
			TestPlayer* mage2 = SpawnRandomPlayer(CLASS_MAGE);

			uint32 const expectedArmorFA = mage1->GetArmor() + 200 * 1.3f;

			uint32 const expectedArmorIA = mage2->GetArmor() + 645 * 1.3f;
			uint32 const expectedFrostResIA = mage2->GetResistance(SPELL_SCHOOL_FROST) + 18 * 1.3f;

			LearnTalent(mage1, Talents::Mage::FROST_WARDING_RNK_2);
			mage1->CastSpell(mage1, ClassSpells::Mage::FROST_ARMOR_RNK_3);
			TEST_ASSERT(Between<float>(mage1->GetArmor(), expectedArmorFA - 1, expectedArmorFA + 1));

			LearnTalent(mage2, Talents::Mage::FROST_WARDING_RNK_2);
			mage2->CastSpell(mage2, ClassSpells::Mage::ICE_ARMOR_RNK_5);
			TEST_ASSERT(Between<float>(mage2->GetArmor(), expectedArmorIA - 1, expectedArmorIA + 1));
			TEST_ASSERT(Between<float>(mage2->GetResistance(SPELL_SCHOOL_FROST), expectedFrostResIA - 1, expectedFrostResIA + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FrostWardingTestImpt>();
	}
};

class PiercingIceTest : public TestCaseScript
{
public:

	PiercingIceTest() : TestCaseScript("talents mage piercing_ice") { }

	class PiercingIceTestImpt : public TestCase
	{
	public:
		PiercingIceTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			// Blizzard rank 7
			uint32 const blizzardTotal = 1920;

			// Cone of cold rank 6
			uint32 const coneMinDamage = 544;
			uint32 const coneMaxDamage = 594;

			// Frost nova rank 5
			uint32 const novaMinDamage = 130;
			uint32 const novaMaxDamage = 147;

			// Frostbolt rank 13
			uint32 const frostboltMinDamage = 780;
			uint32 const frostboltMaxDamage = 841;

			// Ice lance rank 1
			uint32 const lanceMinDamage = 173;
			uint32 const lanceMaxDamage = 200;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Mage::PIERCING_ICE_RNK_3);

			//Test improved damage 6%
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::BLIZZARD_RNK_7, blizzardMinDamage * 1.06f, blizzardMaxDamage * 1.06f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::CONE_OF_COLD_RNK_6, coneMinDamage * 1.06f, coneMaxDamage * 1.06f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FROST_NOVA_RNK_5, novaMinDamage * 1.06f, novaMaxDamage * 1.06f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMinDamage * 1.06f, frostboltMaxDamage * 1.06f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::ICE_LANCE_RNK_1, lanceMinDamage * 1.06f, lanceMaxDamage * 1.06f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<PiercingIceTestImpt>();
	}
};

class ImprovedConeOfColdTest : public TestCaseScript
{
public:

	ImprovedConeOfColdTest() : TestCaseScript("talents mage improved_cone_of_cold") { }

	class ImprovedConeOfColdTestImpt : public TestCase
	{
	public:
		ImprovedConeOfColdTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			// Cone of cold rank 6
			float const coneSpellCoeff = 13.57;
			float const playerFrostSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST);;
			float const coneMinDamage = 544 + coneSpellCoeff * playerFrostSpellPower;
			float const coneMaxDamage = 594 + coneSpellCoeff * playerFrostSpellPower;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Mage::IMPROVED_CONE_OF_COLD_RNK_3);

			//Test improved damage 6%
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::CONE_OF_COLD_RNK_6, coneMinDamage * 1.35f, coneMaxDamage * 1.35f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedConeOfColdTestImpt>();
	}
};

class ArcticWindsTest : public TestCaseScript
{
public:

	ArcticWindsTest() : TestCaseScript("talents mage arctic_winds") { }

	class ArcticWindsTestImpt : public TestCase
	{
	public:
		ArcticWindsTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			// Blizzard rank 7
			uint32 const blizzardTotal = 1920;

			// Cone of cold rank 6
			uint32 const coneMinDamage = 544;
			uint32 const coneMaxDamage = 594;

			// Frost nova rank 5
			uint32 const novaMinDamage = 130;
			uint32 const novaMaxDamage = 147;

			// Frostbolt rank 13
			uint32 const frostboltMinDamage = 780;
			uint32 const frostboltMaxDamage = 841;

			// Ice lance rank 1
			uint32 const lanceMinDamage = 173;
			uint32 const lanceMaxDamage = 200;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Mage::ARCTIC_WINDS_RNK_5);

			//Test improved damage 6%
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::BLIZZARD_RNK_7, blizzardMinDamage * 1.06f, blizzardMaxDamage * 1.06f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::CONE_OF_COLD_RNK_6, coneMinDamage * 1.05f, coneMaxDamage * 1.05f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FROST_NOVA_RNK_5, novaMinDamage * 1.05f, novaMaxDamage * 1.05f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMinDamage * 1.05f, frostboltMaxDamage * 1.05f);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::ICE_LANCE_RNK_1, lanceMinDamage * 1.05f, lanceMaxDamage * 1.05f);

            // TODO: test hit rating from melee and ranged attackers is reduced
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ArcticWindsTestImpt>();
	}
};

class EmpoweredFrostboltTest : public TestCaseScript
{
public:

	EmpoweredFrostboltTest() : TestCaseScript("talents mage empowered_frostbolt") { }

	class EmpoweredFrostboltTestImpt : public TestCase
	{
	public:
		EmpoweredFrostboltTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_MAGE);

			// Cone of cold rank 6
			float const frostboltSpellCoeff = 81.43;
			float const playerFrostSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FROST);
			float const frostboltMinDamage = 780 + (frostboltSpellCoeff + 10) * playerFrostSpellPower;
			float const frostboltMaxDamage = 841 + (frostboltSpellCoeff + 10) * playerFrostSpellPower;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Mage::IMPROVED_FROSTBOLT_RNK_5);
            TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Mage::FROSTBOLT_RNK_13, frostboltMinDamage, frostboltMaxDamage);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EmpoweredFrostboltTestImpt>();
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
