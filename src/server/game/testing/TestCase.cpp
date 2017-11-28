#include "TestCase.h"
#include "MapManager.h"
#include "TestThread.h"
#include "RandomPlayerbotMgr.h"
#include "TestPlayer.h"
#include "RandomPlayerbotFactory.h"
#include "PlayerbotFactory.h"
#include "CharacterCache.h"

#define MAP_TESTING_ID 13
#define TEST_CREATURE_ENTRY 8

//same as TEST_ASSERT but will track caller file and line to print it in case of error
#define INTERNAL_TEST_ASSERT( expr ) _Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr, true, _GetCallerFile(), _GetCallerLine()); _ResetAssertInfo();
//same as last but does not increase test count
#define INTERNAL_TEST_ASSERT_NOCOUNT( expr ) _Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr, false, _GetCallerFile(), _GetCallerLine()); _ResetAssertInfo();

//input info for next check, place this before INTERNAL_TEST_ASSERT
#define INTERNAL_ASSERT_INFO(expr, ...) _AssertInfo(expr, ## __VA_ARGS__);

TestCase::TestCase(bool needMap) :
    _failed(false),
    _testsCount(0),
    _testMapInstanceId(0),
    _diff(REGULAR_DIFFICULTY),
    _map(nullptr),
    _enableMapObjects(false),
    _setup(false)
{
    //default pos:
    if (needMap && _location.GetMapId() == MAPID_INVALID) //not yet defined by the other constructor
        _location = std::move(WorldLocation(MAP_TESTING_ID, TestCase::GetDefaultPositionForMap(MAP_TESTING_ID)));
}

TestCase::TestCase(WorldLocation const& specificPosition)
    : TestCase()
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
        pos = Position(67.0f, -70.0f, -144.0f);
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

void TestCase::_ResetAssertInfo()
{
    _assertInfo = {};
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

void TestCase::TestStacksCount(TestPlayer* caster, Unit* target, uint32 castSpell, uint32 testSpell, uint32 requireCount)
{
   /* TODO: cast */
    uint32 auraCount = target->GetAuraCount(testSpell);
    INTERNAL_TEST_ASSERT(auraCount == requireCount);
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
    TC_LOG_ERROR("test.unit_test", "Creating new random bot for class %d", cls);

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

    TC_LOG_DEBUG("test.unit_test", "Test bot created for account %d - name: \"%s\"; race: %u; class: %u; gender: %u; skin: %u; face: %u; hairStyle: %u; hairColor: %u; facialHair: %u; outfitId: %u",
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

    sCharacterCache->AddCharacterCacheEntry(player->GetGUIDLow(), testAccountId, player->GetName(), cci.Gender, cci.Race, cci.Class, level, 0);

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
        spawnPosition.MoveInFront(_location, 10.0f);

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

void TestCase::EquipItem(TestPlayer* player, uint32 itemID)
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

void TestCase::TestDirectSpellDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedMinDamage, uint32 expectedMaxDamage)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);
    AI->ResetSpellCounters();

    uint32 maxPredictionError = (expectedMaxDamage - expectedMinDamage) / 25; //arbitary
    //const float confidenceLevel = 99.9f;
    uint32 sampleSize = 500;

    for (uint32 i = 0; i < sampleSize; i++)
    {
        uint32 result = caster->CastSpell(target, spellID, true);
        INTERNAL_ASSERT_INFO("Spell casting failed with reason %u", result);
        INTERNAL_TEST_ASSERT(result == SPELL_CAST_OK);
    }

    Wait(10 * SECOND * IN_MILLISECONDS);
    uint32 damageDealtMin;
    uint32 damageDealtMax;
    bool foundData = GetDamagePerSpellsTo(caster, target, spellID, damageDealtMin, damageDealtMax);
    INTERNAL_ASSERT_INFO("No data found for spell: %f", spellID);
    INTERNAL_TEST_ASSERT(foundData);
	TC_LOG_DEBUG("test.unit_test", "spellId: %u -> minDealt: %u - maxDealt %u - expectedMinDamage: %u - expectedMaxDamage: %u", spellID, damageDealtMin, damageDealtMax, expectedMinDamage, expectedMaxDamage);

    uint32 allowedMin = expectedMinDamage > maxPredictionError ? expectedMinDamage - maxPredictionError : 0; //protect against underflow
    uint32 allowedMax = expectedMaxDamage + maxPredictionError;

    INTERNAL_ASSERT_INFO("Enforcing high result for spell %u. allowedMax: %u, damageDealtMax: %u", spellID, allowedMax, damageDealtMax);
    INTERNAL_TEST_ASSERT(damageDealtMax <= allowedMax);
    INTERNAL_ASSERT_INFO("Enforcing low result for spell %u. allowedMin: %u, damageDealtMin: %u", spellID, allowedMin, damageDealtMin);
    INTERNAL_TEST_ASSERT(damageDealtMin >= allowedMin);
}

