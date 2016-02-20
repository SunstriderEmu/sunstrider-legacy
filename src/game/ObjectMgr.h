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

#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Log.h"
#include "Object.h"
#include "Bag.h"
#include "Creature.h"
#include "Player.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Corpse.h"
#include "QuestDef.h"
#include "ItemPrototype.h"
#include "NPCHandler.h"
#include "Database/DatabaseEnv.h"
#include "Mail.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "Policies/Singleton.h"
#include "ConditionMgr.h"

#include <string>
#include <map>
#include <limits>
#include <unordered_map>

class Group;
class Guild;
class ArenaTeam;
class TransportPath;
class Item;

struct PageText
{
    std::string Text;
    uint16 NextPage;
};

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

typedef std::unordered_map<uint32, GameTele > GameTeleContainer;

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;
    uint32 datalong;
    uint32 datalong2;
    int32  dataint;
    float x;
    float y;
    float z;
    float o;
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap > ScriptMapMap;
extern ScriptMapMap sQuestEndScripts;
extern ScriptMapMap sQuestStartScripts;
extern ScriptMapMap sSpellScripts;
extern ScriptMapMap sGameObjectScripts;
extern ScriptMapMap sEventScripts;
extern ScriptMapMap sWaypointScripts;

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
        if (gender == GENDER_FEMALE && (forceGender || !FemaleText[DEFAULT_LOCALE].empty()))
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

typedef std::unordered_map<uint64/*(instance,guid) pair*/,time_t> RespawnTimes;


// mangos string ranges
#define MIN_TRINITY_STRING_ID    1
#define MAX_TRINITY_STRING_ID    2000000000
#define MIN_DB_SCRIPT_STRING_ID MAX_TRINITY_STRING_ID
#define MAX_DB_SCRIPT_STRING_ID 2000010000

struct TrinityStringLocale
{
    std::vector<std::string> Content;                       // 0 -> default, i -> i-1 locale index
};

typedef std::map<uint32,uint32> CreatureLinkedRespawnMap;
typedef std::unordered_map<uint32,CreatureData> CreatureDataMap;
typedef std::unordered_map<uint32,GameObjectData> GameObjectDataMap;
typedef std::unordered_map<uint32,CreatureLocale> CreatureLocaleMap;
typedef std::unordered_map<uint32,GameObjectLocale> GameObjectLocaleMap;
typedef std::unordered_map<uint32,ItemLocale> ItemLocaleMap;
typedef std::unordered_map<uint32,QuestLocale> QuestLocaleMap;
typedef std::unordered_map<uint32,NpcTextLocale> NpcTextLocaleMap;
typedef std::unordered_map<uint32,PageTextLocale> PageTextLocaleMap;
typedef std::unordered_map<uint32,TrinityStringLocale> TrinityStringLocaleMap;
typedef std::unordered_map<uint32,GossipMenuItemsLocale> GossipMenuItemsLocaleContainer;
typedef std::unordered_map<uint32, PointOfInterestLocale> PointOfInterestLocaleContainer;

typedef std::multimap<uint32,uint32> QuestRelations;

// Benchmarked: Faster than std::unordered_map (insert/find)
typedef std::map<uint32, PageText> PageTextContainer;

// Benchmarked: Faster than std::map (insert/find)
typedef std::unordered_map<uint16, InstanceTemplate> InstanceTemplateContainer;
typedef std::unordered_map<uint16, InstanceTemplateAddon> InstanceTemplateAddonContainer;

struct PetLevelInfo
{
    PetLevelInfo() : health(0), mana(0) { for(int i=0; i < MAX_STATS; ++i ) stats[i] = 0; }

    uint16 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
    uint16 armor;
};

