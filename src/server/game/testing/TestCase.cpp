#include "TestCase.h"
#include "MapManager.h"
#include "TestThread.h"
#include "RandomPlayerbotMgr.h"
#include "TestPlayer.h"
#include "RandomPlayerbotFactory.h"
#include "PlayerbotFactory.h"
#include "CharacterCache.h"
#include "SpellHistory.h"

#include <boost/math/special_functions/erf.hpp>

#define MAP_TESTING_ID 13

//same as TEST_ASSERT but will track caller file and line to print it in case of error
#define INTERNAL_TEST_ASSERT( expr ) _Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr, true, _GetCallerFile(), _GetCallerLine()); _ResetInternalAssertInfo();
//same as last but does not increase test count
#define INTERNAL_TEST_ASSERT_NOCOUNT( expr ) _Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr, false, _GetCallerFile(), _GetCallerLine()); _ResetInternalAssertInfo();

//input info for next check, place this before INTERNAL_TEST_ASSERT
#define INTERNAL_ASSERT_INFO(expr, ...) _InternalAssertInfo(expr, ## __VA_ARGS__);

TestCase::TestCase(TestStatus status, bool needMap) :
    _failed(false),
    _testsCount(0),

    _testMapInstanceId(0),
    _diff(REGULAR_DIFFICULTY),
    _map(nullptr),
    _enableMapObjects(false),
    _setup(false),
    _testStatus(status)
{
    //default pos:
    if (needMap && _location.GetMapId() == MAPID_INVALID) //not yet defined by the other constructor
        _location = std::move(WorldLocation(MAP_TESTING_ID, TestCase::GetDefaultPositionForMap(MAP_TESTING_ID)));
}

TestCase::TestCase(TestStatus status, WorldLocation const& specificPosition)
    : TestCase(status, true)
{
    bool hasDefaultPos = specificPosition.GetPositionX() == 0.0f && specificPosition.GetPositionY() == 0.0f && specificPosition.GetPositionZ() == 0.0f;
    _location = specificPosition;
    if (hasDefaultPos)
        _location.Relocate(TestCase::GetDefaultPositionForMap(specificPosition.GetMapId()));
}

Position TestCase::GetDefaultPositionForMap(uint32 mapId)
{
    Position pos;
    switch(mapId)
    {
    case 0:  //Eastern Kingdoms
        pos = Position(-4674.0f, -1640.0f, 504.0f);
    case 1:  //Kalimdor
        pos = Position(2946.0f, -4791.0f, 236.0f);
    case 13: //Test
        pos = Position(-223.97f, 0.23f, -423.2f);
        break;
    case 209: //Zul'Farrak
        pos = Position(1228.0f, 827.0f, 9.0f);
        break;
    case 530: //Outland
        pos = Position(-1520.0f, 8256.0f, -15.0f);
    default:
        auto areaTrigger = sObjectMgr->GetMapEntranceTrigger(mapId);
        if(areaTrigger)
            pos = Position(areaTrigger->target_X, areaTrigger->target_Y, areaTrigger->target_Z, areaTrigger->target_Orientation);
        else
            pos = Position(0.0f, 0.0f, 0.0f);
        break;
    }
    return pos;
}

void TestCase::_Fail(const char* err, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, err);
    vsnprintf(buffer, 256, err, args);
    va_end(args);

    _FailNoException(buffer);
    throw TestException();
}

void TestCase::_FailNoException(std::string msg)
{
    _failed = true;
    _errMsg = msg;
    if (!_assertInfo.empty())
        _errMsg = _errMsg + '\n' + _assertInfo;
    if (!_internalAssertInfo.empty())
        _errMsg = _errMsg + '\n' + _internalAssertInfo;
}

void TestCase::_AssertInfo(const char* err, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, err);
    vsnprintf(buffer, 256, err, args);
    va_end(args);

    _assertInfo = buffer;
}

void TestCase::_InternalAssertInfo(const char* err, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, err);
    vsnprintf(buffer, 256, err, args);
    va_end(args);

    _internalAssertInfo = buffer;
}

void TestCase::_ResetAssertInfo()
{
    _assertInfo = {};
}

void TestCase::_ResetInternalAssertInfo()
{
    _internalAssertInfo = {};
}

void TestCase::Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition)
{
    _Assert(file, line, function, condition, failedCondition, true);
}

void TestCase::_Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition, bool increaseTestCount, std::string callerFile, int32 callerLine)
{
    if(increaseTestCount)
        _testsCount++;
    if (!condition)
    {
        if (callerFile.empty() || callerLine == 0)
            _Fail("'%s:%i in %s CHECK FAILED: %s'", file.c_str(), line, function.c_str(), failedCondition.c_str());
        else
            _Fail("'%s:%i in %s CHECK FAILED: %s'. (Internal: %s:%i)", callerFile.c_str(), callerLine, function.c_str(), failedCondition.c_str(), file.c_str(), line);
    }
}

void TestCase::Wait(Seconds seconds)
{
    Wait(uint32(seconds.count()*IN_MILLISECONDS));
}

void TestCase::Wait(Milliseconds ms)
{
    Wait(uint32(ms.count()));
}

void TestCase::Wait(uint32 ms) 
{ 
    if (ms == 0)
        return;

    bool ok = _testThread->Wait(ms);
    if (!ok)
        _Fail("Test was canceled (TestCase)");
}

void TestCase::_Cleanup()
{
    Cleanup(); //test defined additional cleanup

    if (_testMapInstanceId)
        sMapMgr->UnloadTestMap(_location.GetMapId(), _testMapInstanceId);
}

bool TestCase::_InternalSetup()
{
    ASSERT(!_map);
    if (_location.GetMapId() == MAPID_INVALID) // test has been started with needMap = false
    {
        _setup = true;
        return true;
    }

    _map = sMapMgr->CreateTestMap(_location.GetMapId(), _testMapInstanceId, _diff, _enableMapObjects);
    if (!_map)
        return false;

    _setup = true;
    return true;
}

// ################### Utility functions 

void TestCase::EnableMapObjects()
{
    _enableMapObjects = true;
}

void TestCase::_TestStacksCount(TestPlayer* caster, Unit* target, uint32 castSpellID, uint32 testSpell, uint32 requireCount)
{
	/* TODO: cast */
	uint32 auraCount = target->GetAuraCount(testSpell);
	INTERNAL_TEST_ASSERT(auraCount == requireCount);
}

void TestCase::_TestPowerCost(TestPlayer* caster, Unit* target, uint32 castSpellID, Powers powerType, uint32 expectedPowerCost)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(castSpellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", castSpellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);
	caster->SetPower(powerType, expectedPowerCost);
    INTERNAL_ASSERT_INFO("Caster has not the expected power %u but %u instead", expectedPowerCost, caster->GetPower(powerType));
	INTERNAL_TEST_ASSERT(caster->GetPower(powerType) == expectedPowerCost);
    caster->ForceSpellHitResult(SPELL_MISS_NONE);
    _TestCast(caster, target, castSpellID, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_POWER_AND_REAGENT_COST));
    caster->ResetForceSpellHitResult();
    //special case for channeled spell, spell system currently does not allow casting them instant
    if (spellInfo->IsChanneled())
    {
        caster->DisableRegeneration(true);
        Wait(spellInfo->CalcCastTime() + 1); //may not be exact if spell has modifiers :/
        Wait(1); //wait another update for some spells?
        caster->DisableRegeneration(false);
    }

    uint32 remainingPower = caster->GetPower(powerType);
	INTERNAL_ASSERT_INFO("Caster has %u power remaining after spell %u", remainingPower, castSpellID);
	INTERNAL_TEST_ASSERT(remainingPower == 0);
}

void TestCase::_TestCooldown(TestPlayer* caster, Unit* target, uint32 castSpellID, uint32 cooldownSecond)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(castSpellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", castSpellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);
    caster->ForceSpellHitResult(SPELL_MISS_NONE);
    caster->GetSpellHistory()->ResetCooldown(castSpellID);
    _TestCast(caster, target, castSpellID, SPELL_CAST_OK, TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD));
    caster->ResetForceSpellHitResult();
    //special case for channeled spell, spell system currently does not allow casting them instant
    if (spellInfo->IsChanneled())
    {
        Wait(spellInfo->CalcCastTime() + 1); //may not be exact if spell has modifiers :/
        Wait(1); //wait another update for some spells?
    }

    //all setup, proceed to test CD
    _TestHasCooldown(caster, castSpellID, cooldownSecond);
    caster->GetSpellHistory()->ResetCooldown(castSpellID);
}

