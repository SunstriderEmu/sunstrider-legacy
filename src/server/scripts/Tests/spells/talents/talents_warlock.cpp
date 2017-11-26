#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"
#include "ClassSpells.h"
#include "Pet.h"
#include "ClassSpellsDamage.h"

class ImprovedLifeTapTest : public TestCaseScript
{
public:
	ImprovedLifeTapTest() : TestCaseScript("talents warlock improved_life_tap") { }

	class ImprovedLifeTapTestImpt : public TestCase
	{
	public:
		ImprovedLifeTapTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);
			DisableRegen(player);
			player->SetPower(POWER_MANA, 0);

			float const expectedMana = 580 * 1.2f;

			LearnTalent(player, Talents::Warlock::IMPROVED_LIFE_TAP_RNK_2);
			player->CastSpell(player, ClassSpells::Warlock::LIFE_TAP_RNK_7);
			TEST_ASSERT(player->GetPower(POWER_MANA) == expectedMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedLifeTapTestImpt>();
	}
};

class ImprovedCurseOfAgonyTest : public TestCaseScript
{
public:

	ImprovedCurseOfAgonyTest() : TestCaseScript("talents warlock improved_curse_of_agony") { }

	class ImprovedCurseOfAgonyTestImpt : public TestCase
	{
	public:
		ImprovedCurseOfAgonyTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const coaSpellCoeff = 120.00;
			float const playerShadowSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
			float const expectedCoAMaxDamage = 1356 * 1.1f + coaSpellCoeff * playerShadowSpellPower;
			float const expectedCoADamage = (4 * expectedCoAMaxDamage / 24) + (4 * expectedCoAMaxDamage / 12) + (4 * expectedCoAMaxDamage / 8);

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::IMPROVED_CURSE_OF_AGONY_RNK_2);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedCurseOfAgonyTestImpt>();
	}
};

class EmpoweredCorruptionTest : public TestCaseScript
{
public:

	EmpoweredCorruptionTest() : TestCaseScript("talents warlock empowered_corruption") { }

	class EmpoweredCorruptionTestImpt : public TestCase
	{
	public:
		EmpoweredCorruptionTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const corruptionSpellCoeff = 93.6;
			float const playerShadowSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
			float const corruptionDamage = 900 + (corruptionSpellCoeff * 1.25f) * playerShadowSpellPower;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::EMPOWERED_CORRUPTION_RNK_3);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, corruptionDamage);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EmpoweredCorruptionTestImpt>();
	}
};

class ShadowMasteryTest : public TestCaseScript
{
public:

	ShadowMasteryTest() : TestCaseScript("talents warlock shadow_mastery") { }

	class ShadowMasteryTestImpt : public TestCase
	{
	public:
		ShadowMasteryTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const corruptionDamage = 900;
			float const coaDamage = 1356;
			float const codDamage = 4200;
			float const deathCoilDamage = 519;
			float const drainLifeDamage = 108;
			float const drainSoulDamage = 620;
			float const socDamage = 1044;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::SHADOW_MASTERY_RNK_5);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, corruptionDamage * 1.1f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, coaDamage * 1.1f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, codDamage * 1.1f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::DEATH_COIL_RNK_4, deathCoilDamage * 1.1f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, drainLifeDamage * 1.1f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, drainSoulDamage * 1.1f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, socDamage * 1.1f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ShadowMasteryTestImpt>();
	}
};

class ContagionTest : public TestCaseScript
{
public:

	ContagionTest() : TestCaseScript("talents warlock contagion") { }

	class ContagionTestImpt : public TestCase
	{
	public:
		ContagionTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const corruptionDamage = 900;
			float const coaDamage = 1356;
			float const socDamage = 1044;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::SHADOW_MASTERY_RNK_5);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, corruptionDamage * 1.05f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, coaDamage * 1.05f);
			TestDotDamage(player, dummyTarget, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, socDamage * 1.05f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ContagionTestImpt>();
	}
};

class ImprovedHealthstoneTest : public TestCaseScript
{
public:
	ImprovedHealthstoneTest() : TestCaseScript("talents warlock improved_healthstone") { }

	class ImprovedHealthstoneTestImpt : public TestCase
	{
	public:
		ImprovedHealthstoneTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);
			DisableRegen(player);
			player->SetHealth(1);

			float const expectedHealth = 2080 * 1.2f + 1;

			LearnTalent(player, Talents::Warlock::IMPROVED_HEALTHSTONE_RNK_2);
			player->CastSpell(player, ClassSpells::Warlock::CREATE_HEALTHSTONE_RNK_6);
			Wait(3500);
			//UseItem(22105) // Master Healthstone
			TEST_ASSERT(player->GetHealth() == expectedHealth);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedHealthstoneTestImpt>();
	}
};

