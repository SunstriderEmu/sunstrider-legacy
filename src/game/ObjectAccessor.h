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

#ifndef TRINITY_OBJECTACCESSOR_H
#define TRINITY_OBJECTACCESSOR_H

#include "Define.h"
#include "Policies/Singleton.h"
#include "Policies/ThreadingModel.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "ByteBuffer.h"
#include "UpdateData.h"

#include "GridDefines.h"
#include "Object.h"
#include "Player.h"

#include <set>

class Creature;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class WorldObject;
class Map;

template <class T>
class HashMapHolder
{
    public:

        typedef std::unordered_map<uint64, T*> MapType;

        static void Insert(T* o)
        {
            boost::unique_lock<boost::shared_mutex> lock(_lock);

            m_objectMap[o->GetGUID()] = o;
        }

        static void Remove(T* o)
        {
            boost::unique_lock<boost::shared_mutex> lock(_lock);

            m_objectMap.erase(o->GetGUID());
        }

        static T* Find(uint64 guid)
        {
            boost::shared_lock<boost::shared_mutex> lock(_lock);

            typename MapType::iterator itr = m_objectMap.find(guid);
            return (itr != m_objectMap.end()) ? itr->second : NULL;
        }

        static MapType& GetContainer() { return m_objectMap; }

        static boost::shared_mutex* GetLock() { return &_lock; }
    private:

        //Non instanceable only static
        HashMapHolder() {}

        static boost::shared_mutex _lock;
        static MapType  m_objectMap;
};

class ObjectAccessor
{
    friend class Trinity::OperatorNew<ObjectAccessor>;
    ObjectAccessor();
    ~ObjectAccessor();
    ObjectAccessor(const ObjectAccessor &);
    ObjectAccessor& operator=(const ObjectAccessor &);

    public:
        static ObjectAccessor* instance()
        {
            static ObjectAccessor instance;
            return &instance;
        }
        
        typedef std::unordered_map<uint64, Corpse* > Player2CorpsesMapType;
        typedef std::unordered_map<Player*, UpdateData>::value_type UpdateDataValueType;

        template<class T> static T* GetObjectInWorld(uint64 guid, T* /*fake*/)
        {
            return HashMapHolder<T>::Find(guid);
        }

        static WorldObject* GetObjectInWorld(uint64 guid, WorldObject* p);

        static Unit* GetObjectInWorld(uint64 guid, Unit* /*fake*/)
        {
            if(!guid)
                return NULL;

            if (IS_PLAYER_GUID(guid))
                return (Unit*)HashMapHolder<Player>::Find(guid);

            if (Unit* u = (Unit*)HashMapHolder<Pet>::Find(guid))
                return u;

            return (Unit*)HashMapHolder<Creature>::Find(guid);
        }