struct ReputationOnKillEntry
{
    uint32 repfaction1;
    uint32 repfaction2;
    bool is_teamaward1;
    uint32 reputation_max_cap1;
    int32 repvalue1;
    bool is_teamaward2;
    uint32 reputation_max_cap2;
    int32 repvalue2;
    bool team_dependent;
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

//TODO Old condition system, must be converted to conditions from ConditionMgr
enum OldConditionType
{   // Comment for database support #lootcondition   #condition_value1          #condition_value2   #not used           #comment
    CONDITION_OLD_NONE                      = 0,    // 0                        0                   0                   always true
    CONDITION_OLD_AURA                      = 1,    // spell_id                 effindex            +referenceID        true if has aura of spell_id with effect effindex
    CONDITION_OLD_ITEM                      = 2,    // item_id                  count               +referenceID        true if has #count of item_ids
    CONDITION_OLD_ITEM_EQUIPPED             = 3,    // item_id                  0                   +referenceID        true if has item_id equipped
    CONDITION_OLD_ZONEID                    = 4,    // zone_id                  0                   +referenceID        true if in zone_id
    CONDITION_OLD_REPUTATION_RANK           = 5,    // faction_id               min_rank            +referenceID        true if has min_rank for faction_id
    CONDITION_OLD_TEAM                      = 6,    // player_team              0,                  +referenceID        (playerteam = 469 - Alliance, 67 - Horde)
    CONDITION_OLD_SKILL                     = 7,    // skill_id                 skill_value         +referenceID        true if has skill_value for skill_id
    CONDITION_OLD_QUESTREWARDED             = 8,    // quest_id                 0                   +referenceID        true if quest_id was rewarded before
    CONDITION_OLD_QUESTTAKEN                = 9,    // quest_id                 0,                  +referenceID        true while quest active
    CONDITION_OLD_AD_COMMISSION_AURA        = 10,   // 0                        0,                  +referenceID        true while one from Argent Dawn commission aura active
    CONDITION_OLD_NO_AURA                   = 11,   // spell_id                 effindex            +referenceID        true if does not have aura of spell_id with effect effindex
    CONDITION_OLD_ACTIVE_EVENT              = 12,   // event_id                 0                   +referenceID        true if event is active
    CONDITION_OLD_INSTANCE_DATA             = 13,   // entry                    data                +referenceID        true if data is set in current instance
    CONDITION_OLD_QUEST_NONE                = 14,   // quest_id                 0                   +referenceID        true if doesn't have quest saved
    CONDITION_OLD_CLASS                     = 15,   // class                    0                   +referenceID        true if player's class is equal to class
    CONDITION_OLD_RACE                      = 16,   // race                     0                   +referenceID        true if player's race is equal to race
    //CONDITION_OLD_ACHIEVEMENT             = 17,   // achievement_id           0                   +referenceID        true if achievement is complete
    CONDITION_OLD_SPELL_SCRIPT_TARGET       = 18,   // SpellScriptTargetType,   TargetEntry,        0
    CONDITION_OLD_CREATURE_TARGET           = 19,   // creature_entry           0                   +referenceID        true if current target is creature with value1 entry
    CONDITION_OLD_TARGET_HEALTH_BELOW_PCT   = 20,   // 0-100                    0                   +referenceID        true if target's health is below value1 percent, false if over or no target
    CONDITION_OLD_TARGET_RANGE              = 21,   // minDistance              maxDist             +referenceID        true if target is closer then minDist and further then maxDist or if max is 0 then max dist is infinit
    CONDITION_OLD_MAPID                     = 22,   // map_id                   0                   +referenceID        true if in map_id
    CONDITION_OLD_AREAID                    = 23,   // area_id                  0                   +referenceID        true if in area_id
    CONDITION_OLD_ITEM_TARGET               = 24,   // ItemRequiredTargetType,  TargetEntry,        0
    CONDITION_OLD_QUEST_COMPLETE            = 28,   // quest_id                 0                   +referenceID        true if player has quest_id with all objectives complete, but not yet rewarded
    CONDITION_OLD_NEAR_CREATURE             = 29,   // creature_entry           distance            +referenceID        true if there is a creature of entry in range
    CONDITION_OLD_NEAR_GAMEOBJECT           = 30,   // gameobject_entry         distance            +referenceID        true if there is a gameobject of entry in range
};

#define MAX_CONDITION 31 // maximum value in OldConditionType enum

//TODO Old condition system, must be converted to conditions from ConditionMgr
struct PlayerCondition
{
    OldConditionType condition;                                // additional condition type
    uint32  value1;                                         // data for the condition - see OldConditionType definition
    uint32  value2;

    PlayerCondition(uint8 _condition = 0, uint32 _value1 = 0, uint32 _value2 = 0)
        : condition(OldConditionType(_condition)), value1(_value1), value2(_value2) {}

    static bool IsValid(OldConditionType condition, uint32 value1, uint32 value2);
    // Checks correctness of values
    bool Meets(Player const * APlayer) const;               // Checks if the player meets the condition
    bool operator == (PlayerCondition const& lc) const
    {
        return (lc.condition == condition && lc.value1 == value1 && lc.value2 == value2);
    }
};

// NPC gossip text id
typedef std::unordered_map<uint32, uint32> CacheNpcTextIdMap;
typedef std::unordered_map<uint32, uint32> CacheGoTextIdMap;

typedef std::unordered_map<uint32, VendorItemData> CacheVendorItemMap;
typedef std::unordered_map<uint32, TrainerSpellData> CacheTrainerSpellMap;

typedef std::unordered_map<uint32, ItemExtendedCostEntry> ItemExtendedStore;

enum SkillRangeType
{
    SKILL_RANGE_LANGUAGE,                                   // 300..300
    SKILL_RANGE_LEVEL,                                      // 1..max skill for level
    SKILL_RANGE_MONO,                                       // 1..1, grey monolite bar
    SKILL_RANGE_RANK,                                       // 1..skill for known rank
    SKILL_RANGE_NONE,                                       // 0..0 always
};

struct GM_Ticket
{
  uint64 guid;
  uint64 playerGuid;
  float pos_x;
  float pos_y;
  float pos_z;
  uint32 map;
  std::string message;
  uint64 createtime;
  uint64 timestamp;
  int64 closed; // 0 = Open, -1 = Console, playerGuid = player abandoned ticket, other = GM who closed it.
  uint64 assignedToGM;
  std::string comment;
};
typedef std::list<GM_Ticket*> GmTicketList;

SkillRangeType GetSkillRangeType(SkillLineEntry const *pSkill, bool racial);

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
    ConditionList   Conditions;
};

struct GossipMenus
{
    uint32          entry;
    uint32          text_id;
    ConditionList   conditions;
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

bool normalizePlayerName(std::string& name);

struct LanguageDesc
{
    Language lang_id;
    uint32   spell_id;
    uint32   skill_id;
};

extern LanguageDesc lang_description[LANGUAGES_COUNT];
LanguageDesc const* GetLanguageDescByID(uint32 lang);

class PlayerDumpReader;

class ObjectMgr
{
    friend class PlayerDumpReader;

    public:
        ObjectMgr();
        ~ObjectMgr();

        static ObjectMgr* instance()
        {
            static ObjectMgr instance;
            return &instance;
        }

        typedef std::unordered_map<uint32, Item*> ItemMap;

