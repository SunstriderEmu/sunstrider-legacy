#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"
#include "SpellHistory.h"
#include "PlayerbotAI.h"

class BarkskinTest : public TestCaseScript
{
public:
	BarkskinTest() : TestCaseScript("spells druid barkskin") { }

	class BarkskinTestImpt : public TestCase
	{
	public:
		BarkskinTestImpt() : TestCase(STATUS_INCOMPLETE) { }

		void CastBarkskin(TestPlayer* druid)
		{
            druid->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(druid, druid, ClassSpells::Druid::BARKSKIN_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
		}

		void TestState(TestPlayer* druid, uint32 spellId, bool shapeshifted = false)
		{
			druid->AddAura(spellId, druid);
			if (shapeshifted)
				druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
			CastBarkskin(druid);
			druid->RemoveAurasDueToSpell(spellId);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BARKSKIN_RNK_1);
		}

		void Test() override
		{
			// Init barkskin druid
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			// Init damage druid
			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, -3.0f);
			TestPlayer* druid2 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);
			EQUIP_ITEM(druid2, 34182); // Grand Magister's Staff of Torrents - 266 SP
            int32 staffSP = 266;
			TEST_ASSERT(druid2->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			druid->DisableRegeneration(true);

			// Mana cost
			uint32 const expectedBarkskinMana = 284;
			TEST_POWER_COST(druid, druid, ClassSpells::Druid::BARKSKIN_RNK_1, POWER_MANA, expectedBarkskinMana);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BARKSKIN_RNK_1);

			// Duration & CD
			CastBarkskin(druid);
			Aura* aura = druid->GetAura(ClassSpells::Druid::BARKSKIN_RNK_1);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetDuration() == 12 * SECOND * IN_MILLISECONDS);
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::BARKSKIN_RNK_1, Minutes(1));

			// Pushback
			uint32 startHealth = 2500;
			druid->SetHealth(startHealth);
            TEST_CAST(druid2, druid2, ClassSpells::Druid::CAT_FORM_RNK_1);
			druid2->Attack(druid, true);
            TEST_CAST(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
			Wait(3400);
			TEST_ASSERT(druid->GetHealth() < startHealth);
			Wait(100);
			TEST_ASSERT(druid->GetHealth() == druid->GetMaxHealth());
			druid2->AttackStop();

			// Prep damage tests
			druid->SetMaxHealth(100000000);
			druid->SetFullHealth();

			// Spell coefficient
			float const barkskinFactor = 0.8f;
			float const starfireSpellCoeff = ClassSpellsCoeff::Druid::STARFIRE;
			uint32 const starfireBonusSP = starfireSpellCoeff * staffSP;
			// Spell Hit
			uint32 const expectedStarfireMinDmg = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN + starfireBonusSP;
			uint32 const expectedStarfireMaxDmg = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX + starfireBonusSP;
			CastBarkskin(druid);
			TEST_DIRECT_SPELL_DAMAGE(druid2, druid, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDmg * barkskinFactor, expectedStarfireMaxDmg * barkskinFactor, false);
			// Spell Crit
			uint32 const expectedStarfireCritMinDmg = expectedStarfireMinDmg * 1.5f * barkskinFactor;
			uint32 const expectedStarfireCritMaxDmg = expectedStarfireMaxDmg * 1.5f * barkskinFactor;
			CastBarkskin(druid);
			TEST_DIRECT_SPELL_DAMAGE(druid2, druid, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireCritMinDmg, expectedStarfireCritMaxDmg, true);

			// Stunned
			TestState(druid, ClassSpells::Rogue::KIDNEY_SHOT_RNK_2);
			// Frozen
			TestState(druid, ClassSpells::Mage::FROST_NOVA_RNK_1);
			// Incapacitated
			TestState(druid, ClassSpells::Rogue::GOUGE_RNK_6);
			// Feared
			TestState(druid, ClassSpells::Warlock::FEAR_RNK_3);
			// Asleep
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
			TestState(druid, ClassSpells::Druid::HIBERNATE_RNK_3, true);

			// Melee damage
			// TODO
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<BarkskinTestImpt>();
	}
};

class CycloneTest : public TestCaseScript
{
public:
	CycloneTest() : TestCaseScript("spells druid cyclone") { }

	class CycloneTestImpt : public TestCase
	{
	public:
		CycloneTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			// Init druid
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			// Init cycloned druid 1 
			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* druid2 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);
			druid2->SetHealth(1);

			// Init cycloned druid 2
			Position spawnPosition2(_location);
			spawnPosition2.MoveInFront(_location, 6.0f);
			TestPlayer* druid3 = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition2);

			// Init friend druid
			Position spawnPosition3(_location);
			spawnPosition3.MoveInFront(_location, 9.0f);
			TestPlayer* druid4 = SpawnPlayer(CLASS_DRUID, RACE_TAUREN, 70, spawnPosition3);

			// Fail on friendly
            TEST_CAST(druid, druid4, ClassSpells::Druid::CYCLONE_RNK_1, SPELL_FAILED_BAD_TARGETS);

			// Cooldown & invulnerable to damage & heals
            TEST_CAST(druid2, druid2, ClassSpells::Druid::LIFEBLOOM_RNK_1);
			Wait(3500);
            TEST_CAST(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
			uint32 const startHealth = druid2->GetHealth();
            TEST_AURA_MAX_DURATION(druid2, ClassSpells::Druid::CYCLONE_RNK_1, Seconds(6));
            TEST_CAST(druid, druid2, ClassSpells::Druid::WRATH_RNK_10);
			Wait(500); // wrath hit
			TEST_ASSERT(druid2->GetHealth() == startHealth);
			Wait(2000); // no more lifebloom
			TEST_ASSERT(druid2->GetHealth() == startHealth);

			// Only one target can be affected
            TEST_CAST(druid, druid3, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
            TEST_HAS_NOT_AURA(druid2, ClassSpells::Druid::CYCLONE_RNK_1);
            TEST_HAS_AURA(druid3, ClassSpells::Druid::CYCLONE_RNK_1);

			// Diminishing returns
			// 3s
            TEST_CAST(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
            TEST_AURA_MAX_DURATION(druid2, ClassSpells::Druid::CYCLONE_RNK_1, Seconds(3));
			druid2->RemoveAurasDueToSpell(ClassSpells::Druid::CYCLONE_RNK_1);

			// 1.5s
            TEST_CAST(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
            TEST_AURA_MAX_DURATION(druid2, ClassSpells::Druid::CYCLONE_RNK_1, Milliseconds(1500));

			// Immune
            TEST_CAST(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
            TEST_HAS_NOT_AURA(druid2, ClassSpells::Druid::CYCLONE_RNK_1);

			// Mana cost
			uint32 const expectedCycloneMana = 189;
			TEST_POWER_COST(druid, druid3, ClassSpells::Druid::CYCLONE_RNK_1, POWER_MANA, 189);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<CycloneTestImpt>();
	}
};

class EntanglingRootsTest : public TestCaseScript
{
public:
	EntanglingRootsTest() : TestCaseScript("spells druid entangling_roots") { }

	class EntanglingRootsTestImpt : public TestCase
	{
	public:
		EntanglingRootsTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedEntanglingRootsMana = 160;
            TEST_POWER_COST(druid, creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, POWER_MANA, expectedEntanglingRootsMana);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, Seconds(27));

			// Is rooted
			TEST_ASSERT(creature->IsInRoots());

			// Spell coefficient
			float const entanglingRootsSpellCoeff = 27.0f / 15.0f / 9.0f;
			uint32 const entanglingRootsBonusSP = entanglingRootsSpellCoeff * staffSP;

			// Damage
			float const EntanglingRootsTick = ClassSpellsDamage::Druid::ENTANGLING_ROOTS_RNK_7_TOTAL / 9;
			uint32 const expectedEntanglingRootsTick = floor(EntanglingRootsTick + entanglingRootsBonusSP);
			uint32 const expectedEntanglingRootsDmg = expectedEntanglingRootsTick * 9;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, expectedEntanglingRootsDmg, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EntanglingRootsTestImpt>();
	}
};

class FaerieFireTest : public TestCaseScript
{
public:

	FaerieFireTest() : TestCaseScript("spells druid faerie_fire") { }

	class FaerieFireTestImpt : public TestCase
	{
	public:
		FaerieFireTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);
			TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawnPosition);

			EQUIP_ITEM(rogue, 34211); // S4 Chest
            uint32 const expectedRogueArmor = rogue->GetArmor() - 610;

			// Faerie Fire 
            druid->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_CAST(druid, rogue, ClassSpells::Druid::FAERIE_FIRE_RNK_5);
            druid->ResetForceSpellHitResult();
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Druid::FAERIE_FIRE_RNK_5, Seconds(40));
			ASSERT_INFO("Rogue has %u armor, expected: %i", rogue->GetArmor(), expectedRogueArmor);
			TEST_ASSERT(rogue->GetArmor() == expectedRogueArmor);
			Wait(2000);

			// Rogue can't stealth
            TEST_CAST(rogue, rogue, ClassSpells::Rogue::STEALTH_RNK_4, SPELL_FAILED_CASTER_AURASTATE);

			// Mage can't invisible
			uint32 expectedFaerieFireMana = 145;
			TEST_POWER_COST(druid, mage, ClassSpells::Druid::FAERIE_FIRE_RNK_5, POWER_MANA, expectedFaerieFireMana);
            TEST_CAST(mage, mage, ClassSpells::Mage::INVISIBILITY_RNK_1, SPELL_FAILED_CASTER_AURASTATE);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FaerieFireTestImpt>();
	}
};

class HibernateTest : public TestCaseScript
{
public:

	HibernateTest() : TestCaseScript("spells druid hibernate") { }

	class HibernateTestImpt : public TestCase
	{
	public:
		HibernateTestImpt() : TestCase(STATUS_PASSING) { }

