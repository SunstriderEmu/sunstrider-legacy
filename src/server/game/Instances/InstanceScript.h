
#ifndef SC_INSTANCE_H
#define SC_INSTANCE_H

#include "ZoneScript.h"
#include "Map.h"

class ModuleReference;

#define OUT_SAVE_INST_DATA             
#define OUT_SAVE_INST_DATA_COMPLETE    
#define OUT_LOAD_INST_DATA(a)          
#define OUT_LOAD_INST_DATA_COMPLETE    
#define OUT_LOAD_INST_DATA_FAIL        TC_LOG_ERROR("scripts","Unable to load Instance Data for Instance %s (Map %d, Instance Id: %d).",instance->GetMapName(), instance->GetId(), instance->GetInstanceId())

enum EncounterState
{
    NOT_STARTED = 0,
    IN_PROGRESS = 1,
    FAIL = 2,
    DONE = 3,
    SPECIAL = 4,
    TO_BE_DECIDED = 5,
};

enum BoundaryType
{
    BOUNDARY_NONE = 0,
    BOUNDARY_N,
    BOUNDARY_S,
    BOUNDARY_E,
    BOUNDARY_W,
    BOUNDARY_NE,
    BOUNDARY_NW,
    BOUNDARY_SE,
    BOUNDARY_SW,
    BOUNDARY_MAX_X = BOUNDARY_N,
    BOUNDARY_MIN_X = BOUNDARY_S,
    BOUNDARY_MAX_Y = BOUNDARY_W,
    BOUNDARY_MIN_Y = BOUNDARY_E,
};

typedef std::map<BoundaryType, float> BossBoundaryMap;

typedef std::set<GameObject*> DoorSet;
//NYI typedef std::set<Creature*> MinionSet;

enum DoorType
{
    DOOR_TYPE_ROOM = 0,    // Door can open if encounter is not in progress
    DOOR_TYPE_PASSAGE = 1,    // Door can open if encounter is done
    DOOR_TYPE_SPAWN_HOLE = 2,    // Door can open if encounter is in progress, typically used for spawning places
    MAX_DOOR_TYPES,
};

struct DoorData
{
    uint32 entry, bossId;
    DoorType type;
    uint32 boundary;
};

struct BossInfo
{
   // BossInfo() : state(TO_BE_DECIDED) {}
    //NYI, use instance data for now // EncounterState state;
    uint32 bossId; //replace state with this for now
    DoorSet door[MAX_DOOR_TYPES];
    //NYI MinionSet minion;
    BossBoundaryMap boundary;
};

struct DoorInfo
{
    explicit DoorInfo(BossInfo* _bossInfo, DoorType _type, BoundaryType _boundary)
        : bossInfo(_bossInfo), type(_type), boundary(_boundary) {}
    BossInfo* bossInfo;
    DoorType type;
    BoundaryType boundary;
};
/* NYI
struct MinionInfo
{
    explicit MinionInfo(BossInfo* _bossInfo) : bossInfo(_bossInfo) {}
    BossInfo* bossInfo;
};
*/

typedef std::multimap<uint32 /*entry*/, DoorInfo> DoorInfoMap;
typedef std::pair<DoorInfoMap::const_iterator, DoorInfoMap::const_iterator> DoorInfoMapBounds;

//NYI typedef std::map<uint32 /*entry*/, MinionInfo> MinionInfoMap;

class TC_GAME_API InstanceScript : public ZoneScript
{
    public:

        InstanceScript(Map *map);
        ~InstanceScript() override = default;

		Map* instance;
			
        //All-purpose data storage 64 bit
        virtual uint64 GetData64(uint32 Data) const { return 0; }
        virtual void SetData64(uint32 Data, uint64 Value) { }

        //Used by the map's CannotEnter function.
        //This is to prevent players from entering during boss encounters.
        virtual bool IsEncounterInProgress() const;

		//Called every map update
		virtual void Update(uint32 /*diff*/) {}

        // Save and Load instance data to the database
        virtual std::string GetSaveData() { return ""; } //TC has a more advanced system here but this would need a lot more work to transform actual scripts
        void Load(const char* in) override { }
        
        // Misc
        void CastOnAllPlayers(uint32 spellId);  // TODO: Add Unit* caster as parameter?
        void RemoveAuraOnAllPlayers(uint32 spellId);
        virtual void MonsterPulled(Creature* creature, Unit* puller); // puller can be a pet, thus use a Unit ptr
        virtual void PlayerDied(Player* player);
        
        void SendScriptInTestNoLootMessageToAll();

        BossBoundaryMap const* GetBossBoundary(uint32 id) const { return nullptr; }
        virtual bool SetBossState(uint32 id, EncounterState state);
        ///EncounterState GetBossState(uint32 id) const { return id < bosses.size() ? bosses[id].state : TO_BE_DECIDED; }
        //use Instance data for now
        EncounterState GetBossState(uint32 id) const { return EncounterState(GetData(id)); }
        BossInfo const* GetBossInfo(uint32 id) const { return &bosses[id]; }

        // Checks boss requirements (one boss required to kill other)
        virtual bool CheckRequiredBosses(uint32 /*bossId*/, Player const* /*player*/ = nullptr) const { return true; }

        Player* GetPlayer() const;

		//Handle open / close objects
		//use HandleGameObject(NULL,boolen,GO); in OnGameObjectCreate in instance scripts
		//use HandleGameObject(GUID,boolen,NULL); in any other script
		void HandleGameObject(uint64 GUID, bool open, GameObject *go = nullptr);

		//Respawns a GO having negative spawntimesecs in gameobject-table
		void DoRespawnGameObject(uint64 uiGuid, uint32 uiTimeToDespawn = MINUTE);

		//change active state of doors or buttons
		void DoUseDoorOrButton(uint64 uiGuid, uint32 uiWithRestoreTime = 0, bool bUseAlternativeState = false);

		void SaveToDB();
		//When save is needed, this function generates the data
		virtual const char* Save() { return ""; }


    protected:
        void SetBossNumber(uint32 number) { bosses.resize(number); }
        void LoadDoorData(DoorData const* data);
        //NYI void LoadMinionData(MinionData const* data);

        void AddDoor(GameObject* door, bool add);
        //NYI void AddMinion(Creature* minion, bool add);

        void UpdateDoorState(GameObject* door);
        //NYI void UpdateMinionState(Creature* minion, EncounterState state);

        std::vector<BossInfo> bosses;
        DoorInfoMap doors;
        //NYI MinionInfoMap minions;

#ifdef TRINITY_API_USE_DYNAMIC_LINKING
        // Strong reference to the associated script module
        std::shared_ptr<ModuleReference> module_reference;
#endif // #ifndef TRINITY_API_USE_DYNAMIC_LINKING
};

#endif