        typedef std::set< Group * > GroupSet;

        typedef std::unordered_map<uint32, Guild *> GuildMap;

        typedef std::unordered_map<uint32, ArenaTeam*> ArenaTeamMap;

        typedef std::unordered_map<uint32, Quest*> QuestMap;

        typedef std::unordered_map<uint32, AreaTrigger> AreaTriggerMap;

        typedef std::unordered_map<uint32, uint32> AreaTriggerScriptMap;

        typedef std::unordered_map<uint32, AccessRequirement> AccessRequirementMap;

        typedef std::unordered_map<uint32, ReputationOnKillEntry> RepOnKillMap;

        typedef std::unordered_map<uint32, WeatherZoneChances> WeatherZoneMap;

        typedef std::unordered_map<uint32, PointOfInterest> PointOfInterestContainer;

        typedef std::unordered_map<uint32, PetCreateSpellEntry> PetCreateSpellMap;

        typedef std::vector<std::string> ScriptNameMap;

        std::unordered_map<uint32, uint32> TransportEventMap;

        //Get player in world by high guid
        Player* GetPlayer(uint64 guid) const { return ObjectAccessor::FindPlayer(guid); }
        //Get player in world by low guid
        Player* GetPlayer(uint32 lowguid) const { return ObjectAccessor::FindPlayer(MAKE_NEW_GUID(lowguid,0,HIGHGUID_PLAYER)); }

        GameObjectTemplate const* GetGameObjectTemplate(uint32 id);
        GameObjectTemplateContainer const* GetGameObjectTemplateStore() const { return &_gameObjectTemplateStore; }

        void LoadGameObjectTemplate();
        void AddGameObjectTemplate(GameObjectTemplate *goinfo);

        Group * GetGroupByLeader(const uint64 &guid) const;
        void AddGroup(Group* group) { mGroupSet.insert( group ); }
        void RemoveGroup(Group* group) { mGroupSet.erase( group ); }
        GroupSet::iterator GetGroupSetBegin() { return mGroupSet.begin(); }
        GroupSet::iterator GetGroupSetEnd()   { return mGroupSet.end(); }


        bool IsGuildLeader(uint64 const&guid) const;

        Guild* GetGuildByLeader(uint64 const&guid) const;
        Guild* GetGuildById(const uint32 GuildId);
        Guild* GetGuildByName(const std::string& guildname);
        std::string GetGuildNameById(const uint32 GuildId);

        void AddGuild(Guild* guild);
        void RemoveGuild(uint32 Id);

        ArenaTeam* _GetArenaTeamById(const uint32 arenateamid) const;
        ArenaTeam* _GetArenaTeamByName(const std::string& arenateamname) const;

        ArenaTeam* GetArenaTeamById(const uint32 arenateamid);
        ArenaTeam* GetArenaTeamByName(const std::string& arenateamname);

        ArenaTeam* _GetArenaTeamByCaptain(uint64 const& guid) const;
        bool IsArenaTeamCaptain(uint64 const& guid) const;

        //Add a team to the team list. Given team object will be destroyed upon ObjectMgr destruction.
        void AddArenaTeam(ArenaTeam* arenaTeam);
        void RemoveArenaTeam(uint32 Id);
        ArenaTeamMap::iterator GetArenaTeamMapBegin() { return mArenaTeamMap.begin(); }
        ArenaTeamMap::iterator GetArenaTeamMapEnd()   { return mArenaTeamMap.end(); }

        CreatureTemplateContainer const* GetCreatureTemplates() const { return &_creatureTemplateStore; }
        CreatureTemplate const* GetCreatureTemplate( uint32 id );
        CreatureModelInfo const* GetCreatureModelInfo( uint32 modelid );
        //return a new displayId with same gender and race as baseModel, if possible. Else return model info for displayID. Race NYI
        CreatureModelInfo const* GetCreatureModelSameGenderAndRaceAs(uint32& displayID, uint32 baseDisplayId);
        CreatureModelInfo const* GetCreatureModelRandomGender(uint32& displayID);
        static uint32 ChooseDisplayId(const CreatureTemplate *cinfo, const CreatureData *data = NULL);
        EquipmentInfo const* GetEquipmentInfo( uint32 entry );
        CreatureAddon const *GetCreatureAddon( uint32 lowguid );
        CreatureAddon const *GetCreatureTemplateAddon( uint32 entry );

        ItemTemplate const* GetItemTemplate(uint32 id);
        ItemTemplateContainer const* GetItemTemplateStore() const { return &_itemTemplateStore; }
        ItemExtendedCostEntry const* GetItemExtendedCost(uint32 id) const;

        InstanceTemplate const* GetInstanceTemplate(uint32 map);
        InstanceTemplateContainer const* GetInstanceTemplateStore() const { return &_instanceTemplateStore; }

        InstanceTemplateAddon const* GetInstanceTemplateAddon(uint32 map);

        PetLevelInfo const* GetPetLevelInfo(uint32 creature_id, uint32 level) const;

        PlayerClassInfo const* GetPlayerClassInfo(uint32 class_) const
        {
            if(class_ >= MAX_CLASSES) return NULL;
            return &playerClassInfo[class_];
        }
        void GetPlayerClassLevelInfo(uint32 class_,uint32 level, PlayerClassLevelInfo* info) const;

