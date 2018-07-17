#ifndef TESTCASE_H
#define TESTCASE_H

#include "Duration.h"
#include <string>
#include "SharedDefines.h"
#include "DBCEnums.h"
#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "Unit.h"
#include "PlayerbotAI.h"
#include "TestDefines.h"
#include "TestSectionResult.h"
#include <functional>

class TestMap;
class TestThread;
class TestPlayer;
class WorldLocation;
class Player;
class TestPlayer;
class Creature;
class GameObject;
struct Position;
class TempSummon;


//input info for next TEST_* check
#define ASSERT_INFO(expr, ...) _AssertInfo(expr, ## __VA_ARGS__)
#define TEST_ASSERT( expr ) { Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr); _ResetAssertInfo(); }

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
    TestCase();
    TestCase(TestStatus status); //remove later when test global status is removed, kept for legacy compat
    //Use specific position. If only map was specified in location, default coordinates in map may be chosen instead. If you need creatures and objects, use EnableMapObjects in your test constructor
    TestCase(TestStatus status, WorldLocation const specificPosition);
    ~TestCase() {}
    void SetUsedPattern(std::string const& pattern) { _usedPattern = pattern; }

    std::string GetName() const { return _testName; }
    //bool Failed() const { return _failed; }
    //std::string GetError() const { return _errMsg; }
    std::list<TestSectionResult> const& GetResults() const { return _results; }
    //return true if any TestSectionResult has failures
    bool HasFailures();
    bool IsSetup() const { return _setup; }
    WorldLocation const& GetLocation() const { return _location; }
    //Get a hardcoded default position on map to place the test instead of always going to 0,0,0
    static Position GetDefaultPositionForMap(uint32 mapId);

    //Define a test section. A test section is executed immediately. Failures are logged but do not stop the test.
    void SECTION(std::string const title, TestStatus status, std::function<void()> func);
    //same but with default STATUS_PASSING
    void SECTION(std::string const title, std::function<void()> func);
    //function to execute before each section
    void BEFORE_EACH(std::function<void()> func);
    //function to execute after each section
    void AFTER_EACH(std::function<void()> func);

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
    //Spawn boss. Fail test on failure
    TempSummon* SpawnBoss(bool spawnInFront = true);
    //Spawn creature. Fail test on failure
    TempSummon* SpawnCreatureWithPosition(Position spawnPosition, uint32 entry = 0);

    //This checks if item exists in loot (but we cannot say if player can actually loot it)
    bool HasLootForMe(Creature*, Player*, uint32 itemID);
    //Create item and equip it to player. Will remove any item already in slot. Fail test on failure
    #define EQUIP_NEW_ITEM(player, itemID) { _SetCaller(__FILE__, __LINE__); _EquipItem(player, itemID, true); _ResetCaller(); }
    //Equip an item from player inventory. Will remove any item already in slot. Fail test on failure
    #define EQUIP_EXISTING_ITEM(player, itemID) { _SetCaller(__FILE__, __LINE__); _EquipItem(player, itemID, false); _ResetCaller(); }

    void RemoveAllEquipedItems(TestPlayer* player);
    void RemoveItem(TestPlayer* player, uint32 itemID, uint32 count);
    void LearnTalent(TestPlayer* p, uint32 spellID);
    void EnableCriticals(Unit* caster, bool crit);
    void RestoreCriticals(Unit* caster);
    void SaveCriticals(Unit* caster);

    //Invite player into leader group. Group is created if not yet existing.
    void GroupPlayer(TestPlayer* leader, Player* player);
    static std::string StringifySpellCastResult(uint32 result) { return StringifySpellCastResult(SpellCastResult(result)); }
    static std::string StringifySpellCastResult(SpellCastResult result);

    /* # Test macros are guaranteed to 
        - Preserve health and power of caster and victim
        - Preserve ForceHitResult values
        - Preserve critical score
       # Not guaranteed
        - Threat conservation
    */

    /* Cast a spell and check for spell start return value (SpellCastResult)
    Usage:
    TEST_CAST(Unit* caster, Unit* victim, uint32 spellID, SpellCastResult expectedCode = SPELL_CAST_OK, CastSpellExtraArgs args = {})
    */
    #define TEST_CAST( ... ) { _SetCaller(__FILE__, __LINE__);  _TestCast(__VA_ARGS__); _ResetCaller(); }

    /* Cast a spell with forced hit result (SpellMissInfo). Fails test if spell fail to launch.
    Usage:
    FORCE_CAST(Unit* caster, Unit* victim, uint32 spellID, SpellMissInfo forcedMissInfo = SPELL_MISS_NONE, CastSpellExtraArgs args = {})
    */
    #define FORCE_CAST( ... ) { _SetCaller(__FILE__, __LINE__);  _ForceCast(__VA_ARGS__); _ResetCaller(); }

    /* 
    */
    #define TEST_HAS_AURA(target, spellID) { _SetCaller(__FILE__, __LINE__); _EnsureHasAura(target, spellID); _ResetCaller(); }
    /*
    */
    #define TEST_HAS_NOT_AURA(target, spellID) { _SetCaller(__FILE__, __LINE__); _EnsureHasNotAura(target, spellID); _ResetCaller(); }

    /* check if target has aura and if maximum duration match given duration
    @durationMS Can be either uint32 or std::chrono::duration (such as Milliseconds)
    */
    #define TEST_AURA_MAX_DURATION(target, spellID, durationMS) { _SetCaller(__FILE__, __LINE__); _TestAuraMaxDuration(target, spellID, durationMS); _ResetCaller(); }

    #define TEST_AURA_STACK(target, spellID, stacks) { _SetCaller(__FILE__, __LINE__); _TestAuraStack(target, spellID, stacks, true); _ResetCaller(); }
    #define TEST_AURA_CHARGE(target, spellID, stacks) { _SetCaller(__FILE__, __LINE__); _TestAuraStack(target, spellID, stacks, false); _ResetCaller(); }

    float CalcChance(uint32 iterations, const std::function<bool()>& f);
    ///!\ This is VERY slow, do not abuse of this function. Randomize talents, spells, stuff for this player
    void RandomizePlayer(TestPlayer* player);

    // Testing functions 
    // Test must use macro so that we can store from which line their calling. If calling function does a direct call without using the macro, we just print the internal line */

    typedef std::function<void(Unit*, Unit*)> TestCallback;
    inline static void DefaultCallback(Unit*, Unit*) {};

    /* Will cast the spell a bunch of time and test if results match the expected damage.
       Usage: 
        TEST_DIRECT_SPELL_DAMAGE(Unit* caster, Unit* target, uint32 spellID, uint32 expectedMin, uint32 expectedMax, bool crit, TestCallback callback = {}, uint32 testedSpellId = 0);

        @caster must be a TestPlayer or a pet/summon of him
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
        @testedSpellId check result for this spell instead of spellID. May be useful for spell proccing other spells.
     */
    #define TEST_DIRECT_SPELL_DAMAGE(...) { _SetCaller(__FILE__, __LINE__); _TestDirectValue(false, __VA_ARGS__); _ResetCaller(); }
    /* Same as TEST_DIRECT_SPELL_DAMAGE but you can give a callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
       Usage:
        TEST_DIRECT_HEAL(Unit* caster, Unit* target, uint32 spellID, uint32 expectedMin, uint32 expectedMax, bool crit, TestCallback callback = {}, uint32 testedSpellId = 0);

        @caster must be a TestPlayer or a pet/summon of him
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_DIRECT_HEAL(...) { _SetCaller(__FILE__, __LINE__); _TestDirectValue(true, __VA_ARGS__); _ResetCaller(); }
    /*
       Usage:
        TEST_MELEE_DAMAGE((Unit* caster, Unit* target, WeaponAttackType attackType, uint32 expectedMin, uint32 expectedMax, bool crit, TestCallback callback = {})

        @caster must be a TestPlayer or a pet/summon of him
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_MELEE_DAMAGE(...) { _SetCaller(__FILE__, __LINE__); _TestMeleeDamage(__VA_ARGS__); _ResetCaller(); }

    /* Will cast spell and check if threat is equal to dmg/healing done multiplied by expectedThreatFactor. Group of caster may be disbanded if any. Target aura and caster aura may be removed.
       Usage:
        TEST_THREAT(Unit* caster, Creature* target, uint32 spellID, float expectedThreatFactor, bool heal = false, TestCallback callback = {})
        
        @caster must be a TestPlayer or a pet/summon of him
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_THREAT(...)  { _SetCaller(__FILE__, __LINE__); _TestThreat(__VA_ARGS__); _ResetCaller(); }
    /* 
    @expectedAmount negative values for healing
    @crit Set crit score of caster to maximum
    */
    #define TEST_DOT_DAMAGE(caster, target, spellID, expectedTotalAmount, crit) { _SetCaller(__FILE__, __LINE__); _TestDotDamage(caster, target, spellID, expectedTotalAmount, crit); _ResetCaller(); }
  
    /* Usage:
    TEST_CHANNEL_DAMAGE(Unit* caster, Unit* target, uint32 spellID, uint32 tickCount, int32 expectedTickAmount, uint32 testedSpell = 0)
    */
    #define TEST_CHANNEL_DAMAGE(...) { _SetCaller(__FILE__, __LINE__); _TestChannelDamage(false, __VA_ARGS__); _ResetCaller(); }
    /* Usage:
    TEST_CHANNEL_HEALING(Unit* caster, Unit* target, uint32 spellID, uint32 tickCount, int32 expectedTickAmount, uint32 testedSpell = 0)
    */
    #define TEST_CHANNEL_HEALING(...) { _SetCaller(__FILE__, __LINE__); _TestChannelDamage(true, __VA_ARGS__); _ResetCaller(); }

    /* Cast given spells a bunch of time from caster on victim, and test if results are chance% given missInfo
       Usage:
        TEST_SPELL_HIT_CHANCE(Unit* caster, Unit* victim, uint32 spellID, float chance, SpellMissInfo missInfo, TestCallback callback = {})

        @chance 0-100
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_SPELL_HIT_CHANCE(...) { _SetCaller(__FILE__, __LINE__); _TestSpellHitChance(__VA_ARGS__); _ResetCaller(); }
    /* Triggers attack from caster on victim, and test if results are chance% given missInfo
       Usage:
        TEST_MELEE_HIT_CHANCE(Unit* caster, Unit* victim, WeaponAttackType weaponAttackType, float chance, MeleeHitOutcome meleeHitOutcome, TestCallback callback = {})

        @chance 0-100
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_MELEE_HIT_CHANCE(...) { _SetCaller(__FILE__, __LINE__); _TestMeleeHitChance(__VA_ARGS__); _ResetCaller(); }
    // Test the percentage of a melee hit outcome for already done attacks
    #define TEST_MELEE_OUTCOME_PERCENTAGE(attacker, victim, weaponAttackType, meleeHitOutcome, expectedResult, allowedError)  { _SetCaller(__FILE__, __LINE__); _TestMeleeOutcomePercentage(attacker, victim, weaponAttackType, meleeHitOutcome, expectedResult, allowedError);  _ResetCaller(); }
    // Test the percentage of a spell hit outcome for already done attacks
    #define TEST_SPELL_OUTCOME_PERCENTAGE(attacker, victim, spellId, missType, expectedResult, allowedError)  { _SetCaller(__FILE__, __LINE__); _TestSpellOutcomePercentage(attacker, victim, spellId, missType, expectedResult, allowedError);  _ResetCaller(); }

    //stub, todo!
    #define TEST_STACK_COUNT(caster, target, talent, castSpellID, testSpellID, requireCount) { _SetCaller(__FILE__, __LINE__); _TestStacksCount(caster, target, castSpellID, testSpellID, requireCount); _ResetCaller(); }

    /* Check power cost of spell. Spells is not actually casted and conditions are ignored
    */
    #define TEST_POWER_COST(caster, castSpellID, powerType, expectedPowerCost) { _SetCaller(__FILE__, __LINE__); _TestPowerCost(caster, castSpellID, powerType, expectedPowerCost); _ResetCaller(); }

    /* Cast given spell (fully triggered) and check if CD is triggered
    This will not trigger any CD but keep in mind that spell effects will be applied
    durationMS: can be either uint32 or std::chrono::duration (such as Milliseconds)
    */
    #define TEST_COOLDOWN(caster, target, castSpellID, durationMS) { _SetCaller(__FILE__, __LINE__); _TestCooldown(caster, target, castSpellID, durationMS); _ResetCaller(); }

    /* Check remaining cooldown for given spellID
    cooldownSecond: can be either uint32 or std::chrono::duration (such as Seconds)
    */
    #define TEST_HAS_COOLDOWN(caster, spellID, cooldownSecond) { _SetCaller(__FILE__, __LINE__); _TestHasCooldown(caster, spellID, cooldownSecond); _ResetCaller(); }

    /* Use first item found in inventory with given id
    Will fail if ALL spells of the items failed
    */
    #define USE_ITEM(caster, target, itemID) { _SetCaller(__FILE__, __LINE__); _TestUseItem(caster, target, itemID); _ResetCaller(); }

    /* Test the crit chance of given spell on target
       Usage:
        TEST_SPELL_CRIT_CHANCE(Unit* caster, Unit* victim, uint32 spellID, float chance, TestCallback callback = {})

        @chance 0-100
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_SPELL_CRIT_CHANCE(...) { _SetCaller(__FILE__, __LINE__); _TestSpellCritChance(__VA_ARGS__); _ResetCaller(); }

    /* Check if spell has given cast time for caster 
    (Does not actually cast the spell)
    */
    #define TEST_CAST_TIME(caster, spellID, expectedCastTimeMS) { _SetCaller(__FILE__, __LINE__); _TestSpellCastTime(caster, spellID, expectedCastTimeMS); _ResetCaller(); }

    /* Apply aura of given spell from caster to target, then check if caster has aura after each tick.
    You should cleanup tick effect in the callback before returning result.
    chance: 0-100
    checkSelf: Check if aura proc on caster. Else, on target.
    */
    typedef std::function<bool(Unit*, Unit*)> TestCallbackResult;
    #define TEST_AURA_TICK_PROC_CHANCE(caster, target, spellID, effIdx, chance, checkSelf, spellProcId)  { _SetCaller(__FILE__, __LINE__); _TestAuraTickProcChance(caster, target, spellID, effIdx, chance, procSpellId, checkSelf); _ResetCaller(); }
    /* Same but with additional argument:
    callback: function to use after each tick to determine success.
    */
    #define TEST_AURA_TICK_PROC_CHANCE_CALLBACK(caster, target, spellID, effIdx, chance, procSpellId, callback)  { _SetCaller(__FILE__, __LINE__); _TestAuraTickProcChanceCallback(caster, target, spellID, effIdx, chance, procSpellId, callback); _ResetCaller(); }


    /* Test the proc chance of given aura on caster or victim by casting a spell on a target
       Usage:
        TEST_SPELL_PROC_CHANCE(Unit* caster, Unit* target, uint32 spellID, uint32 procSpellID, bool selfProc, float expectedChancePercent, SpellMissInfo missInfo, bool crit, TestCallback callback = {})

        @selfProc Check of on caster, else check on target
        @missInfo Force spell hit result
        @chance 0-100
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_SPELL_PROC_CHANCE(...) { _SetCaller(__FILE__, __LINE__); _TestSpellProcChance(__VA_ARGS__); _ResetCaller(); }

    /* Test the proc chance of given aura on caster or victim by attacking target on melee/ranged
       Usage:
        TEST_MELEE_PROC_CHANCE(Unit* attacker, Unit* target, uint32 procSpellID, bool selfProc, float expectedChancePercent, MeleeHitOutcome meleeHitOutcome, WeaponAttackType attackType, TestCallback callback = {})

        @selfProc Check of on caster, else check on target
        @meleeHitOutcome Force melee hit result
        @chance 0-100
        @callback function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */

    #define TEST_MELEE_PROC_CHANCE(...)  { _SetCaller(__FILE__, __LINE__); _TestMeleeProcChance(__VA_ARGS__); _ResetCaller(); }

    /* Caster will cast spell on target. Dispeler will then try to dispel it and should be resisted given chance of the time.
       Usage:
        TEST_DISPEL_RESIST_CHANCE(Unit* caster, Unit* target, Unit* dispeler, uint32 spellID, float chance, TestCallback callback = {})

        @chance: 0-100
        @callback: function to use before each cast, with the type std::function<void(Unit*, Unit*)>
    */
    #define TEST_DISPEL_RESIST_CHANCE(...) { _SetCaller(__FILE__, __LINE__); _TestSpellDispelResist(__VA_ARGS__); _ResetCaller(); }

    /* Test how much of the time a cast pushback is resisted (against melee attacks). Target will also be the one attacking the caster.
    chance: 0-100
    */
    #define TEST_PUSHBACK_RESIST_CHANCE(caster, target, spellID, chance)  { _SetCaller(__FILE__, __LINE__); _TestPushBackResistChance(caster, target, spellID, chance); _ResetCaller(); }
    /* Test if target is in range for given spell
    Spell may or may not be really casted on target in this function
    Caster and target combat reach get automatically added in this function, EXCEPT for ranged weapon spells.
       Usage:
         TEST_RANGE(TestPlayer* caster, Unit* target, uint32 spellID, float maxRange, float minRange = 0.0)
    */
    #define TEST_RANGE(...) { _SetCaller(__FILE__, __LINE__); _TestRange(__VA_ARGS__); _ResetCaller(); }
    /* Test if target is in range of spell for given radius (use this of AoE spells, else use TEST_RANGE)
    Spell may or may not be casted on target after this
       Usage:
        TEST_RADIUS(TestPlayer* caster, Unit* castTarget, Unit* checkTarget, uint32 spellID, float radius, bool heal = false, uint32 checkSpellID = 0, bool includeCasterReach = true)
    */
    #define TEST_RADIUS(...) { _SetCaller(__FILE__, __LINE__); _TestRadius(__VA_ARGS__); _ResetCaller(); }

    //Get raw spell data from caster (+ pets) to target with spellID
    std::vector<PlayerbotTestingAI::SpellDamageDoneInfo> GetSpellDamageDoneInfoTo(Unit* caster, Unit* victim, uint32 spellID, bool checkEmpty = true);
    //Get raw healing data from caster (+ pets) to target with spellID
    std::vector<PlayerbotTestingAI::HealingDoneInfo> GetHealingDoneInfoTo(Unit* caster, Unit* target, uint32 spellID, bool checkEmpty = true);
    //crit: get only spells that made crit / only spells that did not
    std::pair<uint32 /*minDmg*/, uint32 /*maxDmg*/> GetDamagePerSpellsTo(Unit* caster, Unit* to, uint32 spellID, Optional<bool> crit, uint32 expectedCount = 0);
    //crit: get only spells that made crit / only spells that did not
    std::pair<uint32 /*minHeal*/, uint32 /*maxHeal*/> GetHealingPerSpellsTo(Unit* caster, Unit* target, uint32 spellID, Optional<bool> crit, uint32 expectedCount = 0);
    
    std::pair<uint32 /*minDmg*/, uint32 /*maxDmg*/> GetWhiteDamageDoneTo(Unit* caster, Unit* target, WeaponAttackType attackType, bool critical, uint32 expectedCount = 0);
    //return total channel damage done
    uint32 GetChannelDamageTo(Unit* caster, Unit* target, uint32 spellID, uint32 expectedTickCount, Optional<bool> crit);
    //return total channel heal done
    uint32 GetChannelHealingTo(Unit* caster, Unit* target, uint32 spellID, uint32 expectedTickCount, Optional<bool> crit);

    static uint32 GetTestBotAccountId();
    TestStatus GetTestStatus() const { return _testStatus; } //for compat, remove later

