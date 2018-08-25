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


		void TestState(TestPlayer* druid, uint32 spellId, bool shapeshifted = false)
		{
			druid->AddAura(spellId, druid);
			if (shapeshifted)
				druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
			TEST_CAST(druid, druid, ClassSpells::Druid::BARKSKIN_RNK_1, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_GCD));
			druid->RemoveAurasDueToSpell(spellId);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BARKSKIN_RNK_1);
		}

		void Test() override
		{
			// Init barkskin druid
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			// Init damage druid
			TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF);
			EQUIP_NEW_ITEM(enemy, 34182); // Grand Magister's Staff of Torrents - 266 SP
            int32 staffSP = enemy->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			// Mana cost
			uint32 const expectedBarkskinMana = 284;
			TEST_POWER_COST(druid, ClassSpells::Druid::BARKSKIN_RNK_1, POWER_MANA, expectedBarkskinMana);

			// Duration & CD
            TEST_CAST(druid, druid, ClassSpells::Druid::BARKSKIN_RNK_1);
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::BARKSKIN_RNK_1, Seconds(12));
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::BARKSKIN_RNK_1, Minutes(1));

			// Pushback
            TEST_PUSHBACK_RESIST_CHANCE(enemy, druid, ClassSpells::Druid::MOONFIRE_RNK_12, 100.f);

			// Spell damage reduced by 20%
			float const barkskinDamageFactor = 0.8f;
			uint32 const starfireBonusSP = ClassSpellsCoeff::Druid::STARFIRE * staffSP;
			uint32 const expectedStarfireMinDmg = (ClassSpellsDamage::Druid::STARFIRE_RNK_8_MIN + starfireBonusSP) * barkskinDamageFactor;
			uint32 const expectedStarfireMaxDmg = (ClassSpellsDamage::Druid::STARFIRE_RNK_8_MAX + starfireBonusSP) * barkskinDamageFactor;
            TEST_DIRECT_SPELL_DAMAGE(enemy, druid, ClassSpells::Druid::STARFIRE_RNK_8, expectedStarfireMinDmg, expectedStarfireMaxDmg, false, [](Unit* caster, Unit* victim) {
                victim->AddAura(ClassSpells::Druid::BARKSKIN_RNK_1, victim);
            });

            // Melee damage reduced by 20%
            auto[minMelee, maxMelee] = CalcMeleeDamage(enemy, druid, BASE_ATTACK);
            minMelee *= barkskinDamageFactor;
            maxMelee *= barkskinDamageFactor;
            TEST_MELEE_DAMAGE(enemy, druid, BASE_ATTACK, minMelee, maxMelee, false, [](Unit* attacker, Unit* victim) {
                victim->AddAura(ClassSpells::Druid::BARKSKIN_RNK_1, victim);
            });

            // Test usable while stunned, frozen, incapacitated, feared and asleep
			TestState(druid, ClassSpells::Rogue::KIDNEY_SHOT_RNK_2);
			TestState(druid, ClassSpells::Mage::FROST_NOVA_RNK_1);
			TestState(druid, ClassSpells::Rogue::GOUGE_RNK_6);
			TestState(druid, ClassSpells::Warlock::FEAR_RNK_3);
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
			TestState(druid, ClassSpells::Druid::HIBERNATE_RNK_3, true);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<BarkskinTestImpt>();
	}
};

class CycloneTest : public TestCaseScript
{
public:
	CycloneTest() : TestCaseScript("spells druid cyclone") { }

	class CycloneTestImpt : public TestCase
	{
	public:


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

			// Cooldown & invulnerable to damage & heals
            uint32 const startHealth = druid2->GetHealth();
            TEST_CAST(druid2, druid2, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            FORCE_CAST(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            TEST_AURA_MAX_DURATION(druid2, ClassSpells::Druid::CYCLONE_RNK_1, Seconds(6));
            FORCE_CAST(druid, druid2, ClassSpells::Druid::WRATH_RNK_10, SPELL_MISS_NONE, TriggerCastFlags(TRIGGERED_CAST_DIRECTLY | TRIGGERED_IGNORE_SPEED));
			WaitNextUpdate(); // wait for wrath to hit
            //lifebloom and wrath should both fail
			TEST_ASSERT(druid2->GetHealth() == startHealth);

			// Only one target can be affected
            FORCE_CAST(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            FORCE_CAST(druid, druid3, ClassSpells::Druid::CYCLONE_RNK_1, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            TEST_HAS_NOT_AURA(druid2, ClassSpells::Druid::CYCLONE_RNK_1);
            TEST_HAS_AURA(druid3, ClassSpells::Druid::CYCLONE_RNK_1);

			// Mana cost
			uint32 const expectedCycloneMana = 189;
			TEST_POWER_COST(druid, ClassSpells::Druid::CYCLONE_RNK_1, POWER_MANA, expectedCycloneMana);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<CycloneTestImpt>();
	}
};

class EntanglingRootsTest : public TestCaseScript
{
public:
	EntanglingRootsTest() : TestCaseScript("spells druid entangling_roots") { }

	class EntanglingRootsTestImpt : public TestCase
	{
		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			int32 staffSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			// Mana cost
			uint32 const expectedEntanglingRootsMana = 160;
            TEST_POWER_COST(druid, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, POWER_MANA, expectedEntanglingRootsMana);

            // Max duration
            FORCE_CAST(druid, creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, Seconds(27));

			// Is rooted
			TEST_ASSERT(creature->IsInRoots());

            //Bug: Spell coeff too high
            SECTION("Damage", STATUS_KNOWN_BUG, [&] {
                // Spell coefficient
                uint32 const tickAmount = 9;
                uint32 const entanglingRootsBonusSP = ClassSpellsCoeff::Druid::ENTANGLING_ROOTS / tickAmount * staffSP;

                uint32 const entanglingRootsTick = ClassSpellsDamage::Druid::ENTANGLING_ROOTS_RNK_7_TICK + entanglingRootsBonusSP;
                uint32 const expectedEntanglingRootsDmg = tickAmount * entanglingRootsTick;
                TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, expectedEntanglingRootsDmg, false);
            });
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<EntanglingRootsTestImpt>();
	}
};

class FaerieFireTest : public TestCaseScript
{
public:

	FaerieFireTest() : TestCaseScript("spells druid faerie_fire") { }

	class FaerieFireTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);
			TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawnPosition);

            uint32 const faerieFireArmorMalus = 610;

			EQUIP_NEW_ITEM(rogue, 34211); // S4 Chest for armor
            uint32 const expectedRogueArmor = rogue->GetArmor() - faerieFireArmorMalus;

			// Faerie Fire 
            FORCE_CAST(druid, rogue, ClassSpells::Druid::FAERIE_FIRE_RNK_5);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Druid::FAERIE_FIRE_RNK_5, Seconds(40));
			ASSERT_INFO("Rogue has %u armor, expected: %u", rogue->GetArmor(), expectedRogueArmor);
			TEST_ASSERT(rogue->GetArmor() == expectedRogueArmor);

			// Rogue can't stealth
            TEST_CAST(rogue, rogue, ClassSpells::Rogue::STEALTH_RNK_4, SPELL_FAILED_CASTER_AURASTATE);

