#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

class ImprovedLifeTapTest : public TestCaseScript
{
public:
	ImprovedLifeTapTest() : TestCaseScript("talents warlock improved_life_tap") { }

	class ImprovedLifeTapTestImpt : public TestCase
	{
	public:
		ImprovedLifeTapTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);
            player->DisableRegeneration(true);
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
		ImprovedCurseOfAgonyTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const coaSpellCoeff = 120.00;
			float const playerShadowSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
			float const expectedCoAMaxDamage = 1356 * 1.1f + coaSpellCoeff * playerShadowSpellPower;
			float const expectedCoADamage = (4 * expectedCoAMaxDamage / 24) + (4 * expectedCoAMaxDamage / 12) + (4 * expectedCoAMaxDamage / 8);

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::IMPROVED_CURSE_OF_AGONY_RNK_2);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoADamage, false);
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
		EmpoweredCorruptionTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const corruptionSpellCoeff = 93.6;
			float const playerShadowSpellPower = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
			float const corruptionDamage = 900 + (corruptionSpellCoeff * 1.25f) * playerShadowSpellPower;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::EMPOWERED_CORRUPTION_RNK_3);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, corruptionDamage, false);
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
		ShadowMasteryTestImpt() : TestCase(STATUS_PASSING, true) { }

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
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, corruptionDamage * 1.1f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, coaDamage * 1.1f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, codDamage * 1.1f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::DEATH_COIL_RNK_4, deathCoilDamage * 1.1f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::DRAIN_LIFE_RNK_8, drainLifeDamage * 1.1f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::DRAIN_SOUL_RNK_5, drainSoulDamage * 1.1f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, socDamage * 1.1f, false);
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
		ContagionTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const corruptionDamage = 900;
			float const coaDamage = 1356;
			float const socDamage = 1044;

			Creature* dummyTarget = SpawnCreature();
			LearnTalent(player, Talents::Warlock::SHADOW_MASTERY_RNK_5);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, corruptionDamage * 1.05f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, coaDamage * 1.05f, false);
            TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, socDamage * 1.05f, false);
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
		ImprovedHealthstoneTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);
            player->DisableRegeneration(true);
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
		DemonicEmbraceTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		ImprovedHealthFunnelTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);
            player->DisableRegeneration(true);

			uint32 const startHealth = player->GetHealth();
			float const expectedHealth = startHealth - 99 - 65 * 10;

			LearnTalent(player, Talents::Warlock::IMPROVED_HEALTH_FUNNEL_RNK_2);
            player->AddItem(6265, 1);
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
		FelIntellectTestImpt() : TestCase(STATUS_PASSING, true) { }

		float GetPetInt(TestPlayer* player, uint32 summon)
		{
			uint32 res = player->CastSpell(player, summon, TRIGGERED_FULL_MASK);
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

            player->AddItem(6265, 8); // Soul shard
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
		FelDominationTestImpt() : TestCase(STATUS_PASSING, true) { }

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

            player->AddItem(6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			uint32 const startMana = 10000;
            player->DisableRegeneration(true);
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
		FelStaminaTestImpt() : TestCase(STATUS_PASSING, true) { }

		float GetPetSta(TestPlayer* player, uint32 summon)
		{
			uint32 res = player->CastSpell(player, summon, TRIGGERED_FULL_MASK);
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


            player->AddItem(6265, 8); // Soul shard
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
		DemonicAegisTestImpt() : TestCase(STATUS_PASSING, true) { }

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
            TEST_HAS_NOT_AURA(player, ClassSpells::Warlock::DEMON_ARMOR_RNK_6);
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
		MasterSummonerTestImpt() : TestCase(STATUS_PASSING, true) { }

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

            player->AddItem(6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			uint32 const startMana = 10000;
            player->DisableRegeneration(true);
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
		DemonicSacrificeTestImpt() : TestCase(STATUS_PASSING, true) { }

		void SacrificePet(TestPlayer* player, uint32 summon, uint32 aura, uint32 previousAura = 0)
		{
			uint32 res = player->CastSpell(player, summon, TRIGGERED_FULL_MASK);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);
			if(previousAura != 0)
                TEST_HAS_NOT_AURA(player, previousAura);

			res = player->CastSpell(player, ClassSpells::Warlock::DEMONIC_SACRIFICE_RNK_1, TRIGGERED_FULL_MASK);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);

            TEST_HAS_AURA(player, aura);
		}

		void TestImpSacrifice(TestPlayer* player, Creature* dummyTarget)
		{
			float const sp = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);

			// Immolate
			uint32 expectedDirectImmolate = floor(ClassSpellsDamage::Warlock::IMMOLATE_RNK_9 * 1.15f + sp * ClassSpellsCoeff::Warlock::IMMOLATE);
			uint32 expectedDotImmolate = floor(ClassSpellsDamage::Warlock::IMMOLATE_RNK_9_DOT * 1.15f + sp * ClassSpellsCoeff::Warlock::IMMOLATE_DOT);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedDirectImmolate, expectedDirectImmolate, false);
			TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::IMMOLATE_RNK_9, expectedDotImmolate, false);

			// Rain of Fire
			uint32 expectedRoFTick = floor((ClassSpellsDamage::Warlock::RAIN_OF_FIRE_RNK_5_TOTAL * 1.15f + sp * ClassSpellsCoeff::Warlock::RAIN_OF_FIRE) * 0.25f);
			TEST_CHANNEL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5, ClassSpells::Warlock::RAIN_OF_FIRE_RNK_5_PROC, 4, expectedRoFTick);

			// Incinerate
			uint32 expectedIncinerateMin = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MIN * 1.15f + sp * ClassSpellsCoeff::Warlock::INCINERATE;
			uint32 expectedIncinerateMax = ClassSpellsDamage::Warlock::INCINERATE_RNK_2_MAX * 1.15f + sp * ClassSpellsCoeff::Warlock::INCINERATE;
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::INCINERATE_RNK_2, expectedIncinerateMin, expectedIncinerateMax, false);

			// Searing Pain
			uint32 expectedSearingPainMin = ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MIN * 1.15f + sp * ClassSpellsCoeff::Warlock::SEARING_PAIN;
			uint32 expectedSearingPainMax = ClassSpellsDamage::Warlock::SEARING_PAIN_RNK_8_MAX * 1.15f + sp * ClassSpellsCoeff::Warlock::SEARING_PAIN;
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SEARING_PAIN_RNK_8, expectedSearingPainMin, expectedSearingPainMax, false);

			// Soul Fire
			uint32 expectedSoulFireMin = ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MIN * 1.15f + sp * ClassSpellsCoeff::Warlock::SOUL_FIRE;
			uint32 expectedSoulFireMax = ClassSpellsDamage::Warlock::SOUL_FIRE_RNK_4_MAX * 1.15f + sp * ClassSpellsCoeff::Warlock::SOUL_FIRE;
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SOUL_FIRE_RNK_4, expectedSoulFireMin, expectedSoulFireMax, false);

			// Hellfire
			TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            player->DisableRegeneration(true);
            enemy->DisableRegeneration(true);
			player->SetMaxHealth(10000);
			player->SetHealth(10000);
			enemy->SetMaxHealth(10000);
			enemy->SetHealth(10000);
			uint32 startPlayerHealth = player->GetHealth();
			uint32 startEnemyHealth = enemy->GetHealth();
			uint32 expectedPlayerHealth = startPlayerHealth - 15 * floor(ClassSpellsDamage::Warlock::HELLFIRE_RNK_4_TICK * 1.15f + sp * ClassSpellsCoeff::Warlock::HELLFIRE_SELF);
			uint32 expectedEnemyHealth = startEnemyHealth - 15 * floor(ClassSpellsDamage::Warlock::HELLFIRE_RNK_4_TICK * 1.15f + sp * ClassSpellsCoeff::Warlock::HELLFIRE_ENEMIES);
			player->CastSpell(player, ClassSpells::Warlock::HELLFIRE_RNK_4);
			Wait(15500);
			TEST_ASSERT(Between<float>(player->GetHealth(), expectedPlayerHealth - 1, expectedPlayerHealth + 1));
			TEST_ASSERT(Between<float>(enemy->GetHealth(), expectedEnemyHealth - 1, expectedEnemyHealth + 1));

		}

		void TestVoidwalkerSacrifice(TestPlayer* player)
		{
			player->DisableRegeneration(false);
			player->SetHealth(1);

			Wait(10 * SECOND * IN_MILLISECONDS);
			const float regen = floor(player->OCTRegenHPPerSpirit());
			uint32 totalHealth = player->GetMaxHealth();
			uint32 expectedHealth = 1 + floor(10 * regen + 2 * totalHealth * 0.01f);
			TEST_ASSERT(player->GetHealth() == expectedHealth);
			Wait(2500);
			expectedHealth = 1 + floor(10 * regen + 3 * totalHealth * 0.01f);
			TEST_ASSERT(player->GetHealth() == expectedHealth);
		}

		void TestSuccubusSacrifice(TestPlayer* player, Creature* dummyTarget, float percentage)
		{
			float const sp = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			// Death Coil
			uint32 expectedDeathCoil = floor(ClassSpellsDamage::Warlock::DEATH_COIL_RNK_4 * percentage + sp * ClassSpellsCoeff::Warlock::DEATH_COIL);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::DEATH_COIL_RNK_4, expectedDeathCoil, expectedDeathCoil, false);

			// Shadow Bolt
			uint32 expectedShadowBoltMin = floor(ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MIN * percentage + sp * ClassSpellsCoeff::Warlock::SHADOW_BOLT);
			uint32 expectedShadowBoltMax = floor(ClassSpellsDamage::Warlock::SHADOW_BOLT_RNK_11_MAX * percentage + sp * ClassSpellsCoeff::Warlock::SHADOW_BOLT);
			TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, expectedShadowBoltMin, expectedShadowBoltMax, false);

			// TODO: Shadowburn (takes 1 soul shard per cast)
			uint32 expectedShadowburnMin = floor(ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MIN * percentage + sp * ClassSpellsCoeff::Warlock::SHADOWBURN);
			uint32 expectedShadowburnMax = floor(ClassSpellsDamage::Warlock::SHADOWBURN_RNK_8_MAX * percentage + sp * ClassSpellsCoeff::Warlock::SHADOWBURN);
			//TEST_DIRECT_SPELL_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SHADOWBURN_RNK_8, expectedShadowburnMin, expectedShadowburnMax, false);

			// Corruption
			uint32 expectedCorruption = floor(ClassSpellsDamage::Warlock::CORRUPTION_RNK_8_TOTAL * percentage + sp * ClassSpellsCoeff::Warlock::CORRUPTION);
			TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CORRUPTION_RNK_8, expectedCorruption, false);

			// Curse of Agony
			uint32 expectedCoA = floor(ClassSpellsDamage::Warlock::CURSE_OF_AGONY_RNK_7_TOTAL * percentage + sp * ClassSpellsCoeff::Warlock::CURSE_OF_AGONY);
			TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_AGONY_RNK_7, expectedCoA, false);

			// Curse of Doom
			uint32 expectedCoD = floor(ClassSpellsDamage::Warlock::CURSE_OF_DOOM_RNK_2 * percentage + sp * ClassSpellsCoeff::Warlock::CURSE_OF_DOOM);
			TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::CURSE_OF_DOOM_RNK_2, expectedCoD, false);

			// TODO: Seed of Corruption test DoT + end damage
			uint32 expectedDot = floor(ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_TOTAL * percentage + sp * ClassSpellsCoeff::Warlock::SEED_OF_CORRUPTION_DOT);
			uint32 expectedDamageMin = floor(ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MIN * percentage + sp * ClassSpellsCoeff::Warlock::SEED_OF_CORRUPTION);
			uint32 expectedDamageMax = floor(ClassSpellsDamage::Warlock::SEED_OF_CORRUPTION_RNK_1_MAX * percentage + sp * ClassSpellsCoeff::Warlock::SEED_OF_CORRUPTION);
			//TEST_DOT_DAMAGE(player, dummyTarget, ClassSpells::Warlock::SEED_OF_CORRUPTION_RNK_1, expectedCoA);
		}

		void TestFelhunterSacrifice(TestPlayer* player, float percentage)
		{
            player->DisableRegeneration(false);
			player->SetMaxPower(POWER_MANA, 10000);
			uint32 totalMana = player->GetMaxPower(POWER_MANA);
			player->SetPower(POWER_MANA, 0);
			const float regen = floor(sqrt(player->GetStat(STAT_INTELLECT)) * player->OCTRegenMPPerSpirit());
			TEST_ASSERT(player->GetPower(POWER_MANA) == 0);
			Wait(10000);
			uint32 expectedMana = floor(10 * regen + 2 * (totalMana * percentage));
			TEST_ASSERT(player->GetPower(POWER_MANA) == expectedMana);
			Wait(1500);
			expectedMana = floor(12 * regen + 3 * (totalMana * percentage));
			TEST_ASSERT(player->GetPower(POWER_MANA) == expectedMana);
		}

		void TestFelguardSacrifice(TestPlayer* player, Creature* dummyTarget)
		{
			TestSuccubusSacrifice(player, dummyTarget, 1.1f);
			TestFelhunterSacrifice(player, 0.02f);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);

			uint32 const summonImp = 1673;
			uint32 const summonVoid = 2092;
			uint32 const summonSuc = 2092;
			uint32 const summonFh = 2092;
			uint32 const summonFg = 2092;

            player->AddItem(6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			uint32 const startMana = 10000;
            player->DisableRegeneration(true);
			player->SetMaxPower(POWER_MANA, startMana);
			player->SetPower(POWER_MANA, startMana);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 10000);

			Creature* dummyTarget = SpawnCreature();

			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(player, Talents::Warlock::DEMONIC_SACRIFICE_RNK_1);
			SacrificePet(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1, 18789); // Burning Wish
			TestImpSacrifice(player, dummyTarget);
			SacrificePet(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, 18790, 18789); // Fel Stamina
			TestVoidwalkerSacrifice(player);
			SacrificePet(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, 18791, 18790); // Touch of Shadow
			TestSuccubusSacrifice(player, dummyTarget, 1.15f);
			SacrificePet(player, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, 18792, 18791); // Fel Energy
			TestFelhunterSacrifice(player, 0.03f);
			SacrificePet(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, 35701, 18792); // Touch of Shadow
			TestFelguardSacrifice(player, dummyTarget);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicSacrificeTestImpt>();
	}
};

