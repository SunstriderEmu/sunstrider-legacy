#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

class DivineStrengthTest : public TestCaseScript
{
public:
	DivineStrengthTest() : TestCaseScript("talents paladin divine_strength") { }

	class DivineStrengthTestImpt : public TestCase
	{
	public:
		DivineStrengthTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const startStr = player->GetStat(STAT_STRENGTH);
			uint32 const expectedStr = startStr * 1.1f;

			LearnTalent(player, Talents::Paladin::DIVINE_STRENGTH_RNK_5);
			TEST_ASSERT(Between<float>(player->GetStat(STAT_STRENGTH), expectedStr - 1, expectedStr + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DivineStrengthTestImpt>();
	}
};

class DivineIntellectTest : public TestCaseScript
{
public:
	DivineIntellectTest() : TestCaseScript("talents paladin divine_intellect") { }

	class DivineIntellectTestImpt : public TestCase
	{
	public:
		DivineIntellectTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const startInt = player->GetStat(STAT_INTELLECT);
			uint32 const expectedInt = startInt * 1.1f;

			LearnTalent(player, Talents::Paladin::DIVINE_INTELLECT_RNK_5);
			TEST_ASSERT(Between<float>(player->GetStat(STAT_INTELLECT), expectedInt - 1, expectedInt + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DivineIntellectTestImpt>();
	}
};

class HealingLightTest : public TestCaseScript
{
public:
	HealingLightTest() : TestCaseScript("talents paladin healing_light") { }

	class ImprovedSealOfRighteousnessTestImpt : public TestCase
	{
	public:
		ImprovedSealOfRighteousnessTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const bh = player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL);
			uint32 const expectedFoLMin = ClassSpellsDamage::Paladin::FLASH_OF_LIGHT_RNK_7_MIN * 1.12f + bh * ClassSpellsCoeff::Paladin::FLASH_OF_LIGHT;
			uint32 const expectedFoLMax = ClassSpellsDamage::Paladin::FLASH_OF_LIGHT_RNK_7_MAX * 1.12f + bh * ClassSpellsCoeff::Paladin::FLASH_OF_LIGHT;
			uint32 const expectedHLMin = ClassSpellsDamage::Paladin::HOLY_LIGHT_RNK_11_MIN * 1.12f + bh * ClassSpellsCoeff::Paladin::HOLY_LIGHT;
			uint32 const expectedHLMax = ClassSpellsDamage::Paladin::HOLY_LIGHT_RNK_11_MAX * 1.12f + bh * ClassSpellsCoeff::Paladin::HOLY_LIGHT;

			LearnTalent(player, Talents::Paladin::HEALING_LIGHT_RNK_3);
			TEST_DIRECT_HEAL(player, player, ClassSpells::Paladin::FLASH_OF_LIGHT_RNK_7, expectedFoLMin, expectedFoLMax);
			TEST_DIRECT_HEAL(player, player, ClassSpells::Paladin::HOLY_LIGHT_RNK_11, expectedHLMin, expectedHLMax);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedSealOfRighteousnessTestImpt>();
	}
};

class ImprovedLayOnHandsTest : public TestCaseScript
{
public:
	ImprovedLayOnHandsTest() : TestCaseScript("talents paladin improved_lay_on_hands") { }

	class ImprovedLayOnHandsTestImpt : public TestCase
	{
	public:
		ImprovedLayOnHandsTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const startInt = player->GetStat(STAT_INTELLECT);
			uint32 const expectedInt = startInt * 1.1f;

			RemoveAllItems(player);
			uint32 const startingArmor = player->GetArmor();
			EquipItem(player, 34135); // Sword Breaker's Bulwark - 6459 armor

			uint32 const shieldArmor = player->GetArmor() - startingArmor;
			TEST_ASSERT(shieldArmor == 6459);