			// Mage can't invisible
            FORCE_CAST(druid, mage, ClassSpells::Druid::FAERIE_FIRE_RNK_5, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            TEST_CAST(mage, mage, ClassSpells::Mage::INVISIBILITY_RNK_1, SPELL_FAILED_CASTER_AURASTATE);

            uint32 expectedFaerieFireMana = 145;
            TEST_POWER_COST(druid, ClassSpells::Druid::FAERIE_FIRE_RNK_5, POWER_MANA, expectedFaerieFireMana);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<FaerieFireTestImpt>();
	}
};

class HibernateTest : public TestCaseScript
{
public:

	HibernateTest() : TestCaseScript("spells druid hibernate") { }

	class HibernateTestImpt : public TestCase
	{
	public:


		void HibernateDuration(TestPlayer* druid, Unit* enemy, Milliseconds durationMS = Milliseconds(40000))
		{
            FORCE_CAST(druid, enemy, ClassSpells::Druid::HIBERNATE_RNK_3, SPELL_MISS_NONE, TRIGGERED_CAST_DIRECTLY);
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
            HibernateDuration(druid, beast);

			// Only one target at a time
            HibernateDuration(druid, dragonkin);
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
			TEST_CAST(druid, enemy, ClassSpells::Druid::HIBERNATE_RNK_3);
            TEST_HAS_NOT_AURA(enemy, ClassSpells::Druid::HIBERNATE_RNK_3);

            uint32 expectedHibernateMana = 150;
            TEST_POWER_COST(druid, ClassSpells::Druid::HIBERNATE_RNK_3, POWER_MANA, expectedHibernateMana);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<HibernateTestImpt>();
	}
};

class HurricaneTest : public TestCaseScript
{
public:
	HurricaneTest() : TestCaseScript("spells druid hurricane") { }

	class HurricaneTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
            int32 staffSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			uint32 const expectedRogueAttackSpeed = rogue->GetAttackTimer(BASE_ATTACK) * 1.25f;

            // Mana cost
            uint32 const expectedHurricaneMana = 1905;
            TEST_POWER_COST(druid, ClassSpells::Druid::HURRICANE_RNK_4, POWER_MANA, expectedHurricaneMana);

            TEST_CAST(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Cooldown
            TEST_COOLDOWN(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, Minutes(1));

            // Duration: TODO: any better way to check it?
			SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ClassSpells::Druid::HURRICANE_RNK_4);
			TEST_ASSERT(spellInfo != nullptr);
			TEST_ASSERT(spellInfo->GetDuration() == 10 * SECOND * IN_MILLISECONDS);

            // 25% AS
			TEST_ASSERT(rogue->GetAttackTimer(BASE_ATTACK) == expectedRogueAttackSpeed);

			// Damage
			float const hurricaneSpellCoeff = 10.0f / 3.5f / 2.0f / 10.0f;
			uint32 const hurricaneBonusSP = hurricaneSpellCoeff * staffSP;
			uint32 const expectedHurricaneDmg = ClassSpellsDamage::Druid::HURRICANE_RNK_4_TICK + hurricaneBonusSP;
			TEST_CHANNEL_DAMAGE(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, 10, expectedHurricaneDmg, ClassSpells::Druid::HURRICANE_RNK_4_PROC);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<HurricaneTestImpt>();
	}
};

class InnervateTest : public TestCaseScript
{
public:
	InnervateTest() : TestCaseScript("spells druid innervate") { }

	class InnervateTestImpt : public TestCase
	{
	public:


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
            TEST_POWER_COST(druid, ClassSpells::Druid::INNERVATE_RNK_1, POWER_MANA, expectedInnervateManaCost);

			// Duration & CD
            TEST_CAST(druid, druid, ClassSpells::Druid::INNERVATE_RNK_1);
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::INNERVATE_RNK_1, Seconds(20));
			TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::INNERVATE_RNK_1, Minutes(6));

            // Mana regen
            TEST_ASSERT(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) == regenPerSecondWithInnervate);
            TEST_ASSERT(druid->GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) == regenPerSecondWithInnervate);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<InnervateTestImpt>();
	}
};

class MoonfireTest : public TestCaseScript
{
public:
	MoonfireTest() : TestCaseScript("spells druid moonfire") { }

	class MoonfireTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			int32 staffSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			// Mana cost
			uint32 const expectedMoonfireManaCost = 495;
			TEST_POWER_COST(druid, ClassSpells::Druid::MOONFIRE_RNK_12, POWER_MANA, expectedMoonfireManaCost);

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
            uint32 const moonfireTickAmount = 4;
            uint32 const moonfireTick = ClassSpellsDamage::Druid::MOONFIRE_RNK_12_TICK + moonfireDoTBonusSP / moonfireTickAmount;
			uint32 const expectedMoonfireTotalDmg = moonfireTickAmount * moonfireTick;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireTotalDmg, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MoonfireTestImpt>();
	}
};

class StarfireTest : public TestCaseScript
{
public:
	StarfireTest() : TestCaseScript("spells druid starfire") { }

	class StarfireTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

            int32 staffSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			// Mana cost
			uint32 const expectedStarfireMana = 370;
			TEST_POWER_COST(druid, ClassSpells::Druid::STARFIRE_RNK_8, POWER_MANA, expectedStarfireMana);

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

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<StarfireTestImpt>();
	}
};

class ThornsTest : public TestCaseScript
{
public:
	ThornsTest() : TestCaseScript("spells druid thorns") { }

	class ThornsTestImpt : public TestCase
	{
	public:


		void Test() override
		{
            TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);
            _location.MoveInFront(_location, 3.0f);
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            druid->ForceSpellHitResultOverride(SPELL_MISS_NONE); //Thorns can miss

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);
            rogue->DisableRegeneration(true);

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
            TEST_POWER_COST(druid, ClassSpells::Druid::THORNS_RNK_7, POWER_MANA, expectedThornsMana);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ThornsTestImpt>();
	}
};

class WrathTest : public TestCaseScript
{
public:
	WrathTest() : TestCaseScript("spells druid wrath") { }

	class WrathTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* dummy = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

            int32 staffSP = druid->GetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);

			// Mana cost
			uint32 const expectedWrathMana = 255;
			TEST_POWER_COST(druid, ClassSpells::Druid::WRATH_RNK_10, POWER_MANA, expectedWrathMana);

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

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<WrathTestImpt>();
	}
};

class BashTest : public TestCaseScript
{
public:
	BashTest() : TestCaseScript("spells druid bash") { }

	class BashTestImpt : public TestCase
	{
	public:
		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);

            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

			// Rage cost
			uint32 const expectedBashRageCost = 10 * 10;
            TEST_POWER_COST(druid, ClassSpells::Druid::BASH_RNK_3, POWER_RAGE, expectedBashRageCost);

            // Aura duration
            druid->SetFullPower(POWER_RAGE);
            FORCE_CAST(druid, rogue, ClassSpells::Druid::BASH_RNK_3, SPELL_MISS_NONE);
            TEST_AURA_MAX_DURATION(rogue, ClassSpells::Druid::BASH_RNK_3, Seconds(4));
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<BashTestImpt>();
	}
};

class GrowlTest : public TestCaseScript
{
public:

    GrowlTest() : TestCaseScript("spells druid growl") { }