protected:
    // Main test function to be implemented by each test
    virtual void Test() = 0;
    // Cleanup function, always called after the test whatever the result
    virtual void Cleanup() { }

    //Scripting function
    void Wait(uint32 ms);
    void Wait(Seconds s);
    void Wait(Milliseconds s);
    void WaitNextUpdate();
    //This will make the thread pause itself and resume in the next udpate if it has been running too long
    void HandleThreadPause();
    //Free finished spells from memory. This is usually done at next map update but we may go a long time without it in tests.
    void HandleSpellsCleanup(Unit* caster);
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
    void Sadness();

    // <Test macros related functions>
    //heal: check for healing instead of damage
    void _TestDirectValue(bool heal, Unit* caster, Unit* target, uint32 spellID, uint32 expectedMin, uint32 expectedMax, bool crit, TestCallback callback = DefaultCallback, uint32 testedSpellId = 0);
    void _TestMeleeDamage(Unit* caster, Unit* target, WeaponAttackType attackType, uint32 expectedMin, uint32 expectedMax, bool crit, TestCallback callback = DefaultCallback);
    void _TestDotDamage(Unit* caster, Unit* target, uint32 spellID, int32 expectedAmount, bool crit = false);
    void _TestThreat(Unit* caster, Creature* target, uint32 spellID, float expectedThreatFactor, bool heal = false, TestCallback callback = DefaultCallback);

    void _TestChannelDamage(bool healing, Unit* caster, Unit* target, uint32 spellID, uint32 tickCount, int32 expectedTickAmount, uint32 testedSpell = 0);
    /* if sampleSize != 0, check if results count = sampleSize
    expectedResult: 0 - 100
    allowedError: 0 - 100
    */
    void _TestMeleeOutcomePercentage(Unit* attacker, Unit* victim, WeaponAttackType weaponAttackType, MeleeHitOutcome meleeHitOutcome, float expectedResult, float allowedError, uint32 sampleSize = 0);
    /* if sampleSize != 0, check if results count = sampleSize
    expectedResult: 0 - 100
    allowedError: 0 - 100
    */
    void _TestSpellOutcomePercentage(Unit* caster, Unit* victim, uint32 spellId, SpellMissInfo hitInfo, float expectedResult, float allowedError, uint32 sampleSize = 0);
    /* if sampleSize != 0, check if results count = sampleSize
    expectedResult: 0 - 100
    allowedError: 0 - 100
    */
    void _TestSpellCritPercentage(Unit* caster, Unit* victim, uint32 spellId, float expectedResult, float allowedError, uint32 sampleSize = 0);
    void _TestSpellHitChance(Unit* caster, Unit* victim, uint32 spellID, float chance, SpellMissInfo missInfo, TestCallback callback = DefaultCallback);
    void _TestMeleeHitChance(Unit* caster, Unit* victim, WeaponAttackType weaponAttackType, float chance, MeleeHitOutcome meleeHitOutcome, TestCallback callback = DefaultCallback);
    void _TestSpellCritChance(Unit* caster, Unit* victim, uint32 spellID, float chance, TestCallback callback = DefaultCallback);
    void _TestSpellCastTime(Unit* caster, uint32 spellID, uint32 expectedCastTimeMS);
    void _TestAuraTickProcChance(Unit* caster, Unit* target, uint32 spellID, SpellEffIndex index, float chance, uint32 procSpellId, bool checkSelf);
    void _TestAuraTickProcChanceCallback(Unit* caster, Unit* target, uint32 spellID, SpellEffIndex index, float chance, uint32 procSpellId, TestCallbackResult callback);
    void _TestSpellProcChance(Unit* caster, Unit* target, uint32 spellID, uint32 procSpellID, bool selfProc, float expectedChancePercent, SpellMissInfo missInfo, bool crit, TestCallback callback = DefaultCallback);
    void _TestMeleeProcChance(Unit* attacker, Unit* target, uint32 procSpellID, bool selfProc, float expectedChancePercent, MeleeHitOutcome meleeHitOutcome, WeaponAttackType attackType, TestCallback callback = DefaultCallback);
    //return actual proc chance
    std::pair<float /*procChance*/, float /*absoluteTolerance*/> _TestProcChance(Unit* attacker, Unit* target, uint32 procSpellID, bool selfProc, float expectedChancePercent, TestCallback launchCallback, TestCallback callback = DefaultCallback);
    void _TestPushBackResistChance(Unit* caster, Unit* target, uint32 spellID, float chance);
    void _TestSpellDispelResist(Unit* caster, Unit* target, Unit* dispeler, uint32 spellID, float chance, TestCallback callback = DefaultCallback);
	void _TestStacksCount(TestPlayer* caster, Unit* target, uint32 castSpellID, uint32 testSpell, uint32 requireCount);
	void _TestPowerCost(TestPlayer* caster, uint32 castSpellID, Powers powerType, uint32 expectedPowerCost);
    inline void _TestCooldown(Unit* caster, Unit* target, uint32 castSpellID, Milliseconds ms) { _TestCooldown(caster, target, castSpellID, uint32(ms.count())); }
    void _TestCooldown(Unit* caster, Unit* target, uint32 castSpellID, uint32 cooldownMS);
    void _EquipItem(TestPlayer* p, uint32 itemID, bool newItem);
    //if spellID negative, ensure has NOT aura
    void _EnsureHasAura(Unit* target, int32 spellID);
    inline void _EnsureHasNotAura(Unit* target, uint32 spellID) { _EnsureHasAura(target, -int32(spellID)); }
    void _TestHasCooldown(Unit* caster, uint32 castSpellID, uint32 cooldownMs);
    inline void _TestHasCooldown(Unit* caster, uint32 castSpellID, Milliseconds ms) { _TestHasCooldown(caster, castSpellID, uint32(ms.count())); }
    void _TestAuraMaxDuration(Unit* target, uint32 spellID, uint32 durationMS);
    inline void _TestAuraMaxDuration(Unit* target, uint32 spellID, Milliseconds ms) { _TestAuraMaxDuration(target, spellID, uint32(ms.count())); }
    void _TestAuraStack(Unit* target, uint32 spellID,uint32 stacks, bool stack);
    void _TestCast(Unit* caster, Unit* victim, uint32 spellID, SpellCastResult expectedCode = SPELL_CAST_OK, CastSpellExtraArgs args = {});
    void _ForceCast(Unit* caster, Unit* victim, uint32 spellID, SpellMissInfo forcedMissInfo = SPELL_MISS_NONE, CastSpellExtraArgs args = {});
    void _TestUseItem(TestPlayer* caster, Unit* target, uint32 itemId);
    void _TestRadius(TestPlayer* caster, Unit* castTarget, Unit* checkTarget, uint32 spellID, float radius, bool heal = false, uint32 checkSpellID = 0, bool includeCasterReach = true);
    void _TestRange(TestPlayer* caster, Unit* target, uint32 spellID, float maxRange, float minRange = 0.0f);
    // <Test macros related functions/>

    /* Gets a default error tolerance for given expected percentage
    expectedResult: % from 0 to 100

    Returns result from 0 to 100
    */
    float _GetPercentTestTolerance(float const expectedResultPercent);
    /* Return true while the test should continue to iterate to gather enough sample size
    expectedResult: % from 0 to 100
    resultsSoFar: Result so far from 0 to 100
    successesSoFar: How much successes have been done so far
    absoluteTolerance: % from 0 to 100s. How much error tolerance you're allowing. This is a maximum diff with expectedResult.
    */
    bool _ShouldIteratePercent(float const expectedResultPercent, float const successesSoFar, uint32 const iterationsDone, float const absoluteTolerance);

    // Returns how much iterations you should do and how much error you should allow for a given damage range (with a 99.9% certainty)
    std::pair<uint32 /*sampleSize*/, uint32 /*absoluteTolerance*/>  _GetApproximationParams(uint32 const expectedMin, uint32 const expectedMax);
    /* Returns how much iterations and how much tolerance you should allow for given:
    expectedResult: % from absoluteTolerance*2 to 100
    absoluteTolerance: % from 0 to 100. Error tolerance, maximum diff with expectedResult. If 0 given, will use a default value depending on expectedResult.
    */
    std::pair<uint32 /*sampleSize*/, float /*resultingAbsoluteTolerance*/> _GetPercentApproximationParams(float const expectedResultPercent, float absoluteTolerancePercent = 0.0f);

    // <Helpers>
    void _EnsureAlive(Unit* caster, Unit* victim);
    //Try to get caster AI or owner caster AI (if pet or summon). Fail if no caster AI found. Changes caster arg to owner if pet/summon.
    PlayerbotTestingAI* _GetCasterAI(Unit*& caster, bool failOnNotFound = true);
    //Same but with an already provided TestPlayer (no checking summoner or owner)
    PlayerbotTestingAI* _GetCasterAI(TestPlayer* caster, bool failOnNotFound = true);
    //Get SpellInfo, fails test if not found
    SpellInfo const* _GetSpellInfo(uint32 spellID);
    //See comments inside
    void _StartUnitChannels(Unit* unit);
    //Set unit hp and max hp to high values. lowHealth = hp to 1 and high max hp. Will also call _SaveUnitState
    void _MaxHealth(Unit* unit, bool lowHealth = false);
    //Save health, max health, power, max power for later usage
    void _SaveUnitState(Unit* unit);
    //Restore values saved by _SaveUnitState
    void _RestoreUnitState(Unit* unit);
    float GetArmorFactor(Player const* attacker, Unit const* target);
    /* Calc min max damage with attacker weapon on target given attacker weapon damage, weapon class, ap and target armor
    @spellBonusDmg How much bonus damage in the spell tooltip, if any
    @spellNormalizedWeaponSpeed See http://wowwiki.wikia.com/wiki/Normalization . May simply be the weapon speed for non normalized attacks.
    */
    std::pair<uint32 /*min*/, uint32 /*max*/> CalcMeleeDamage(Player const* attacker, Unit const* target, WeaponAttackType attackType = BASE_ATTACK, int32 spellBonusDmg = 0, float spellNormalizedWeaponSpeed = 0.0f);
    //generic spell id and name printing for errors
    std::string _SpellString(uint32 spellID);
    // <Helpers/>

