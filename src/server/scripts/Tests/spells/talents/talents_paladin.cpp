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

void AddSC_test_talents_paladin()
{
	new DivineStrengthTest();
	new DivineIntellectTest();
	new HealingLightTest();
	new ImprovedLayOnHandsTest();
    new ImprovedBlessingOfWisdomTest();
	new HolyGuidanceTest();
}
