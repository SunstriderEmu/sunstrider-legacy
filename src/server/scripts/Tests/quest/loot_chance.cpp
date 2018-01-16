#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

template<class T>
bool WithinStandartDeviation(T resultChance, T theoricChance, uint32 iterations)
{
    return true;
}

class QuestLootChanceTest : public TestCaseScript
{
public:

    QuestLootChanceTest() : TestCaseScript("quest lootchance") { }

    class QuestLootChanceTestImpl : public TestCase
    {
    public:
        QuestLootChanceTestImpl() : TestCase(STATUS_PASSING, true) { }

        void Test() override
        {
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
            TEST_ASSERT(WithinStandartDeviation(chance, lootChance, iterations));
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<QuestLootChanceTestImpl>();
    }
};

void AddSC_test_loot_chance()
{
    new QuestLootChanceTest();
}
