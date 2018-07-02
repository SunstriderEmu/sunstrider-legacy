#include "TestCase.h"
#include "MapManager.h"
#include "TestThread.h"
#include "RandomPlayerbotMgr.h"
#include "TestPlayer.h"
#include "RandomPlayerbotFactory.h"
#include "PlayerbotFactory.h"
#include "CharacterCache.h"
#include "SpellHistory.h"
#include "SpellAuraEffects.h"
//#include "ClassSpells.h" //I'm avoiding including this for now since it is changed often and will break script rebuild it is modified and TestCase.cpp has to be rebuilt too

#include <boost/math/special_functions/erf.hpp>

#define MAP_TESTING_ID 13

TestCase::TestCase() 
    : TestCase(STATUS_PASSING) 
{}

TestCase::TestCase(TestStatus status) :
    _callerLine(0),
    _testMapInstanceId(0),
    _diff(REGULAR_DIFFICULTY),
    _map(nullptr),
    _enableMapObjects(false),
    _setup(false),
    _testStatus(status)
{
    //default pos:
    if (_location.GetMapId() == MAPID_INVALID) //not yet defined by the other constructor
        _location = std::move(WorldLocation(MAP_TESTING_ID, TestCase::GetDefaultPositionForMap(MAP_TESTING_ID)));
}

TestCase::TestCase(TestStatus status, WorldLocation const specificPosition)
    : TestCase(status)
{
    bool useDefaultPos = specificPosition.GetPositionX() == 0.0f && specificPosition.GetPositionY() == 0.0f && specificPosition.GetPositionZ() == 0.0f;
    _location = specificPosition;
    if (useDefaultPos)
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
    TestSectionResult result(_testName,
        _inSection ? _inSection->title : "<no section>",
        false,
        _inSection ? _inSection->status : /*STATUS_PASSING*/ _testStatus, //out of section code is considered always passing
        msg
    );

    if (!_assertInfo.empty())
        result.AppendToError(_assertInfo);
    if (!_internalAssertInfo.empty())
        result.AppendToError(_internalAssertInfo);

    _results.push_back(std::move(result));
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
    _Assert(file, line, function, condition, failedCondition);
}

void TestCase::_Assert(std::string file, int32 line, std::string function, bool condition, std::string failedCondition, std::string callerFile, int32 callerLine)
{
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

    auto thread = _testThread.lock();
    if(thread)
        thread->Wait(ms);
}

void TestCase::WaitNextUpdate()
{
    Wait(1);
}

void TestCase::HandleThreadPause()
{
    auto thread = _testThread.lock();
    if(thread)
        thread->HandleThreadPause();
}

void TestCase::_Cleanup()
{
    Cleanup(); //test defined additional cleanup

    if (_testMapInstanceId) //may have no map if setup failed
    {
        //sMapMgr is not concurrency friendly, mutex it
        static std::mutex mapMgrMutex;
        mapMgrMutex.lock();
        sMapMgr->UnloadTestMap(_location.GetMapId(), _testMapInstanceId);
        mapMgrMutex.unlock();
    }
}

bool TestCase::_InternalSetup()
{
    ASSERT(!_map);
    ASSERT(_location.GetMapId() != MAPID_INVALID);

    //sMapMgr is not concurrency friendly, mutex it
    static std::mutex mapMgrMutex;
    mapMgrMutex.lock();
    auto pair = sMapMgr->CreateTestMap(_testThread, _location.GetMapId(), _diff, _enableMapObjects);
    mapMgrMutex.unlock();
    _map = pair.first;
    if (!_map)
        return false;

    _testMapInstanceId = pair.second;
    ASSERT(_testMapInstanceId);

    _setup = true;
    return true;
}

// ################### Utility functions 

void TestCase::EnableMapObjects()
{
    _enableMapObjects = true;
}

