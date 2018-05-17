
#ifndef TRINITY_OBJECTACCESSOR_H
#define TRINITY_OBJECTACCESSOR_H

#include <mutex>
#include <set>
#include <unordered_map>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>


#include "Define.h"
#include "GridDefines.h"
#include "Object.h"
#include "UpdateData.h"

class Creature;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class WorldObject;
class Map;

/** Static hash map */
template <class T>
class TC_GAME_API HashMapHolder
{
    //Non instanceable only static
    HashMapHolder() { }

public:
	static_assert(std::is_same<Player, T>::value
		|| std::is_same<MotionTransport, T>::value,
		"Only Player and Transport can be registered in global HashMapHolder");

	typedef std::unordered_map<ObjectGuid, T*> MapType;

    static void Insert(T* o);

    static void Remove(T* o);

	static T* Find(ObjectGuid guid);

    static MapType& GetContainer();

    static boost::shared_mutex* GetLock();
};

namespace ObjectAccessor
{
		// these functions return objects only if in map of specified object
		TC_GAME_API WorldObject* GetWorldObject(WorldObject const&, ObjectGuid const& guid);
		TC_GAME_API Object* GetObjectByTypeMask(WorldObject const&, ObjectGuid const&, uint32 typemask);
		TC_GAME_API Corpse* GetCorpse(WorldObject const& u, ObjectGuid const& guid);
		TC_GAME_API Creature* GetCreature(WorldObject const &, ObjectGuid const& guid);
		TC_GAME_API Pet* GetPet(WorldObject const&, ObjectGuid const& guid);
		TC_GAME_API Creature* GetCreatureOrPetOrVehicle(WorldObject const&, ObjectGuid const&);
		TC_GAME_API GameObject* GetGameObject(WorldObject const &, ObjectGuid const& guid);
		TC_GAME_API Transport* GetTransport(WorldObject const& u, ObjectGuid const& guid);
		TC_GAME_API DynamicObject* GetDynamicObject(WorldObject const& u, ObjectGuid const& guid);
		TC_GAME_API Unit* GetUnit(WorldObject const&, ObjectGuid const& guid);
		TC_GAME_API Player* GetPlayer(Map const*, ObjectGuid const& guid);
		TC_GAME_API Player* GetPlayer(WorldObject const&, ObjectGuid const& guid);
        
		// these functions return objects if found in whole world
		// ACCESS LIKE THAT IS NOT THREAD SAFE
		TC_GAME_API Player* FindPlayer(ObjectGuid const&);
		/* Find a player in all connected players by name, thread-safe. */
		TC_GAME_API Player* FindPlayerByName(std::string const& name);
		TC_GAME_API Player* FindPlayerByLowGUID(ObjectGuid::LowType lowguid);

		// this returns Player even if he is not in world, for example teleporting
		TC_GAME_API Player* FindConnectedPlayer(ObjectGuid const&);
		TC_GAME_API Player* FindConnectedPlayerByName(std::string const& name);

        /* when using this, you must use the hashmapholder's lock
         Example: boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
        */
		TC_GAME_API HashMapHolder<Player>::MapType const& GetPlayers();

        /** Add an object in hash map holder, thread-safe */
        template<class T> 
		void AddObject(T *object)
        {
            HashMapHolder<T>::Insert(object);
        }

        /** Removes an object from hash map holder, thread-safe */
        template<class T> 
		void RemoveObject(T *object)
        {
            HashMapHolder<T>::Remove(object);
        }

		template<>
		void AddObject(Player* player);

		template<>
		void RemoveObject(Player* player);

		TC_GAME_API void SaveAllPlayers();

		//void UpdateObjectVisibility(WorldObject *obj);
};

#endif
