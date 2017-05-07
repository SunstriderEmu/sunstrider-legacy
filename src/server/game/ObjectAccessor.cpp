
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
#include "ObjectGuid.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectDefines.h"
#include "MapInstanced.h"
#include "World.h"
#include "Transport.h"
#include <cmath>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>


template<class T>
void HashMapHolder<T>::Insert(T* o)
{
	boost::unique_lock<boost::shared_mutex> lock(*GetLock());

	GetContainer()[o->GetGUID()] = o;
}

template<class T>
void HashMapHolder<T>::Remove(T* o)
{
	boost::unique_lock<boost::shared_mutex> lock(*GetLock());

	GetContainer().erase(o->GetGUID());
}

template<class T>
T* HashMapHolder<T>::Find(ObjectGuid guid)
{
	boost::shared_lock<boost::shared_mutex> lock(*GetLock());

	typename MapType::iterator itr = GetContainer().find(guid);
	return (itr != GetContainer().end()) ? itr->second : NULL;
}

template<class T>
auto HashMapHolder<T>::GetContainer() -> MapType&
{
	static MapType _objectMap;
	return _objectMap;
}

template<class T>
boost::shared_mutex* HashMapHolder<T>::GetLock()
{
	static boost::shared_mutex _lock;
	return &_lock;
}

HashMapHolder<Player>::MapType const& ObjectAccessor::GetPlayers()
{
	return HashMapHolder<Player>::GetContainer();
}

template class TC_GAME_API HashMapHolder<Player>;
template class TC_GAME_API HashMapHolder<Transport>;

namespace PlayerNameMapHolder
{
	typedef std::unordered_map<std::string, Player*> MapType;
	static MapType PlayerNameMap;

	void Insert(Player* p)
	{
		PlayerNameMap[p->GetName()] = p;
	}

	void Remove(Player* p)
	{
		PlayerNameMap.erase(p->GetName());
	}

	Player* Find(std::string const& name)
	{
		std::string charName(name);
		if (!normalizePlayerName(charName))
			return nullptr;

		auto itr = PlayerNameMap.find(charName);
		return (itr != PlayerNameMap.end()) ? itr->second : nullptr;
	}
} // namespace PlayerNameMapHolder

WorldObject* ObjectAccessor::GetWorldObject(WorldObject const& p, ObjectGuid const& guid)
{
	switch (guid.GetHigh())
	{
	case HighGuid::Player:        return GetPlayer(p, guid);
	case HighGuid::Transport:
	case HighGuid::Mo_Transport:
	case HighGuid::GameObject:    return GetGameObject(p, guid);
	case HighGuid::Vehicle:
	case HighGuid::Unit:          return GetCreature(p, guid);
	case HighGuid::Pet:           return GetPet(p, guid);
	case HighGuid::DynamicObject: return GetDynamicObject(p, guid);
	case HighGuid::Corpse:        return GetCorpse(p, guid);
	default:                      return nullptr;
	}
}

Object* ObjectAccessor::GetObjectByTypeMask(WorldObject const& p, ObjectGuid const& guid, uint32 typemask)
{
	switch (guid.GetHigh())
	{
	case HighGuid::Item:
		if (typemask & TYPEMASK_ITEM && p.GetTypeId() == TYPEID_PLAYER)
			return ((Player const&)p).GetItemByGuid(guid);
		break;
	case HighGuid::Player:
		if (typemask & TYPEMASK_PLAYER)
			return GetPlayer(p, guid);
		break;
	case HighGuid::Transport:
	case HighGuid::Mo_Transport:
	case HighGuid::GameObject:
		if (typemask & TYPEMASK_GAMEOBJECT)
			return GetGameObject(p, guid);
		break;
	case HighGuid::Unit:
	case HighGuid::Vehicle:
		if (typemask & TYPEMASK_UNIT)
			return GetCreature(p, guid);
		break;
	case HighGuid::Pet:
		if (typemask & TYPEMASK_UNIT)
			return GetPet(p, guid);
		break;
	case HighGuid::DynamicObject:
		if (typemask & TYPEMASK_DYNAMICOBJECT)
			return GetDynamicObject(p, guid);
		break;
	case HighGuid::Corpse:
		break;
	default:
		break;
	}

	return NULL;
}

Corpse* ObjectAccessor::GetCorpse(WorldObject const& u, ObjectGuid const& guid)
{
	return u.GetMap()->GetCorpse(guid);
}

