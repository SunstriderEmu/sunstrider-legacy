#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

class QuestLootChanceTest : public TestCaseScript
{
public:

    QuestLootChanceTest() : TestCaseScript("quest lootchance") { }

    class QuestLootChanceTestImpl : public TestCase
    {
    public:
        QuestLootChanceTestImpl() : TestCase(true) { }

        void Test() override
        {
            const float lootChance = 0.35f; //Scroll of Demonic Unbanishing loot chance. Change if value changed in db. // SELECT * FROM creature_loot_template WHERE entry = 21503 AND item = 30811
            Player* player = SpawnRandomPlayer();
            Quest const* quest = sObjectMgr->GetQuestTemplate(10637); // A Necessary Distraction
            TEST_ASSERT(quest != nullptr);
            /*
            TEST_ASSERT(player->CanAddQuest(quest));
            player->AddQuest(quest, nullptr);
            Creature* warlock = SpawnCreature(21503); //Sunfury Warlock
            player->DealDamage(warlock, 9999999);
            TEST_ASSERT(warlock->GetLootRecipient() == player);
            */
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<QuestLootChanceTestImpl>();
    }
};

void AddSC_loot_chance()
{
    new QuestLootChanceTest();
}