		void HibernateDuration(TestPlayer* druid, Unit* enemy, Milliseconds durationMS)
		{
            druid->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_CAST(druid, enemy, ClassSpells::Druid::HIBERNATE_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            druid->ResetForceSpellHitResult();
            TEST_AURA_MAX_DURATION(enemy, ClassSpells::Druid::HIBERNATE_RNK_3, Milliseconds(durationMS));
			enemy->RemoveAurasDueToSpell(ClassSpells::Druid::HIBERNATE_RNK_3);
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 20.0f);
			Creature* beast = SpawnCreature(21408, true); // Felfire Diemetradon
			Creature* dragonkin = SpawnCreature(21722, true); // Enslaved Netherwing Drake

			// PvE
            HibernateDuration(druid, beast, Milliseconds(40000));

			// Only one target at a time
            HibernateDuration(druid, dragonkin, Milliseconds(40000));
            TEST_HAS_NOT_AURA(beast, ClassSpells::Druid::HIBERNATE_RNK_3);

			// PvP
			TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);
			TEST_CAST(enemy, enemy, ClassSpells::Druid::CAT_FORM_RNK_1);
			Wait(1000);
			// Diminishing return
			HibernateDuration(druid, enemy, Milliseconds(10000)); // 10s
			HibernateDuration(druid, enemy, Milliseconds(5000)); // 5s
			HibernateDuration(druid, enemy, Milliseconds(2500)); // 2.5s
			// Immune
			uint32 expectedHibernateMana = 150;
			TEST_POWER_COST(druid, enemy, ClassSpells::Druid::HIBERNATE_RNK_3, POWER_MANA, expectedHibernateMana);
            TEST_HAS_NOT_AURA(enemy, ClassSpells::Druid::HIBERNATE_RNK_3);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<HibernateTestImpt>();
	}
};

class HurricaneTest : public TestCaseScript
{
public:
	HurricaneTest() : TestCaseScript("spells druid hurricane") { }

	class HurricaneTestImpt : public TestCase
	{
	public:
		HurricaneTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
            int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			druid->DisableRegeneration(true);
			uint32 expectedRogueAttackSpeed = rogue->GetAttackTimer(BASE_ATTACK) * 1.25f;

            // Mana cost
            uint32 const expectedHurricaneMana = 1905;
            TEST_POWER_COST(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, POWER_MANA, expectedHurricaneMana);

            // Cooldown
            TEST_COOLDOWN(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, Minutes(1));

            // Duration: TODO: any better way to check it?
			SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ClassSpells::Druid::HURRICANE_RNK_4);
			TEST_ASSERT(spellInfo != nullptr);
			TEST_ASSERT(spellInfo->GetDuration() == 10 * SECOND *  IN_MILLISECONDS);

            // 25% AS
			TEST_ASSERT(rogue->GetAttackTimer(BASE_ATTACK) == expectedRogueAttackSpeed);

			// Damage
			float const hurricaneSpellCoeff = 10.0f / 3.5f / 2.0f / 10.0f;
			uint32 const hurricaneBonusSP = hurricaneSpellCoeff * staffSP;
			uint32 const expectedHurricaneDmg = ClassSpellsDamage::Druid::HURRICANE_RNK_4_TICK + hurricaneBonusSP;
			TEST_CHANNEL_DAMAGE(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, ClassSpells::Druid::HURRICANE_RNK_4_PROC, 10, expectedHurricaneDmg);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<HurricaneTestImpt>();
	}
};

class InnervateTest : public TestCaseScript
{
public:
	InnervateTest() : TestCaseScript("spells druid innervate") { }

	class InnervateTestImpt : public TestCase
	{
	public:
		InnervateTestImpt() : TestCase(STATUS_PASSING) { }

		float getSpiritRatioBasedRegen(TestPlayer* player, float spiritFactor = 1.0f)
		{
			uint32 level = player->GetLevel();
			uint32 pclass = player->GetClass();

			if (level>GT_MAX_LEVEL)
				level = GT_MAX_LEVEL;

			GtRegenMPPerSptEntry const *moreRatio = sGtRegenMPPerSptStore.LookupEntry((pclass - 1) * GT_MAX_LEVEL + level - 1);
			TEST_ASSERT(moreRatio != nullptr);

			float const spirit = player->GetStat(STAT_SPIRIT) * spiritFactor;
			float const regen = spirit * moreRatio->ratio;
			return regen;
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            float const innervateSpiritFactor = 5.0f;
            float const regenPerSecondWithInnervate = sqrt(druid->GetStat(STAT_INTELLECT)) * getSpiritRatioBasedRegen(druid, innervateSpiritFactor);

			// Power cost
			uint32 expectedInnervateManaCost = 94;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::INNERVATE_RNK_1, POWER_MANA, expectedInnervateManaCost);

			// Duration & CD
            TEST_CAST(druid, druid, ClassSpells::Druid::INNERVATE_RNK_1);
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::INNERVATE_RNK_1, Seconds(20));
			TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::INNERVATE_RNK_1, Minutes(6));

            // Mana regen
            TEST_ASSERT(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) == regenPerSecondWithInnervate);
            TEST_ASSERT(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) == regenPerSecondWithInnervate);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<InnervateTestImpt>();
	}
};

class MoonfireTest : public TestCaseScript
{
public:
	MoonfireTest() : TestCaseScript("spells druid moonfire") { }

	class MoonfireTestImpt : public TestCase
	{
	public:
		MoonfireTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedMoonfireManaCost = 495;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, POWER_MANA, expectedMoonfireManaCost);

			// Spell coefficient
			float const moonfireCastTimeDuration = 1.5f; // GCD
			float const moonfireDoTDuration = 12.0f;
			float const moonfireDoTSpellCoeff = (moonfireDoTDuration / 15.0f) / ((moonfireDoTDuration / 15.0f) + (moonfireCastTimeDuration / 3.5f));
			float const moonfireDirectSpellCoeff = 1 - moonfireDoTSpellCoeff;

			uint32 const moonfireDirectBonusSP	= moonfireCastTimeDuration / 3.5f * moonfireDirectSpellCoeff * staffSP;
			uint32 const moonfireDoTBonusSP		= moonfireDoTDuration / 15.0f * moonfireDoTSpellCoeff * staffSP;

			// Direct Damage
			uint32 const expectedMoonfireMinDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MIN + moonfireDirectBonusSP;
			uint32 const expectedMoonfireMaxDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_MAX + moonfireDirectBonusSP;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireMinDmg, expectedMoonfireMaxDmg, false);

			// Direct Damage Crit
			uint32 const expectedMoonfireMinCrit = expectedMoonfireMinDmg * 1.5f;
			uint32 const expectedMoonfireMaxCrit = expectedMoonfireMaxDmg * 1.5f;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireMinCrit, expectedMoonfireMaxCrit, true);

			// DoT
			uint32 const expectedMoonfireTotalDmg = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_TICK + moonfireDoTBonusSP;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireTotalDmg, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MoonfireTestImpt>();
	}
};

class StarfireTest : public TestCaseScript
{
public:
	StarfireTest() : TestCaseScript("spells druid starfire") { }

	class StarfireTestImpt : public TestCase
	{
	public:
		StarfireTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

            int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedStarfireMana = 370;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::STARFIRE_RNK_8, POWER_MANA, expectedStarfireMana);

			// Spell coefficient
			float const starfireSpellCoeff = ClassSpellsCoeff::Druid::STARFIRE;
			uint32 const starfireBonusSP = starfireSpellCoeff * staffSP;

			// Damage
			uint32 const expectedStarfireMinDmg = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN + starfireBonusSP;
			uint32 const expectedStarfireMaxDmg = ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX + starfireBonusSP;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDmg, expectedStarfireMaxDmg, false);

			// Crit damage
			uint32 const expectedStarfireCritMinDmg = expectedStarfireMinDmg * 1.5f;
			uint32 const expectedStarfireCritMaxDmg = expectedStarfireMaxDmg * 1.5f;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireCritMinDmg, expectedStarfireCritMaxDmg, true);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<StarfireTestImpt>();
	}
};

class ThornsTest : public TestCaseScript
{
public:
	ThornsTest() : TestCaseScript("spells druid thorns") { }

	class ThornsTestImpt : public TestCase
	{
	public:
		ThornsTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            _location.MoveInFront(_location, 3.0f);
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            druid->ForceSpellHitResult(SPELL_MISS_NONE); //Thorns can miss

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);
            rogue->DisableRegeneration(true);

			int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

            TEST_CAST(druid, druid, ClassSpells::Druid::THORNS_RNK_7);
			// Damage 
			uint32 const thornsDmg = 25;
            uint32 previousRogueHealth = rogue->GetHealth();
            TEST_ASSERT(rogue->IsFullHealth());
            rogue->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            rogue->AttackerStateUpdate(druid, BASE_ATTACK);
			rogue->AttackStop();
            rogue->ResetForceMeleeHitResult();
            uint32 const doneDamage = previousRogueHealth - rogue->GetHealth();
            ASSERT_INFO("doneDamage %u == thornsDmg %u", doneDamage, thornsDmg);
			TEST_ASSERT(doneDamage == thornsDmg);

            // Mana cost
            uint32 const expectedThornsMana = 400;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::THORNS_RNK_7, POWER_MANA, expectedThornsMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ThornsTestImpt>();
	}
};

class WrathTest : public TestCaseScript
{
public:
	WrathTest() : TestCaseScript("spells druid wrath") { }

	class WrathTestImpt : public TestCase
	{
	public:
		WrathTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummy = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

            int32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedWrathMana = 255;
			TEST_POWER_COST(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, POWER_MANA, expectedWrathMana);

			// Spell coefficient
			float const wrathSpellCoeff = ClassSpellsCoeff::Druid::WRATH;
			uint32 const wrathBonusSP = wrathSpellCoeff * staffSP;

			// Damage
			uint32 const expectedWrathMinDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN + wrathBonusSP;
			uint32 const expectedWrathMaxDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX + wrathBonusSP;
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDmg, expectedWrathMaxDmg, false);

			// Crit damage
			uint32 const expectedWrathCritMinDmg = expectedWrathMinDmg * 1.5f;
			uint32 const expectedWrathCritMaxDmg = expectedWrathMaxDmg * 1.5f;
			TEST_DIRECT_SPELL_DAMAGE(druid, dummy, ClassSpells::Druid::WRATH_RNK_10, expectedWrathCritMinDmg, expectedWrathCritMaxDmg, true);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<WrathTestImpt>();
	}
};

class BashTest : public TestCaseScript
{
public:

	BashTest() : TestCaseScript("spells druid bash") { }

	class BashTestImpt : public TestCase
	{
	public:
		BashTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);

            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(1500); // GCD

