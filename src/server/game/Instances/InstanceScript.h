/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

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
			
        // Default accessor functions
        Creature* GetSingleCreatureFromStorage(uint32 uiEntry, bool bSkipDebugLog = false);

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

        // Storage for GO-Guids and NPC-Guids
        typedef std::map<uint32, uint64> EntryGuidMap;
        EntryGuidMap m_mNpcEntryGuidStore;                  ///< Store unique NPC-Guids by entry

#ifdef TRINITY_API_USE_DYNAMIC_LINKING
                                                            // Strong reference to the associated script module
        std::shared_ptr<ModuleReference> module_reference;
#endif // #ifndef TRINITY_API_USE_DYNAMIC_LINKING
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntry
{
    int32 iTextEntry;                                       ///< To be said text entry
    uint32 uiSayerEntry;                                    ///< Entry of the mob who should say
    uint32 uiTimer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntryTwoSide
{
    int32 iTextEntry;                                       ///< To be said text entry (first side)
    uint32 uiSayerEntry;                                    ///< Entry of the mob who should say (first side)
    int32 iTextEntryAlt;                                    ///< To be said text entry (second side)
    uint32 uiSayerEntryAlt;                                 ///< Entry of the mob who should say (second side)
    uint32 uiTimer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// Helper class handling a dialogue given as static const array of DialogueEntry or DialogueEntryTwoSide
class TC_GAME_API DialogueHelper
{
    public:
        // The array MUST be terminated by {0,0,0}
        DialogueHelper(DialogueEntry const* pDialogueArray);
        // The array MUST be terminated by {0,0,0,0,0}
        DialogueHelper(DialogueEntryTwoSide const* aDialogueTwoSide);

        /// Function to initialize the dialogue helper for instances. If not used with instances, GetSpeakerByEntry MUST be overwritten to obtain the speakers
        void InitializeDialogueHelper(InstanceScript* pInstance) { m_pInstance = pInstance; }
        /// Set if take first entries or second entries
        void SetDialogueSide(bool bIsFirstSide) { m_bIsFirstSide = bIsFirstSide; }

        void StartNextDialogueText(int32 iTextEntry);

        void DialogueUpdate(uint32 uiDiff);

        void SetNewArray(DialogueEntry const* pDialogueArray) { m_pDialogueArray = pDialogueArray; }
        void SetNewArray(DialogueEntryTwoSide const* aDialogueTwoSide) { m_pDialogueTwoSideArray = aDialogueTwoSide; }

    protected:
        /// Will be called when a dialogue step was done
        virtual void JustDidDialogueStep(int32 /*iEntry*/) {}
        /// Will be called to get a speaker, MUST be implemented if not used in instances
        virtual Creature* GetSpeakerByEntry(uint32 /*uiEntry*/) { return nullptr; }

    private:
        void DoNextDialogueStep();

        InstanceScript* m_pInstance;

        DialogueEntry const* m_pDialogueArray;
        DialogueEntry const* m_pCurrentEntry;
        DialogueEntryTwoSide const* m_pDialogueTwoSideArray;
        DialogueEntryTwoSide const* m_pCurrentEntryTwoSide;

        uint32 m_uiTimer;
        bool m_bIsFirstSide;
};

//create AI only if instance has a script with given name
template<class AI, class T>
AI* GetInstanceAI(T* obj, char const* scriptName)
{
    if (InstanceMap* instance = obj->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId(scriptName))
                return new AI(obj);

    return NULL;
};

//create AI only if instance has a script
template<class AI, class T>
AI* GetInstanceAI(T* obj)
{
    if (InstanceMap* instance = obj->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            return new AI(obj);

    return NULL;
};

#endif

