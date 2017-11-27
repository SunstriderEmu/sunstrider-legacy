
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Transport.h"
#include "GridDefines.h"
#include "MapInstanced.h"
#include "World.h"
#include "CellImpl.h"
#include "Corpse.h"
#include "ObjectMgr.h"
#include "GridMap.h"

#define TEST_MAP_STARTING_ID 10000

MapManager::MapManager() : 
    _nextInstanceId(0), _scheduledScripts(0)
{
    i_gridCleanUpDelay = 5 * MINUTE * IN_MILLISECONDS ; //sWorld->getIntConfig(CONFIG_INTERVAL_GRIDCLEAN);
    i_timer.SetInterval(sWorld->getConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{ }

void MapManager::Initialize()
{
    Map::InitStateMachine();

    int num_threads(sWorld->getIntConfig(CONFIG_NUMTHREADS));
    // Start mtmaps if needed.
    if (num_threads > 0)
        m_updater.activate(num_threads);
}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (auto & i_map : i_maps)
        i_map.second->InitVisibilityDistance();
}

Map* MapManager::CreateBaseMap(uint32 id, bool testing)
{
    Map* m = FindBaseMap(id, testing);

    if( m == nullptr )
    {
        std::lock_guard<std::mutex> lock(_mapsLock);

        const MapEntry* entry = sMapStore.LookupEntry(id);
        if (!entry)
            return nullptr; //unknown map

        if (entry->Instanceable() || testing)
        {
            m = new MapInstanced(id, i_gridCleanUpDelay);
        }
        else
        {
            m = new Map(MAP_TYPE_MAP, id, i_gridCleanUpDelay, 0, REGULAR_DIFFICULTY);
            //TC map->LoadRespawnTimes();
            m->LoadCorpseData();
        }
        uint32 internal_id = testing ? id + TEST_MAP_STARTING_ID : id;
        i_maps[internal_id] = m;
    }

    assert(m != nullptr);
    return m;
}

Map* MapManager::FindBaseNonInstanceMap(uint32 mapId) const
{
    Map* map = FindBaseMap(mapId);
    if (map && map->Instanceable())
        return nullptr;
    return map;
}

Map* MapManager::FindBaseMap(uint32 id, bool testing) const
{
    uint32 internal_id = testing ? id + TEST_MAP_STARTING_ID : id;
    auto iter = i_maps.find(internal_id);
    return (iter == i_maps.end() ? nullptr : iter->second);
}

Map* MapManager::CreateMap(uint32 id, Player* player, uint32 loginInstanceId)
{
    Map* m = CreateBaseMap(id);

    if (player && player->GetTeleportingToTest()) //testing code
    {
        uint32 teleportToTestInstance = player->GetTeleportingToTest();
        uint32 internal_id = id + TEST_MAP_STARTING_ID;
        auto iter = i_maps.find(internal_id);
        m = iter != i_maps.end() ? iter->second : nullptr;
        if (m)
        {
            ASSERT(m->GetMapType() == MAP_TYPE_MAP_INSTANCED);
            m = static_cast<MapInstanced*>(m)->FindInstanceMap(teleportToTestInstance);
            if (m == nullptr)
                return nullptr;
            ASSERT(m->GetId() == id);
        }
    } else if (m && m->Instanceable())
        m = ((MapInstanced*)m)->CreateInstanceForPlayer(id, player, loginInstanceId);

    return m;
}

Map* MapManager::FindMap(uint32 mapid, uint32 instanceId)
{
    Map *map = FindBaseMap(mapid);
    if(!map) 
        return nullptr;

    if(!map->Instanceable()) 
        return instanceId == 0 ? map : nullptr;

    return ((MapInstanced*)map)->FindInstanceMap(instanceId);
}

Map::EnterState MapManager::PlayerCannotEnter(uint32 mapid, Player* player, bool loginCheck)
{
    MapEntry const* entry = sMapStore.LookupEntry(mapid);
    if (!entry)
        return Map::CANNOT_ENTER_NO_ENTRY;

    if (!entry->IsDungeon() || player->GetTeleportingToTest())
        return Map::CAN_ENTER;

    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(mapid);
    if (!instance)
        return Map::CANNOT_ENTER_UNINSTANCED_DUNGEON;

    Difficulty targetDifficulty, requestedDifficulty;
    targetDifficulty = requestedDifficulty = player->GetDifficulty(entry->IsRaid());
    // Get the highest available difficulty if current setting is higher than the instance allows
    MapDifficulty const* mapDiff = GetDownscaledMapDifficultyData(entry->MapID, targetDifficulty);
    if (!mapDiff)
        return Map::CANNOT_ENTER_DIFFICULTY_UNAVAILABLE;

    //Bypass checks for GMs
    if (player->IsGameMaster())
        return Map::CAN_ENTER;

    char const* mapName = entry->name[player->GetSession()->GetSessionDbcLocale()];

    Group* group = player->GetGroup();
    if (entry->IsRaid()) // can only enter in a raid group
        if ((!group || !group->isRaidGroup()) && !sWorld->getBoolConfig(CONFIG_INSTANCE_IGNORE_RAID))
            return Map::CANNOT_ENTER_NOT_IN_RAID;

    if (!player->IsAlive())
    {
        if (player->HasCorpse())
        {
            // let enter in ghost mode in instance that connected to inner instance with corpse
            uint32 corpseMap = player->GetCorpseLocation().GetMapId();
            do
            {
                if (corpseMap == mapid)
                    break;

                InstanceTemplate const* corpseInstance = sObjectMgr->GetInstanceTemplate(corpseMap);
                corpseMap = corpseInstance ? corpseInstance->parent : 0;
            } while (corpseMap);

            if (!corpseMap)
                return Map::CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE;

            TC_LOG_DEBUG("maps", "MAP: Player '%s' has corpse in instance '%s' and can enter.", player->GetName().c_str(), mapName);
        }
        else
            TC_LOG_DEBUG("maps", "Map::PlayerCannotEnter - player '%s' is dead but does not have a corpse!", player->GetName().c_str());
    }

    //Get instance where player's group is bound & its map
    if (!loginCheck && group)
    {
        InstanceGroupBind* boundInstance = group->GetBoundInstance(entry);
        if (boundInstance && boundInstance->save)
            if (Map* boundMap = sMapMgr->FindMap(mapid, boundInstance->save->GetInstanceId()))
                if (Map::EnterState denyReason = boundMap->CannotEnter(player))
                    return denyReason;
    }

    // players are only allowed to enter 5 instances per hour
    if (entry->IsDungeon() && (!player->GetGroup() || (player->GetGroup() && !player->GetGroup()->isLFGGroup())))
    {
        uint32 instanceIdToCheck = 0;
        if (InstanceSave* save = player->GetInstanceSave(mapid, entry->IsRaid()))
            instanceIdToCheck = save->GetInstanceId();

        // instanceId can never be 0 - will not be found
        /* TC
        if (!player->CheckInstanceCount(instanceIdToCheck) && !player->IsDead())
            return Map::CANNOT_ENTER_TOO_MANY_INSTANCES;
            */
    }

    //Other requirements
    //TC if (player->Satisfy(sObjectMgr->GetAccessRequirement(mapid, targetDifficulty), mapid, true))
    if (player->Satisfy(sObjectMgr->GetAccessRequirement(instance->access_id), mapid, true))
        return Map::CAN_ENTER;
    else
        return Map::CANNOT_ENTER_UNSPECIFIED_REASON;
}

void MapManager::Update(time_t diff)
{
    i_timer.Update(diff);
    if( !i_timer.Passed() )
        return;

    for (auto & i_map : i_maps)
    {
        if (m_updater.activated())
            m_updater.schedule_update(*i_map.second, uint32(i_timer.GetCurrent()));
        else
            i_map.second->DoUpdate(uint32(i_timer.GetCurrent()));
    }

    if (m_updater.activated())
    {
        /* We keep instances updates looping while continents are updated.
        Once all continents are done, we wait for the current instances updates to finish and stop.
        */
        m_updater.enableUpdateLoop(true);
        m_updater.waitUpdateOnces();
        m_updater.enableUpdateLoop(false);
        m_updater.waitUpdateLoops();
    }

    //delayed map updates
    for (auto & i_map : i_maps)
        i_map.second->DelayedUpdate(uint32(i_timer.GetCurrent()));

    i_timer.SetCurrent(0);
}

bool MapManager::ExistMapAndVMap(uint32 mapid, float x,float y)
{
    GridCoord p = Trinity::ComputeGridCoord(x,y);

    int gx = (MAX_NUMBER_OF_GRIDS - 1) - p.x_coord;
    int gy = (MAX_NUMBER_OF_GRIDS - 1) - p.y_coord;

    return GridMap::ExistMap(mapid,gx,gy) && GridMap::ExistVMap(mapid,gx,gy);
}

bool MapManager::IsValidMAP(uint32 mapid, bool startUp)
{
    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);
    if (startUp)
        return mEntry ? true : false;
    else
        return mEntry && (!mEntry->Instanceable() || sObjectMgr->GetInstanceTemplate(mapid));

    /// @todo add check for battleground template
}

