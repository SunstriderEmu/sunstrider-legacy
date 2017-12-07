
#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "BattleGround.h"
#include "Management/VMapFactory.h"
#include "Management/MMapFactory.h"
#include "InstanceSaveMgr.h"
#include "World.h"
#include "DBCStores.h"

MapInstanced::MapInstanced(uint32 id, time_t expiry) : Map(MAP_TYPE_MAP_INSTANCED, id, expiry, 0, DUNGEON_DIFFICULTY_NORMAL)
{
    // fill with zero
    memset(&GridMapReference, 0, MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS*sizeof(uint16));
}

void MapInstanced::InitVisibilityDistance()
{
    if (m_InstancedMaps.empty())
        return;
    //initialize visibility distances for all instance copies
    for (auto & m_InstancedMap : m_InstancedMaps)
    {
        m_InstancedMap.second->InitVisibilityDistance();
    }
}

void MapInstanced::Update(const uint32& t)
{
    // take care of loaded GridMaps (when unused, unload it!)
    Map::Update(t);

    // update the instanced maps
    auto i = m_InstancedMaps.begin();
    while (i != m_InstancedMaps.end())
    {
        if(i->second->CanUnload(t))
        {
            if (!DestroyInstance(i))                             // iterator incremented
            {
                //m_unloadTimer
            }
        }
        else
        {
            // update only here, because it may schedule some bad things before delete
            if (sMapMgr->GetMapUpdater()->activated())
                sMapMgr->GetMapUpdater()->schedule_update(*i->second, t);
            else
                i->second->Update(t);

            ++i;
        }
    }

    //crash recovery
    for (auto crashedMap : crashedMaps)
    {
        crashedMap->HandleCrash();
        //remove it from map list. Do not clear memory (map memory may be corrupted)
        for (auto iter = m_InstancedMaps.begin(); iter != m_InstancedMaps.end(); iter++)
        {
            if (iter->second == crashedMap)
            {
                TC_LOG_FATAL("mapcrash", "Crashed map remove");
                m_InstancedMaps.erase(iter);
                break;
            }
        }
    }
    crashedMaps.clear();

}

void MapInstanced::DelayedUpdate(const uint32 diff)
{
    for (auto & m_InstancedMap : m_InstancedMaps)
        m_InstancedMap.second->DelayedUpdate(diff);

    Map::DelayedUpdate(diff);
}

void MapInstanced::MapCrashed(Map* map)
{
    TC_LOG_FATAL("mapcrash", "Prevented crash in map updater. Map: %u - InstanceId: %u", map->GetId(), map->GetInstanceId());
    std::cerr << "Prevented crash in map updater. Map: " << map->GetId() << " - InstanceId: " << map->GetInstanceId() << std::endl;

    //backtrace is generated in the signal handler in Main.cpp (unix only)

    crashedMaps.push_back(map);
}

/*
bool MapInstanced::RemoveBones(uint64 guid, float x, float y)
{
    bool remove_result = false;

    for (auto & m_InstancedMap : m_InstancedMaps)
    {
        remove_result = remove_result || m_InstancedMap.second->RemoveBones(guid, x, y);
    }

    return remove_result || Map::RemoveBones(guid,x,y);
}
*/

void MapInstanced::UnloadAll()
{
    // Unload instanced maps
    for (auto & m_InstancedMap : m_InstancedMaps)
        m_InstancedMap.second->UnloadAll();

    // Delete the maps only after everything is unloaded to prevent crashes
    for (auto & m_InstancedMap : m_InstancedMaps)
        delete m_InstancedMap.second;

    m_InstancedMaps.clear();

    // Unload own grids (just dummy(placeholder) grids, neccesary to unload GridMaps!)
    Map::UnloadAll();
}

