#include "../../ClassSpellsDamage.h"
#include "../../ClassSpellsCoeff.h"

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
    new ImprovedBlessingOfWisdomTest();
}