TestPlayer* TestCase::SpawnRandomPlayer()
{
    Classes cls = CLASS_NONE;
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race);
    TestPlayer* playerBot = _CreateTestBot(_location, cls, race);
    INTERNAL_ASSERT_INFO("Creating random test bot with class %u and race %u", uint32(cls), uint32(race));
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Powers power, uint32 level)
{
    Classes cls = CLASS_NONE;
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race, true, power);

    TestPlayer* playerBot = _CreateTestBot(_location, cls, race, level);
    INTERNAL_ASSERT_INFO("Creating random test with power %u and level %u", uint32(power), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Races race, uint32 level)
{
    TestPlayer* playerBot = _CreateTestBot(_location, _GetRandomClassForRace(race), race, level);
    INTERNAL_ASSERT_INFO("Creating random test bot with race %u and level %u", uint32(race), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Classes cls, uint32 level)
{
    TestPlayer* playerBot = _CreateTestBot(_location, cls, _GetRandomRaceForClass(cls), level);
    INTERNAL_ASSERT_INFO("Creating random test bot with class %u and level %u", uint32(cls), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnPlayer(Classes _class, Races _race, uint32 level, Position spawnPosition)
{
    Position targetLocation = _location;
    if (spawnPosition.GetPositionX() != 0.0f && spawnPosition.GetPositionY() != 0.0f && spawnPosition.GetPositionZ() != 0.0f)
        targetLocation.Relocate(spawnPosition);
    TestPlayer* playerBot = _CreateTestBot(targetLocation, _class, _race, level);
    INTERNAL_ASSERT_INFO("Creating random test bot with class %u, race %u and level %u", uint32(_class), uint32(_race), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
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
    /* This function is called from TestCase with concurrency, but several things in here don't handle it:
    - sCharacterCache->AddCharacterCacheEntry
    - session->_HandlePlayerLogin
    - player->Create

    Sooo, mutex the whole thing! No need for amazing performance here.
    */
    static std::mutex function_mutex;
    std::lock_guard<std::mutex> lock(function_mutex);

    INTERNAL_TEST_ASSERT(cls != CLASS_NONE && race != RACE_NONE);
    TC_LOG_TRACE("test.unit_test", "Creating new random bot for class %d", cls);

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

    { //Handle Player::SetMapAtCreation here
        //Players may cast spells at creation so they need to be on a map in Create
        player->Relocate(loc);
        player->SetMap(_map);
        player->UpdatePositionData();
    }

    if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), &cci))
    {
        delete player; //delete player before session! Player destructor does reference session
        delete session;
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
        delete session;
        delete player;
        delete ai;
        delete holder;                                      // delete all unprocessed queries
        return nullptr;
    }

    if (level == 0)
        level = sWorld->getConfig(CONFIG_MAX_PLAYER_LEVEL);

    player->GiveLevel(level);
    player->LearnAllClassProficiencies();
    player->UpdateSkillsToMaxSkillsForLevel();

    session->_HandlePlayerLogin((Player*)player, holder);

    sCharacterCache->AddCharacterCacheEntry(player->GetGUID().GetCounter(), testAccountId, player->GetName(), cci.Gender, cci.Race, cci.Class, level, 0);

    //usually done in LoadFromDB
    player->SetCanModifyStats(true);
    player->UpdateAllStats();

    //usually done in load from DB
    if (player->GetClass() == CLASS_WARRIOR)
        player->CastSpell(player, SPELL_ID_PASSIVE_BATTLE_STANCE, true);

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
        INTERNAL_TEST_ASSERT(!availableRacesForClass.empty());
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
        INTERNAL_TEST_ASSERT(!availableClassesForRace.empty());
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
        INTERNAL_TEST_ASSERT(false);
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
    if (spawnInFront)
        spawnPosition.MoveInFront(_location, 3.0f);

    return SpawnCreatureWithPosition(spawnPosition, entry);
}

TempSummon* TestCase::SpawnBoss(bool spawnInFront)
{
    return SpawnCreature(TEST_BOSS_ENTRY, spawnInFront);
}

TempSummon* TestCase::SpawnCreatureWithPosition(Position spawnPosition, uint32 entry)
{
    INTERNAL_ASSERT_INFO("Test has no map");
    INTERNAL_TEST_ASSERT(GetMap() != nullptr);
    uint32 creatureEntry = entry ? entry : TEST_CREATURE_ENTRY;

    TempSummon* summon = GetMap()->SummonCreature(creatureEntry, spawnPosition);
    INTERNAL_ASSERT_INFO("Failed to summon creature with entry %u", creatureEntry);
    INTERNAL_TEST_ASSERT(summon != nullptr);
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

void TestCase::_EquipItem(TestPlayer* player, uint32 itemID, bool newItem)
{
    Item* item;
    if (newItem)
    {
        item = player->AddItem(itemID, 1);
        INTERNAL_ASSERT_INFO("_EquipItem: Failed to add item %u to player", itemID);
        INTERNAL_TEST_ASSERT(item != nullptr);
    }
    else 
    {
        item = player->GetFirstItem(itemID);
        INTERNAL_ASSERT_INFO("_EquipItem: failed to find any item with id %u", itemID);
        INTERNAL_TEST_ASSERT(item != nullptr);

        INTERNAL_ASSERT_INFO("_EquipItem: First item found with id %u is already equipped", itemID);
        INTERNAL_TEST_ASSERT(!item->IsEquipped());
    }

    uint16 dest;
    uint8 msg = player->CanEquipItem(NULL_SLOT, dest, item, !item->IsBag());
    INTERNAL_ASSERT_INFO("Player cannot equip item %u, reason: %u", itemID, msg);
    INTERNAL_TEST_ASSERT(msg == EQUIP_ERR_OK);

    player->GetSession()->_HandleAutoEquipItemOpcode(item->GetBagSlot(), item->GetSlot());

    Item* equipedItem = player->GetItemByPos(dest);
    INTERNAL_ASSERT_INFO("Player failed to equip item %u (dest: %u)", itemID, dest);
    INTERNAL_TEST_ASSERT(equipedItem != nullptr);
}

void TestCase::LearnTalent(TestPlayer* p, uint32 spellID)
{
    if (p->HasSpell(spellID))
        return;

    p->LearnSpell(spellID, false);
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

std::pair<uint32 /*sampleSize*/, float /*absoluteTolerance*/> TestCase::_GetPercentApproximationParams(float const expectedResult, float absoluteTolerance /*= 0*/)
{
    //special speedups for extreme cases
    if(expectedResult == 1.0f || expectedResult == 0.0f)
        return std::make_pair(1000, 0.0001f);

    float const minExpectedResult = 0.01f;
    float const maxExpectedResult = 1.0f - minExpectedResult;

    float const defaultRelativeTolerance = 0.1f;

    float const minTolerance = minExpectedResult * defaultRelativeTolerance;
    float const maxTolerance = 0.1f;

    //enforce a minimum chance... else sample size will really go through the roof
    INTERNAL_ASSERT_INFO("Expected result %f too low", expectedResult);
    INTERNAL_TEST_ASSERT(expectedResult >= minExpectedResult);
    INTERNAL_ASSERT_INFO("Expected result %f too high", expectedResult);
    INTERNAL_TEST_ASSERT(expectedResult <= maxExpectedResult);

    //auto tolerance deduction:
    if (!absoluteTolerance)
    {
        float distFromExtreme = expectedResult <= 0.5f ? expectedResult : 1.0f - expectedResult; //distance from 0 or 100, whichever is closer (thus from 0 to 50)
        absoluteTolerance = distFromExtreme * defaultRelativeTolerance;

	//clamp
	absoluteTolerance = std::min(absoluteTolerance, maxTolerance);
	absoluteTolerance = std::max(absoluteTolerance, minTolerance);
    }
    else 
    {
        INTERNAL_ASSERT_INFO("Invalid input tolerance %f", absoluteTolerance);
        INTERNAL_TEST_ASSERT(Between(absoluteTolerance, minTolerance, maxTolerance));
    }

    INTERNAL_ASSERT_INFO("Expected result %f too low for tolerance %f", expectedResult, absoluteTolerance);
    INTERNAL_TEST_ASSERT(expectedResult >= absoluteTolerance);

    float const certainty = 0.999f;

    uint32 sampleSize = 2 * pow(boost::math::erf_inv(certainty) / absoluteTolerance, 2);
    sampleSize = std::max(sampleSize, uint32(100)); //min sample size

    //temp hack
    if (sampleSize > 200000)
        sampleSize = 200000;

    return std::make_pair(sampleSize, absoluteTolerance);
}

std::pair<uint32 /*sampleSize*/, uint32 /*absoluteTolerance*/> TestCase::_GetApproximationParams(uint32 const expectedMin, uint32 const expectedMax)
{
    uint32 sampleSize, absoluteAllowedError;

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

    return std::make_pair(sampleSize, absoluteAllowedError);
}

PlayerbotTestingAI* TestCase::_GetCasterAI(Unit*& caster, bool failOnNotFound)
{
    caster = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (caster == nullptr)
    {
        if (failOnNotFound)
        {
            INTERNAL_ASSERT_INFO("Caster is not a player or a pet/summon of him");
            INTERNAL_TEST_ASSERT(false);
        }
        else
            return nullptr;
    }

    TestPlayer* casterOwner = dynamic_cast<TestPlayer*>(caster);
    if (casterOwner == nullptr)
    {
        if (failOnNotFound)
        {
            INTERNAL_ASSERT_INFO("Caster in not a testing bot (or a pet/summon of testing bot)");
            INTERNAL_TEST_ASSERT(false);
        }
        else
            return nullptr;
    }

    return _GetCasterAI(casterOwner, failOnNotFound);
}

PlayerbotTestingAI* TestCase::_GetCasterAI(TestPlayer* caster, bool failOnNotFound)
{
    auto AI = caster->GetTestingPlayerbotAI();
    if (failOnNotFound)
    {
        INTERNAL_ASSERT_INFO("Caster in not a testing bot");
        INTERNAL_TEST_ASSERT(AI != nullptr);
    }

    return AI;
}

std::vector<PlayerbotTestingAI::SpellDamageDoneInfo> TestCase::GetSpellDamageDoneInfoTo(Unit* caster, Unit* victim, uint32 spellID, bool checkEmpty /*= true*/)
{
    auto AI = _GetCasterAI(caster);
    /*SpellInfo const* spellInfo = */ _GetSpellInfo(spellID);
    auto damageToTarget = AI->GetSpellDamageDoneInfo(victim);
    if (checkEmpty)
    {
        INTERNAL_ASSERT_INFO("GetSpellDamageDoneInfoTo found no data for this victim (%s)", victim->GetName().c_str());
        INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());
    }
    else {
        if (!damageToTarget)
            return {};
    }

    std::vector<PlayerbotTestingAI::SpellDamageDoneInfo> filteredDamageToTarget;
    //Copy only relevent entries
    for (auto itr : *damageToTarget)
        if (itr.spellID == spellID)
            filteredDamageToTarget.push_back(itr);

    return filteredDamageToTarget;
}

std::vector<PlayerbotTestingAI::HealingDoneInfo> TestCase::GetHealingDoneInfoTo(Unit* caster, Unit* target, uint32 spellID, bool checkEmpty /*= true*/)
{
    auto AI = _GetCasterAI(caster);
    /*SpellInfo const* spellInfo =*/ _GetSpellInfo(spellID);

    auto healingToTarget = AI->GetHealingDoneInfo(target);
    if (checkEmpty)
    {
        INTERNAL_ASSERT_INFO("GetHealingDoneInfoTo found no data for this victim (%s)", target->GetName().c_str());
        INTERNAL_TEST_ASSERT(healingToTarget && !healingToTarget->empty());
    }
    else
    {
        if (!healingToTarget)
            return {};
    }

    //Copy only relevent entries
    std::vector<PlayerbotTestingAI::HealingDoneInfo> filteredHealingToTarget;
    for (auto& itr : *healingToTarget)
        if (itr.spellID == spellID)
            filteredHealingToTarget.push_back(itr);

    return filteredHealingToTarget;
}

uint32 TestCase::GetChannelDamageTo(Unit* caster, Unit* victim, uint32 spellID, uint32 expectedTickCount, Optional<bool> crit)
{
    auto AI = _GetCasterAI(caster);
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);

    /* this functions actually handle two kind of spells:
    - Channels triggering another spells to do their damage
    - Channels using an aura on target to do damage
    */
    if (spellInfo->HasAuraEffect(SPELL_AURA_PERIODIC_DAMAGE))
    {
        auto [dotDmg, actualTicksCount] = AI->GetDotDamage(victim, spellID);
        INTERNAL_ASSERT_INFO("GetChannelDamageTo found some damage but tick count is %u instead of %u", actualTicksCount, expectedTickCount);
        INTERNAL_TEST_ASSERT(actualTicksCount == expectedTickCount);
        return dotDmg;
    }

    auto [dealtMin, dealtMax] = GetDamagePerSpellsTo(caster, victim, spellID, crit, expectedTickCount);
    INTERNAL_ASSERT_INFO("dealtMin %u != dealtMax %u", dealtMin, dealtMax);
    INTERNAL_TEST_ASSERT(dealtMin == dealtMax);

    return dealtMin * expectedTickCount;
}

uint32 TestCase::GetChannelHealingTo(Unit* caster, Unit* target, uint32 spellID, uint32 expectedTickCount, Optional<bool> crit)
{
    auto [dealtMin, dealtMax] = GetHealingPerSpellsTo(caster, target, spellID, crit, expectedTickCount);
    INTERNAL_ASSERT_INFO("dealtMin %u != dealtMax %u", dealtMin, dealtMax);
    INTERNAL_TEST_ASSERT(dealtMin == dealtMax);

    return dealtMin * expectedTickCount;
}

std::pair<uint32 /*minHeal*/, uint32 /*maxHeal*/> TestCase::GetHealingPerSpellsTo(Unit* caster, Unit* target, uint32 spellID, Optional<bool> crit, uint32 expectedCount)
{
    auto healingToTarget = GetHealingDoneInfoTo(caster, target, spellID);

    uint32 minHeal = std::numeric_limits<uint32>::max();
    uint32 maxHeal = 0;

    uint32 count = 0;
    for (auto itr : healingToTarget)
    {
        //use only spells that hit target
        if (itr.missInfo != SPELL_MISS_NONE)
            continue;

        if (crit && itr.crit != crit.get())
            continue;

        minHeal = std::min(minHeal, itr.healing);
        maxHeal = std::max(maxHeal, itr.healing);

        count++;
    }

    INTERNAL_ASSERT_INFO("GetHealingPerSpellsTo was prompted for a target (%s) with no valid data for %s", target->GetName().c_str(), _SpellString(spellID).c_str());
    INTERNAL_TEST_ASSERT(count != 0);

    if (expectedCount)
    {
        INTERNAL_ASSERT_INFO("GetHealingPerSpellsTo did find data for %s and target %s, but not expected count (%u instead of %u)", _SpellString(spellID).c_str(), target->GetName().c_str(), count, expectedCount);
        INTERNAL_TEST_ASSERT(count == expectedCount);
    }
    return std::make_pair(minHeal, maxHeal);
}

std::pair<uint32 /*minDmg*/, uint32 /*maxDmg*/> TestCase::GetWhiteDamageDoneTo(Unit* caster, Unit* victim, WeaponAttackType attackType, bool critical, uint32 expectedCount)
{
    if (attackType == RANGED_ATTACK)
    { //shoot is handled like a spell
        uint32 const SHOOT_SPELL_ID = 75;
        return GetDamagePerSpellsTo(caster, victim, SHOOT_SPELL_ID, false, expectedCount);
    }

    auto AI = _GetCasterAI(caster);

    auto damageToTarget = AI->GetMeleeDamageDoneInfo(victim);
    INTERNAL_ASSERT_INFO("GetWhiteDamageDoneTo found no data for this victim (%s)", victim->GetName().c_str());
    INTERNAL_TEST_ASSERT(damageToTarget && !damageToTarget->empty());

    uint32 minDamage = std::numeric_limits<uint32>::max();
    uint32 maxDamage = 0;

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
    return std::make_pair(minDamage, maxDamage);
}

std::pair<uint32 /*minDmg*/, uint32 /*maxDmg*/> TestCase::GetDamagePerSpellsTo(Unit* caster, Unit* victim, uint32 spellID, Optional<bool> crit, uint32 expectedCount)
{
    auto damageToTarget = GetSpellDamageDoneInfoTo(caster, victim, spellID);

    uint32 minDamage = std::numeric_limits<uint32>::max();
    uint32 maxDamage = 0;

    //uint64 totalDamage = 0;
    uint32 count = 0;
    for (auto itr : damageToTarget)
    {
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

    INTERNAL_ASSERT_INFO("GetDamagePerSpellsTo was prompted for a victim(%s) with no valid data for %s", victim->GetName().c_str(), _SpellString(spellID).c_str());
    INTERNAL_TEST_ASSERT(count != 0);

    if (expectedCount)
    {
        INTERNAL_ASSERT_INFO("GetDamagePerSpellsTo did find data for %s and target %s, but not expected count (%u instead of %u)", _SpellString(spellID).c_str(), victim->GetName().c_str(), count, expectedCount);
        INTERNAL_TEST_ASSERT(count == expectedCount);
    }
    return std::make_pair(minDamage, maxDamage);
}

void TestCase::EnableCriticals(Unit* caster, bool crit)
{
    SaveCriticals(caster);

    float critChance = -100.0f;
	if (crit)
		critChance = 200.0f;

	for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
		caster->SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i, critChance);

	caster->SetFloatValue(PLAYER_CRIT_PERCENTAGE, critChance); // BASE_ATTACK
	caster->SetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE, critChance); // OFF_ATTACK
	caster->SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, critChance); // RANGED_ATTACK

    caster->m_baseSpellCritChance = int32(critChance);
}