			// Assert cooldown, armor through items, mana restored
			LearnTalent(player, Talents::Paladin::IMPROVED_LAY_ON_HANDS_RNK_2);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
			uint32 cooldown = player->GetSpellCooldownDelay(ClassSpells::Paladin::LAY_ON_HANDS_RNK_4);
			TEST_ASSERT(cooldown == 40 * MINUTE);
			uint32 const newShieldArmor = player->GetArmor() - startingArmor;
			uint32 const expectedShieldArmor = shieldArmor * 1.3f;
			TEST_ASSERT(Between<uint32>(newShieldArmor, expectedShieldArmor - 1, expectedShieldArmor + 1));
			TEST_ASSERT(player->GetPower(POWER_MANA) == 900);

			// Assert armor not from items is not taken into account
			player->AddAura(33079, player); // Scroll of Protection V - 300 armor
			player->RemoveAurasDueToSpell(20236); // Remove Lay on Hands proc aura
			player->RemoveAllSpellCooldown();
			res = player->CastSpell(player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(Between<uint32>(newShieldArmor, expectedShieldArmor - 1, expectedShieldArmor + 1));

			TEST_DIRECT_HEAL(player, player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4, player->GetHealth(), player->GetHealth());
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedLayOnHandsTestImpt>();
	}
};

class ImprovedBlessingOfWisdomTest : public TestCaseScript
{
public:
	ImprovedBlessingOfWisdomTest() : TestCaseScript("talents paladin improved_blessing_of_wisdom") { }

	class ImprovedBlessingOfWisdomTestImpt : public TestCase
	{
	public:
		ImprovedBlessingOfWisdomTestImpt() : TestCase(true) { }

		// Wait for next tick and return mana at new tick
		uint32 WaitNextManaTick(Player* p)
		{
			uint32 waitCount = 0;
			uint32 const waitCountMax = 50;
			uint32 currentValue = p->GetPower(POWER_MANA);
			while (currentValue == p->GetPower(POWER_MANA) && waitCount < waitCountMax)
			{
				Wait(1);
				waitCount++;
			}
			TEST_ASSERT(waitCount != waitCountMax);
			return p->GetPower(POWER_MANA);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);

			const float spiritRegen = sqrt(player->GetStat(STAT_SPIRIT)) * player->OCTRegenMPPerSpirit();
			const float blessingRegen = ClassSpellsDamage::Paladin::BLESSING_OF_WISDOM_RNK_7_MIN / 5.0f;
			TC_LOG_TRACE("test.unit_test", "regen: %f, wisdomRegen: %f", spiritRegen, blessingRegen);

			const uint32 startMana = 150; // Blessing of Wisdom mana cost

			player->DisableRegeneration(false);
			player->SetMaxPower(POWER_MANA, 10000);
			player->SetPower(POWER_MANA, startMana);
			TEST_ASSERT(player->GetPower(POWER_MANA) == startMana);

			uint32 res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_WISDOM_RNK_7);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 0);