        PlayerInfo const* GetPlayerInfo(uint32 race, uint32 class_) const
        {
            if(race   >= MAX_RACES)   return NULL;
            if(class_ >= MAX_CLASSES) return NULL;
            PlayerInfo const* info = &playerInfo[race][class_];
            if(info->displayId_m==0 || info->displayId_f==0) return NULL;
            return info;
        }
        void GetPlayerLevelInfo(uint32 race, uint32 class_,uint32 level, PlayerLevelInfo* info) const;

        uint64 GetPlayerGUIDByName(std::string name) const;
        uint32 GetPlayerLowGUIDByName(std::string name) const;
        bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;
        bool GetPlayerNameByLowGUID(uint32 guid, std::string &name) const;
        uint32 GetPlayerTeamByGUID(const uint64 &guid) const;
        uint32 GetPlayerAccountIdByGUID(const uint64 &guid) const;
        uint32 GetPlayerAccountIdByPlayerName(const std::string& name) const;

        uint32 GetNearestTaxiNode( float x, float y, float z, uint32 mapid, uint32 team);
        void GetTaxiPath( uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint32 GetTaxiMountDisplayId(uint32 id, uint32 team, bool allowed_alt_team = false);

        Quest const* GetQuestTemplate(uint32 quest_id) const
        {
            QuestMap::const_iterator itr = mQuestTemplates.find(quest_id);
            return itr != mQuestTemplates.end() ? itr->second : NULL;
        }
        QuestMap const& GetQuestTemplates() const { return mQuestTemplates; }

        uint32 GetQuestForAreaTrigger(uint32 Trigger_ID) const
        {
            QuestAreaTriggerMap::const_iterator itr = mQuestAreaTriggerMap.find(Trigger_ID);
            if(itr != mQuestAreaTriggerMap.end())
                return itr->second;
            return 0;
        }
        bool IsTavernAreaTrigger(uint32 Trigger_ID) const { return mTavernAreaTriggerSet.count(Trigger_ID) != 0; }
        bool IsGameObjectForQuests(uint32 entry) const { return _gameObjectForQuestStore.count(entry) != 0; }
        bool IsGuildVaultGameObject(Player *player, uint64 guid) const
        {
            if(GameObject *go = ObjectAccessor::GetGameObject(*player, guid))
                if(go->GetGoType() == GAMEOBJECT_TYPE_GUILD_BANK)
                    return true;
            return false;
        }

        BattlegroundTypeId GetBattleMasterBG(uint32 entry) const;

        void AddGossipText(GossipText *pGText);
        GossipText *GetGossipText(uint32 Text_ID);

        WorldSafeLocsEntry const *GetClosestGraveYard(float x, float y, float z, uint32 MapId, uint32 team);
        bool AddGraveYardLink(uint32 id, uint32 zone, uint32 team, bool inDB = true);
        void RemoveGraveYardLink(uint32 id, uint32 zone, uint32 team, bool inDB = false);
        void LoadGraveyardZones();
        GraveYardData const* FindGraveYardData(uint32 id, uint32 zone);

        AreaTrigger const* GetAreaTrigger(uint32 trigger) const
        {
            AreaTriggerMap::const_iterator itr = mAreaTriggers.find( trigger );
            if( itr != mAreaTriggers.end( ) )
                return &itr->second;
            return NULL;
        }

        AccessRequirement const* GetAccessRequirement(uint32 requirement) const
        {
            AccessRequirementMap::const_iterator itr = mAccessRequirements.find( requirement );
            if( itr != mAccessRequirements.end( ) )
                return &itr->second;
            return NULL;
        }

        AreaTrigger const* GetGoBackTrigger(uint32 Map) const;
        AreaTrigger const* GetMapEntranceTrigger(uint32 Map) const;

        uint32 GetAreaTriggerScriptId(uint32 trigger_id);

        ReputationOnKillEntry const* GetReputationOnKilEntry(uint32 id) const
        {
            RepOnKillMap::const_iterator itr = mRepOnKill.find(id);
            if(itr != mRepOnKill.end())
                return &itr->second;
            return NULL;
        }

        PointOfInterest const* GetPointOfInterest(uint32 id) const
        {
            PointOfInterestContainer::const_iterator itr = _pointsOfInterestStore.find(id);
            if (itr != _pointsOfInterestStore.end())
                return &itr->second;
            return NULL;
        }

        PetCreateSpellEntry const* GetPetCreateSpellEntry(uint32 id) const
        {
            PetCreateSpellMap::const_iterator itr = mPetCreateSpell.find(id);
            if(itr != mPetCreateSpell.end())
                return &itr->second;
            return NULL;
        }

        void LoadGuilds();
        void LoadArenaTeams();
        void LoadGroups();
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

        bool LoadTrinityStrings(WorldDatabaseWorkerPool& db, char const* table, int32 min_value, int32 max_value);
        bool LoadTrinityStrings() { return LoadTrinityStrings(WorldDatabase,"trinity_string",MIN_TRINITY_STRING_ID,MAX_TRINITY_STRING_ID); }
        void LoadDbScriptStrings();
        void LoadPetCreateSpells();
        void LoadBroadcastTexts();
        void LoadBroadcastTextLocales();
        void LoadCreatureLocales();
        void LoadCreatureClassLevelStats();
        void LoadCreatureTemplates();
        void LoadCreatureTemplate(Field* fields);
        void CheckCreatureTemplate(CreatureTemplate const* cInfo);
        void LoadCreatures();
        void LoadCreatureLinkedRespawn();
        bool CheckCreatureLinkedRespawn(uint32 guid, uint32 linkedGuid) const;
        bool SetCreatureLinkedRespawn(uint32 guid, uint32 linkedGuid);
        void LoadCreatureRespawnTimes();
        void LoadCreatureAddons();
        void LoadCreatureTemplateAddons();
        void LoadCreatureModelInfo();
        void LoadEquipmentTemplates();
        void LoadGameObjectLocales();
        void LoadGameobjects();
        void LoadGameobjectRespawnTimes();
        void LoadItemTemplates();
        void LoadItemLocales();
        void LoadItemExtendedCost();
        void LoadQuestLocales();
        void LoadGossipTextLocales();
        void LoadPageTextLocales();
        void LoadGossipMenuItemsLocales();
        void LoadInstanceTemplate();

        void LoadGossipText();

        void LoadAreaTriggerTeleports();
        void LoadAccessRequirements();
        void LoadQuestAreaTriggers();
        void LoadAreaTriggerScripts();
        void LoadTavernAreaTriggers();
        void LoadBattleMastersEntry();
        void LoadGameObjectForQuests();

        void LoadItemTexts();
        void LoadPageTexts();
        PageText const* GetPageText(uint32 pageEntry);

        void LoadPlayerInfo();
        void LoadPetLevelInfo();
        void LoadExplorationBaseXP();
        void LoadPetNames();
        void LoadPetNumber();
        void LoadCorpses();
        void LoadFishingBaseSkillLevel();

        void LoadReputationOnKill();

        void LoadPointsOfInterest();
        void LoadQuestPOI();

        void LoadWeatherZoneChances();
        void LoadGameTele();

        void LoadGossipMenu();
        // Generic items are loaded in the same container as creature specific items, under menu id 0
        static const int GENERIC_OPTIONS_MENU = 0;
        //load both gossip_menu_option and gossip_menu_option_generic
        void LoadGossipMenuItems();
        void LoadCreatureGossip();
        void LoadVendors();
        void LoadTrainerSpell();

        void LoadGMTickets();

        void LoadSpellTemplates();
        SpellEntry const* GetSpellTemplate(uint32 id) const;
        SpellEntryStore& GetSpellStore() { return spellTemplates; }
        uint32 GetMaxSpellId() { return maxSpellId; }
        
        std::string GeneratePetName(uint32 entry);
        uint32 GetBaseXP(uint32 level);

        int32 GetFishingBaseSkillLevel(uint32 entry) const
        {
            FishingBaseSkillMap::const_iterator itr = mFishingBaseForArea.find(entry);
            return itr != mFishingBaseForArea.end() ? itr->second : 0;
        }

        void ReturnOrDeleteOldMails(bool serverUp);

        CreatureBaseStats const* GetCreatureBaseStats(uint8 level, uint8 unitClass);

        void SetHighestGuids();
        //setting temporary to true will use an alternate (higher) set of guid. This is done to prevent overflows.
        uint32 GenerateLowGuid(HighGuid guidhigh, bool temporary = false);
        uint32 AltGenerateLowGuid(uint32 type, bool& temporary);
        uint32 GenerateAuctionID();
        uint32 GenerateMailID();
        uint32 GenerateItemTextID();
        uint32 GeneratePetNumber();
        uint32 GenerateArenaTeamId();
        uint32 GenerateGuildId();

        bool IsInTemporaryGuidRange(uint32 type, uint32 guid);

        uint32 CreateItemText(SQLTransaction& charTrans, std::string const& text);
        uint32 CreateItemText(std::string const& text);
        std::string GetItemText( uint32 id )
        {
            ItemTextMap::const_iterator itr = mItemTexts.find( id );
            if ( itr != mItemTexts.end() )
                return itr->second;
            else
                return "There is no info for this item";
        }

        typedef std::multimap<int32, uint32> ExclusiveQuestGroups;
        ExclusiveQuestGroups mExclusiveQuestGroups;

        WeatherZoneChances const* GetWeatherChances(uint32 zone_id) const
        {
            WeatherZoneMap::const_iterator itr = mWeatherZoneMap.find(zone_id);
            if(itr != mWeatherZoneMap.end())
                return &itr->second;
            else
                return NULL;
        }

        CellObjectGuids const& GetCellObjectGuids(uint16 mapid, uint8 spawnMode, uint32 cell_id)
        {
            return mMapObjectGuids[MAKE_PAIR32(mapid,spawnMode)][cell_id];
        }

        CellObjectGuidsMap const& GetMapObjectGuids(uint16 mapid, uint8 spawnMode)
        {
            return mMapObjectGuids[MAKE_PAIR32(mapid, spawnMode)];
        }

        BroadcastText const* GetBroadcastText(uint32 id) const
        {
            BroadcastTextContainer::const_iterator itr = _broadcastTextStore.find(id);
            if (itr != _broadcastTextStore.end())
                return &itr->second;
            return NULL;
        }

        CreatureData const* GetCreatureData(uint32 guid) const
        {
            CreatureDataMap::const_iterator itr = mCreatureDataMap.find(guid);
            if(itr==mCreatureDataMap.end()) return NULL;
            return &itr->second;
        }
        CreatureData& NewOrExistCreatureData(uint32 guid) { return mCreatureDataMap[guid]; }
        void DeleteCreatureData(uint32 guid);
        uint32 GetLinkedRespawnGuid(uint32 guid) const
        {
            CreatureLinkedRespawnMap::const_iterator itr = mCreatureLinkedRespawnMap.find(guid);
            if(itr == mCreatureLinkedRespawnMap.end()) return 0;
            return itr->second;
        }
        CreatureLocale const* GetCreatureLocale(uint32 entry) const
        {
            CreatureLocaleMap::const_iterator itr = _creatureLocaleStore.find(entry);
            if(itr==_creatureLocaleStore.end()) return NULL;
            return &itr->second;
        }
        GameObjectLocale const* GetGameObjectLocale(uint32 entry) const
        {
            GameObjectLocaleMap::const_iterator itr = mGameObjectLocaleMap.find(entry);
            if(itr==mGameObjectLocaleMap.end()) return NULL;
            return &itr->second;
        }
        ItemLocale const* GetItemLocale(uint32 entry) const
        {
            ItemLocaleMap::const_iterator itr = mItemLocaleMap.find(entry);
            if(itr==mItemLocaleMap.end()) return NULL;
            return &itr->second;
        }
        QuestLocale const* GetQuestLocale(uint32 entry) const
        {
            QuestLocaleMap::const_iterator itr = mQuestLocaleMap.find(entry);
            if(itr==mQuestLocaleMap.end()) return NULL;
            return &itr->second;
        }
        NpcTextLocale const* GetNpcTextLocale(uint32 entry) const
        {
            NpcTextLocaleMap::const_iterator itr = mGossipTextLocaleMap.find(entry);
            if(itr==mGossipTextLocaleMap.end()) return NULL;
            return &itr->second;
        }
        PageTextLocale const* GetPageTextLocale(uint32 entry) const
        {
            PageTextLocaleMap::const_iterator itr = mPageTextLocaleMap.find(entry);
            if(itr==mPageTextLocaleMap.end()) return NULL;
            return &itr->second;
        }
        GossipMenuItemsLocale const* GetGossipMenuItemsLocale(uint16 menuId, uint16 optionIndex) const
        {
            GossipMenuItemsLocaleContainer::const_iterator itr = _gossipMenuItemsLocaleStore.find(MAKE_PAIR32(menuId, optionIndex));
            if(itr==_gossipMenuItemsLocaleStore.end()) 
                return nullptr;

            return &itr->second;
        }
        PointOfInterestLocale const* GetPointOfInterestLocale(uint32 poi_id) const
        {
            PointOfInterestLocaleContainer::const_iterator itr = _pointOfInterestLocaleStore.find(poi_id);
            if (itr == _pointOfInterestLocaleStore.end()) return NULL;
            return &itr->second;
        }

        GameObjectData const* GetGOData(uint32 guid) const
        {
            GameObjectDataMap::const_iterator itr = mGameObjectDataMap.find(guid);
            if(itr==mGameObjectDataMap.end()) return NULL;
            return &itr->second;
        }
        GameObjectData& NewGOData(uint32 guid) { return mGameObjectDataMap[guid]; }
        void DeleteGOData(uint32 guid);

        TrinityStringLocale const* GetTrinityStringLocale(int32 entry) const
        {
            TrinityStringLocaleMap::const_iterator itr = mTrinityStringLocaleMap.find(entry);
            if( itr == mTrinityStringLocaleMap.end()) return NULL;
            return &itr->second;
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

        void AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance);
        void DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid);

