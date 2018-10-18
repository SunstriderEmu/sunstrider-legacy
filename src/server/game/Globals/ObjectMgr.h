
#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Creature.h"
#include "GameObject.h"
#include "QuestDef.h"
#include "ItemTemplate.h"
#include "NPCHandler.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "ConditionMgr.h"
#include "World.h"
#include "Position.h"
#include "IteratorPair.h"

#include <string>
#include <map>
#include <limits>
#include <unordered_map>

class Group;
class Item;
enum PetNameInvalidReason : int;
class Player;
struct PlayerClassInfo;
struct PlayerClassLevelInfo;
struct PlayerInfo;
struct PlayerLevelInfo;

struct PageText
{
    std::string Text;
    uint16 NextPage;
};

enum SummonerType
{
    SUMMONER_TYPE_CREATURE = 0,
    SUMMONER_TYPE_GAMEOBJECT = 1,
    SUMMONER_TYPE_MAP = 2
};

#pragma pack(push, 1)

/// Key for storing temp summon data in TempSummonDataContainer
struct TempSummonGroupKey
{
    TempSummonGroupKey(uint32 summonerEntry, SummonerType summonerType, uint8 group)
        : _summonerEntry(summonerEntry), _summonerType(summonerType), _summonGroup(group)
    {
    }

    bool operator<(TempSummonGroupKey const& rhs) const
    {
        return std::tie(_summonerEntry, _summonerType, _summonGroup) <
            std::tie(rhs._summonerEntry, rhs._summonerType, rhs._summonGroup);
    }

private:
    uint32 _summonerEntry;      ///< Summoner's entry
    SummonerType _summonerType; ///< Summoner's type, see SummonerType for available types
    uint8 _summonGroup;         ///< Summon's group id
};

/// Stores data for temp summons
struct TempSummonData
{
    uint32 entry;        ///< Entry of summoned creature
    Position pos;        ///< Position, where should be creature spawned
    TempSummonType type; ///< Summon type, see TempSummonType for available types
    uint32 time;         ///< Despawn time, usable only with certain temp summon types
};

#pragma pack(pop)

struct GameTele
{
    float  position_x;
    float  position_y;
    float  position_z;
    float  orientation;
    uint32 mapId;
    std::string name;
    std::wstring wnameLow;
};

typedef std::unordered_map<uint32, GameTele> GameTeleContainer;

enum eScriptFlags
{    
    // Talk Flags
    SF_TALK_USE_PLAYER          = 0x1,

    // CastSpell flags
    SF_CASTSPELL_SOURCE_TO_TARGET = 0,
    SF_CASTSPELL_SOURCE_TO_SOURCE = 1,
    SF_CASTSPELL_TARGET_TO_TARGET = 2,
    SF_CASTSPELL_TARGET_TO_SOURCE = 3,
    SF_CASTSPELL_SEARCH_CREATURE  = 4,
    SF_CASTSPELL_TRIGGERED      = 0x1,

    
    // PlaySound flags
    SF_PLAYSOUND_TARGET_PLAYER  = 0x1,
    SF_PLAYSOUND_DISTANCE_SOUND = 0x2,
};


struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;

    union
    {
        struct
        {
            uint32 nData[3];
            float  fData[4];
        } Raw;

        struct                      // SCRIPT_COMMAND_TALK (0)
        {
            uint32 ChatType;        // datalong !! Not TC type, special type for scripts only...
            uint32 Flags;           // datalong2
            int32  TextID;          // dataint
        } Talk;

        struct                      // SCRIPT_COMMAND_EMOTE (1)
        {
            uint32 EmoteID;         // datalong
            uint32 Flags;           // datalong2
        } Emote;

        struct                      // SCRIPT_COMMAND_FIELD_SET (2)
        {
            uint32 FieldID;         // datalong
            uint32 FieldValue;      // datalong2
        } FieldSet;

        struct                      // SCRIPT_COMMAND_MOVE_TO (3)
        {
            uint32 Unused1;         // datalong
            uint32 TravelTime;      // datalong2
            int32  CalculateSpeed;  // dataint, added by sunstrider to keep retro compat

            float DestX;
            float DestY;
            float DestZ;
        } MoveTo;

        struct                      // SCRIPT_COMMAND_FLAG_SET (4)
                                    // SCRIPT_COMMAND_FLAG_REMOVE (5)
        {
            uint32 FieldID;         // datalong
            uint32 FieldValue;      // datalong2
        } FlagToggle;

        struct                      // SCRIPT_COMMAND_TELEPORT_TO (6)
        {
            uint32 MapID;           // datalong
            uint32 Flags;           // datalong2
            int32  Unused1;         // dataint

            float DestX;
            float DestY;
            float DestZ;
            float Orientation;
        } TeleportTo;

        struct                      // SCRIPT_COMMAND_QUEST_EXPLORED (7)
        {
            uint32 QuestID;         // datalong
            uint32 Distance;        // datalong2
        } QuestExplored;

        struct                      // SCRIPT_COMMAND_KILL_CREDIT (8)
        {
            uint32 CreatureEntry;   // datalong
            uint32 SpellID;         // datalong2 /!\ Not the same as TC, TC has Flags here
        } KillCredit;

        struct                      // SCRIPT_COMMAND_RESPAWN_GAMEOBJECT (9)
        {
            uint32 GOGuid;          // datalong
            uint32 DespawnDelay;    // datalong2
        } RespawnGameobject;

        struct                      // SCRIPT_COMMAND_TEMP_SUMMON_CREATURE (10)
        {
            uint32 CreatureEntry;   // datalong
            uint32 DespawnDelay;    // datalong2
            int32  Unused1;         // dataint

            float PosX;
            float PosY;
            float PosZ;
            float Orientation;
        } TempSummonCreature;

        struct                      // SCRIPT_COMMAND_CLOSE_DOOR (12)
                                    // SCRIPT_COMMAND_OPEN_DOOR (11)
        {
            uint32 GOGuid;          // datalong
            uint32 ResetDelay;      // datalong2
        } ToggleDoor;

        struct                      // SCRIPT_COMMAND_REMOVE_AURA (14)
        {
            uint32 SpellID;         // datalong
            uint32 Flags;           // datalong2
        } RemoveAura;

        struct                      // SCRIPT_COMMAND_CAST_SPELL (15)
        {
            uint32 SpellID;         // datalong
            uint32 Flags;           // datalong2
            int32  CreatureEntry;   // dataint

            float SearchRadius;
        } CastSpell;

        struct                      // SCRIPT_COMMAND_PLAY_SOUND (16)
        {
            uint32 SoundID;         // datalong
            uint32 Flags;           // datalong2
        } PlaySound;

        struct                      // SCRIPT_COMMAND_LOAD_PATH (20)
        {
            uint32 PathID;          // datalong
            uint32 IsRepeatable;    // datalong2
        } LoadPath;

        struct                      // SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT (21)
        {
            uint32 CreatureEntry;   // datalong
            uint32 ScriptID;        // datalong2
            uint32 ScriptType;      // dataint
        } CallScript;

        struct                      // SCRIPT_COMMAND_KILL (22)
        {
            uint32 Unused1;         // datalong
            uint32 Unused2;         // datalong2
            int32  RemoveCorpse;    // dataint
        } Kill;

        struct                      // SCRIPT_COMMAND_SMART_SET_DATA
        {
            uint32 DataID;          // datalong
            uint32 Value;           // datalong2
        } SmartSetData;
    };

    std::string GetDebugInfo() const;
    static std::string GetScriptCommandName(ScriptCommands command);
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap > ScriptMapMap;
typedef std::multimap<uint32 /*spell id*/, std::pair<uint32 /*script id*/, bool /*disabled*/>> SpellScriptsContainer;
typedef std::pair<SpellScriptsContainer::iterator, SpellScriptsContainer::iterator> SpellScriptsBounds;
TC_GAME_API extern ScriptMapMap sQuestEndScripts;
TC_GAME_API extern ScriptMapMap sQuestStartScripts;
TC_GAME_API extern ScriptMapMap sSpellScripts;
TC_GAME_API extern ScriptMapMap sGameObjectScripts;
TC_GAME_API extern ScriptMapMap sEventScripts;
TC_GAME_API extern ScriptMapMap sWaypointScripts;

