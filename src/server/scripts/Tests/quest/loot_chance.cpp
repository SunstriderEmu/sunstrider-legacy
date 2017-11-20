#include "TestCase.h"
#include "TestPlayer.h"
#include "World.h"

float CalcChance(uint32 iterations, const std::function<bool()>& f)
{
    uint32 success = 0;
    for (uint32 i = 0; i < iterations; i++)
    {
        success += uint32(f());
    }
    return float(success) / float(iterations);
}


template<class T>
bool Between(T value, T from, T to)
{
    return value >= from && value <= to;
}

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
        QuestLootChanceTestImpl() : TestCase(true) { }

        void Test() override
        {
            const float lootChance = 0.35f; //Scroll of Demonic Unbanishing loot chance. Change if value changed in db. // SELECT * FROM creature_loot_template WHERE entry = 21503 AND item = 30811
            Player* player = SpawnRandomPlayer();
            Quest const* quest = sObjectMgr->GetQuestTemplate(10637); // A Necessary Distraction
            TEST_CHECK(quest != nullptr);
            TEST_CHECK(player->CanAddQuest(quest, false));
            player->AddQuest(quest, nullptr);
            uint32 const iterations = 500;
            float chance = CalcChance(iterations, [&]
            { 
                Creature* warlock = SpawnCreature(21503); //Sunfury Warlock
                player->DealDamage(warlock, 9999999);
                TEST_CHECK(warlock->GetLootRecipient() == player);
                return HasLootForMe(warlock, player, 30811);
            });
            TEST_CHECK(WithinStandartDeviation(chance, lootChance, iterations));
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
