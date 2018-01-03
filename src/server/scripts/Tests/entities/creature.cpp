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
        CreatureLinkedRespawnTestImpl() : TestCase(STATUS_INCOMPLETE, WorldLocation(13)) { }

        Creature* masterCreature = nullptr;
        Creature* slaveCreature = nullptr;

        void Test() override
        {
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
          
            sObjectMgr->SetCreatureLinkedRespawn(slaveCreature->GetSpawnId(), masterCreature->GetSpawnId());

            masterCreature->SetRespawnTime(1 * DAY);
            slaveCreature->SetRespawnTime(1 * SECOND);

            //Test
            masterCreature->DisappearAndDie();
            slaveCreature->DisappearAndDie();

            //give time to slave to respawn
            Wait(10);

            ASSERT_INFO("Slave creature shouldn't have respawned if master is dead");
            TEST_ASSERT(!slaveCreature->IsAlive());

            ASSERT_INFO("Slave creature should have copied master respawn time"); //+ a little offset
            TEST_ASSERT(Between<time_t>(slaveCreature->GetRespawnTime(), masterCreature->GetRespawnTime(), masterCreature->GetRespawnTime() + MINUTE));
        }

        void Cleanup() override
        {
            if (masterCreature)
            {
                masterCreature->DeleteFromDB();
            }
            if (slaveCreature)
            {
                slaveCreature->DeleteFromDB();
                sObjectMgr->SetCreatureLinkedRespawn(slaveCreature->GetSpawnId(), 0);
            }
        }
    };

    std::shared_ptr<TestCase> GetTest() const override
    {
        return std::make_shared<CreatureLinkedRespawnTestImpl>();
    }
};

void AddSC_test_creature()
{
    new CreatureLinkedRespawnTest();
}