    class GrowlTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            Position spawn3m(_location);
            spawn3m.MoveInFront(_location, 3.0f);
            TestPlayer* warlock = SpawnPlayer(CLASS_WARLOCK, RACE_HUMAN, 70, spawn3m);
            Creature* creature = SpawnCreatureWithPosition(spawn3m, 6); // Kobold Vermin

            // Setup
            creature->SetMaxHealth(10000);
            creature->SetHealth(10000);
            FORCE_CAST(warlock, creature, ClassSpells::Warlock::SEARING_PAIN_RNK_8, SPELL_MISS_NONE, TRIGGERED_FULL_DEBUG_MASK);
            uint32 warlockThreat = creature->GetThreatManager().GetThreat(warlock);
            TEST_ASSERT(warlockThreat > 0);
            creature->AI()->UpdateVictim(); //update immediately to avoid waiting an update
            TEST_ASSERT(creature->GetVictim() == warlock);

            // Acquire threat, aura duration, cooldown
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            FORCE_CAST(druid, creature, ClassSpells::Druid::GROWL_RNK_1);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::GROWL_RNK_1, Seconds(3));
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::GROWL_RNK_1, Seconds(10));
            TEST_ASSERT(creature->GetThreatManager().GetThreat(druid) == warlockThreat);
            creature->AI()->UpdateVictim();
            TEST_ASSERT(creature->GetVictim() == druid);

            // Make some more instant threat
            FORCE_CAST(warlock, creature, ClassSpells::Warlock::SEARING_PAIN_RNK_8, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
            warlockThreat = creature->GetThreatManager().GetThreat(warlock);
            TEST_ASSERT(warlockThreat > creature->GetThreatManager().GetThreat(druid) * 1.1f); //1.1 is the retake aggro threshold
                                                                                               //warlock now has higher threat but target should stay unchanged

            creature->AI()->UpdateVictim();
            TEST_ASSERT(creature->GetVictim() == druid);

            Wait(3300); //taunt aura last 3000, let it expire
            TEST_HAS_NOT_AURA(creature, ClassSpells::Druid::GROWL_RNK_1);
            //Taunt faded and warlock has higher threat, target should be restored!
            TEST_ASSERT(creature->GetVictim() == warlock);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<GrowlTestImpt>();
    }
};

class ChallengingRoarTest : public TestCaseScript
{
public:

	ChallengingRoarTest() : TestCaseScript("spells druid challenging_roar") { }

	class ChallengingRoarTestImpt : public TestCase
	{
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
            //force update victim immediately to avoid waiting more updates
            creature3m->AI()->UpdateVictim();
            creature6m->AI()->UpdateVictim();
            creature15m->AI()->UpdateVictim();
            TEST_ASSERT(creature3m->GetVictim() == player3m);
            TEST_ASSERT(creature6m->GetVictim() == player6m);
            TEST_ASSERT(creature15m->GetVictim() == player15m);

			TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            druid->SetFullPower(POWER_RAGE);
            FORCE_CAST(druid, druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1);
            // CD
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, Minutes(10));

            // Aura
            TEST_AURA_MAX_DURATION(creature3m, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, Seconds(6));
            TEST_AURA_MAX_DURATION(creature6m, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, Seconds(6));
            TEST_HAS_NOT_AURA(creature15m, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1);

            //force update victim immediately to avoid waiting more updates
            creature3m->AI()->UpdateVictim();
            creature6m->AI()->UpdateVictim();
		
			// Target changed
			TEST_ASSERT(creature3m->GetVictim() == druid);
			TEST_ASSERT(creature6m->GetVictim() == druid);

			// Back to original target
			Wait(8000);
			TEST_ASSERT(creature3m->GetVictim() == player3m);
			TEST_ASSERT(creature6m->GetVictim() == player6m);

            SECTION("Threat", STATUS_WIP, [&] {
                //TODO: test base threat generated by challenging roar
            });

            // Rage cost
            uint32 const expectedChallengingRoarRage = 15 * 10;
            TEST_POWER_COST(druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, POWER_RAGE, expectedChallengingRoarRage);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ChallengingRoarTestImpt>();
	}
};

class ClawTest : public TestCaseScript
{
public:

	ClawTest() : TestCaseScript("spells druid claw") { }

	class ClawTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            druid->AddAura(ClassSpells::Druid::CAT_FORM_RNK_1, druid);

            FORCE_CAST(druid, creature, ClassSpells::Druid::CLAW_RNK_6);

			// Combo point added
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Damage
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			uint32 const expectedClawMin = floor(ClassSpellsDamage::Druid::GetCatMinDmg() + AP / 14 + ClassSpellsDamage::Druid::CLAW_RNK_6) * armorFactor;
			uint32 const expectedClawMax = floor(ClassSpellsDamage::Druid::GetCatMaxDmg() + AP / 14 + ClassSpellsDamage::Druid::CLAW_RNK_6) * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::CLAW_RNK_6, expectedClawMin, expectedClawMax, false);

            // Energy cost
            uint32 const expectedClawEnergy = 45;
            TEST_POWER_COST(druid, ClassSpells::Druid::CLAW_RNK_6, POWER_ENERGY, expectedClawEnergy);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ClawTestImpt>();
	}
};

class CowerTest : public TestCaseScript
{
public:
	CowerTest() : TestCaseScript("spells druid cower") { }

	class CowerTestImpt : public TestCase
	{
		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            //Test threat
            SECTION("Threat", [&] {
                //generate some threat
                druid->ForceMeleeHitResult(MELEE_HIT_NORMAL);
                for (int i = 0; i < 50; i++)
                    druid->AttackerStateUpdate(creature, BASE_ATTACK);

                druid->AttackStop();
                druid->ResetForceMeleeHitResult();

                uint32 const cowerPoints = 1170;
                float const currentThreat = creature->GetThreatManager().GetThreat(druid);
                ASSERT_INFO("Druids needs to have higher threat than %f to test spell", currentThreat);
                TEST_ASSERT(currentThreat > cowerPoints);
                float const expectedThreat = currentThreat - cowerPoints;
                TEST_CAST(druid, creature, ClassSpells::Druid::COWER_RNK_5);
                ASSERT_INFO("Before: %f, current: %f, expected: %f", currentThreat, creature->GetThreatManager().GetThreat(druid), expectedThreat);
                TEST_ASSERT(Between<float>(creature->GetThreatManager().GetThreat(druid), expectedThreat - 2.f, expectedThreat + 2.f));
            });

            //Test CD
			TEST_COOLDOWN(druid, creature, ClassSpells::Druid::COWER_RNK_5, Seconds(10));

            //Test power cost
            uint32 expectedCowerEnergy = 20;
            TEST_POWER_COST(druid, ClassSpells::Druid::COWER_RNK_5, POWER_ENERGY, expectedCowerEnergy);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<CowerTestImpt>();
	}
};

class DemoralizingRoarTest : public TestCaseScript
{
public:

	DemoralizingRoarTest() : TestCaseScript("spells druid demoralizing_roar") { }

	class DemoralizingRoarTestImpt : public TestCase
	{
	public:


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

            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

            druid->SetFullPower(POWER_RAGE);
            FORCE_CAST(druid, druid, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6);

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

