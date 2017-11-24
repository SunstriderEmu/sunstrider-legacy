#include "TestCase.h"
#include "MapManager.h"
#include "TestThread.h"
#include "RandomPlayerBotMgr.h"
#include "TestPlayer.h"
#include "RandomPlayerbotFactory.h"
#include "PlayerbotFactory.h"
#include "CharacterCache.h"

#define MAP_TESTING_ID 13
#define TEST_CREATURE_ENTRY 8

//same as TEST_ASSERT but will track caller file and line to print it in case of error
#define INTERNAL_TEST_ASSERT( expr ) _Assert(__FILE__, __LINE__, __FUNCTION__, (expr == true), #expr, true, _GetCallerFile(), _GetCallerLine()); _ResetAssertInfo();
//input info for next check, place this before INTERNAL_TEST_ASSERT
#define ASSERT_INFO( expr, ... ) _AssertInfo(expr, __VA_ARGS__);

TestCase::TestCase(bool needMap) :
    _failed(false),
    _testsCount(0),
    _testMapInstanceId(0),
    _diff(REGULAR_DIFFICULTY),
    _map(nullptr),
    _enableMapObjects(false)
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
    throw std::exception(EXCEPTION_MESSAGE);
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
        return true;

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

void TestCase::TestStacksCount(TestPlayer* caster, Unit* target, uint32 talent, uint32 castSpell, uint32 testSpell, uint32 requireCount)
{
    caster->LearnSpell(castSpell, false);
    caster->LearnSpell(talent, false);

    uint32 castCount = 0;
    while (castCount < requireCount * 5) { //cast a bunch more
        caster->CastSpell(target, castSpell, true);
        castCount++;
    }
    uint32 auraCount = target->GetAuraCount(testSpell);
    INTERNAL_TEST_ASSERT(auraCount == requireCount);
}

