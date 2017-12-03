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

void AddSC_test_spells_druid()
{
	// Total:
	// Balance:
	new BarkskinTest();
	new CycloneTest();
	new EntanglingRootsTest();
	new FaerieFireTest();
	new HibernateTest();
	new HurricaneTest();
	new InnervateTest();
	new StarfireTest();
	new WrathTest();
}
