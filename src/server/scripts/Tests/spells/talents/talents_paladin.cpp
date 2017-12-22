#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"
#include "PlayerbotAI.h"

class SalvationTest : public TestCaseScript
{
public:
	SalvationTest() : TestCaseScript("talents paladin salvation") { }

	class SalvationTestImpt : public TestCase
	{
	public:
		SalvationTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			// TODO
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SalvationTestImpt>();
	}
};

class DivineStrengthTest : public TestCaseScript
{
public:
	DivineStrengthTest() : TestCaseScript("talents paladin divine_strength") { }

	class DivineStrengthTestImpt : public TestCase
	{
	public:
		DivineStrengthTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		DivineIntellectTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		ImprovedSealOfRighteousnessTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const bh = player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL);
			uint32 const expectedFoLMin = ClassSpellsDamage::Paladin::FLASH_OF_LIGHT_RNK_7_MIN * 1.12f + bh * ClassSpellsCoeff::Paladin::FLASH_OF_LIGHT;
			uint32 const expectedFoLMax = ClassSpellsDamage::Paladin::FLASH_OF_LIGHT_RNK_7_MAX * 1.12f + bh * ClassSpellsCoeff::Paladin::FLASH_OF_LIGHT;
			uint32 const expectedHLMin = ClassSpellsDamage::Paladin::HOLY_LIGHT_RNK_11_MIN * 1.12f + bh * ClassSpellsCoeff::Paladin::HOLY_LIGHT;
			uint32 const expectedHLMax = ClassSpellsDamage::Paladin::HOLY_LIGHT_RNK_11_MAX * 1.12f + bh * ClassSpellsCoeff::Paladin::HOLY_LIGHT;

			LearnTalent(player, Talents::Paladin::HEALING_LIGHT_RNK_3);
			TEST_DIRECT_HEAL(player, player, ClassSpells::Paladin::FLASH_OF_LIGHT_RNK_7, expectedFoLMin, expectedFoLMax, false);
			TEST_DIRECT_HEAL(player, player, ClassSpells::Paladin::HOLY_LIGHT_RNK_11, expectedHLMin, expectedHLMax, false);
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
		ImprovedLayOnHandsTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			uint32 const startInt = player->GetStat(STAT_INTELLECT);
			uint32 const expectedInt = startInt * 1.1f;

            RemoveAllEquipedItems(player);
			uint32 const startingArmor = player->GetArmor();
			EQUIP_ITEM(player, 34185); // Sword Breaker's Bulwark - 6459 armor

			uint32 const shieldArmor = player->GetArmor() - startingArmor;
			TEST_ASSERT(shieldArmor == 6459);

			// Assert cooldown, armor through items, mana restored
			LearnTalent(player, Talents::Paladin::IMPROVED_LAY_ON_HANDS_RNK_2);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
            TEST_HAS_COOLDOWN(player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4, 40 * MINUTE);
			uint32 const newShieldArmor = player->GetArmor() - startingArmor;
			uint32 const expectedShieldArmor = shieldArmor * 1.3f;
			TEST_ASSERT(Between<uint32>(newShieldArmor, expectedShieldArmor - 1, expectedShieldArmor + 1));
            ASSERT_INFO("player mana %u", player->GetPower(POWER_MANA));
			TEST_ASSERT(player->GetPower(POWER_MANA) == 900); //fails because mana is removed AFTER getting the 900 mana