			// Rage & aura duration
			uint32 const expectedBashRageCost = 10 * 10;
            TEST_POWER_COST(druid, rogue, ClassSpells::Druid::BASH_RNK_3, POWER_RAGE, expectedBashRageCost);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Druid::BASH_RNK_3, Seconds(4));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<BashTestImpt>();
	}
};

class ChallengingRoarTest : public TestCaseScript
{
public:

	ChallengingRoarTest() : TestCaseScript("spells druid challenging_roar") { }

	class ChallengingRoarTestImpt : public TestCase
	{
	public:
		ChallengingRoarTestImpt() : TestCase(STATUS_PARTIAL) { }

		void Test() override
		{
            uint32 const KOBOLD_VERMIN = 6;
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawn3m(_location);
			spawn3m.MoveInFront(_location, 3.0f);
			TestPlayer* player3m = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 1, spawn3m);
			Creature* creature3m = SpawnCreatureWithPosition(spawn3m, KOBOLD_VERMIN);

			Position spawn6m(_location);
			spawn6m.MoveInFront(_location, 6.0f);
			TestPlayer* player6m = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 1, spawn6m);
			Creature* creature6m = SpawnCreatureWithPosition(spawn6m, KOBOLD_VERMIN);

			Position spawn15m(_location);
			spawn15m.MoveInFront(_location, 15.0f);
			TestPlayer* player15m = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 1, spawn15m);
			Creature* creature15m = SpawnCreatureWithPosition(spawn15m, KOBOLD_VERMIN);

			// Setup
            creature3m->GetThreatManager().AddThreat(player3m, 1000);
            creature6m->GetThreatManager().AddThreat(player6m, 1000);
            creature15m->GetThreatManager().AddThreat(player15m, 1000);
            Wait(1000);
            TEST_ASSERT(creature3m->GetVictim() == player3m);
            TEST_ASSERT(creature6m->GetVictim() == player6m);
            TEST_ASSERT(creature15m->GetVictim() == player15m);

			TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(2000);

            Wait(10000);

            druid->SetFullPower(POWER_RAGE);
            druid->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_CAST(druid, druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1);

            // Aura
            TEST_AURA_MAX_DURATION(creature3m, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, Seconds(6));
            TEST_AURA_MAX_DURATION(creature6m, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, Seconds(6));
            TEST_HAS_NOT_AURA(creature15m, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1);
            Wait(1);
		
			// Target changed
			TEST_ASSERT(creature3m->GetVictim() == druid);
			TEST_ASSERT(creature6m->GetVictim() == druid);

			// Back to original target
			Wait(8000);
			TEST_ASSERT(creature3m->GetVictim() == player3m);
			TEST_ASSERT(creature6m->GetVictim() == player6m);

            //TODO: test base threat generated by challenging roar

            // Rage cost
            uint32 const expectedChallengingRoarRage = 15 * 10;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, POWER_RAGE, expectedChallengingRoarRage);
            TEST_COOLDOWN(druid, druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, Minutes(10));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ChallengingRoarTestImpt>();
	}
};

class ClawTest : public TestCaseScript
{
public:

	ClawTest() : TestCaseScript("spells druid claw") { }

	class ClawTestImpt : public TestCase
	{
	public:
		ClawTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1);
			Wait(1500); // GCD

			// Energy cost
			uint32 const expectedClawEnergy = 45;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::CLAW_RNK_6, POWER_ENERGY, expectedClawEnergy);

			// Combo point added
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Damage
			uint32 const level = 60;
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const catAttackSpeed = 1.0f;
			uint32 const catMinBaseDamage = 14 + level * 0.5f;
			uint32 const catMaxBaseDamage = catMinBaseDamage * 1.5f;
			uint32 const weaponMinDamage = (catMinBaseDamage + AP / 14 + ClassSpellsDamage::Druid::CLAW_RNK_6) * catAttackSpeed;
			uint32 const weaponMaxDamage = (catMaxBaseDamage + AP / 14 + ClassSpellsDamage::Druid::CLAW_RNK_6) * catAttackSpeed;
			uint32 const expectedClawMin = weaponMinDamage * armorFactor;
			uint32 const expectedClawMax = weaponMaxDamage * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::CLAW_RNK_6, expectedClawMin, expectedClawMax, false);

			uint32 const expectedClawCritMin = weaponMinDamage * 2.0f * armorFactor;
			uint32 const expectedClawCritMax = weaponMaxDamage * 2.0f * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::CLAW_RNK_6, expectedClawCritMin, expectedClawCritMax, true);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ClawTestImpt>();
	}
};

class CowerTest : public TestCaseScript
{
public:

	CowerTest() : TestCaseScript("spells druid cower") { }

	class CowerTestImpt : public TestCase
	{
	public:
		CowerTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1);
            Wait(2000);
            //generate some threat
            druid->ForceMeleeHitResult(MELEE_HIT_NORMAL);
            for (int i = 0; i < 50; i++)
                druid->AttackerStateUpdate(creature, BASE_ATTACK);
			druid->AttackStop();
            druid->ResetForceMeleeHitResult();

            //Test threat
            uint32 cowerPoints = 1170;
            float const threat = creature->GetThreatManager().GetThreat(druid);
            ASSERT_INFO("Druids needs to have higher threat than %u to test spell", threat);
            TEST_ASSERT(threat > cowerPoints);
            float const expectedThreat = threat - cowerPoints;
            TEST_CAST(druid, creature, ClassSpells::Druid::COWER_RNK_5);
			ASSERT_INFO("Before: %f, current: %f, expected: %f", threat, creature->GetThreatManager().GetThreat(druid), expectedThreat);
			TEST_ASSERT(Between(creature->GetThreatManager().GetThreat(druid), expectedThreat - 1, expectedThreat + 1));

            //Test CD
			TEST_COOLDOWN(druid, creature, ClassSpells::Druid::COWER_RNK_5, Seconds(10));

            //Test power cost
            uint32 expectedCowerEnergy = 20;
            TEST_POWER_COST(druid, creature, ClassSpells::Druid::COWER_RNK_5, POWER_ENERGY, expectedCowerEnergy);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<CowerTestImpt>();
	}
};

class DemoralizingRoarTest : public TestCaseScript
{
public:

	DemoralizingRoarTest() : TestCaseScript("spells druid demoralizing_roar") { }

	class DemoralizingRoarTestImpt : public TestCase
	{
	public:
		DemoralizingRoarTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawn3m(_location);
			spawn3m.MoveInFront(_location, 3.0f);
			TestPlayer* player3m = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 1, spawn3m);

			Position spawn6m(_location);
			spawn6m.MoveInFront(_location, 6.0f);
			Creature* creature6m = SpawnCreatureWithPosition(spawn6m, 6);

			Position spawn15m(_location);
			spawn15m.MoveInFront(_location, 15.0f);
			Creature* creature15m = SpawnCreatureWithPosition(spawn15m, 6);

			int32 startAP3m = player3m->GetTotalAttackPowerValue(BASE_ATTACK);
			int32 startAP6m = creature6m->GetTotalAttackPowerValue(BASE_ATTACK);
			int32 expectedAP3m = std::max(startAP3m - 240, 0);
			int32 expectedAP6m = std::max(startAP6m - 240, 0);

            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(1500);

			// Rage cost
			uint32 const expectedDemoralizingRoarRage = 10 * 10;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, POWER_RAGE, expectedDemoralizingRoarRage);

			// Aura
            TEST_AURA_MAX_DURATION(player3m, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, Seconds(30));
            TEST_AURA_MAX_DURATION(creature6m, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, Seconds(30));
            TEST_HAS_NOT_AURA(creature15m, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6);

			// AP loss
			TEST_ASSERT(player3m->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP3m);
			TEST_ASSERT(creature6m->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP6m);
			Wait(31000);
			TEST_ASSERT(player3m->GetTotalAttackPowerValue(BASE_ATTACK) == startAP3m);
			TEST_ASSERT(creature6m->GetTotalAttackPowerValue(BASE_ATTACK) == startAP6m);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DemoralizingRoarTestImpt>();
	}
};

class EnrageTest : public TestCaseScript
{
public:

	EnrageTest() : TestCaseScript("spells druid enrage") { }

	class EnrageTestImpt : public TestCase
	{
	public:
		EnrageTestImpt() : TestCase(STATUS_PASSING) { }

        /* WoWWiki: IMPORTANT: this only reduces your base armor, not your total armor. 
        So if you normally have 1000 armour from your leathers, it becomes 840, 
        however you still get the additional 4500 armor from dire bear form, 
        giving you a 5340 rather than 4620. (Normal bear form works the same way).
        */
		void TestEnrage(TestPlayer* druid, uint32 spellFormId, float armorReduction)
		{
            druid->SetPower(POWER_RAGE, 0);
			druid->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(druid, druid, spellFormId, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            uint32 baseArmor = druid->GetFlatModifierValue(UNIT_MOD_ARMOR, BASE_VALUE);    // base armor (from items)
            baseArmor *= druid->GetPctModifierValue(UNIT_MOD_ARMOR, BASE_PCT);             // armor percent from items
            uint32 armorDiff = baseArmor - (baseArmor * (1 - armorReduction)); //how much armor we should loose from items
            uint32 expectedArmor = druid->GetArmor() - armorDiff;

            TEST_CAST(druid, druid, ClassSpells::Druid::ENRAGE_RNK_1);
			ASSERT_INFO("Form: %u, armor: %u, expected: %u", spellFormId, druid->GetArmor(), expectedArmor);
            TEST_ASSERT(Between(druid->GetArmor(), expectedArmor - 1, expectedArmor + 1));
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::ENRAGE_RNK_1, Minutes(1));
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::ENRAGE_RNK_1, Seconds(10));

            //we cant really test per tick values so, just check if rage is increased
			Wait(2000);
			TEST_ASSERT(druid->GetPower(POWER_RAGE) > 0);
			druid->RemoveAurasDueToSpell(spellFormId);
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            EQUIP_ITEM(druid, 31042); // "Thunderheart Chestguard" (T6 torso)  // give him some more armor

			TestEnrage(druid, ClassSpells::Druid::BEAR_FORM_RNK_1, 0.27f);
			TestEnrage(druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, 0.16f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EnrageTestImpt>();
	}
};

class FerociousBiteTest : public TestCaseScript
{
public:

