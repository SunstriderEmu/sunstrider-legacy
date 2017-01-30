
#ifndef TRINITY_INSTANCE_DATA_H
#define TRINITY_INSTANCE_DATA_H

class Map;
class Unit;
class Player;
class GameObject;
class Creature;


class TC_GAME_API ZoneScript
{
    public:

        explicit ZoneScript(Map *map) : instance(map) {}
        virtual ~ZoneScript() = default;


        virtual uint32 GetCreatureEntry(uint32 /*guidlow*/, CreatureData const* data) { return data->id; }
        virtual uint32 GetGameObjectEntry(uint32 /*guidlow*/, uint32 entry) { return entry; }

        Map *instance;

        //On creation, NOT load.
        virtual void Initialize() {}

        //On load
        virtual void Load(const char* /*data*/) {}

        //When save is needed, this function generates the data
        virtual const char* Save() { return ""; }

        void SaveToDB();

        //Called every map update
        virtual void Update(uint32 /*diff*/) {}

        //Called when a player successfully enters the instance.
        virtual void OnPlayerEnter(Player *) {}

        //Called when a gameobject is created
        virtual void OnGameObjectCreate(GameObject *) {}

        //called on creature creation
        virtual void OnCreatureCreate(Creature * /*creature*/, uint32 /*creature_entry*/) {}
        
        // Called on creature respawn
        virtual void OnCreatureRespawn(Creature* /*creature*/, uint32 /*creature_entry*/) {}
        
        virtual void OnCreatureDeath(Creature* /*creature*/) {}

        virtual void OnCreatureRemove(Creature*) {}
        virtual void OnGameObjectRemove(GameObject*) {}

        // Called on player kill creature
        virtual void OnCreatureKill(Creature*) {}

        //All-purpose data storage 32 bit
        virtual uint32 GetData(uint32) const { return 0; }
        virtual void SetData(uint32, uint32 data) {}

        //Handle open / close objects
        //use HandleGameObject(NULL,boolen,GO); in OnGameObjectCreate in instance scripts
        //use HandleGameObject(GUID,boolen,NULL); in any other script
        void HandleGameObject(uint64 GUID, bool open, GameObject *go = nullptr);
        
        //Respawns a GO having negative spawntimesecs in gameobject-table
        void DoRespawnGameObject(uint64 uiGuid, uint32 uiTimeToDespawn = MINUTE);
        
        //change active state of doors or buttons
        void DoUseDoorOrButton(uint64 uiGuid, uint32 uiWithRestoreTime = 0, bool bUseAlternativeState = false);
};
#endif