class ManaFeedTest : public TestCaseScript
{
public:
	ManaFeedTest() : TestCaseScript("talents warlock mana_feed") { }

	class ManaFeedTestImpt : public TestCase
	{
	public:
		ManaFeedTestImpt() : TestCase(STATUS_PASSING, true) { }

		// drain mana + life tap 100% to pet

		void AssertManaFeed(TestPlayer* player, TestPlayer* enemy, uint32 summonSpell)
		{
			uint32 res = player->CastSpell(player, summonSpell, TRIGGERED_FULL_MASK);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);

			pet->DisableRegeneration(true);
			pet->SetPower(POWER_MANA, 0);
			player->SetPower(POWER_MANA, 2000);

			Wait(4000);

			// Drain Mana
			uint32 expectedDrain = ClassSpellsDamage::Warlock::DRAIN_MANA_RNK_6_TICK + player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) * ClassSpellsCoeff::Warlock::DRAIN_MANA;
			
			res = player->CastSpell(enemy, ClassSpells::Warlock::DRAIN_MANA_RNK_6, TRIGGERED_FULL_MASK);
            TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(5500);
			TEST_ASSERT(pet->GetPower(POWER_MANA) == expectedDrain);
			
			Wait(4000);

			// Life Tap
			player->CastSpell(player, ClassSpells::Warlock::LIFE_TAP_RNK_7);
			Wait(1500);
			uint32 expectedLT = ClassSpellsDamage::Warlock::LIFE_TAP_RNK_7;
            TC_LOG_TRACE("test.unit_test", "current: %u, expected: %u", pet->GetPower(POWER_MANA), expectedLT);
			Wait(500);
			TEST_ASSERT(pet->GetPower(POWER_MANA) == expectedLT);


