#ifndef TESTCASE_H
#define TESTCASE_H

class TestMap;
class TestThread;
class TestPlayer;

//input info for next TEST_ASSERT check
#define ASSERT_INFO(expr, ...) _AssertInfo(expr, ## __VA_ARGS__)
#define TEST_ASSERT( expr ) Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr); _ResetAssertInfo()

template<class T>
bool Between(T value, T from, T to)
{
    if (from > to) //extra protection against both typos and underflow
        return false;
    return value >= from && value <= to;
}

class TC_GAME_API TestCase
{
    friend class TestMgr;
    friend class TestThread;
    friend class TestMap;

public:
    //If needMap is specified, test will be done in an instance of the test map (13). 
    TestCase(bool needMap = true);
    //Use specific position. If only map was specified in location, default coordinates in map may be chosen instead. If you need creatures and objects, use EnableMapObjects in your test constructor
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
    #define EQUIP_ITEM(player, itemID) _SetCaller(__FILE__, __LINE__); _EquipItem(player, itemID); _ResetCaller()

    void RemoveAllEquipedItems(TestPlayer* player);
    void RemoveItem(TestPlayer* player, uint32 itemID, uint32 count);
    void LearnTalent(TestPlayer* p, uint32 spellID);

    /* Cast a spell and check for spell start return value
    Usage:
    #define TEST_CAST(caster, victim, spellID) 
    #define TEST_CAST(caster, victim, spellID, expectedCode) 
    #define TEST_CAST(caster, victim, spellID, expectedCode, triggeredFlags) 
    */
    #define TEST_CAST( ... ) _SetCaller(__FILE__, __LINE__);  _TestCast(__VA_ARGS__); _ResetCaller()

    /* Usage 
    TEST_HAS_AURA(target, spellID)
    TEST_HAS_AURA(target, spellID, effectIndex)
    */
    #define TEST_HAS_AURA( ... ) _SetCaller(__FILE__, __LINE__); _EnsureHasAura(__VA_ARGS__); _ResetCaller()
    /* Usage
    TEST_HAS_NOT_AURA(target, spellID)
    TEST_HAS_NOT_AURA(target, spellID, effectIndex)
    */
    #define TEST_HAS_NOT_AURA( ... ) _SetCaller(__FILE__, __LINE__); _EnsureHasNotAura(__VA_ARGS__); _ResetCaller()
 
    //check if target has aura and if duration match given duration. Only checks effect0
    #define TEST_AURA_MAX_DURATION(target, spellID, durationMS) _SetCaller(__FILE__, __LINE__); _TestAuraMaxDuration(target, spellID, durationMS); _ResetCaller()

    std::vector<uint32 /*SpellMissInfo count*/> GetHitChance(TestPlayer* caster, Unit* target, uint32 spellID);
    float CalcChance(uint32 iterations, const std::function<bool()>& f);
    ///!\ This is VERY slow, do not abuse of this function. Randomize talents, spells, stuff for this player
    void RandomizePlayer(TestPlayer* player);

    // Testing functions 
    // Test must use macro so that we can store from which line their calling. If calling function does a direct call without using the macro, we just print the internal line */

    /* Will cast the spell a bunch of time and test if results match the expected damage.
     Caster must be a TestPlayer or a pet/summon of him
     Note for multithread: You can only have only one TestDirectSpellDamage function running for each caster/target combination at the same time*/
    #define TEST_DIRECT_SPELL_DAMAGE(caster, target, spellID, expectedMinDamage, expectedMaxDamage, crit) _SetCaller(__FILE__, __LINE__); _TestDirectValue(caster, target, spellID, expectedMinDamage, expectedMaxDamage, crit, true); _ResetCaller()
    //Caster must be a TestPlayer or a pet/summon of him
    #define TEST_DIRECT_HEAL(caster, target, spellID, expectedHealMin, expectedHealMax, crit) _SetCaller(__FILE__, __LINE__); _TestDirectValue(caster, target, spellID, expectedHealMin, expectedHealMax, crit, false); _ResetCaller()
    //Caster must be a TestPlayer or a pet/summon of him
	#define TEST_MELEE_DAMAGE(player, target, attackType, expectedMin, expectedMax, crit) _SetCaller(__FILE__, __LINE__); _TestMeleeDamage(player, target, attackType, expectedMin, expectedMax, crit); _ResetCaller()
  
    //use expectedAmount negative values for healing
    #define TEST_DOT_DAMAGE(caster, target, spellID, expectedAmount, crit) _SetCaller(__FILE__, __LINE__); _TestDotDamage(caster, target, spellID, expectedAmount, crit); _ResetCaller()
  
    #define TEST_CHANNEL_DAMAGE(caster, target, spellID, testedSpellID, tickCount, expectedAmount) _SetCaller(__FILE__, __LINE__); _TestChannelDamage(caster, target, spellID, testedSpellID, tickCount, expectedAmount); _ResetCaller()