class DemonicEmbraceTest : public TestCaseScript
{
public:
	DemonicEmbraceTest() : TestCaseScript("talents warlock demonic_embrace") { }

	class DemonicEmbraceTestImpt : public TestCase
	{
	public:
		DemonicEmbraceTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			uint32 const startSta = player->GetStat(STAT_STAMINA);
			uint32 const expectedSta = startSta * 1.15f;
			uint32 const startSpi = player->GetStat(STAT_SPIRIT);
			uint32 const expectedSpi = startSpi * 0.95f;

			LearnTalent(player, Talents::Warlock::DEMONIC_EMBRACE_RNK_5);
			TEST_ASSERT(Between<float>(player->GetStat(STAT_STAMINA), expectedSta - 1, expectedSta + 1));
			TEST_ASSERT(Between<float>(player->GetStat(STAT_SPIRIT), expectedSpi - 1, expectedSpi + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicEmbraceTestImpt>();
	}
};

class ImprovedHealthFunnelTest : public TestCaseScript
{
public:
	ImprovedHealthFunnelTest() : TestCaseScript("talents warlock improved_health_funnel") { }

	class ImprovedHealthFunnelTestImpt : public TestCase
	{
	public:
		ImprovedHealthFunnelTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);
			DisableRegen(player);

			uint32 const startHealth = player->GetHealth();
			float const expectedHealth = startHealth - 99 - 65 * 10;

			LearnTalent(player, Talents::Warlock::IMPROVED_HEALTH_FUNNEL_RNK_2);
			AddItem(player, 6265, 1);
			Wait(1000);
			uint32 res = player->CastSpell(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			Wait(10500);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);

			pet->SetHealth(1);
			float const expectedPetHealth = 1 + 188 * 10;

			uint32 result = player->CastSpell(player, ClassSpells::Warlock::HEALTH_FUNNEL_RNK_8);
			TEST_ASSERT(result == SPELL_CAST_OK);
			TEST_ASSERT(player->GetHealth() == expectedHealth);
			TEST_ASSERT(pet->GetHealth() == expectedPetHealth);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedHealthFunnelTestImpt>();
	}
};

class FelIntellectTest : public TestCaseScript
{
public:
	FelIntellectTest() : TestCaseScript("talents warlock fel_intellect") { }

	class FelIntellectTestImpt : public TestCase
	{
	public:
		FelIntellectTestImpt() : TestCase(true) { }

		float GetPetInt(TestPlayer* player, uint32 summon)
		{
			uint32 res = player->CastSpell(player, summon, true);
			Wait(1000);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);
			return pet->GetStat(STAT_INTELLECT);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			uint32 const startMana = player->GetPower(POWER_MANA);
			float const expectedMana = startMana * 1.03f;


			AddItem(player, 6265, 8); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 8, false));

			float const impInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			float const voidwalkerInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			float const succubusInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			float const felhunterInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			float const felguardInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);

			LearnTalent(player, Talents::Warlock::FEL_INTELLECT_RNK_3);
			Wait(1000);
			TEST_ASSERT(Between<float>(player->GetPower(POWER_MANA), startMana - 1, expectedMana + 1));

			float const impNextInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			TEST_ASSERT(Between<float>(impNextInt, impInt * 1.15f - 1, impInt * 1.15f - 1));

			float const voidwalkerNextInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			TEST_ASSERT(Between<float>(voidwalkerNextInt, voidwalkerInt * 1.15f - 1, voidwalkerInt * 1.15f - 1));

			float const succubusNextInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			TEST_ASSERT(Between<float>(succubusNextInt, succubusInt * 1.15f - 1, succubusInt * 1.15f - 1));

			float const felhunterNextInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
			TEST_ASSERT(Between<float>(felhunterNextInt, felhunterInt * 1.15f - 1, felhunterInt * 1.15f - 1));

			float const felguardNextInt = GetPetInt(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);
			TEST_ASSERT(Between<float>(felguardNextInt, felguardInt * 1.15f - 1, felguardInt * 1.15f - 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FelIntellectTestImpt>();
	}
};

class FelDominationTest : public TestCaseScript
{
public:
	FelDominationTest() : TestCaseScript("talents warlock fel_domination") { }

	class FelDominationTestImpt : public TestCase
	{
	public:
		FelDominationTestImpt() : TestCase(true) { }