/*
- return the right instance for the object, based on its InstanceId.
- If InstanceId == 0, get it from save or generate a new one
- create the instance if it's not created already
- the player is not actually added to the instance (only in InstanceMap::Add)
*/
Map* MapInstanced::CreateInstanceForPlayer(const uint32 mapId, Player* player, uint32 loginInstanceId)
{
    if (GetId() != mapId || !player)
        return nullptr;

    Map* map = nullptr;
    uint32 newInstanceId = 0;                       // instanceId of the resulting map
    
    //uint32 instanceId = player->GetInstanceId();

    if (IsBattlegroundOrArena())
    {
        newInstanceId = player->GetBattlegroundId();
        if (!newInstanceId)
            return nullptr;

        map = sMapMgr->FindMap(mapId, newInstanceId);
        if(!map)
        {
            if (Battleground* bg = player->GetBattleground())
                return CreateBattleground(newInstanceId, bg);
            else
            {
                player->TeleportToBGEntryPoint();
                return nullptr;
            }
        }
    }
    else
    {
        InstancePlayerBind* pBind = player->GetBoundInstance(GetId(), player->GetDifficulty(IsRaid()));
        InstanceSave* pSave = pBind ? pBind->save : nullptr;

        // priority:
        // 1. player's permanent bind
        // 2. player's current instance id if this is at login
        // 3. group's current bind
        // 4. player's current bind
        if (!pBind || !pBind->perm)
        {
            if (loginInstanceId) // if the player has a saved instance id on login, we either use this instance or relocate him out (return null)
            {
                //sunstrider: logic changed a bit here: we don't want to relocate player out of instance after a crash
                map = FindInstanceMap(loginInstanceId);
                if (map)
                    return map->GetId() == GetId() ? map : nullptr;
                else if (pSave && pSave->GetInstanceId() == loginInstanceId) //else create map if a save exists
                    return CreateInstance(loginInstanceId, pSave, pSave->GetDifficulty());
                else
                    return nullptr; //relocate him out
            }

            InstanceGroupBind* groupBind = nullptr;
            Group* group = player->GetGroup();
            // use the player's difficulty setting (it may not be the same as the group's)
            if (group)
            {
                groupBind = group->GetBoundInstance(this);
                if (groupBind)
                {
                    // solo saves should be reset when entering a group's instance
                    player->UnbindInstance(GetId(), player->GetDifficulty(IsRaid()));
                    pSave = groupBind->save;
                }
            }
        }
        if (pSave)
        {
            // solo/perm/group
            newInstanceId = pSave->GetInstanceId();
            map = FindInstanceMap(newInstanceId);
            // it is possible that the save exists but the map doesn't
            if (!map)
                map = CreateInstance(newInstanceId, pSave, pSave->GetDifficulty());
        }
        else
        {
            // if no instanceId via group members or instance saves is found
            // the instance will be created for the first time
            newInstanceId = sMapMgr->GenerateInstanceId();

            Difficulty diff = player->GetGroup() ? player->GetGroup()->GetDifficulty(IsRaid()) : player->GetDifficulty(IsRaid());
            //Seems it is now possible, but I do not know if it should be allowed
            //ASSERT(!FindInstanceMap(NewInstanceId));
            map = FindInstanceMap(newInstanceId);
            if (!map)
                map = CreateInstance(newInstanceId, NULL, diff);
        }
    }

    return map;
}

Map* MapInstanced::FindInstanceMap(uint32 InstanceId)
{
    auto i = m_InstancedMaps.find(InstanceId);
    return i == m_InstancedMaps.end() ? nullptr : i->second;
}