			// Assert armor not from items is not taken into account
			player->AddAura(33079, player); // Scroll of Protection V - 300 armor
			player->RemoveAurasDueToSpell(20236); // Remove Lay on Hands proc aura
			player->RemoveAllSpellCooldown();
			res = player->CastSpell(player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_ASSERT(Between<uint32>(newShieldArmor, expectedShieldArmor - 1, expectedShieldArmor + 1));

			TEST_DIRECT_HEAL(player, player, ClassSpells::Paladin::LAY_ON_HANDS_RNK_4, player->GetHealth(), player->GetHealth(), false);
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
		ImprovedBlessingOfWisdomTestImpt() : TestCase(STATUS_PASSING, true) { }

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
			res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_WISDOM_RNK_7, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
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
		HolyGuidanceTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			int32 const startBH = player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL);
			int32 const startSP = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL);

			float const startInt = player->GetStat(STAT_INTELLECT);
			int32 const expectedBH = startBH + startInt * 0.35f;
			int32 const expectedSP = startSP + startInt * 0.35f;

			LearnTalent(player, Talents::Paladin::HOLY_GUIDANCE_RNK_5);
            TC_LOG_TRACE("test.unit_test", "current bh: %i, expected: %i", player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedBH);
			TEST_ASSERT(Between<int32>(player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedBH - 1, expectedBH + 1));
            TC_LOG_TRACE("test.unit_test", "current sp: %i, expected: %i", player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL), expectedSP);
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
		ImprovedDevotionAuraTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		GuardiansFavorTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);

			// Blessing of protection
			LearnTalent(player, Talents::Paladin::GUARDIANS_FAVOR_RNK_2);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_PROTECTION_RNK_3);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_HAS_COOLDOWN(player, ClassSpells::Paladin::BLESSING_OF_PROTECTION_RNK_3, 3 * MINUTE);
			Wait(1500);

			// Blessing of freedom
			res = player->CastSpell(player, ClassSpells::Paladin::BLESSING_OF_FREEDOM_RNK_1);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Aura* aura = player->GetAura(ClassSpells::Paladin::BLESSING_OF_FREEDOM_RNK_1, EFFECT_0);
			TEST_ASSERT(aura->GetDuration() == 14 * SECOND * IN_MILLISECONDS);
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
		ToughnessTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

            RemoveAllEquipedItems(player);
			uint32 const startingArmor = player->GetArmor();
			EQUIP_ITEM(player, 34135); // Sword Breaker's Bulwark - 6459 armor

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
		AnticipationTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		ImprovedHammerOfJusticeTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
			Creature* dummyTarget = SpawnCreature();

			LearnTalent(player, Talents::Paladin::IMPROVED_HAMMER_OF_JUSTICE_RNK_3);
			uint32 res = player->CastSpell(dummyTarget, ClassSpells::Paladin::HAMMER_OF_JUSTICE_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
			TEST_HAS_COOLDOWN(player, ClassSpells::Paladin::HAMMER_OF_JUSTICE_RNK_4, 45 * SECOND);
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
		SacredDutyTestImpt() : TestCase(STATUS_PASSING, true) { }

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
			TEST_HAS_COOLDOWN(player, ClassSpells::Paladin::DIVINE_SHIELD_RNK_2, 4 * MINUTE);
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
		ImprovedHolyShieldTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			EQUIP_ITEM(player, 34164); // 1H Sword
			EQUIP_ITEM(player, 34185); // Shield
			
			// Holy shield stacks
			LearnTalent(player, Talents::Paladin::HOLY_SHIELD_RNK_1);
			LearnTalent(player, Talents::Paladin::IMPROVED_HOLY_SHIELD_RNK_2);
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::HOLY_SHIELD_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Aura* aura = player->GetAura(ClassSpells::Paladin::HOLY_SHIELD_RNK_4, EFFECT_0);
			TEST_ASSERT(aura != nullptr);
			TEST_ASSERT(aura->GetCharges() == 8);
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
		CombatExpertiseTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		ImprovedBlessingOfMightTestImpt() : TestCase(STATUS_PASSING, true) { }

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
		BenedictionTestImpt() : TestCase(STATUS_PASSING, true) { }

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
			TestSealMana(player, player, ClassSpells::Paladin::SEAL_OF_VENGEANCE_RNK_1, expectedSealOfVengeanceMana);
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
		ImprovedJudgementTestImpt() : TestCase(STATUS_PASSING, true) { }

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
			TEST_HAS_COOLDOWN(player, ClassSpells::Paladin::JUDGEMENT_RNK_1, 8 * SECOND);
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
		DeflectionTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnRandomPlayer(CLASS_PALADIN);

			float const expectedParry = player->GetUnitParryChance(BASE_ATTACK, player) + 5.0f;

			LearnTalent(player, Talents::Paladin::DEFLECTION_RNK_5);
			TEST_ASSERT(player->GetUnitParryChance(BASE_ATTACK, player) == expectedParry);
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
		CrusadeTestImpt() : TestCase(STATUS_PASSING, true) { }

		void TestCreatureType(TestPlayer* player, uint32 entry, bool demonOrUndead = false)
		{
			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition, entry);
			creature->DisableRegeneration(true);
			creature->SetHealth(creature->GetHealth() - 1); // issue with Hammer of wrath

			player->SetMaxHealth(10000000); // Used for Judgement of Blood and Holy shield
			player->SetHealth(player->GetMaxHealth());

			// Judgement of Righteousness
			float const expectedJoRMin = ClassSpellsDamage::Paladin::JUDGEMENT_OF_RIGHTEOUSNESS_RNK_9_MIN * 1.03f;
			float const expectedJoRMax = ClassSpellsDamage::Paladin::JUDGEMENT_OF_RIGHTEOUSNESS_RNK_9_MAX * 1.03f;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::JUDGEMENT_OF_RIGHTEOUSNESS_RNK_9, expectedJoRMin, expectedJoRMax, false);


			// Consecration
			uint32 const creatureStartHP = creature->GetHealth();
			uint32 res = player->CastSpell(player, ClassSpells::Paladin::CONSECRATION_RNK_6);
			TEST_ASSERT(res == SPELL_CAST_OK);
			Wait(1500);
			if (creature->GetHealth() == creatureStartHP) { // wait for second tick
				Wait(1500);
				if (creature->GetHealth() == creatureStartHP) // wait for third tick, hopefully it doesnt resist the first 3 ticks ._.
					Wait(1500);
			}
			float const tick = floor(ClassSpellsDamage::Paladin::CONSECRATION_RNK_6_TOTAL * 1.03f / 8.0f);
			float const expectedHP = creatureStartHP - tick;
			uint32 const currentHealth = creature->GetHealth();
			TEST_ASSERT(currentHealth == expectedHP);

			// Exorcism & Holy wrath
			if (demonOrUndead)
			{
				float const expectedExorcismMin = ClassSpellsDamage::Paladin::EXORCISM_RNK_7_MIN * 1.03f;
				float const expectedExorcismMax = ClassSpellsDamage::Paladin::EXORCISM_RNK_7_MAX * 1.03f;
				TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::EXORCISM_RNK_7, expectedExorcismMin, expectedExorcismMax, false);

				float const expectedHolyWrathMin = ClassSpellsDamage::Paladin::HOLY_WRATH_RNK_3_MIN * 1.03f;
				float const expectedHolyWrathMax = ClassSpellsDamage::Paladin::HOLY_WRATH_RNK_3_MAX * 1.03f;
				TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::HOLY_WRATH_RNK_3, expectedHolyWrathMin, expectedHolyWrathMax, false);
			}

			// Hammer of wrath
			float const expectedHoWMin = ClassSpellsDamage::Paladin::HAMMER_OF_WRATH_RNK_4_MIN * 1.03f;
			float const expectedHoWMax = ClassSpellsDamage::Paladin::HAMMER_OF_WRATH_RNK_4_MAX * 1.03f;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::HAMMER_OF_WRATH_RNK_4, expectedHoWMin, expectedHoWMax, false);

			// Holy shock
			float const expectedHolyShockMin = ClassSpellsDamage::Paladin::HOLY_SHOCK_RNK_5_MIN_DAMAGE * 1.03f;
			float const expectedHolyShockMax = ClassSpellsDamage::Paladin::HOLY_SHOCK_RNK_5_MAX_DAMAGE * 1.03f;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::HOLY_SHOCK_RNK_5_DMG, expectedHolyShockMin, expectedHolyShockMax, false);

			// Judgement of Blood
			float const expectedJoBMin = ClassSpellsDamage::Paladin::JUDGEMENT_OF_BLOOD_RNK_1_MIN * 1.03f;
			float const expectedJoBMax = ClassSpellsDamage::Paladin::JUDGEMENT_OF_BLOOD_RNK_1_MAX * 1.03f;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::JUDGEMENT_OF_BLOOD_RNK_1, expectedJoBMin, expectedJoBMax, false);

			// Judgement of Vengeance
			float const expectedJoVMin = ClassSpellsDamage::Paladin::JUDGEMENT_OF_VENGEANCE_PER_STACK * 1.03f;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::JUDGEMENT_OF_VENGEANCE_RNK_1, expectedJoBMin, expectedJoVMin, false);

			// Crusader Strike
			EQUIP_ITEM(player, 34247); // Apolyon, the Soul-Render - 404-607 damage
			LearnTalent(player, Talents::Paladin::CRUSADER_STRIKE_RNK_1);
			float const AP = player->GetTotalAttackPowerValue(BASE_ATTACK);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			float const weaponMinDamage = 404 + (AP / 14 * 3.3f); // 3.3 weapon speed because it's an normalized spell
			float const weaponMaxDamage = 607 + (AP / 14 * 3.3f);
			float const crusadeTalentFactor = 1.03;
			float const expectedCSMin = (weaponMinDamage * 1.1f) * armorFactor * crusadeTalentFactor;
			float const expectedCSMax = (weaponMaxDamage * 1.1f) * armorFactor * crusadeTalentFactor;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::CRUSADER_STRIKE_RNK_1, expectedCSMin, expectedCSMax, false);

			/*
			// Holy shield
			EQUIP_ITEM(player, 34164); // 1H Sword
			EQUIP_ITEM(player, 34185); // Shield

			LearnTalent(player, Talents::Paladin::HOLY_SHIELD_RNK_1);
			res = player->CastSpell(player, ClassSpells::Paladin::HOLY_SHIELD_RNK_4);
			TEST_ASSERT(res == SPELL_CAST_OK);

			TestPlayer* enemy = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN);

			auto AI = player->GetTestingPlayerbotAI();
			TEST_ASSERT(AI != nullptr);

			// Do some melee attacks

			float expectedDamage = ClassSpellsDamage::Paladin::HOLY_SHIELD_RNK_4_TICK * 8.0f;
			float avgDamageDealt = GetDamagePerSpellsTo(player, enemy, ClassSpells::Paladin::HOLY_SHIELD_RNK_4);
			TEST_ASSERT(avgDamageDealt == expectedDamage);
			*/

			// TODO: Seal of Righteouness: melee attack
			// Crusader Strike
			// Avenger's Shield
			// Melee
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);

			LearnTalent(player, Talents::Paladin::CRUSADE_RNK_3);

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

