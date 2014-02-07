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

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/FastMutex.h"
#include "Utilities/UnorderedMap.h"
#include "Policies/ThreadingModel.h"

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

        typedef UNORDERED_MAP< uint64, T* >   MapType;
        typedef ZThread::FastMutex LockType;
        typedef Trinity::GeneralLock<LockType > Guard;

        static void Insert(T* o) { Guard guard(i_lock); m_objectMap[o->GetGUID()] = o; }

        static void Remove(T* o, uint64 guid)
        {   
            Guard guard(i_lock);
            typename MapType::iterator itr = m_objectMap.find(guid);
            if (itr != m_objectMap.end())
                m_objectMap.erase(itr);
        }

        static void Remove(T* o)
        {
            Remove(o, o->GetGUID());
        }

        static T* Find(uint64 guid)
        {
            typename MapType::iterator itr = m_objectMap.find(guid);
            return (itr != m_objectMap.end()) ? itr->second : NULL;
        }

        static MapType& GetContainer() { return m_objectMap; }

        static LockType* GetLock() { return &i_lock; }
    private:

        //Non instanceable only static
        HashMapHolder() {}

        static LockType i_lock;
        static MapType  m_objectMap;
};

class ObjectAccessor : public Trinity::Singleton<ObjectAccessor, Trinity::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex> >
{

    friend class Trinity::OperatorNew<ObjectAccessor>;
    ObjectAccessor();
    ~ObjectAccessor();
    ObjectAccessor(const ObjectAccessor &);
    ObjectAccessor& operator=(const ObjectAccessor &);

    public:
        typedef UNORDERED_MAP<uint64, Corpse* >      Player2CorpsesMapType;
        typedef UNORDERED_MAP<Player*, UpdateData>::value_type UpdateDataValueType;

        template<class T> static T* GetObjectInWorld(uint64 guid, T* /*fake*/)
        {
            return HashMapHolder<T>::Find(guid);
        }


        // returns object if is in map
        template<class T> static T* GetObjectInMap(uint64 guid, Map* map, T* /*typeSpecifier*/)
        {
            ASSERT(map);
            if (T * obj = GetObjectInWorld(guid, (T*)NULL))
                if (obj->GetMap() == map)
                    return obj;
            return NULL;
        }

        static Unit* GetObjectInWorld(uint64 guid, Unit* /*fake*/)
        {
            if (IS_PLAYER_GUID(guid))
                return (Unit*)GetObjectInWorld(guid, (Player*) NULL);

            if(Unit* pet = GetObjectInWorld(guid, (Pet*) NULL))
                return pet;
            /* Can't use this since pets on live server have some invalid guid for a lot of players
            if (IS_PET_GUID(guid))
                return (Unit*)GetObjectInWorld(guid, (Pet*) NULL);
                */

            return (Unit*)GetObjectInWorld(guid, (Creature*) NULL);
        }

        /*
        // Player may be not in world while in ObjectAccessor
        static Player* GetObjectInWorld(uint64 guid, Player* typeSpecifie)
        {
            Player* player = HashMapHolder<Player>::Find(guid);
            return player && player->IsInWorld() ? player : NULL;
        }
        */

        template<class T> static T* GetObjectInWorld(uint32 mapid, float x, float y, uint64 guid, T* /*fake*/);

        static WorldObject* GetWorldObject(WorldObject const& p, uint64 guid);

        // these functions return objects only if in map of specified object
        static Object*   GetObjectByTypeMask(Player const &, uint64, uint32 typemask);
        static Creature* GetNPCIfCanInteractWith(Player const &player, uint64 guid, uint32 npcflagmask);
        static Creature* GetCreature(WorldObject const &, uint64);
        static Creature* GetCreatureOrPet(WorldObject const &, uint64);
        static Unit* GetUnit(WorldObject const &, uint64);
        static Player* GetPlayer(WorldObject const &, uint64 guid);
        static GameObject* GetGameObject(WorldObject const &, uint64);
        static DynamicObject* GetDynamicObject(WorldObject const &, uint64);
        static Corpse* GetCorpse(WorldObject const &u, uint64 guid);
        static Pet* GetPet(WorldObject const &u, uint64 guid);

        // these functions return objects if found in whole world
        // ACCESS LIKE THAT IS NOT THREAD SAFE
        static Player* FindPlayer(uint64);
        static Unit* FindUnit(uint64);
        static Pet* FindPet(uint64);
        static Creature* FindCreature(uint64);

        Player* FindPlayerByName(const char *name) ;

        HashMapHolder<Player>::MapType& GetPlayers()
        {
            return HashMapHolder<Player>::GetContainer();
        }

        template<class T> void AddObject(T *object)
        {
            HashMapHolder<T>::Insert(object);
        }

        template<class T> void RemoveObject(T *object)
        {
            HashMapHolder<T>::Remove(object);
        }

        template<class T> void RemoveObject(T *object, uint64 guid)
        {
            HashMapHolder<T>::Remove(object, guid);
        }

        void RemoveObject(Player *pl)
        {
            HashMapHolder<Player>::Remove(pl);

            Guard guard(i_updateGuard);

            std::set<Object *>::iterator iter2 = std::find(i_objects.begin(), i_objects.end(), (Object *)pl);
            if( iter2 != i_objects.end() )
                i_objects.erase(iter2);
        }

        void SaveAllPlayers();

        void AddUpdateObject(Object *obj);
        void RemoveUpdateObject(Object *obj);

        void Update(uint32 diff);
        void UpdatePlayers(uint32 diff);

        Corpse* GetCorpseForPlayerGUID(uint64 guid);
        void RemoveCorpse(Corpse *corpse);
        void AddCorpse(Corpse* corpse);
        void AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map);
        Corpse* ConvertCorpseForPlayer(uint64 player_guid, bool insignia = false);

        static void UpdateObject(Object* obj, Player* exceptPlayer);
        static void _buildUpdateObject(Object* obj, UpdateDataMapType &);

        static void UpdateObjectVisibility(WorldObject* obj);
        //static void UpdateVisibilityForPlayer(Player* player);
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
        Player2CorpsesMapType   i_player2corpse;

        typedef ZThread::FastMutex LockType;
        typedef Trinity::GeneralLock<LockType > Guard;

        static void _buildChangeObjectForPlayer(WorldObject *, UpdateDataMapType &);
        static void _buildPacket(Player *, Object *, UpdateDataMapType &);
        void _update(void);
        std::set<Object *> i_objects;
        LockType i_playerGuard;
        LockType i_updateGuard;
        LockType i_corpseGuard;
        LockType i_petGuard;
};
#endif