        template<class T> static T* GetObjectInWorld(uint32 mapid, float x, float y, uint64 guid, T* /*fake*/)
        {
            T* obj = HashMapHolder<T>::Find(guid);
            if(!obj || obj->GetMapId() != mapid) return NULL;

            CellPair p = Trinity::ComputeCellPair(x,y);
            if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
            {
                TC_LOG_ERROR("FIXME","ObjectAccessor::GetObjectInWorld: invalid coordinates supplied X:%f Y:%f grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
                return NULL;
            }

            CellPair q = Trinity::ComputeCellPair(obj->GetPositionX(),obj->GetPositionY());
            if(q.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || q.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
            {
                TC_LOG_ERROR("FIXME","ObjectAccessor::GetObjecInWorld: object " UI64FMTD " has invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), q.x_coord, q.y_coord);
                return NULL;
            }

            int32 dx = int32(p.x_coord) - int32(q.x_coord);
            int32 dy = int32(p.y_coord) - int32(q.y_coord);

            if (dx > -2 && dx < 2 && dy > -2 && dy < 2) return obj;
            else return NULL;
        }

        static Object*   GetObjectByTypeMask(Player const &, uint64, uint32 typemask);
        static Creature* GetCreature(WorldObject const &, uint64);
        static Creature* GetCreatureOrPetOrVehicle(WorldObject const &, uint64);
        static Unit* GetUnit(WorldObject const &, uint64);
        static Pet* GetPet(Unit const &, uint64 guid) { return GetPet(guid); }
        static Player* GetPlayer(Unit const &, uint64 guid) { return FindPlayer(guid); }
        static GameObject* GetGameObject(WorldObject const &, uint64);
        static DynamicObject* GetDynamicObject(Unit const &, uint64);
        static Corpse* GetCorpse(WorldObject const &u, uint64 guid);
        static Pet* GetPet(uint64 guid);
        static Player* FindPlayer(uint64);
        static Player* FindConnectedPlayer(uint64);
        static Unit* FindUnit(uint64);

        Player* FindPlayerByName(std::string const& name);
        static Player* FindConnectedPlayerByName(std::string const& name);

        // when using this, you must use the hashmapholder's lock
        static HashMapHolder<Player>::MapType& GetPlayers()
        {
            return HashMapHolder<Player>::GetContainer();
        }

        // when using this, you must use the hashmapholder's lock
        static HashMapHolder<Creature>::MapType const& GetCreatures()
        {
            return HashMapHolder<Creature>::GetContainer();
        }

        // when using this, you must use the hashmapholder's lock
        static HashMapHolder<GameObject>::MapType const& GetGameObjects()
        {
            return HashMapHolder<GameObject>::GetContainer();
        }

        template<class T> void AddObject(T *object)
        {
            HashMapHolder<T>::Insert(object);
        }

        template<class T> void RemoveObject(T *object)
        {
            HashMapHolder<T>::Remove(object);
        }

        void RemoveObject(Player *pl)
        {
            HashMapHolder<Player>::Remove(pl);

            std::lock_guard<std::mutex> lock(_objectLock);

            std::set<Object *>::iterator iter2 = std::find(i_objects.begin(), i_objects.end(), (Object *)pl);
            if( iter2 != i_objects.end() )
                i_objects.erase(iter2);
        }

        void SaveAllPlayers();

        void AddUpdateObject(Object *obj);
        void RemoveUpdateObject(Object *obj);

        void Update(uint32 diff);

        Corpse* GetCorpseForPlayerGUID(uint64 guid);
        void RemoveCorpse(Corpse *corpse);
        void AddCorpse(Corpse* corpse);
        void AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map);
        Corpse* ConvertCorpseForPlayer(uint64 player_guid, bool insignia = false);

        static void UpdateObject(Object* obj, Player* exceptPlayer);
        static void _buildUpdateObject(Object* obj, UpdateDataMapType &);

        static void UpdateObjectVisibility(WorldObject* obj);
        
        void UnloadAll();

    private:
        struct WorldObjectChangeAccumulator
        {
            UpdateDataMapType &i_updateDatas;
            WorldObject &i_object;
            std::set<uint64> plr_list;
            WorldObjectChangeAccumulator(WorldObject &obj, UpdateDataMapType &d) : i_updateDatas(d), i_object(obj) {}
            void Visit(PlayerMapType &);
            void Visit(CreatureMapType &);
            void Visit(DynamicObjectMapType &);
            void BuildPacket(Player* plr);
            template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
        };

        friend struct WorldObjectChangeAccumulator;

        static void _buildChangeObjectForPlayer(WorldObject *, UpdateDataMapType&);
        static void _buildPacket(Player *, Object *, UpdateDataMapType &);
        void _update(void);
        
        std::set<Object *> i_objects;
        Player2CorpsesMapType i_player2corpse;

        std::mutex _objectLock;
        boost::shared_mutex _corpseLock;
};

#define sObjectAccessor ObjectAccessor::instance()
#endif