TestPlayer* TestCase::SpawnRandomPlayer()
{
    Classes cls = CLASS_NONE;
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race);
    TestPlayer* playerBot = _CreateTestBot(_location, cls, race);
    INTERNAL_ASSERT_INFO("Creating random test bot with class %u and race %u", uint32(cls), uint32(race));
    INTERNAL_TEST_ASSERT_NOCOUNT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Powers power, uint32 level)
{
    Classes cls = CLASS_NONE;
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race, true, power);

    TestPlayer* playerBot = _CreateTestBot(_location, cls, race, level);
    INTERNAL_ASSERT_INFO("Creating random test with power %u and level %u", uint32(power), level);
    INTERNAL_TEST_ASSERT_NOCOUNT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Races race, uint32 level)
{
    TestPlayer* playerBot = _CreateTestBot(_location, _GetRandomClassForRace(race), race, level);
    INTERNAL_ASSERT_INFO("Creating random test bot with race %u and level %u", uint32(race), level);
    INTERNAL_TEST_ASSERT_NOCOUNT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Classes cls, uint32 level)
{
    TestPlayer* playerBot = _CreateTestBot(_location, cls, _GetRandomRaceForClass(cls), level);
    INTERNAL_ASSERT_INFO("Creating random test bot with class %u and level %u", uint32(cls), level);
    INTERNAL_TEST_ASSERT_NOCOUNT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnPlayer(Classes _class, Races _race, uint32 level, Position spawnPosition)
{
    Position targetLocation = _location;
    if (spawnPosition.GetPositionX() != 0.0f && spawnPosition.GetPositionY() != 0.0f && spawnPosition.GetPositionZ() != 0.0f)
        targetLocation.Relocate(spawnPosition);
    TestPlayer* playerBot = _CreateTestBot(targetLocation, _class, _race, level);
    INTERNAL_ASSERT_INFO("Creating random test bot with class %u, race %u and level %u", uint32(_class), uint32(_race), level);
    INTERNAL_TEST_ASSERT_NOCOUNT(playerBot != nullptr);
    return playerBot;
}

#define TEST_ACCOUNT_NAME "autotestaccount"

uint32 TestCase::GetTestBotAccountId()
{
    const std::string testAccountName = TEST_ACCOUNT_NAME;
    uint32 id = sAccountMgr->GetId(testAccountName);
    if (id != 0)
        return id;

    auto result = sAccountMgr->CreateAccount(testAccountName, testAccountName, "playertestbot");
    if (result != AOR_OK)
        return false;

    return sAccountMgr->GetId(testAccountName);
}

void TestCase::RandomizePlayer(TestPlayer* player)
{
    //main reason it's slow is the random equipment in randomize
    PlayerbotFactory factory(player, player->GetLevel());
    factory.Randomize();
}

void TestCase::_RemoveTestBot(Player* player)
{
    WorldSession* botWorldSessionPtr = player->GetSession();
    botWorldSessionPtr->LogoutPlayer(true); // this will delete the bot Player object and PlayerbotAI object
    delete botWorldSessionPtr;  // finally delete the bot's WorldSession
}

//create a player of random level with no equipement, no talents, max skills for his class
TestPlayer* TestCase::_CreateTestBot(Position loc, Classes cls, Races race, uint32 level)
{
    INTERNAL_TEST_ASSERT_NOCOUNT(cls != CLASS_NONE && race != RACE_NONE);
    TC_LOG_TRACE("test.unit_test", "Creating new random bot for class %d", cls);

    if (!_map)
        return nullptr;

    std::string name = RandomPlayerbotFactory::CreateTestBotName();  //note that by doing this test bots name may sometime overlap with other connected bots name... BUT WELL WHATEVER.
    if (name.empty())
        return nullptr;
   
    uint32 testAccountId = TestCase::GetTestBotAccountId();
    WorldSession* session = new WorldSession(testAccountId, BUILD_243, TEST_ACCOUNT_NAME, NULL, SEC_PLAYER, 1, 0, LOCALE_enUS, 0, false);
    if (!session)
    {
        TC_LOG_ERROR("test.unit_test", "Failed to create session for test bot");
        delete session;
        return nullptr;
    }

    TestPlayer* player = new TestPlayer(session);

    CharacterCreateInfo cci;
    cci.RandomizeAppearance();
    cci.Name = name;
    cci.Race = race;
    cci.Class = cls;

    if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), &cci))
    {
        delete session;
        delete player;
        TC_LOG_ERROR("test.unit_test", "Unable to create test bot for account %d - name: \"%s\"; race: %u; class: %u; gender: %u; skin: %u; face: %u; hairStyle: %u; hairColor: %u; facialHair: %u; outfitId: %u",
            testAccountId, name.c_str(), race, cls, cci.Gender, cci.Skin, cci.Face, cci.HairStyle, cci.HairColor, cci.FacialHair, cci.OutfitId);
        return nullptr;
    }

    TC_LOG_TRACE("test.unit_test", "Test bot created for account %d - name: \"%s\"; race: %u; class: %u; gender: %u; skin: %u; face: %u; hairStyle: %u; hairColor: %u; facialHair: %u; outfitId: %u",
        testAccountId, name.c_str(), race, cls, cci.Gender, cci.Skin, cci.Face, cci.HairStyle, cci.HairColor, cci.FacialHair, cci.OutfitId);

    PlayerbotAI* ai = new PlayerbotTestingAI(player);
    player->SetPlayerbotAI(ai);

    auto holder = new LoginQueryHolder(testAccountId, player->GetGUID());
    if (!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return nullptr;
    }

    if (level == 0)
        level = sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL);

    player->GiveLevel(level);
    player->LearnAllClassProficiencies();
    player->UpdateSkillsToMaxSkillsForLevel();

    //make sure player is alreary linked to this map before calling _HandlePlayerLogin, else a lot of stuff will be loaded around default player location
    player->ResetMap();
    player->SetMap(_map); 
    session->_HandlePlayerLogin((Player*)player, holder);

    player->SetTeleportingToTest(_testMapInstanceId);

    //handle bot position
    bool teleportOK = player->TeleportTo(WorldLocation(_location.GetMapId(), loc), TELE_TO_GM_MODE);
    if (!teleportOK)
        return nullptr;
    ai->HandleTeleportAck(); //immediately handle teleport packet

    sCharacterCache->AddCharacterCacheEntry(player->GetGUID().GetCounter(), testAccountId, player->GetName(), cci.Gender, cci.Race, cci.Class, level, 0);

    //usually done in LoadFromDB
    player->SetCanModifyStats(true);
    player->UpdateAllStats();

    return player;
}

void TestCase::_GetRandomClassAndRace(Classes& cls, Races& race, bool forcePower, Powers forcedPower)
{
    //prepare selectable classes. Class can be specified in input (cls != CLASS_NONE), limited by power, or free (full random in this case)
    std::vector<Classes> allSelectableClasses;
    if (cls != CLASS_NONE)
        allSelectableClasses = { cls };
    if (forcePower)
    {
        switch (forcedPower)
        {
        case POWER_RAGE:
            allSelectableClasses = {
                CLASS_WARRIOR,
            };
            break;
        case POWER_ENERGY:
            allSelectableClasses = {
                CLASS_ROGUE,
            };
            break;
        case POWER_MANA:
            allSelectableClasses = {
                CLASS_PALADIN,
                CLASS_HUNTER,
                CLASS_PRIEST,
                CLASS_SHAMAN,
                CLASS_MAGE,
                CLASS_WARLOCK,
                CLASS_DRUID,
            };
            break;
        default:
            INTERNAL_ASSERT_INFO("_GetRandomClassAndRace: invalid power forcedPower %u", forcedPower);
            INTERNAL_TEST_ASSERT(false);
        }
    }
    else
    {
        allSelectableClasses = {
            CLASS_WARRIOR,
            CLASS_PALADIN,
            CLASS_HUNTER,
            CLASS_ROGUE,
            CLASS_PRIEST,
#ifdef LICH_KING
            CLASS_DEATH_KNIGHT = 6,
#endif
            CLASS_SHAMAN,
            CLASS_MAGE,
            CLASS_WARLOCK,
            CLASS_DRUID,
        };
    }

    //case 1 - we want a random race for given race
    if (cls != CLASS_NONE && race == RACE_NONE)
    {
        std::map<uint8, std::vector<uint8>> availableRacesForClasses = RandomPlayerbotFactory::GetAvailableRacesForClasses();
        auto availableRacesForClass = availableRacesForClasses[uint8(cls)];
        INTERNAL_TEST_ASSERT_NOCOUNT(!availableRacesForClass.empty());
        race = Races(availableRacesForClass[urand(0, availableRacesForClass.size() - 1)]);
    }
    //case 2 - we want a random class for given race
    else if (race != RACE_NONE && cls == CLASS_NONE )
    { 
        std::map<uint8, std::vector<uint8>> availableRacesForClasses = RandomPlayerbotFactory::GetAvailableRacesForClasses();
        //List all available classes for given race
        std::vector<Classes> availableClassesForRace;
        for (auto itr = availableRacesForClasses.begin(); itr != availableRacesForClasses.end(); itr++)
        {
            Classes _class = Classes(itr->first);
            //exclude this class if it was ruled out by force power 
            if (std::find(allSelectableClasses.begin(), allSelectableClasses.end(), _class) == allSelectableClasses.end())
                continue;

            auto allAvailableRacesForClass = itr->second;
            for (uint32 j = 0; j < allAvailableRacesForClass.size(); j++)
            {
                if (Races(allAvailableRacesForClass[j]) == race)
                {
                    availableClassesForRace.push_back(_class);
                    break;
                }
            }
        }
        INTERNAL_TEST_ASSERT_NOCOUNT(!availableClassesForRace.empty());
        //random on on resulting available classes
        cls = Classes(availableClassesForRace[urand(0, availableClassesForRace.size() - 1)]);
    }
    //case 3 - we want a random class and race
    else  if (race == RACE_NONE && cls == CLASS_NONE)
    {
        cls = allSelectableClasses[urand(0, allSelectableClasses.size() - 1)];
        TestCase::_GetRandomClassAndRace(cls, race);
    }
    else 
    {
        //if we reach here, both race and class were specified, so no use using this randomize function
        INTERNAL_TEST_ASSERT_NOCOUNT(false);
    }
}

Classes TestCase::_GetRandomClassForRace(Races race)
{
    Classes cls = CLASS_NONE;
    _GetRandomClassAndRace(cls, race);
    return cls;
}

Races TestCase::_GetRandomRaceForClass(Classes cls)
{
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race);
    return race;
}

TempSummon* TestCase::SpawnCreature(uint32 entry, bool spawnInFront)
{
    Position spawnPosition(_location);
    if(spawnInFront)
        spawnPosition.MoveInFront(_location, 3.0f);

    return SpawnCreatureWithPosition(spawnPosition, entry);
}

TempSummon* TestCase::SpawnCreatureWithPosition(Position spawnPosition, uint32 entry)
{
    INTERNAL_ASSERT_INFO("Test has no map");
    INTERNAL_TEST_ASSERT_NOCOUNT(GetMap() != nullptr);
    uint32 creatureEntry = entry ? entry : TEST_CREATURE_ENTRY;

    TempSummon* summon = GetMap()->SummonCreature(creatureEntry, spawnPosition);
    INTERNAL_ASSERT_INFO("Failed to summon creature with entry %u", creatureEntry);
    INTERNAL_TEST_ASSERT_NOCOUNT(summon != nullptr);
    summon->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN); //Make sur it does not despawn
    return summon;
}