struct TC_GAME_API InstanceSpawnGroupInfo
{
    enum
    {
        FLAG_ACTIVATE_SPAWN = 0x01,
        FLAG_BLOCK_SPAWN = 0x02,

        FLAG_ALL = (FLAG_ACTIVATE_SPAWN | FLAG_BLOCK_SPAWN)
    };
    uint8 BossStateId;
    uint8 BossStates;
    uint32 SpawnGroupId;
    uint8 Flags;
};

struct AreaTrigger
{
    uint32 access_id;
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};

struct BroadcastText
{
    BroadcastText() : Id(0), Language(0), EmoteId0(0), EmoteId1(0), EmoteId2(0),
                      EmoteDelay0(0), EmoteDelay1(0), EmoteDelay2(0), SoundId(0), Unk1(0), Unk2(0)
    {
        MaleText.resize(DEFAULT_LOCALE + 1);
        FemaleText.resize(DEFAULT_LOCALE + 1);
    }

    uint32 Id;
    uint32 Language;
    StringVector MaleText;
    StringVector FemaleText;
    uint32 EmoteId0;
    uint32 EmoteId1;
    uint32 EmoteId2;
    uint32 EmoteDelay0;
    uint32 EmoteDelay1;
    uint32 EmoteDelay2;
    uint32 SoundId;
    uint32 Unk1;
    uint32 Unk2;
    // uint32 VerifiedBuild;

    std::string const& GetText(LocaleConstant locale = DEFAULT_LOCALE, uint8 gender = GENDER_MALE, bool forceGender = false) const
    {
        if ((gender == GENDER_FEMALE || gender == GENDER_NONE) && (forceGender || !FemaleText[DEFAULT_LOCALE].empty()))
        {
            if (FemaleText.size() > size_t(locale) && !FemaleText[locale].empty())
                return FemaleText[locale];
            return FemaleText[DEFAULT_LOCALE];
        }
        // else if (gender == GENDER_MALE)
        {
            if (MaleText.size() > size_t(locale) && !MaleText[locale].empty())
                return MaleText[locale];
            return MaleText[DEFAULT_LOCALE];
        }
    }
};

typedef std::unordered_map<uint32, BroadcastText> BroadcastTextContainer;

typedef std::set<uint32> CellGuidSet;
typedef std::map<uint32/*player guid*/,uint32/*instance*/> CellCorpseSet;
struct CellObjectGuids
{
    CellGuidSet creatures;
    CellGuidSet gameobjects;
    CellCorpseSet corpses;
};
typedef std::unordered_map<uint32/*cell_id*/,CellObjectGuids> CellObjectGuidsMap;
typedef std::unordered_map<uint32/*(mapid,spawnMode) pair*/,CellObjectGuidsMap> MapObjectGuids;

// mangos string ranges
#define MIN_TRINITY_STRING_ID    1
#define MAX_TRINITY_STRING_ID    2000000000
#define MIN_DB_SCRIPT_STRING_ID MAX_TRINITY_STRING_ID
#define MAX_DB_SCRIPT_STRING_ID 2000010000

struct TrinityStringLocale
{
    std::vector<std::string> Content;                       // 0 -> default, i -> i-1 locale index
};

struct QuestGreetingLocale
{
    std::vector<std::string> greeting;
};

typedef std::unordered_map<uint32,CreatureData> CreatureDataContainer;
typedef std::unordered_map<uint32,GameObjectData> GameObjectDataContainer;
typedef std::unordered_map<uint32,CreatureLocale> CreatureLocaleContainer;
typedef std::unordered_map<uint32,GameObjectLocale> GameObjectLocaleContainer;
typedef std::unordered_map<uint32,ItemLocale> ItemLocaleContainer;
typedef std::unordered_map<uint32,QuestLocale> QuestLocaleContainer;
typedef std::unordered_map<uint32,NpcTextLocale> NpcTextLocaleContainer;
typedef std::unordered_map<uint32, QuestGreetingLocale> QuestGreetingLocaleContainer;
typedef std::unordered_map<uint32,PageTextLocale> PageTextLocaleContainer;
typedef std::unordered_map<uint32,TrinityStringLocale> TrinityStringLocaleContainer;
typedef std::unordered_map<uint32,GossipMenuItemsLocale> GossipMenuItemsLocaleContainer;
typedef std::unordered_map<uint32, PointOfInterestLocale> PointOfInterestLocaleContainer;
typedef std::unordered_map<uint32, SpawnGroupTemplateData> SpawnGroupDataContainer;
typedef std::multimap<uint32, SpawnData const*> SpawnGroupLinkContainer;
typedef std::unordered_map<uint16, std::vector<InstanceSpawnGroupInfo>> InstanceSpawnGroupContainer;
typedef std::map<TempSummonGroupKey, std::vector<TempSummonData>> TempSummonDataContainer;

