#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class BarkskinTest : public TestCaseScript
{
public:
	BarkskinTest() : TestCaseScript("spells druid barkskin") { }

	class BarkskinTestImpt : public TestCase
	{
	public:
		BarkskinTestImpt() : TestCase(true) { }

		void CastBarkskin(TestPlayer* druid)
		{
			druid->RemoveAllSpellCooldown();
			CastSpell(druid, druid, ClassSpells::Druid::BARKSKIN_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
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
			uint32 staffSP = 266;
			TEST_ASSERT(druid2->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			druid->DisableRegeneration(true);

			// Mana cost
			uint32 const expectedBarkskinMana = 284;
			TEST_POWER_COST(druid, druid, ClassSpells::Druid::BARKSKIN_RNK_1, 1500, POWER_MANA, expectedBarkskinMana);
			druid->RemoveAurasDueToSpell(ClassSpells::Druid::BARKSKIN_RNK_1);

			// Duration & CD
			CastBarkskin(druid);
			Aura* aura = druid->GetAura(ClassSpells::Druid::BARKSKIN_RNK_1, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetAuraDuration() == 12 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(druid->GetSpellCooldownDelay(ClassSpells::Druid::BARKSKIN_RNK_1) == 1 * MINUTE);

			// Pushback
			uint32 startHealth = 2500;
			druid->SetHealth(startHealth);
			CastSpell(druid2, druid2, ClassSpells::Druid::CAT_FORM_RNK_1);
			druid2->Attack(druid, true);
			CastSpell(druid, druid, ClassSpells::Druid::HEALING_TOUCH_RNK_13, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
			Wait(3400);
			TEST_ASSERT(druid->GetHealth() < startHealth);
			Wait(100);
			TEST_ASSERT(druid->GetHealth() == druid->GetMaxHealth());
			druid2->AttackStop();

			// Prep damage tests
			druid->SetMaxHealth(100000000);
			druid->SetHealth(druid->GetMaxHealth());

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
			CastSpell(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
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
		CycloneTestImpt() : TestCase(true) { }

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
			CastSpell(druid, druid4, ClassSpells::Druid::CYCLONE_RNK_1, SPELL_FAILED_BAD_TARGETS);

			// Cooldown & invulnerable to damage & heals
			CastSpell(druid2, druid2, ClassSpells::Druid::LIFEBLOOM_RNK_1);
			Wait(3500);
			CastSpell(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
			uint32 const startHealth = druid2->GetHealth();
			Aura* aura = druid2->GetAura(ClassSpells::Druid::CYCLONE_RNK_1, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetAuraDuration() == 6 * SECOND * IN_MILLISECONDS);
			CastSpell(druid, druid2, ClassSpells::Druid::WRATH_RNK_10);
			Wait(500); // wrath hit
			TEST_ASSERT(druid2->GetHealth() == startHealth);
			Wait(2000); // no more lifebloom
			TEST_ASSERT(druid2->GetHealth() == startHealth);

			// Only one target can be affected
			CastSpell(druid, druid3, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
			TEST_ASSERT(!druid2->HasAura(ClassSpells::Druid::CYCLONE_RNK_1));
			TEST_ASSERT(druid3->HasAura(ClassSpells::Druid::CYCLONE_RNK_1));

			// Diminishing returns
			// 3s
			CastSpell(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
			aura = druid2->GetAura(ClassSpells::Druid::CYCLONE_RNK_1, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetAuraDuration() == 3 * SECOND * IN_MILLISECONDS);
			druid2->RemoveAurasDueToSpell(ClassSpells::Druid::CYCLONE_RNK_1);

			// 1.5s
			CastSpell(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
			aura = druid2->GetAura(ClassSpells::Druid::CYCLONE_RNK_1, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetAuraDuration() == 1500);

			// Immune
			CastSpell(druid, druid2, ClassSpells::Druid::CYCLONE_RNK_1);
			Wait(1500); // Cyclone cast time
			TEST_ASSERT(!druid2->HasAura(ClassSpells::Druid::CYCLONE_RNK_1));

			// Mana cost
			uint32 const expectedCycloneMana = 189;
			TEST_POWER_COST(druid, druid3, ClassSpells::Druid::CYCLONE_RNK_1, 1500, POWER_MANA, 189);
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
		EntanglingRootsTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedEntanglingRootsMana = 160;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, 1500, POWER_MANA, expectedEntanglingRootsMana);

			// Is rooted
			TEST_ASSERT(creature->IsInRoots());

			// Spell coefficient
			float const starfireSpellCoeff = 27.0f / 15.0f / 9.0f;
			uint32 const starfireBonusSP = starfireSpellCoeff * staffSP;

			// Damage
			float const EntanglingRootsTick = ClassSpellsDamage::Druid::ENTANGLING_ROOTS_RNK_7_TOTAL / 9;
			uint32 const expectedEntanglingRootsTick = floor(EntanglingRootsTick + starfireBonusSP);
			uint32 const expectedEntanglingRootsDmg = expectedEntanglingRootsTick * 9;
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::ENTANGLING_ROOTS_RNK_7, expectedEntanglingRootsDmg);

			// TODO: Aura duration
			// TODO: Diminishing returns
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

	FaerieFireTest() : TestCaseScript("talents spells faerie_feral") { }

	class FaerieFireTestImpt : public TestCase
	{
	public:
		FaerieFireTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);
			TestPlayer* mage = SpawnPlayer(CLASS_MAGE, RACE_HUMAN, 70, spawnPosition);

			EQUIP_ITEM(rogue, 34211); // S4 Chest
			int32 const expectedRogueArmor = rogue->GetArmor() - 610;

			// Faerie Fire 
			CastSpell(player, rogue, ClassSpells::Druid::FAERIE_FIRE_RNK_5);
			Aura* aura = rogue->GetAura(ClassSpells::Druid::FAERIE_FIRE_RNK_5, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			ASSERT_INFO("Rogue has %u armor, expected: %i", rogue->GetArmor(), expectedRogueArmor);
			TEST_ASSERT(rogue->GetArmor() == expectedRogueArmor);
			TEST_ASSERT(aura->GetAuraDuration() == 40 * SECOND * IN_MILLISECONDS);
			Wait(2000);

			// Rogue can't stealth
			CastSpell(rogue, rogue, ClassSpells::Rogue::STEALTH_RNK_4, SPELL_FAILED_CASTER_AURASTATE);

			// Mage can't invisible
			uint32 expectedFaerieFireMana = 145;
			TEST_POWER_COST(player, mage, ClassSpells::Druid::FAERIE_FIRE_RNK_5, 1500, POWER_MANA, expectedFaerieFireMana);
			CastSpell(mage, mage, ClassSpells::Mage::INVISIBILITY_RNK_1, SPELL_FAILED_CASTER_AURASTATE);
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

	HibernateTest() : TestCaseScript("talents spells hibernate") { }

	class HibernateTestImpt : public TestCase
	{
	public:
		HibernateTestImpt() : TestCase(true) { }

		void TestDuration(TestPlayer* druid, Unit* enemy, float duration)
		{
			while (!enemy->HasAura(ClassSpells::Druid::HIBERNATE_RNK_3))
				CastSpell(druid, enemy, ClassSpells::Druid::HIBERNATE_RNK_3, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);

			TEST_ASSERT(enemy->HasAura(ClassSpells::Druid::HIBERNATE_RNK_3));
			Aura* aura = enemy->GetAura(ClassSpells::Druid::HIBERNATE_RNK_3, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			uint32 expectedAuraDuration = duration * SECOND * IN_MILLISECONDS;
			ASSERT_INFO("Hibernate lasts %i, expected %us", aura->GetAuraDuration(), expectedAuraDuration);
			TEST_ASSERT(aura->GetAuraDuration() == expectedAuraDuration);
			enemy->RemoveAurasDueToSpell(ClassSpells::Druid::HIBERNATE_RNK_3);
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* beast = SpawnCreature(26, true); // beast dummy
			Creature* dragonkin = SpawnCreature(20, true); // dragonkin dummy

			// PvE
			TestDuration(druid, beast, 40);

			// Only one target at a time
			TestDuration(druid, dragonkin, 40);
			TEST_ASSERT(!beast->HasAura(ClassSpells::Druid::HIBERNATE_RNK_3));

			Wait(5000);

			// PvP
			TestPlayer* enemy = SpawnPlayer(CLASS_DRUID, RACE_NIGHTELF, 70, spawnPosition);
			CastSpell(enemy, enemy, ClassSpells::Druid::CAT_FORM_RNK_1);
			Wait(1000);
			// Diminishing return
			TestDuration(druid, enemy, 10.0f); // 10s
			TestDuration(druid, enemy, 5.0f); // 5s
			TestDuration(druid, enemy, 2.5f); // 2.5s
			// Immune
			uint32 expectedHibernateMana = 150;
			TEST_POWER_COST(druid, enemy, ClassSpells::Druid::HIBERNATE_RNK_3, 1500, POWER_MANA, expectedHibernateMana);
			TEST_ASSERT(!enemy->HasAura(ClassSpells::Druid::HIBERNATE_RNK_3));
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
		HurricaneTestImpt() : TestCase(true) { }

		void CastHurricane(TestPlayer* druid, Unit* target)
		{
			druid->RemoveAllSpellCooldown();
			CastSpell(druid, target, ClassSpells::Druid::HURRICANE_RNK_4, SPELL_CAST_OK, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
		}

		void Test() override
		{
			// Init barkskin druid
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			// Init rogue
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			druid->DisableRegeneration(true);

			// Mana cost
			uint32 const expectedHurricaneMana = 1905;
			TEST_POWER_COST(druid, rogue, ClassSpells::Druid::HURRICANE_RNK_4, 1500, POWER_MANA, expectedHurricaneMana);

			// Duration & CD & +25% melee speed

			uint32 rogueAttackSpeed = rogue->GetAttackTimer(BASE_ATTACK);
			CastHurricane(druid, rogue);
			SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ClassSpells::Druid::HURRICANE_RNK_4);
			TEST_ASSERT(spellInfo != nullptr);
			ASSERT_INFO("Duration: %i", spellInfo->GetDuration());
			Wait(100);
			TEST_ASSERT(spellInfo->GetDuration() == 10 * SECOND *  IN_MILLISECONDS);
			TEST_ASSERT(druid->GetSpellCooldownDelay(ClassSpells::Druid::HURRICANE_RNK_4) == 1 * MINUTE);
			uint32 expectedAttackSpeed = rogueAttackSpeed * 1.25f;
			ASSERT_INFO("WS: %u, expected:%u", rogue->GetAttackTimer(BASE_ATTACK), expectedAttackSpeed);
			TEST_ASSERT(rogue->GetAttackTimer(BASE_ATTACK) == expectedAttackSpeed);

			// Damage
			float const hurricaneSpellCoeff = 10 / 3.5 / 2 / 10;
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
		InnervateTestImpt() : TestCase(true) { }

		float CalculateInnervateSpiritRegen(TestPlayer* player)
		{
			uint32 level = player->GetLevel();
			uint32 pclass = player->GetClass();

			if (level>GT_MAX_LEVEL)
				level = GT_MAX_LEVEL;

			GtRegenMPPerSptEntry const *moreRatio = sGtRegenMPPerSptStore.LookupEntry((pclass - 1)*GT_MAX_LEVEL + level - 1);
			TEST_ASSERT(moreRatio != nullptr);

			float const innervateFactor = 5.0f;
			float const spirit = player->GetStat(STAT_SPIRIT) * innervateFactor;
			float const regen = spirit * moreRatio->ratio;
			return regen;
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			uint32 totalMana = 10000;
			druid->SetMaxPower(POWER_MANA, totalMana);

			uint32 const regenTick = 2.0f * sqrt(druid->GetStat(STAT_INTELLECT)) * druid->OCTRegenMPPerSpirit();
			uint32 const expectedInnervateRegenTick = 2.0f * sqrt(druid->GetStat(STAT_INTELLECT)) * CalculateInnervateSpiritRegen(druid);

			// Power cost
			uint32 expectedInnervateMana = 94;
			druid->SetPower(POWER_MANA, expectedInnervateMana);
			CastSpell(druid, druid, ClassSpells::Druid::INNERVATE_RNK_1);
			TEST_ASSERT(druid->GetPower(POWER_MANA) == 0);

			// Duration & CD
			TEST_ASSERT(druid->HasAura(ClassSpells::Druid::INNERVATE_RNK_1));
			Aura* aura = druid->GetAura(ClassSpells::Druid::INNERVATE_RNK_1, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetAuraDuration() == 20 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(druid->GetSpellCooldownDelay(ClassSpells::Druid::INNERVATE_RNK_1) == 6 * MINUTE);

			// Mana regen
			TEST_ASSERT(druid->GetPower(POWER_MANA) == 0);
			Wait(18500);
			uint32 expectedMana = 10 * expectedInnervateRegenTick;
			ASSERT_INFO("Mana: %u, expected: %u", druid->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(druid->GetPower(POWER_MANA) == expectedMana);
			Wait(2000);
			TEST_ASSERT(!druid->HasAura(ClassSpells::Druid::INNERVATE_RNK_1));
			expectedMana = 10 * expectedInnervateRegenTick + regenTick;
			ASSERT_INFO("Mana: %u, expected: %u", druid->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(druid->GetPower(POWER_MANA) == expectedMana);
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
		MoonfireTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedMoonfireMana = 495;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, 1500, POWER_MANA, expectedMoonfireMana);

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
			TEST_DOT_DAMAGE(druid, creature, ClassSpells::Druid::MOONFIRE_RNK_12, expectedMoonfireTotalDmg);
			/*
			(Duration / 15) / ((Duration / 15) + (Cast Time of Spell / 3.5)) = Portion to Over Time
			The standard portion receives the rest:

			1 - Portion to Over Time = Portion to Standard 
			The duration and cast time limits are then applied:

			(Cast Time of Spell / 3.5) * Portion to Standard = Total bonus to Standard Portion 
			(Duration / 15) * Portion to Over Time = Total Bonus to Over Time Portion 
			*/

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
		StarfireTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedStarfireMana = 370;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::STARFIRE_RNK_8, 3500, POWER_MANA, expectedStarfireMana);

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
		ThornsTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN);

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedThornsMana = 400;
			TEST_POWER_COST(druid, druid, ClassSpells::Druid::THORNS_RNK_7, 1500, POWER_MANA, expectedThornsMana);

			// Damage
			uint32 const thornsDmg = 25;
			uint32 expectedRogueHealth = rogue->GetHealth() - thornsDmg;
			rogue->Attack(druid, true);
			Wait(500);
			TEST_ASSERT(rogue->GetHealth() == expectedRogueHealth);
			rogue->AttackStop();
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
		WrathTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnRandomPlayer(CLASS_DRUID);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 34182); // Grand Magister's Staff of Torrents - 266 SP
			druid->DisableRegeneration(true);

			uint32 staffSP = 266;
			TEST_ASSERT(druid->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL) == staffSP);

			// Mana cost
			uint32 const expectedWrathMana = 255;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::WRATH_RNK_10, 2000, POWER_MANA, expectedWrathMana);

			// Spell coefficient
			float const wrathSpellCoeff = ClassSpellsCoeff::Druid::WRATH;
			uint32 const wrathBonusSP = wrathSpellCoeff * staffSP;

			// Damage
			uint32 const expectedWrathMinDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MIN + wrathBonusSP;
			uint32 const expectedWrathMaxDmg = ClassSpellsDamage::Druid::WRATH_RNK_10_MAX + wrathBonusSP;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::WRATH_RNK_10, expectedWrathMinDmg, expectedWrathMaxDmg, false);

			// Crit damage
			uint32 const expectedWrathCritMinDmg = expectedWrathMinDmg * 1.5f;
			uint32 const expectedWrathCritMaxDmg = expectedWrathMaxDmg * 1.5f;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::WRATH_RNK_10, expectedWrathCritMinDmg, expectedWrathCritMaxDmg, true);
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
		BashTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			TestPlayer* rogue = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 70, spawnPosition);

			CastSpell(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(1500); // GCD

			// Rage & aura duration
			uint32 const expectedBashRage = 10 * 10;
			while (!rogue->HasAura(ClassSpells::Druid::BASH_RNK_3))
			{
				druid->SetPower(POWER_RAGE, expectedBashRage);
				CastSpell(druid, rogue, ClassSpells::Druid::BASH_RNK_3);
			}
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 0);
			Aura* aura = rogue->GetAura(ClassSpells::Druid::BASH_RNK_3, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetAuraDuration() == 4 * SECOND * IN_MILLISECONDS);
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
		ChallengingRoarTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			Position spawn3m(_location);
			spawn3m.MoveInFront(_location, 3.0f);
			TestPlayer* player3m = SpawnPlayer(CLASS_ROGUE, RACE_HUMAN, 1, spawn3m);
			Creature* creature3m = SpawnCreatureWithPosition(spawn3m, 6);

			Position spawn6m(_location);
			spawn6m.MoveInFront(_location, 6.0f);
			TestPlayer* player6m = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 1, spawn6m);
			Creature* creature6m = SpawnCreatureWithPosition(spawn6m, 6);

			Position spawn11m(_location);
			spawn11m.MoveInFront(_location, 15.0f);
			TestPlayer* player11m = SpawnPlayer(CLASS_ROGUE, RACE_BLOODELF, 1, spawn11m);
			Creature* creature11m = SpawnCreatureWithPosition(spawn11m, 6);

			// Setup
			player3m->Attack(creature3m, true);
			player6m->Attack(creature6m, true);
			player11m->Attack(creature11m, true);

			CastSpell(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(5000);

			// Rage cost
			uint32 const expectedChallengingRoarRage = 15 * 10;
			druid->SetPower(POWER_RAGE, expectedChallengingRoarRage);
			CastSpell(druid, druid, ClassSpells::Druid::CHALLENGING_ROAR_RNK_1);
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 0);

			// Cooldown
			TEST_ASSERT(druid->GetSpellCooldownDelay(ClassSpells::Druid::CHALLENGING_ROAR_RNK_1) == 10 * MINUTE);

			// Aura
			Aura* aura3m = creature3m->GetAura(ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, EFFECT_0);
			TEST_ASSERT(aura3m != nullptr);
			Aura* aura6m = creature6m->GetAura(ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, EFFECT_0);
			TEST_ASSERT(aura6m != nullptr);
			Aura* aura11m = creature11m->GetAura(ClassSpells::Druid::CHALLENGING_ROAR_RNK_1, EFFECT_0);
			TEST_ASSERT(aura11m == nullptr);

			// Aura duration
			TEST_ASSERT(aura3m->GetAuraDuration() == 6 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(aura6m->GetAuraDuration() == 6 * SECOND * IN_MILLISECONDS);
		
			// Target changed
			TEST_ASSERT(creature3m->GetTarget() == druid->GetGUID());
			TEST_ASSERT(creature6m->GetTarget() == druid->GetGUID());

			// Back to original target
			Wait(6500);
			TEST_ASSERT(creature3m->GetTarget() == player3m->GetGUID());
			TEST_ASSERT(creature6m->GetTarget() == player6m->GetGUID());
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
		ClawTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			EQUIP_ITEM(druid, 30883); // Pillar of Ferocity -- 1059 AP
			CastSpell(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1);
			Wait(1500); // GCD

			// Energy cost
			uint32 const expectedClawEnergy = 45;
			TEST_POWER_COST(druid, creature, ClassSpells::Druid::CLAW_RNK_6, 100, POWER_ENERGY, expectedClawEnergy);

			// Combo point added
			TEST_ASSERT(druid->GetComboPoints(creature) == 1);

			// Damage
			float const AP = druid->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const catAttackSpeed = 1.0f;
			uint32 const weaponMinDamage = (AP / 14) * catAttackSpeed + ClassSpellsDamage::Druid::CLAW_RNK_6;
			uint32 const weaponMaxDamage = (AP / 14) * catAttackSpeed + ClassSpellsDamage::Druid::CLAW_RNK_6;
			uint32 const expectedClawMin = weaponMinDamage * armorFactor;
			uint32 const expectedClawMax = weaponMaxDamage * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(druid, creature, ClassSpells::Druid::CLAW_RNK_6, expectedClawMin, expectedClawMax, false);

			uint32 const expectedClawCritMin = weaponMinDamage * 1.5f * armorFactor;
			uint32 const expectedClawCritMax = weaponMaxDamage * 1.5f * armorFactor;
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
		CowerTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);
			Creature* creature = SpawnCreature();

			CastSpell(druid, druid, ClassSpells::Druid::CAT_FORM_RNK_1);
			druid->Attack(creature, true);
			Wait(10000);
			druid->AttackStop();

			uint32 cowerPoints = 1170;
			float const threat = creature->GetThreat(druid);
			float const expectedThreat = threat - cowerPoints;
			uint32 expectedCowerEnergy = 20;

			druid->SetPower(POWER_ENERGY, expectedCowerEnergy);
			CastSpell(druid, creature, ClassSpells::Druid::COWER_RNK_5);
			TEST_ASSERT(druid->GetPower(POWER_ENERGY) == 0);
			ASSERT_INFO("Before: %f, current: %f, expected: %f", threat, creature->GetThreat(druid), expectedThreat);
			TEST_ASSERT(creature->GetThreat(druid) == expectedThreat);
			TEST_ASSERT(druid->GetSpellCooldownDelay(ClassSpells::Druid::COWER_RNK_5) == 10 * SECOND * IN_MILLISECONDS);
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
		DemoralizingRoarTestImpt() : TestCase(true) { }

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
			int32 expectedAP3m = (startAP3m - 240 > 0) ? startAP3m - 240 : 0;
			int32 expectedAP6m = (startAP6m - 240 > 0) ? startAP6m - 240 : 0;

			CastSpell(druid, druid, ClassSpells::Druid::BEAR_FORM_RNK_1);
			Wait(5000);

			// Rage cost
			uint32 const expectedDemoralizingRoarRage = 10 * 10;
			druid->SetPower(POWER_RAGE, expectedDemoralizingRoarRage);
			CastSpell(druid, druid, ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6);
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 0);

			// Aura
			Aura* aura3m = player3m->GetAura(ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, EFFECT_0);
			TEST_ASSERT(aura3m != nullptr);
			Aura* aura6m = creature6m->GetAura(ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, EFFECT_0);
			TEST_ASSERT(aura6m != nullptr);
			Aura* aura15m = creature15m->GetAura(ClassSpells::Druid::DEMORALIZING_ROAR_RNK_6, EFFECT_0);
			TEST_ASSERT(aura15m == nullptr);

			// Aura duration
			TEST_ASSERT(aura3m->GetAuraDuration() == 30 * SECOND * IN_MILLISECONDS);
			TEST_ASSERT(aura6m->GetAuraDuration() == 30 * SECOND * IN_MILLISECONDS);

			// AP loss
			TEST_ASSERT(player3m->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP3m);
			TEST_ASSERT(creature6m->GetTotalAttackPowerValue(BASE_ATTACK) == expectedAP6m);

			// Back to original target
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
		EnrageTestImpt() : TestCase(true) { }

		void TestEnrage(TestPlayer* druid, uint32 spellFormId, float armorReduction)
		{
			druid->RemoveAllSpellCooldown();
			CastSpell(druid, druid, spellFormId, SPELL_CAST_OK, TRIGGERED_CAST_DIRECTLY);
			uint32 expectedArmor = druid->GetArmor() * (1 - armorReduction);
			CastSpell(druid, druid, ClassSpells::Druid::ENRAGE_RNK_1);
			ASSERT_INFO("Form: %u, armor: %u, expected: %u", spellFormId, druid->GetArmor(), expectedArmor);
			TEST_ASSERT(druid->GetArmor() == expectedArmor);
			Aura* aura = druid->GetAura(ClassSpells::Druid::ENRAGE_RNK_1, EFFECT_0);
			TEST_ASSERT(aura->GetAuraDuration() == 10 * SECOND * IN_MILLISECONDS);
			Wait(2000);
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 4 * 10);
			Wait(2000);
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 8 * 10);
			Wait(6000);
			TEST_ASSERT(druid->GetPower(POWER_RAGE) == 17 * 10); // Upon reaching 20, starts to decrease by 3
			druid->RemoveAurasDueToSpell(spellFormId);
		}

		void Test() override
		{
			TestPlayer* druid = SpawnPlayer(CLASS_DRUID, RACE_TAUREN);

			TestEnrage(druid, ClassSpells::Druid::BEAR_FORM_RNK_1, 0.27f);
			TestEnrage(druid, ClassSpells::Druid::DIRE_BEAR_FORM_RNK_2, 0.16f);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<EnrageTestImpt>();
	}
};

void AddSC_test_spells_druid()
{
	// Total:
	// Balance: 11/12
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
	// Feral: 3/
	new BashTest();
	new ChallengingRoarTest();
	new ClawTest();
	new DemoralizingRoarTest();
	new EnrageTest();
	new CowerTest();
}
