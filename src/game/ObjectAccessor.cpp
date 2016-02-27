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

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Item.h"
#include "Corpse.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectDefines.h"
#include "MapInstanced.h"
#include "World.h"

#include <cmath>

namespace Trinity
{
    struct BuildUpdateForPlayer
    {
        Player &i_player;
        UpdateDataMapType &i_updatePlayers;

        BuildUpdateForPlayer(Player &player, UpdateDataMapType &data_map) : i_player(player), i_updatePlayers(data_map) {}

        void Visit(PlayerMapType &m)
        {
            for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
            {
                if( iter->GetSource() == &i_player )
                    continue;

                UpdateDataMapType::iterator iter2 = i_updatePlayers.find(iter->GetSource());
                if( iter2 == i_updatePlayers.end() )
                {
                    std::pair<UpdateDataMapType::iterator, bool> p = i_updatePlayers.insert( ObjectAccessor::UpdateDataValueType(iter->GetSource(), UpdateData()) );
                    assert(p.second);
                    iter2 = p.first;
                }

                i_player.BuildValuesUpdateBlockForPlayer(&iter2->second, iter2->first);
            }
        }

        template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
    };
} // namespace Trinity

ObjectAccessor::ObjectAccessor() {}
ObjectAccessor::~ObjectAccessor() 
{
    for (auto itr : i_player2corpse)
        delete itr.second;

    i_player2corpse.clear();
}

Creature* ObjectAccessor::GetCreatureOrPetOrVehicle(WorldObject const &u, uint64 guid)
{
    if(Creature *unit = GetPet(guid))
        return unit;

    if(IS_CREATURE_OR_VEHICLE_GUID(guid))
        return GetCreature(u, guid);

    return nullptr;
}

Creature* ObjectAccessor::GetCreature(WorldObject const &u, uint64 guid)
{
    Creature * ret = GetObjectInWorld(guid, (Creature*)NULL);
    if(!ret)
        return NULL;

    if(ret->GetMapId() != u.GetMapId())
        return NULL;

    if(ret->GetInstanceId() != u.GetInstanceId())
        return NULL;

    return ret;
}

Unit* ObjectAccessor::GetUnit(WorldObject const &u, uint64 guid)
{
    if(!guid)
        return NULL;

    if(IS_PLAYER_GUID(guid))
        return FindPlayer(guid);

    return GetCreatureOrPetOrVehicle(u, guid);
}