void TestCase::RemoveAllEquipedItems(TestPlayer* player)
{
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        if (player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
}

void TestCase::RemoveItem(TestPlayer* player, uint32 itemID, uint32 count)
{
    player->DestroyItemCount(itemID, count, true, false);
}

void TestCase::_EquipItem(TestPlayer* player, uint32 itemID)
{
    Item* item = player->AddItem(itemID, 1);
    INTERNAL_ASSERT_INFO("Failed to add item %u to player", itemID);
    INTERNAL_TEST_ASSERT_NOCOUNT(item != nullptr);

    uint16 dest;
    uint8 msg2 = player->CanEquipItem(NULL_SLOT, dest, item, !item->IsBag());
    INTERNAL_ASSERT_INFO("Player cannot equip item %u, reason: %u", itemID, msg2);
    INTERNAL_TEST_ASSERT_NOCOUNT(msg2 == EQUIP_ERR_OK);

    player->GetSession()->_HandleAutoEquipItemOpcode(item->GetBagSlot(), item->GetSlot());

    Item* equipedItem = player->GetItemByPos(dest);
    INTERNAL_ASSERT_INFO("Player failed to equip item %u (dest: %u)", itemID, dest);
    INTERNAL_TEST_ASSERT_NOCOUNT(equipedItem != nullptr);
    Wait(1); //not sure this is needed but... let's just wait next update to make sure item spells are properly applied
}

void TestCase::LearnTalent(TestPlayer* p, uint32 spellID)
{
    if (p->HasSpell(spellID))
        return;

    p->LearnSpell(spellID, false);
}

void TestCase::_TestCast(Unit* caster, Unit* victim, uint32 spellID, SpellCastResult expectedCode, TriggerCastFlags triggeredFlags)
{
    uint32 res = caster->CastSpell(victim, spellID, triggeredFlags);
    if (expectedCode == SPELL_CAST_OK)
    {
        INTERNAL_ASSERT_INFO("Caster couldn't cast %u, error %s", spellID, StringifySpellCastResult(res).c_str());
    }
    else
    {
        INTERNAL_ASSERT_INFO("Caster cast %u returned unexpected result %s", spellID, StringifySpellCastResult(res).c_str());
    }

	INTERNAL_TEST_ASSERT(res == uint32(expectedCode));
}

void TestCase::_ForceCast(Unit* caster, Unit* victim, uint32 spellID, SpellMissInfo forcedMissInfo, TriggerCastFlags triggeredFlags)
{
    caster->ForceSpellHitResult(forcedMissInfo);
    uint32 res = caster->CastSpell(victim, spellID, triggeredFlags);
    caster->ResetForceSpellHitResult();
    INTERNAL_ASSERT_INFO("Caster couldn't cast %u, error %s", spellID, StringifySpellCastResult(res).c_str());
    INTERNAL_TEST_ASSERT(res == uint32(SPELL_CAST_OK));
}

bool TestCase::HasLootForMe(Creature* creature, Player* player, uint32 itemID)
{
    auto items = creature->loot.items;
    auto quest_items = creature->loot.quest_items;
    for (auto itr : items)
        if (itr.itemid == itemID)
            return true;

    for (auto itr : quest_items)
        if (itr.itemid == itemID)
            return true;

    return false;
}

float myErfInv2(float x) {
    float tt1, tt2, lnx, sgn;
    sgn = (x < 0) ? -1.0f : 1.0f;

    x = (1 - x)*(1 + x);        // x = 1 - x*x;
    lnx = logf(x);

    tt1 = 2 / (M_PI*0.147) + 0.5f * lnx;
    tt2 = 1 / (0.147) * lnx;

    return(sgn*sqrtf(-tt1 + sqrtf(tt1*tt1 - tt2)));
}

void TestCase::_GetPercentApproximationParams(uint32& sampleSize, float& resultingAbsoluteTolerance, float const expectedResult, float const absoluteTolerance)
{
    INTERNAL_ASSERT_INFO("Invalid input tolerance %f", absoluteTolerance);
    INTERNAL_TEST_ASSERT(Between(absoluteTolerance, 0.001f, 0.1f));

    INTERNAL_ASSERT_INFO("Expected result %f", expectedResult);
    INTERNAL_TEST_ASSERT(Between(expectedResult, absoluteTolerance * 2, 1.0f));

    float const certainty = 0.999f;

    resultingAbsoluteTolerance = absoluteTolerance / 2;
    sampleSize = 2 * pow(boost::math::erf_inv(certainty) / absoluteTolerance, 2);
    sampleSize = std::max(sampleSize, uint32(100)); //min sample size
}

void TestCase::_GetApproximationParams(uint32& sampleSize, uint32& absoluteAllowedError, uint32 const expectedMin, uint32 const expectedMax)
{
    double certainty = 0.999;
    absoluteAllowedError = (expectedMax - expectedMin) / 25; //arbitary
    absoluteAllowedError = std::max(absoluteAllowedError, uint32(1)); //min 1

    uint32 LB_max = expectedMax - absoluteAllowedError; // lower bound on the maximum
    uint32 UB_max = expectedMax + absoluteAllowedError; // upper bound on the maximum

    uint32 LB_min = std::max(expectedMin, absoluteAllowedError) - absoluteAllowedError; // lower bound on the minimum, and try to avoid negative numbers
    uint32 UB_min = expectedMin + absoluteAllowedError; // upper bound on the minimum

    // The probability of having the estimation of the maximum between LB_max and UB_max is
    // 1 - [(y-LB_min)/(UB_max-LB_min)]^n
    // where y is the estimation, and n the number of samples. In our case, y = LB_max.
    //
    // If we want x% of probability of being correct, then
    // x = 1 - [(y-LB_min)/(UB_max-LB_min)]^n
    //
    // So, n should follow
    // n = log(1-x) / log( (y-LB_min)/(UB_max-LB_min) )
    //
    // For the minimum, the same reasoning applies by multiplying all values by -1. We end with the formula
    // n = log(1-x) / log( (y-UB_max)/(UB_max-LB_min) ) with y = UB_min.
    //
    // Reference: 
    // (probability distribution of maximum of uniform distribution) 
    // https://stats.stackexchange.com/questions/18433/how-do-you-calculate-the-probability-density-function-of-the-maximum-of-a-sample

    double ratio_min = (double)(UB_max - UB_min) / (double)(UB_max - LB_min);
    double ratio_max = (double)(LB_max - LB_min) / (double)(UB_max - LB_min);

    uint32 sampleSize_min = std::ceil(std::log(1.0 - certainty) / std::log(ratio_min));
    uint32 sampleSize_max = std::ceil(std::log(1.0 - certainty) / std::log(ratio_max));

    // Set sampleSize minimum to 10
    sampleSize = std::max(uint32(10), std::max(sampleSize_min, sampleSize_max));
}

void ResetSpellCast(Unit* caster)
{
    caster->InterruptNonMeleeSpells(true);
    caster->m_Events.KillAllEvents(true); //also kill spells in flight
}

void TestCase::_TestDirectValue(Unit* caster, Unit* target, uint32 spellID, uint32 expectedMin, uint32 expectedMax, bool crit, bool damage, Optional<TestCallback> callback) //if !damage, then use healing
{
    INTERNAL_TEST_ASSERT(expectedMax >= expectedMin);
    Player* _casterOwner = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
    TestPlayer* casterOwner = dynamic_cast<TestPlayer*>(_casterOwner);
    INTERNAL_ASSERT_INFO("Caster in not a testing bot (or a pet/summon of testing bot)");
    INTERNAL_TEST_ASSERT(casterOwner != nullptr);
    auto AI = caster->ToPlayer()->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    uint32 sampleSize;
    uint32 maxPredictionError;
    _GetApproximationParams(sampleSize, maxPredictionError, expectedMin, expectedMax);

	EnableCriticals(caster, crit);

    for (uint32 i = 0; i < sampleSize; i++)
    {
        if (callback)
            callback.get()(caster, target);

        caster->ForceSpellHitResult(SPELL_MISS_NONE);
        uint32 result = caster->CastSpell(target, spellID, TRIGGERED_FULL_DEBUG_MASK);
        caster->ResetForceSpellHitResult();
        INTERNAL_ASSERT_INFO("Spell casting failed with reason %s", StringifySpellCastResult(result).c_str());
        INTERNAL_TEST_ASSERT(result == SPELL_CAST_OK);
    }

    Wait(5 * SECOND * IN_MILLISECONDS);
    uint32 dealtMin;
    uint32 dealtMax;
    if(damage)
        GetDamagePerSpellsTo( casterOwner, target, spellID, dealtMin, dealtMax, crit, sampleSize);
    else 
        GetHealingPerSpellsTo(casterOwner, target, spellID, dealtMin, dealtMax, crit, sampleSize);

    TC_LOG_TRACE("test.unit_test", "spellId: %u -> dealtMin: %u - dealtMax %u - expectedMin: %u - expectedMax: %u - sampleSize: %u", spellID, dealtMin, dealtMax, expectedMin, expectedMax, sampleSize);

    uint32 allowedMin = expectedMin > maxPredictionError ? expectedMin - maxPredictionError : 0; //protect against underflow
    uint32 allowedMax = expectedMax + maxPredictionError;

    INTERNAL_ASSERT_INFO("Enforcing high result for spell %u. allowedMax: %u, dealtMax: %u", spellID, allowedMax, dealtMax);
    INTERNAL_TEST_ASSERT(dealtMax <= allowedMax);
    INTERNAL_ASSERT_INFO("Enforcing low result for spell %u. allowedMin: %u, dealtMin: %u", spellID, allowedMin, dealtMin);
    INTERNAL_TEST_ASSERT(dealtMin >= allowedMin);
}

void TestCase::_TestMeleeDamage(Unit* caster, Unit* target, WeaponAttackType attackType, uint32 expectedMin, uint32 expectedMax, bool crit)
{
    Player* _casterOwner = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
    INTERNAL_ASSERT_INFO("Caster is not a player or a pet/summon of him");
    INTERNAL_TEST_ASSERT(_casterOwner != nullptr);
    TestPlayer* casterOwner = dynamic_cast<TestPlayer*>(_casterOwner);
    INTERNAL_ASSERT_INFO("Caster in not a testing bot (or a pet/summon of testing bot)");
    INTERNAL_TEST_ASSERT(casterOwner != nullptr);

    auto AI = casterOwner->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    uint32 sampleSize;
    uint32 maxPredictionError;
    _GetApproximationParams(sampleSize, maxPredictionError, expectedMin, expectedMax);

    caster->ForceMeleeHitResult(crit ? MELEE_HIT_CRIT : MELEE_HIT_NORMAL);
    for (uint32 i = 0; i < sampleSize; i++)
    if (attackType != RANGED_ATTACK)
        caster->AttackerStateUpdate(target, attackType);
    else
        caster->CastSpell(target, 75, true); //shoot
    caster->ResetForceMeleeHitResult();

    Wait(3 * SECOND * IN_MILLISECONDS);
    uint32 dealtMin;
    uint32 dealtMax;
    GetWhiteDamageDoneTo(casterOwner, target, attackType, crit, dealtMin, dealtMax, sampleSize);

    //TC_LOG_DEBUG("test.unit_test", "attackType: %u - crit %u -> dealtMin: %u - dealtMax %u - expectedMin: %u - expectedMax: %u - sampleSize: %u", uint32(attackType), uint32(crit), dealtMin, dealtMax, expectedMin, expectedMax, sampleSize);

    uint32 allowedMin = expectedMin > maxPredictionError ? expectedMin - maxPredictionError : 0; //protect against underflow
    uint32 allowedMax = expectedMax + maxPredictionError;

    INTERNAL_ASSERT_INFO("Enforcing high result for attackType: %u - crit %u. allowedMax: %u, dealtMax: %u", uint32(attackType), uint32(crit), allowedMax, dealtMax);
    INTERNAL_TEST_ASSERT(dealtMax <= allowedMax);
    INTERNAL_ASSERT_INFO("Enforcing low result for attackType: %u - crit %u. allowedMin: %u, dealtMin: %u", uint32(attackType), uint32(crit), allowedMin, dealtMin);
    INTERNAL_TEST_ASSERT(dealtMin >= allowedMin);

}

float TestCase::GetChannelDamageTo(TestPlayer* caster, Unit* victim, uint32 spellID, uint32 tickCount, bool& mustRetry)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("GetChannelDamageTo was prompted for non existing spell ID %u", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);

    auto damageToTarget = AI->GetSpellDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("GetChannelDamageTo found no data for this victim (%s)", victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());

    std::vector<PlayerbotTestingAI::SpellDamageDoneInfo> filteredDamageToTarget;
    //Copy only relevent entries
    for (auto itr : *damageToTarget)
        if (itr.spellID == spellID)
            filteredDamageToTarget.push_back(itr);

    if (filteredDamageToTarget.size() != tickCount)
    {
        INTERNAL_ASSERT_INFO("Victim did not received expected tick count %u but received %u instead", tickCount, uint32(filteredDamageToTarget.size()));
        INTERNAL_TEST_ASSERT(false);
    }

    uint64 totalDamage = 0;
    uint32 count = 0;
    for (auto itr : filteredDamageToTarget)
    {
        //some spells were resisted or did crit... cannot use this data
        if (itr.missInfo != SPELL_MISS_NONE || itr.crit)
        {
            mustRetry = true;
            return 0.0f;
        }

        uint32 damage = itr.damageInfo.damage;
        damage += itr.damageInfo.resist;
        damage += itr.damageInfo.blocked;
        damage += itr.damageInfo.absorb;
        //resilience not taken into account...

        totalDamage += damage;
        count++;
    }

    INTERNAL_ASSERT_INFO("GetChannelDamageTo was prompted for a victim(%s) with no valid data for this spell(ID %u)", victim->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(count != 0);

    return totalDamage;
}

float TestCase::GetChannelHealingTo(TestPlayer* caster, Unit* victim, uint32 spellID, uint32 tickCount, bool& mustRetry)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("GetChannelHealingTo was prompted for non existing spell ID %u", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);

    auto healingToTarget = AI->GetHealingDoneInfo(victim);
    INTERNAL_ASSERT_INFO("GetChannelHealingTo found no data for this victim (%s)", victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(healingToTarget && !healingToTarget->empty());

    std::vector<PlayerbotTestingAI::HealingDoneInfo> filteredHealingToTarget;
    //Copy only relevent entries
    for (auto itr : *healingToTarget)
        if (itr.spellID == spellID)
            filteredHealingToTarget.push_back(itr);

    if (filteredHealingToTarget.size() != tickCount)
    {
        INTERNAL_ASSERT_INFO("Victim did not received expected tick count %u but received %u instead", tickCount, uint32(filteredHealingToTarget.size()));
        Wait(5000);
        INTERNAL_TEST_ASSERT(false);
    }

    uint64 totalHealing = 0;
    uint32 count = 0;
    for (auto itr : filteredHealingToTarget)
    {
        //some spells were resisted or did crit... cannot use this data
        if (itr.missInfo != SPELL_MISS_NONE || itr.crit)
        {
            mustRetry = true;
            return 0.0f;
        }

        totalHealing += itr.healing;
        count++;
    }

    INTERNAL_ASSERT_INFO("GetChannelHealingTo was prompted for a victim(%s) with no valid data for this spell(ID %u)", victim->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(count != 0);

    return totalHealing;
}

void TestCase::GetHealingPerSpellsTo(TestPlayer* caster, Unit* target, uint32 spellID, uint32& minHeal, uint32& maxHeal, Optional<bool> crit, uint32 expectedCount)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto healingToTarget = AI->GetHealingDoneInfo(target);
    INTERNAL_ASSERT_INFO("GetHealingPerSpellsTo found no data for this victim (%s)", target->GetName().c_str());
    INTERNAL_TEST_ASSERT(healingToTarget && !healingToTarget->empty());

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("GetHealingPerSpellsTo was prompted for non existing spell ID %u", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);

    minHeal = std::numeric_limits<uint32>::max();
    maxHeal = 0;

    uint32 count = 0;
    for (auto itr : *healingToTarget)
    {
        if (itr.spellID != spellID)
            continue;

        //use only spells that hit target
        if (itr.missInfo != SPELL_MISS_NONE)
            continue;

        if (crit && itr.crit != crit.get())
            continue;

        minHeal = std::min(minHeal, itr.healing);
        maxHeal = std::max(maxHeal, itr.healing);

        count++;
    }

    INTERNAL_ASSERT_INFO("GetHealingPerSpellsTo was prompted for a target (%s) with no valid data for this spell(ID %u)", target->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(count != 0);

    if (expectedCount)
    {
        INTERNAL_ASSERT_INFO("GetHealingPerSpellsTo did find data for spell %u and target %s, but not expected count (%u instead of %u)", spellID, target->GetName().c_str(), count, expectedCount);
        INTERNAL_TEST_ASSERT(count == expectedCount);
    }
}

void TestCase::GetWhiteDamageDoneTo(TestPlayer* caster, Unit* victim, WeaponAttackType attackType, bool critical, uint32& minDamage, uint32& maxDamage, uint32 expectedCount)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto damageToTarget = AI->GetMeleeDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("GetWhiteDamageDoneTo found no data for this victim (%s)", victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());

    minDamage = std::numeric_limits<uint32>::max();
    maxDamage = 0;

    //uint64 totalDamage = 0;
    uint32 count = 0;
    for (auto itr : *damageToTarget)
    {
        if (itr.damageInfo.AttackType != attackType)
            continue;

        //only use full hits
        if (critical)
        {
            if (itr.damageInfo.HitInfo != HITINFO_CRITICALHIT && itr.damageInfo.HitOutCome != MELEE_HIT_CRIT)
                continue;
        } else {
            if ((itr.damageInfo.HitInfo != HITINFO_NORMALSWING || itr.damageInfo.HitInfo != HITINFO_NORMALSWING2) && itr.damageInfo.HitOutCome != MELEE_HIT_NORMAL)
                continue;
        }
        uint32 damage = 0;
        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {

            damage += itr.damageInfo.Damages[i].Damage;
            damage += itr.damageInfo.Damages[i].Resist;
            damage += itr.damageInfo.Damages[i].Absorb;
        }
        damage += itr.damageInfo.Blocked;

        //resilience not taken into account here...

        minDamage = std::min(minDamage, damage);
        maxDamage = std::max(maxDamage, damage);

        //totalDamage += damage;
        count++;
    }

    INTERNAL_ASSERT_INFO("GetWhiteDamageDoneTo was prompted for a victim (%s) with no valid data for white damage (attackType %u)", victim->GetName().c_str(), attackType);
    INTERNAL_TEST_ASSERT(count != 0);

    if (expectedCount)
    {
        INTERNAL_ASSERT_INFO("GetWhiteDamageDoneTo did find data for target %s, but not expected count (%u instead of %u)", victim->GetName().c_str(), count, expectedCount);
        INTERNAL_TEST_ASSERT(count == expectedCount);
    }
}

void TestCase::GetDamagePerSpellsTo(TestPlayer* caster, Unit* victim, uint32 spellID, uint32& minDamage, uint32& maxDamage, Optional<bool> crit, uint32 expectedCount)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto damageToTarget = AI->GetSpellDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("GetDamagePerSpellsTo found no data for this victim (%s)", victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("GetDamagePerSpellsTo was prompted for non existing spell ID %u", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);

    minDamage = std::numeric_limits<uint32>::max();
    maxDamage = 0;

    //uint64 totalDamage = 0;
    uint32 count = 0;
    for (auto itr : *damageToTarget)
    {
        if (itr.spellID != spellID)
            continue;

        //use only spells that hit target
        if (itr.missInfo != SPELL_MISS_NONE)
            continue;

        if (crit && itr.crit != crit.get())
            continue;

        uint32 damage = itr.damageInfo.damage;
        damage += itr.damageInfo.resist;
        damage += itr.damageInfo.blocked;
        damage += itr.damageInfo.absorb;
        //resilience not taken into account...

        minDamage = std::min(minDamage, damage);
        maxDamage = std::max(maxDamage, damage);

        //totalDamage += damage;
        count++;
    }

    INTERNAL_ASSERT_INFO("GetDamagePerSpellsTo was prompted for a victim(%s) with no valid data for this spell(ID %u)", victim->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(count != 0);

    if (expectedCount)
    {
        INTERNAL_ASSERT_INFO("GetDamagePerSpellsTo did find data for spell %u and target %s, but not expected count (%u instead of %u)", spellID, victim->GetName().c_str(), count, expectedCount);
        INTERNAL_TEST_ASSERT(count == expectedCount);
    }
}

void TestCase::_TestDotDamage(TestPlayer* caster, Unit* target, uint32 spellID, int32 expectedAmount, bool crit /* = false*/)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);
    bool spellHasFlyTime = spellInfo->Speed != 0.0f;

	EnableCriticals(caster, crit);

    ResetSpellCast(caster);
    AI->ResetSpellCounters();

    caster->ForceSpellHitResult(SPELL_MISS_NONE);
    uint32 result = caster->CastSpell(target, spellID, true);
    if (result != SPELL_CAST_OK)
    {
        caster->ResetForceSpellHitResult();
        INTERNAL_ASSERT_INFO("_TestDotDamage: Spell cast failed with result %s ", StringifySpellCastResult(result).c_str());
        INTERNAL_TEST_ASSERT(false);
    }
    if (spellHasFlyTime)
        Wait(5 * SECOND*IN_MILLISECONDS);

    Wait(1);
    Aura* aura = target->GetAura(spellID, caster->GetGUID());
    INTERNAL_ASSERT_INFO("Target has not %u aura with caster %u after spell successfully casted", spellID, caster->GetGUID().GetCounter());
    INTERNAL_TEST_ASSERT(aura != nullptr);

    //spell did hit, let's wait for dot duration
    uint32 waitTime = aura->GetDuration() + 1 * SECOND * IN_MILLISECONDS;
    Wait(waitTime);
    caster->ResetForceSpellHitResult();
    //aura may be deleted at this point, do not use anymore

    //make sure aura expired
    INTERNAL_ASSERT_INFO("Target still has %u aura after %u ms", spellID, waitTime);
    INTERNAL_TEST_ASSERT(!target->HasAura(spellID, caster->GetGUID()));

    int32 dotDamageToTarget = AI->GetDotDamage(target, spellID);
	TC_LOG_TRACE("test.unit_test", "spellId: %u -> dotDamageToTarget: %i - expectedAmount: %i", spellID, dotDamageToTarget, expectedAmount);
    INTERNAL_ASSERT_INFO("Enforcing dot damage. dotDamageToTarget: %i, expectedAmount: %i", dotDamageToTarget, expectedAmount);
    TEST_ASSERT(dotDamageToTarget >= (expectedAmount - 6) && dotDamageToTarget <= (expectedAmount + 6)); //dots have greater error since they got their damage divided in several ticks
}