class SanctifiedJudgementTest : public TestCaseScript
{
public:
	SanctifiedJudgementTest() : TestCaseScript("talents paladin sanctified_judgement") { }

	class SanctifiedJudgementTestImpt : public TestCase
	{
	public:
		SanctifiedJudgementTestImpt() : TestCase(STATUS_PASSING, true) { }

		void TestSealMana(TestPlayer* player, Creature* creature, uint32 sealSpellId, uint32 sealManaCost)
		{
			player->Regenerate(POWER_MANA);
			player->RemoveAllSpellCooldown();
			uint32 const startMana = player->GetPower(POWER_MANA);

			int32 const judgementMana = 147;

			// Seal
            TEST_CAST(player, player, sealSpellId);
            TEST_HAS_AURA(player, sealSpellId);
			Wait(1500);
			// Judgement
            TEST_CAST(player, creature, ClassSpells::Paladin::JUDGEMENT_RNK_1);

			uint32 expectedMana = startMana - sealManaCost - judgementMana + sealManaCost * 0.8f;
			ASSERT_INFO("Seal: %u - Current Mana: %u - Expected Mana: %u", sealSpellId, player->GetPower(POWER_MANA), expectedMana);
			TEST_ASSERT(player->GetPower(POWER_MANA) == expectedMana);
			Wait(1500);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);