    FerociousBiteTest() : TestCaseScript("spells druid ferocious_bite") { }

    class FerociousBiteTestImpt : public TestCase
    {
    public:
        FerociousBiteTestImpt() : TestCase(STATUS_INCOMPLETE) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            Creature* creature = SpawnCreature();

            EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

            // Must be in Cat Form
            TEST_CAST(druid, creature, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            // Need combo point
            TEST_CAST(druid, creature, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, SPELL_FAILED_CASTER_AURASTATE);
            druid->AddComboPoints(creature, 1);

            // Energy cost
            uint32 const expectedFerociousBiteEnergy = 35;
            TEST_POWER_COST(druid, creature, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, POWER_ENERGY, expectedFerociousBiteEnergy);

            // Damages
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            float APperCP = 0.03f;

            for (uint32 comboPoints = 1; comboPoints < 6; comboPoints++)
            {
                uint32 const energyAfterFB = 65;
                float const dmgPerRemainingEnergy = 4.1f;

                uint32 cpDmgMin = ClassSpellsDamage::Druid::FEROCIOUS_BITE_RNK_6_CP_1_MIN;
                uint32 cpDmgMax = ClassSpellsDamage::Druid::FEROCIOUS_BITE_RNK_6_CP_1_MAX;

                uint32 minFerociousBiteDmg = (AP * comboPoints * APperCP + cpDmgMin + dmgPerRemainingEnergy * energyAfterFB) * armorFactor;
                uint32 maxFerociousBiteDmg = (AP * comboPoints * APperCP + cpDmgMax + dmgPerRemainingEnergy * energyAfterFB) * armorFactor;

                uint32 sampleSize;
                uint32 absoluteAllowedError;
                _GetApproximationParams(sampleSize, absoluteAllowedError, minFerociousBiteDmg, maxFerociousBiteDmg);

                auto AI = druid->GetTestingPlayerbotAI();
                uint32 minDamage = std::numeric_limits<uint32>::max();
                uint32 maxDamage = 0;
                druid->ForceMeleeHitResult(MELEE_HIT_NORMAL);
                for (uint32 i = 0; i < sampleSize; i++)
                {
                    druid->AddComboPoints(creature, comboPoints);
                    druid->SetPower(POWER_ENERGY, 100 * 10);
                    FORCE_CAST(druid, creature, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, SPELL_MISS_NONE);

                    auto damageToTarget = AI->GetMeleeDamageDoneInfo(creature);
                    TEST_ASSERT(damageToTarget->size() == i + 1);
                    auto& data = damageToTarget->back();

                    uint32 damage = data.damageInfo.Damages[0].Damage;

                    minDamage = std::min(minDamage, damage);
                    maxDamage = std::max(maxDamage, damage);
                }
                druid->ForceMeleeHitResult(MELEE_HIT_MISS);

                uint32 allowedMin = minFerociousBiteDmg > absoluteAllowedError ? minFerociousBiteDmg - absoluteAllowedError : 0; //protect against underflow
                uint32 allowedMax = maxFerociousBiteDmg + absoluteAllowedError;

                TEST_ASSERT(maxFerociousBiteDmg <= allowedMax);
                TEST_ASSERT(minFerociousBiteDmg >= allowedMin);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FerociousBiteTestImpt>();
    }
};

class FrenziedRegenerationTest : public TestCaseScript
{
public:

	FrenziedRegenerationTest() : TestCaseScript("spells druid frenzied_regeneration") { }

	class FrenziedRegenerationTestImpt : public TestCase
	{
	public:
		FrenziedRegenerationTestImpt() : TestCase(STATUS_PASSING) { }

        void TestValue(TestPlayer* druid, uint32 expected, bool crit)
        {
            auto AI = druid->GetTestingPlayerbotAI();
            TEST_ASSERT(AI != nullptr);
            AI->ResetSpellCounters();

			druid->GetSpellHistory()->ResetAllCooldowns();
            druid->SetPower(POWER_RAGE, 100 * 10);
            EnableCriticals(druid, crit);
            TEST_CAST(druid, druid, ClassSpells::Druid::FRENZIED_REGENERATION_RNK_4, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            Wait(11000); //wait for all ticks
            uint32 dealtMin, dealtMax;
            //FR actually triggers this healing spell at each tick
            GetHealingPerSpellsTo(druid, druid, ClassSpells::Druid::FRENZIED_REGENERATION_TRIGGER, dealtMin, dealtMax, crit, 10);
            TEST_ASSERT(dealtMin == dealtMax);
            TEST_ASSERT(expected == dealtMin);
        }
         
		void TestFrenziedRegeneration(TestPlayer* druid, uint32 spellFormId)
		{
            druid->GetSpellHistory()->ResetAllCooldowns();
            druid->DisableRegeneration(true); //also disable rage regen and decaying
            //setup
            TEST_CAST(druid, druid, spellFormId, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            druid->SetPower(POWER_RAGE, 100 * 10); //max rage

            //test heal values
            //WoWiki: Rank 4 : Each point of Rage is converted into 25 health. [...] Unaffected by +healing stats.
            // -> 250 at each tick, and it can crit
			uint32 expectedFRHeal = 10 * 25;
			uint32 expectedFRCritHeal = expectedFRHeal * 1.5f;

            TestValue(druid, expectedFRHeal, false);
            TestValue(druid, expectedFRCritHeal, true);

            //test if rage is properly removed
            //1000 rage, we should have 900 when spell ends (10 ticks at 10 rage)
            druid->SetMaxPower(POWER_RAGE, 1000 * 10);
            druid->SetFullPower(POWER_RAGE);
            druid->GetSpellHistory()->ResetAllCooldowns();
            TEST_CAST(druid, druid, ClassSpells::Druid::FRENZIED_REGENERATION_RNK_4);
            //also test CD and duration
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::FRENZIED_REGENERATION_RNK_4, Seconds(10));
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::FRENZIED_REGENERATION_RNK_4, Minutes(3));

            Wait(11000); //wait for all ticks
            TEST_ASSERT(druid->GetPower(POWER_RAGE) == 900 * 10);

			druid->RemoveAurasDueToSpell(spellFormId);
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			druid->DisableRegeneration(true);

			TestFrenziedRegeneration(druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			TestFrenziedRegeneration(druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<FrenziedRegenerationTestImpt>();
	}
};

class GrowlTest : public TestCaseScript
{
public:

	GrowlTest() : TestCaseScript("spells druid growl") { }

	class GrowlTestImpt : public TestCase
	{
	public:
		GrowlTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawn3m(_location);
			spawn3m.MoveInFront(_location, 3.0f);
			TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawn3m);
			Creature* creature = SpawnCreatureWithPosition(spawn3m, 6);

			// Setup
			creature->SetMaxHealth(10000);
			creature->SetHealth(10000);
            warlock->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_CAST(warlock, creature, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			Wait(500);
			uint32 warlockThreat = creature->GetThreatManager().GetThreat(warlock);
			TEST_ASSERT(warlockThreat > 0);
			TEST_ASSERT(creature->GetTarget() == warlock->GetGUID());

			// Acquire threat, aura duration, cooldown
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            druid->ForceSpellHitResult(SPELL_MISS_NONE);
            TEST_CAST(druid, creature, ClassSpells::Druid::GROWL_RNK_1);
            druid->ResetForceSpellHitResult();
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::GROWL_RNK_1, Seconds(3));
			TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::GROWL_RNK_1, Seconds(10));
			TEST_ASSERT(creature->GetThreatManager().GetThreat(druid) == warlockThreat);

			// Keep aggro
			Wait(1000);
            TEST_CAST(warlock, creature, ClassSpells::Warlock::SHADOW_BOLT_RNK_11, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            warlock->ResetForceSpellHitResult();
			Wait(500);
			warlockThreat = creature->GetThreatManager().GetThreat(warlock);
			TEST_ASSERT(warlockThreat > creature->GetThreatManager().GetThreat(druid));
			TEST_ASSERT(creature->GetTarget() == druid->GetGUID());

			// Lose aggro
			Wait(2000);
			TEST_ASSERT(creature->GetTarget() == warlock->GetGUID());
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<GrowlTestImpt>();
	}
};

class LacerateTest : public TestCaseScript
{
public:

	LacerateTest() : TestCaseScript("spells druid lacerate") { }

	class LacerateTestImpt : public TestCase
	{
	public:
		LacerateTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			creature->DisableRegeneration(true);
			druid->DisableRegeneration(true);
			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

			// Rage cost
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			uint32 const expectedLacerateRage = 15 * 10;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::LACERATE_RNK_1, POWER_RAGE, expectedLacerateRage);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);

			// Direct damage (always fixed damage, no ap bonus)
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::LACERATE_RNK_1, ClassSpellsDamage::Druid::LACERATE_RNK_1, ClassSpellsDamage::Druid::LACERATE_RNK_1, false);
            creature->RemoveAllAuras();

			// Bleed damage
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			uint32 const apBonus = AP * 0.05f;

			creature->RemoveAurasDueToSpell(ClassSpells::Druid::LACERATE_RNK_1);
			creature->SetHealth(10000);
            druid->ForceSpellHitResult(SPELL_MISS_NONE);
            for (uint32 lacerateStack = 1; lacerateStack <= 7; lacerateStack++)
            {
                TEST_CAST(druid, creature, ClassSpells::Druid::LACERATE_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
                TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::LACERATE_RNK_1, Seconds(15));
            }
            //we casted 7 times but stacks should only go to 5
            TEST_AURA_STACK(creature, ClassSpells::Druid::LACERATE_RNK_1, 5);

            // test damage with all stacks. Target has currently 5 stacks
            int32 const totalDotDamage = floor(ClassSpellsDamage::Druid::LACERATE_RNK_1_BLEED + apBonus) * 5;
            auto AI = druid->GetTestingPlayerbotAI();
            TEST_ASSERT(AI != nullptr);
            AI->ResetSpellCounters();
            Wait(16000); // wait all ticks to finish

            int32 doneToTarget = AI->GetDotDamage(creature, ClassSpells::Druid::LACERATE_RNK_1);
            TEST_ASSERT(Between(doneToTarget, totalDotDamage - 1, totalDotDamage + 1));

            druid->ResetForceSpellHitResult();
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<LacerateTestImpt>();
	}
};

class MaulTest : public TestCaseScript
{
public:

	MaulTest() : TestCaseScript("spells druid maul") { }

	class MaulTestImpt : public TestCase
	{
	public:
		MaulTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(1500); // GCD

			// Rage cost
			uint32 const expectedMaulRage = 15 * 10;
            druid->AttackerStateUpdate(creature, BASE_ATTACK);
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::MAUL_RNK_8, POWER_RAGE, expectedMaulRage);
			druid->AttackStop();

			// Shapeshift
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2);
			Wait(1500); // GCD

			// Damage
			uint32 const level = 60;
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const bearAttackSpeed = 2.5f;
			uint32 const bearMinBaseDamage = 14 + level * 0.5f;
			uint32 const bearMaxBaseDamage = bearMinBaseDamage * 1.5f;
			uint32 const weaponMinDamage = (bearMinBaseDamage + AP / 14) * bearAttackSpeed + ClassSpellsDamage::Druid::MAUL_RNK_8;
            uint32 const weaponMaxDamage = (bearMaxBaseDamage + AP / 14) * bearAttackSpeed + ClassSpellsDamage::Druid::MAUL_RNK_8;
			uint32 const expectedMaulMin = weaponMinDamage * armorFactor;
			uint32 const expectedMaulMax = weaponMaxDamage * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::MAUL_RNK_8, expectedMaulMin, expectedMaulMax, false);

			uint32 const expectedMaulCritMin = weaponMinDamage * 2.0f * armorFactor;
			uint32 const expectedMaulCritMax = weaponMaxDamage * 2.0f * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::MAUL_RNK_8, expectedMaulCritMin, expectedMaulCritMax, true);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<MaulTestImpt>();
	}
};

class PounceTest : public TestCaseScript
{
public:

	PounceTest() : TestCaseScript("spells druid pounce") { }

	class PounceTestImpt : public TestCase
	{
	public:
		PounceTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

			// Must be in Cat Form
            TEST_CAST(druid, creature, ClassSpells::Druid::POUNCE_RNK_1, SPELL_FAILED_ONLY_SHAPESHIFT);

			// Only castable stealthed
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_CAST(druid, creature, ClassSpells::Druid::POUNCE_RNK_1, SPELL_FAILED_ONLY_STEALTHED);
            TEST_CAST(druid, druid, ClassSpells::Druid::PROWL_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			
			// Energy cost
			uint32 const expectedPounceEnergy = 50;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::POUNCE_RNK_4, POWER_ENERGY, expectedPounceEnergy);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::POUNCE_RNK_4, Seconds(3));
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::POUNCE_RNK_4_PROC, Seconds(18));

			// Combo
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Damage -- bug here, not taking ap coeff
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const pounceBleedCoeff = 0.18f;
			uint32 const pounceBleedTotal = ClassSpellsDamage::Druid::POUNCE_RNK_4_TOTAL + AP * pounceBleedCoeff;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::POUNCE_RNK_4_PROC, pounceBleedTotal, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<PounceTestImpt>();
	}
};

class ProwlTest : public TestCaseScript
{
public:

	ProwlTest() : TestCaseScript("spells druid prowl") { }

	class ProwlTestImpt : public TestCase
	{
	public:
		ProwlTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			// INFO: stealth to be tested with Stealth Mechanic
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			float const expectedRunSpeed = druid->GetSpeed(MOVE_RUN) * 0.7f;

			// Must be in Cat Form
            TEST_CAST(druid, creature, ClassSpells::Druid::PROWL_RNK_3, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_CAST(druid, druid, ClassSpells::Druid::PROWL_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			TEST_HAS_AURA(druid, ClassSpells::Druid::PROWL_RNK_3);

			// Speed
			TEST_ASSERT(druid->GetSpeed(MOVE_RUN) == expectedRunSpeed);

			// Remove
            druid->AttackerStateUpdate(creature, BASE_ATTACK);
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::PROWL_RNK_3, Seconds(10));
            TEST_HAS_NOT_AURA(druid, ClassSpells::Druid::PROWL_RNK_3);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ProwlTestImpt>();
	}
};

class RakeTest : public TestCaseScript
{
public:

	RakeTest() : TestCaseScript("spells druid rake") { }

	class RakeTestImpt : public TestCase
	{
	public:
		RakeTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

			// Must be in Cat Form
            TEST_CAST(druid, creature, ClassSpells::Druid::RAKE_RNK_5, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

			// Energy cost
			uint32 const expectedRakeEnergy = 40;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::RAKE_RNK_5, POWER_ENERGY, expectedRakeEnergy);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::RAKE_RNK_5, Seconds(9));

			// Combo
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Direct Damage
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			uint32 const rakeDamage = floor(AP / 100 + ClassSpellsDamage::Druid::RAKE_RNK_5);
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::RAKE_RNK_5, rakeDamage, rakeDamage, false);

			// Bleed -- bug here, ap coeff not taken
            float const rakeAPCoeff = 0.06f;
            float const rakeTickCount = 3.0f;
            uint32 const rakeBleedTotal = AP * rakeAPCoeff + ClassSpellsDamage::Druid::RAKE_RNK_5_BLEED;
			uint32 const rakeBleedTick = floor(rakeBleedTotal / rakeTickCount);
			uint32 const expectedRakeBleedDamage = 3 * rakeBleedTick;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeBleedDamage, false);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<RakeTestImpt>();
	}
};

class RavageTest : public TestCaseScript
{
public:

	RavageTest() : TestCaseScript("spells druid ravage") { }

	class RavageTestImpt : public TestCase
	{
	public:
		RavageTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition);
            creature->SetOrientation(creature->GetOrientation() + M_PI);

			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

            Wait(5000);

			// Must be behind, in cat form
            TEST_CAST(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, SPELL_FAILED_NOT_BEHIND);
            creature->SetOrientation(druid->GetOrientation());
            TEST_CAST(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

			// Only castable stealthed
            TEST_CAST(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, SPELL_FAILED_ONLY_STEALTHED);
            TEST_CAST(druid, druid, ClassSpells::Druid::PROWL_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

			// Energy cost
			uint32 const expectedRavageEnergy = 60;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, POWER_ENERGY, expectedRavageEnergy);

			// Combo
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Damage -- damage is too high, calculations below are in agreement with DrDamage
			uint32 const level = 60;
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const catAttackSpeed = 1.0f;
			uint32 const catMinBaseDamage = 14 + level * 0.5f;
			uint32 const catMaxBaseDamage = catMinBaseDamage * 1.5f;
			float const ravageFactor = 3.85f;
			uint32 const weaponMinDamage = (catMinBaseDamage + AP / 14) * ravageFactor * catAttackSpeed + ClassSpellsDamage::Druid::RAVAGE_RNK_5;
            uint32 const weaponMaxDamage = (catMaxBaseDamage + AP / 14) * ravageFactor * catAttackSpeed + ClassSpellsDamage::Druid::RAVAGE_RNK_5;
			uint32 const expectedRavageMin = weaponMinDamage * armorFactor;
			uint32 const expectedRavageMax = weaponMaxDamage * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, expectedRavageMin, expectedRavageMax, false);

			uint32 const expectedRavageCritMin = weaponMinDamage * 2.0f * armorFactor;
			uint32 const expectedRavageCritMax = weaponMaxDamage * 2.0f * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, expectedRavageCritMin, expectedRavageCritMax, true);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<RavageTestImpt>();
	}
};

class ShredTest : public TestCaseScript
{
public:

    ShredTest() : TestCaseScript("spells druid shred") { }

    class ShredTestImpt : public TestCase
    {
    public:
        ShredTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            Position spawnPosition(_location);
            spawnPosition.MoveInFront(_location, 3.0f);
            Creature* creature = SpawnCreatureWithPosition(spawnPosition);
            creature->SetOrientation(creature->GetOrientation() + M_PI);

            EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

            // Must be behind & in cat form
            TEST_CAST(druid, creature, ClassSpells::Druid::SHRED_RNK_7, SPELL_FAILED_NOT_BEHIND);
            creature->SetOrientation(druid->GetOrientation());
            TEST_CAST(druid, creature, ClassSpells::Druid::SHRED_RNK_7, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Energy cost
            uint32 const expectedShredEnergy = 60;
            TEST_POWER_COST(druid, creature, ClassSpells::Druid::SHRED_RNK_7, POWER_ENERGY, expectedShredEnergy);

            // Combo
            TEST_ASSERT(druid->GetComboPoints(creature) == 1);

            // Damage
            uint32 const level = 60;
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            float const catAttackSpeed = 1.0f;
            uint32 const catMinBaseDamage = 14 + level * 0.5f;
            uint32 const catMaxBaseDamage = catMinBaseDamage * 1.5f;
            float const shredFactor = 2.25f;
            uint32 const weaponMinDamage = (catMinBaseDamage + AP / 14) * shredFactor * catAttackSpeed + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const weaponMaxDamage = (catMaxBaseDamage + AP / 14) * shredFactor * catAttackSpeed + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const expectedShredMin = weaponMinDamage * armorFactor;
            uint32 const expectedShredMax = weaponMaxDamage * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::SHRED_RNK_7, expectedShredMin, expectedShredMax, false);

            uint32 const expectedShredCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedShredCritMax = weaponMaxDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::SHRED_RNK_7, expectedShredCritMin, expectedShredCritMax, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<ShredTestImpt>();
    }
};

class SwipeTest : public TestCaseScript
{
public:

    SwipeTest() : TestCaseScript("spells druid swipe") { }

    class SwipeTestImpt : public TestCase
    {
    public:
        SwipeTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void Test() override
        {
            /*
                Bugged: hack in SpellEffects.cpp with 0.08f factor on AP
            */
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            Position spawn3m(_location);
            spawn3m.MoveInFront(_location, 3.0f);
            Creature* creature1 = SpawnCreatureWithPosition(spawn3m);
            Creature* creature2 = SpawnCreatureWithPosition(spawn3m);
            Creature* creature3 = SpawnCreatureWithPosition(spawn3m);

            uint32 const startHealth = creature1->GetHealth();
            TEST_ASSERT(creature2->GetHealth() == startHealth);
            TEST_ASSERT(creature3->GetHealth() == startHealth);

            uint32 const armor = creature1->GetArmor();
            TEST_ASSERT(creature2->GetArmor() == armor);
            TEST_ASSERT(creature3->GetArmor() == armor);

            EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Rage cost
            uint32 const expectedSwipeRage = 20 * 10;
            TEST_POWER_COST(druid, creature1, ClassSpells::Druid::SWIPE_RNK_6, POWER_RAGE, expectedSwipeRage);

            // Shapeshift
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Damage
            uint32 const level = 60;
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (armor / (armor + 10557.5));
            float const swipeFactor = 0.07f;
            uint32 const weaponDamage = ClassSpellsDamage::Druid::SWIPE_RNK_6 + AP * swipeFactor;
            uint32 const expectedSwipe = weaponDamage * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature1, ClassSpells::Druid::SWIPE_RNK_6, expectedSwipe, expectedSwipe, false);