void TestCase::_TestChannelDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 testedSpell, uint32 tickCount, int32 expectedTickAmount, bool healing /* = false*/)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);
    uint32 baseCastTime = spellInfo->CalcCastTime(nullptr);
    uint32 baseDurationTime = spellInfo->GetDuration();

    for (uint32 i = 0; i < 100; i++)
    {
        ResetSpellCast(caster);
        AI->ResetSpellCounters();
        caster->ForceSpellHitResult(SPELL_MISS_NONE);
        uint32 result = caster->CastSpell(target, spellID, true);
        if (result != SPELL_CAST_OK)
        {
            caster->ResetForceSpellHitResult();
            INTERNAL_ASSERT_INFO("_TestDotDamage: Spell cast failed with result %s ", StringifySpellCastResult(result).c_str());
            INTERNAL_TEST_ASSERT(false);
        }
        Wait(baseCastTime + baseDurationTime + 1000); //reason we do this is that currently we can't instantly cast a channeled spell with our spell system
        caster->ResetForceSpellHitResult();
        bool mustRetry = false;
        float totalChannelDmg = 0; 
        if(healing)
            totalChannelDmg = GetChannelHealingTo(caster, target, testedSpell, tickCount, mustRetry);
        else
            totalChannelDmg = GetChannelDamageTo(caster, target, testedSpell, tickCount, mustRetry);

        if (mustRetry)
            continue;
        INTERNAL_ASSERT_INFO("Check if totalChannelDmg (%f) is round", totalChannelDmg);
        INTERNAL_TEST_ASSERT(totalChannelDmg == std::floor(totalChannelDmg));
        uint32 resultTickAmount = totalChannelDmg / tickCount;
        INTERNAL_ASSERT_INFO("Enforcing channel damage. resultTickAmount: %i, expectedTickAmount: %i", resultTickAmount, expectedTickAmount);
        INTERNAL_TEST_ASSERT(resultTickAmount >= (expectedTickAmount - 2) && resultTickAmount <= (expectedTickAmount + 2)); //channels have greater error since they got their damage divided in several ticks
        return;
    }
    caster->ResetForceSpellHitResult();
    INTERNAL_ASSERT_INFO("Failed to cast spell (%u) 100 times", spellID);
    INTERNAL_TEST_ASSERT(false); //failed to cast the spell 100 times
}