			player->DisableRegeneration(true);
			LearnTalent(player, Talents::Paladin::SANCTIFIED_JUDGEMENT_RNK_3);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, 3.0f);
			Creature* creature = SpawnCreatureWithPosition(spawnPosition, 18); // Dummy Humanoid 20% HP

			int32 const judgementMana = 147;
			int32 const sealOfLightMana = 280;
			int32 const sealOfRighteousnessMana = 260;
			int32 const sealOfVengeanceMana = 250;
			int32 const sealOfWisdomMana = 270;
			int32 const sealOfJusticeMana = 295;
			int32 const sealOfBloodMana = 210;
			int32 const sealOfCommandMana = 280;
			int32 const sealOfCrusaderMana = 210;

			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_LIGHT_RNK_5, sealOfLightMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_RIGHTEOUSNESS_RNK_9, sealOfRighteousnessMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_VENGEANCE_RNK_1, sealOfVengeanceMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_WISDOM_RNK_4, sealOfWisdomMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_JUSTICE_RNK_2, sealOfJusticeMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_BLOOD_RNK_1, sealOfBloodMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_COMMAND_RNK_6, sealOfCommandMana);
			TestSealMana(player, creature, ClassSpells::Paladin::SEAL_OF_THE_CRUSADER_RNK_7, sealOfCrusaderMana);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SanctifiedJudgementTestImpt>();
	}
};