            uint32 const expectedSwipeCrit = weaponDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature1, ClassSpells::Druid::SWIPE_RNK_6, expectedSwipeCrit, expectedSwipeCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<SwipeTestImpt>();
    }
};

class TigersFuryTest : public TestCaseScript
{
public:

    TigersFuryTest() : TestCaseScript("spells druid tigers_fury") { }

    class TigersFuryTestImpt : public TestCase
    {
    public:
        TigersFuryTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            //Creature* inFront = SpawnCreature();

            Position spawnPosition(_location);
            spawnPosition.MoveInFront(_location, 3.0f);
            Creature* creature = SpawnCreatureWithPosition(spawnPosition);

            EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

            // Must be in cat form
            TEST_CAST(druid, creature, ClassSpells::Druid::TIGERS_FURY_RNK_4, SPELL_FAILED_ONLY_SHAPESHIFT);

            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            Wait(2000);
            TEST_CAST(druid, creature, ClassSpells::Druid::TIGERS_FURY_RNK_4);

            // Test cd & aura duration
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::TIGERS_FURY_RNK_4, Seconds(1));
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::TIGERS_FURY_RNK_4, Seconds(6));

            // Damage
            uint32 const level = 60;
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            float const catAttackSpeed = 1.0f;
            uint32 const tigersFuryBonus = 40;
            uint32 const APBonus = AP / 14;
            uint32 const catMinBaseDamage = 14 + level * 0.5f + tigersFuryBonus;
            uint32 const catMaxBaseDamage = catMinBaseDamage * 1.5f;

            uint32 const catMeleeMin = (catMinBaseDamage + APBonus) * armorFactor;
            uint32 const catMeleeMax = (catMaxBaseDamage + APBonus) * armorFactor;
            TEST_MELEE_DAMAGE(druid, creature, BASE_ATTACK, catMeleeMin, catMeleeMax, false);
            druid->AddAura(ClassSpells::Druid::TIGERS_FURY_RNK_4, druid);
            TEST_MELEE_DAMAGE(druid, creature, BASE_ATTACK, catMeleeMin * 2.0f, catMeleeMax * 2.0f, true);

            float const shredFactor = 2.25f;
            uint32 const weaponMinDamage = (catMinBaseDamage + APBonus) * shredFactor * catAttackSpeed + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const weaponMaxDamage = (catMaxBaseDamage + APBonus) * shredFactor * catAttackSpeed + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const expectedTigersFuryMin = weaponMinDamage * armorFactor;
            uint32 const expectedTigersFuryMax = weaponMaxDamage * armorFactor;
            druid->AddAura(ClassSpells::Druid::TIGERS_FURY_RNK_4, druid);
            TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::SHRED_RNK_7, expectedTigersFuryMin, expectedTigersFuryMax, false);

            uint32 const expectedTigersFuryCritMin = weaponMinDamage * 2.0f * armorFactor;
            uint32 const expectedTigersFuryCritMax = weaponMaxDamage * 2.0f * armorFactor;
            druid->AddAura(ClassSpells::Druid::TIGERS_FURY_RNK_4, druid);
            TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::SHRED_RNK_7, expectedTigersFuryCritMin, expectedTigersFuryCritMax, true);

            // Energy cost
            uint32 const expectedTigersFuryEnergy = 30;
            TEST_POWER_COST(druid, creature, ClassSpells::Druid::TIGERS_FURY_RNK_4, POWER_ENERGY, expectedTigersFuryEnergy);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<TigersFuryTestImpt>();
    }
};

class AbolishPoisonTest : public TestCaseScript
{
public:
    AbolishPoisonTest() : TestCaseScript("spells druid abolish_poison") { }

    class AbolishPoisonTestImpt : public TestCase
    {
    public:
        AbolishPoisonTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // setup
            druid->DisableRegeneration(true);
            uint32 const WOUND_POISON_V = 27189; // 15s
            uint32 const MIND_NUMBING_POISON_III = 11398; // 14s
            uint32 const DEADLY_POISON_VII = 27187; // 12s
            warrior->AddAura(WOUND_POISON_V, warrior);
            Wait(1);
            warrior->AddAura(MIND_NUMBING_POISON_III, warrior);
            Wait(1);
            warrior->AddAura(DEADLY_POISON_VII, warrior);
            Wait(1);

            // Mana cost
            uint32 const expectedAbolishPoisonMana = 308;
            TEST_POWER_COST(druid, warrior, ClassSpells::Druid::ABOLISH_POISON_RNK_1, POWER_MANA, expectedAbolishPoisonMana);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Druid::ABOLISH_POISON_RNK_1, Seconds(8));

            Wait(4500);
            TEST_HAS_NOT_AURA(warrior, WOUND_POISON_V);
            TEST_HAS_NOT_AURA(warrior, MIND_NUMBING_POISON_III);
            TEST_HAS_NOT_AURA(warrior, DEADLY_POISON_VII);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<AbolishPoisonTestImpt>();
    }
};

class CurePoisonTest : public TestCaseScript
{
public:
    CurePoisonTest() : TestCaseScript("spells druid cure_poison") { }

    class CurePoisonTestImpt : public TestCase
    {
    public:
        CurePoisonTestImpt() : TestCase(STATUS_PASSING) { }

        uint32 getPoisonCount(TestPlayer* victim, uint32 poison1, uint32 poison2, uint32 poison3, bool cast = false)
        {
            uint32 count = 0;
            if (cast)
                TEST_CAST(victim, victim, ClassSpells::Druid::CURE_POISON_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            if (victim->HasAura(poison1))
                count++;
            if (victim->HasAura(poison2))
                count++;
            if (victim->HasAura(poison3))
                count++;
            return count;
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // No poison
            TEST_CAST(druid, warrior, ClassSpells::Druid::CURE_POISON_RNK_1, SPELL_FAILED_NOTHING_TO_DISPEL);

            // setup
            druid->DisableRegeneration(true);
            uint32 const WOUND_POISON_V = 27189; // 15s
            uint32 const MIND_NUMBING_POISON_III = 11398; // 14s
            uint32 const DEADLY_POISON_VII = 27187; // 12s
            warrior->AddAura(WOUND_POISON_V, warrior);
            Wait(1);
            warrior->AddAura(MIND_NUMBING_POISON_III, warrior);
            Wait(1);
            warrior->AddAura(DEADLY_POISON_VII, warrior);
            Wait(1);

            uint32 const expectedCurePoisonMana = 308;
            TEST_POWER_COST(druid, warrior, ClassSpells::Druid::CURE_POISON_RNK_1, POWER_MANA, expectedCurePoisonMana);
            TEST_ASSERT(2 == getPoisonCount(warrior, WOUND_POISON_V, MIND_NUMBING_POISON_III, DEADLY_POISON_VII));
            TEST_ASSERT(1 == getPoisonCount(warrior, WOUND_POISON_V, MIND_NUMBING_POISON_III, DEADLY_POISON_VII, true));
            TEST_ASSERT(0 == getPoisonCount(warrior, WOUND_POISON_V, MIND_NUMBING_POISON_III, DEADLY_POISON_VII, true));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CurePoisonTestImpt>();
    }
};

class GiftOfTheWildTest : public TestCaseScript
{
public:
    GiftOfTheWildTest() : TestCaseScript("spells druid gift_of_the_wild") { }

    class GiftOfTheWildTestImpt : public TestCase
    {
    public:
        GiftOfTheWildTestImpt() : TestCase(STATUS_PASSING) { }

        void TestOfTheWild(TestPlayer* caster, TestPlayer* victim, uint32 spellId, uint32 manaCost, uint8 statBonus, uint8 resistanceBonus, uint16 armorBonus, uint32 reagentId)
        {
            uint32 const startArmor = victim->GetArmor();
            uint32 const expectedArmor      = victim->GetArmor() + armorBonus + statBonus * 2; //also add armor related to agility increase
            uint32 const expectedAgi        = victim->GetStat(STAT_AGILITY) + statBonus;
            uint32 const expectedInt        = victim->GetStat(STAT_INTELLECT) + statBonus;
            uint32 const expectedSpi        = victim->GetStat(STAT_SPIRIT) + statBonus;
            uint32 const expectedSta        = victim->GetStat(STAT_STAMINA) + statBonus;
            uint32 const expectedStr        = victim->GetStat(STAT_STRENGTH) + statBonus;
            uint32 const expectedResArcane  = victim->GetResistance(SPELL_SCHOOL_ARCANE) + resistanceBonus;
            uint32 const expectedResFire    = victim->GetResistance(SPELL_SCHOOL_FIRE) + resistanceBonus;
            uint32 const expectedResFrost   = victim->GetResistance(SPELL_SCHOOL_FROST) + resistanceBonus;
            uint32 const expectedResNature  = victim->GetResistance(SPELL_SCHOOL_NATURE) + resistanceBonus;
            uint32 const expectedResShadow  = victim->GetResistance(SPELL_SCHOOL_SHADOW) + resistanceBonus;

            if (!victim->HasAura(spellId)) {
                caster->AddItem(reagentId, 1);
                TEST_ASSERT(caster->HasItemCount(reagentId, 1, false));
                TEST_POWER_COST(caster, victim, spellId, POWER_MANA, manaCost);
                TEST_ASSERT(caster->GetItemCount(reagentId, false) == 0);
            }

            // Aura duration
            TEST_AURA_MAX_DURATION(victim, spellId, Hours(1));
            TEST_AURA_MAX_DURATION(caster, spellId, Hours(1));

            // Stats, resistances & armor
            ASSERT_INFO("Armor: %u, expected :%u, spell: %u, class: %u, start: %u", victim->GetArmor(), expectedArmor, spellId, victim->GetClass(), startArmor);
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

            // Reset for next test
            victim->RemoveAurasDueToSpell(spellId);
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);