void MapManager::UnloadAll()
{
    for (auto iter = i_maps.begin(); iter != i_maps.end();)
    {
        iter->second->UnloadAll();
        delete iter->second;
        i_maps.erase(iter++);
    }

    if (m_updater.activated())
        m_updater.deactivate();

    Map::DeleteStateMachine();
}

TestMap* MapManager::CreateTestMap(uint32 mapid, uint32& testMapInstanceId, Difficulty diff, bool enableMapObjects)
{
    MapInstanced* mapInstanced = static_cast<MapInstanced*>(CreateBaseMap(mapid, true));
    if (!mapInstanced)
        return nullptr;

    testMapInstanceId = sMapMgr->GenerateInstanceId();
    TestMap* testMap = mapInstanced->CreateTestInsteance(testMapInstanceId, diff, enableMapObjects);
    return testMap;
}

bool MapManager::UnloadTestMap(uint32 mapId, uint32 instanceId)
{
    uint32 internalId = TEST_MAP_STARTING_ID + mapId;
    auto itr = i_maps.find(internalId);
    if (itr == i_maps.end())
        return false;

    MapInstanced* map = dynamic_cast<MapInstanced*>(itr->second);
    ASSERT(map != nullptr);
    Map* _testMap = map->FindInstanceMap(instanceId);
    if (!_testMap)
    {
        TC_LOG_ERROR("test.unit_test", "Failed to delete map %u because it was not found", instanceId);
        return false;
    }
    if (_testMap->GetMapType() != MAP_TYPE_TEST_MAP)
    {
        TC_LOG_ERROR("test.unit_test", "Failed to delete map %u because it is not a test map!", instanceId);
        return false;
    }
    TestMap* testMap = static_cast<TestMap*>(_testMap);

    testMap->DisconnectAllBots(); //This will delete players objects
    testMap->RemoveAllPlayers();
    //testMap should trigger unload when all players have left
    return true;
}