Corpse* ObjectAccessor::GetCorpse(WorldObject const &u, uint64 guid)
{
    Corpse * ret = GetObjectInWorld(guid, (Corpse*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

Object* ObjectAccessor::GetObjectByTypeMask(Player const &p, uint64 guid, uint32 typemask)
{
    
    
    Object *obj = NULL;

    if(typemask & TYPEMASK_PLAYER)
    {
        obj = FindPlayer(guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_UNIT)
    {
        obj = GetCreatureOrPetOrVehicle(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_GAMEOBJECT)
    {
        obj = GetGameObject(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_DYNAMICOBJECT)
    {
        obj = GetDynamicObject(p,guid);
        if(obj) return obj;
    }

    if(typemask & TYPEMASK_ITEM)
    {
        obj = p.GetItemByGuid( guid );
        if(obj) return obj;
    }

    return NULL;
}

GameObject*
ObjectAccessor::GetGameObject(WorldObject const &u, uint64 guid)
{
    GameObject * ret = GetObjectInWorld(guid, (GameObject*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

DynamicObject* ObjectAccessor::GetDynamicObject(Unit const &u, uint64 guid)
{
    DynamicObject * ret = GetObjectInWorld(guid, (DynamicObject*)NULL);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
    return ret;
}

Player* ObjectAccessor::FindPlayer(uint64 guid)
{
    return GetObjectInWorld(guid, (Player*)NULL);
}

Player* ObjectAccessor::FindPlayer(uint32 guidLow)
{
    return FindPlayer(MAKE_PAIR64(guidLow, HIGHGUID_PLAYER));
}

Player* ObjectAccessor::FindConnectedPlayer(uint64 guid)
{
    return HashMapHolder<Player>::Find(guid);
}

Unit* ObjectAccessor::FindUnit(uint64 guid)
{
    return GetObjectInWorld(guid, (Unit*)NULL);
}

void ObjectAccessor::SaveAllPlayers()
{
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    HashMapHolder<Player>::MapType const& m = GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        itr->second->SaveToDB();
}

void ObjectAccessor::AddUpdateObject(Object *obj)
{
    std::lock_guard<std::mutex> lock(_objectLock);
    i_objects.insert(obj);
}

void ObjectAccessor::RemoveUpdateObject(Object *obj)
{
    std::lock_guard<std::mutex> lock(_objectLock);
    i_objects.erase(obj);
}

Pet* ObjectAccessor::GetPet(uint64 guid)
{
    return GetObjectInWorld(guid, (Pet*)NULL);
}

Corpse* ObjectAccessor::GetCorpseForPlayerGUID(uint64 guid)
{
    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);

    Player2CorpsesMapType::iterator iter = i_player2corpse.find(guid);
    if( iter == i_player2corpse.end() ) return NULL;

    assert(iter->second->GetType() != CORPSE_BONES);

    return iter->second;
}

void ObjectAccessor::RemoveCorpse(Corpse *corpse)
{
    assert(corpse && corpse->GetType() != CORPSE_BONES);

    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);
    Player2CorpsesMapType::iterator iter = i_player2corpse.find(corpse->GetOwnerGUID());
    if( iter == i_player2corpse.end() )
        return;

    // build mapid*cellid -> guid_set map
    CellCoord cell_pair = Trinity::ComputeCellCoord(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    sObjectMgr->DeleteCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID());
    corpse->RemoveFromWorld();

   // delete iter->second;
    i_player2corpse.erase(iter);
}

void ObjectAccessor::AddCorpse(Corpse* corpse)
{
    assert(corpse && corpse->GetType() != CORPSE_BONES);

    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);
    assert(i_player2corpse.find(corpse->GetOwnerGUID()) == i_player2corpse.end());
    i_player2corpse[corpse->GetOwnerGUID()] = corpse;

    // build mapid*cellid -> guid_set map
    CellCoord cell_pair = Trinity::ComputeCellCoord(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    sObjectMgr->AddCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID(),corpse->GetInstanceId());
}

void ObjectAccessor::AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map)
{
    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);
    for(Player2CorpsesMapType::iterator iter = i_player2corpse.begin(); iter != i_player2corpse.end(); ++iter)
        if(iter->second->GetGrid()==gridpair)
    {
        // verify, if the corpse in our instance (add only corpses which are)
        if (map->Instanceable())
        {
            if (iter->second->GetInstanceId() == map->GetInstanceId())
            {
                grid.AddWorldObject(iter->second);
            }
        }
        else
        {
            grid.AddWorldObject(iter->second);
        }
    }
}

Corpse* ObjectAccessor::ConvertCorpseForPlayer(uint64 player_guid, bool insignia)
{
    Corpse *corpse = GetCorpseForPlayerGUID(player_guid);
    if(!corpse)
    {
        //in fact this function is called from several places
        //even when player doesn't have a corpse, not an error
        //TC_LOG_ERROR("FIXME","ERROR: Try remove corpse that not in map for GUID %ul", player_guid);
        return NULL;
    }

    TC_LOG_DEBUG("FIXME","Deleting Corpse and spawning bones.\n");

    // remove corpse from player_guid -> corpse map
    RemoveCorpse(corpse);

    // remove resurrectble corpse from grid object registry (loaded state checked into call)
    // do not load the map if it's not loaded
    Map *map = sMapMgr->FindMap(corpse->GetMapId(), corpse->GetInstanceId());
    if(map) map->Remove(corpse,false);

    // remove corpse from DB
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    corpse->DeleteFromDB(trans);
    CharacterDatabase.CommitTransaction(trans);

    Corpse* bones = nullptr;
    // create the bones only if the map and the grid is loaded at the corpse's location
    // ignore bones creating option in case insignia
    if (map && (insignia ||
       (map->IsBattlegroundOrArena() ? sWorld->getConfig(CONFIG_DEATH_BONES_BG_OR_ARENA) : sWorld->getConfig(CONFIG_DEATH_BONES_WORLD))) &&
        !map->IsRemovalGrid(corpse->GetPositionX(), corpse->GetPositionY()))
    {
        // Create bones, don't change Corpse
        bones = new Corpse;
        bones->Create(corpse->GetGUIDLow());

        for (int i = 3; i < CORPSE_END; i++)                    // don't overwrite guid and object type
            bones->SetUInt32Value(i, corpse->GetUInt32Value(i));

        bones->SetGrid(corpse->GetGrid());
        // bones->m_time = m_time;                              // don't overwrite time
        // bones->m_inWorld = m_inWorld;                        // don't overwrite world state
        // bones->m_type = m_type;                              // don't overwrite type
        bones->Relocate(corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetOrientation());
        bones->SetMapId(corpse->GetMapId());
        bones->SetInstanceId(corpse->GetInstanceId());

        bones->SetUInt32Value(CORPSE_FIELD_FLAGS, CORPSE_FLAG_UNK2 | CORPSE_FLAG_BONES);
        bones->SetUInt64Value(CORPSE_FIELD_OWNER, 0);

        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if(corpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                bones->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
        }

        // add bones in grid store if grid loaded where corpse placed
        map->Add(bones);
    }

    // all references to the corpse should be removed at this point
    delete corpse;

    return bones;
}

void ObjectAccessor::Update(uint32 diff)
{
    //build updates for each objects
    UpdateDataMapType update_players; //one UpdateData object per player, containing updates for all objects
    {
        std::lock_guard<std::mutex> lock(_objectLock);
        while (!i_objects.empty())
        {
            Object* obj = *i_objects.begin();
            ASSERT(obj && obj->IsInWorld());
            i_objects.erase(i_objects.begin());
            obj->BuildUpdate(update_players);
        }
    }

    //send those to players
    WorldPacket packet;                                     // here we allocate a std::vector with a size of 0x10000
    for(UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        iter->second.BuildPacket(&packet);
        iter->first->GetSession()->SendPacket(&packet);
        packet.clear();                                     // clean the string
    }
}

void ObjectAccessor::UpdateObjectVisibility(WorldObject *obj)
{
    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(p);

    obj->GetMap()->UpdateObjectVisibility(obj,cell,p);
}

/// Define the static member of HashMapHolder

template <class T> std::unordered_map< uint64, T* > HashMapHolder<T>::m_objectMap;
template <class T> boost::shared_mutex HashMapHolder<T>::_lock;

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Pet>;
template class HashMapHolder<GameObject>;
template class HashMapHolder<DynamicObject>;
template class HashMapHolder<Creature>;
template class HashMapHolder<Corpse>;

template Player* ObjectAccessor::GetObjectInWorld<Player>(uint32 mapid, float x, float y, uint64 guid, Player* /*fake*/);
template Pet* ObjectAccessor::GetObjectInWorld<Pet>(uint32 mapid, float x, float y, uint64 guid, Pet* /*fake*/);
template Creature* ObjectAccessor::GetObjectInWorld<Creature>(uint32 mapid, float x, float y, uint64 guid, Creature* /*fake*/);
template Corpse* ObjectAccessor::GetObjectInWorld<Corpse>(uint32 mapid, float x, float y, uint64 guid, Corpse* /*fake*/);
template GameObject* ObjectAccessor::GetObjectInWorld<GameObject>(uint32 mapid, float x, float y, uint64 guid, GameObject* /*fake*/);
template DynamicObject* ObjectAccessor::GetObjectInWorld<DynamicObject>(uint32 mapid, float x, float y, uint64 guid, DynamicObject* /*fake*/);

WorldObject* ObjectAccessor::GetObjectInWorld(uint64 guid, WorldObject* p)
{
    switch (GUID_HIPART(guid))
    {
        case HIGHGUID_UNIT:
        case HIGHGUID_PET:
        case HIGHGUID_PLAYER:        return GetObjectInWorld(guid, (Unit*)NULL);
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_MO_TRANSPORT:
        case HIGHGUID_GAMEOBJECT:    return GetObjectInWorld(guid, (GameObject*)NULL);
        case HIGHGUID_DYNAMICOBJECT: return GetObjectInWorld(guid, (DynamicObject*)NULL);
        case HIGHGUID_CORPSE:        return GetObjectInWorld(guid, (Corpse*)NULL);
        default:                     return NULL;
    }
}

Unit* ObjectAccessor::GetObjectInWorld(uint64 guid, Unit* /*fake*/)
{
    if (!guid)
        return NULL;

    if (IS_PLAYER_GUID(guid))
        return (Unit*)HashMapHolder<Player>::Find(guid);

    if (Unit* u = (Unit*)HashMapHolder<Pet>::Find(guid))
        return u;

    return (Unit*)HashMapHolder<Creature>::Find(guid);
}

void ObjectAccessor::UnloadAll()
{
    /* LK
    for (Player2CorpsesMapType::const_iterator itr = i_player2corpse.begin(); itr != i_player2corpse.end(); ++itr)
    {
        itr->second->RemoveFromWorld();
        delete itr->second;
    }*/
}


Player*
ObjectAccessor::FindPlayerByName(std::string const& name)
{
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    std::string nameStr = name;
    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), ::tolower);
    HashMapHolder<Player>::MapType const& m = GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (!iter->second->IsInWorld())
            continue;
        std::string currentName = iter->second->GetName();
        std::transform(currentName.begin(), currentName.end(), currentName.begin(), ::tolower);
        if (nameStr.compare(currentName) == 0)
            return iter->second;
    }
    return nullptr;
}

Player* ObjectAccessor::FindConnectedPlayerByName(std::string const& name)
{
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    
    std::string nameStr = name;
    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), ::tolower);
    HashMapHolder<Player>::MapType const& m = GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        std::string currentName = iter->second->GetName();
        std::transform(currentName.begin(), currentName.end(), currentName.begin(), ::tolower);
        if (nameStr.compare(currentName) == 0)
            return iter->second;
    }

    //TODO :use some kind of cache
    
    return nullptr;
}
