#ifndef TESTCASE_H
#define TESTCASE_H

class TestMap;
class TestThread;
class TestPlayer;

#define TEST_ASSERT( expr ) Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr)

template<class T>
bool Between(T value, T from, T to)
{
    return value >= from && value <= to;
}

class TC_GAME_API TestCase
{
    friend class TestMgr;
    friend class TestThread;
    friend class TestMap;

public:
    //If needMap is specified, test will be done in an instance of the test map (13)
    TestCase(bool needMap = true);
    //Use specific position. If only map was specified in location, default coordinates in map may be chosen instead
    TestCase(WorldLocation const& specificPosition);

    std::string GetName() const { return _testName; }
    bool Failed() const { return _failed; }
    std::string GetError() const { return _errMsg; }
    uint32 GetTestCount() const { return _testsCount; }
    bool IsSetup() const { return _setup; }
    WorldLocation const& GetLocation() const { return _location; }
    //Get a hardcoded default position on map to place the test instead of always going to 0,0,0
    static Position GetDefaultPositionForMap(uint32 mapId);

    // Main test function to be implemented by each test
    virtual void Test() = 0;


    // Utility functions
    void SetDifficulty(Difficulty diff) { _diff = diff; }
    TestMap* GetMap() const { return _map; }
    void EnableMapObjects();
    
    //Spawn player. Fail test on failure
    TestPlayer* SpawnRandomPlayer();
    //Spawn player. Fail test on failure
    TestPlayer* SpawnRandomPlayer(Powers power, uint32 level = 70);
    //Spawn player. Fail test on failure
    TestPlayer* SpawnRandomPlayer(Races race, uint32 level = 70);
    //Spawn player. Fail test on failure
    TestPlayer* SpawnRandomPlayer(Classes cls, uint32 level = 70);
    //Spawn player. Fail test on failure
    TestPlayer* SpawnPlayer(Classes cls, Races _race, uint32 level = 70, Position spawnPosition = {});
    //Spawn creature. Fail test on failure
    TempSummon* SpawnCreature(uint32 entry = 0, bool spawnInFront = true);
    //Spawn creature. Fail test on failure
    TempSummon* SpawnCreatureWithPosition(Position spawnPosition, uint32 entry = 0);

    //This checks if item exists in loot (but we cannot say if player can actually loot it)
    bool HasLootForMe(Creature*, Player*, uint32 itemID);

    //Create item and equip it to player. Will remove any item already in slot. Fail test on failure
    void EquipItem(TestPlayer* p, uint32 itemID);
    void LearnTalent(TestPlayer* p, uint32 spellID);

    //Will cast the spell a bunch of time and test if results match the expected damage. Reason I keep expected min and max separated is because it gives me some data to do some math magic later to reduce iterations 
    //Note for multithread: You can only have only one TestSpellDamage function running for each caster/target combination at the same time
    void TestSpellDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedDamageMin, uint32 expectedDamageMax);
    void TestHeal(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedHealMin, uint32 expectedHealMax);
    std::vector<uint32 /*SpellMissInfo count*/> GetHitChance(TestPlayer* caster, Unit* target, uint32 spellID);
    void DisableRegen(TestPlayer* caster);

    void TestStacksCount(TestPlayer* caster, Unit* target, uint32 talent, uint32 castSpell, uint32 testSpell, uint32 requireCount);
    ///!\ This is VERY slow, do not abuse of this function. Randomize talents, spells, stuff for this player
    void RandomizePlayer(TestPlayer* player);

    static uint32 GetTestBotAccountId();
protected:

    //Scripting function
    void Wait(uint32 ms);
    //Main check function, used by TEST_ASSERT macro. Will stop execution on failure
    void Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition);

    // Test Map
    TestMap*                 _map;
    uint32                   _testMapInstanceId;
    Difficulty               _diff;
    WorldLocation            _location;

private:
    std::string              _testName;
    std::string              _errMsg;
    uint32                   _testsCount;
    bool                     _failed;
    std::atomic<bool>        _setup;
    bool                     _enableMapObjects;

    bool _InternalSetup();
    void _Cleanup();
    void _Fail(const char* err, ...) ATTR_PRINTF(2, 3);
    void _FailNoException(std::string);
    void _SetName(std::string name) { _testName = name; }
    void _SetThread(TestThread* testThread) { _testThread = testThread; }
    void _Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition, bool increaseTestCount);

    /* return a new randomized test bot. Returned player must be deleted by the caller
    if level == 0, set bot at max player level
    */
    TestPlayer* _CreateTestBot(WorldLocation loc, Classes cls, Races race, uint32 level = 0);
    void _GetRandomClassAndRace(Classes& cls, Races& race, bool forcePower = false, Powers power = POWER_MANA);
    Classes _GetRandomClassForRace(Races race);
    Races _GetRandomRaceForClass(Classes race);
    static void _RemoveTestBot(Player* player);

    TestThread* _testThread;
};

#endif //TESTCASE_H