GameObject* ObjectAccessor::GetGameObject(WorldObject const& u, ObjectGuid const& guid)
{
	return u.GetMap()->GetGameObject(guid);
}

Transport* ObjectAccessor::GetTransport(WorldObject const& u, ObjectGuid const& guid)
{
	return u.GetMap()->GetTransport(guid);
}

DynamicObject* ObjectAccessor::GetDynamicObject(WorldObject const& u, ObjectGuid const& guid)
{
	return u.GetMap()->GetDynamicObject(guid);
}

Unit* ObjectAccessor::GetUnit(WorldObject const& u, ObjectGuid const& guid)
{
	if (guid.IsPlayer())
		return GetPlayer(u, guid);

	if (guid.IsPet())
		return GetPet(u, guid);

	return GetCreature(u, guid);
}

Creature* ObjectAccessor::GetCreature(WorldObject const& u, ObjectGuid const& guid)
{
	return u.GetMap()->GetCreature(guid);
}

Pet* ObjectAccessor::GetPet(WorldObject const& u, ObjectGuid const& guid)
{
	return u.GetMap()->GetPet(guid);
}

Player* ObjectAccessor::GetPlayer(Map const* m, ObjectGuid const& guid)
{
	if (Player* player = HashMapHolder<Player>::Find(guid))
		if (player->IsInWorld() && player->GetMap() == m)
			return player;

	return nullptr;
}

Player* ObjectAccessor::GetPlayer(WorldObject const& u, ObjectGuid const& guid)
{
	return GetPlayer(u.GetMap(), guid);
}

Creature* ObjectAccessor::GetCreatureOrPetOrVehicle(WorldObject const& u, ObjectGuid const& guid)
{
	if (guid.IsPet())
		return GetPet(u, guid);

	if (guid.IsCreatureOrVehicle())
		return GetCreature(u, guid);

	return NULL;
}

Player* ObjectAccessor::FindPlayer(ObjectGuid const& guid)
{
	Player* player = HashMapHolder<Player>::Find(guid);
	return player && player->IsInWorld() ? player : nullptr;
}

Player* ObjectAccessor::FindPlayerByName(std::string const& name)
{
	Player* player = PlayerNameMapHolder::Find(name);
	if (!player || !player->IsInWorld())
		return nullptr;

	return player;
}

Player* ObjectAccessor::FindPlayerByLowGUID(ObjectGuid::LowType lowguid)
{
	ObjectGuid guid(HighGuid::Player, lowguid);
	return ObjectAccessor::FindPlayer(guid);
}

Player* ObjectAccessor::FindConnectedPlayer(ObjectGuid const& guid)
{
	return HashMapHolder<Player>::Find(guid);
}

Player* ObjectAccessor::FindConnectedPlayerByName(std::string const& name)
{
	return PlayerNameMapHolder::Find(name);
}

void ObjectAccessor::SaveAllPlayers()
{
	boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

	HashMapHolder<Player>::MapType const& m = GetPlayers();
	for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
		itr->second->SaveToDB();
}

template<>
void ObjectAccessor::AddObject(Player* player)
{
	HashMapHolder<Player>::Insert(player);
	PlayerNameMapHolder::Insert(player);
}

template<>
void ObjectAccessor::RemoveObject(Player* player)
{
	HashMapHolder<Player>::Remove(player);
	PlayerNameMapHolder::Remove(player);
}