class SanctifiedSealsTest : public TestCaseScript
{
public:
	SanctifiedSealsTest() : TestCaseScript("talents paladin sanctified_seals") { }

	class SanctifiedSealsTestImpt : public TestCase
	{
	public:
		SanctifiedSealsTestImpt() : TestCase(STATUS_PASSING, true) { }

		void TestDispelSeal(TestPlayer* player, TestPlayer* shaman, uint32 sealSpellId)
		{
			// Seal
            TEST_CAST(player, player, sealSpellId);
            TEST_HAS_AURA(player, sealSpellId);

			// Dispel
            TEST_CAST(shaman, player, sealSpellId);

			ASSERT_INFO("Seal %u was dispelled ", sealSpellId);
			TEST_ASSERT(player->HasAura(sealSpellId));
			Wait(1500);
		}

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
			LearnTalent(player, Talents::Paladin::SANCTIFIED_SEALS_RNK_3);

			Position spawnPosition(_location);
			spawnPosition.MoveInFront(_location, -5.0f);
			TestPlayer* shaman = SpawnPlayer(CLASS_SHAMAN, RACE_DRAENEI, 70, spawnPosition); // Spawn behind the paladin

			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_LIGHT_RNK_5);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_RIGHTEOUSNESS_RNK_9);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_VENGEANCE_RNK_1);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_WISDOM_RNK_4);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_JUSTICE_RNK_2);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_BLOOD_RNK_1);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_COMMAND_RNK_6);
			TestDispelSeal(player, shaman, ClassSpells::Paladin::SEAL_OF_THE_CRUSADER_RNK_7);
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<SanctifiedSealsTestImpt>();
	}
};

class CrusaderStrikeTest : public TestCaseScript
{
public:
	CrusaderStrikeTest() : TestCaseScript("talents paladin crusader_strike") { }

	class CrusaderStrikeTestImpt : public TestCase
	{
	public:
		CrusaderStrikeTestImpt() : TestCase(STATUS_PASSING, true) { }

		void Test() override
		{
			TestPlayer* player = SpawnPlayer(CLASS_PALADIN, RACE_BLOODELF);
			Creature* creature = SpawnCreature();

			LearnTalent(player, Talents::Paladin::CRUSADER_STRIKE_RNK_1);

			// Damage
			EQUIP_ITEM(player, 34247); // Apolyon, the Soul-Render - 404-607 damage
			float const AP = player->GetTotalAttackPowerValue(BASE_ATTACK);
			TEST_ASSERT(AP == 562);
			float const armorFactor = 1 - (creature->GetArmor() / (creature->GetArmor() + 10557.5));
			uint32 const weaponMinDamage = 404 + (AP / 14 * 3.3f); // 3.3 weapon speed because it's an normalized spell
			uint32 const weaponMaxDamage = 607 + (AP / 14 * 3.3f);
			uint32 const expectedCSMin = weaponMinDamage * 1.1f * armorFactor;
			uint32 const expectedCSMax = weaponMaxDamage * 1.1f * armorFactor;
			TEST_DIRECT_SPELL_DAMAGE(player, creature, ClassSpells::Paladin::CRUSADER_STRIKE_RNK_1, expectedCSMin, expectedCSMax, false);

			// TODO: refresh judgements
		}
	};

	std::shared_ptr<TestCase> GetTest() const override
	{
		return std::make_shared<CrusaderStrikeTestImpt>();
	}
};

void AddSC_test_talents_paladin()
{
	// Total: 22/63
	new SalvationTest(); // TODO: innate ability
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
	// Retribution: 8/22
	new ImprovedBlessingOfMightTest();
	new BenedictionTest();
	new ImprovedJudgementTest();
	new DeflectionTest();
	new CrusadeTest();
	new SanctifiedJudgementTest();
	new SanctifiedSealsTest();
	new CrusaderStrikeTest();
}