TestPlayer* TestCase::SpawnRandomPlayer()
{
    Classes cls = CLASS_NONE;
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race);
    TestPlayer* playerBot = _CreateTestBot(_location, cls, race);
    ASSERT_INFO("Creating random test bot with class %u and race %u", uint32(cls), uint32(race));
    TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Powers power, uint32 level)
{
    Classes cls = CLASS_NONE;
    Races race = RACE_NONE;
    _GetRandomClassAndRace(cls, race, true, power);

    TestPlayer* playerBot = _CreateTestBot(_location, cls, race, level);
    ASSERT_INFO("Creating random test with power %u and level %u", uint32(power), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Races race, uint32 level)
{
    TestPlayer* playerBot = _CreateTestBot(_location, _GetRandomClassForRace(race), race, level);
    ASSERT_INFO("Creating random test bot with race %u and level %u", uint32(race), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnRandomPlayer(Classes cls, uint32 level)
{
    TestPlayer* playerBot = _CreateTestBot(_location, cls, _GetRandomRaceForClass(cls), level);
    ASSERT_INFO("Creating random test bot with class %u and level %u", uint32(cls), level);
    INTERNAL_TEST_ASSERT(playerBot != nullptr);
    return playerBot;
}

TestPlayer* TestCase::SpawnPlayer(Classes _class, Races _race, uint32 level, Position spawnPosition)
{
    TestPlayer* playerBot = _CreateTestBot(_location, _class, _race, level);
    ASSERT_INFO("Creating random test bot with class %u, race %u and level %u", uint32(_class), uint32(_race), level);
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
TestPlayer* TestCase::_CreateTestBot(WorldLocation loc, Classes cls, Races race, uint32 level)
{
    INTERNAL_TEST_ASSERT(cls != CLASS_NONE && race != RACE_NONE);
    TC_LOG_ERROR("test.unit_test", "Creating new random bot for class %d", cls);

    std::string name = RandomPlayerbotFactory::CreateTestBotName();
    if (name.empty())
        return 0;
    //note that test bots name may sometime overlap with connected bots name... BUT WELL WHATEVER.

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

    session->_HandlePlayerLogin(player, holder);

    player->SetTeleportingToTest(_testMapInstanceId);

    //handle bot position
    bool teleportOK = player->TeleportTo(loc, TELE_TO_GM_MODE);
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
            ASSERT_INFO("_GetRandomClassAndRace: invalid power forcedPower %u", forcedPower);
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
    {
        //get a position in front of default location
        spawnPosition.m_positionX = spawnPosition.m_positionX + 10.0f * std::cos(spawnPosition.m_orientation);
        spawnPosition.m_positionY = spawnPosition.m_positionY + 10.0f * std::sin(spawnPosition.m_orientation);
    }

    return SpawnCreatureWithPosition(spawnPosition, entry);
}

TempSummon* TestCase::SpawnCreatureWithPosition(Position spawnPosition, uint32 entry)
{
    ASSERT_INFO("Test has no map");
    INTERNAL_TEST_ASSERT(GetMap() != nullptr);
    uint32 creatureEntry = entry ? entry : TEST_CREATURE_ENTRY;

    TempSummon* summon = GetMap()->SummonCreature(creatureEntry, spawnPosition);
    ASSERT_INFO("Failed to summon creature with entry %u", creatureEntry);
    INTERNAL_TEST_ASSERT(summon != nullptr);
    summon->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN); //Make sur it does not despawn
    return summon;
}

void TestCase::EquipItem(TestPlayer* player, uint32 itemID)
{
    Item* item = AddItem(player, itemID);
    ASSERT_INFO("Failed to add item %u to player", itemID);
    INTERNAL_TEST_ASSERT(item != nullptr);

    uint16 dest2;
    uint8 msg2 = player->CanEquipItem(NULL_SLOT, dest2, item, !item->IsBag());
    ASSERT_INFO("Player cannot equip item %u, reason: %u", itemID, msg2);
    INTERNAL_TEST_ASSERT(msg2 == EQUIP_ERR_OK);

    player->GetSession()->_HandleAutoEquipItemOpcode(item->GetBagSlot(), item->GetSlot());

    Item* equipedItem = player->GetItemByPos(dest2);
    ASSERT_INFO("Player failed to equip item (dest: %u)", itemID, dest2);
    INTERNAL_TEST_ASSERT(equipedItem != nullptr);
    Wait(1); //not sure this is needed but... let's just wait next update to make sure item spells are properly applied
}

Item* TestCase::AddItem(TestPlayer* player, uint32 itemID)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemID);
    ASSERT_INFO("Item %u does not exists", itemID);
    INTERNAL_TEST_ASSERT(proto != nullptr);

    ItemPosCountVec dest;
    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemID, 1);
    ASSERT_INFO("Player cannot store item %u in bags", itemID);
    INTERNAL_TEST_ASSERT(msg == EQUIP_ERR_OK);

    Item* item = player->StoreNewItem(dest, itemID, true, Item::GenerateItemRandomPropertyId(itemID));
    ASSERT_INFO("Failed to store item %u in bags", itemID);
    INTERNAL_TEST_ASSERT(item != nullptr);
    return item;
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
    ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);
    AI->ResetSpellCounters();

    uint32 maxPredictionError = (expectedMaxDamage - expectedMinDamage) / 25; //arbitary
    //const float confidenceLevel = 99.9f;
    uint32 sampleSize = 500;

    for (uint32 i = 0; i < sampleSize; i++)
    {
        uint32 result = caster->CastSpell(target, spellID, true);
        ASSERT_INFO("Spell casting failed with reason %u", result);
        INTERNAL_TEST_ASSERT(result == SPELL_CAST_OK);
    }

    Wait(10 * SECOND * IN_MILLISECONDS);
    float avgDamageDealt = GetDamagePerSpellsTo(caster, target, spellID);
	TC_LOG_DEBUG("test.unit_test", "spellId: %u -> avgDamageDealt: %f - expectedMinDamage: %u - expectedMaxDamage: %u", spellID, avgDamageDealt, expectedMinDamage, expectedMaxDamage);
    //test hard limits
    ASSERT_INFO("Enforcing higher limit for spell %u. expectedMaxDamage: %u, avgDamageDealt %f", spellID, expectedMaxDamage, avgDamageDealt);
    INTERNAL_TEST_ASSERT(avgDamageDealt <= expectedMaxDamage);
    ASSERT_INFO("Enforcing lower limit for spell %u. expectedMinDamage: %u, avgDamageDealt %f", spellID, expectedMinDamage, avgDamageDealt);
    INTERNAL_TEST_ASSERT(avgDamageDealt >= expectedMinDamage);

    //test if avg is close enough to expected value
    float avgExpected = (expectedMinDamage + expectedMaxDamage) / 2.0f;
    uint32 allowedMin = avgExpected > maxPredictionError ? avgExpected - maxPredictionError : 0;
    uint32 allowedMax = avgExpected + maxPredictionError;

    ASSERT_INFO("Enforcing high average result for spell %u. allowedMax: %u, avgDamageDealt %f", spellID, allowedMax, avgDamageDealt);
    INTERNAL_TEST_ASSERT(avgDamageDealt <= allowedMax);
    ASSERT_INFO("Enforcing low average result for spell %u. allowedMin: %u, avgDamageDealt %f", spellID, allowedMin, avgDamageDealt);
    INTERNAL_TEST_ASSERT(avgDamageDealt >= allowedMin);
}

