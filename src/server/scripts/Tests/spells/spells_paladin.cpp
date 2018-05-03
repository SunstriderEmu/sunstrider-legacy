#include "../ClassSpellsDamage.h"
#include "../ClassSpellsCoeff.h"

class FlashOfLightTest : public TestCaseScript
{
public:
    
    FlashOfLightTest() : TestCaseScript("spells paladin flash_of_light") { }
    
    class FlashOfLightTestImpt : public TestCase
    {
    public:
        FlashOfLightTestImpt() : TestCase(STATUS_WIP) { }
        
        void Test() override
        {
            TestPlayer* paladin = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN);
            
            EQUIP_ITEM(paladin, 34335); // Hammer of Sanctification - 550 BH
            paladin->DisableRegeneration(true);
            
            int32 maceBH = 550;
            TEST_ASSERT(paladin->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);
            
            // Mana cost
            uint32 const expectedFlashOfLightMana = 180;
            TEST_POWER_COST(paladin, paladin, ClassSpells::Paladin::FLASH_OF_LIGHT_RNK_7, POWER_MANA, expectedFlashOfLightMana);
            
            // Spell coefficient
            float const flashOfLightCastTIme = 1.5f;
            float const flashOfLightSpellCoeff = flashOfLightCastTIme / 3.5f;
            uint32 const flashOfLightBHBonus = maceBH * flashOfLightSpellCoeff;
            uint32 const flashOfLightMinHeal = ClassSpellsDamage::Paladin::FLASH_OF_LIGHT_RNK_7_MIN + flashOfLightBHBonus;
            uint32 const flashOfLightMaxHeal = ClassSpellsDamage::Paladin::FLASH_OF_LIGHT_RNK_7_MAX + flashOfLightBHBonus;
            TEST_DIRECT_HEAL(paladin, paladin, ClassSpells::Paladin::FLASH_OF_LIGHT_RNK_7, flashOfLightMinHeal, flashOfLightMaxHeal, false);
            
            uint32 const flashOfLightMinCritHeal = flashOfLightMinHeal * 1.5f;
            uint32 const flashOfLightMaxCritHeal = flashOfLightMaxHeal * 1.5f;
            TEST_DIRECT_HEAL(paladin, paladin, ClassSpells::Paladin::FLASH_OF_LIGHT_RNK_7, flashOfLightMinCritHeal, flashOfLightMaxCritHeal, true);
        }
    };
    
    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<FlashOfLightTestImpt>();
    }
};


class HolyLightTest : public TestCaseScript
{
public:
    
    HolyLightTest() : TestCaseScript("spells paladin flash_of_light") { }
    
    class HolyLightTestImpt : public TestCase
    {
    public:
        HolyLightTestImpt() : TestCase(STATUS_WIP) { }
        
        void Test() override
        {
            TestPlayer* paladin = SpawnPlayer(CLASS_PALADIN, RACE_HUMAN);
            
            EQUIP_ITEM(paladin, 34335); // Hammer of Sanctification - 550 BH
            paladin->DisableRegeneration(true);
            
            int32 maceBH = 550;
            TEST_ASSERT(paladin->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL) == maceBH);
            
            // Mana cost
            uint32 const expectedHolyLightMana = 840;
            TEST_POWER_COST(paladin, paladin, ClassSpells::Paladin::HOLY_LIGHT_RNK_11, POWER_MANA, expectedHolyLightMana);
            
            // Spell coefficient
            float const holyLightCastTIme = 2.5f;
            float const holyLightSpellCoeff = holyLightCastTIme / 3.5f;
            uint32 const holyLightBHBonus = maceBH * holyLightSpellCoeff;
            uint32 const holyLightMinHeal = ClassSpellsDamage::Paladin::HOLY_LIGHT_RNK_11_MIN + holyLightBHBonus;
            uint32 const holyLightMaxHeal = ClassSpellsDamage::Paladin::HOLY_LIGHT_RNK_11_MAX + holyLightBHBonus;
            TEST_DIRECT_HEAL(paladin, paladin, ClassSpells::Paladin::HOLY_LIGHT_RNK_11, holyLightMinHeal, holyLightMaxHeal, false);
            
            uint32 const holyLightMinCritHeal = holyLightMinHeal * 1.5f;
            uint32 const holyLightMaxCritHeal = holyLightMaxHeal * 1.5f;
            TEST_DIRECT_HEAL(paladin, paladin, ClassSpells::Paladin::HOLY_LIGHT_RNK_11, holyLightMinCritHeal, holyLightMaxCritHeal, true);
        }
    };
    
    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<HolyLightTestImpt>();
    }
};

void AddSC_test_spells_paladin()
{
    new FlashOfLightTest();
    new HolyLightTest();
}