void TestCase::TestDirectHeal(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedHealMin, uint32 expectedHealMax)
{
    INTERNAL_ASSERT_INFO("TestDirectHeal NYI");
    INTERNAL_TEST_ASSERT(false);
}

float TestCase::GetChannelDamageTo(TestPlayer* caster, Unit* victim, uint32 spellID, uint32 tickCount, bool& mustRetry)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    if (spellInfo == nullptr)
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo was prompted for non existing spell ID %u", spellID);
        return 0.0f;
    }

    auto damageToTarget = AI->GetDamageDoneInfo(victim);
    if (damageToTarget.empty())
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo found no data for this victim (%s)", victim->GetName().c_str());
        return 0.0f;
    }

    decltype(damageToTarget) filteredDamageToTarget;
    //Copy only relevent entries
    for (auto itr : damageToTarget)
        if (itr.spellID == spellID)
            filteredDamageToTarget.push_back(itr);

    if (filteredDamageToTarget.size() != tickCount)
    {
        INTERNAL_ASSERT_INFO("Victim did not received expected tick count %u but received %u instead", tickCount, uint32(filteredDamageToTarget.size()));
        INTERNAL_TEST_ASSERT(false);
    }

    uint64 totalDamage = 0;
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
    }

    return totalDamage;
}

bool TestCase::GetDamagePerSpellsTo(TestPlayer* caster, Unit* victim, uint32 spellID, uint32& minDamage, uint32& maxDamage)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto damageToTarget = AI->GetDamageDoneInfo(victim);
    if (damageToTarget.empty())
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo found no data for this victim (%s)", victim->GetName().c_str());
        return false;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    if (spellInfo == nullptr)
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo was prompted for non existing spell ID %u", spellID);
        return false;
    }

    minDamage = std::numeric_limits<uint32>::max();
    maxDamage = 0;

    uint64 totalDamage = 0;
    uint32 count = 0;
    for (auto itr : damageToTarget)
    {
        if (itr.spellID != spellID)
            continue;

        //use only spells that hit target
        if (itr.missInfo != SPELL_MISS_NONE)
            continue;

        if (itr.crit)
            continue; //ignore crit... damage crits are affected by a whole lot of other factors so best just using regulars hit

        uint32 damage = itr.damageInfo.damage;
        damage += itr.damageInfo.resist;
        damage += itr.damageInfo.blocked;
        damage += itr.damageInfo.absorb;
        //resilience not taken into account...

        minDamage = std::min(minDamage, damage);
        maxDamage = std::max(maxDamage, damage);

        totalDamage += damage;
        count++;
    }

    if (count == 0) 
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo was prompted for a victim (%s) with no valid data for this spell (ID %u)", victim->GetName().c_str(), spellID);
        return false;
    }

    return true;
}