			uint32 lastMana = 0;
			uint32 newMana = WaitNextManaTick(player);
			//between 0 - 2s have passed, we're still in fsr
			uint32 expectedMana = floor(2 * blessingRegen);
			lastMana = newMana;
			TC_LOG_TRACE("test.unit_test", "t1: current: %u, expected: %u", player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(newMana == expectedMana);

			//between 2 - 4s have passed, we're still in fsr
			newMana = WaitNextManaTick(player);
			expectedMana = lastMana + floor(2 * blessingRegen);
			lastMana = newMana;
			TC_LOG_TRACE("test.unit_test", "t2: current: %u, expected: %u", player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(newMana == expectedMana);

			//wait some ticks to make sure we're out of fsr
			WaitNextManaTick(player);
			WaitNextManaTick(player);
			lastMana = WaitNextManaTick(player);
			newMana = WaitNextManaTick(player);
			expectedMana = lastMana + floor(2 * blessingRegen + 2 * spiritRegen);       //spirit should now increase regen                                         
			lastMana = newMana;
			TC_LOG_TRACE("test.unit_test", "t3: current: %u, expected: %u", player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(newMana == expectedMana);

			//one last time to be sure
			newMana = WaitNextManaTick(player);
			expectedMana = lastMana + floor(2 * blessingRegen + 2 * spiritRegen);
			lastMana = newMana;
			TC_LOG_TRACE("test.unit_test", "t4: current: %u, expected: %u", player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(newMana == expectedMana);

			//now with talent!
			LearnTalent(player, Talents::Paladin::IMPROVED_BLESSING_OF_WISDOM_RNK_2);
			const float improvedBlessingRegen = blessingRegen * 1.2f;
			//re cast spell
			res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_WISDOM_RNK_7, true); //triggered has no mana cost
			TEST_ASSERT(res == SPELL_CAST_OK);

			newMana = WaitNextManaTick(player);
			expectedMana = lastMana + floor(2 * improvedBlessingRegen + 2 * spiritRegen);
			lastMana = newMana;
			TC_LOG_TRACE("test.unit_test", "t5: current: %u, expected: %u", player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(Between<uint32>(newMana, expectedMana - 1, expectedMana + 1));

			newMana = WaitNextManaTick(player);
			expectedMana = lastMana + floor(2 * improvedBlessingRegen + 2 * spiritRegen);
			lastMana = newMana;
			TC_LOG_TRACE("test.unit_test", "t6: current: %u, expected: %u", player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(Between<uint32>(newMana, expectedMana - 1, expectedMana + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedBlessingOfWisdomTestImpt>();
	}
};

class HolyGuidanceTest : public TestCaseScript
{
public:
	HolyGuidanceTest() : TestCaseScript("talents paladin holy_guidance") { }

	class HolyGuidanceTestImpt : public TestCase
	{
	public:
		HolyGuidanceTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			int32 const startBH = player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL);
			int32 const startSP = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL);

			float const startInt = player->GetStat(STAT_INTELLECT);
			int32 const expectedBH = startBH + startInt * 0.35f;
			int32 const expectedSP = startSP + startInt * 0.35f;

			LearnTalent(player, Talents::Paladin::HOLY_GUIDANCE_RNK_5);
			TC_LOG_DEBUG("test.unit_test", "current bh: %i, expected: %i", player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedBH);
			TEST_ASSERT(Between<int32>(player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedBH - 1, expectedBH + 1));
			TC_LOG_DEBUG("test.unit_test", "current sp: %i, expected: %i", player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedSP);
			TEST_ASSERT(Between<int32>(player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedSP - 1, expectedSP + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<HolyGuidanceTestImpt>();
	}
};

class ImprovedDevotionAuraTest : public TestCaseScript
{
public:
	ImprovedDevotionAuraTest() : TestCaseScript("talents paladin improved_devotion_aura") { }

	class ImprovedDevotionAuraTestImpt : public TestCase
	{
	public:
		ImprovedDevotionAuraTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			float const expectedArmor = player->GetArmor() + ClassSpellsDamage::Paladin::DEVOTION_AURA_RNK_8 * 1.4f;

			LearnTalent(player, Talents::Paladin::IMPROVED_DEVOTION_AURA_RNK_5);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::DEVOTION_AURA_RNK_8);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(Between<float>(player->GetArmor(), expectedArmor - 1, expectedArmor + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedDevotionAuraTestImpt>();
	}
};

class GuardiansFavorTest : public TestCaseScript
{
public:
	GuardiansFavorTest() : TestCaseScript("talents paladin guardians_favor") { }

	class GuardiansFavorTestImpt : public TestCase
	{
	public:
		GuardiansFavorTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);

			// Blessing of protection
			LearnTalent(player, Talents::Paladin::GUARDIANS_FAVOR_RNK_2);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_PROTECTION_RNK_3);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(player->GetSpellCooldownDelay(ClassSpells::Paladin::BLESSING_OF_PROTECTION_RNK_3) == 3 * MINUTE);
			Wait(1500);

			// Blessing of freedom
			res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_FREEDOM_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Aura* aura = player->GetAura(ClassSpells::Paladin::BLESSING_OF_FREEDOM_RNK_1, EFFECT_0);
			TEST_ASSERT(aura->GetAuraDuration() == 14 * SECOND * IN_MILLISECONDS);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<GuardiansFavorTestImpt>();
	}
};

class ToughnessTest : public TestCaseScript
{
public:
	ToughnessTest() : TestCaseScript("talents paladin toughness") { }

	class ToughnessTestImpt : public TestCase
	{
	public:
		ToughnessTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			RemoveAllItems(player);
			uint32 const startingArmor = player->GetArmor();
			EquipItem(player, 34135); // Sword Breaker's Bulwark - 6459 armor

			uint32 const shieldArmor = player->GetArmor() - startingArmor;
			TEST_ASSERT(shieldArmor == 6459);

			// Assert armor through items
			LearnTalent(player, Talents::Paladin::TOUGHNESS_RNK_5);
			uint32 const newShieldArmor = player->GetArmor() - startingArmor;
			uint32 const expectedShieldArmor = shieldArmor * 1.1f;
			TEST_ASSERT(Between<uint32>(newShieldArmor, expectedShieldArmor - 1, expectedShieldArmor + 1));

			// Assert armor not from items is not taken into account
			player->AddAura(33079, player); // Scroll of Protection V - 300 armor
			TEST_ASSERT(Between<uint32>(newShieldArmor, expectedShieldArmor - 1, expectedShieldArmor + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ToughnessTestImpt>();
	}
};

class AnticipationTest : public TestCaseScript
{
public:
	AnticipationTest() : TestCaseScript("talents paladin anticipation") { }

	class AnticipationTestImpt : public TestCase
	{
	public:
		AnticipationTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const expectedDef = player->GetDefenseSkillValue() + 20;

			LearnTalent(player, Talents::Paladin::ANTICIPATION_RNK_5);
			TEST_ASSERT(player->GetDefenseSkillValue() == expectedDef);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<AnticipationTestImpt>();
	}
};

class ImprovedHammerOfJusticeTest : public TestCaseScript
{
public:
	ImprovedHammerOfJusticeTest() : TestCaseScript("talents paladin improved_hammer_of_justice") { }

	class ImprovedHammerOfJusticeTestImpt : public TestCase
	{
	public:
		ImprovedHammerOfJusticeTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
			Creature* dummyTarget = SpawnCreature();

			LearnTalent(player, Talents::Paladin::IMPROVED_HAMMER_OF_JUSTICE_RNK_3);
			uint32 res = player->CastSpell(dummyTarget, ClassSpells::Paladin::HAMMER_OF_JUSTICE_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(player->GetSpellCooldownDelay(ClassSpells::Paladin::HAMMER_OF_JUSTICE_RNK_4) == 45 * SECOND);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedHammerOfJusticeTestImpt>();
	}
};

class SacredDutyTest : public TestCaseScript
{
public:
	SacredDutyTest() : TestCaseScript("talents paladin sacred_duty") { }

	class SacredDutyTestImpt : public TestCase
	{
	public:
		SacredDutyTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			// +6% stamina
			uint32 const expectedSta = player->GetStat(STAT_STAMINA) * 1.06f;
			LearnTalent(player, Talents::Paladin::SACRED_DUTY_RNK_2);
			TEST_ASSERT(Between<float>(player->GetStat(STAT_STAMINA), expectedSta - 1, expectedSta + 1));

			// -1min cooldown on Divine Shield
			uint32 startAttackTime = player->GetAttackTime(BASE_ATTACK);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(player->GetSpellCooldownDelay(ClassSpells::Paladin::DIVINE_SHIELD_RNK_2) == 4 * MINUTE);
			TEST_ASSERT(player->GetAttackTime(BASE_ATTACK) == startAttackTime);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SacredDutyTestImpt>();
	}
};

class ImprovedHolyShieldTest : public TestCaseScript
{
public:
	ImprovedHolyShieldTest() : TestCaseScript("talents paladin improved_holy_shield") { }

	class ImprovedHolyShieldTestImpt : public TestCase
	{
	public:
		ImprovedHolyShieldTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			EquipItem(player, 34164); // 1H Sword
			EquipItem(player, 34185); // Shield
			
			// Holy shield stacks
			LearnTalent(player, Talents::Paladin::HOLY_SHIELD_RNK_1);
			LearnTalent(player, Talents::Paladin::IMPROVED_HOLY_SHIELD_RNK_2);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Aura* aura = player->GetAura(Talents::Paladin::HOLY_SHIELD_RNK_1, EFFECT_0);
			TEST_ASSERT(aura->GetStackAmount() == 8);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedHolyShieldTestImpt>();
	}
};

class CombatExpertiseTest : public TestCaseScript
{
public:
	CombatExpertiseTest() : TestCaseScript("talents paladin combat_expertise") { }

	class CombatExpertiseTestImpt : public TestCase
	{
	public:
		CombatExpertiseTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const expectedSta = player->GetStat(STAT_STAMINA) * 1.1f;
			uint32 const expectedExp = 5;

			LearnTalent(player, Talents::Paladin::COMBAT_EXPERTISE_RNK_5);

			// +10% stamina
			TEST_ASSERT(Between<float>(player->GetStat(STAT_STAMINA), expectedSta - 1, expectedSta + 1));

			// +5 expertise
			//TEST_ASSERT( == expectedExp);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<CombatExpertiseTestImpt>();
	}
};

class ImprovedBlessingOfMightTest : public TestCaseScript
{
public:
	ImprovedBlessingOfMightTest() : TestCaseScript("talents paladin improved_blessing_of_might") { }

	class ImprovedBlessingOfMightTestImpt : public TestCase
	{
	public:
		ImprovedBlessingOfMightTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			float const expectedAP = player->GetTotalAttackPowerValue(BASE_ATTACK) + ClassSpellsDamage::Paladin::BLESSING_OF_MIGHT_RNK_8 * 1.2f;

			LearnTalent(player, Talents::Paladin::IMPROVED_BLESSING_OF_MIGHT_RNK_5);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_MIGHT_RNK_8);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(Between<float>(player->GetTotalAttackPowerValue(BASE_ATTACK), expectedAP - 1, expectedAP + 1));
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedBlessingOfMightTestImpt>();
	}
};

class BenedictionTest : public TestCaseScript
{
public:
	BenedictionTest() : TestCaseScript("talents paladin benediction") { }

	class BenedictionTestImpt : public TestCase
	{
	public:
		BenedictionTestImpt() : TestCase(true) { }

		void TestSealMana(TestPlayer* player, TestPlayer* enemy, uint32 sealSpellId, uint32 expectedSealMana)
		{
			player->SetPower(POWER_MANA, expectedSealMana);
			uint32 res = player->CastSpell(enemy, sealSpellId);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(player->GetPower(POWER_MANA) == 0);
			Wait(2000);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 5.0f);
			TestPlayer* enemy = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN, 70, spawnPosition);

			player->DisableRegeneration(true);

			uint32 const expectedSealOfLightMana			= floor(280 * 0.85f);
			uint32 const expectedSealOfRighteousnessMana	= floor(260 * 0.85f);
			uint32 const expectedSealOfVengeanceMana		= floor(250 * 0.85f);
			uint32 const expectedSealOfWisdomMana			= floor(270 * 0.85f);
			uint32 const expectedSealOfJusticeMana			= floor(295 * 0.85f);
			uint32 const expectedSealOfBloodMana			= floor(210 * 0.85f);
			uint32 const expectedSealOfCommandMana			= floor(280 * 0.85f);
			uint32 const expectedSealOfCrusaderMana			= floor(210 * 0.85f);
			uint32 const expectedJudgementMana				= floor(147 * 0.85f);

			LearnTalent(player, Talents::Paladin::BENEDICTION_RNK_5);

			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_LIGHT_RNK_5, expectedSealOfLightMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_RIGHTEOUSNESS_RNK_9, expectedSealOfRighteousnessMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_VENGEANCE_RANK_1, expectedSealOfVengeanceMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_WISDOM_RNK_4, expectedSealOfWisdomMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_JUSTICE_RNK_2, expectedSealOfJusticeMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_BLOOD_RNK_1, expectedSealOfBloodMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_COMMAND_RNK_6, expectedSealOfCommandMana);
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_THE_CRUSADER_RNK_7, expectedSealOfCrusaderMana);
			TestSealMana(player, enemy, ClassSpells::Paladin::JUDGEMENT_RNK_1, expectedJudgementMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<BenedictionTestImpt>();
	}
};

class ImprovedJudgementTest : public TestCaseScript
{
public:
	ImprovedJudgementTest() : TestCaseScript("talents paladin improved_judgement") { }

	class ImprovedJudgementTestImpt : public TestCase
	{
	public:
		ImprovedJudgementTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 5.0f);
			TestPlayer* enemy = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN, 70, spawnPosition);

			LearnTalent(player, Talents::Paladin::IMPROVED_JUDGEMENT_RNK_2);

			uint32 res = player->CastSpell(player, ClassSpells::Paladin::SEAL_OF_LIGHT_RNK_5);
			TEST_ASSERT(res == SPELL_CAST_OK);

			res = player->CastSpell(enemy, ClassSpells::Paladin::JUDGEMENT_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(player->GetSpellCooldownDelay(ClassSpells::Paladin::JUDGEMENT_RNK_1) == 8 * SECOND);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<ImprovedJudgementTestImpt>();
	}
};

class DeflectionTest : public TestCaseScript
{
public:
	DeflectionTest() : TestCaseScript("talents paladin deflection") { }

	class DeflectionTestImpt : public TestCase
	{
	public:
		DeflectionTestImpt() : TestCase(true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			float const expectedParry = player->GetUnitParryChance() + 5;

			LearnTalent(player, Talents::Paladin::DEFLECTION_RNK_5);
			TEST_ASSERT(player->GetUnitParryChance() == expectedParry);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<DeflectionTestImpt>();
	}
};

class CrusadeTest : public TestCaseScript
{
public:
	CrusadeTest() : TestCaseScript("talents paladin crusade") { }

	class CrusadeTestImpt : public TestCase
	{
	public:
		CrusadeTestImpt() : TestCase(true) { }

		void TestCreatureType(TestPlayer* player, uint32 entry, bool demonOrUndead = false)
		{
			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition, entry);

			// Consecration
			// Exorcism (undead + demon)
			// Holy wrath (undead + demon)
			// Holy shock
			// Hammer of wrath
			// Every judgement
			// Crusader Strike
			// Holy Shield
			// Avenger's Shield
			// Melee
		}

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			// Creatures are training dummys with 20% HP
			TestCreatureType(player, 18); // Humanoid
			TestCreatureType(player, 21, true); // Demon
			TestCreatureType(player, 24, true); // Undead
			TestCreatureType(player, 22); // Elemental
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<CrusadeTestImpt>();
	}
};

void AddSC_test_talents_paladin()
{
	// Total: 18/62
	// Holy: 6/20
	new DivineStrengthTest();
	new DivineIntellectTest();
	new HealingLightTest();
	new ImprovedLayOnHandsTest();
    new ImprovedBlessingOfWisdomTest();
	new HolyGuidanceTest();
	// Protection: 8/22
	new ImprovedDevotionAuraTest();
	new GuardiansFavorTest();
	new ToughnessTest();
	new AnticipationTest();
	new ImprovedHammerOfJusticeTest();
	new SacredDutyTest();
	new ImprovedHolyShieldTest();
	new CombatExpertiseTest();
	// Retribution: 4/22
	new ImprovedBlessingOfMightTest();
	new BenedictionTest();
	new ImprovedJudgementTest();
	new DeflectionTest();
	new CrusadeTest();
}