		uint32 GetRemainingMana(TestPlayer* player, uint32 summon, uint32 startMana, uint32 manaCost)
		{
			player->RemoveAllSpellCooldown();
			uint32 feldom = player->CastSpell(player, ClassSpells::Warlock::FEL_DOMINATION_RNK_1);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(feldom == SPELL_CAST_OK);

			uint32 res = player->CastSpell(player, summon);
			Wait(5 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);

			TEST_ASSERT(player->GetPower(POWER_MANA) == startMana - floor(manaCost * 0.5));

			return player->GetPower(POWER_MANA);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			uint32 const summonImp = 1673;
			uint32 const summonVoid = 2092;
			uint32 const summonSuc = 2092;
			uint32 const summonFh = 2092;
			uint32 const summonFg = 2092;

			AddItem(player, 6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			uint32 const startMana = 10000;
			DisableRegen(player);
			player->SetMaxPower(POWER_MANA, startMana);
			player->SetPower(POWER_MANA, startMana);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 10000);

			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(player, Talents::Warlock::FEL_DOMINATION_RNK_1);
			uint32 currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1, startMana, summonImp);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, currentMana, summonVoid);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, currentMana, summonSuc);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, currentMana, summonFh);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, currentMana, summonFg);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FelDominationTestImpt>();
	}
};

class FelStaminaTest : public TestCaseScript
{
public:
	FelStaminaTest() : TestCaseScript("talents warlock fel_stamina") { }

	class FelStaminaTestImpt : public TestCase
	{
	public:
		FelStaminaTestImpt() : TestCase(true) { }

		float GetPetSta(TestPlayer* player, uint32 summon)
		{
			uint32 res = player->CastSpell(player, summon, true);
			Wait(1000);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);
			return pet->GetStat(STAT_STAMINA);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			uint32 const startHealth = player->GetHealth();
			float const expectedHealth = startHealth * 1.03f;


			AddItem(player, 6265, 8); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 8, false));

			float const impInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			float const voidwalkerInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			float const succubusInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			float const felhunterInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			float const felguardInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);

			LearnTalent(player, Talents::Warlock::FEL_STAMINA_RNK_3);
			Wait(1000);
			TEST_ASSERT(Between<float>(player->GetHealth(), startHealth - 1, expectedHealth + 1));

			float const impNextInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			TEST_ASSERT(Between<float>(impNextInt, impInt * 1.15f - 1, impInt * 1.15f - 1));

			float const voidwalkerNextInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			TEST_ASSERT(Between<float>(voidwalkerNextInt, voidwalkerInt * 1.15f - 1, voidwalkerInt * 1.15f - 1));

			float const succubusNextInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			TEST_ASSERT(Between<float>(succubusNextInt, succubusInt * 1.15f - 1, succubusInt * 1.15f - 1));

			float const felhunterNextInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_FELSTEED_RNK_1);
			TEST_ASSERT(Between<float>(felhunterNextInt, felhunterInt * 1.15f - 1, felhunterInt * 1.15f - 1));

			float const felguardNextInt = GetPetSta(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);
			TEST_ASSERT(Between<float>(felguardNextInt, felguardInt * 1.15f - 1, felguardInt * 1.15f - 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FelStaminaTestImpt>();
	}
};

class DemonicAegisTest : public TestCaseScript
{
public:
	DemonicAegisTest() : TestCaseScript("talents warlock demonic_aegis") { }

	class DemonicAegisTestImpt : public TestCase
	{
	public:
		DemonicAegisTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			// Demon Armor
			const float regen = floor(player->OCTRegenHPPerSpirit());
			const uint32 startHealth = 1;
			player->SetHealth(startHealth);

			const uint32 startArmor = player->GetArmor();
			const uint32 startDASR = player->GetResistance(SPELL_SCHOOL_SHADOW);

			const float expectedDAArmor = startArmor + floor(660 * 1.3f);
			const float expectedDASR = startDASR + floor(18 * 1.3f);

			LearnTalent(player, Talents::Warlock::DEMONIC_AEGIS_RNK_3);
			uint32 result = player->CastSpell(player, ClassSpells::Warlock::DEMON_ARMOR_RNK_6);
			TEST_ASSERT(player->GetHealth() == 1);
			Wait(1000);
			TEST_ASSERT(result == SPELL_CAST_OK);
			TEST_ASSERT(player->GetArmor() == expectedDAArmor);
			TEST_ASSERT(player->GetResistance(SPELL_SCHOOL_SHADOW) == expectedDASR);
			Wait(10000);
			TEST_ASSERT(player->GetHealth() == startHealth + 10 * regen + 2 * floor(18 * 1.3f));
			Wait(1000);
			TEST_ASSERT(player->GetHealth() == startHealth + 12 * regen + 2 * floor(18 * 1.3f));


			// Fel armor
			float const startSP = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
			float const expectedSP = startSP + 100 * 1.3f;

