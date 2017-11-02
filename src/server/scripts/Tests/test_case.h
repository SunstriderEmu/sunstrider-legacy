#ifndef TEST_CASEOLD__H
#define TEST_CASEOLD__H
/*
#include "test_utils.h"

class TestCase
{
public:
    TestCase(bool mapIsolated = false);
    TestCase(bool mapIsolated, WorldLocation specificPosition);
    ~TestCase();

    //Spawn a player with random class, random race and level 70
    Player* SpawnRandomPlayer();
    // spawnInFront : spawn creature 10m in front of test location
    Creature* SpawnCreature(uint32 entry = 0, bool spawnInFront = true);
    Creature* SpawnCreatureWithPosition(Position spawnPosition, uint32 entry = 0);

    Player* SpawnPlayer(Classes _class, Races _race, uint32 level = 70, Position spawnPosition = {});

    //TOOLS
    static void TestStacksCount(Player* caster, Unit* target, uint32 talent, uint32 castSpell, uint32 testSpell, uint32 requireCount);

private:
    std::vector<WorldObject*> _spawnedObjects;
    WorldLocation _testLocation;
    bool _mapIsolated;
    Map* _testMap;
};

*/
#endif // TEST_CASEOLD__H