void TestCase::SaveCriticals(Unit* caster)
{
    SavedCriticalValues& values = _savedCriticalValues[caster];

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        values.spellCrit[i] = caster->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i);

    values.mainCrit = caster->GetFloatValue(PLAYER_CRIT_PERCENTAGE);
    values.offCrit = caster->GetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE);
    values.rangedCrit = caster->GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE);
    values.baseCrit = caster->m_baseSpellCritChance;
}

void TestCase::RestoreCriticals(Unit* caster)
{
    auto itr = _savedCriticalValues.find(caster);
    INTERNAL_ASSERT_INFO("RestoreCriticals: Trying to restore non existent saved critical values");
    INTERNAL_TEST_ASSERT(itr != _savedCriticalValues.end());

    SavedCriticalValues const values = itr->second;

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        caster->SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + i, values.spellCrit[i]);

    caster->SetFloatValue(PLAYER_CRIT_PERCENTAGE, values.mainCrit); // BASE_ATTACK
    caster->SetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE, values.offCrit); // OFF_ATTACK
    caster->SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, values.rangedCrit); // RANGED_ATTACK

    caster->m_baseSpellCritChance = values.baseCrit;
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

void TestCase::_EnsureAlive(Unit* caster, Unit* target)
{
    INTERNAL_ASSERT_INFO("Only support alive caster");
    INTERNAL_TEST_ASSERT(caster->IsAlive());
    INTERNAL_ASSERT_INFO("Only support alive victim");
    INTERNAL_TEST_ASSERT(target->IsAlive());
}