void TestCase::TestDirectHeal(TestPlayer* caster, Unit* target, uint32 spellID, uint32 expectedHealMin, uint32 expectedHealMax)
{

}

float TestCase::GetDamagePerSpellsTo(TestPlayer* caster, Unit* victim, uint32 spellID)
{
    auto AI = caster->GetTestingPlayerbotAI();
    INTERNAL_TEST_ASSERT(AI != nullptr);

    auto damageToTarget = AI->GetDamageDoneInfo(victim);
    if (damageToTarget.empty())
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo found no data for this victim (%s)", victim->GetName().c_str());
        return 0.0f;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    if (spellInfo == nullptr)
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo was prompted for non existing spell ID %u", spellID);
        return 0.0f;
    }

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

        totalDamage += damage;
        count++;
    }

    if (count == 0)
    {
        TC_LOG_WARN("test.unit_test", "GetDamagePerSpellsTo was prompted for a victim (%s) with no valid data for this spell (ID %u)", victim->GetName().c_str(), spellID);
        return 0.0f;
    }

    return totalDamage / count;
}

void TestCase::TestDotDamage(TestPlayer* caster, Unit* target, uint32 spellID, int32 expectedAmount)
{
    auto AI = caster->GetTestingPlayerbotAI();
    ASSERT_INFO("Caster in not a testing bot");
    INTERNAL_TEST_ASSERT(AI != nullptr);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellID);
    ASSERT_INFO("Spell %u does not exists", spellID);
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
            ASSERT_INFO("Target still has %u aura after %u ms", spellID, waitTime);
            INTERNAL_TEST_ASSERT(!target->HasAuraWithCaster(spellID, 0, caster->GetGUID()));

            int32 dotDamageToTarget = AI->GetDotDamage(target, spellID);
			TC_LOG_DEBUG("test.unit_test", "spellId: %u -> dotDamageToTarget: %i - expectedAmount: %i", spellID, dotDamageToTarget, expectedAmount);
            ASSERT_INFO("Enforcing dot damage. dotDamageToTarget: %i, expectedAmount: %i", dotDamageToTarget, expectedAmount);
            TEST_ASSERT(dotDamageToTarget >= (expectedAmount - 1) && dotDamageToTarget <= (expectedAmount + 1));
            return;
        }
    }
    INTERNAL_TEST_ASSERT((false && "Failed to cast the spell 100 times")); //failed to cast the spell 100 times
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

void TestCase::DisableRegen(TestPlayer* caster)
{
    caster->DisableRegen(true);
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