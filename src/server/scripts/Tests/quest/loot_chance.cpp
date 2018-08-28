#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

class QuestLootChanceTest : public TestCase
{
    void Test() override
    {
        SECTION("loot chance", STATUS_WIP, [&]() {
            const float lootChance = 0.35f; //Scroll of Demonic Unbanishing loot chance. Change if value changed in db. // SELECT * FROM creature_loot_template WHERE entry = 21503 AND item = 30811
            Player* player = SpawnRandomPlayer();
            Quest const* quest = sObjectMgr->GetQuestTemplate(10637); // A Necessary Distraction
            TEST_ASSERT(quest != nullptr);
            TEST_ASSERT(player->CanAddQuest(quest, false));
            player->AddQuest(quest, nullptr);
            uint32 const iterations = 500;
            float chance = CalcChance(iterations, [&]
            {
                Creature* warlock = SpawnCreature(21503); //Sunfury Warlock
                Unit::DealDamage(player, warlock, 9999999);
                TEST_ASSERT(warlock->GetLootRecipient() == player);
                return HasLootForMe(warlock, player, 30811);
            });
            //TODO!
            //TEST_ASSERT(WithinStandartDeviation(chance, lootChance, iterations));
        });
    }
};

void AddSC_test_loot_chance()
{
    RegisterTestCase("quest lootchance", QuestLootChanceTest);
}
