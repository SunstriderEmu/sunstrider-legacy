
#ifndef TRINITY_INSTANCE_DATA_H
#define TRINITY_INSTANCE_DATA_H

class Map;
class Unit;
class Player;
class GameObject;
class Creature;
struct CreatureData;
class ObjectGuid;
class WorldObject;

#include "ObjectGuid.h"

class TC_GAME_API ZoneScript
{
    public:

	ZoneScript() { }
	virtual ~ZoneScript() { }

    virtual uint32 GetCreatureEntry(ObjectGuid::LowType /*guidlow*/, CreatureData const* data);
    virtual uint32 GetGameObjectEntry(ObjectGuid::LowType /*guidlow*/, uint32 entry) { return entry; }

	virtual void OnUnitDeath(Unit*) { }

    //Called when a player successfully enters the instance.
    virtual void OnPlayerEnter(Player *) {}

    //Called when a gameobject is created
    virtual void OnGameObjectCreate(GameObject *) {}
	virtual void OnGameObjectRemove(GameObject*) {}

    //called on creature creation
    virtual void OnCreatureCreate(Creature * /*creature*/) {}
	virtual void OnCreatureRemove(Creature*) {}
	// Called on creature respawn
	virtual void OnCreatureRespawn(Creature* /*creature*/) {}
	virtual void OnCreatureDeath(Creature* /*creature*/) {}
        
    // Called on player kill creature
    virtual void OnCreatureKill(Creature*) {}

	//All-purpose data storage 64 bit
	virtual ObjectGuid GetGuidData(uint32 /*DataId*/) const;
	virtual void SetGuidData(uint32 /*DataId*/, ObjectGuid /*Value*/) { }

	virtual uint64 GetData64(uint32 /*DataId*/) const { return 0; }
	virtual void SetData64(uint32 /*DataId*/, uint64 /*Value*/) { }

    //All-purpose data storage 32 bit
    virtual uint32 GetData(uint32) const { return 0; }
    virtual void SetData(uint32, uint32 data) {}

	virtual void ProcessEvent(WorldObject* /*obj*/, uint32 /*eventId*/) { }
};
#endif

