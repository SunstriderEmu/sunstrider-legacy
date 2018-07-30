#include "TestCase.h"
#include "TestPlayer.h"
#include "ObjectMgr.h"

class CreatureLinkedRespawnTest : public TestCaseScript
{
public:
    CreatureLinkedRespawnTest() : TestCaseScript("creature linkedrespawn") { }

    class CreatureLinkedRespawnTestImpl : public TestCase
    {
    public:
        CreatureLinkedRespawnTestImpl() : TestCase(WorldLocation(560)),
            masterCreature(nullptr), slaveCreature(nullptr) 
        { }

        Creature* masterCreature;
        Creature* slaveCreature;

        void Test() override
        {
            masterCreature = nullptr;
            slaveCreature = nullptr;

            //setting up
            auto spawnCreature = [&]() 
            {
                Creature* creature = new Creature;
                if (!creature->Create(sObjectMgr->GenerateCreatureSpawnId(), GetMap(), PHASEMASK_NORMAL, TEST_CREATURE_ENTRY, _location))
                {
                    delete creature;
                    TEST_ASSERT(false);
                }

                creature->SaveToDB(GetMap()->GetId(), (1 << GetMap()->GetSpawnMode()));
                ObjectGuid::LowType db_guid = creature->GetSpawnId();
                creature->LoadFromDB(db_guid, GetMap(), true, false);

                sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
                return creature;
            };

            masterCreature = spawnCreature();
            slaveCreature = spawnCreature();

            bool linked = sObjectMgr->SetCreatureLinkedRespawn(slaveCreature->GetSpawnId(), masterCreature->GetSpawnId());
            ASSERT_INFO("Creature link failed");
            TEST_ASSERT(linked);

            masterCreature->SetRespawnTime(1 * DAY);
            slaveCreature->SetRespawnTime(1 * SECOND);

            //Test
            masterCreature->KillSelf();
            slaveCreature->KillSelf();

            //give time to slave to respawn
            Wait(Seconds(10));

            ASSERT_INFO("Slave creature shouldn't have respawned if master is dead");
            TEST_ASSERT(!slaveCreature->IsAlive());

            ASSERT_INFO("Slave creature should have copied master respawn time"); //+ a little offset
            TEST_ASSERT(Between<time_t>(slaveCreature->GetRespawnTime(), masterCreature->GetRespawnTime(), masterCreature->GetRespawnTime() + MINUTE));
        }

        void Cleanup() override
        {
            if (masterCreature)
            {
                masterCreature->DisappearAndDie();
                masterCreature->DeleteFromDB();
                delete masterCreature;
            }
            if (slaveCreature)
            {
                slaveCreature->DisappearAndDie();
                slaveCreature->DeleteFromDB();
                delete slaveCreature;
            }
        }
    };

    std::unique_ptr<TestCase> GetTest() const override
    {
        return std::make_unique<CreatureLinkedRespawnTestImpl>();
    }
};

void AddSC_test_creature()
{
    new CreatureLinkedRespawnTest();
}