typedef std::multimap<uint32,uint32> QuestRelations;

// Benchmarked: Faster than std::unordered_map (insert/find)
typedef std::map<uint32, PageText> PageTextContainer;

// Benchmarked: Faster than std::map (insert/find)
typedef std::unordered_map<uint16, InstanceTemplate> InstanceTemplateContainer;
typedef std::unordered_map<uint16, InstanceTemplateAddon> InstanceTemplateAddonContainer;

struct PetLevelInfo
{
    PetLevelInfo() : health(0), mana(0), minDamage(0), maxDamage(0), armor(0), stats()
    { 
    }

    uint16 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
    uint32 armor;
    uint16 minDamage;
    uint16 maxDamage;
};

struct ReputationOnKillEntry
{
    uint32 RepFaction1;
    uint32 RepFaction2;
    bool is_teamaward1;
    uint32 ReputationMaxCap1;
    int32 RepValue1;
    bool is_teamaward2;
    uint32 ReputationMaxCap2;
    int32 RepValue2;
    bool TeamDependent;
};

struct PetCreateSpellEntry
{
    uint32 spellid[4];
};

#define WEATHER_SEASONS 4
struct WeatherSeasonChances
{
    uint32 rainChance;
    uint32 snowChance;
    uint32 stormChance;
};

struct WeatherZoneChances
{
    WeatherSeasonChances data[WEATHER_SEASONS];
};

struct GraveYardData
{
    uint32 safeLocId;
    uint32 team;
};
typedef std::multimap<uint32,GraveYardData> GraveYardMap;

// Generic items are loaded in the same container as creature specific items, under menu id 0
#define GENERIC_OPTIONS_MENU 0

// NPC gossip text id
typedef std::unordered_map<uint32, uint32> CacheNpcTextIdMap;
typedef std::unordered_map<uint32, uint32> CacheGoTextIdMap;

typedef std::unordered_map<uint32, VendorItemData> CacheVendorItemMap;
typedef std::unordered_map<uint32, TrainerSpellData> CacheTrainerSpellMap;

typedef std::unordered_map<uint32, ItemExtendedCostEntry> ItemExtendedStore;

struct QuestGreeting
{
    uint16 greetEmoteType;
    uint32 greetEmoteDelay;
    std::string greeting;

    QuestGreeting() : greetEmoteType(0), greetEmoteDelay(0) { }
    QuestGreeting(uint16 _greetEmoteType, uint32 _greetEmoteDelay, std::string _greeting)
        : greetEmoteType(_greetEmoteType), greetEmoteDelay(_greetEmoteDelay), greeting(_greeting) { }
};

typedef std::unordered_map<uint8, std::unordered_map<uint32, QuestGreeting>> QuestGreetingContainer;

enum SkillRangeType
{
    SKILL_RANGE_LANGUAGE,                                   // 300..300
    SKILL_RANGE_LEVEL,                                      // 1..max skill for level
    SKILL_RANGE_MONO,                                       // 1..1, grey monolite bar
    SKILL_RANGE_RANK,                                       // 1..skill for known rank
    SKILL_RANGE_NONE,                                       // 0..0 always
};

SkillRangeType GetSkillRangeType(SkillRaceClassInfoEntry const* rcEntry);

struct GM_Ticket
{
  uint64 guid;
  ObjectGuid playerGuid;
  float pos_x;
  float pos_y;
  float pos_z;
  uint32 map;
  std::string message;
  uint64 createtime;
  uint64 timestamp;
  int64 closed; // 0 = Open, -1 = Console, playerGuid = player abandoned ticket, other = GM who closed it.
  ObjectGuid assignedToGM;
  std::string comment;
};
typedef std::list<GM_Ticket*> GmTicketList;

struct PointOfInterest
{
    uint32 entry;
    float x;
    float y;
    uint32 icon;
    uint32 flags;
    uint32 data;
    std::string icon_name;
};

struct GossipMenuItems
{
    GossipMenuItems() :
        MenuId(0),
        OptionIndex(0),
        OptionIcon(0),
        OptionText(""),
        OptionBroadcastTextId(0),
        OptionType(0),
        OptionNpcflag(0),
        ActionMenuId(0),
        ActionPoiId(0),
        BoxCoded(false),
        BoxMoney(0),
        BoxText(""),
        BoxBroadcastTextId(0)
    {}

    uint32          MenuId;
    uint32          OptionIndex;
    uint8           OptionIcon;
    std::string     OptionText;
    uint32          OptionBroadcastTextId;
    uint32          OptionType;
    uint32          OptionNpcflag;
    uint32          ActionMenuId;
    uint32          ActionPoiId;
    bool            BoxCoded;
    uint32          BoxMoney;
    std::string     BoxText;
    uint32          BoxBroadcastTextId;
    ConditionContainer   Conditions;
};

struct GossipMenus
{
    uint32          entry;
    uint32          text_id;
    ConditionContainer   conditions;
};

typedef std::multimap<uint32, GossipMenuItems> GossipMenuItemsContainer;

typedef std::list<GossipMenuItems> GossipOptionList;

typedef std::multimap<uint32, GossipMenus> GossipMenusContainer;
typedef std::pair<GossipMenusContainer::const_iterator, GossipMenusContainer::const_iterator> GossipMenusMapBounds;
typedef std::pair<GossipMenusContainer::iterator, GossipMenusContainer::iterator> GossipMenusMapBoundsNonConst;
typedef std::multimap<uint32, GossipMenuItems> GossipMenuItemsContainer;
typedef std::pair<GossipMenuItemsContainer::const_iterator, GossipMenuItemsContainer::const_iterator> GossipMenuItemsMapBounds;
typedef std::pair<GossipMenuItemsContainer::iterator, GossipMenuItemsContainer::iterator> GossipMenuItemsMapBoundsNonConst;


typedef std::map<uint32, SpellEntry*> SpellEntryStore;

#define MAX_PLAYER_NAME 12                                  // max allowed by client name length
#define MAX_INTERNAL_PLAYER_NAME 15                         // max server internal player name length ( > MAX_PLAYER_NAME for support declined names )
#define MAX_PET_NAME             12                         // max allowed by client name length
#define MAX_CHARTER_NAME         24                         // max allowed by client name length

