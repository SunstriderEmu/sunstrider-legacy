/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <omp.h>
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

MapManager::MapManager() : 
    _nextInstanceId(0)
{
    i_timer.SetInterval(sWorld->getConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{ }

void MapManager::Initialize()
{
    int num_threads(sWorld->getIntConfig(CONFIG_NUMTHREADS));
    // Start mtmaps if needed.
    if (num_threads > 0)
        m_updater.activate(num_threads);
}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (MapMapType::iterator iter = i_maps.begin(); iter != i_maps.end(); ++iter)
        (*iter).second->InitVisibilityDistance();
}

Map* MapManager::CreateBaseMap(uint32 id)
{
    Map *m = FindBaseMap(id);

    if( m == NULL )
    {
        std::lock_guard<std::mutex> lock(_mapsLock);

        const MapEntry* entry = sMapStore.LookupEntry(id);
        if (entry && entry->Instanceable())
        {
            m = new MapInstanced(id);
        }
        else
        {
            m = new Map(id, 0, REGULAR_DIFFICULTY);
        }
        i_maps[id] = m;
    }

    assert(m != nullptr);
    return m;
}

Map* MapManager::FindBaseNonInstanceMap(uint32 mapId) const
{
    Map* map = FindBaseMap(mapId);
    if (map && map->Instanceable())
        return NULL;
    return map;
}

Map* MapManager::CreateMap(uint32 id, const WorldObject* obj)
{
    Map *m = CreateBaseMap(id);

    if (m && obj && m->Instanceable()) 
        m = ((MapInstanced*)m)->GetInstance(obj);

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

/*
    checks that do not require a map to be created
    will send transfer error messages on fail
*/
bool MapManager::CanPlayerEnter(uint32 mapid, Player* player)
{
    const MapEntry *entry = sMapStore.LookupEntry(mapid);
    if(!entry) return false;
    const char *mapName = entry->name[player->GetSession()->GetSessionDbcLocale()];

    if(entry->map_type == MAP_INSTANCE || entry->map_type == MAP_RAID)
    {
        if (entry->map_type == MAP_RAID)
        {
            // GMs can avoid raid limitations
            if((player->GetSession()->GetSecurity() == SEC_PLAYER) /* && player->GetSession()->GetGroupId() == 0 */&& !sWorld->getConfig(CONFIG_INSTANCE_IGNORE_RAID))
            {
                // can only enter in a raid group
                Group* group = player->GetGroup();
                if (!group || !group->isRaidGroup())
                {
                    // probably there must be special opcode, because client has this string constant in GlobalStrings.lua
                    // TODO: this is not a good place to send the message
                    player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetTrinityString(810), mapName);
                    return false;
                }
            }
        }

        Difficulty targetDifficulty, requestedDifficulty;
        targetDifficulty = requestedDifficulty = player->GetDifficulty(entry->IsRaid());
        // Get the highest available difficulty if current setting is higher than the instance allows
        MapDifficulty const* mapDiff = GetDownscaledMapDifficultyData(entry->MapID, targetDifficulty);
        if (!mapDiff)
        {
            player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);      //Send aborted message
            return false;
        }

        if (!player->IsAlive())
        {
            if(Corpse *corpse = player->GetCorpse())
            {
                // let enter in ghost mode in instance that connected to inner instance with corpse
                uint32 instance_map = corpse->GetMapId();
                do
                {
                    if(instance_map==mapid)
                        break;

                    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(instance_map);
                    instance_map = instance ? instance->parent : 0;
                }
                while (instance_map);

                if (!instance_map)
                {
                    player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetTrinityString(811), mapName);
                    return false;
                }
                //TC_LOG_DEBUG("FIXME","MAP: Player '%s' has corpse in instance '%s' and can enter", player->GetName(), mapName);
                player->ResurrectPlayer(0.5f, false);
                player->SpawnCorpseBones();
            }
            else
            {
                //TC_LOG_ERROR("FIXME","Map::CanEnter - player '%s' is dead but doesn't have a corpse!", player->GetName());
            }
        }

        // Requirements
        InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(mapid);
        if(!instance)
            return false;
        
        return player->Satisfy(sObjectMgr->GetAccessRequirement(instance->access_id), mapid, true);
    }
    else
        return true;
}

void MapManager::RemoveBonesFromMap(uint32 mapid, uint64 guid, float x, float y)
{
    CreateBaseMap(mapid)->RemoveBones(guid, x, y);
}

void MapManager::Update(time_t diff)
{
    i_timer.Update(diff);
    if( !i_timer.Passed() )
        return;

    MapMapType::iterator iter = i_maps.begin();
    for (; iter != i_maps.end(); ++iter)
    {
        if (m_updater.activated())
            m_updater.schedule_update(*iter->second, uint32(i_timer.GetCurrent()));
        else
            iter->second->Update(uint32(i_timer.GetCurrent()));
    }

    if (m_updater.activated())
        m_updater.wait();

    sObjectAccessor->Update(i_timer.GetCurrent());
    sWorld->RecordTimeDiff("UpdateObjectAccessor");

    i_timer.SetCurrent(0);
}

bool MapManager::ExistMapAndVMap(uint32 mapid, float x,float y)
{
    GridCoord p = Trinity::ComputeGridPair(x,y);

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
    for (MapMapType::iterator iter = i_maps.begin(); iter != i_maps.end();)
    {
        iter->second->UnloadAll();
        delete iter->second;
        i_maps.erase(iter++);
    }

    if (m_updater.activated())
        m_updater.deactivate();
}

uint32 MapManager::GetNumInstances()
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    uint32 ret = 0;
    for(MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map *map = itr->second;
        if(!map->Instanceable()) 
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced *)map)->GetInstancedMaps();
        for(MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
            if(mitr->second->IsDungeon()) ret++;
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    uint32 ret = 0;
    for(MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map *map = itr->second;
        if(!map->Instanceable()) 
            continue;
        MapInstanced::InstancedMaps &maps = ((MapInstanced *)map)->GetInstancedMaps();
        for(MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
            if(mitr->second->IsDungeon())
                ret += ((InstanceMap*)mitr->second)->GetPlayers().getSize();
    }
    return ret;
}

uint32 MapManager::GetNumPlayersInMap(uint32 mapId)
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    uint32 ret = 0;

    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map* map = itr->second;

        if (map->GetId() != mapId)
            continue;

        if (!map->Instanceable()) {
            ret = map->GetPlayers().getSize();
            break;
        } else {
            MapInstanced::InstancedMaps& maps = ((MapInstanced *)map)->GetInstancedMaps();
            for (MapInstanced::InstancedMaps::iterator mitr = maps.begin(); mitr != maps.end(); ++mitr)
                ret += ((InstanceMap *)mitr->second)->GetPlayers().getSize();
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
            _instanceIds.resize((newInstanceId / 32) * 32 + (newInstanceId % 32 > 0 ? 32 : 0));
    }

    _instanceIds[newInstanceId] = true;

    return newInstanceId;
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