            // Rage cost
            uint32 const expectedDemoralizingRoarRage = 10 * 10;
            TEST_POWER_COST(druid, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, POWER_RAGE, expectedDemoralizingRoarRage);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<DemoralizingRoarTestImpt>();
	}
};

class EnrageTest : public TestCaseScript
{
public:

	EnrageTest() : TestCaseScript("spells druid enrage") { }

	class EnrageTestImpt : public TestCase
	{
	public:


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
            EQUIP_NEW_ITEM(druid, 31042); // "Thunderheart Chestguard" (T6 torso)  // give him some more armor

			TestEnrage(druid, ClassSpells::Druid::BEAR_FORM_RNK_1, 0.27f);
			TestEnrage(druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, 0.16f);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<EnrageTestImpt>();
	}
};

class FerociousBiteTest : public TestCaseScript
{
public:

    FerociousBiteTest() : TestCaseScript("spells druid ferocious_bite") { }

    class FerociousBiteTestImpt : public TestCase
    {
        void Test() override
        {
            SECTION("WIP", STATUS_WIP, [&] {
                TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
                Creature* creature = SpawnCreature();

                EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

                // Must be in Cat Form
                TEST_CAST(druid, creature, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, SPELL_FAILED_ONLY_SHAPESHIFT);
                TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

                // Need combo point
                TEST_CAST(druid, creature, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, SPELL_FAILED_CASTER_AURASTATE);
                druid->AddComboPoints(creature, 1);

                // Energy cost
                uint32 const expectedFerociousBiteEnergy = 35;
                TEST_POWER_COST(druid, ClassSpells::Druid::FEROCIOUS_BITE_RNK_6, POWER_ENERGY, expectedFerociousBiteEnergy);

                // Damages
                float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
                float const armorFactor = GetArmorFactor(druid, creature);
                float APperCP = 0.03f;

                for (uint32 comboPoints = 1; comboPoints < 6; comboPoints++)
                {
                    uint32 const energyAfterFB = 65;
                    float const dmgPerRemainingEnergy = 4.1f;

                    uint32 cpDmgMin = ClassSpellsDamage::Druid::FEROCIOUS_BITE_RNK_6_CP_1_MIN;
                    uint32 cpDmgMax = ClassSpellsDamage::Druid::FEROCIOUS_BITE_RNK_6_CP_1_MAX;

                    uint32 minFerociousBiteDmg = (AP * comboPoints * APperCP + cpDmgMin + dmgPerRemainingEnergy * energyAfterFB) * armorFactor;
                    uint32 maxFerociousBiteDmg = (AP * comboPoints * APperCP + cpDmgMax + dmgPerRemainingEnergy * energyAfterFB) * armorFactor;

                    auto[sampleSize, absoluteAllowedError] = _GetApproximationParams(minFerociousBiteDmg, maxFerociousBiteDmg);

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
                        HandleThreadPause();
                    }
                    druid->ForceMeleeHitResult(MELEE_HIT_MISS);

                    uint32 allowedMin = minFerociousBiteDmg > absoluteAllowedError ? minFerociousBiteDmg - absoluteAllowedError : 0; //protect against underflow
                    uint32 allowedMax = maxFerociousBiteDmg + absoluteAllowedError;

                    TEST_ASSERT(maxFerociousBiteDmg <= allowedMax);
                    TEST_ASSERT(minFerociousBiteDmg >= allowedMin);
                }
            });
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<FerociousBiteTestImpt>();
    }
};

class FrenziedRegenerationTest : public TestCaseScript
{
public:

	FrenziedRegenerationTest() : TestCaseScript("spells druid frenzied_regeneration") { }

	class FrenziedRegenerationTestImpt : public TestCase
	{
	public:


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
            //FR actually triggers this healing spell at each tick
            auto [dealtMin, dealtMax] = GetHealingPerSpellsTo(druid, druid, ClassSpells::Druid::FRENZIED_REGENERATION_TRIGGER, crit, 10);
            TEST_ASSERT(dealtMin == dealtMax);
            TEST_ASSERT(expected == dealtMin);
        }
         
		void TestFrenziedRegeneration(TestPlayer* druid, uint32 spellFormId)
		{
            druid->GetSpellHistory()->ResetAllCooldowns();
            druid->DisableRegeneration(true); //also disable rage regen and decaying
            //setup
            TEST_CAST(druid, druid, spellFormId, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            druid->SetFullPower(POWER_RAGE);

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

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<FrenziedRegenerationTestImpt>();
	}
};

class LacerateTest : public TestCaseScript
{
public:

	LacerateTest() : TestCaseScript("spells druid lacerate") { }

	class LacerateTestImpt : public TestCase
	{
		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			creature->DisableRegeneration(true);
			druid->DisableRegeneration(true);
			EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

			// Rage cost
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			uint32 const expectedLacerateRage = 15 * 10;
			TEST_POWER_COST(druid, ClassSpells::Druid::LACERATE_RNK_1, POWER_RAGE, expectedLacerateRage);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);

            SECTION("Direct damage", [&] {
                // Always fixed damage, no ap bonus
                TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
                TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::LACERATE_RNK_1, ClassSpellsDamage::Druid::LACERATE_RNK_1, ClassSpellsDamage::Druid::LACERATE_RNK_1, false);
                creature->RemoveAllAuras();
            });

            uint32 const expectedMaxStacks = 5;
            SECTION("Stacks", [&] {
                for (uint32 i = 0; i < expectedMaxStacks + 2; i++)
                {
                    FORCE_CAST(druid, creature, ClassSpells::Druid::LACERATE_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);
                    TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::LACERATE_RNK_1, Seconds(15));
                }
                //we casted 7 times but stacks should only go to 5
                TEST_AURA_STACK(creature, ClassSpells::Druid::LACERATE_RNK_1, expectedMaxStacks);
                creature->RemoveAurasDueToSpell(ClassSpells::Druid::LACERATE_RNK_1);
            });

            SECTION("Bleed damage", [&] {
                float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
                uint32 const lacerateTickAmount = 5;
                uint32 const apBonus = AP * 0.05f / lacerateTickAmount; // AP coeff since 2.4, validated by DrDamage

                // test damage with all stacks. Target has currently 5 stacks
                int32 const expectedTotalDotDamage = expectedMaxStacks * lacerateTickAmount * (ClassSpellsDamage::Druid::LACERATE_RNK_1_BLEED_TICK + apBonus);
                auto AI = _GetCasterAI(druid);
                AI->ResetSpellCounters();

                for (uint32 i = 0; i < expectedMaxStacks; i++)
                    FORCE_CAST(druid, creature, ClassSpells::Druid::LACERATE_RNK_1, SPELL_MISS_NONE, TRIGGERED_FULL_MASK);

                Wait(16000); // wait all ticks to finish

                auto[doneToTarget, tickCount] = AI->GetDotDamage(creature, ClassSpells::Druid::LACERATE_RNK_1);
                TEST_ASSERT(tickCount == lacerateTickAmount);
                ASSERT_INFO("Lacerate did %u dmg but %u was expected.", doneToTarget, expectedTotalDotDamage);
                TEST_ASSERT(Between<uint32>(doneToTarget, expectedTotalDotDamage - 1, expectedTotalDotDamage + 1));
            });
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<LacerateTestImpt>();
	}
};