void TestCase::EnableCriticals(Unit* caster, bool crit)
{
    float critChance = -100.0f;
	if (crit)
		critChance = 200.0f;

	for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
		caster->SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i, critChance);

	caster->SetFloatValue(PLAYER_CRIT_PERCENTAGE, critChance); // BASE_ATTACK
	caster->SetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE, critChance); // OFF_ATTACK
	caster->SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, critChance); // RANGED_ATTACK
}

void TestCase::GroupPlayer(TestPlayer* leader, Player* player)
{
    Group* group = leader->GetGroup();
    if (group != nullptr && group->IsFull())
        group->ConvertToRaid();

    //use WorldSession function to avoid duplicating logic
    leader->GetSession()->_HandleGroupInviteOpcode(player, player->GetName());

    WorldPacket acceptPacket(CMSG_GROUP_ACCEPT);
    player->GetSession()->HandleGroupAcceptOpcode(acceptPacket);

    if (group != nullptr && group->isRaidGroup())
    {
        INTERNAL_ASSERT_INFO("Failed to add %s(R%uC%u) to %s(R%uC%u)'s raid", player->GetName().c_str(), player->GetRace(), player->GetClass(), leader->GetName().c_str(), leader->GetRace(), leader->GetClass());
        INTERNAL_TEST_ASSERT(player->IsInSameRaidWith(leader));
    }
    else
    {
        INTERNAL_ASSERT_INFO("Failed to add %s(R%uC%u) to %s(R%uC%u)'s group", player->GetName().c_str(), player->GetRace(), player->GetClass(), leader->GetName().c_str(), leader->GetRace(), leader->GetClass());
        INTERNAL_TEST_ASSERT(player->IsInSameGroupWith(leader));
    }
}

void TestCase::_TestSpellHitChance(TestPlayer* caster, TestPlayer* victim, uint32 spellID, float expectedResultPercent, SpellMissInfo missInfo)
{
    INTERNAL_ASSERT_INFO("_TestSpellHitChance only support alive caster");
    INTERNAL_TEST_ASSERT(caster->IsAlive());
    INTERNAL_ASSERT_INFO("_TestSpellHitChance only support alive victim");
    INTERNAL_TEST_ASSERT(victim->IsAlive());

    uint32 startingHealth = victim->GetHealth();
    uint32 startingMaxHealth = victim->GetMaxHealth();

    victim->SetMaxHealth(std::numeric_limits<uint32>::max());

    float const absoluteTolerance = 0.02f;
    uint32 sampleSize;
    float resultingAbsoluteTolerance;
    _GetPercentApproximationParams(sampleSize, resultingAbsoluteTolerance, expectedResultPercent / 100.0f, absoluteTolerance);

    for (uint32 i = 0; i < sampleSize; i++)
    {
        victim->SetFullHealth();
        caster->CastSpell(victim, spellID, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_IGNORE_SPEED));
    }

    Wait(1); //wait an update before restoring health, some procs may have occured

    victim->SetMaxHealth(startingMaxHealth);
    victim->SetHealth(startingHealth);

    _TestSpellOutcomePercentage(caster, victim, spellID, missInfo, expectedResultPercent, resultingAbsoluteTolerance * 100, sampleSize);
}

void TestCase::_TestMeleeHitChance(TestPlayer* caster, TestPlayer* victim, WeaponAttackType weaponAttackType, float expectedResultPercent, MeleeHitOutcome meleeHitOutcome)
{
    INTERNAL_ASSERT_INFO("_TestMeleeHitChance only support alive caster");
    INTERNAL_TEST_ASSERT(caster->IsAlive());
    INTERNAL_ASSERT_INFO("_TestMeleeHitChance only support alive victim");
    INTERNAL_TEST_ASSERT(victim->IsAlive());
    INTERNAL_ASSERT_INFO("_TestMeleeHitChance can only be used with BASE_ATTACK and OFF_ATTACK");
    INTERNAL_TEST_ASSERT(weaponAttackType <= OFF_ATTACK);

    uint32 startingHealth = victim->GetHealth();
    uint32 startingMaxHealth = victim->GetMaxHealth();

    victim->SetMaxHealth(std::numeric_limits<uint32>::max());

    float const absoluteTolerance = 0.02f;
    uint32 sampleSize;
    float resultingAbsoluteTolerance;
    _GetPercentApproximationParams(sampleSize, resultingAbsoluteTolerance, expectedResultPercent / 100.0f, absoluteTolerance);

    for (uint32 i = 0; i < sampleSize; i++)
    {
        victim->SetFullHealth();
        caster->AttackerStateUpdate(victim, weaponAttackType);
    }

    Wait(1); //wait an update before restoring health, some procs may have occured

    victim->SetMaxHealth(startingMaxHealth);
    victim->SetHealth(startingHealth);

    _TestMeleeOutcomePercentage(caster, victim, weaponAttackType, meleeHitOutcome, expectedResultPercent, resultingAbsoluteTolerance * 100, sampleSize);
}