/*
template<class T>
void HashMapHolder<T>::Insert(T* o)
{
	boost::unique_lock<boost::shared_mutex> lock(*GetLock());

	GetContainer()[o->GetGUID()] = o;
}

template<class T>
void HashMapHolder<T>::Remove(T* o)
{
	boost::unique_lock<boost::shared_mutex> lock(*GetLock());

	GetContainer().erase(o->GetGUID());
}

template<class T>
T* HashMapHolder<T>::Find(uint64 guid)
{
	boost::unique_lock<boost::shared_mutex> lock(*GetLock());

	auto itr = GetContainer().find(guid);
	return (itr != GetContainer().end()) ? itr->second : nullptr;
}

template<class T>
auto HashMapHolder<T>::GetContainer() -> MapType&
{
	static MapType _objectMap;
	return _objectMap;
}

template<class T>
boost::shared_mutex* HashMapHolder<T>::GetLock()
{
	static boost::shared_mutex _lock;
	return &_lock;
}

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
    Creature * ret = GetObjectInWorld(guid, (Creature*)nullptr);
    if(!ret)
        return nullptr;

    if(ret->GetMapId() != u.GetMapId())
        return nullptr;

    if(ret->GetInstanceId() != u.GetInstanceId())
        return nullptr;

    return ret;
}

Unit* ObjectAccessor::GetUnit(WorldObject const &u, uint64 guid)
{
    if(!guid)
        return nullptr;

    if(IS_PLAYER_GUID(guid))
        return FindPlayer(guid);

    return GetCreatureOrPetOrVehicle(u, guid);
}

Corpse* ObjectAccessor::GetCorpse(WorldObject const &u, uint64 guid)
{
    Corpse * ret = GetObjectInWorld(guid, (Corpse*)nullptr);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = nullptr;
    return ret;
}

Object* ObjectAccessor::GetObjectByTypeMask(Player const &p, uint64 guid, uint32 typemask)
{
    
    
    Object *obj = nullptr;

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

    return nullptr;
}

GameObject*
ObjectAccessor::GetGameObject(WorldObject const &u, uint64 guid)
{
    GameObject * ret = GetObjectInWorld(guid, (GameObject*)nullptr);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = nullptr;
    return ret;
}

DynamicObject* ObjectAccessor::GetDynamicObject(Unit const &u, uint64 guid)
{
    DynamicObject * ret = GetObjectInWorld(guid, (DynamicObject*)nullptr);
    if(ret && ret->GetMapId() != u.GetMapId()) ret = nullptr;
    return ret;
}

Player* ObjectAccessor::FindPlayer(uint64 guid)
{
    return GetObjectInWorld(guid, (Player*)nullptr);
}

Player* ObjectAccessor::FindPlayer(uint32 guidLow)
{
    return FindPlayer(MAKE_PAIR64(guidLow, HighGuid::Player));
}

Player* ObjectAccessor::FindConnectedPlayer(uint64 guid)
{
    return HashMapHolder<Player>::Find(guid);
}

Unit* ObjectAccessor::FindUnit(uint64 guid)
{
    return GetObjectInWorld(guid, (Unit*)nullptr);
}

void ObjectAccessor::SaveAllPlayers()
{
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    HashMapHolder<Player>::MapType const& m = GetPlayers();
    for (const auto & itr : m)
        itr.second->SaveToDB();
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
    return GetObjectInWorld(guid, (Pet*)nullptr);
}

Corpse* GetMap()->GetCorpseByPlayer(uint64 guid)
{
    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);

    auto iter = i_player2corpse.find(guid);
    if( iter == i_player2corpse.end() ) return nullptr;

    assert(iter->second->GetType() != CORPSE_BONES);

    return iter->second;
}

void ObjectAccessor::RemoveCorpse(Corpse *corpse)
{
    assert(corpse && corpse->GetType() != CORPSE_BONES);

    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);
    auto iter = i_player2corpse.find(corpse->GetOwnerGUID());
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

void GetMap()->AddCorpse(Corpse* corpse)
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

void GetMap()->AddCorpsesToGrid(GridCoord const& gridpair,GridType& grid,Map* map)
{
    boost::shared_lock<boost::shared_mutex> lock(_corpseLock);
    for(auto & iter : i_player2corpse)
        if(iter.second->GetGrid()==gridpair)
    {
        // verify, if the corpse in our instance (add only corpses which are)
        if (map->Instanceable())
        {
            if (iter.second->GetInstanceId() == map->GetInstanceId())
            {
                grid.AddWorldObject(iter.second);
            }
        }
        else
        {
            grid.AddWorldObject(iter.second);
        }
    }
}

Corpse* GetMap()->ConvertCorpseToBones(uint64 player_guid, bool insignia)
{
    Corpse *corpse = GetCorpseForPlayerGUID(player_guid);
    if(!corpse)
    {
        //in fact this function is called from several places
        //even when player doesn't have a corpse, not an error
        //TC_LOG_ERROR("FIXME","ERROR: Try remove corpse that not in map for GUID %ul", player_guid);
        return nullptr;
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
       (map->IsBattlegroundOrArena() ? sWorld->getConfig(CONFIG_DEATH_BONES_BG_OR_ARENA) : sWorld->getConfig(CONFIG_DEATH_BONES_WORLD))))
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
    UpdatePlayerSet player_set;

    {
        std::lock_guard<std::mutex> lock(_objectLock);
        while (!i_objects.empty())
        {
            Object* obj = *i_objects.begin();
            ASSERT(obj && obj->IsInWorld());
            i_objects.erase(i_objects.begin());
            obj->BuildUpdate(update_players, player_set);
        }
    }

    //send those to players
    WorldPacket packet;                                     // here we allocate a std::vector with a size of 0x10000
    for(auto & update_player : update_players)
    {
        update_player.second.BuildPacket(&packet, update_player.first->GetSession()->GetClientBuild());
        update_player.first->SendDirectMessage(&packet);
        packet.clear();                                     // clean the string
    }
}
*/