uint32 MapManager::GetNumInstances()
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    uint32 ret = 0;
    for(auto & i_map : i_maps)
    {
        Map *map = i_map.second;
        if(!map->Instanceable()) 
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced *)map)->GetInstancedMaps();
        for(auto & map : maps)
            if(map.second->IsDungeon()) ret++;
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    uint32 ret = 0;
    for(auto & i_map : i_maps)
    {
        Map *map = i_map.second;
        if(!map->Instanceable()) 
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced *)map)->GetInstancedMaps();
        for(auto & map : maps)
            if(map.second->IsDungeon())
                ret += ((InstanceMap*)map.second)->GetPlayers().getSize();
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInMap(uint32 mapId)
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    uint32 ret = 0;

    for (auto & i_map : i_maps)
    {
        Map* map = i_map.second;

        if (map->GetId() != mapId)
            continue;

        if (!map->Instanceable()) {
            ret = map->GetPlayers().getSize();
            break;
        } else {
            MapInstanced::InstancedMaps& maps = ((MapInstanced *)map)->GetInstancedMaps();
            for (auto & map : maps)
                ret += ((InstanceMap *)map.second)->GetPlayers().getSize();
        }
    }

    return ret;
}


void MapManager::InitInstanceIds()
{
    _nextInstanceId = 1;

    QueryResult result = CharacterDatabase.Query("SELECT MAX(id) FROM instance");
    if (result)
    {
        uint32 maxId = (*result)[0].GetUInt32();

        // Resize to multiples of 32 (vector<bool> allocates memory the same way)
        _instanceIds.resize((maxId / 32) * 32 + (maxId % 32 > 0 ? 32 : 0));
    }
}