void TestCase::_TestMeleeOutcomePercentage(TestPlayer* attacker, Unit* victim, WeaponAttackType weaponAttackType, MeleeHitOutcome meleeHitOutcome, float expectedResult, float allowedError, uint32 sampleSize /*= 0*/)
{
    auto AI = attacker->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto damageToTarget = AI->GetMeleeDamageDoneInfo(victim);
    if (!damageToTarget || damageToTarget->empty())
    {
        TC_LOG_WARN("test.unit_test", "_TestMeleeOutcomePercentage found no data for this victim (%s)", victim->GetName().c_str());
        return;
    }

    uint32 success = 0;
    uint32 attacks = 0; //total attacks with correct type
    //uint32 total = 0;
    for (auto itr : *damageToTarget)
    {
      /*  total++;
        float mean = (success / float(total))* 100.f;
        if(total % 100)
            TC_LOG_ERROR("test.unit_test", "%u error %f \t\t mean %f \t\t %f", total, std::abs(mean - expectedResult), mean, expectedResult);*/

        if (itr.damageInfo.AttackType != weaponAttackType)
            continue;

        attacks++;

        if (itr.damageInfo.HitOutCome != meleeHitOutcome)
            continue;

        success++;
    }

    if (sampleSize)
    {
        INTERNAL_ASSERT_INFO("_TestMeleeOutcomePercentage found %u results instead of expected sample size %u", attacks, sampleSize);
        INTERNAL_TEST_ASSERT(attacks == sampleSize)
    }

    float const result = success / float(damageToTarget->size()) * 100;;
    INTERNAL_ASSERT_INFO("_TestMeleeOutcomePercentage: expected result: %f, result: %f", expectedResult, result);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResult, result - allowedError, result + allowedError));
}

void TestCase::_TestSpellOutcomePercentage(TestPlayer* caster, Unit* victim, uint32 spellId, SpellMissInfo missInfo, float expectedResult, float allowedError, uint32 sampleSize /*= 0*/)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto damageToTarget = AI->GetSpellDamageDoneInfo(victim);
    if (!damageToTarget || damageToTarget->empty())
    {
        TC_LOG_WARN("test.unit_test", "_TestSpellOutcomePercentage found no data for this victim (%s)", victim->GetName().c_str());
        return;
    }

    if (sampleSize)
    {
        INTERNAL_ASSERT_INFO("_TestSpellOutcomePercentage found %u results instead of expected sample size %u for spell %u", uint32(damageToTarget->size()), sampleSize, spellId);
        INTERNAL_TEST_ASSERT(damageToTarget->size() == sampleSize)
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    INTERNAL_ASSERT_INFO("_TestSpellOutcomePercentage was prompted for non existing spell ID %u", spellId);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);

    uint32 count = 0;
    for (auto itr : *damageToTarget)
    {
        if (itr.damageInfo.SpellID != spellId)
            continue;

        if (itr.missInfo != missInfo)
            continue;

        count++;
    }

    float const result = (count / float(damageToTarget->size())) * 100.0f;
    INTERNAL_ASSERT_INFO("TestSpellOutcomePercentage on spell %u: expected result: %f, result: %f", spellId, expectedResult, result);
    INTERNAL_TEST_ASSERT(Between<float>(expectedResult, result - allowedError, result + allowedError));
}

float TestCase::CalcChance(uint32 iterations, const std::function<bool()>& f)
{
	uint32 success = 0;
	for (uint32 i = 0; i < iterations; i++)
	{
		success += uint32(f());
	}
	return float(success) / float(iterations);
}

void TestCase::_SetCaller(std::string callerFile, int32 callerLine) 
{ 
    _callerFile = callerFile; 
    _callerLine = callerLine; 
}

void TestCase::_ResetCaller() 
{
    _callerFile = {}; _callerLine = 0;
}
std::string TestCase::_GetCallerFile() 
{ 
    return _callerFile; 
}

int32 TestCase::_GetCallerLine() 
{ 
    return _callerLine; 
}

void TestCase::Celebrate()
{
    if (!_map)
        return;

    if (Player* player = _map->GetFirstHumanPlayer())
    {
        //get a position in front of default location
        Position celebrateLocation;
        celebrateLocation.MoveInFront(player->GetPosition(), 10.0f);

        if (GameObject* gob = player->SummonGameObject(urand(180860, 180865), celebrateLocation, G3D::Quat(0, 0, 0, 0), 0))
        {
            gob->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5f);
            _map->RemoveFromMap(gob, false);
            gob->SetMap(_map);
            _map->AddToMap(gob);
            Wait(1);
            gob->Delete(); //this trigger explosion

            Wait(5 * SECOND * IN_MILLISECONDS);
        }
    }
}