SpellInfo const* TestCase::_GetSpellInfo(uint32 spellID)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);

    return spellInfo;
}

void TestCase::_StartUnitChannels(Unit* unit)
{
    //this function actually exists just to have this comment not repeated all over the place:

    //Currently, channeled spells start at the next update so we need to wait for it to be applied.
    //(If this isn't true anymore, this function and all its calls can be deleted)
    //We can't loop on spells thousands of times if we need to wait each time...
    //So we force a SpellEvent update to help with it
    unit->m_Events.Update(0);
}

void TestCase::HandleSpellsCleanup(Unit* caster)
{
    //Spell deletions are done in SpellEvent
    EventList& eventList = caster->m_Events.m_events;
    for (auto itr = eventList.begin(); itr != eventList.end();)
    {
        if (SpellEvent* spellEvent = dynamic_cast<SpellEvent*>(itr->second))
            if (spellEvent->m_Spell->getState() == SPELL_STATE_FINISHED && spellEvent->m_Spell->IsDeletable())
            {
                //what we're doing here is mimicing the EventProcessor::Update + SpellEvent::Execute behavior in this case, that is -> just delete the event.
                itr = eventList.erase(itr);
                delete spellEvent; //SpellEvent deletion handle spell deletion
                continue;
            }

        itr++;
    }
}