class MaulTest : public TestCaseScript
{
public:

	MaulTest() : TestCaseScript("spells druid maul") { }

	class MaulTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

			// Rage cost
			uint32 const expectedMaulRage = 15 * 10;
			TEST_POWER_COST(druid, ClassSpells::Druid::MAUL_RNK_8, POWER_RAGE, expectedMaulRage);

			// Shapeshift
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_FULL_MASK);

			// Damage
			uint32 const level = 60;
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = GetArmorFactor(druid, creature);
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

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<MaulTestImpt>();
	}
};

class PounceTest : public TestCaseScript
{
public:

	PounceTest() : TestCaseScript("spells druid pounce") { }

	class PounceTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

			// Must be in Cat Form
            TEST_CAST(druid, creature, ClassSpells::Druid::POUNCE_RNK_1, SPELL_FAILED_ONLY_SHAPESHIFT);

			// Only castable stealthed
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_CAST(druid, creature, ClassSpells::Druid::POUNCE_RNK_1, SPELL_FAILED_ONLY_STEALTHED);
            TEST_CAST(druid, druid, ClassSpells::Druid::PROWL_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			
			// Energy cost
			uint32 const expectedPounceEnergy = 50;
			TEST_POWER_COST(druid, ClassSpells::Druid::POUNCE_RNK_4, POWER_ENERGY, expectedPounceEnergy);

            FORCE_CAST(druid, creature, ClassSpells::Druid::POUNCE_RNK_4);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::POUNCE_RNK_4, Seconds(3));
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::POUNCE_RNK_4_PROC, Seconds(18));

			// Combo
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Damage
            // WoWWiki: Pounce damage is affected by attack power. Empirical data gathered by player Staralfur in patch 2.0 suggests this calculation: Base Damage + .18 x Attack Power
            // This is also the coef listed in DrDamage
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const pounceBleedCoeff = 0.18f;
            uint32 const pounceTickAmount = 6;
            uint32 const pounceTick = ClassSpellsDamage::Druid::POUNCE_RNK_4_TICK + AP * pounceBleedCoeff / pounceTickAmount;
			uint32 const pounceBleedTotal = pounceTickAmount * pounceTick;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::POUNCE_RNK_4_PROC, pounceBleedTotal, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<PounceTestImpt>();
	}
};

class ProwlTest : public TestCaseScript
{
public:

	ProwlTest() : TestCaseScript("spells druid prowl") { }

	class ProwlTestImpt : public TestCase
	{
	public:


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

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<ProwlTestImpt>();
	}
};

class RakeTest : public TestCaseScript
{
public:

	RakeTest() : TestCaseScript("spells druid rake") { }

	class RakeTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

			// Must be in Cat Form
            TEST_CAST(druid, creature, ClassSpells::Druid::RAKE_RNK_5, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

			// Energy cost
			uint32 const expectedRakeEnergy = 40;
			TEST_POWER_COST(druid, ClassSpells::Druid::RAKE_RNK_5, POWER_ENERGY, expectedRakeEnergy);

            FORCE_CAST(druid, creature, ClassSpells::Druid::RAKE_RNK_5);
            TEST_AURA_MAX_DURATION(creature, ClassSpells::Druid::RAKE_RNK_5, Seconds(9));

			// Combo
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Direct Damage
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			uint32 const rakeDamage = floor(AP / 100 + ClassSpellsDamage::Druid::RAKE_RNK_5);
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::RAKE_RNK_5, rakeDamage, rakeDamage, false);

			// Bleed
            //WoWwiki: Rank5: Rake the target for (AP/100+78) damage and an additional (108+AP*0.06) damage over 9 sec.
            float const rakeAPCoeff = 0.06f;    
            float const rakeTickCount = 3.0f;
			uint32 const rakeBleedTick = ClassSpellsDamage::Druid::RAKE_RNK_5_BLEED_TICK + AP * rakeAPCoeff / rakeTickCount;
			uint32 const expectedRakeBleedDamage = rakeTickCount * rakeBleedTick;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::RAKE_RNK_5, expectedRakeBleedDamage, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<RakeTestImpt>();
	}
};

class RavageTest : public TestCaseScript
{
public:
	RavageTest() : TestCaseScript("spells druid ravage") { }

    //Ravage the target, causing 385 % damage plus 514 to the target.Must be prowling and behind the target.Awards 1 combo point.
	class RavageTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition);
            creature->SetOrientation(druid->GetOrientation() + M_PI); //face druid

			EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

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
			TEST_POWER_COST(druid, ClassSpells::Druid::RAVAGE_RNK_5, POWER_ENERGY, expectedRavageEnergy);
            
            creature->SetOrientation(druid->GetOrientation());
            FORCE_CAST(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5);
			// Combo
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			/*  Damage -- damage is too high? calculations below are in agreement with DrDamage (I could not find any other source for now)
             kelno: spell has 147 in base points in dbc, so should do +565 and not +514... value 514 is because 147 x 350% = 514 and "In Patch 2.3.2, damage (all ranks) changed from 350% to 385%." 
             So I'm pretty sure our calculation is correct, but either:
             - Blizzard forgot to update the tooptip and the bonus damage is actually 565 (147 x 385%) since 2.3.2 (note that the 514 value is hardcoded in the tooltip so that's a likely error)
             - Blizzard reduced the base value to 133 but our dbc did not follow this change (dbc are indeed sometimes late on patches)
             Failing to find any good source, I'll use the tooltip data for now and update the base value of the spell to 133. 
             It may be a nerf but in case someone looks into it it's looking better to match the tooltip + the spell is not in the dps rotation so that's not a big loss.
            */

			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = GetArmorFactor(druid, creature);
			float const ravageFactor = 3.85f;
			uint32 const minDamage = floor(ClassSpellsDamage::Druid::GetCatMinDmg() + AP / 14) * ravageFactor + ClassSpellsDamage::Druid::RAVAGE_RNK_5;
            uint32 const maxDamage = floor(ClassSpellsDamage::Druid::GetCatMaxDmg() + AP / 14) * ravageFactor + ClassSpellsDamage::Druid::RAVAGE_RNK_5;
			uint32 const expectedRavageMin = minDamage * armorFactor;
			uint32 const expectedRavageMax = maxDamage * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::RAVAGE_RNK_5, expectedRavageMin, expectedRavageMax, false);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<RavageTestImpt>();
	}
};

class ShredTest : public TestCaseScript
{
public:

    ShredTest() : TestCaseScript("spells druid shred") { }

    class ShredTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            Position spawnPosition(_location);
            spawnPosition.MoveInFront(_location, 3.0f);
            Creature* creature = SpawnCreatureWithPosition(spawnPosition);
            creature->SetOrientation(creature->GetOrientation() + M_PI);

            EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

            // Must be behind & in cat form
            TEST_CAST(druid, creature, ClassSpells::Druid::SHRED_RNK_7, SPELL_FAILED_NOT_BEHIND);
            creature->SetOrientation(druid->GetOrientation());
            TEST_CAST(druid, creature, ClassSpells::Druid::SHRED_RNK_7, SPELL_FAILED_ONLY_SHAPESHIFT);
            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Energy cost
            uint32 const expectedShredEnergy = 60;
            TEST_POWER_COST(druid, ClassSpells::Druid::SHRED_RNK_7, POWER_ENERGY, expectedShredEnergy);