void TestCase::_EnsureHasAura(Unit* target, int32 spellID)
{
    bool hasAura = target->HasAura(spellID);
    if (spellID > 0)
    {
        INTERNAL_ASSERT_INFO("Target %u (%s) does not have aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
        INTERNAL_TEST_ASSERT(hasAura);
    }
    else 
    {
        INTERNAL_ASSERT_INFO("Target %u (%s) has aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
        INTERNAL_TEST_ASSERT(!hasAura);
    }
}

void TestCase::_TestHasCooldown(TestPlayer* caster, uint32 castSpellID, uint32 cooldownSecond)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(castSpellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", castSpellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);
    uint32 cooldown = caster->GetSpellHistory()->GetRemainingCooldown(spellInfo);
    INTERNAL_ASSERT_INFO("Caster %s has cooldown %u for spell %u instead of expected %u", caster->GetName().c_str(), cooldown, castSpellID, cooldownSecond * IN_MILLISECONDS);
    INTERNAL_TEST_ASSERT(cooldown == cooldownSecond * IN_MILLISECONDS);
}

void TestCase::_TestAuraMaxDuration(Unit* target, uint32 spellID, uint32 durationMS)
{
    Aura* aura = target->GetAura(spellID);
    INTERNAL_ASSERT_INFO("Target %u (%s) does not have aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(aura != nullptr);

    uint32 auraDuration = aura->GetMaxDuration();
    INTERNAL_ASSERT_INFO("Target %u (%s) has aura (%u) with duration %u instead of %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID, auraDuration, durationMS);
    INTERNAL_TEST_ASSERT(auraDuration == durationMS);
}

void TestCase::_TestAuraStack(Unit* target, uint32 spellID, uint32 stacks, bool stack)
{
    Aura* aura = target->GetAura(spellID);
    INTERNAL_ASSERT_INFO("Target %u (%s) does not have aura of spell %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID);
    INTERNAL_TEST_ASSERT(aura != nullptr);

    uint32 auraStacks = 0;
    std::string type = "stacks";
    if (stack)
        auraStacks = aura->GetStackAmount();
    else
    {
        auraStacks = aura->GetCharges();
        type = "charges";
    }
    INTERNAL_ASSERT_INFO("Target %u (%s) has aura (%u) with %u %s instead of %u", target->GetGUID().GetCounter(), target->GetName().c_str(), spellID, auraStacks, type.c_str(), stacks);
    INTERNAL_TEST_ASSERT(auraStacks == stacks);
}

std::string TestCase::StringifySpellCastResult(SpellCastResult result)
{
    std::string str;
    switch(result)
    {
    case SPELL_CAST_OK: str = "SPELL_CAST_OK"; break;
    case SPELL_FAILED_AFFECTING_COMBAT: str = "SPELL_FAILED_AFFECTING_COMBAT"; break;
    case SPELL_FAILED_ALREADY_AT_FULL_HEALTH: str = "SPELL_FAILED_ALREADY_AT_FULL_HEALTH"; break;
    case SPELL_FAILED_ALREADY_AT_FULL_MANA: str = "SPELL_FAILED_ALREADY_AT_FULL_MANA"; break;
    case SPELL_FAILED_ALREADY_AT_FULL_POWER: str = "SPELL_FAILED_ALREADY_AT_FULL_POWER"; break;
    case SPELL_FAILED_ALREADY_BEING_TAMED: str = "SPELL_FAILED_ALREADY_BEING_TAMED"; break;
    case SPELL_FAILED_ALREADY_HAVE_CHARM: str = "SPELL_FAILED_ALREADY_HAVE_CHARM"; break;
    case SPELL_FAILED_ALREADY_HAVE_SUMMON: str = "SPELL_FAILED_ALREADY_HAVE_SUMMON"; break;
    case SPELL_FAILED_ALREADY_OPEN: str = "SPELL_FAILED_ALREADY_OPEN"; break;
    case SPELL_FAILED_AURA_BOUNCED: str = "SPELL_FAILED_AURA_BOUNCED"; break;
    case SPELL_FAILED_AUTOTRACK_INTERRUPTED: str = "SPELL_FAILED_AUTOTRACK_INTERRUPTED"; break;
    case SPELL_FAILED_BAD_IMPLICIT_TARGETS: str = "SPELL_FAILED_BAD_IMPLICIT_TARGETS"; break;
    case SPELL_FAILED_BAD_TARGETS: str = "SPELL_FAILED_BAD_TARGETS"; break;
    case SPELL_FAILED_CANT_BE_CHARMED: str = "SPELL_FAILED_CANT_BE_CHARMED"; break;
    case SPELL_FAILED_CANT_BE_DISENCHANTED: str = "SPELL_FAILED_CANT_BE_DISENCHANTED"; break;
    case SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL: str = "SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL"; break;
    case SPELL_FAILED_CANT_BE_PROSPECTED: str = "SPELL_FAILED_CANT_BE_PROSPECTED"; break;
    case SPELL_FAILED_CANT_CAST_ON_TAPPED: str = "SPELL_FAILED_CANT_CAST_ON_TAPPED"; break;
    case SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE: str = "SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE"; break;
    case SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED: str = "SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED"; break;
    case SPELL_FAILED_CANT_STEALTH: str = "SPELL_FAILED_CANT_STEALTH"; break;
    case SPELL_FAILED_CASTER_AURASTATE: str = "SPELL_FAILED_CASTER_AURASTATE"; break;
    case SPELL_FAILED_CASTER_DEAD: str = "SPELL_FAILED_CASTER_DEAD"; break;
    case SPELL_FAILED_CHARMED: str = "SPELL_FAILED_CHARMED"; break;
    case SPELL_FAILED_CHEST_IN_USE: str = "SPELL_FAILED_CHEST_IN_USE"; break;
    case SPELL_FAILED_CONFUSED: str = "SPELL_FAILED_CONFUSED"; break;
    case SPELL_FAILED_DONT_REPORT: str = "SPELL_FAILED_DONT_REPORT"; break;
    case SPELL_FAILED_EQUIPPED_ITEM: str = "SPELL_FAILED_EQUIPPED_ITEM"; break;
    case SPELL_FAILED_EQUIPPED_ITEM_CLASS: str = "SPELL_FAILED_EQUIPPED_ITEM_CLASS"; break;
    case SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND: str = "SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND"; break;
    case SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND: str = "SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND"; break;
    case SPELL_FAILED_ERROR: str = "SPELL_FAILED_ERROR"; break;
    case SPELL_FAILED_FIZZLE: str = "SPELL_FAILED_FIZZLE"; break;
    case SPELL_FAILED_FLEEING: str = "SPELL_FAILED_FLEEING"; break;
    case SPELL_FAILED_FOOD_LOWLEVEL: str = "SPELL_FAILED_FOOD_LOWLEVEL"; break;
    case SPELL_FAILED_HIGHLEVEL: str = "SPELL_FAILED_HIGHLEVEL"; break;
    case SPELL_FAILED_HUNGER_SATIATED: str = "SPELL_FAILED_HUNGER_SATIATED"; break;
    case SPELL_FAILED_IMMUNE: str = "SPELL_FAILED_IMMUNE"; break;
    case SPELL_FAILED_INTERRUPTED: str = "SPELL_FAILED_INTERRUPTED"; break;
    case SPELL_FAILED_INTERRUPTED_COMBAT: str = "SPELL_FAILED_INTERRUPTED_COMBAT"; break;
    case SPELL_FAILED_ITEM_ALREADY_ENCHANTED: str = "SPELL_FAILED_ITEM_ALREADY_ENCHANTED"; break;
    case SPELL_FAILED_ITEM_GONE: str = "SPELL_FAILED_ITEM_GONE"; break;
    case SPELL_FAILED_ITEM_NOT_FOUND: str = "SPELL_FAILED_ITEM_NOT_FOUND"; break;
    case SPELL_FAILED_ITEM_NOT_READY: str = "SPELL_FAILED_ITEM_NOT_READY"; break;
    case SPELL_FAILED_LEVEL_REQUIREMENT: str = "SPELL_FAILED_LEVEL_REQUIREMENT"; break;
    case SPELL_FAILED_LINE_OF_SIGHT: str = "SPELL_FAILED_LINE_OF_SIGHT"; break;
    case SPELL_FAILED_LOWLEVEL: str = "SPELL_FAILED_LOWLEVEL"; break;
    case SPELL_FAILED_LOW_CASTLEVEL: str = "SPELL_FAILED_LOW_CASTLEVEL"; break;
    case SPELL_FAILED_MAINHAND_EMPTY: str = "SPELL_FAILED_MAINHAND_EMPTY"; break;
    case SPELL_FAILED_MOVING: str = "SPELL_FAILED_MOVING"; break;
    case SPELL_FAILED_NEED_AMMO: str = "SPELL_FAILED_NEED_AMMO"; break;
    case SPELL_FAILED_NEED_AMMO_POUCH: str = "SPELL_FAILED_NEED_AMMO_POUCH"; break;
    case SPELL_FAILED_NEED_EXOTIC_AMMO: str = "SPELL_FAILED_NEED_EXOTIC_AMMO"; break;
    case SPELL_FAILED_NOPATH: str = "SPELL_FAILED_NOPATH"; break;
    case SPELL_FAILED_NOT_BEHIND: str = "SPELL_FAILED_NOT_BEHIND"; break;
    case SPELL_FAILED_NOT_FISHABLE: str = "SPELL_FAILED_NOT_FISHABLE"; break;
    case SPELL_FAILED_NOT_FLYING: str = "SPELL_FAILED_NOT_FLYING"; break;
    case SPELL_FAILED_NOT_HERE: str = "SPELL_FAILED_NOT_HERE"; break;
    case SPELL_FAILED_NOT_INFRONT: str = "SPELL_FAILED_NOT_INFRONT"; break;
    case SPELL_FAILED_NOT_IN_CONTROL: str = "SPELL_FAILED_NOT_IN_CONTROL"; break;
    case SPELL_FAILED_NOT_KNOWN: str = "SPELL_FAILED_NOT_KNOWN"; break;
    case SPELL_FAILED_NOT_MOUNTED: str = "SPELL_FAILED_NOT_MOUNTED"; break;
    case SPELL_FAILED_NOT_ON_TAXI: str = "SPELL_FAILED_NOT_ON_TAXI"; break;
    case SPELL_FAILED_NOT_ON_TRANSPORT: str = "SPELL_FAILED_NOT_ON_TRANSPORT"; break;
    case SPELL_FAILED_NOT_READY: str = "SPELL_FAILED_NOT_READY"; break;
    case SPELL_FAILED_NOT_SHAPESHIFT: str = "SPELL_FAILED_NOT_SHAPESHIFT"; break;
    case SPELL_FAILED_NOT_STANDING: str = "SPELL_FAILED_NOT_STANDING"; break;
    case SPELL_FAILED_NOT_TRADEABLE: str = "SPELL_FAILED_NOT_TRADEABLE"; break;
    case SPELL_FAILED_NOT_TRADING: str = "SPELL_FAILED_NOT_TRADING"; break;
    case SPELL_FAILED_NOT_UNSHEATHED: str = "SPELL_FAILED_NOT_UNSHEATHED"; break;
    case SPELL_FAILED_NOT_WHILE_GHOST: str = "SPELL_FAILED_NOT_WHILE_GHOST"; break;
    case SPELL_FAILED_NO_AMMO: str = "SPELL_FAILED_NO_AMMO"; break;
    case SPELL_FAILED_NO_CHARGES_REMAIN: str = "SPELL_FAILED_NO_CHARGES_REMAIN"; break;
    case SPELL_FAILED_NO_CHAMPION: str = "SPELL_FAILED_NO_CHAMPION"; break;
    case SPELL_FAILED_NO_COMBO_POINTS: str = "SPELL_FAILED_NO_COMBO_POINTS"; break;
    case SPELL_FAILED_NO_DUELING: str = "SPELL_FAILED_NO_DUELING"; break;
    case SPELL_FAILED_NO_ENDURANCE: str = "SPELL_FAILED_NO_ENDURANCE"; break;
    case SPELL_FAILED_NO_FISH: str = "SPELL_FAILED_NO_FISH"; break;
    case SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED: str = "SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED"; break;
    case SPELL_FAILED_NO_MOUNTS_ALLOWED: str = "SPELL_FAILED_NO_MOUNTS_ALLOWED"; break;
    case SPELL_FAILED_NO_PET: str = "SPELL_FAILED_NO_PET"; break;
    case SPELL_FAILED_NO_POWER: str = "SPELL_FAILED_NO_POWER"; break;
    case SPELL_FAILED_NOTHING_TO_DISPEL: str = "SPELL_FAILED_NOTHING_TO_DISPEL"; break;
    case SPELL_FAILED_NOTHING_TO_STEAL: str = "SPELL_FAILED_NOTHING_TO_STEAL"; break;
    case SPELL_FAILED_ONLY_ABOVEWATER: str = "SPELL_FAILED_ONLY_ABOVEWATER"; break;
    case SPELL_FAILED_ONLY_DAYTIME: str = "SPELL_FAILED_ONLY_DAYTIME"; break;
    case SPELL_FAILED_ONLY_INDOORS: str = "SPELL_FAILED_ONLY_INDOORS"; break;
    case SPELL_FAILED_ONLY_MOUNTED: str = "SPELL_FAILED_ONLY_MOUNTED"; break;
    case SPELL_FAILED_ONLY_NIGHTTIME: str = "SPELL_FAILED_ONLY_NIGHTTIME"; break;
    case SPELL_FAILED_ONLY_OUTDOORS: str = "SPELL_FAILED_ONLY_OUTDOORS"; break;
    case SPELL_FAILED_ONLY_SHAPESHIFT: str = "SPELL_FAILED_ONLY_SHAPESHIFT"; break;
    case SPELL_FAILED_ONLY_STEALTHED: str = "SPELL_FAILED_ONLY_STEALTHED"; break;
    case SPELL_FAILED_ONLY_UNDERWATER: str = "SPELL_FAILED_ONLY_UNDERWATER"; break;
    case SPELL_FAILED_OUT_OF_RANGE: str = "SPELL_FAILED_OUT_OF_RANGE"; break;
    case SPELL_FAILED_PACIFIED: str = "SPELL_FAILED_PACIFIED"; break;
    case SPELL_FAILED_POSSESSED: str = "SPELL_FAILED_POSSESSED"; break;
    case SPELL_FAILED_REAGENTS: str = "SPELL_FAILED_REAGENTS"; break;
    case SPELL_FAILED_REQUIRES_AREA: str = "SPELL_FAILED_REQUIRES_AREA"; break;
    case SPELL_FAILED_REQUIRES_SPELL_FOCUS: str = "SPELL_FAILED_REQUIRES_SPELL_FOCUS"; break;
    case SPELL_FAILED_ROOTED: str = "SPELL_FAILED_ROOTED"; break;
    case SPELL_FAILED_SILENCED: str = "SPELL_FAILED_SILENCED"; break;
    case SPELL_FAILED_SPELL_IN_PROGRESS: str = "SPELL_FAILED_SPELL_IN_PROGRESS"; break;
    case SPELL_FAILED_SPELL_LEARNED: str = "SPELL_FAILED_SPELL_LEARNED"; break;
    case SPELL_FAILED_SPELL_UNAVAILABLE: str = "SPELL_FAILED_SPELL_UNAVAILABLE"; break;
    case SPELL_FAILED_STUNNED: str = "SPELL_FAILED_STUNNED"; break;
    case SPELL_FAILED_TARGETS_DEAD: str = "SPELL_FAILED_TARGETS_DEAD"; break;
    case SPELL_FAILED_TARGET_AFFECTING_COMBAT: str = "SPELL_FAILED_TARGET_AFFECTING_COMBAT"; break;
    case SPELL_FAILED_TARGET_AURASTATE: str = "SPELL_FAILED_TARGET_AURASTATE"; break;
    case SPELL_FAILED_TARGET_DUELING: str = "SPELL_FAILED_TARGET_DUELING"; break;
    case SPELL_FAILED_TARGET_ENEMY: str = "SPELL_FAILED_TARGET_ENEMY"; break;
    case SPELL_FAILED_TARGET_ENRAGED: str = "SPELL_FAILED_TARGET_ENRAGED"; break;
    case SPELL_FAILED_TARGET_FRIENDLY: str = "SPELL_FAILED_TARGET_FRIENDLY"; break;
    case SPELL_FAILED_TARGET_IN_COMBAT: str = "SPELL_FAILED_TARGET_IN_COMBAT"; break;
    case SPELL_FAILED_TARGET_IS_PLAYER: str = "SPELL_FAILED_TARGET_IS_PLAYER"; break;
    case SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED: str = "SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED"; break;
    case SPELL_FAILED_TARGET_NOT_DEAD: str = "SPELL_FAILED_TARGET_NOT_DEAD"; break;
    case SPELL_FAILED_TARGET_NOT_IN_PARTY: str = "SPELL_FAILED_TARGET_NOT_IN_PARTY"; break;
    case SPELL_FAILED_TARGET_NOT_LOOTED: str = "SPELL_FAILED_TARGET_NOT_LOOTED"; break;
    case SPELL_FAILED_TARGET_NOT_PLAYER: str = "SPELL_FAILED_TARGET_NOT_PLAYER"; break;
    case SPELL_FAILED_TARGET_NO_POCKETS: str = "SPELL_FAILED_TARGET_NO_POCKETS"; break;
    case SPELL_FAILED_TARGET_NO_WEAPONS: str = "SPELL_FAILED_TARGET_NO_WEAPONS"; break;
    case SPELL_FAILED_TARGET_UNSKINNABLE: str = "SPELL_FAILED_TARGET_UNSKINNABLE"; break;
    case SPELL_FAILED_THIRST_SATIATED: str = "SPELL_FAILED_THIRST_SATIATED"; break;
    case SPELL_FAILED_TOO_CLOSE: str = "SPELL_FAILED_TOO_CLOSE"; break;
    case SPELL_FAILED_TOO_MANY_OF_ITEM: str = "SPELL_FAILED_TOO_MANY_OF_ITEM"; break;
    case SPELL_FAILED_TOTEM_CATEGORY: str = "SPELL_FAILED_TOTEM_CATEGORY"; break;
    case SPELL_FAILED_TOTEMS: str = "SPELL_FAILED_TOTEMS"; break;
    case SPELL_FAILED_TRAINING_POINTS: str = "SPELL_FAILED_TRAINING_POINTS"; break;
    case SPELL_FAILED_TRY_AGAIN: str = "SPELL_FAILED_TRY_AGAIN"; break;
    case SPELL_FAILED_UNIT_NOT_BEHIND: str = "SPELL_FAILED_UNIT_NOT_BEHIND"; break;
    case SPELL_FAILED_UNIT_NOT_INFRONT: str = "SPELL_FAILED_UNIT_NOT_INFRONT"; break;
    case SPELL_FAILED_WRONG_PET_FOOD: str = "SPELL_FAILED_WRONG_PET_FOOD"; break;
    case SPELL_FAILED_NOT_WHILE_FATIGUED: str = "SPELL_FAILED_NOT_WHILE_FATIGUED"; break;
    case SPELL_FAILED_TARGET_NOT_IN_INSTANCE: str = "SPELL_FAILED_TARGET_NOT_IN_INSTANCE"; break;
    case SPELL_FAILED_NOT_WHILE_TRADING: str = "SPELL_FAILED_NOT_WHILE_TRADING"; break;
    case SPELL_FAILED_TARGET_NOT_IN_RAID: str = "SPELL_FAILED_TARGET_NOT_IN_RAID"; break;
    case SPELL_FAILED_DISENCHANT_WHILE_LOOTING: str = "SPELL_FAILED_DISENCHANT_WHILE_LOOTING"; break;
    case SPELL_FAILED_PROSPECT_WHILE_LOOTING: str = "SPELL_FAILED_PROSPECT_WHILE_LOOTING"; break;
    case SPELL_FAILED_PROSPECT_NEED_MORE: str = "SPELL_FAILED_PROSPECT_NEED_MORE"; break;
    case SPELL_FAILED_TARGET_FREEFORALL: str = "SPELL_FAILED_TARGET_FREEFORALL"; break;
    case SPELL_FAILED_NO_EDIBLE_CORPSES: str = "SPELL_FAILED_NO_EDIBLE_CORPSES"; break;
    case SPELL_FAILED_ONLY_BATTLEGROUNDS: str = "SPELL_FAILED_ONLY_BATTLEGROUNDS"; break;
    case SPELL_FAILED_TARGET_NOT_GHOST: str = "SPELL_FAILED_TARGET_NOT_GHOST"; break;
    case SPELL_FAILED_TOO_MANY_SKILLS: str = "SPELL_FAILED_TOO_MANY_SKILLS"; break;
    case SPELL_FAILED_TRANSFORM_UNUSABLE: str = "SPELL_FAILED_TRANSFORM_UNUSABLE"; break;
    case SPELL_FAILED_WRONG_WEATHER: str = "SPELL_FAILED_WRONG_WEATHER"; break;
    case SPELL_FAILED_DAMAGE_IMMUNE: str = "SPELL_FAILED_DAMAGE_IMMUNE"; break;
    case SPELL_FAILED_PREVENTED_BY_MECHANIC: str = "SPELL_FAILED_PREVENTED_BY_MECHANIC"; break;
    case SPELL_FAILED_PLAY_TIME: str = "SPELL_FAILED_PLAY_TIME"; break;
    case SPELL_FAILED_REPUTATION: str = "SPELL_FAILED_REPUTATION"; break;
    case SPELL_FAILED_MIN_SKILL: str = "SPELL_FAILED_MIN_SKILL"; break;
    case SPELL_FAILED_NOT_IN_ARENA: str = "SPELL_FAILED_NOT_IN_ARENA"; break;
    case SPELL_FAILED_NOT_ON_SHAPESHIFT: str = "SPELL_FAILED_NOT_ON_SHAPESHIFT"; break;
    case SPELL_FAILED_NOT_ON_STEALTHED: str = "SPELL_FAILED_NOT_ON_STEALTHED"; break;
    case SPELL_FAILED_NOT_ON_DAMAGE_IMMUNE: str = "SPELL_FAILED_NOT_ON_DAMAGE_IMMUNE"; break;
    case SPELL_FAILED_NOT_ON_MOUNTED: str = "SPELL_FAILED_NOT_ON_MOUNTED"; break;
    case SPELL_FAILED_TOO_SHALLOW: str = "SPELL_FAILED_TOO_SHALLOW"; break;
    case SPELL_FAILED_TARGET_NOT_IN_SANCTUARY: str = "SPELL_FAILED_TARGET_NOT_IN_SANCTUARY"; break;
    case SPELL_FAILED_TARGET_IS_TRIVIAL: str = "SPELL_FAILED_TARGET_IS_TRIVIAL"; break;
    case SPELL_FAILED_BM_OR_INVISGOD: str = "SPELL_FAILED_BM_OR_INVISGOD"; break;
    case SPELL_FAILED_EXPERT_RIDING_REQUIREMENT: str = "SPELL_FAILED_EXPERT_RIDING_REQUIREMENT"; break;
    case SPELL_FAILED_ARTISAN_RIDING_REQUIREMENT: str = "SPELL_FAILED_ARTISAN_RIDING_REQUIREMENT"; break;
    case SPELL_FAILED_NOT_IDLE: str = "SPELL_FAILED_NOT_IDLE"; break;
    case SPELL_FAILED_NOT_INACTIVE: str = "SPELL_FAILED_NOT_INACTIVE"; break;
    case SPELL_FAILED_PARTIAL_PLAYTIME: str = "SPELL_FAILED_PARTIAL_PLAYTIME"; break;
    case SPELL_FAILED_NO_PLAYTIME: str = "SPELL_FAILED_NO_PLAYTIME"; break;
    case SPELL_FAILED_NOT_IN_BATTLEGROUND: str = "SPELL_FAILED_NOT_IN_BATTLEGROUND"; break;
    case SPELL_FAILED_ONLY_IN_ARENA: str = "SPELL_FAILED_ONLY_IN_ARENA"; break;
    case SPELL_FAILED_TARGET_LOCKED_TO_RAID_INSTANCE: str = "SPELL_FAILED_TARGET_LOCKED_TO_RAID_INSTANCE"; break;
    case SPELL_FAILED_UNKNOWN: str = "SPELL_FAILED_UNKNOWN"; break;
    default:
        str = "Unknown spell result" + std::to_string(result);
        break;
    }
    return str;
}
