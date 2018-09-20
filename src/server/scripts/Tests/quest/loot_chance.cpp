#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

class QuestLootChanceTest : public TestCase
{
    void Test() override
    {
        SECTION("loot chance", STATUS_WIP, [&]() {
            const float expectedLootChance = 35.f; //Scroll of Demonic Unbanishing loot chance. Change if value changed in db. // SELECT * FROM creature_loot_template WHERE entry = 21503 AND item = 30811
            Player* player = SpawnRandomPlayer();
            Quest const* quest = sObjectMgr->GetQuestTemplate(10637); // A Necessary Distraction
            TEST_ASSERT(quest != nullptr);
            TEST_ASSERT(player->CanAddQuest(quest, false));
            player->AddQuest(quest, nullptr);

            uint32 successCount = 0;
            uint32 sampleSize = 0;
            float resultingAbsoluteTolerance = _GetPercentTestTolerance(expectedLootChance);
            while (_ShouldIteratePercent(expectedLootChance, successCount, sampleSize, resultingAbsoluteTolerance))
            {
                Creature* warlock = SpawnCreature(21503); //Sunfury Warlock
                Unit::DealDamage(player, warlock, 9999999);
                TEST_ASSERT(warlock->GetLootRecipient() == player);
                successCount += uint32(HasLootForMe(warlock, player, 30811));
            }

            TEST_ASSERT(sampleSize != 0);
            float actualSuccessPercent = 100 * (successCount / float(sampleSize));
            ASSERT_INFO("Expected result: %f, result: %f", expectedLootChance, actualSuccessPercent);
            TEST_ASSERT(Between<float>(expectedLootChance, expectedLootChance - resultingAbsoluteTolerance, expectedLootChance + resultingAbsoluteTolerance));

            /*
            uint32 const iterations = 500;
            float chance = CalcChance(iterations, [&]
            {
                Creature* warlock = SpawnCreature(21503); //Sunfury Warlock
                Unit::DealDamage(player, warlock, 9999999);
                TEST_ASSERT(warlock->GetLootRecipient() == player);
                return HasLootForMe(warlock, player, 30811);
            });
            //TODO!
            //TEST_ASSERT(WithinStandartDeviation(chance, lootChance, iterations));*/
        });
    }
};

void AddSC_test_loot_chance()
{
    RegisterTestCase("quest lootchance", QuestLootChanceTest);
}