            // Reagents
            uint32 const WILD_BERRIES   = 17021;
            uint32 const WILD_THONROOT  = 17026;
            uint32 const WILD_QUILLVINE = 22148;

            TestOfTheWild(druid, druid, ClassSpells::Druid::GIFT_OF_THE_WILD_RNK_1, 900, 10, 15, 240, WILD_BERRIES);
            TestOfTheWild(druid, druid, ClassSpells::Druid::GIFT_OF_THE_WILD_RNK_2, 1200, 12, 20, 285, WILD_THONROOT);

            // Group
            GroupPlayer(druid, warrior);
            TestOfTheWild(druid, warrior, ClassSpells::Druid::GIFT_OF_THE_WILD_RNK_3, 1515, 14, 25, 340, WILD_QUILLVINE);

        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<GiftOfTheWildTestImpt>();
    }
};

class HealingTouchTest : public TestCaseScript
{
public:
    HealingTouchTest() : TestCaseScript("spells druid healing_touch") { }

    class HealingTouchTestImpt : public TestCase
    {
    public:
        HealingTouchTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
            druid->DisableRegeneration(true);

            int32 maceBH = 550;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedHealingTouchMana = 935;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, POWER_MANA, expectedHealingTouchMana);

            // Spell coefficient
            float const healingTouchCastTIme = 3.5f;
            float const healingTouchSpellCoeff = healingTouchCastTIme / 3.5f;
            uint32 const healingTouchBHBonus = maceBH * healingTouchSpellCoeff;
            uint32 const healingTouchMinHeal = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MIN + healingTouchBHBonus;
            uint32 const healingTouchMaxHeal = ClassSpellsDamage::Druid::HEALING_TOUCH_RNK_13_MAX + healingTouchBHBonus;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, healingTouchMinHeal, healingTouchMaxHeal, false);

            uint32 const healingTouchMinCritHeal = healingTouchMinHeal * 1.5f;
            uint32 const healingTouchMaxCritHeal = healingTouchMaxHeal * 1.5f;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, healingTouchMinCritHeal, healingTouchMaxCritHeal, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HealingTouchTestImpt>();
    }
};

class LifebloomTest : public TestCaseScript
{
public:
    LifebloomTest() : TestCaseScript("spells druid lifebloom") { }

    class LifebloomTestImpt : public TestCase
    {
    public:
        LifebloomTestImpt() : TestCase(STATUS_KNOWN_BUG) { }

        void TestLifebloom(TestPlayer* druid, uint32 tickTotal, uint32 expectedBloom, bool crit)
        {
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::LIFEBLOOM_RNK_1);
            uint32 startHealth = 1;
            druid->SetHealth(startHealth);
            EnableCriticals(druid, crit);
            TEST_CAST(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            Wait(8000);
            //uint32 expectedHealth = druid->GetHealth() - (startHealth + tickTotal);
            TEST_ASSERT(druid->GetHealth() == expectedBloom);
        }

        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
            druid->DisableRegeneration(true);

            int32 maceBH = 550;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedLifebloomMana = 220;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, POWER_MANA, expectedLifebloomMana);

            // Aura
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, Seconds(7));
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::LIFEBLOOM_RNK_1);

            // Spell coeffs -- bug here, calculations below are on par with DrDamage
            float const lifebloomDuration = 7.0f;
            float const lifebloomCastTime = 1.5f;
            float const lifebloomTotalCoeff = (lifebloomDuration / 15.0f) / ((lifebloomDuration / 15.0f) + (lifebloomCastTime / 3.5f));
            uint32 const lifebloomTotalBHBonus = maceBH * lifebloomTotalCoeff;
            uint32 const lifebloomBurstBHBonus = maceBH * ClassSpellsCoeff::Druid::LIFEBLOOM;

            // Tick
            uint32 const expectedLifebloomTick = floor((ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_TOTAL + lifebloomTotalBHBonus) / 7.0f);
            uint32 const expectedLifebloomTotal = 7 * expectedLifebloomTick;
            uint32 const expectedBloom = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST + lifebloomBurstBHBonus;
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, expectedLifebloomTotal, false);
            // Bloom
            TestLifebloom(druid, expectedLifebloomTotal, expectedBloom, false);
            TestLifebloom(druid, expectedLifebloomTotal * 1.5f, expectedBloom, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<LifebloomTestImpt>();
    }
};

class MarkOfTheWildTest : public TestCaseScript
{
public:
    MarkOfTheWildTest() : TestCaseScript("spells druid mark_of_the_wild") { }

    class MarkOfTheWildTestImpt : public TestCase
    {
    public:
        MarkOfTheWildTestImpt() : TestCase(STATUS_PASSING) { }

        void TestOfTheWild(TestPlayer* caster, TestPlayer* victim, uint32 spellId, uint32 manaCost, uint8 statBonus, uint8 resistanceBonus, uint16 armorBonus)
        {
            uint32 const expectedArmor = victim->GetArmor() + armorBonus + statBonus * 2; //also add armor related to agility increase
            uint32 const expectedAgi = victim->GetStat(STAT_AGILITY) + statBonus;
            uint32 const expectedInt = victim->GetStat(STAT_INTELLECT) + statBonus;
            uint32 const expectedSpi = victim->GetStat(STAT_SPIRIT) + statBonus;
            uint32 const expectedSta = victim->GetStat(STAT_STAMINA) + statBonus;
            uint32 const expectedStr = victim->GetStat(STAT_STRENGTH) + statBonus;
            uint32 const expectedResArcane = victim->GetResistance(SPELL_SCHOOL_ARCANE) + resistanceBonus;
            uint32 const expectedResFire = victim->GetResistance(SPELL_SCHOOL_FIRE) + resistanceBonus;
            uint32 const expectedResFrost = victim->GetResistance(SPELL_SCHOOL_FROST) + resistanceBonus;
            uint32 const expectedResNature = victim->GetResistance(SPELL_SCHOOL_NATURE) + resistanceBonus;
            uint32 const expectedResShadow = victim->GetResistance(SPELL_SCHOOL_SHADOW) + resistanceBonus;

            // Mana cost
            TEST_POWER_COST(caster, victim, spellId, POWER_MANA, manaCost);

            // Aura duration
            TEST_AURA_MAX_DURATION(victim, spellId, Minutes(30));

            // Stats, resistances & armor
            TEST_ASSERT(victim->GetArmor() == expectedArmor);
            TEST_ASSERT(victim->GetStat(STAT_AGILITY) ==  expectedAgi);
            TEST_ASSERT(victim->GetStat(STAT_INTELLECT) == expectedInt);
            TEST_ASSERT(victim->GetStat(STAT_SPIRIT) == expectedSpi);
            TEST_ASSERT(victim->GetStat(STAT_STAMINA) == expectedSta);
            TEST_ASSERT(victim->GetStat(STAT_STRENGTH) == expectedStr);
            TEST_ASSERT(victim->GetResistance(SPELL_SCHOOL_ARCANE) == expectedResArcane);
            TEST_ASSERT(victim->GetResistance(SPELL_SCHOOL_FIRE) == expectedResFire);
            TEST_ASSERT(victim->GetResistance(SPELL_SCHOOL_FROST) == expectedResFrost);
            TEST_ASSERT(victim->GetResistance(SPELL_SCHOOL_NATURE) == expectedResNature);
            TEST_ASSERT(victim->GetResistance(SPELL_SCHOOL_SHADOW) == expectedResShadow);

            // Reset for next test
            victim->RemoveAurasDueToSpell(spellId);
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_1, 20, 0, 0, 25);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_2, 50, 2, 0, 65);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_3, 100, 4, 0, 105);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_4, 160, 6, 5, 150);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_5, 240, 8, 10, 195);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_6, 340, 10, 15, 240);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_7, 445, 12, 20, 285);
            TestOfTheWild(druid, druid, ClassSpells::Druid::MARK_OF_THE_WILD_RNK_8, 565, 14, 25, 340);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<MarkOfTheWildTestImpt>();
    }
};

class RebirthTest : public TestCaseScript
{
public:
    RebirthTest() : TestCaseScript("spells druid rebirth") { }

    class RebirthTestImpt : public TestCase
    {
    public:
        RebirthTestImpt() : TestCase(STATUS_PASSING) { }

        void TestRebirth(TestPlayer* caster, TestPlayer* victim, uint32 spellId, uint32 manaCost, uint32 reagentId, uint32 expectedHealth, uint32 expectedMana)
        {
            victim->KillSelf(true);
            caster->GetSpellHistory()->ResetAllCooldowns();
            caster->AddItem(reagentId, 1);
            TEST_POWER_COST(caster, victim, spellId, POWER_MANA, manaCost);
            TEST_COOLDOWN(caster, victim, spellId, Minutes(20));
            Wait(1);
            victim->RessurectUsingRequestData();
            Wait(1); //resurrect needs 1 update to be done
            ASSERT_INFO("Victim has %u instead of expected %u health", victim->GetHealth(), expectedHealth);
            TEST_ASSERT(victim->GetHealth() == expectedHealth);
            ASSERT_INFO("Victim has %u instead of expected %u mana", victim->GetPower(POWER_MANA), expectedMana);
            TEST_ASSERT(victim->GetPower(POWER_MANA) == expectedMana);
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* ally  = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

            uint32 const manaCost = 1611;

            uint32 const MAPLE_SEED         = 17034;
            uint32 const STRANGLETHRON_SEED = 17035;
            uint32 const ASHWOOD_SEED       = 17036;
            uint32 const HORNBEAM_SEED      = 17037;
            uint32 const IRONWOOD_SEED      = 17038;
            uint32 const FLINTWEED_SEED     = 22147;

            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_1, manaCost, MAPLE_SEED, 400, 700);
            TestRebirth(druid, enemy, ClassSpells::Druid::REBIRTH_RNK_2, manaCost, STRANGLETHRON_SEED, 750, 1200);
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_3, manaCost, ASHWOOD_SEED, 1100, 1700);
            TestRebirth(druid, enemy, ClassSpells::Druid::REBIRTH_RNK_4, manaCost, HORNBEAM_SEED, 1600, 2200);
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_5, manaCost, IRONWOOD_SEED, 2200, 2800);
            TestRebirth(druid, enemy, ClassSpells::Druid::REBIRTH_RNK_6, manaCost, FLINTWEED_SEED, 3200, 3200);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RebirthTestImpt>();
    }
};