TC_GAME_API bool normalizePlayerName(std::string& name);
#define SPAWNGROUP_MAP_UNSET            0xFFFFFFFF

struct TC_GAME_API LanguageDesc
{
    Language lang_id;
    uint32   spell_id;
    uint32   skill_id;
};

TC_GAME_API extern LanguageDesc lang_description[LANGUAGES_COUNT];
LanguageDesc const* GetLanguageDescByID(uint32 lang);

enum QueryDataGroup
{
    QUERY_DATA_CREATURES        = 0x01,
    QUERY_DATA_GAMEOBJECTS      = 0x02,
    QUERY_DATA_ITEMS            = 0x04,
    QUERY_DATA_QUESTS           = 0x08,
#ifdef LICH_KING
    QUERY_DATA_POIS             = 0x10,
#endif

    QUERY_DATA_ALL              = 0xFF
};

class PlayerDumpReader;

class TC_GAME_API ObjectMgr
{
    friend class PlayerDumpReader;
    friend class TestCase;

    public:
        ObjectMgr();
        ~ObjectMgr();

        static ObjectMgr* instance()
        {
            static ObjectMgr instance;
            return &instance;
        }

        typedef std::unordered_map<uint32, Item*> ItemMap;
        typedef std::set<Group*> GroupSet;
        typedef std::unordered_map<uint32, Quest> QuestContainer;
        typedef std::unordered_map<uint32, AreaTrigger> AreaTriggerMap;
        typedef std::unordered_map<uint32, uint32> AreaTriggerScriptMap;
        typedef std::unordered_map<uint32, AccessRequirement> AccessRequirementContainer;
        typedef std::unordered_map<uint32, ReputationOnKillEntry> RepOnKillMap;
        typedef std::unordered_map<uint32, WeatherZoneChances> WeatherZoneMap;
        typedef std::unordered_map<uint32, PointOfInterest> PointOfInterestContainer;
        typedef std::vector<std::string> ScriptNameContainer;
        typedef std::unordered_map<uint32, PetCreateSpellEntry> PetCreateSpellMap;
        typedef std::vector<std::string> ScriptNameMap;

        std::unordered_map<uint32, uint32> TransportEventMap;

        GameObjectTemplate const* GetGameObjectTemplate(uint32 id);
        GameObjectTemplateContainer const* GetGameObjectTemplateStore() const { return &_gameObjectTemplateStore; }
        GameObject* CreateGameObject(uint32 entry);

        void LoadGameObjectTemplate();

        CreatureTemplateContainer const& GetCreatureTemplates() const { return _creatureTemplateStore; }
        CreatureTemplate const* GetCreatureTemplate( uint32 id );
        CreatureModelInfo const* GetCreatureModelInfo( uint32 modelid );
        //return a new displayId with same gender and race as baseModel, if possible. Else return model info for displayID. Race NYI
        CreatureModelInfo const* GetCreatureModelSameGenderAndRaceAs(uint32& displayID, uint32 baseDisplayId);
        CreatureModelInfo const* GetCreatureModelRandomGender(uint32& displayID);
        static uint32 ChooseDisplayId(const CreatureTemplate *cinfo, const CreatureData *data = nullptr);
        static void ChooseCreatureFlags(CreatureTemplate const* cinfo, uint32& npcflag, uint32& unit_flags, uint32& dynamicflags, CreatureData const* data = nullptr);
        EquipmentInfo const* GetEquipmentInfo(uint32 entry, int8& id) const;
        CreatureAddon const* GetCreatureAddon(ObjectGuid::LowType lowguid) const;
        CreatureAddon const* GetCreatureTemplateAddon(uint32 entry) const;
        CreatureMovementData const* GetCreatureMovementOverride(ObjectGuid::LowType spawnId) const;

        ItemTemplate const* GetItemTemplate(uint32 id);
        ItemTemplateContainer const& GetItemTemplateStore() const { return _itemTemplateStore; }
        ItemExtendedCostEntry const* GetItemExtendedCost(uint32 id) const;

        InstanceTemplate const* GetInstanceTemplate(uint32 map);
        InstanceTemplateContainer const* GetInstanceTemplateStore() const { return &_instanceTemplateStore; }

        InstanceTemplateAddon const* GetInstanceTemplateAddon(uint32 map);

        PetLevelInfo const* GetPetLevelInfo(uint32 creature_id, uint32 level) const;

        PlayerClassInfo const* GetPlayerClassInfo(uint32 class_) const 
        {
            return class_ < MAX_CLASSES ? _playerClassInfo[class_].get() : nullptr; 
        }
        void GetPlayerClassLevelInfo(uint32 class_,uint32 level, PlayerClassLevelInfo* info) const;

		PlayerInfo const* GetPlayerInfo(uint32 race, uint32 class_) const;
        void GetPlayerLevelInfo(uint32 race, uint32 class_,uint32 level, PlayerLevelInfo* info) const;

        uint32 GetNearestTaxiNode( float x, float y, float z, uint32 mapid, uint32 team);
        void GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint32 GetTaxiMountDisplayId(uint32 id, uint32 team, bool allowed_alt_team = false);

        Quest const* GetQuestTemplate(uint32 quest_id) const;
        QuestContainer const& GetQuestTemplates() const { return _questTemplates; }

        uint32 GetQuestForAreaTrigger(uint32 Trigger_ID) const
        {
            auto itr = _questAreaTriggerStore.find(Trigger_ID);
            if(itr != _questAreaTriggerStore.end())
                return itr->second;
            return 0;
        }
        bool IsTavernAreaTrigger(uint32 Trigger_ID) const { return _tavernAreaTriggerStore.count(Trigger_ID) != 0; }
        bool IsGameObjectForQuests(uint32 entry) const { return _gameObjectForQuestStore.count(entry) != 0; }

        BattlegroundTypeId GetBattleMasterBG(uint32 entry) const;

        GossipText const* GetGossipText(uint32 Text_ID);
        QuestGreeting const* GetQuestGreeting(ObjectGuid guid) const;