            FORCE_CAST(druid, creature, ClassSpells::Druid::SHRED_RNK_7);

            // Combo
            TEST_ASSERT(druid->GetComboPoints(creature) == 1);

            // Damage
            uint32 const level = 60;
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
            float const shredFactor = 2.25f;
            uint32 const minDamage = floor(ClassSpellsDamage::Druid::GetCatMinDmg() + AP / 14) * shredFactor + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const maxDamage = floor(ClassSpellsDamage::Druid::GetCatMaxDmg() + AP / 14) * shredFactor + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const expectedShredMin = minDamage * armorFactor;
            uint32 const expectedShredMax = maxDamage * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::SHRED_RNK_7, expectedShredMin, expectedShredMax, false);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<ShredTestImpt>();
    }
};

class SwipeTest : public TestCaseScript
{
public:

    SwipeTest() : TestCaseScript("spells druid swipe") { }

    class SwipeTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            /* DrDamage AP factor: 0.07
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

            EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
            TEST_CAST(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Rage cost
            uint32 const expectedSwipeRage = 20 * 10;
            TEST_POWER_COST(druid, ClassSpells::Druid::SWIPE_RNK_6, POWER_RAGE, expectedSwipeRage);

            // Shapeshift
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::BEAR_FORM_RNK_1);
            TEST_CAST(druid, druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Damage
            uint32 const level = 60;
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = GetArmorFactor(druid, creature1);
            float const swipeFactor = 0.07f;
            uint32 const weaponDamage = ClassSpellsDamage::Druid::SWIPE_RNK_6 + AP * swipeFactor;
            uint32 const expectedSwipe = weaponDamage * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature1, ClassSpells::Druid::SWIPE_RNK_6, expectedSwipe, expectedSwipe, false);

            uint32 const expectedSwipeCrit = weaponDamage * 2.0f * armorFactor;
            TEST_DIRECT_SPELL_DAMAGE(druid, creature1, ClassSpells::Druid::SWIPE_RNK_6, expectedSwipeCrit, expectedSwipeCrit, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<SwipeTestImpt>();
    }
};

class TigersFuryTest : public TestCaseScript
{
public:
    TigersFuryTest() : TestCaseScript("spells druid tigers_fury") { }

    // "Increases damage done by 40 for 6s."
    class TigersFuryTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
            //Creature* inFront = SpawnCreature();

            Position spawnPosition(_location);
            spawnPosition.MoveInFront(_location, 3.0f);
            Creature* creature = SpawnCreatureWithPosition(spawnPosition);

            EQUIP_NEW_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP

            // Must be in cat form
            TEST_CAST(druid, creature, ClassSpells::Druid::TIGERS_FURY_RNK_4, SPELL_FAILED_ONLY_SHAPESHIFT);

            TEST_CAST(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_FULL_MASK);
            TEST_CAST(druid, creature, ClassSpells::Druid::TIGERS_FURY_RNK_4);

            // Test cd & aura duration
            TEST_HAS_COOLDOWN(druid, ClassSpells::Druid::TIGERS_FURY_RNK_4, Seconds(1));
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::TIGERS_FURY_RNK_4, Seconds(6));

            // Damage
            //uint32 const level = 60;
            float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
            float const armorFactor = GetArmorFactor(druid, creature);
            uint32 const tigersFuryBonus = 40;
            uint32 const APBonus = AP / 14;

            // Melee
            uint32 const catMeleeMin = (ClassSpellsDamage::Druid::GetCatMinDmg() + APBonus + tigersFuryBonus) * armorFactor;
            uint32 const catMeleeMax = (ClassSpellsDamage::Druid::GetCatMaxDmg() + APBonus + tigersFuryBonus) * armorFactor;
            TEST_MELEE_DAMAGE(druid, creature, BASE_ATTACK, catMeleeMin, catMeleeMax, false);
            druid->AddAura(ClassSpells::Druid::TIGERS_FURY_RNK_4, druid);
            TEST_MELEE_DAMAGE(druid, creature, BASE_ATTACK, catMeleeMin * 2.0f, catMeleeMax * 2.0f, true);

            // Shred
            float const shredFactor = 2.25f;
            uint32 const shredMinDamage = (ClassSpellsDamage::Druid::GetCatMinDmg() + APBonus + tigersFuryBonus) * shredFactor + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const shredMaxDamage = (ClassSpellsDamage::Druid::GetCatMaxDmg() + APBonus + tigersFuryBonus) * shredFactor + ClassSpellsDamage::Druid::SHRED_RNK_7;
            uint32 const expectedTigersFuryMin = shredMinDamage * armorFactor;
            uint32 const expectedTigersFuryMax = shredMaxDamage * armorFactor;
            druid->AddAura(ClassSpells::Druid::TIGERS_FURY_RNK_4, druid);
            TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::SHRED_RNK_7, expectedTigersFuryMin, expectedTigersFuryMax, false);

            // Energy cost
            uint32 const expectedTigersFuryEnergy = 30;
            TEST_POWER_COST(druid, ClassSpells::Druid::TIGERS_FURY_RNK_4, POWER_ENERGY, expectedTigersFuryEnergy);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<TigersFuryTestImpt>();
    }
};

class AbolishPoisonTest : public TestCaseScript
{
public:
    AbolishPoisonTest() : TestCaseScript("spells druid abolish_poison") { }

    class AbolishPoisonTestImpt : public TestCase
    {
    public:


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
            WaitNextUpdate();
            warrior->AddAura(MIND_NUMBING_POISON_III, warrior);
            WaitNextUpdate();
            warrior->AddAura(DEADLY_POISON_VII, warrior);
            WaitNextUpdate();

            // Mana cost
            TEST_CAST(druid, warrior, ClassSpells::Druid::ABOLISH_POISON_RNK_1);
            TEST_AURA_MAX_DURATION(warrior, ClassSpells::Druid::ABOLISH_POISON_RNK_1, Seconds(8));

            Wait(4500);
            TEST_HAS_NOT_AURA(warrior, WOUND_POISON_V);
            TEST_HAS_NOT_AURA(warrior, MIND_NUMBING_POISON_III);
            TEST_HAS_NOT_AURA(warrior, DEADLY_POISON_VII);

            // Mana cost
            uint32 const expectedAbolishPoisonMana = 308;
            TEST_POWER_COST(druid, ClassSpells::Druid::ABOLISH_POISON_RNK_1, POWER_MANA, expectedAbolishPoisonMana);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<AbolishPoisonTestImpt>();
    }
};

class CurePoisonTest : public TestCaseScript
{
public:
    CurePoisonTest() : TestCaseScript("spells druid cure_poison") { }

    class CurePoisonTestImpt : public TestCase
    {
    public:


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
            WaitNextUpdate();
            warrior->AddAura(MIND_NUMBING_POISON_III, warrior);
            WaitNextUpdate();
            warrior->AddAura(DEADLY_POISON_VII, warrior);
            WaitNextUpdate();

            TEST_CAST(druid, warrior, ClassSpells::Druid::CURE_POISON_RNK_1);
            TEST_ASSERT(2 == getPoisonCount(warrior, WOUND_POISON_V, MIND_NUMBING_POISON_III, DEADLY_POISON_VII));
            TEST_ASSERT(1 == getPoisonCount(warrior, WOUND_POISON_V, MIND_NUMBING_POISON_III, DEADLY_POISON_VII, true));
            TEST_ASSERT(0 == getPoisonCount(warrior, WOUND_POISON_V, MIND_NUMBING_POISON_III, DEADLY_POISON_VII, true));

            uint32 const expectedCurePoisonMana = 308;
            TEST_POWER_COST(druid, ClassSpells::Druid::CURE_POISON_RNK_1, POWER_MANA, expectedCurePoisonMana);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<CurePoisonTestImpt>();
    }
};

class GiftOfTheWildTest : public TestCaseScript
{
public:
    GiftOfTheWildTest() : TestCaseScript("spells druid gift_of_the_wild") { }

    class GiftOfTheWildTestImpt : public TestCase
    {
    public:


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

            TEST_POWER_COST(caster, spellId, POWER_MANA, manaCost);

            if (!victim->HasAura(spellId)) {
                caster->AddItem(reagentId, 1);
                TEST_ASSERT(caster->HasItemCount(reagentId, 1, false));
                TEST_CAST(caster, victim, spellId, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);
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

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<GiftOfTheWildTestImpt>();
    }
};

class HealingTouchTest : public TestCaseScript
{
public:
    HealingTouchTest() : TestCaseScript("spells druid healing_touch") { }

    class HealingTouchTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_NEW_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
            druid->DisableRegeneration(true);

            int32 maceBH = 550;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedHealingTouchMana = 935;
            TEST_POWER_COST(druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, POWER_MANA, expectedHealingTouchMana);

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

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<HealingTouchTestImpt>();
    }
};

class LifebloomTest : public TestCaseScript
{
public:
    LifebloomTest() : TestCaseScript("spells druid lifebloom") { }

    class LifebloomTestImpt : public TestCase
    {
    public:


        void TestLifebloom(TestPlayer* druid, uint32 expectedBloom, bool crit)
        {
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::LIFEBLOOM_RNK_1);

            auto AI = druid->GetTestingPlayerbotAI();
            ASSERT_INFO("Caster in not a testing bot");
            TEST_ASSERT(AI != nullptr);
            AI->ResetSpellCounters();
            EnableCriticals(druid, crit);

            TEST_CAST(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            Wait(8000);
            auto [dealtMin, dealtMax] = GetHealingPerSpellsTo(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1_FINAL_PROC, crit, 1);

            ASSERT_INFO("dealtMin %u == expectedBloom %u", dealtMin, expectedBloom);
            TEST_ASSERT(dealtMin == expectedBloom);
        }

        void Test() override
        {
            TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

            EQUIP_NEW_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH

            int32 maceBH = 550;
            TEST_ASSERT(druid->GetInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS) == maceBH);

            uint32 const expectedLifebloomMana = 220;
            TEST_POWER_COST(druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, POWER_MANA, expectedLifebloomMana);

            TEST_CAST(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, Seconds(7));
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::LIFEBLOOM_RNK_1);

            // Spell coeffs -- bug here, calculations below are on par with DrDamage
            uint32 const lifebloomTotalBHBonus = maceBH * ClassSpellsCoeff::Druid::LIFEBLOOM_HOT;
            uint32 const lifebloomBurstBHBonus = maceBH * ClassSpellsCoeff::Druid::LIFEBLOOM;

            // Tick
            uint32 const expectedLifebloomTick = floor((ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_TOTAL + lifebloomTotalBHBonus) / 7.0f);
            uint32 const expectedLifebloomTotal = 7 * expectedLifebloomTick;
            uint32 const expectedBloom = ClassSpellsDamage::Druid::LIFEBLOOM_RNK_1_BURST + lifebloomBurstBHBonus;
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::LIFEBLOOM_RNK_1, expectedLifebloomTotal, false);
            // Bloom
            TestLifebloom(druid, expectedBloom, false);
            TestLifebloom(druid, expectedBloom * 1.5f, true);

            // 2008/08/02
            // Lifebloom HoT generates threat at 0.25 threat per point healed, the final bloom of Lifebloom causes 0 threat.
            // https://authors.curseforge.com/forums/world-of-warcraft/addon-chat/libraries/209220-threat-2-0-and-lifebloom
            TestPlayer* warrior = SpawnPlayer(CLASS_WARRIOR, RACE_TAUREN);
            Creature* dummy = SpawnCreature();
            warrior->DisableRegeneration(true);
            warrior->SetHealth(1);
            warrior->AttackerStateUpdate(dummy, BASE_ATTACK);
            TEST_CAST(druid, warrior, ClassSpells::Druid::LIFEBLOOM_RNK_1);
            Wait(Seconds(8));
            warrior->AttackStop();

            const float expectedDruidThreat = 7 * expectedLifebloomTick / 0.25f;
            const float expectedWarriorThreat = dummy->GetMaxHealth() - dummy->GetHealth();
            ASSERT_INFO("Druid threat: %f, expected: %f", dummy->GetThreatManager().GetThreat(druid), expectedDruidThreat);
            const float druidThreat = dummy->GetThreatManager().GetThreat(druid);
            TEST_ASSERT(Between<float>(druidThreat, druidThreat - 0.1f, druidThreat + 0.1f));

            const float warriorthreat = dummy->GetThreatManager().GetThreat(warrior);
            ASSERT_INFO("Warrior threat: %f, expected: %f", dummy->GetThreatManager().GetThreat(warrior), expectedWarriorThreat);
            TEST_ASSERT(Between<float>(warriorthreat, warriorthreat - 0.1f, warriorthreat + 0.1f));
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<LifebloomTestImpt>();
    }
};

class MarkOfTheWildTest : public TestCaseScript
{
public:
    MarkOfTheWildTest() : TestCaseScript("spells druid mark_of_the_wild") { }

    class MarkOfTheWildTestImpt : public TestCase
    {
    public:


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
            TEST_POWER_COST(caster, spellId, POWER_MANA, manaCost);

            TEST_CAST(caster, victim, spellId, SPELL_CAST_OK, TRIGGERED_IGNORE_GCD);

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

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<MarkOfTheWildTestImpt>();
    }
};

class RebirthTest : public TestCaseScript
{
public:
    RebirthTest() : TestCaseScript("spells druid rebirth") { }

    class RebirthTestImpt : public TestCase
    {
    public:


        void TestRebirth(TestPlayer* caster, TestPlayer* victim, uint32 spellId, uint32 manaCost, uint32 reagentId, uint32 expectedHealth, uint32 expectedMana, bool fail = false)
        {
            victim->KillSelf(true);
            caster->GetSpellHistory()->ResetAllCooldowns();
            caster->AddItem(reagentId, 1);
            caster->SetFullPower(POWER_MANA);
            TEST_CAST(caster, victim, spellId, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
            TEST_COOLDOWN(caster, victim, spellId, Minutes(20));
            WaitNextUpdate();
            victim->RessurectUsingRequestData();
            WaitNextUpdate(); //resurrect needs 1 update to be done
            if (!fail)
            {
                ASSERT_INFO("Victim has %u instead of expected %u health", victim->GetHealth(), expectedHealth);
                TEST_ASSERT(victim->GetHealth() == expectedHealth);
                ASSERT_INFO("Victim has %u instead of expected %u mana", victim->GetPower(POWER_MANA), expectedMana);
                TEST_ASSERT(victim->GetPower(POWER_MANA) == expectedMana);
            }
            else 
            {
                ASSERT_INFO("Victim was resurrected but spell should have failed");
                TEST_ASSERT(victim->IsDead());
            }

            TEST_POWER_COST(caster, spellId, POWER_MANA, manaCost);
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
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_2, manaCost, STRANGLETHRON_SEED, 750, 1200);
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_3, manaCost, ASHWOOD_SEED, 1100, 1700);
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_4, manaCost, HORNBEAM_SEED, 1600, 2200);
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_5, manaCost, IRONWOOD_SEED, 2200, 2800);
            TestRebirth(druid, ally, ClassSpells::Druid::REBIRTH_RNK_6, manaCost, FLINTWEED_SEED, 3200, 3200);

            /*WoWWiki: Note that there is no way to tell whether or not a corpse belongs to the player's faction except by inspecting
            it visually. If the corpse does belong to the player's faction, it is possible to tell whether the player is still online
            or not by typing "/who playername." A response of "0 players found" indicates that the player has logged off (or is of the
            wrong faction). A response giving the target's race, level, and class indicates that the player is still online and can be
            successfully resurrected. Note that landing a resurrection on an offline or cross-faction player's corpses is
            indistinguishable from landing one on an online player's corpse who simply declines the resurrection. In both cases,
            the mana is wasted, and resurrection will not take place.*/
            //-> Should not work crossfaction
            TestRebirth(druid, enemy, ClassSpells::Druid::REBIRTH_RNK_6, manaCost, FLINTWEED_SEED, 3200, 3200, true);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RebirthTestImpt>();
    }
};

