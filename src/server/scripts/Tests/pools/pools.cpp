#include "TestCase.h"
#include "TestPlayer.h"
#include "ObjectMgr.h"
#include "PoolMgr.h"
#include "GameTime.h"

// "pools maxlimit"
class PoolsMaxLimitTest : public TestCase
{
public:
    static uint32 const CREATURE_COUNT = 10;
    static uint32 const CREATURE_LIMIT = 5;
    static uint32 const RESPAWN_DELAY = 1;
    Creature* creatures[CREATURE_COUNT];

    void PrepareCreature(uint32 poolId, Creature*& c)
    {
        // PoolMgr currenctly only supports continent, not instances... so spawn on continent
        c = SpawnDatabaseCreature();
        c->SetRespawnDelay(RESPAWN_DELAY);
        TEST_ASSERT(poolId != 0);
        sPoolMgr->LoadEntry<Creature>(poolId, c->GetSpawnId(), 0.0f); //equal chanced

        TEST_ASSERT(!c->GetRespawnCompatibilityMode());
        TEST_ASSERT(bool(sPoolMgr->IsPartOfAPool<Creature>(c->GetSpawnId())));

        c->KillSelf();
    }

    uint32 CountActiveInPool()
    {
        uint32 count = 0;
        for (uint32 i = 0; i < CREATURE_COUNT; i++)
            count += uint32(sPoolMgr->IsSpawnedObject<Creature>(creatures[i]->GetSpawnId()));

        return count;
    }

    uint32 CountAlive()
    {
        uint32 count = 0;
        for (uint32 i = 0; i < CREATURE_COUNT; i++)
            count += uint32(creatures[i]->IsAlive());

        return count;
    }

    void Test() override
    {
        /* Commented out for now because:
        Currently PoolMgr handle handle continent. This test spawn creatures on continent instead of its own map but this
        leads to various racing conditions, we're not supposed to handle creatures on a map from another. I see no way of
        testing this without having the PoolMgr handle instances.

        // Setup
        TestPlayer* p = SpawnRandomPlayer(); //spawn any player to keep grid active 
        WaitNextUpdate();

        uint32 poolId = sPoolMgr->GetFirstFreePoolId<Creature>();
        sPoolMgr->LoadPoolTemplate(poolId, CREATURE_LIMIT, 0.0f);

        for (uint32 i = 0; i < CREATURE_COUNT; i++)
            PrepareCreature(poolId, creatures[i]);

        // Creature are NOT spawned on test map, we have to move the player on it. However due to a current teleport bug in the core, we have to move the player to another map THEN to the right map (bug when they share same id)
        auto ai = _GetCasterAI(p);
        p->TeleportTo(0, 0, 0, 0, 0);
        ai->HandleTeleportAck(); //immediately handle teleport packet
        p->TeleportTo(creatures[0]->GetWorldLocation());
        ai->HandleTeleportAck(); 
        TEST_ASSERT(p->GetMap() == creatures[0]->GetMap());
        TEST_ASSERT(p->GetMap() != nullptr);
        TEST_ASSERT(p->GetMap()->IsGridLoaded(_location));

        // Wait for respawn (world wait, not test map wait, since our creatures are on world map)
        uint32 startTime = GameTime::GetGameTime();
        uint32 waitTime = std::max(sWorld->getIntConfig(CONFIG_RESPAWN_MINCHECKINTERVALMS) / 1000, uint32(RESPAWN_DELAY)) + 1;
        for (uint32 i = 0; ; i++)
        {
            Wait(1s);
            if (GameTime::GetGameTime() > startTime + waitTime)
                break;
            if (i >= 100)
            {
                ASSERT_INFO("Logic failure in world wait logic");
                TEST_ASSERT(false);
            }
        }

        uint32 const countActive = CountActiveInPool();
        ASSERT_INFO("Count active is %u instead of %u", countActive, CREATURE_LIMIT);
        TEST_ASSERT(countActive == CREATURE_LIMIT);
        uint32 const countAlive = CountAlive();
        ASSERT_INFO("Count alive is %u instead of %u", countAlive, CREATURE_LIMIT);
        TEST_ASSERT(countAlive == CREATURE_LIMIT);
        */
    }

    void Cleanup() override
    {
        /*
        for (uint32 i = 0; i < CREATURE_COUNT; i++)
            CleanupDBCreature(creatures[i]);
            */
    }
};

void AddSC_test_pools()
{
    RegisterTestCase("pools maxlimit", PoolsMaxLimitTest);
}