void TestCase::_MaxHealth(Unit* unit, bool lowHealth /*= false*/)
{
    _SaveUnitState(unit);

    unit->SetMaxHealth(std::numeric_limits<int32>::max());
    if (lowHealth)
        unit->SetHealth(1);
    else
        unit->SetFullHealth();
}

void TestCase::_SaveUnitState(Unit* unit)
{
    INTERNAL_ASSERT_INFO("Unit %s was dead when trying to save state", unit->GetName().c_str());
    INTERNAL_TEST_ASSERT(unit->IsAlive());

    Powers powerType = unit->GetPowerType();
    _saveUnitStates.emplace(std::piecewise_construct,
        std::forward_as_tuple(unit),
        std::forward_as_tuple(unit->GetHealth(), unit->GetMaxHealth(), powerType, unit->GetPower(powerType), unit->GetMaxPower(powerType))
    );
}

void TestCase::_RestoreUnitState(Unit* unit)
{
    auto itr = _saveUnitStates.find(unit);
    if (itr != _saveUnitStates.end())
    {
        SavedUnitState& state = itr->second;
        unit->SetMaxHealth(state.maxHealth);
        unit->SetHealth(state.health);
        unit->SetMaxPower(state.powerType, state.maxPower);
        unit->SetPower(state.powerType, state.power);
    }
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
    _callerFile = {}; 
    _callerLine = 0;
    _ResetAssertInfo(); //also reset assert info to allow usage of ASSERT_INFO with all TEST_* macro
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
    if (Player* player = _map->GetFirstHumanPlayer())
    {
        player->PlaySound(6199, false); //Peon "Work complete!"

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

void TestCase::Sadness()
{
    //This is actually useful to let the player see what happened
    if (Player* player = _map->GetFirstHumanPlayer())
    {
        player->PlaySound(10072, false); //fel orc death
        Wait(15 * SECOND * IN_MILLISECONDS);
    }
}

float TestCase::GetArmorFactor(Player const* attacker, Unit const* target)
{
    return 1.0f - (target->GetArmor() / (target->GetArmor() + (467.5 * attacker->GetLevel() - 22167.5)));
}

std::pair<uint32 /*min*/, uint32 /*max*/> TestCase::CalcMeleeDamage(Player const* attacker, Unit const* target, WeaponAttackType attackType, int32 spellBonusDmg /*= 0*/, float spellNormalizedWeaponSpeed /* = 0.0f*/)
{
    Item* item = attacker->GetWeaponForAttack(attackType);
    INTERNAL_ASSERT_INFO("Failed to get weapon for attack type %u", uint32(attackType));
    INTERNAL_TEST_ASSERT(item != nullptr);
    uint32 const weaponMinDmg = item->GetTemplate()->Damage->DamageMin;
    uint32 const weaponMaxDmg = item->GetTemplate()->Damage->DamageMax;
    float const weaponSpeed = spellNormalizedWeaponSpeed ? spellNormalizedWeaponSpeed : item->GetTemplate()->Delay / 1000.0f;
    float const AP = attacker->GetTotalAttackPowerValue(attackType);
    float const armorFactor = GetArmorFactor(attacker, target);
    float const offHandMultiplier = attacker->GetPctModifierValue(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT);
    INTERNAL_ASSERT_INFO("Invalid offhand multiplier returned by core %f", offHandMultiplier);
    INTERNAL_TEST_ASSERT(offHandMultiplier >= 0.5f && offHandMultiplier <= 1.0f);
    float const damageMultiplier = attackType == OFF_ATTACK ? 0.5f : 1.f;
    uint32 const minMelee = floor(weaponMinDmg + AP / 14.f * weaponSpeed + spellBonusDmg) * damageMultiplier * armorFactor;
    uint32 const maxMelee = floor(weaponMaxDmg + AP / 14.f * weaponSpeed + spellBonusDmg) * damageMultiplier * armorFactor;
    return std::make_pair(minMelee, maxMelee);
}

std::string TestCase::_SpellString(uint32 spellID)
{
    SpellInfo const* spellInfo = _GetSpellInfo(spellID);
    std::stringstream stream;
    stream << "[Spell " << spellID << "|" << spellInfo->SpellName[LOCALE_enUS] << "]";
    return stream.str();
}

template<typename E, typename T = char const*>
struct EnumName
{
    E Value;
    T Name;
};
#define CREATE_NAMED_ENUM(VALUE) { VALUE, STRINGIZE(VALUE) }

EnumName<SpellCastResult> const spellCastResults[MAX_SPELL_FAILED_RESULT] =
{
    //SPELL_CAST_OK handled separately
    CREATE_NAMED_ENUM(SPELL_FAILED_AFFECTING_COMBAT),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_AT_FULL_HEALTH),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_AT_FULL_MANA),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_AT_FULL_POWER),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_BEING_TAMED),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_HAVE_CHARM),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_HAVE_SUMMON),
    CREATE_NAMED_ENUM(SPELL_FAILED_ALREADY_OPEN),
    CREATE_NAMED_ENUM(SPELL_FAILED_AURA_BOUNCED),
    CREATE_NAMED_ENUM(SPELL_FAILED_AUTOTRACK_INTERRUPTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_BAD_IMPLICIT_TARGETS),
    CREATE_NAMED_ENUM(SPELL_FAILED_BAD_TARGETS),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_BE_CHARMED),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_BE_DISENCHANTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_BE_PROSPECTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_CAST_ON_TAPPED),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED),
    CREATE_NAMED_ENUM(SPELL_FAILED_CANT_STEALTH),
    CREATE_NAMED_ENUM(SPELL_FAILED_CASTER_AURASTATE),
    CREATE_NAMED_ENUM(SPELL_FAILED_CASTER_DEAD),
    CREATE_NAMED_ENUM(SPELL_FAILED_CHARMED),
    CREATE_NAMED_ENUM(SPELL_FAILED_CHEST_IN_USE),
    CREATE_NAMED_ENUM(SPELL_FAILED_CONFUSED),
    CREATE_NAMED_ENUM(SPELL_FAILED_DONT_REPORT),
    CREATE_NAMED_ENUM(SPELL_FAILED_EQUIPPED_ITEM),
    CREATE_NAMED_ENUM(SPELL_FAILED_EQUIPPED_ITEM_CLASS),
    CREATE_NAMED_ENUM(SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND),
    CREATE_NAMED_ENUM(SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND),
    CREATE_NAMED_ENUM(SPELL_FAILED_ERROR),
    CREATE_NAMED_ENUM(SPELL_FAILED_FIZZLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_FLEEING),
    CREATE_NAMED_ENUM(SPELL_FAILED_FOOD_LOWLEVEL),
    CREATE_NAMED_ENUM(SPELL_FAILED_HIGHLEVEL),
    CREATE_NAMED_ENUM(SPELL_FAILED_HUNGER_SATIATED),
    CREATE_NAMED_ENUM(SPELL_FAILED_IMMUNE),
    CREATE_NAMED_ENUM(SPELL_FAILED_INTERRUPTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_INTERRUPTED_COMBAT),
    CREATE_NAMED_ENUM(SPELL_FAILED_ITEM_ALREADY_ENCHANTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_ITEM_GONE),
    CREATE_NAMED_ENUM(SPELL_FAILED_ITEM_NOT_FOUND),
    CREATE_NAMED_ENUM(SPELL_FAILED_ITEM_NOT_READY),
    CREATE_NAMED_ENUM(SPELL_FAILED_LEVEL_REQUIREMENT),
    CREATE_NAMED_ENUM(SPELL_FAILED_LINE_OF_SIGHT),
    CREATE_NAMED_ENUM(SPELL_FAILED_LOWLEVEL),
    CREATE_NAMED_ENUM(SPELL_FAILED_LOW_CASTLEVEL),
    CREATE_NAMED_ENUM(SPELL_FAILED_MAINHAND_EMPTY),
    CREATE_NAMED_ENUM(SPELL_FAILED_MOVING),
    CREATE_NAMED_ENUM(SPELL_FAILED_NEED_AMMO),
    CREATE_NAMED_ENUM(SPELL_FAILED_NEED_AMMO_POUCH),
    CREATE_NAMED_ENUM(SPELL_FAILED_NEED_EXOTIC_AMMO),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOPATH),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_BEHIND),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_FISHABLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_FLYING),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_HERE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_INFRONT),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_IN_CONTROL),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_KNOWN),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_MOUNTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_ON_TAXI),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_ON_TRANSPORT),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_READY),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_SHAPESHIFT),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_STANDING),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_TRADEABLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_TRADING),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_UNSHEATHED),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_WHILE_GHOST),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_AMMO),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_CHARGES_REMAIN),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_CHAMPION),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_COMBO_POINTS),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_DUELING),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_ENDURANCE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_FISH),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_MOUNTS_ALLOWED),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_PET),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_POWER),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOTHING_TO_DISPEL),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOTHING_TO_STEAL),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_ABOVEWATER),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_DAYTIME),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_INDOORS),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_MOUNTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_NIGHTTIME),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_OUTDOORS),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_SHAPESHIFT),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_STEALTHED),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_UNDERWATER),
    CREATE_NAMED_ENUM(SPELL_FAILED_OUT_OF_RANGE),
    CREATE_NAMED_ENUM(SPELL_FAILED_PACIFIED),
    CREATE_NAMED_ENUM(SPELL_FAILED_POSSESSED),
    CREATE_NAMED_ENUM(SPELL_FAILED_REAGENTS),
    CREATE_NAMED_ENUM(SPELL_FAILED_REQUIRES_AREA),
    CREATE_NAMED_ENUM(SPELL_FAILED_REQUIRES_SPELL_FOCUS),
    CREATE_NAMED_ENUM(SPELL_FAILED_ROOTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_SILENCED),
    CREATE_NAMED_ENUM(SPELL_FAILED_SPELL_IN_PROGRESS),
    CREATE_NAMED_ENUM(SPELL_FAILED_SPELL_LEARNED),
    CREATE_NAMED_ENUM(SPELL_FAILED_SPELL_UNAVAILABLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_STUNNED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGETS_DEAD),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_AFFECTING_COMBAT),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_AURASTATE),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_DUELING),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_ENEMY),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_ENRAGED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_FRIENDLY),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_IN_COMBAT),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_IS_PLAYER),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_DEAD),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_IN_PARTY),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_LOOTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_PLAYER),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NO_POCKETS),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NO_WEAPONS),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_UNSKINNABLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_THIRST_SATIATED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TOO_CLOSE),
    CREATE_NAMED_ENUM(SPELL_FAILED_TOO_MANY_OF_ITEM),
    CREATE_NAMED_ENUM(SPELL_FAILED_TOTEM_CATEGORY),
    CREATE_NAMED_ENUM(SPELL_FAILED_TOTEMS),
    CREATE_NAMED_ENUM(SPELL_FAILED_TRAINING_POINTS),
    CREATE_NAMED_ENUM(SPELL_FAILED_TRY_AGAIN),
    CREATE_NAMED_ENUM(SPELL_FAILED_UNIT_NOT_BEHIND),
    CREATE_NAMED_ENUM(SPELL_FAILED_UNIT_NOT_INFRONT),
    CREATE_NAMED_ENUM(SPELL_FAILED_WRONG_PET_FOOD),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_WHILE_FATIGUED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_IN_INSTANCE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_WHILE_TRADING),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_IN_RAID),
    CREATE_NAMED_ENUM(SPELL_FAILED_DISENCHANT_WHILE_LOOTING),
    CREATE_NAMED_ENUM(SPELL_FAILED_PROSPECT_WHILE_LOOTING),
    CREATE_NAMED_ENUM(SPELL_FAILED_PROSPECT_NEED_MORE),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_FREEFORALL),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_EDIBLE_CORPSES),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_BATTLEGROUNDS),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_GHOST),
    CREATE_NAMED_ENUM(SPELL_FAILED_TOO_MANY_SKILLS),
    CREATE_NAMED_ENUM(SPELL_FAILED_TRANSFORM_UNUSABLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_WRONG_WEATHER),
    CREATE_NAMED_ENUM(SPELL_FAILED_DAMAGE_IMMUNE),
    CREATE_NAMED_ENUM(SPELL_FAILED_PREVENTED_BY_MECHANIC),
    CREATE_NAMED_ENUM(SPELL_FAILED_PLAY_TIME),
    CREATE_NAMED_ENUM(SPELL_FAILED_REPUTATION),
    CREATE_NAMED_ENUM(SPELL_FAILED_MIN_SKILL),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_IN_ARENA),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_ON_SHAPESHIFT),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_ON_STEALTHED),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_ON_DAMAGE_IMMUNE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_ON_MOUNTED),
    CREATE_NAMED_ENUM(SPELL_FAILED_TOO_SHALLOW),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_NOT_IN_SANCTUARY),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_IS_TRIVIAL),
    CREATE_NAMED_ENUM(SPELL_FAILED_BM_OR_INVISGOD),
    CREATE_NAMED_ENUM(SPELL_FAILED_EXPERT_RIDING_REQUIREMENT),
    CREATE_NAMED_ENUM(SPELL_FAILED_ARTISAN_RIDING_REQUIREMENT),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_IDLE),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_INACTIVE),
    CREATE_NAMED_ENUM(SPELL_FAILED_PARTIAL_PLAYTIME),
    CREATE_NAMED_ENUM(SPELL_FAILED_NO_PLAYTIME),
    CREATE_NAMED_ENUM(SPELL_FAILED_NOT_IN_BATTLEGROUND),
    CREATE_NAMED_ENUM(SPELL_FAILED_ONLY_IN_ARENA),
    CREATE_NAMED_ENUM(SPELL_FAILED_TARGET_LOCKED_TO_RAID_INSTANCE),
    CREATE_NAMED_ENUM(SPELL_FAILED_UNKNOWN),
};