class RejuvenationTest : public TestCaseScript
{
public:
	RejuvenationTest() : TestCaseScript("spells druid rejuvenation") { }

	class RejuvenationTestImpt : public TestCase
	{
	public:


		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

			EQUIP_NEW_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
			druid->DisableRegeneration(true);

            int32 maceBH = 550;
			TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

			// Mana cost
			uint32 const expectedRejuvenationMana = 415;
			TEST_POWER_COST(druid, ClassSpells::Druid::REJUVENATION_RNK_13, POWER_MANA, expectedRejuvenationMana);

			// Spell coefficient
            uint32 const rejuvenationTickAmount = 4;
			float const rejuvenationDuration = 12.0f;
			float const rejuvenationSpellCoeff = rejuvenationDuration / 15.0f;
			uint32 const rejuvenationTickBHBonus = maceBH * rejuvenationSpellCoeff / rejuvenationTickAmount;

			uint32 const expectedRejuvenationTotal = rejuvenationTickAmount * (ClassSpellsDamage::Druid::REJUVENATION_RNK_13_TICK + rejuvenationTickBHBonus);
			TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REJUVENATION_RNK_13, expectedRejuvenationTotal, true);
		}
	};

	std::unique_ptr<TestCase> GetTest() const override
	{
		return std::make_unique<RejuvenationTestImpt>();
	}
};