        WorldSafeLocsEntry const *GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team);
        bool AddGraveYardLink(uint32 id, uint32 zone, uint32 team, bool inDB = true);
        void RemoveGraveYardLink(uint32 id, uint32 zone, uint32 team, bool inDB = false);
        void LoadGraveyardZones();
        GraveYardData const* FindGraveYardData(uint32 id, uint32 zone);

		AreaTrigger const* GetAreaTrigger(uint32 trigger) const;

		AccessRequirement const* GetAccessRequirement(uint32 requirement) const;

        AreaTrigger const* GetGoBackTrigger(uint32 Map) const;
        AreaTrigger const* GetMapEntranceTrigger(uint32 Map) const;

        uint32 GetAreaTriggerScriptId(uint32 trigger_id);
        SpellScriptsBounds GetSpellScriptsBounds(uint32 spell_id);

        ReputationOnKillEntry const* GetReputationOnKilEntry(uint32 id) const
        {
            auto itr = _repOnKillStore.find(id);
            if(itr != _repOnKillStore.end())
                return &itr->second;
            return nullptr;
        }

        PointOfInterest const* GetPointOfInterest(uint32 id) const
        {
            auto itr = _pointsOfInterestStore.find(id);
            if (itr != _pointsOfInterestStore.end())
                return &itr->second;
            return nullptr;
        }

        PetCreateSpellEntry const* GetPetCreateSpellEntry(uint32 id) const
        {
            auto itr = mPetCreateSpell.find(id);
            if(itr != mPetCreateSpell.end())
                return &itr->second;
            return nullptr;
        }

        void LoadQuests();
        void LoadQuestRelations()
        {
            LoadGameobjectQuestStarters();
            LoadGameobjectQuestEnders();
            LoadCreatureQuestStarters();
            LoadCreatureQuestEnders();
        }
        void LoadGameobjectQuestStarters();
        void LoadGameobjectQuestEnders();
        void LoadCreatureQuestStarters();
        void LoadCreatureQuestEnders();

        QuestRelations mGOQuestRelations;
        QuestRelations mGOQuestInvolvedRelations;
        QuestRelations mCreatureQuestRelations;
        QuestRelations mCreatureQuestInvolvedRelations;

        void LoadGameObjectScripts();
        void LoadQuestEndScripts();
        void LoadQuestStartScripts();
        void LoadEventScripts();
        void LoadSpellScripts();
        void LoadWaypointScripts();

        void LoadSpellScriptNames();
        void ValidateSpellScripts();

        bool LoadTrinityStrings(DatabaseWorkerPool<WorldDatabaseConnection>& db, char const* table, int32 min_value, int32 max_value);
        bool LoadTrinityStrings() { return LoadTrinityStrings(WorldDatabase,"trinity_string",MIN_TRINITY_STRING_ID,MAX_TRINITY_STRING_ID); }
        void LoadPetCreateSpells();
        void LoadBroadcastTexts();
        void LoadBroadcastTextLocales();
        void LoadCreatureLocales();
        void LoadCreatureClassLevelStats();
        void LoadCreatureTemplates(bool reload = false);
        void LoadCreatureTemplate(Field* fields);
        void CheckCreatureTemplate(CreatureTemplate const* cInfo);
        void CheckCreatureMovement(char const* table, uint64 id, CreatureMovementData& creatureMovement);
        void LoadTempSummons();
        void LoadCreatures();
        void LoadCreatureAddons();
        void LoadCreatureTemplateAddons();
        void LoadCreatureModelInfo();
        void LoadEquipmentTemplates();
        void LoadCreatureMovementOverrides();

        void LoadGameObjectLocales();
        void LoadGameObjects();
        void LoadSpawnGroupTemplates();
        void LoadSpawnGroups();
        void LoadInstanceSpawnGroups();
        void LoadItemTemplates();
        void LoadItemLocales();
        void LoadItemExtendedCost();
        void LoadQuestLocales();
        void LoadGossipTextLocales();
        void LoadPageTextLocales();
        void LoadGossipMenuItemsLocales();
        void LoadQuestGreetingsLocales();
        void LoadInstanceTemplate();

        void LoadGossipText();

        void LoadAreaTriggerTeleports();
        void LoadAccessRequirements();
        void LoadQuestAreaTriggers();
        void LoadAreaTriggerScripts();
        void LoadQuestGreetings();
        void LoadTavernAreaTriggers();
        void LoadBattleMastersEntry();
        void LoadGameObjectForQuests();

        void LoadPageTexts();
        PageText const* GetPageText(uint32 pageEntry);

        void LoadPlayerInfo();
        void LoadPetLevelInfo();
        void LoadExplorationBaseXP();
        void LoadPetNames();
        void LoadPetNumber();
        void LoadFishingBaseSkillLevel();

        void LoadReputationOnKill();

        void LoadPointsOfInterest();
        void LoadQuestPOI();

        void LoadWeatherZoneChances();
        void LoadGameTele();

        void LoadGossipMenu();
        //load both gossip_menu_option and gossip_menu_option_generic
        void LoadGossipMenuItems();
        void LoadCreatureGossip();
        void LoadVendors();
        void LoadTrainerSpell();

        void InitializeQueriesData(QueryDataGroup mask);

        void LoadGMTickets();

        /* (Re)Load spell_template from database, and apply some hardcoded changes with LoadSpellCustomAttr(). You still need to reload SpellInfo's with LoadSpellInfoStore if you want this data to be used.
        */
        void LoadSpellTemplates();
        SpellEntry const* GetSpellTemplate(uint32 id) const;
        SpellEntryStore& GetSpellStore() { return spellTemplates; }
        uint32 GetMaxSpellId() { return maxSpellId; }
        
        std::string GeneratePetName(uint32 entry);
        uint32 GetBaseXP(uint32 level);
		uint32 GetXPForLevel(uint8 level) const;

        int32 GetFishingBaseSkillLevel(uint32 entry) const
        {
            auto itr = mFishingBaseForArea.find(entry);
            return itr != mFishingBaseForArea.end() ? itr->second : 0;
        }

        void ReturnOrDeleteOldMails(bool serverUp);

        CreatureBaseStats const* GetCreatureBaseStats(uint8 level, uint8 unitClass);

        void SetHighestGuids();

		template<HighGuid type>
		inline ObjectGuidGeneratorBase& GetGenerator()
		{
			static_assert(ObjectGuidTraits<type>::Global, "Only global guid can be generated in ObjectMgr context");
			return GetGuidSequenceGenerator<type>();
		}

        uint32 GenerateAuctionID();
        uint32 GenerateMailID();
        uint32 GenerateItemTextID();
        uint32 GeneratePetNumber();
		uint32 GenerateCreatureSpawnId();
		uint32 GenerateGameObjectSpawnId();

        SpawnGroupTemplateData const* GetSpawnGroupData(uint32 groupId) const { auto it = _spawnGroupDataStore.find(groupId); return it != _spawnGroupDataStore.end() ? &it->second : nullptr; }
        SpawnGroupTemplateData const* GetDefaultSpawnGroup() const { return &_spawnGroupDataStore.at(0); }
        SpawnGroupTemplateData const* GetLegacySpawnGroup() const { return &_spawnGroupDataStore.at(1); }
        Trinity::IteratorPair<SpawnGroupLinkContainer::const_iterator> GetSpawnDataForGroup(uint32 groupId) const { return Trinity::Containers::MapEqualRange(_spawnGroupMapStore, groupId); }
        std::vector<InstanceSpawnGroupInfo> const* GetSpawnGroupsForInstance(uint32 instanceId) const { auto it = _instanceSpawnGroupStore.find(instanceId); return it != _instanceSpawnGroupStore.end() ? &it->second : nullptr; }

        uint32 CreateItemText(SQLTransaction& charTrans, std::string const& text);
        uint32 CreateItemText(std::string const& text);
        std::string GetItemText(uint32 id);

        typedef std::multimap<int32, uint32> ExclusiveQuestGroups;
        ExclusiveQuestGroups mExclusiveQuestGroups;

        WeatherZoneChances const* GetWeatherChances(uint32 zone_id) const
        {
            auto itr = mWeatherZoneMap.find(zone_id);
            if(itr != mWeatherZoneMap.end())
                return &itr->second;
            else
                return nullptr;
        }

        CellObjectGuids const& GetCellObjectGuids(uint16 mapid, uint8 spawnMode, uint32 cell_id)
        {
            return _mapObjectGuidsStore[MAKE_PAIR32(mapid,spawnMode)][cell_id];
        }

        CellObjectGuidsMap const& GetMapObjectGuids(uint16 mapid, uint8 spawnMode)
        {
            return _mapObjectGuidsStore[MAKE_PAIR32(mapid, spawnMode)];
        }

        /**
        * Gets temp summon data for all creatures of specified group.
        *
        * @param summonerId   Summoner's entry.
        * @param summonerType Summoner's type, see SummonerType for available types.
        * @param group        Id of required group.
        *
        * @return null if group was not found, otherwise reference to the creature group data
        */
        std::vector<TempSummonData> const* GetSummonGroup(uint32 summonerId, SummonerType summonerType, uint8 group) const
        {
            TempSummonDataContainer::const_iterator itr = _tempSummonDataStore.find(TempSummonGroupKey(summonerId, summonerType, group));
            if (itr != _tempSummonDataStore.end())
                return &itr->second;

            return nullptr;
        }

        BroadcastText const* GetBroadcastText(uint32 id) const
        {
            return Trinity::Containers::MapGetValuePtr(_broadcastTextStore, id);
        }

        SpawnData const* GetSpawnData(SpawnObjectType type, ObjectGuid::LowType guid)
        {
            if (type == SPAWN_TYPE_CREATURE)
                return GetCreatureData(guid);
            else if (type == SPAWN_TYPE_GAMEOBJECT)
                return GetGameObjectData(guid);
            else
                ASSERT(false, "Invalid spawn object type %u", uint32(type));
            return nullptr;
        }
        void OnDeleteSpawnData(SpawnData const* data);
        CreatureDataContainer const& GetAllCreatureData() const { return _creatureDataStore; }
        CreatureData const* GetCreatureData(ObjectGuid::LowType guid) const
        {
            return Trinity::Containers::MapGetValuePtr(_creatureDataStore, guid);
        }
        CreatureData& NewOrExistCreatureData(ObjectGuid::LowType guid) { return _creatureDataStore[guid]; }
        void DeleteCreatureData(ObjectGuid::LowType guid);
        CreatureLocale const* GetCreatureLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_creatureLocaleStore, entry);
        }
        GameObjectLocale const* GetGameObjectLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_gameObjectLocaleStore, entry);
        }
        ItemLocale const* GetItemLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_itemLocaleStore, entry);
        }
        QuestLocale const* GetQuestLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_questLocaleStore, entry);
        }
        NpcTextLocale const* GetNpcTextLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(mGossipTextLocaleMap, entry);
        }
        PageTextLocale const* GetPageTextLocale(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(mPageTextLocaleMap, entry);
        }
        GossipMenuItemsLocale const* GetGossipMenuItemsLocale(uint16 menuId, uint16 optionIndex) const
        {
            return Trinity::Containers::MapGetValuePtr(_gossipMenuItemsLocaleStore, MAKE_PAIR32(menuId, optionIndex));
        }
        PointOfInterestLocale const* GetPointOfInterestLocale(uint32 poi_id) const
        {
            return Trinity::Containers::MapGetValuePtr(_pointOfInterestLocaleStore, poi_id);
        }

        GameObjectData const* GetGameObjectData(ObjectGuid::LowType guid) const
        {
            return Trinity::Containers::MapGetValuePtr(_gameObjectDataStore, guid);
        }

        GameObjectDataContainer const& GetAllGameObjectData() const
        {
            return _gameObjectDataStore;
        }

        GameObjectData& NewOrExistGameObjectData(ObjectGuid::LowType guid) { return _gameObjectDataStore[guid]; }
        void DeleteGameObjectData(ObjectGuid::LowType guid);

        QuestGreetingLocale const* GetQuestGreetingLocale(uint32 id) const
        {
            return Trinity::Containers::MapGetValuePtr(_questGreetingLocaleStore, id);
        }

        TrinityStringLocale const* GetTrinityStringLocale(int32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(_trinityStringStore, entry);
        }

        static void AddLocaleString(std::string const& s, LocaleConstant locale, StringVector& data);
        static inline void GetLocaleString(const StringVector& data, int loc_idx, std::string& value)
        {
            if (data.size() > size_t(loc_idx) && !data[loc_idx].empty())
                value = data[loc_idx];
        }

        const char *GetTrinityString(int32 entry, LocaleConstant locale_idx) const;
        const char *GetTrinityStringForDBCLocale(int32 entry) const { return GetTrinityString(entry,DBCLocaleIndex); }
        LocaleConstant GetDBCLocaleIndex() const { return DBCLocaleIndex; }
        void SetDBCLocaleIndex(LocaleConstant locale) { DBCLocaleIndex = locale; }

		/*
        void AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance);
        void DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid);
		*/

        // grid objects. Grids object are only used to load new cells
        bool AddCreatureToGrid(ObjectGuid::LowType spawnId, CreatureData const* data);
        void RemoveCreatureFromGrid(ObjectGuid::LowType spawnId, CreatureData const* data);
        bool AddGameobjectToGrid(ObjectGuid::LowType spawnId, GameObjectData const* data);
        void RemoveGameobjectFromGrid(ObjectGuid::LowType spawnId, GameObjectData const* data);
        uint32 AddGameObjectData(uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0, float rotation0 = 0, float rotation1 = 0, float rotation2 = 0, float rotation3 = 0);
        uint32 AddCreatureData(uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0);

        // reserved names
        void LoadReservedPlayersNames();
        bool IsReservedName(const std::string& name) const
        {
            return _reservedNamesStore.find(name) != _reservedNamesStore.end();
        }

        // name with valid structure and symbols
        static ResponseCodes CheckPlayerName( const std::string& name, bool create = false );
        static PetNameInvalidReason CheckPetName( const std::string& name );
        static bool IsValidCharterName( const std::string& name );

        static bool CheckDeclinedNames(std::wstring mainpart, DeclinedName const& names);

        void LoadSpellDisabledEntrys();
        bool IsPlayerSpellDisabled(uint32 spellid) { return (m_DisabledPlayerSpells.count(spellid) != 0); }
        bool IsCreatureSpellDisabled(uint32 spellid) { return (m_DisabledCreatureSpells.count(spellid) != 0); }
        bool IsPetSpellDisabled(uint32 spellid) { return (m_DisabledPetSpells.count(spellid) != 0); }

        GameTele const* GetGameTele(uint32 id) const
        {
            auto itr = m_GameTeleMap.find(id);
            if(itr==m_GameTeleMap.end()) 
                return nullptr;

            return &itr->second;
        }
        GameTele const* GetGameTele(const std::string& name) const;
        GameTeleContainer const& GetGameTeleMap() const { return m_GameTeleMap; }
        bool AddGameTele(GameTele& data);
        bool DeleteGameTele(const std::string& name);

        GossipMenusMapBounds GetGossipMenusMapBounds(uint32 uiMenuId) const
        {
            return _gossipMenusStore.equal_range(uiMenuId);
        }

        GossipMenusMapBoundsNonConst GetGossipMenusMapBoundsNonConst(uint32 uiMenuId)
        {
            return _gossipMenusStore.equal_range(uiMenuId);
        }

        GossipMenuItemsMapBounds GetGossipMenuItemsMapBounds(uint32 uiMenuId) const
        {
            return _gossipMenuItemsStore.equal_range(uiMenuId);
        }
        GossipMenuItemsMapBoundsNonConst GetGossipMenuItemsMapBoundsNonConst(uint32 uiMenuId)
        {
            return _gossipMenuItemsStore.equal_range(uiMenuId);
        }

        uint32 GetNpcGossipMenu(ObjectGuid::LowType guid) const
        {
            auto iter = m_mCacheNpcMenuIdMap.find(guid);
            if(iter == m_mCacheNpcMenuIdMap.end())
                return 0;

            return iter->second;
        }

        uint32 GetGameobjectGossipMenu(ObjectGuid::LowType guid) const
        {
            auto iter = m_mCacheGoMenuIdMap.find(guid);
            if(iter == m_mCacheGoMenuIdMap.end())
                return 0;

            return iter->second;
        }

        TrainerSpellData const* GetNpcTrainerSpells(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(m_mCacheTrainerSpellMap, entry);
        }

        VendorItemData const* GetNpcVendorItemList(uint32 entry) const
        {
            return Trinity::Containers::MapGetValuePtr(m_mCacheVendorItemMap, entry);
        }
        void AddVendorItem(uint32 entry,ItemTemplate const *proto, uint32 maxcount, uint32 incrtime, uint32 ExtendedCost, bool savetodb = false); // for event
        bool RemoveVendorItem(uint32 entry,ItemTemplate const *proto, bool savetodb = false); // for event
        bool IsVendorItemValid( uint32 vendor_entry, ItemTemplate const *proto, uint32 maxcount, uint32 ptime, uint32 ExtendedCost, Player* pl = nullptr, std::set<uint32>* skip_vendors = nullptr, uint32 ORnpcflag = 0 ) const;
        bool AddTrainerSpell(uint32 creatureId, TrainerSpell const& spell);
        bool RemoveTrainerSpell(uint32 creatureId, uint32 spellId);

        void LoadScriptNames();
        ScriptNameContainer const& GetAllScriptNames() const;
        std::string const& GetScriptName(uint32 id) const;
        uint32 GetScriptId(std::string const& name);

        GM_Ticket* GetGMTicket(uint64 ticketGuid);
        GM_Ticket* GetGMTicketByPlayer(ObjectGuid playerGuid);
        void AddOrUpdateGMTicket(GM_Ticket &ticket, bool create = false);
        void _AddOrUpdateGMTicket(GM_Ticket &ticket);
        void RemoveGMTicket(uint64 ticketGuid, int64 source = -1, bool permanently = false);
        void RemoveGMTicket(GM_Ticket *ticket, int64 source = -1, bool permanently = false);
        GmTicketList m_GMTicketList;
        uint64 GenerateGMTicketId();

        typedef std::map<uint32, uint32> FactionChangeMap;
        
        FactionChangeMap factionchange_items;
        FactionChangeMap factionchange_spells;
        FactionChangeMap factionchange_titles;
        FactionChangeMap factionchange_quests;
        FactionChangeMap factionchange_reput_generic;
        
        void LoadFactionChangeItems();
        void LoadFactionChangeSpells();
        void LoadFactionChangeTitles();
        void LoadFactionChangeQuests();
        void LoadFactionChangeReputGeneric();

        bool IsTransportMap(uint32 mapId) const { return _transportMaps.count(mapId); }