TestMap* MapInstanced::CreateTestInsteance(uint32 instanceId, Difficulty difficulty, bool enableMapObjects)
{
    // load/create a map
    std::lock_guard<std::mutex> lock(_mapLock);

    // make sure we have a valid map id
    const MapEntry* entry = sMapStore.LookupEntry(GetId());
    if (!entry)
    {
        TC_LOG_ERROR("maps", "CreateInstance: no entry for map %d", GetId());
        ABORT();
    }

    // some instances only have one difficulty
    GetDownscaledMapDifficultyData(GetId(), difficulty);

    TC_LOG_DEBUG("maps", "MapInstanced::CreateTestInsteance: map instance %d for %d created with difficulty %s", instanceId, GetId(), difficulty ? "heroic" : "normal");

    TestMap* map = new TestMap(GetId(), instanceId, difficulty, this, enableMapObjects);

    map->CreateInstanceData(false);

    m_InstancedMaps[instanceId] = map;
    return map;
}

InstanceMap* MapInstanced::CreateInstance(uint32 instanceId, InstanceSave* save, Difficulty difficulty)
{
    // load/create a map
    std::lock_guard<std::mutex> lock(_mapLock);

    // make sure we have a valid map id
    const MapEntry* entry = sMapStore.LookupEntry(GetId());
    if(!entry)
    {
        TC_LOG_ERROR("maps","CreateInstance: no entry for map %d", GetId());
        ABORT();
    }
    const InstanceTemplate* iTemplate = sObjectMgr->GetInstanceTemplate(GetId());
    if(!iTemplate)
    {
        TC_LOG_ERROR("maps","CreateInstance: no instance template for map %d", GetId());
        ABORT();
    }

    // some instances only have one difficulty
    GetDownscaledMapDifficultyData(GetId(), difficulty);

    TC_LOG_DEBUG("maps", "MapInstanced::CreateInstance: %s map instance %d for %d created with difficulty %s", save ? "" : "new ", instanceId, GetId(), difficulty ? "heroic" : "normal");

    InstanceMap* map = new InstanceMap(GetId(), GetGridExpiry(), instanceId, difficulty, this);

    assert(map->IsDungeon());

    map->LoadRespawnTimes();
    map->LoadCorpseData();

    bool load_data = save != nullptr;
    map->CreateInstanceData(load_data);

    m_InstancedMaps[instanceId] = map;
    return map;
}

BattlegroundMap* MapInstanced::CreateBattleground(uint32 InstanceId, Battleground* bg)
{
    // load/create a map
    std::lock_guard<std::mutex> lock(_mapLock);

    TC_LOG_DEBUG("maps", "MapInstanced::CreateBattleground: map bg %d for %d created.", InstanceId, GetId());

    auto map = new BattlegroundMap(GetId(), GetGridExpiry(), InstanceId, this);
    assert(map->IsBattlegroundOrArena());
    map->SetBG(bg);
    bg->SetBgMap(map);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

bool MapInstanced::DestroyInstance(uint32 InstanceId)
{
    auto itr = m_InstancedMaps.find(InstanceId);
    if(itr != m_InstancedMaps.end())
       return DestroyInstance(itr);

    return false;
}

// increments the iterator after erase
bool MapInstanced::DestroyInstance(InstancedMaps::iterator &itr)
{
    itr->second->RemoveAllPlayers();
    if (itr->second->HavePlayers())
    {
        ++itr;
        return false;
    }

    itr->second->UnloadAll();
    // should only unload VMaps if this is the last instance and grid unloading is enabled
    if(m_InstancedMaps.size() <= 1 && sWorld->getConfig(CONFIG_GRID_UNLOAD))
    {
        VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(itr->second->GetId());
        MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(itr->second->GetId());
        // in that case, unload grids of the base map, too
        // so in the next map creation, (EnsureGridCreated actually) VMaps will be reloaded
        Map::UnloadAll();
    }

    // Free up the instance id and allow it to be reused for bgs and arenas (other instances are handled in the InstanceSaveMgr)
    if (itr->second->IsBattlegroundOrArena())
        sMapMgr->FreeInstanceId(itr->second->GetInstanceId());

    // erase map
    delete itr->second;
    m_InstancedMaps.erase(itr++);

    return true;
}

Map::EnterState MapInstanced::CannotEnter(Player* /*player*/)
{
    //ABORT();
    return CAN_ENTER;
}