class RegrowthTest : public TestCaseScript
{
public:
    RegrowthTest() : TestCaseScript("spells druid regrowth") { }

    // Heals a friendly target for 1215 to 1356 and another 1274 over 21sec.
    class RegrowthTestImpt : public TestCase
    {
        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_NEW_ITEM(druid, 34335); // Hammer of Sanctification - 550 BH
            druid->DisableRegeneration(true);
            Wait(1500);

            int32 maceBH = 550;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedRegrowthMana = 675;
            TEST_POWER_COST(druid, ClassSpells::Druid::REGROWTH_RNK_10, POWER_MANA, expectedRegrowthMana);

            TEST_CAST(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

            // Aura
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::REGROWTH_RNK_10, Seconds(21));
            druid->RemoveAurasDueToSpell(ClassSpells::Druid::REGROWTH_RNK_10);

            // Spell coeffs
            uint32 const regrowthTickBHBonus = maceBH * ClassSpellsCoeff::Druid::REGROWTH_HOT;
            uint32 const regrowthDirectBHBonus = maceBH * ClassSpellsCoeff::Druid::REGROWTH;

            // Tick
            uint32 const expectedRegrowthTick = ClassSpellsDamage::Druid::REGROWTH_RNK_10_TICK + regrowthTickBHBonus;
            uint32 const expectedRegrowthTotal = 7 * expectedRegrowthTick;
            TEST_DOT_DAMAGE(druid, druid, ClassSpells::Druid::REGROWTH_RNK_10, expectedRegrowthTotal, false);
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

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RegrowthTestImpt>();
    }
};

class RemoveCurseTest : public TestCaseScript
{
public:
    RemoveCurseTest() : TestCaseScript("spells druid remove_curse") { }

    class RemoveCurseTestImpt : public TestCase
    {
    public:


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
            TEST_POWER_COST(druid, ClassSpells::Druid::REMOVE_CURSE_RNK_1, POWER_MANA, expectedRemoveCurseMana);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<RemoveCurseTestImpt>();
    }
};

class TranquilityTest : public TestCaseScript
{
public:
    TranquilityTest() : TestCaseScript("spells druid tranquility") { }

    class TranquilityTestImpt : public TestCase
    {
    public:


        void Test() override
        {
            TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);

            EQUIP_NEW_ITEM(druid, 32500); // Crystal Spire of Karabor - 486 BH
            druid->DisableRegeneration(true);
            Wait(1500);

            int32 maceBH = 486;
            TEST_ASSERT(druid->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);

            // Mana cost
            uint32 const expectedTranquilityMana = 1650;
            TEST_POWER_COST(druid, ClassSpells::Druid::TRANQUILITY_RNK_5, POWER_MANA, expectedTranquilityMana);

            TEST_CAST(druid, druid, ClassSpells::Druid::TRANQUILITY_RNK_5);
            WaitNextUpdate();
            TEST_AURA_MAX_DURATION(druid, ClassSpells::Druid::TRANQUILITY_RNK_5, Seconds(8));

            // Spell coeffs
            float const TranquilityCastTime = 8.0f;
            //WoWiki says: The Tranquility coefficient scales with level, at level 52, it has a base coefficient of 26%, where as at 70, it has a base coefficient of 73%. Tested on March 4th, 2008.
            //DrDamage says 0.762 for 70
            float const TranquilityCoeff = 0.762f;
            uint32 const TranquilityBHBonus = maceBH * TranquilityCoeff;
            uint32 const TranquilityTickBHBonus = floor(TranquilityBHBonus / 4.0f);

            uint32 const expectedTranquilityTick = ClassSpellsDamage::Druid::TRANQUILITY_RNK_5_TICK + TranquilityTickBHBonus;
            TEST_CHANNEL_HEALING(druid, druid, ClassSpells::Druid::TRANQUILITY_RNK_5, 4, expectedTranquilityTick, ClassSpells::Druid::TRANQUILITY_RNK_5_PROC);
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<TranquilityTestImpt>();
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
