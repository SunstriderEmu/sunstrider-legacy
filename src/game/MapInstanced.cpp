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

#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "BattleGround.h"
#include "Management/VMapFactory.h"
#include "Management/MMapFactory.h"
#include "InstanceSaveMgr.h"
#include "World.h"

MapInstanced::MapInstanced(uint32 id) : Map(id, 0, DUNGEON_DIFFICULTY_NORMAL)
{
    // fill with zero
    memset(&GridMapReference, 0, MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_GRIDS*sizeof(uint16));
}

void MapInstanced::InitVisibilityDistance()
{
    if (m_InstancedMaps.empty())
        return;
    //initialize visibility distances for all instance copies
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        (*i).second->InitVisibilityDistance();
    }
}

void MapInstanced::Update(const uint32& t)
{
    // take care of loaded GridMaps (when unused, unload it!)
    Map::Update(t);

    // update the instanced maps
    InstancedMaps::iterator i = m_InstancedMaps.begin();

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
}

void MapInstanced::MoveAllCreaturesInMoveList()
{
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        i->second->MoveAllCreaturesInMoveList();
    }

    Map::MoveAllCreaturesInMoveList();
}

void MapInstanced::RemoveAllObjectsInRemoveList()
{
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        i->second->RemoveAllObjectsInRemoveList();
    }

    Map::RemoveAllObjectsInRemoveList();
}

bool MapInstanced::RemoveBones(uint64 guid, float x, float y)
{
    bool remove_result = false;

    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
    {
        remove_result = remove_result || i->second->RemoveBones(guid, x, y);
    }

    return remove_result || Map::RemoveBones(guid,x,y);
}

void MapInstanced::UnloadAll()
{
    // Unload instanced maps
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        i->second->UnloadAll();

    // Delete the maps only after everything is unloaded to prevent crashes
    for (InstancedMaps::iterator i = m_InstancedMaps.begin(); i != m_InstancedMaps.end(); ++i)
        delete i->second;

    m_InstancedMaps.clear();

    // Unload own grids (just dummy(placeholder) grids, neccesary to unload GridMaps!)
    Map::UnloadAll();
}

/*
- return the right instance for the object, based on its InstanceId
- create the instance if it's not created already
- the player is not actually added to the instance (only in InstanceMap::Add)
*/
Map* MapInstanced::GetInstance(const WorldObject* obj)
{
    if (obj->GetTypeId() != TYPEID_PLAYER)
    {
        assert(obj->GetMapId() == GetId() && obj->GetInstanceId());
        return FindInstanceMap(obj->GetInstanceId());
    }

    Player *player = const_cast<Player*>(obj->ToPlayer());
    uint32 instanceId = player->GetInstanceId();

    if (instanceId)
    {
        if (Map *map = FindInstanceMap(instanceId))
            return map;
    }

    if (IsBattlegroundOrArena())
    {
        instanceId = player->GetBattlegroundId();

        if (instanceId)
        {
            if (Map *map = FindInstanceMap(instanceId))
                return map;
            else
            {
                if (Battleground* bg = player->GetBattleground())
                    return CreateBattleground(instanceId, bg);
            }
        } else {
            return NULL;
        }
    }

    if (InstanceSave *pSave = player->GetInstanceSave(GetId()))
    {
        if (!instanceId || player->IsGameMaster())
        {
            instanceId = pSave->GetInstanceId(); // go from outside to instance
            if (Map *map = FindInstanceMap(instanceId))
                return map;
        }
        else if (instanceId != pSave->GetInstanceId()) // cannot go from one instance to another
            return NULL;
        // else log in at a saved instance

        return CreateInstance(instanceId, pSave, pSave->GetDifficulty());
    }
    else if (!player->GetSession()->PlayerLoading())
    {
        if (!instanceId)
            instanceId = sMapMgr->GenerateInstanceId();

        return CreateInstance(instanceId, NULL, player->GetDifficulty());
    }

    return NULL;
}

Map* MapInstanced::FindInstanceMap(uint32 InstanceId)
{
    InstancedMaps::iterator i = m_InstancedMaps.find(InstanceId);
    return(i == m_InstancedMaps.end() ? NULL : i->second);
}

InstanceMap* MapInstanced::CreateInstance(uint32 InstanceId, InstanceSave *save, Difficulty difficulty)
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
    const InstanceTemplate * iTemplate = sObjectMgr->GetInstanceTemplate(GetId());
    if(!iTemplate)
    {
        TC_LOG_ERROR("maps","CreateInstance: no instance template for map %d", GetId());
        ABORT();
    }

    // some instances only have one difficulty
    GetDownscaledMapDifficultyData(GetId(), difficulty);

    TC_LOG_DEBUG("maps", "MapInstanced::CreateInstance: %s map instance %d for %d created with difficulty %s", save ? "" : "new ", InstanceId, GetId(), difficulty ? "heroic" : "normal");

    InstanceMap *map = new InstanceMap(GetId(), InstanceId, difficulty);
    assert(map->IsDungeon());

    bool load_data = save != NULL;
    map->CreateInstanceData(load_data);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

BattlegroundMap* MapInstanced::CreateBattleground(uint32 InstanceId, Battleground* bg)
{
    // load/create a map
    std::lock_guard<std::mutex> lock(_mapLock);

    TC_LOG_DEBUG("maps", "MapInstanced::CreateBattleground: map bg %d for %d created.", InstanceId, GetId());

    BattlegroundMap *map = new BattlegroundMap(GetId(), InstanceId);
    assert(map->IsBattlegroundOrArena());
    map->SetBG(bg);

    m_InstancedMaps[InstanceId] = map;
    return map;
}

bool MapInstanced::DestroyInstance(uint32 InstanceId)
{
    InstancedMaps::iterator itr = m_InstancedMaps.find(InstanceId);
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

    // Free up the instance id and allow it to be reused for bgs and arenas (other instances are handled in the InstanceSaveMgr)
/*TCMAP    if (itr->second->IsBattlegroundOrArena())
        sMapMgr->FreeInstanceId(itr->second->GetInstanceId()); */

    // erase map
    delete itr->second;
    m_InstancedMaps.erase(itr++);

    return true;
}

bool MapInstanced::CanEnter(Player *player)
{
    if(Map* map = GetInstance(player))
        return map->CanEnter(player);

    return false;
}