    #define TEST_STACK_COUNT(caster, target, talent, castSpellID, testSpellID, requireCount) _SetCaller(__FILE__, __LINE__); _TestStacksCount(caster, target, castSpellID, testSpellID, requireCount); _ResetCaller()

    //Cast given spell and check its power cost
	#define TEST_POWER_COST(caster, target, castSpellID, powerType, expectedPowerCost) _SetCaller(__FILE__, __LINE__); _TestPowerCost(caster, target, castSpellID, powerType, expectedPowerCost); _ResetCaller()

    #define TEST_HAS_COOLDOWN(caster, spellID, cooldownSecond) _SetCaller(__FILE__, __LINE__); _TestHasCooldown(caster, spellID, cooldownSecond); _ResetCaller()

    bool GetDamagePerSpellsTo(TestPlayer* caster, Unit* to, uint32 spellID, uint32& minDamage, uint32& maxDamage);
    bool GetHealingPerSpellsTo(TestPlayer* caster, Unit* target, uint32 spellID, uint32& minHeal, uint32& maxHeal);
    float GetChannelDamageTo(TestPlayer* caster, Unit* to, uint32 spellID, uint32 tickCount, bool& mustRetry);

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

    void _AssertInfo(const char* err, ...) ATTR_PRINTF(2, 3);
    void _ResetAssertInfo();
    void _SetCaller(std::string callerFile, int32 callerLine);
    void _ResetCaller();
    void Celebrate();

    void _TestDirectValue(Unit* caster, Unit* target, uint32 spellID, uint32 expectedMin, uint32 expectedMax, bool crit, bool damage); //if !damage, then use healing
    void _TestMeleeDamage(Unit* caster, Unit* target, WeaponAttackType attackType, uint32 expectedMin, uint32 expectedMax, bool crit);
    void _TestDotDamage(TestPlayer* caster, Unit* target, uint32 spellID, int32 expectedAmount, bool crit);
	void _SetCriticalChances(Unit* caster, bool crit);
    void _TestChannelDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 testedSpell, uint32 tickCount, int32 expectedTickAmount);
	void _TestStacksCount(TestPlayer* caster, Unit* target, uint32 castSpell, uint32 testSpell, uint32 requireCount);
	void _TestPowerCost(TestPlayer* caster, Unit* target, uint32 castSpell, Powers powerType, uint32 expectedPowerCost);
    void _EquipItem(TestPlayer* p, uint32 itemID);
    //if negative, ensure has NOT aura
    void _EnsureHasAura(Unit* target, int32 spellID);
    //if negative, ensure has NOT aura
    void _EnsureHasAura(Unit* target, int32 spellID, uint8 effectIndex);
    void _EnsureHasNotAura(Unit* target, int32 spellID) { _EnsureHasAura(target, -spellID); }
    void _EnsureHasNotAura(Unit* target, int32 spellID, uint8 effectIndex) { _EnsureHasAura(target, -spellID, effectIndex);  }
    void _TestHasCooldown(TestPlayer* caster, uint32 castSpellID, uint32 cooldownSecond);
    void _TestAuraMaxDuration(Unit* target, uint32 spellID, uint32 durationMS);
    void _TestCast(Unit* caster, Unit* victim, uint32 spellID, SpellCastResult expectedCode = SPELL_CAST_OK, TriggerCastFlags triggeredFlags = TRIGGERED_NONE);

private:
    std::string              _testName;
    std::string              _errMsg;
    uint32                   _testsCount;
    bool                     _failed;
    std::atomic<bool>        _setup;
    bool                     _enableMapObjects;
    std::string              _callerFile; //used for error output
    int32                    _callerLine; //used for error output
    std::string              _internalAssertInfo;
    std::string              _assertInfo;

    bool _InternalSetup();
    void _Cleanup();
    void _Fail(const char* err, ...) ATTR_PRINTF(2, 3);
    void _FailNoException(std::string);
    void _SetName(std::string name) { _testName = name; }
    void _SetThread(TestThread* testThread) { _testThread = testThread; }
    //if callerFile and callerLine are specified, also print them in message
    void _Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition, bool increaseTestCount, std::string callerFile = "", int32 callerLine = 0);
    void _InternalAssertInfo(const char* err, ...) ATTR_PRINTF(2, 3);
    void _ResetInternalAssertInfo();

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

    static void _GetApproximationParams(uint32& sampleSize, uint32& allowedError, uint32 const expectedMin, uint32 const expectedMax);
   
    TestThread* _testThread;

    //those two just to help avoiding calling SpawnRandomPlayer with the wrong arguments, SpawnPlayer should be called in those case
    TestPlayer* SpawnRandomPlayer(Races race, Classes cls) { return nullptr; }
    TestPlayer* SpawnRandomPlayer(Classes cls, Races races) { return nullptr; }
};

#endif //TESTCASE_H