class RejuvenationTest : public TestCaseScript
{
public:
	RejuvenationTest() : TestCaseScript("spells druid rejuvenation") { }

	class RejuvenationTestImpt : public TestCase
	{
	public:
		RejuvenationTestImpt() : TestCase(STATUS_PASSING) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			EQUIP_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
			druid->DisableRegeneration(true);

            int32 maceBH = 550;
			TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

			// Mana cost
			uint32 const expectedRejuvenationMana = 415;
			TEST_POWER_COST(druid, druid, ClassSpells::Druid::REJUVENATION_RNK_13, POWER_MANA, expectedRejuvenationMana);

			// Spell coefficient
			float const rejuvenationDuration = 12.0f;
			float const rejuvenationSpellCoeff = rejuvenationDuration / 15.0f;
			uint32 const rejuvenationBHBonus = maceBH * rejuvenationSpellCoeff;

			uint32 const expectedRejuvenationTotal = ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TOTAL + rejuvenationBHBonus;
			TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REJUVENATION_RNK_13, expectedRejuvenationTotal, true);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<RejuvenationTestImpt>();
	}
};

class RegrowthTest : public TestCaseScript
{
public:
    RegrowthTest() : TestCaseScript("spells druid regrowth") { }

    class RegrowthTestImpt : public TestCase
    {
    public:
        RegrowthTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
            druid->DisableRegeneration(true);

            int32 maceBH = 550;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedRegrowthMana = 675;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, POWER_MANA, expectedRegrowthMana);
            Wait(2000);

            // Aura
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::REGROWTH_RNK_10, Seconds(21));
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::REGROWTH_RNK_10);

            // Spell coeffs
            float const regrowthDuration = 21.0f;
            float const regrowthCastTime = 2.0f;
            float const regrowthHoTCoeff = (regrowthDuration / 15.0f) / ((regrowthDuration / 15.0f) + (regrowthCastTime / 3.5f));
            float const regrowthDirectCoeff = 1 - regrowthHoTCoeff;
            uint32 const regrowthTickBHBonus = maceBH * regrowthHoTCoeff;
            uint32 const regrowthDirectBHBonus = maceBH * regrowthDirectCoeff;

            // Tick
            uint32 const expectedRegrowthTick = floor((ClassSpellsDamage::Druid::REGROWTH_RNK_10_TOTAL + regrowthTickBHBonus) / 7.0f);
            uint32 const expectedRegrowthTotal = 7 * expectedRegrowthTick;
            //TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, expectedRegrowthTotal, false);
            // Direct no crit
            uint32 const expectedRegrowthMin = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MIN + regrowthDirectBHBonus;
            uint32 const expectedRegrowthMax = ClassSpellsDamage::Druid::REGROWTH_RNK_10_MAX + regrowthDirectBHBonus;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, expectedRegrowthMin, expectedRegrowthMax, false);
            // Direct crit
            uint32 const expectedRegrowthMinCrit = expectedRegrowthMin * 1.5f;
            uint32 const expectedRegrowthMaxCrit = expectedRegrowthMax * 1.5f;
            TEST_DIRECT_HEAL(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, expectedRegrowthMinCrit, expectedRegrowthMaxCrit, true);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RegrowthTestImpt>();
    }
};

class RemoveCurseTest : public TestCaseScript
{
public:
    RemoveCurseTest() : TestCaseScript("spells druid remove_curse") { }

    class RemoveCurseTestImpt : public TestCase
    {
    public:
        RemoveCurseTestImpt() : TestCase(STATUS_PASSING) { }

        uint32 getCurseCount(TestPlayer* victim, uint32 curse1, uint32 curse2, uint32 curse3)
        {
            uint32 count = 0;
            count += uint32(victim->HasAura(curse1));
            count += uint32(victim->HasAura(curse2));
            count += uint32(victim->HasAura(curse3));
            return count;
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            _location.MoveInFront(_location, 3.0f);
            TestPlayer* ally  = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            _location.MoveInFront(_location, 3.0f);
            TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);

            _location.MoveInFront(_location, 3.0f);
            TestPlayer* warlock1 = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            _location.MoveInFront(_location, 3.0f);
            TestPlayer* warlock2 = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);
            _location.MoveInFront(_location, 3.0f);
            TestPlayer* warlock3 = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN);

            // Setup
            druid->DisableRegeneration(true);
            uint32 const CURSE_OF_THE_ELEMENTS  = ClassSpells::Warlock::CURSE_OF_THE_ELEMENTS_RNK_4;
            uint32 const CURSE_OF_WEAKNESS      = ClassSpells::Warlock::CURSE_OF_WEAKNESS_RNK_8;
            uint32 const CURSE_OF_RECKLESSNESS  = ClassSpells::Warlock::CURSE_OF_RECKLESSNESS_RNK_5;

            // Wrong target
            enemy->AddAura(CURSE_OF_THE_ELEMENTS, enemy);
            TEST_HAS_AURA(enemy, CURSE_OF_THE_ELEMENTS);
            TEST_CAST(druid, enemy, ClassSpells::Druid::REMOVE_CURSE_RNK_1, SPELL_FAILED_BAD_TARGETS);
            enemy->KillSelf();

            // No curse
            TEST_CAST(druid, ally, ClassSpells::Druid::REMOVE_CURSE_RNK_1, SPELL_FAILED_NOTHING_TO_DISPEL);
            
            // Setup
            FORCE_CAST(warlock1, ally, CURSE_OF_THE_ELEMENTS, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(warlock2, ally, CURSE_OF_WEAKNESS, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            FORCE_CAST(warlock3, ally, CURSE_OF_RECKLESSNESS, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_HAS_AURA(ally, CURSE_OF_THE_ELEMENTS);
            TEST_HAS_AURA(ally, CURSE_OF_WEAKNESS);
            TEST_HAS_AURA(ally, CURSE_OF_RECKLESSNESS);

            // Test dispells
            TEST_CAST(druid, ally, ClassSpells::Druid::REMOVE_CURSE_RNK_1);
            TEST_ASSERT(2 == getCurseCount(ally, CURSE_OF_THE_ELEMENTS, CURSE_OF_WEAKNESS, CURSE_OF_RECKLESSNESS));
            TEST_CAST(druid, ally, ClassSpells::Druid::REMOVE_CURSE_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_ASSERT(1 == getCurseCount(ally, CURSE_OF_THE_ELEMENTS, CURSE_OF_WEAKNESS, CURSE_OF_RECKLESSNESS));
            TEST_CAST(druid, ally, ClassSpells::Druid::REMOVE_CURSE_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_ASSERT(0 == getCurseCount(ally, CURSE_OF_THE_ELEMENTS, CURSE_OF_WEAKNESS, CURSE_OF_RECKLESSNESS));

            // Mana cost
            FORCE_CAST(warlock1, ally, CURSE_OF_THE_ELEMENTS, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            uint32 const expectedRemoveCurseMana = 189;
            TEST_POWER_COST(druid, ally, ClassSpells::Druid::REMOVE_CURSE_RNK_1, POWER_MANA, expectedRemoveCurseMana);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<RemoveCurseTestImpt>();
    }
};

class TranquilityTest : public TestCaseScript
{
public:
    TranquilityTest() : TestCaseScript("spells druid tranquility") { }

    class TranquilityTestImpt : public TestCase
    {
    public:
        TranquilityTestImpt() : TestCase(STATUS_PASSING) { }

        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_ITEM(druid, 32500); // Crystal Spire of Karabor - 486 BH
            druid->DisableRegeneration(true);

            int32 maceBH = 486;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedTranquilityMana = 1650;
            TEST_POWER_COST(druid, druid, ClassSpells::Druid::TRANQUILITY_RNK_5, POWER_MANA, expectedTranquilityMana);
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::TRANQUILITY_RNK_5, Seconds(8));

            // Spell coeffs
            float const TranquilityCastTime = 8.0f;
            //WoWiki says: The Tranquility coefficient scales with level, at level 52, it has a base coefficient of 26%, where as at 70, it has a base coefficient of 73%. Tested on March 4th, 2008.
            //DrDamage says 0.762 for 70
            float const TranquilityCoeff = 0.762f;
            uint32 const TranquilityBHBonus = maceBH * TranquilityCoeff;
            uint32 const TranquilityTickBHBonus = floor(TranquilityBHBonus / 4.0f);

            uint32 const expectedTranquilityTick = ClassSpellsDamage::Druid::TRANQUILITY_RNK_5_TICK + TranquilityTickBHBonus;
            TEST_CHANNEL_HEALING(druid, druid, ClassSpells::Druid::TRANQUILITY_RNK_5, ClassSpells::Druid::TRANQUILITY_RNK_5_PROC, 4, expectedTranquilityTick);
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<TranquilityTestImpt>();
    }
};

void AddSC_test_spells_druid()
{
	// Total: 39/46
	// Balance: 11/12 - Soothe Animal
	new BarkskinTest();
	new CycloneTest();
	new EntanglingRootsTest();
	new FaerieFireTest();
	new HibernateTest();
	new HurricaneTest();
	new InnervateTest();
	new MoonfireTest();
	new StarfireTest();
	new ThornsTest();
	new WrathTest();
	// Feral: 17/23 - Dash, Feline Grace, Ferocious Bite, Maim, Rip, Track Humanoids
	new BashTest();
	new ChallengingRoarTest();
	new ClawTest();
	new CowerTest();
	new DemoralizingRoarTest();
	new EnrageTest();
    new FerociousBiteTest();
	new FrenziedRegenerationTest();
	new GrowlTest();
	new LacerateTest();
	new MaulTest();
	new PounceTest();
	new ProwlTest();
	new RakeTest();
	new RavageTest();
    new ShredTest();
    new SwipeTest();
    new TigersFuryTest();
	// Restoration: 11/11
    new AbolishPoisonTest();
    new CurePoisonTest();
    new GiftOfTheWildTest();
    new HealingTouchTest();
    new LifebloomTest();
    new MarkOfTheWildTest();
    new RebirthTest();
    new RegrowthTest();
	new RejuvenationTest();
    new RemoveCurseTest();
    new TranquilityTest();
}
