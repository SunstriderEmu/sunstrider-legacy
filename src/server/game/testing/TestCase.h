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
	void RemoveAllItems(TestPlayer* player) {}
	void RemoveItem(TestPlayer* player, uint32 itemID, uint32 count) {}
    void LearnTalent(TestPlayer* p, uint32 spellID);
    std::vector<uint32 /*SpellMissInfo count*/> GetHitChance(TestPlayer* caster, Unit* target, uint32 spellID);
    float CalcChance(uint32 iterations, const std::function<bool()>& f);
    ///!\ This is VERY slow, do not abuse of this function. Randomize talents, spells, stuff for this player
    void RandomizePlayer(TestPlayer* player);

    // Testing functions 
    // Test must use macro so that we can store from which line their calling. If calling function does a direct call without using the macro, we just print the internal line */

    //Will cast the spell a bunch of time and test if results match the expected damage. Reason I keep expected min and max separated is because it gives me some data to do some math magic later to reduce iterations 
    //Note for multithread: You can only have only one TestDirectSpellDamage function running for each caster/target combination at the same time
    #define TEST_DIRECT_SPELL_DAMAGE(caster, target, spellID, expectedMinDamage, expectedMaxDamage) _SetCaller(__FILE__, __LINE__); TestDirectSpellDamage(caster, target, spellID, expectedMinDamage, expectedMaxDamage); _ResetCaller()
    void TestDirectSpellDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedDamageMin, uint32 expectedDamageMax);
    #define TEST_DIRECT_HEAL(caster, target, spellID, expectedHealMin, expectedHealMax) _SetCaller(__FILE__, __LINE__); TestDirectHeal(caster, target, spellID, expectedHealMin, expectedHealMax); _ResetCaller()
    void TestDirectHeal(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedHealMin, uint32 expectedHealMax);
	#define TEST_MELEE_DAMAGE(player, target, attackType, expectedMin, expectedMax, crit) _SetCaller(__FILE__, __LINE__); TestMeleeDamage(player, target, attackType, expectedMin, expectedMax, crit); _ResetCaller()
	void TestMeleeDamage(TestPlayer* player, Unit* target, WeaponAttackType attackType, uint32 expectedMin, uint32 expectedMax, bool crit) {}

    float GetDamagePerSpellsTo(TestPlayer* caster, Unit* to, uint32 spellID);
    float GetChannelDamageTo(TestPlayer* caster, Unit* to, uint32 spellID, uint32 tickCount, bool& mustRetry);
    //use expectedAmount negative values for healing
    #define TEST_DOT_DAMAGE(caster, target, spellID, expectedAmount) _SetCaller(__FILE__, __LINE__); TestDotDamage(caster, target, spellID, expectedAmount); _ResetCaller()
    void TestDotDamage(TestPlayer* caster, Unit* target, uint32 spellID, int32 expectedAmount);

    #define TEST_CHANNEL_DAMAGE(caster, target, spellID, testedSpellID, tickCount, expectedAmount) _SetCaller(__FILE__, __LINE__); TestChannelDamage(caster, target, spellID, testedSpellID, tickCount, expectedAmount); _ResetCaller()
    void TestChannelDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 testedSpell, uint32 tickCount, int32 expectedTickAmount);

    #define TEST_STACK_COUNT(caster, target, talent, castSpellID, testSpellID, requireCount) _SetCaller(__FILE__, __LINE__); TestStacksCount(caster, target, talent, castSpellID, testSpellID, requireCount); _ResetCaller()
    void TestStacksCount(TestPlayer* caster, Unit* target, uint32 talent, uint32 castSpell, uint32 testSpell, uint32 requireCount);

    static uint32 GetTestBotAccountId();
    void _SetCaller(std::string callerFile, int32 callerLine);
    void _ResetCaller();
    void Celebrate();

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
    std::string              _callerFile; //used for error output
    int32                    _callerLine; //used for error output
    std::string              _assertInfo;

    bool _InternalSetup();
    void _Cleanup();
    void _Fail(const char* err, ...) ATTR_PRINTF(2, 3);
    void _FailNoException(std::string);
    void _SetName(std::string name) { _testName = name; }
    void _SetThread(TestThread* testThread) { _testThread = testThread; }
    //if callerFile and callerLine are specified, also print them in message
    void _Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition, bool increaseTestCount, std::string callerFile = "", int32 callerLine = 0);
    void _AssertInfo(const char* err, ...) ATTR_PRINTF(2, 3);
    void _ResetAssertInfo();

      std::string _GetCallerFile();
    int32 _GetCallerLine();

    /* return a new randomized test bot. Returned player must be deleted by the caller
    if level == 0, set bot at max player level
    */
    TestPlayer* _CreateTestBot(Position loc, Classes cls, Races race, uint32 level = 0);
    void _GetRandomClassAndRace(Classes& cls, Races& race, bool forcePower = false, Powers power = POWER_MANA);
    Classes _GetRandomClassForRace(Races race);
    Races _GetRandomRaceForClass(Classes race);
    static void _RemoveTestBot(Player* player);

    TestThread* _testThread;
};

#endif //TESTCASE_H