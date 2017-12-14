
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

enum EncounterState : uint32
{
    NOT_STARTED   = 0,
    IN_PROGRESS   = 1,
    FAIL          = 2,
    DONE          = 3,
    SPECIAL       = 4,
    TO_BE_DECIDED = 5
};

enum DoorType
{
    DOOR_TYPE_ROOM          = 0,    // Door can open if encounter is not in progress
    DOOR_TYPE_PASSAGE       = 1,    // Door can open if encounter is done
    DOOR_TYPE_SPAWN_HOLE    = 2,    // Door can open if encounter is in progress, typically used for spawning places
    MAX_DOOR_TYPES
};

struct DoorData
{
    uint32 entry, bossId;
    DoorType type;
};

struct BossBoundaryEntry
{
    uint32 BossId;
    AreaBoundary const* Boundary;
};

struct TC_GAME_API BossBoundaryData
{
    typedef std::vector<BossBoundaryEntry> StorageType;
    typedef StorageType::const_iterator const_iterator;

    BossBoundaryData(std::initializer_list<BossBoundaryEntry> data) : _data(data) { }
    ~BossBoundaryData();
    const_iterator begin() const { return _data.begin(); }
    const_iterator end() const { return _data.end(); }

private:
    StorageType _data;
};

/* NYI
struct MinionData
{
    uint32 entry, bossId;
};

struct ObjectData
{
    uint32 entry;
    uint32 type;
};
*/ 

struct BossInfo
{
    BossInfo() : state(TO_BE_DECIDED) { }
    EncounterState state;
    GuidSet door[MAX_DOOR_TYPES];
    //NYI GuidSet minion;
    CreatureBoundary boundary;
};

struct DoorInfo
{
    explicit DoorInfo(BossInfo* _bossInfo, DoorType _type)
        : bossInfo(_bossInfo), type(_type) {}
    BossInfo* bossInfo;
    DoorType type;
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
typedef std::map<uint32 /*type*/, ObjectGuid /*guid*/> ObjectGuidMap;
typedef std::map<uint32 /*entry*/, uint32 /*type*/> ObjectInfoMap;

class TC_GAME_API InstanceScript : public ZoneScript
{
    public:

        InstanceScript(Map *map);
        ~InstanceScript() override = default;

	    Map* instance;

        // On creation, NOT load.
        // PLEASE INITIALIZE FIELDS IN THE CONSTRUCTOR INSTEAD !!!
        // KEEPING THIS METHOD ONLY FOR BACKWARD COMPATIBILITY !!!
        virtual void Initialize() { }

        // On instance load, exactly ONE of these methods will ALWAYS be called:
        // if we're starting without any saved instance data
        virtual void Create();
        // if we're loading existing instance save data
        virtual void Load(char const* data);

        // When save is needed, this function generates the data
        virtual std::string GetSaveData();

        void SaveToDB();

        //Called every map update
        virtual void Update(uint32 /*diff*/) {}

        //Used by the map's CannotEnter function.
        //This is to prevent players from entering during boss encounters.
        virtual bool IsEncounterInProgress() const;

        // Called when a creature/gameobject is added to map or removed from map.
        // Insert/Remove objectguid to dynamic guid store
        virtual void OnCreatureCreate(Creature* creature) override;
        virtual void OnCreatureRemove(Creature* creature) override;

        virtual void OnGameObjectCreate(GameObject* go) override;
        virtual void OnGameObjectRemove(GameObject* go) override;

        ObjectGuid GetObjectGuid(uint32 type) const;
        virtual ObjectGuid GetGuidData(uint32 type) const override;

        Creature* GetCreature(uint32 type);
        GameObject* GetGameObject(uint32 type);

        // Called when a player successfully enters the instance.
        virtual void OnPlayerEnter(Player* /*player*/) { }

        //Handle open / close objects
        //use HandleGameObject(NULL,boolen,GO); in OnGameObjectCreate in instance scripts
        //use HandleGameObject(GUID,boolen,NULL); in any other script
        void HandleGameObject(uint64 GUID, bool open, GameObject *go = nullptr);

        // Change active state of doors or buttons
        void DoUseDoorOrButton(ObjectGuid guid, uint32 withRestoreTime = 0, bool useAlternativeState = false);
        void DoCloseDoorOrButton(ObjectGuid guid);

        // Respawns a GO having negative spawntimesecs in gameobject-table
        void DoRespawnGameObject(ObjectGuid guid, uint32 timeToDespawn = MINUTE);