			result = player->CastSpell(player, ClassSpells::Warlock::FEL_ARMOR_RNK_2);
			Wait(1000);
			TEST_ASSERT(result == SPELL_CAST_OK);
			TEST_ASSERT(!player->HasAura(ClassSpells::Warlock::DEMON_ARMOR_RNK_6));
			TEST_ASSERT(Between<float>(player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW), expectedSP - 1, expectedSP + 1));
			// TODO: test healing done on warlock
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicAegisTestImpt>();
	}
};

class MasterSummonerTest : public TestCaseScript
{
public:
	MasterSummonerTest() : TestCaseScript("talents warlock master_summoner") { }

	class MasterSummonerTestImpt : public TestCase
	{
	public:
		MasterSummonerTestImpt() : TestCase(true) { }

		uint32 GetRemainingMana(TestPlayer* player, uint32 summon, uint32 startMana, uint32 manaCost)
		{
			player->RemoveAllSpellCooldown();
			uint32 feldom = player->CastSpell(player, ClassSpells::Warlock::FEL_DOMINATION_RNK_1);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(feldom == SPELL_CAST_OK);

			uint32 res = player->CastSpell(player, summon);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);

			TEST_ASSERT(player->GetPower(POWER_MANA) == startMana - floor(manaCost * (0.5 - 0.4)));

			return player->GetPower(POWER_MANA);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			uint32 const summonImp = 1673;
			uint32 const summonVoid = 2092;
			uint32 const summonSuc = 2092;
			uint32 const summonFh = 2092;
			uint32 const summonFg = 2092;

			AddItem(player, 6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			uint32 const startMana = 10000;
			DisableRegen(player);
			player->SetMaxPower(POWER_MANA, startMana);
			player->SetPower(POWER_MANA, startMana);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 10000);

			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(player, Talents::Warlock::FEL_DOMINATION_RNK_1);
			LearnTalent(player, Talents::Warlock::MASTER_SUMMONER_RNK_2);
			uint32 currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1, startMana, summonImp);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, currentMana, summonVoid);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, currentMana, summonSuc);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, currentMana, summonFh);
			currentMana = GetRemainingMana(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, currentMana, summonFg);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MasterSummonerTestImpt>();
	}
};

class DemonicSacrificeTest : public TestCaseScript
{
public:
	DemonicSacrificeTest() : TestCaseScript("talents warlock demonic_sacrifice") { }

	class DemonicSacrificeTestImpt : public TestCase
	{
	public:
		DemonicSacrificeTestImpt() : TestCase(true) { }

		void SacrificePet(TestPlayer* player, uint32 summon, uint32 aura, uint32 previousAura = 0)
		{
			uint32 res = player->CastSpell(player, summon, true);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);
			if(previousAura != 0)
				TEST_ASSERT(!player->HasAura(previousAura));

			res = player->CastSpell(player, ClassSpells::Warlock::DEMONIC_SACRIFICE_RNK_1, true);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);

			TEST_ASSERT(player->HasAura(aura));
		}

		void TestImpSacrifice(TestPlayer* player, Creature* dummyTarget)
		{
			float const sp = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);

			uint32 expectedIncinerateMin = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * 1.15f + ClassSpellsCoeff::Warlock::INCINERATE * sp;
			uint32 expectedIncinerateMax = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * 1.15f + ClassSpellsCoeff::Warlock::INCINERATE * sp;

			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin, expectedIncinerateMax);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			uint32 const summonImp = 1673;
			uint32 const summonVoid = 2092;
			uint32 const summonSuc = 2092;
			uint32 const summonFh = 2092;
			uint32 const summonFg = 2092;

			AddItem(player, 6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			uint32 const startMana = 10000;
			DisableRegen(player);
			player->SetMaxPower(POWER_MANA, startMana);
			player->SetPower(POWER_MANA, startMana);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 10000);


			Creature* dummyTarget = SpawnCreature();

			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(player, Talents::Warlock::DEMONIC_SACRIFICE_RNK_1);
			SacrificePet(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1, 18789); // Burning Wish
			SacrificePet(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, 18790, 18789); // Fel Stamina
			SacrificePet(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, 18791, 18790); // Touch of Shadow
			SacrificePet(player, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, 18792, 18791); // Fel Energy
			SacrificePet(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, 35701, 18792); // Touch of Shadow
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicSacrificeTestImpt>();
	}
};

void AddSC_test_talents_warlock()
{
	// Affliction
	new ImprovedLifeTapTest();
	new ImprovedCurseOfAgonyTest();
	new EmpoweredCorruptionTest();
	new ShadowMasteryTest();
	new ContagionTest();
	// Demonology
	new ImprovedHealthstoneTest();
	new DemonicEmbraceTest();
	new ImprovedHealthFunnelTest();
	new FelIntellectTest();
	new FelDominationTest();
	new FelStaminaTest();
	new DemonicAegisTest();
	new MasterSummonerTest();
	new DemonicSacrificeTest();
}