#ifndef LICH_KING
        //storage used instead of the PvPDifficulty.dbc, which does not exists on BC
        std::map<uint32, PvPDifficultyEntry> BCDifficultyEntries;
#endif

    protected:

        // first free id for selected id type
        uint32 _auctionId;
		std::atomic<uint32> _mailid;
        uint32 _ItemTextId;
		std::atomic<uint32> _hiPetNumber;
        uint64 _GMticketid;

		uint32 _creatureSpawnId;
		uint32 _gameObjectSpawnId;

		// first free low guid for selected guid type
		template<HighGuid high>
		inline ObjectGuidGeneratorBase& GetGuidSequenceGenerator()
		{
			auto itr = _guidGenerators.find(high);
			if (itr == _guidGenerators.end())
				itr = _guidGenerators.insert(std::make_pair(high, std::unique_ptr<ObjectGuidGenerator<high>>(new ObjectGuidGenerator<high>()))).first;

			return *itr->second;
		}
		std::map<HighGuid, std::unique_ptr<ObjectGuidGeneratorBase>> _guidGenerators;

        QuestContainer            _questTemplates;

        typedef std::unordered_map<uint32, GossipText> GossipTextContainer;
        typedef std::unordered_map<uint32, uint32> QuestAreaTriggerMap;
        typedef std::unordered_map<uint32, BattlegroundTypeId> BattleMastersMap;
        typedef std::unordered_map<uint32, std::string> ItemTextMap;
        typedef std::set<uint32> TavernAreaTriggerSet;
        typedef std::set<uint32> GameObjectForQuestSet;

        ItemMap             mItems;
        ItemTextMap         mItemTexts;

        QuestAreaTriggerMap _questAreaTriggerStore;
        BattleMastersMap    _battleMastersStore;
        TavernAreaTriggerSet _tavernAreaTriggerStore;
        GameObjectForQuestSet _gameObjectForQuestStore;
        GossipTextContainer _gossipTextStore;
        QuestGreetingContainer _questGreetingStore;
        AreaTriggerMap      _areaTriggerStore;
        AreaTriggerScriptMap  _areaTriggerScriptStore;
        AccessRequirementContainer  _accessRequirementStore;

        RepOnKillMap        _repOnKillStore;

        WeatherZoneMap      mWeatherZoneMap;

        PetCreateSpellMap   mPetCreateSpell;

        //character reserved names
        typedef std::set<std::string> ReservedNamesMap;
        ReservedNamesMap    _reservedNamesStore;

        std::set<uint32>    m_DisabledPlayerSpells;
        std::set<uint32>    m_DisabledCreatureSpells;
        std::set<uint32>    m_DisabledPetSpells;

        GraveYardMap        mGraveYardMap;

        GameTeleContainer   m_GameTeleMap;

        ScriptNameContainer  _scriptNamesStore;
        
        LocaleConstant DBCLocaleIndex;

        PageTextContainer _pageTextStore;
        EquipmentInfoContainer _equipmentInfoStore;
        SpawnGroupDataContainer _spawnGroupDataStore;
        SpawnGroupLinkContainer _spawnGroupMapStore;
        InstanceSpawnGroupContainer _instanceSpawnGroupStore;
        /// Stores temp summon data grouped by summoner's entry, summoner's type and group id
        TempSummonDataContainer _tempSummonDataStore;
        InstanceTemplateContainer _instanceTemplateStore;
        CreatureModelContainer _creatureModelStore;
        InstanceTemplateAddonContainer _instanceTemplateAddonStore;
        CreatureAddonContainer _creatureAddonStore;
        CreatureAddonContainer _creatureTemplateAddonStore;
        std::unordered_map<ObjectGuid::LowType, CreatureMovementData> _creatureMovementOverrides;
        GameObjectTemplateContainer _gameObjectTemplateStore;
        CreatureTemplateContainer _creatureTemplateStore;
        ItemTemplateContainer _itemTemplateStore;
        BroadcastTextContainer _broadcastTextStore;
        SpellScriptsContainer _spellScriptsStore;

    private:
        void LoadScripts(ScriptMapMap& scripts, char const* tablename);
        void LoadQuestRelationsHelper(QuestRelations& map,char const* table);

        typedef std::unordered_map<uint32 /*creatureId*/, std::unique_ptr<PetLevelInfo[] /*level*/>> PetLevelInfoContainer;
        PetLevelInfoContainer _petInfoStore;                            // [creature_id][level]

        CreatureBaseStatsContainer _creatureBaseStatsStore;

        std::unique_ptr<PlayerClassInfo> _playerClassInfo[MAX_CLASSES];

        void BuildPlayerLevelInfo(uint8 race, uint8 class_, uint8 level, PlayerLevelInfo* plinfo) const;
        std::unique_ptr<PlayerInfo> _playerInfo[MAX_RACES][MAX_CLASSES];

        typedef std::map<uint32,uint32> BaseXPMap;          // [area level][base xp]
        BaseXPMap mBaseXPTable;

        typedef std::map<uint32,int32> FishingBaseSkillMap; // [areaId][base skill level]
        FishingBaseSkillMap mFishingBaseForArea;

        typedef std::map<uint32,std::vector<std::string> > HalfNameMap;
        HalfNameMap PetHalfName0;
        HalfNameMap PetHalfName1;

        MapObjectGuids _mapObjectGuidsStore;
        CreatureDataContainer _creatureDataStore;
        CreatureLocaleContainer _creatureLocaleStore;
        GameObjectDataContainer _gameObjectDataStore;
        GameObjectLocaleContainer _gameObjectLocaleStore;
        ItemLocaleContainer _itemLocaleStore;
        QuestLocaleContainer _questLocaleStore;
        NpcTextLocaleContainer mGossipTextLocaleMap;
        PageTextLocaleContainer mPageTextLocaleMap;
        TrinityStringLocaleContainer _trinityStringStore;
        GossipMenuItemsLocaleContainer _gossipMenuItemsLocaleStore;
        PointOfInterestLocaleContainer _pointOfInterestLocaleStore;
        QuestGreetingLocaleContainer _questGreetingLocaleStore;

        GossipMenusContainer _gossipMenusStore;
        GossipMenuItemsContainer _gossipMenuItemsStore;
        PointOfInterestContainer _pointsOfInterestStore;
        CacheGoTextIdMap m_mCacheGoMenuIdMap;
        CacheNpcTextIdMap m_mCacheNpcMenuIdMap;
        CacheVendorItemMap m_mCacheVendorItemMap;
        CacheTrainerSpellMap m_mCacheTrainerSpellMap;

        ItemExtendedStore sItemExtendedCostStore;

        SpellEntryStore spellTemplates;
        uint32 maxSpellId;

        std::set<uint32> _transportMaps; // Helper container storing map ids that are for transports only, loaded from gameobject_template
};

#define sObjectMgr ObjectMgr::instance()

// scripting access functions
bool LoadTrinityStrings(DatabaseWorkerPool<WorldDatabaseConnection>& db, char const* table,int32 start_value = -1, int32 end_value = std::numeric_limits<int32>::min());
uint32 GetAreaTriggerScriptId(uint32 trigger_id);

#endif