void TestCase::TestDotDamage(TestPlayer* caster, Unit* target, uint32 spellID, int32 expectedAmount)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    INTERNAL_ASSERT_INFO("Spell %u does not exists", spellID);
    INTERNAL_TEST_ASSERT(spellInfo != nullptr);
    bool spellHasFlyTime = spellInfo->Speed != 0.0f;

    for (uint32 i = 0; i < 100; i++)
    {
        AI->ResetSpellCounters();
        uint32 result = caster->CastSpell(target, spellID, true);
        if (result == SPELL_CAST_OK)
        {
            if (spellHasFlyTime)
                Wait(5 * SECOND*IN_MILLISECONDS);

            Wait(1);
            Aura* aura = target->GetAuraWithCaster(spellID, caster->GetGUID());
            if (!aura)
                continue; //aura could have been resisted

            //spell did hit, let's wait for dot duration
            uint32 waitTime = aura->GetAuraDuration() + 1 * SECOND * IN_MILLISECONDS;
            Wait(waitTime);
            //aura may be deleted at this point, do not use anymore

            //make sure aura expired
            INTERNAL_ASSERT_INFO("Target still has %u aura after %u ms", spellID, waitTime);
            INTERNAL_TEST_ASSERT(!target->HasAuraWithCaster(spellID, 0, caster->GetGUID()));

            int32 dotDamageToTarget = AI->GetDotDamage(target, spellID);
			TC_LOG_DEBUG("test.unit_test", "spellId: %u -> dotDamageToTarget: %i - expectedAmount: %i", spellID, dotDamageToTarget, expectedAmount);
            INTERNAL_ASSERT_INFO("Enforcing dot damage. dotDamageToTarget: %i, expectedAmount: %i", dotDamageToTarget, expectedAmount);
            TEST_ASSERT(dotDamageToTarget >= (expectedAmount - 6) && dotDamageToTarget <= (expectedAmount + 6)); //dots have greater error since they got their damage divided in several ticks
            return;
        }
    }
    INTERNAL_TEST_ASSERT((false && "Failed to cast the spell 100 times")); //failed to cast the spell 100 times
}

void TestCase::TestChannelDamage(TestPlayer* caster, Unit* target, uint32 spellID, uint32 testedSpell, uint32 tickCount, int32 expectedTickAmount)
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
        AI->ResetSpellCounters();
        uint32 result = caster->CastSpell(target, spellID, true);
        if (result == SPELL_CAST_OK)
        {
            Wait(baseCastTime + baseDurationTime + 1000);
            bool mustRetry = false;
            float totalChannelDmg = GetChannelDamageTo(caster, target, testedSpell, tickCount, mustRetry);
            if (mustRetry)
                continue;
            INTERNAL_ASSERT_INFO("Check if totalChannelDmg (%f) is round", totalChannelDmg);
            INTERNAL_TEST_ASSERT(totalChannelDmg == std::floor(totalChannelDmg));
            uint32 resultTickAmount = totalChannelDmg / tickCount;
            INTERNAL_ASSERT_INFO("Enforcing channel damage. resultTickAmount: %i, expectedTickAmount: %i", resultTickAmount, expectedTickAmount);
            INTERNAL_TEST_ASSERT(resultTickAmount >= (expectedTickAmount - 2) && resultTickAmount <= (expectedTickAmount + 2)); //channels have greater error since they got their damage divided in several ticks
            return;
        }
    }
    INTERNAL_ASSERT_INFO("Failed to cast spell (%u) 100 times", spellID);
    INTERNAL_TEST_ASSERT(false); //failed to cast the spell 100 times
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
        Position celebrateLocation = player->GetPosition();
        celebrateLocation.m_positionX = player->GetPosition().m_positionX + 10.0f * std::cos(player->GetPosition().m_orientation);
        celebrateLocation.m_positionY = player->GetPosition().m_positionY + 10.0f * std::sin(player->GetPosition().m_orientation);

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