        time_t GetCreatureRespawnTime(uint32 loguid, uint32 instance) { return mCreatureRespawnTimes[MAKE_PAIR64(loguid,instance)]; }
        void SaveCreatureRespawnTime(uint32 loguid, uint32 mapId, uint32 instance, time_t t);
        time_t GetGORespawnTime(uint32 loguid, uint32 instance) { return mGORespawnTimes[MAKE_PAIR64(loguid,instance)]; }
        void SaveGORespawnTime(uint32 loguid, uint32 mapId, uint32 instance, time_t t);
        void DeleteRespawnTimeForInstance(uint32 instance);

        // grid objects
        void AddCreatureToGrid(uint32 guid, CreatureData const* data);
        void RemoveCreatureFromGrid(uint32 guid, CreatureData const* data);
        void AddGameobjectToGrid(uint32 guid, GameObjectData const* data);
        void RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data);
        //NYI uint32 AddGOData(uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0, float rotation0 = 0, float rotation1 = 0, float rotation2 = 0, float rotation3 = 0);
        uint32 AddCreData(uint32 entry, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0);
        //NYI bool MoveCreData(uint32 guid, uint32 map, const Position& pos);

        // reserved names
        void LoadReservedPlayersNames();
        bool IsReservedName(const std::string& name) const
        {
            return m_ReservedNames.find(name) != m_ReservedNames.end();
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

        // guild bank tabs
        uint32 GetGuildBankTabPrice(uint8 Index) const { return Index < GUILD_BANK_MAX_TABS ? mGuildBankTabPrice[Index] : 0; }

        uint16 GetConditionId(OldConditionType condition, uint32 value1, uint32 value2);
        bool IsPlayerMeetToCondition(Player const* player, uint16 condition_id) const
        {
            if(condition_id >= mConditions.size())
                return false;

            return mConditions[condition_id].Meets(player);
        }

        GameTele const* GetGameTele(uint32 id) const
        {
            GameTeleContainer::const_iterator itr = m_GameTeleMap.find(id);
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

        uint32 GetNpcGossipMenu(uint32 guid) const
        {
            CacheNpcTextIdMap::const_iterator iter = m_mCacheNpcMenuIdMap.find(guid);
            if(iter == m_mCacheNpcMenuIdMap.end())
                return 0;

            return iter->second;
        }

        uint32 GetGameobjectGossipMenu(uint32 guid) const
        {
            auto iter = m_mCacheGoMenuIdMap.find(guid);
            if(iter == m_mCacheGoMenuIdMap.end())
                return 0;

            return iter->second;
        }

        TrainerSpellData const* GetNpcTrainerSpells(uint32 entry) const
        {
            CacheTrainerSpellMap::const_iterator  iter = m_mCacheTrainerSpellMap.find(entry);
            if(iter == m_mCacheTrainerSpellMap.end())
                return NULL;

            return &iter->second;
        }

        VendorItemData const* GetNpcVendorItemList(uint32 entry) const
        {
            CacheVendorItemMap::const_iterator  iter = m_mCacheVendorItemMap.find(entry);
            if(iter == m_mCacheVendorItemMap.end())
                return NULL;

            return &iter->second;
        }
        void AddVendorItem(uint32 entry,ItemTemplate const *proto, uint32 maxcount, uint32 incrtime, uint32 ExtendedCost, bool savetodb = true); // for event
        bool RemoveVendorItem(uint32 entry,ItemTemplate const *proto, bool savetodb = true); // for event
        bool IsVendorItemValid( uint32 vendor_entry, ItemTemplate const *proto, uint32 maxcount, uint32 ptime, uint32 ExtendedCost, Player* pl = NULL, std::set<uint32>* skip_vendors = NULL, uint32 ORnpcflag = 0 ) const;
        bool AddTrainerSpell(uint32 creatureId, TrainerSpell const& spell);
        bool RemoveTrainerSpell(uint32 creatureId, uint32 spellId);

        void LoadScriptNames();
        ScriptNameMap &GetScriptNames() { return m_scriptNames; }
        const char * GetScriptName(uint32 id) { return id < m_scriptNames.size() ? m_scriptNames[id].c_str() : ""; }
        uint32 GetScriptId(const char *name);


        
        GM_Ticket *GetGMTicket(uint64 ticketGuid)
        {
          for(GmTicketList::const_iterator i = m_GMTicketList.begin(); i != m_GMTicketList.end(); ++i)
            if((*i) && (*i)->guid == ticketGuid) 
              return (*i);

          return NULL;
        }
        GM_Ticket *GetGMTicketByPlayer(uint64 playerGuid)
        {
          for(GmTicketList::const_iterator i = m_GMTicketList.begin(); i != m_GMTicketList.end(); ++i)
            if((*i) && (*i)->playerGuid == playerGuid && (*i)->closed == 0) 
              return (*i);

          return NULL;        
        }

        void AddOrUpdateGMTicket(GM_Ticket &ticket, bool create = false);
        void _AddOrUpdateGMTicket(GM_Ticket &ticket);
        void RemoveGMTicket(uint64 ticketGuid, int64 source = -1, bool permanently = false);
        void RemoveGMTicket(GM_Ticket *ticket, int64 source = -1, bool permanently = false);
        GmTicketList m_GMTicketList;
        uint64 GenerateGMTicketId();

        uint32 GetMaxCreatureGUID() { return m_hiCreatureGuid; }
        
        uint32 GetLoadedScriptsStats(uint32& entryLoaded, uint32& guidLoaded)
        {
            entryLoaded = m_creatureScriptsByEntry.size();
            guidLoaded = m_creatureScriptsByGUID.size();
            return 0;
        }
        
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

        bool isUsingAlternateGuidGeneration() { return m_hiCreatureRegularModeGuid; };
        uint32 getCurrentCreatureGuidIndex() { return m_hiCreatureGuid; };
        uint32 getAltCurrentCreatureGuidIndex() { return m_hiTempCreatureGuid; };
        uint32 getAltCreatureGuidStartIndex() { return m_hiTempCreatureGuidStart; };
        uint32 getCurrentGoGuidIndex() { return m_hiTempGoGuid; };
        uint32 getAltCurrentGoGuidIndex() { return m_hiTempGoGuidStart; };
        uint32 getAltGoGuidStartIndex() { return m_hiTempGoGuidStart; };
        
        bool IsTransportMap(uint32 mapId) const { return _transportMaps.count(mapId); }

    protected:

        // first free id for selected id type
        uint32 m_auctionid;
        uint32 m_mailid;
        uint32 m_ItemTextId;
        uint32 m_arenaTeamId;
        uint32 m_guildId;
        uint32 m_hiPetNumber;
        uint64 m_GMticketid;

        // first free low guid for seelcted guid type
        uint32 m_hiCharGuid;

        uint32 m_hiCreatureGuid;
        uint32 m_hiTempCreatureGuidStart;
        uint32 m_hiTempCreatureGuid;
        bool m_hiCreatureRegularModeGuid; //Debug or if m_hiCreatureGuid has reached m_hiTempCreatureGuidStart

        uint32 m_hiGoGuid;
        uint32 m_hiTempGoGuidStart;
        uint32 m_hiTempGoGuid;
        bool m_hiGoRegularModeGuid;

        uint32 m_hiPetGuid;
        uint32 m_hiItemGuid;
        uint32 m_hiDoGuid;
        uint32 m_hiCorpseGuid;
        uint32 m_hiTransportGuid;

        QuestMap            mQuestTemplates;

        typedef std::unordered_map<uint32, GossipText*> GossipTextMap;
        typedef std::unordered_map<uint32, uint32> QuestAreaTriggerMap;
        typedef std::unordered_map<uint32, BattlegroundTypeId> BattleMastersMap;
        typedef std::unordered_map<uint32, std::string> ItemTextMap;
        typedef std::set<uint32> TavernAreaTriggerSet;
        typedef std::set<uint32> GameObjectForQuestSet;

        GroupSet            mGroupSet; //groups are identified by leader guid
        GuildMap            mGuildMap;
        ArenaTeamMap        mArenaTeamMap;

        ItemMap             mItems;

        ItemTextMap         mItemTexts;

        QuestAreaTriggerMap mQuestAreaTriggerMap;
        BattleMastersMap    mBattleMastersMap;
        TavernAreaTriggerSet mTavernAreaTriggerSet;
        GameObjectForQuestSet _gameObjectForQuestStore;
        GossipTextMap       mGossipText;
        AreaTriggerMap      mAreaTriggers;
        AreaTriggerScriptMap  mAreaTriggerScripts;
        AccessRequirementMap  mAccessRequirements;

        RepOnKillMap        mRepOnKill;

        WeatherZoneMap      mWeatherZoneMap;

        PetCreateSpellMap   mPetCreateSpell;

        //character reserved names
        typedef std::set<std::string> ReservedNamesMap;
        ReservedNamesMap    m_ReservedNames;

        std::set<uint32>    m_DisabledPlayerSpells;
        std::set<uint32>    m_DisabledCreatureSpells;
        std::set<uint32>    m_DisabledPetSpells;

        GraveYardMap        mGraveYardMap;

        GameTeleContainer   m_GameTeleMap;

        ScriptNameMap       m_scriptNames;
        
        typedef std::map<uint32, std::string> EntryScriptsMap;
        EntryScriptsMap m_creatureScriptsByEntry;
        typedef std::map<uint64, std::string> GUIDScriptsMap;
        GUIDScriptsMap m_creatureScriptsByGUID;

        LocaleConstant DBCLocaleIndex;

        PageTextContainer _pageTextStore;
        EquipmentInfoContainer _equipmentInfoStore;
        InstanceTemplateContainer _instanceTemplateStore;
        CreatureModelContainer _creatureModelStore;
        InstanceTemplateAddonContainer _instanceTemplateAddonStore;
        CreatureAddonContainer _creatureAddonStore;
        CreatureAddonContainer _creatureTemplateAddonStore;
        GameObjectTemplateContainer _gameObjectTemplateStore;
        CreatureTemplateContainer _creatureTemplateStore;
        ItemTemplateContainer _itemTemplateStore;
        BroadcastTextContainer _broadcastTextStore;

    private:
        void LoadScripts(ScriptMapMap& scripts, char const* tablename);
        void CheckScripts(ScriptMapMap const& scripts,std::set<int32>& ids);
        void ConvertCreatureAddonAuras(CreatureAddon* addon, char const* table, char const* guidEntryStr);
        void LoadQuestRelationsHelper(QuestRelations& map,char const* table);

        typedef std::map<uint32,PetLevelInfo*> PetLevelInfoMap;
        // PetLevelInfoMap[creature_id][level]
        PetLevelInfoMap petInfo;                            // [creature_id][level]

        CreatureBaseStatsContainer _creatureBaseStatsStore;

        PlayerClassInfo playerClassInfo[MAX_CLASSES];

        void BuildPlayerLevelInfo(uint8 race, uint8 class_, uint8 level, PlayerLevelInfo* plinfo) const;
        PlayerInfo playerInfo[MAX_RACES][MAX_CLASSES];

        typedef std::map<uint32,uint32> BaseXPMap;          // [area level][base xp]
        BaseXPMap mBaseXPTable;

        typedef std::map<uint32,int32> FishingBaseSkillMap; // [areaId][base skill level]
        FishingBaseSkillMap mFishingBaseForArea;

        typedef std::map<uint32,std::vector<std::string> > HalfNameMap;
        HalfNameMap PetHalfName0;
        HalfNameMap PetHalfName1;

        MapObjectGuids mMapObjectGuids;
        CreatureDataMap mCreatureDataMap;
        CreatureLinkedRespawnMap mCreatureLinkedRespawnMap;
        CreatureLocaleMap _creatureLocaleStore;
        GameObjectDataMap mGameObjectDataMap;
        GameObjectLocaleMap mGameObjectLocaleMap;
        ItemLocaleMap mItemLocaleMap;
        QuestLocaleMap mQuestLocaleMap;
        NpcTextLocaleMap mGossipTextLocaleMap;
        PageTextLocaleMap mPageTextLocaleMap;
        TrinityStringLocaleMap mTrinityStringLocaleMap;
        GossipMenuItemsLocaleContainer _gossipMenuItemsLocaleStore;
        PointOfInterestLocaleContainer _pointOfInterestLocaleStore;
        std::mutex _creatureRespawnTimeLock;
        RespawnTimes mCreatureRespawnTimes;
        std::mutex _goRespawnTimeLock;
        RespawnTimes mGORespawnTimes;

        typedef std::vector<uint32> GuildBankTabPriceMap;
        GuildBankTabPriceMap mGuildBankTabPrice;

        // Storage for Conditions. First element (index 0) is reserved for zero-condition (nothing required)
        typedef std::vector<PlayerCondition> ConditionStore;
        ConditionStore mConditions;
        
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
bool LoadTrinityStrings(WorldDatabaseWorkerPool& db, char const* table,int32 start_value = -1, int32 end_value = std::numeric_limits<int32>::min());
uint32 GetAreaTriggerScriptId(uint32 trigger_id);
uint32 GetScriptId(const char *name);
ObjectMgr::ScriptNameMap& GetScriptNames();
CreatureTemplate const* GetCreatureTemplates(uint32 entry);
Quest const* GetQuestTemplateStore(uint32 entry);

#endif