void MapManager::RegisterInstanceId(uint32 instanceId)
{
    // Allocation and sizing was done in InitInstanceIds()
    _instanceIds[instanceId] = true;
}

uint32 MapManager::GenerateInstanceId()
{
    uint32 newInstanceId = _nextInstanceId;

    // Find the lowest available id starting from the current NextInstanceId (which should be the lowest according to the logic in FreeInstanceId()
    for (uint32 i = ++_nextInstanceId; i < 0xFFFFFFFF; ++i)
    {
        if ((i < _instanceIds.size() && !_instanceIds[i]) || i >= _instanceIds.size())
        {
            _nextInstanceId = i;
            break;
        }
    }

    if (newInstanceId == _nextInstanceId)
    {
        TC_LOG_ERROR("maps", "Instance ID overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }

    // Allocate space if necessary
    if (newInstanceId >= uint32(_instanceIds.size()))
    {
        // Due to the odd memory allocation behavior of vector<bool> we match size to capacity before triggering a new allocation
        if (_instanceIds.size() < _instanceIds.capacity())
        {
            _instanceIds.resize(_instanceIds.capacity());
        }
        else
            _instanceIds.resize(1 + (newInstanceId / 32) * 32 + (newInstanceId % 32 > 0 ? 32 : 0)); //sunstrider: +1 to handle newInstanceId == 32 case. No idea how this can work on trinitycore
    }

    _instanceIds[newInstanceId] = true;

    return newInstanceId;
}

void MapManager::MapCrashed(Map& map)
{
    InstanceMap* instanceMap = dynamic_cast<InstanceMap*>(&map);
    if (!instanceMap)
    {
        std::cerr << "MapManager::MapCrashed could not convert crashed map to InstanceMap" << std::endl;
        return;
    }

    //find right MapInstanced to pass crashed map to it and let it handle it
    Map* baseMap = FindBaseMap(map.GetId());
    if(!baseMap) {
        std::cerr << "MapManager::MapCrashed could not find basemap for id " << map.GetId() << std::endl;
        return;
    }

    MapInstanced* mapInstanced = dynamic_cast<MapInstanced*>(baseMap);
    if (!mapInstanced)
    {
        std::cerr << "MapManager::MapCrashed could not convert map iterator to MapInstanced" << std::endl;
        return;
    }

    mapInstanced->MapCrashed(instanceMap);
}

void MapManager::FreeInstanceId(uint32 instanceId)
{
    // If freed instance id is lower than the next id available for new instances, use the freed one instead
    if (instanceId < _nextInstanceId)
        SetNextInstanceId(instanceId);

    _instanceIds[instanceId] = false;
 //LK   sAchievementMgr->OnInstanceDestroyed(instanceId);
}

uint32 MapManager::GetAreaId(uint32 mapid, float x, float y, float z) const
{
    Map const* m = const_cast<MapManager*>(this)->CreateBaseMap(mapid);
    return m->GetAreaId(x, y, z);
}
uint32 MapManager::GetZoneId(uint32 mapid, float x, float y, float z) const
{
    Map const* m = const_cast<MapManager*>(this)->CreateBaseMap(mapid);
    return m->GetZoneId(x, y, z);
}
void MapManager::GetZoneAndAreaId(uint32& zoneid, uint32& areaid, uint32 mapid, float x, float y, float z)
{
    Map const* m = const_cast<MapManager*>(this)->CreateBaseMap(mapid);
    m->GetZoneAndAreaId(zoneid, areaid, x, y, z);
}

bool MapManager::IsValidMapCoord(WorldLocation const& loc)
{
    return IsValidMapCoord(loc.m_mapId, loc.m_positionX, loc.m_positionY, loc.m_positionZ, loc.m_orientation);
}