void ObjectAccessor::UpdateObjectVisibility(WorldObject *obj)
{
    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(p);

    obj->GetMap()->UpdateObjectVisibility(obj,cell,p);
}

/*

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Pet>;
template class HashMapHolder<GameObject>;
template class HashMapHolder<DynamicObject>;
template class HashMapHolder<Creature>;
template class HashMapHolder<Corpse>;

template Player* ObjectAccessor::GetObjectInWorld<Player>(uint32 mapid, float x, float y, uint64 guid, Player*fake);
template Pet* ObjectAccessor::GetObjectInWorld<Pet>(uint32 mapid, float x, float y, uint64 guid, Pet*fake);
template Creature* ObjectAccessor::GetObjectInWorld<Creature>(uint32 mapid, float x, float y, uint64 guid, Creature*fake);
template Corpse* ObjectAccessor::GetObjectInWorld<Corpse>(uint32 mapid, float x, float y, uint64 guid, Corpse*fake);
template GameObject* ObjectAccessor::GetObjectInWorld<GameObject>(uint32 mapid, float x, float y, uint64 guid, GameObject*fake);
template DynamicObject* ObjectAccessor::GetObjectInWorld<DynamicObject>(uint32 mapid, float x, float y, uint64 guid, DynamicObject*fake);

WorldObject* ObjectAccessor::GetObjectInWorld(uint64 guid, WorldObject* p)
{
    switch (GUID_HIPART(guid))
    {
        case HighGuid::Unit:
        case HighGuid::Pet:
        case HighGuid::Player:        return GetObjectInWorld(guid, (Unit*)nullptr);
        case HighGuid::Transport:
        case HighGuid::Mo_Transport:
        case HighGuid::GameObject:    return GetObjectInWorld(guid, (GameObject*)nullptr);
        case HighGuid::DynamicObject: return GetObjectInWorld(guid, (DynamicObject*)nullptr);
        case HighGuid::Corpse:        return GetObjectInWorld(guid, (Corpse*)nullptr);
        default:                     return nullptr;
    }
}


Player* ObjectAccessor::GetObjectInWorld(uint64 guid, Player* typeSpecifier)
{
    Player* player = HashMapHolder<Player>::Find(guid);
    return player && player->IsInWorld() ? player : nullptr;
}

Unit* ObjectAccessor::GetObjectInWorld(uint64 guid, Unit*fake)
{
    if (!guid)
        return nullptr;

    if (IS_PLAYER_GUID(guid))
        return (Unit*)HashMapHolder<Player>::Find(guid);

    if (Unit* u = (Unit*)HashMapHolder<Pet>::Find(guid))
        return u;

    return (Unit*)HashMapHolder<Creature>::Find(guid);
}

void ObjectAccessor::UnloadAll()
{
}


Player*
ObjectAccessor::FindPlayerByName(std::string const& name)
{
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    std::string nameStr = name;
    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), ::tolower);
    HashMapHolder<Player>::MapType const& m = GetPlayers();
    for (const auto & iter : m)
    {
        if (!iter.second->IsInWorld())
            continue;
        std::string currentName = iter.second->GetName();
        std::transform(currentName.begin(), currentName.end(), currentName.begin(), ::tolower);
        if (nameStr.compare(currentName) == 0)
            return iter.second;
    }
    return nullptr;
}

Player* ObjectAccessor::FindConnectedPlayerByName(std::string const& name)
{
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    
    std::string nameStr = name;
    std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), ::tolower);
    HashMapHolder<Player>::MapType const& m = GetPlayers();
    for (const auto & iter : m)
    {
        std::string currentName = iter.second->GetName();
        std::transform(currentName.begin(), currentName.end(), currentName.begin(), ::tolower);
        if (nameStr.compare(currentName) == 0)
            return iter.second;
    }

    //TODO :use some kind of cache. See PlayerNameMapHolder at TrinityCore
    
    return nullptr;
}
*/