private:

    struct InSection
    {
        InSection(std::string title, TestStatus status) : title(title), status(status) {}
        std::string title;
        TestStatus status;
    };

    std::string                  _testName;
    std::string                  _usedPattern;
    Optional<InSection>          _inSection;
    std::list<TestSectionResult> _results;
    std::atomic<bool>            _setup;
    bool                         _enableMapObjects;
    std::string                  _callerFile; //used for error output
    int32                        _callerLine; //used for error output
    std::string                  _internalAssertInfo;
    std::string                  _assertInfo;
    TestStatus                   _testStatus; //for compat, remove later

    bool _InternalSetup();
    void _Cleanup();
    void _Test(); //main test to call by TestThread. Will call Test() implemented in tests.
    void _Fail(const char* err, ...) ATTR_PRINTF(2, 3);
    void _FailNoException(std::string);
    void _SetName(std::string name) { _testName = name; }
    void _SetThread(std::shared_ptr<TestThread> testThread) { _testThread = testThread; }
    //if callerFile and callerLine are specified, also print them in message
    void _Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition, std::string callerFile = "", int32 callerLine = 0);
    void _InternalAssertInfo(const char* err, ...) ATTR_PRINTF(2, 3);
    void _ResetInternalAssertInfo();

      std::string _GetCallerFile();
    int32 _GetCallerLine();

    /* Return a new randomized test bot. Returned player must be deleted by the caller
    if level == 0, set bot at max player level */
    TestPlayer* _CreateTestBot(Position loc, Classes cls, Races race, uint32 level = 0);
    void _GetRandomClassAndRace(Classes& cls, Races& race, bool forcePower = false, Powers power = POWER_MANA);
    Classes _GetRandomClassForRace(Races race);
    Races _GetRandomRaceForClass(Classes race);
    static void _RemoveTestBot(Player* player);
   
    std::weak_ptr<TestThread> _testThread;

    //those two just to help avoiding calling SpawnRandomPlayer with the wrong arguments, SpawnPlayer should be called in those case
    TestPlayer* SpawnRandomPlayer(Races race, Classes cls) = delete;
    TestPlayer* SpawnRandomPlayer(Classes cls, Races races) = delete;

    //return aura amount of effect 0
    int32 _CastDotAndWait(Unit* caster, Unit* target, uint32 spellID, bool crit = false);

    void ResetSpellCast(Unit* caster);

    struct SavedUnitState
    {
        SavedUnitState(uint32 health, uint32 maxHealth, Powers powerType, uint32 power, uint32 maxPower) :
            health(health),
            maxHealth(maxHealth),
            powerType(powerType),
            power(power),
            maxPower(maxPower)
        { }

        uint32 health;
        uint32 maxHealth;
        Powers powerType;
        uint32 power;
        uint32 maxPower;
    };

    struct SavedCriticalValues
    {
        float spellCrit[MAX_SPELL_SCHOOL]; //PLAYER_SPELL_CRIT_PERCENTAGE1 + i
        float mainCrit; //PLAYER_CRIT_PERCENTAGE
        float offCrit; //PLAYER_OFFHAND_CRIT_PERCENTAGE
        float rangedCrit; //PLAYER_RANGED_CRIT_PERCENTAGE
        int32 baseCrit; //m_baseSpellCritChance
    };

    std::unordered_map<Unit*, SavedUnitState> _saveUnitStates;
    std::unordered_map<Unit*, SavedCriticalValues> _savedCriticalValues;

    inline static void EmptyCallback() {};
    std::function<void()> _beforeEach = EmptyCallback;
    std::function<void()> _afterEach = EmptyCallback;
};

// -- internal uses defines:
//same as TEST_ASSERT but will track caller file and line to print it in case of error
#define INTERNAL_TEST_ASSERT( expr ) { _Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr, _GetCallerFile(), _GetCallerLine()); _ResetInternalAssertInfo(); }

//input info for next check, place this before INTERNAL_TEST_ASSERT
#define INTERNAL_ASSERT_INFO(expr, ...) { _InternalAssertInfo(expr, ## __VA_ARGS__); }
// --

#endif //TESTCASE_H