        // Sends world state update to all players in instance
        void DoUpdateWorldState(uint32 worldstateId, uint32 worldstateValue);

        // Send Notify to all players in instance
        void DoSendNotifyToInstance(char const* format, ...);

        // Remove Auras due to Spell on all players in instance
        void DoRemoveAurasDueToSpellOnPlayers(uint32 spellId);

        // Cast spell on all players in instance
        void DoCastSpellOnPlayers(uint32 spell);

        virtual void MonsterPulled(Creature* creature, Unit* puller) {} // puller can be a pet, thus use a Unit ptr
        virtual void PlayerDied(Player* player) {}
        Player* GetPlayer() const; //Old function, kept for compat but avoid using

        void SendScriptInTestNoLootMessageToAll();

        virtual bool SetBossState(uint32 id, EncounterState state);
        EncounterState GetBossState(uint32 id) const { return id < bosses.size() ? bosses[id].state : TO_BE_DECIDED; }
        static char const* GetBossStateName(uint8 state);
        CreatureBoundary const* GetBossBoundary(uint32 id) const { return id < bosses.size() ? &bosses[id].boundary : nullptr; }

#ifdef LICH_KING
        // Achievement criteria additional requirements check
        // NOTE: not use this if same can be checked existed requirement types from AchievementCriteriaRequirementType
        virtual bool CheckAchievementCriteriaMeet(uint32 /*criteria_id*/, Player const* /*source*/, Unit const* /*target*/ = nullptr, uint32 /*miscvalue1*/ = 0);
#endif

        // Checks boss requirements (one boss required to kill other)
        virtual bool CheckRequiredBosses(uint32 /*bossId*/, Player const* /*player*/ = nullptr) const { return true; }

        virtual void FillInitialWorldStates(WorldPacket& /*data*/) { }

        uint32 GetEncounterCount() const { return bosses.size(); }

        static bool InstanceHasScript(WorldObject const* obj, char const* scriptName);

    protected:
        void SetHeaders(std::string const& dataHeaders);
        void SetBossNumber(uint32 number) { bosses.resize(number); }
        void LoadBossBoundaries(BossBoundaryData const& data);
        void LoadDoorData(DoorData const* data);
        //NYI void LoadMinionData(MinionData const* data);

        void AddObject(Creature* obj, bool add);
        void AddObject(GameObject* obj, bool add);
        void AddObject(WorldObject* obj, uint32 type, bool add);

        void AddDoor(GameObject* door, bool add);
        //NYI void AddMinion(Creature* minion, bool add);

        void UpdateDoorState(GameObject* door);
        //NYI void UpdateMinionState(Creature* minion, EncounterState state);

        void UpdateSpawnGroups();

        // Exposes private data that should never be modified unless exceptional cases.
        // Pay very much attention at how the returned BossInfo data is modified to avoid issues.
        BossInfo* GetBossInfo(uint32 id);

        // Instance Load and Save
        bool ReadSaveDataHeaders(std::istringstream& data);
        void ReadSaveDataBossStates(std::istringstream& data);
        virtual void ReadSaveDataMore(std::istringstream& /*data*/) { }
        void WriteSaveDataHeaders(std::ostringstream& data);
        void WriteSaveDataBossStates(std::ostringstream& data);
        virtual void WriteSaveDataMore(std::ostringstream& /*data*/) { }

        bool _SkipCheckRequiredBosses(Player const* player = nullptr) const;
    private:
        //NYI static void LoadObjectData(ObjectData const* creatureData, ObjectInfoMap& objectInfo);
        //LK? void UpdateEncounterState(EncounterCreditType type, uint32 creditEntry, Unit* source);

        std::vector<char> headers;
        std::vector<BossInfo> bosses;
        DoorInfoMap doors;
        //NYI MinionInfoMap minions;
        ObjectInfoMap _creatureInfo;
        ObjectInfoMap _gameObjectInfo;
        ObjectGuidMap _objectGuids;
#ifdef LICH_KING
        uint32 completedEncounters; // completed encounter mask, bit indexes are DungeonEncounter.dbc boss numbers, used for packets
#endif
        std::vector<InstanceSpawnGroupInfo> const* const _instanceSpawnGroups;
        //NYI std::unordered_set<uint32> _activatedAreaTriggers;

        //NYI MinionInfoMap minions;

#ifdef TRINITY_API_USE_DYNAMIC_LINKING
        // Strong reference to the associated script module
        std::shared_ptr<ModuleReference> module_reference;
#endif // #ifndef TRINITY_API_USE_DYNAMIC_LINKING
};

#endif