			Wait(4000);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_WARLOCK, RACE_ORC);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 10.0f);
			TestPlayer* enemy = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawnPosition);
			Creature* dummyTarget = SpawnCreature();

			player->AddItem(6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(player, Talents::Warlock::MANA_FEED_RNK_3);
			AssertManaFeed(player, enemy, ClassSpells::Warlock::SUMMON_IMP_RNK_1);
			AssertManaFeed(player, enemy, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1);
			AssertManaFeed(player, enemy, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1);
			AssertManaFeed(player, enemy, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1);
			AssertManaFeed(player, enemy, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ManaFeedTestImpt>();
	}
};

class DemonicKnowledgeTest : public TestCaseScript
{
public:
	DemonicKnowledgeTest() : TestCaseScript("talents warlock demonic_knowledge") { }

	class DemonicKnowledgeTestImpt : public TestCase
	{
	public:
		DemonicKnowledgeTestImpt() : TestCase(STATUS_PASSING, true) { }

		void AssertDemonicKnowledge(TestPlayer* player, uint32 summon, float spellPower)
		{
			uint32 res = player->CastSpell(player, summon, TRIGGERED_FULL_MASK);
			Wait(1 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Pet* pet = player->GetPet();
			TEST_ASSERT(pet != nullptr);

			float expectedSP = spellPower + (pet->GetStat(STAT_STAMINA) + pet->GetStat(STAT_INTELLECT)) * 0.12;
            TC_LOG_TRACE("test.unit_test", "current: %f, start: %f, expected: %f", player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW), spellPower, expectedSP);
			TEST_ASSERT(player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW) == expectedSP);
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_WARLOCK);

			float const sp = player->GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			player->AddItem(6265, 4); // Soul shard
			Wait(1000);
			TEST_ASSERT(player->HasItemCount(6265, 4, false));

			LearnTalent(player, Talents::Warlock::SUMMON_FELGUARD_RNK_1);
			LearnTalent(player, Talents::Warlock::DEMONIC_KNOWLEDGE_RNK_3);
			AssertDemonicKnowledge(player, ClassSpells::Warlock::SUMMON_IMP_RNK_1, sp);
			AssertDemonicKnowledge(player, ClassSpells::Warlock::SUMMON_VOIDWALKER_RNK_1, sp);
			AssertDemonicKnowledge(player, ClassSpells::Warlock::SUMMON_SUCCUBUS_RNK_1, sp);
			AssertDemonicKnowledge(player, ClassSpells::Warlock::SUMMON_FELHUNTER_RNK_1, sp);
			AssertDemonicKnowledge(player, ClassSpells::Warlock::SUMMON_FELGUARD_RNK_1, sp);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemonicKnowledgeTestImpt>();
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
	new DemonicKnowledgeTest();
	new ManaFeedTest();
}
