#include "TestCase.h"
#include "TestPlayer.h"
#include "ObjectMgr.h"

class CreatureLinkedRespawnTest : public TestCase
{
public:
    CreatureLinkedRespawnTest() : TestCase(WorldLocation(560))
    { }

    Creature* masterCreature = nullptr;
    Creature* slaveCreature  = nullptr;

    void Test() override
    {
        masterCreature = SpawnDatabaseCreature();
        slaveCreature  = SpawnDatabaseCreature();

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
        CleanupDBCreature(masterCreature);
        CleanupDBCreature(slaveCreature);
    }
};

void AddSC_test_creature()
{
    RegisterTestCase("creature linkedrespawn", CreatureLinkedRespawnTest);
}