std::string TestCase::StringifySpellCastResult(SpellCastResult result)
{
    if (result == SPELL_CAST_OK)
        return "SPELL_CAST_OK";

    if(result >= MAX_SPELL_FAILED_RESULT)
        return "Unknown spell result" + std::to_string(result);

    return spellCastResults[result].Name;
}

void TestCase::ResetSpellCast(Unit* caster)
{

}

void TestCase::SECTION(std::string const title, std::function<void()> func)
{
    SECTION(title, STATUS_PASSING, std::move(func));
}

void TestCase::SECTION(std::string const title, TestStatus status, std::function<void()> func)
{
    //special case, only run incomplete sections if test was directly called
    if (status == STATUS_WIP && _testName != _usedPattern)
        return;

    _inSection.emplace(title, status);
    _beforeEach();
    try 
    {
        func();
        _results.emplace_back(_testName, title, true, status, "(no error)");
    }
    catch (TestException e)
    {
        // Framework triggered an exception, section failed!
        // Nothing more to do, _FailNoException has already created a SectionResult
    }
    catch (std::exception& e)
    {
        // A regular exception happened (not one the framework triggered). Set its what in failure message
        _FailNoException(e.what());
    }
    _afterEach();
    _inSection.reset();
}

void TestCase::_Test()
{
    try
    {
        Test();
        //no section result for out of section code
    }
    catch (TestException e)
    {
        // Framework triggered an exception, section failed!
        // Nothing more to do, _FailNoException has already created a SectionResult
    }
    catch (std::exception& e)
    {
        // A regular exception happened (not one the framework triggered). Set its what in failure message
        _FailNoException(e.what());
    }
}

void TestCase::BEFORE_EACH(std::function<void()> func)
{
    _beforeEach = func;
}

void TestCase::AFTER_EACH(std::function<void()> func)
{
    _afterEach = func;
}

bool TestCase::HasFailures()
{
    for (auto const& itr : _results)
        if (!itr.IsSuccess())
            return true;

    return false;
}
