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

/// \addtogroup world The World
/// @{
/// \file

#ifndef __WORLD_H
#define __WORLD_H

#include "Common.h"
#include "Timer.h"
#include "Policies/Singleton.h"
#include "SharedDefines.h"
#include "QueryResult.h"
#include "Callback.h"

#include <map>
#include <set>
#include <list>

class Object;
class WorldPacket;
class WorldSession;
class Player;
struct Gladiator;
class Weather;
struct ScriptAction;
struct ScriptInfo;
class SQLResultQueue;
class WorldSocket;
class ArenaTeam;
struct CharTitlesEntry;

typedef std::unordered_map<uint32, WorldSession*> SessionMap;

struct GlobalPlayerData
{
    uint32 guidLow;
    uint32 accountId;
    std::string name;
    uint8 race;
    uint8 playerClass;
    uint8 gender;
    uint8 level;
    uint16 mailCount;
    uint32 guildId;
    uint32 groupId;
    uint32 arenaTeamId[3];
};

enum GlobalPlayerUpdateMask
{
    PLAYER_UPDATE_DATA_LEVEL = 0x01,
    PLAYER_UPDATE_DATA_RACE = 0x02,
    PLAYER_UPDATE_DATA_CLASS = 0x04,
    PLAYER_UPDATE_DATA_GENDER = 0x08,
    PLAYER_UPDATE_DATA_NAME = 0x10,
};

typedef std::unordered_map<uint32, GlobalPlayerData> GlobalPlayerDataMap;
typedef std::map<std::string, uint32> GlobalPlayerNameMap;

enum ShutdownMask
{
    SHUTDOWN_MASK_RESTART = 1,
    SHUTDOWN_MASK_IDLE    = 2,
};

enum ShutdownExitCode
{
    SHUTDOWN_EXIT_CODE = 0,
    ERROR_EXIT_CODE    = 1,
    RESTART_EXIT_CODE  = 2,
};

/// Timers for different object refresh rates
enum WorldTimers
{
    WUPDATE_OBJECTS        = 0,
    WUPDATE_SESSIONS       = 1,
    WUPDATE_AUCTIONS       = 2,
    WUPDATE_WEATHERS       = 3,
    WUPDATE_UPTIME         = 4,
    WUPDATE_CORPSES        = 5,
    WUPDATE_EVENTS         = 6,
    WUPDATE_ANNOUNCES      = 7,
    WUPDATE_ARENASEASONLOG = 8,

    WUPDATE_COUNT          = 9
};

/// Configuration elements
enum WorldConfigs
{
    CONFIG_COMPRESSION = 0,
    CONFIG_GRID_UNLOAD,
    CONFIG_INTERVAL_SAVE,
    CONFIG_INTERVAL_GRIDCLEAN,
    CONFIG_INTERVAL_MAPUPDATE,
    CONFIG_INTERVAL_CHANGEWEATHER,
    CONFIG_INTERVAL_DISCONNECT_TOLERANCE,
    CONFIG_PORT_WORLD,
    CONFIG_SOCKET_SELECTTIME,
    CONFIG_SOCKET_TIMEOUTTIME,
    CONFIG_GROUP_XP_DISTANCE,
    CONFIG_SIGHT_MONSTER,
    CONFIG_SIGHT_GUARDER,
    CONFIG_GAME_TYPE,
    CONFIG_REALM_ZONE,
    CONFIG_ALLOW_TWO_SIDE_ACCOUNTS,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION,
    CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL,
    CONFIG_ALLOW_TWO_SIDE_WHO_LIST,
    CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND,
    CONFIG_ALLOW_TWO_SIDE_TRADE,
    CONFIG_STRICT_PLAYER_NAMES,
    CONFIG_STRICT_CHARTER_NAMES,
    CONFIG_STRICT_PET_NAMES,
    CONFIG_CHARACTERS_CREATING_DISABLED,
    CONFIG_CHARACTERS_PER_ACCOUNT,
    CONFIG_CHARACTERS_PER_REALM,
    CONFIG_SKIP_CINEMATICS,
    CONFIG_MAX_PLAYER_LEVEL,
    CONFIG_START_PLAYER_LEVEL,
    CONFIG_START_PLAYER_MONEY,
    CONFIG_MAX_HONOR_POINTS,
    CONFIG_START_HONOR_POINTS,
    CONFIG_MAX_ARENA_POINTS,
    CONFIG_START_ARENA_POINTS,
    CONFIG_INSTANCE_IGNORE_LEVEL,
    CONFIG_INSTANCE_IGNORE_RAID,
    CONFIG_BATTLEGROUND_CAST_DESERTER,
    CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE,
    CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY,
    CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_WORLDONLY,
    CONFIG_BATTLEGROUND_ARENA_RATED_ENABLE,
    CONFIG_BATTLEGROUND_ARENA_CLOSE_AT_NIGHT_MASK,
    CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING,
    CONFIG_BATTLEGROUND_ARENA_ANNOUNCE,
    CONFIG_INSTANCE_RESET_TIME_HOUR,
    CONFIG_INSTANCE_UNLOAD_DELAY,
    CONFIG_CAST_UNSTUCK,
    CONFIG_MAX_PRIMARY_TRADE_SKILL,
    CONFIG_MIN_PETITION_SIGNS,
    CONFIG_GM_LOGIN_STATE,
    CONFIG_GM_VISIBLE_STATE,
    CONFIG_GM_CHAT,
    CONFIG_GM_WISPERING_TO,
    CONFIG_GM_LEVEL_IN_GM_LIST,
    CONFIG_GM_LEVEL_IN_WHO_LIST,
    CONFIG_START_GM_LEVEL,
    CONFIG_ALLOW_GM_GROUP,
    CONFIG_ALLOW_GM_FRIEND,
    CONFIG_GM_DEFAULT_GUILD,
    CONFIG_GM_FORCE_GUILD,
    CONFIG_GROUP_VISIBILITY,
    CONFIG_MAIL_DELIVERY_DELAY,
    CONFIG_UPTIME_UPDATE,
    CONFIG_SKILL_CHANCE_ORANGE,
    CONFIG_SKILL_CHANCE_YELLOW,
    CONFIG_SKILL_CHANCE_GREEN,
    CONFIG_SKILL_CHANCE_GREY,
    CONFIG_SKILL_CHANCE_MINING_STEPS,
    CONFIG_SKILL_CHANCE_SKINNING_STEPS,
    CONFIG_SKILL_PROSPECTING,
    CONFIG_SKILL_GAIN_CRAFTING,
    CONFIG_SKILL_GAIN_DEFENSE,
    CONFIG_SKILL_GAIN_GATHERING,
    CONFIG_SKILL_GAIN_WEAPON,
    CONFIG_MAX_OVERSPEED_PINGS,
    CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY,
    CONFIG_ALWAYS_MAX_SKILL_FOR_LEVEL,
    CONFIG_WEATHER,
    CONFIG_EXPANSION,
    CONFIG_CHATFLOOD_MESSAGE_COUNT,
    CONFIG_CHATFLOOD_MESSAGE_DELAY,
    CONFIG_CHATFLOOD_MUTE_TIME,
    CONFIG_EVENT_ANNOUNCE,
    CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS,
    CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY,
    CONFIG_CREATURE_FAMILY_FLEE_DELAY,
    CONFIG_CREATURE_MAX_UNREACHABLE_TARGET_TIME,
    CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF,
    CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF,
    CONFIG_RESTRICTED_LFG_CHANNEL,
    CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL,
    CONFIG_TALENTS_INSPECTING,
    CONFIG_CHAT_FAKE_MESSAGE_PREVENTING,
    CONFIG_CORPSE_DECAY_NORMAL,
    CONFIG_CORPSE_DECAY_RARE,
    CONFIG_CORPSE_DECAY_ELITE,
    CONFIG_CORPSE_DECAY_RAREELITE,
    CONFIG_CORPSE_DECAY_WORLDBOSS,
    CONFIG_ADDON_CHANNEL,
    CONFIG_DEATH_SICKNESS_LEVEL,
    CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP,
    CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE,
    CONFIG_DEATH_BONES_WORLD,
    CONFIG_DEATH_BONES_BG_OR_ARENA,
    CONFIG_THREAT_RADIUS,
    CONFIG_INSTANT_LOGOUT,
    CONFIG_GROUPLEADER_RECONNECT_PERIOD,
    CONFIG_ALL_TAXI_PATHS,
    CONFIG_INSTANT_TAXI,
    CONFIG_DECLINED_NAMES_USED,
    CONFIG_LISTEN_RANGE_SAY,
    CONFIG_LISTEN_RANGE_TEXTEMOTE,
    CONFIG_LISTEN_RANGE_YELL,
    CONFIG_ARENA_MAX_RATING_DIFFERENCE,
    CONFIG_ARENA_RATING_DISCARD_TIMER,
    CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS,
    CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS,
    CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER,
    CONFIG_BATTLEGROUND_TIMELIMIT_WARSONG,
    CONFIG_BATTLEGROUND_TIMELIMIT_ARENA,

    CONFIG_START_ALL_SPELLS,
    CONFIG_START_ALL_EXPLORED,
    CONFIG_START_ALL_REP,
    CONFIG_ALWAYS_MAXSKILL,
    CONFIG_PVP_TOKEN_ENABLE,
    CONFIG_PVP_TOKEN_MAP_TYPE,
    CONFIG_PVP_TOKEN_ID,
    CONFIG_PVP_TOKEN_COUNT,
    CONFIG_PVP_TOKEN_ZONE_ID,
    CONFIG_NO_RESET_TALENT_COST,
    CONFIG_SHOW_KICK_IN_WORLD,
    CONFIG_INTERVAL_LOG_UPDATE,
    CONFIG_MIN_LOG_UPDATE,
    CONFIG_ENABLE_SINFO_LOGIN,
    CONFIG_PREMATURE_BG_REWARD,
    CONFIG_PET_LOS,
    CONFIG_NUMTHREADS,
    
    CONFIG_WORLDCHANNEL_MINLEVEL,

    CONFIG_VMAP_INDOOR_CHECK,
    CONFIG_VMAP_INDOOR_INST_CHECK,
    CONFIG_BOOL_MMAP_ENABLED,
    
    //logs duration in days. -1 to keep forever. 0 to disable logging. 
    CONFIG_LOG_BG_STATS,
    CONFIG_LOG_BOSS_DOWNS,
    CONFIG_LOG_CHAR_DELETE,
    CONFIG_GM_LOG_CHAR_DELETE,
    CONFIG_LOG_CHAR_CHAT,
    CONFIG_GM_LOG_CHAR_CHAT,
    CONFIG_LOG_CHAR_GUILD_MONEY,
    CONFIG_GM_LOG_CHAR_GUILD_MONEY,
    CONFIG_LOG_CHAR_ITEM_DELETE,
    CONFIG_GM_LOG_CHAR_ITEM_DELETE,
    CONFIG_LOG_CHAR_ITEM_GUILD_BANK,
    CONFIG_GM_LOG_CHAR_ITEM_GUILD_BANK,
    CONFIG_LOG_CHAR_ITEM_VENDOR,
    CONFIG_GM_LOG_CHAR_ITEM_VENDOR,
    CONFIG_LOG_CHAR_ITEM_ENCHANT,
    CONFIG_GM_LOG_CHAR_ITEM_ENCHANT,
    CONFIG_LOG_CHAR_ITEM_AUCTION,
    CONFIG_GM_LOG_CHAR_ITEM_AUCTION,
    CONFIG_LOG_CHAR_ITEM_TRADE,
    CONFIG_GM_LOG_CHAR_ITEM_TRADE,
    CONFIG_LOG_CHAR_MAIL,
    CONFIG_GM_LOG_CHAR_MAIL,
    CONFIG_LOG_CHAR_RENAME,
    CONFIG_GM_LOG_CHAR_RENAME,
    CONFIG_LOG_GM_COMMANDS,
    CONFIG_LOG_SANCTIONS,

    CONFIG_MAX_AVERAGE_TIMEDIFF,
    
    CONFIG_PLAYER_GENDER_CHANGE_DELAY,

    CONFIG_MONITORING_ENABLED,
    CONFIG_MONITORING_UPDATE,
    CONFIG_MONITORING_KEEP_DURATION,
    
    CONFIG_MYSQL_BUNDLE_LOGINDB,
    CONFIG_MYSQL_BUNDLE_CHARDB,
    CONFIG_MYSQL_BUNDLE_WORLDDB,

    CONFIG_BUGGY_QUESTS_AUTOCOMPLETE,
    
    CONFIG_AUTOANNOUNCE_ENABLED,
    
    CONFIG_WARDEN_ENABLED,
    CONFIG_WARDEN_KICK,
    CONFIG_WARDEN_NUM_CHECKS,
    CONFIG_WARDEN_CLIENT_CHECK_HOLDOFF,
    CONFIG_WARDEN_CLIENT_RESPONSE_DELAY,
    CONFIG_WARDEN_DB_LOG,
    CONFIG_WARDEN_BAN_TIME,
    
    CONFIG_WHISPER_MINLEVEL,

    CONFIG_GUIDDISTRIB_NEWMETHOD,
    CONFIG_GUIDDISTRIB_PROPORTION,

    CONFIG_ARENA_SPECTATOR_ENABLE,
    CONFIG_ARENA_SPECTATOR_MAX,
    CONFIG_ARENA_SPECTATOR_GHOST,
    CONFIG_ARENA_SPECTATOR_STEALTH,

    CONFIG_ARENA_SEASON,
    CONFIG_ARENA_NEW_TITLE_DISTRIB,
    CONFIG_ARENA_NEW_TITLE_DISTRIB_MIN_RATING,
    CONFIG_ARENA_DECAY_ENABLED,
    CONFIG_ARENA_DECAY_MINIMUM_RATING,
    CONFIG_ARENA_DECAY_VALUE,
    CONFIG_ARENA_DECAY_CONSECUTIVE_WEEKS,

    CONFIG_IRC_ENABLED,
    CONFIG_IRC_COMMANDS,
    
    CONFIG_PACKET_SPOOF_POLICY,
    CONFIG_PACKET_SPOOF_BANMODE,
    CONFIG_PACKET_SPOOF_BANDURATION,

    CONFIG_SPAM_REPORT_THRESHOLD,
    CONFIG_SPAM_REPORT_PERIOD,
    CONFIG_SPAM_REPORT_COOLDOWN,
    
    CONFIG_FACTION_CHANGE_ENABLED,
    CONFIG_FACTION_CHANGE_A2H,
    CONFIG_FACTION_CHANGE_H2A,
    CONFIG_FACTION_CHANGE_A2H_COST,
    CONFIG_FACTION_CHANGE_H2A_COST,
    CONFIG_RACE_CHANGE_COST, //Not interfaction

    CONFIG_ARENASERVER_ENABLED,
    CONFIG_ARENASERVER_USE_CLOSESCHEDULE,
    CONFIG_ARENASERVER_PLAYER_REPARTITION_THRESHOLD,

    CONFIG_DEBUG_DISABLE_MAINHAND,
    CONFIG_DEBUG_DISABLE_ARMOR,
    CONFIG_DEBUG_LOG_LAST_PACKETS, //See WorldSocket::_lastPacketSent
    CONFIG_DEBUG_LOG_ALL_PACKETS,
    CONFIG_DEBUG_DISABLE_CREATURES_LOADING,

    CONFIG_CLIENTCACHE_VERSION,

    CONFIG_VALUE_COUNT,
};

/// Server rates
enum Rates
{
    RATE_HEALTH=0,
    RATE_POWER_MANA,
    RATE_POWER_RAGE_INCOME,
    RATE_POWER_RAGE_LOSS,
    RATE_POWER_FOCUS,
    RATE_SKILL_DISCOVERY,
    RATE_DROP_ITEM_POOR,
    RATE_DROP_ITEM_NORMAL,
    RATE_DROP_ITEM_UNCOMMON,
    RATE_DROP_ITEM_RARE,
    RATE_DROP_ITEM_EPIC,
    RATE_DROP_ITEM_LEGENDARY,
    RATE_DROP_ITEM_ARTIFACT,
    RATE_DROP_ITEM_REFERENCED,
    RATE_DROP_MONEY,
    RATE_XP_KILL,
    RATE_XP_QUEST,
    RATE_XP_EXPLORE,
    RATE_XP_PAST_70,
    RATE_REPUTATION_GAIN,
    RATE_CREATURE_AGGRO,
    RATE_REST_INGAME,
    RATE_REST_OFFLINE_IN_TAVERN_OR_CITY,
    RATE_REST_OFFLINE_IN_WILDERNESS,
    RATE_DAMAGE_FALL,
    RATE_AUCTION_TIME,
    RATE_AUCTION_DEPOSIT,
    RATE_AUCTION_CUT,
    RATE_HONOR,
    RATE_MINING_AMOUNT,
    RATE_MINING_NEXT,
    RATE_TALENT,
    RATE_LOYALTY,
    RATE_CORPSE_DECAY_LOOTED,
    RATE_INSTANCE_RESET_TIME,
    RATE_TARGET_POS_RECALCULATION_RANGE,
    RATE_DURABILITY_LOSS_DAMAGE,
    RATE_DURABILITY_LOSS_PARRY,
    RATE_DURABILITY_LOSS_ABSORB,
    RATE_DURABILITY_LOSS_BLOCK,
    MAX_RATES
};

/// Type of server
enum RealmType
{
    REALM_TYPE_NORMAL = 0,
    REALM_TYPE_PVP = 1,
    REALM_TYPE_NORMAL2 = 4,
    REALM_TYPE_RP = 6,
    REALM_TYPE_RPPVP = 8,
    REALM_TYPE_FFA_PVP = 16                                 // custom, free for all pvp mode like arena PvP in all zones except rest activated places and sanctuaries
                                                            // replaced by REALM_PVP in realm list
};

enum RealmZone
{
    REALM_ZONE_UNKNOWN       = 0,                           // any language
    REALM_ZONE_DEVELOPMENT   = 1,                           // any language
    REALM_ZONE_UNITED_STATES = 2,                           // extended-Latin
    REALM_ZONE_OCEANIC       = 3,                           // extended-Latin
    REALM_ZONE_LATIN_AMERICA = 4,                           // extended-Latin
    REALM_ZONE_TOURNAMENT_5  = 5,                           // basic-Latin at create, any at login
    REALM_ZONE_KOREA         = 6,                           // East-Asian
    REALM_ZONE_TOURNAMENT_7  = 7,                           // basic-Latin at create, any at login
    REALM_ZONE_ENGLISH       = 8,                           // extended-Latin
    REALM_ZONE_GERMAN        = 9,                           // extended-Latin
    REALM_ZONE_FRENCH        = 10,                          // extended-Latin
    REALM_ZONE_SPANISH       = 11,                          // extended-Latin
    REALM_ZONE_RUSSIAN       = 12,                          // Cyrillic
    REALM_ZONE_TOURNAMENT_13 = 13,                          // basic-Latin at create, any at login
    REALM_ZONE_TAIWAN        = 14,                          // East-Asian
    REALM_ZONE_TOURNAMENT_15 = 15,                          // basic-Latin at create, any at login
    REALM_ZONE_CHINA         = 16,                          // East-Asian
    REALM_ZONE_CN1           = 17,                          // basic-Latin at create, any at login
    REALM_ZONE_CN2           = 18,                          // basic-Latin at create, any at login
    REALM_ZONE_CN3           = 19,                          // basic-Latin at create, any at login
    REALM_ZONE_CN4           = 20,                          // basic-Latin at create, any at login
    REALM_ZONE_CN5           = 21,                          // basic-Latin at create, any at login
    REALM_ZONE_CN6           = 22,                          // basic-Latin at create, any at login
    REALM_ZONE_CN7           = 23,                          // basic-Latin at create, any at login
    REALM_ZONE_CN8           = 24,                          // basic-Latin at create, any at login
    REALM_ZONE_TOURNAMENT_25 = 25,                          // basic-Latin at create, any at login
    REALM_ZONE_TEST_SERVER   = 26,                          // any language
    REALM_ZONE_TOURNAMENT_27 = 27,                          // basic-Latin at create, any at login
    REALM_ZONE_QA_SERVER     = 28,                          // any language
    REALM_ZONE_CN9           = 29                           // basic-Latin at create, any at login
};

struct AutoAnnounceMessage
{
    std::string message;
    uint64 nextAnnounce;
};

enum HonorKillPvPRank
{
    HKRANK00,
    HKRANK01,
    HKRANK02,
    HKRANK03,
    HKRANK04,
    HKRANK05,
    HKRANK06,
    HKRANK07,
    HKRANK08,
    HKRANK09,
    HKRANK10,
    HKRANK11,
    HKRANK12,
    HKRANK13,
    HKRANK14,
    HKRANKMAX
};

// DB scripting commands
enum ScriptCommands
{
    SCRIPT_COMMAND_TALK =                  0,              // source = unit, target=any, datalong (0,=say,1,=whisper,2,=yell,3,=emote text)
    SCRIPT_COMMAND_EMOTE =                 1,              // source = unit, datalong = anim_id
    SCRIPT_COMMAND_FIELD_SET =             2,              // source = any, datalong = field_id, datalog2 = value
    SCRIPT_COMMAND_MOVE_TO =               3,              // source = Creature, datalog2 = time, x/y/z
    SCRIPT_COMMAND_FLAG_SET =              4,              // source = any, datalong = field_id, datalog2 = bitmask
    SCRIPT_COMMAND_FLAG_REMOVE =           5,              // source = any, datalong = field_id, datalog2 = bitmask
    SCRIPT_COMMAND_TELEPORT_TO =           6,              // source or target with Player, datalong = map_id, x/y/z
    SCRIPT_COMMAND_QUEST_EXPLORED =        7,              // one from source or target must be Player, another GO/Creature, datalong=quest_id, datalong2=distance or0,
    SCRIPT_COMMAND_KILL_CREDIT =           8,              // source = player, target = player, datalong = kill credit entry
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT =    9,              // source = any (summoner), datalong=db_guid, datalong2=despawn_delay
    SCRIPT_COMMAND_TEMP_SUMMON_CREATURE = 10,              // source = any (summoner), datalong=creature entry, datalong2=despawn_delay
    SCRIPT_COMMAND_OPEN_DOOR =            11,              // source = unit, datalong=db_guid, datalong2=reset_delay
    SCRIPT_COMMAND_CLOSE_DOOR =           12,              // source = unit, datalong=db_guid, datalong2=reset_delay
    SCRIPT_COMMAND_ACTIVATE_OBJECT =      13,              // source = unit, target=GO
    SCRIPT_COMMAND_REMOVE_AURA =          14,              // source (datalong2!=0) or target (datalong==0) unit, datalong = spell_id
    SCRIPT_COMMAND_CAST_SPELL =           15,              // source (datalong2!=0) or target (datalong==0) unit, datalong = spell_id
    SCRIPT_COMMAND_PLAYSOUND =            16,              // datalong soundid, datalong2 play only self

    SCRIPT_COMMAND_LOAD_PATH =            20,              // source = unit, path = datalong
    SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT =   21,              // datalong scriptid, lowguid datalong2, dataint table
    SCRIPT_COMMAND_KILL =                 22,              // datalong removecorpse
};

/// Storage class for commands issued for delayed execution
struct CliCommandHolder
{
    typedef void Print(void*, const char*);
    typedef void CommandFinished(void*, bool success);

    void* m_callbackArg;
    char *m_command;
    Print* m_print;

    CommandFinished* m_commandFinished;

    CliCommandHolder(void* callbackArg, const char *command, Print* zprint, CommandFinished* commandFinished)
        : m_callbackArg(callbackArg), m_command(strdup(command)), m_print(zprint), m_commandFinished(commandFinished)
    {
    }

    ~CliCommandHolder() { free(m_command); }

private:
    CliCommandHolder(CliCommandHolder const& right) = delete;
    CliCommandHolder& operator=(CliCommandHolder const& right) = delete;
};
/// The World
class World
{
    public:
        static volatile uint32 m_worldLoopCounter;

        static World* instance()
        {
            static World instance;
            return &instance;
        }

        World();
        ~World();

        WorldSession* FindSession(uint32 id) const;
        void AddSession(WorldSession *s);
        bool RemoveSession(uint32 id);
        /// Get the number of current active sessions
        const SessionMap& GetAllSessions() const { return m_sessions; }
        void UpdateMaxSessionCounters();
        uint32 GetActiveAndQueuedSessionCount() const { return m_sessions.size(); }
        uint32 GetActiveSessionCount() const { return m_sessions.size() - m_QueuedPlayer.size(); }
        uint32 GetQueuedSessionCount() const { return m_QueuedPlayer.size(); }
        /// Get the maximum number of parallel sessions on the server since last reboot
        uint32 GetMaxQueuedSessionCount() const { return m_maxQueuedSessionCount; }
        uint32 GetMaxActiveSessionCount() const { return m_maxActiveSessionCount; }
        Player* FindPlayerInZone(uint32 zone);

        Weather* FindWeather(uint32 id) const;
        Weather* AddWeather(uint32 zone_id);
        void RemoveWeather(uint32 zone_id);

        static bool IsZoneSanctuary(uint32);
        static bool IsZoneFFA(uint32);

        /// Get the active session server limit (or security level limitations)
        uint32 GetPlayerAmountLimit() const { return m_playerLimit >= 0 ? m_playerLimit : 0; }
        AccountTypes GetPlayerSecurityLimit() const { return m_allowedSecurityLevel < 0 ? SEC_PLAYER : m_allowedSecurityLevel; }

        /// Set the active session server limit (or security level limitation)
        void SetPlayerLimit(int32 limit);

        //player Queue
        typedef std::list<WorldSession*> Queue;
        void AddQueuedPlayer(WorldSession*);
        bool RemoveQueuedPlayer(WorldSession* session);
        int32 GetQueuePos(WorldSession*);
        bool HasRecentlyDisconnected(WorldSession*);
        uint32 GetQueueSize() const { return m_QueuedPlayer.size(); }

        /// Set a new Message of the Day
        void SetMotd(std::string motd) { m_motd = motd; }
        /// Get the current Message of the Day
        const char* GetMotd() const { return m_motd.c_str(); }
        /// Set a new last Twitter
        void SetTwitter(std::string twitt) { m_lastTwitter = twitt; }
        /// Get the last Twitter
        const char* GetLastTwitter() const { return m_lastTwitter.c_str(); }

        /// Set the string for new characters (first login)
        void SetNewCharString(std::string str) { m_newCharString = str; }
        /// Get the string for new characters (first login)
        const std::string& GetNewCharString() const { return m_newCharString; }

        LocaleConstant GetDefaultDbcLocale() const { return m_defaultDbcLocale; }

        /// Get the path where data (dbc, maps) are stored on disk
        std::string GetDataPath() const { return m_dataPath; }

        /// When server started?
        time_t const& GetStartTime() const { return m_startTime; }
        /// What time is it?
        time_t const& GetGameTime() const { return m_gameTime; }
        /// Uptime (in secs)
        uint32 GetUptime() const { return uint32(m_gameTime - m_startTime); }
        /// Update time
        uint32 GetUpdateTime() const { return m_updateTime; }
        uint32 GetFastTimeDiff() const { return fastTd; }
        void SetRecordDiffInterval(int32 t) { if(t >= 0) m_configs[CONFIG_INTERVAL_LOG_UPDATE] = (uint32)t; }

        /// Next daily quests and random bg reset time
        time_t GetNextDailyQuestsResetTime() const { return m_NextDailyQuestReset; }

        /// Get the maximum skill level a player can reach
        uint16 GetConfigMaxSkillValue() const
        {
            uint32 lvl = getConfig(CONFIG_MAX_PLAYER_LEVEL);
            return lvl > 60 ? 300 + ((lvl - 60) * 75) / 10 : lvl*5;
        }

        void SetInitialWorldSettings();
        void LoadConfigSettings(bool reload = false);
        void LoadMotdAndTwitter();

        void SendWorldText(int32 string_id, ...);
        void SendGlobalText(const char* text, WorldSession *self);
        void SendGMText(int32 string_id, ...);
        void SendGlobalMessage(WorldPacket *packet, WorldSession *self = 0, uint32 team = 0);
        void SendGlobalGMMessage(WorldPacket *packet, WorldSession *self = 0, uint32 team = 0);
        void SendZoneMessage(uint32 zone, WorldPacket *packet, WorldSession *self = 0, uint32 team = 0);
        void SendZoneText(uint32 zone, const char *text, WorldSession *self = 0, uint32 team = 0);
        void SendServerMessage(uint32 type, const char *text = "", Player* player = NULL);

        //Force all connected clients to clear specified player cache
        void InvalidatePlayerDataToAllClient(uint64 guid);

        void SendZoneUnderAttack(uint32 zoneId, Team team);

        uint32 pvp_ranks[HKRANKMAX];
        uint32 confStaticLeaders[12];
        std::list<Gladiator> confGladiators;

        /// Are we in the middle of a shutdown?
        bool IsShuttingDown() const { return IsStopped() || m_ShutdownTimer > 0; }
        uint32 const GetShutDownTimeLeft() { return m_ShutdownTimer; }
        void ShutdownServ(uint32 time, uint32 options, /*uint8 exitcode*/ const char* reason);
        void ShutdownCancel();
        void ShutdownMsg(bool show = false, Player* player = NULL, std::string reason = "");
        std::string GetShutdownReason() { return m_ShutdownReason; }
        static uint8 GetExitCode() { return m_ExitCode; }
        static void StopNow(uint8 exitcode) { m_stopEvent = true; m_ExitCode = exitcode; }
        static bool IsStopped() { return m_stopEvent; }

        void Update(time_t diff);

        void UpdateSessions(uint32 diff);
        /// Set a server rate (see #Rates)
        void setRate(Rates rate,float value) { rate_values[rate]=value; }
        /// Get a server rate (see #Rates)
        float GetRate(Rates rate) const { return rate_values[rate]; }

        /// Set a server configuration element (see #WorldConfigs)
        void setConfig(uint32 index,uint32 value)
        {
            if(index < CONFIG_VALUE_COUNT)
                m_configs[index]=value;
        }

        /// Get a server configuration element (see #WorldConfigs)
        uint32 getConfig(uint32 index) const
        {
            if(index<CONFIG_VALUE_COUNT)
                return m_configs[index];
            else
                return 0;
        }
        //temp compatibility macros
#define getIntConfig(a) getConfig(a)
#define getBoolConfig(a) getConfig(a)

        /// Are we on a "Player versus Player" server?
        bool IsPvPRealm() { return (getConfig(CONFIG_GAME_TYPE) == REALM_TYPE_PVP || getConfig(CONFIG_GAME_TYPE) == REALM_TYPE_RPPVP || getConfig(CONFIG_GAME_TYPE) == REALM_TYPE_FFA_PVP); }
        bool IsFFAPvPRealm() { return getConfig(CONFIG_GAME_TYPE) == REALM_TYPE_FFA_PVP; }

        bool KickPlayer(const std::string& playerName);
        void KickAll();
        void KickAllLess(AccountTypes sec);
        //author_session can be null
        BanReturn BanAccount(SanctionType mode, std::string const& nameOrIP, std::string const& duration, std::string const& reason, std::string const& author, WorldSession const* author_session);
        //author_session can be null
        BanReturn BanAccount(SanctionType mode, std::string const& nameOrIP, uint32 duration_secs, std::string const& reason, std::string const& author, WorldSession const* author_session);
        // unbanAuthor can be null
        bool RemoveBanAccount(SanctionType mode, std::string nameOrIP, WorldSession const* unbanAuthor);

        void ScriptsStart(std::map<uint32, std::multimap<uint32, ScriptInfo> > const& scripts, uint32 id, Object* source, Object* target, bool start = true);
        void ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target);
        bool IsScriptScheduled() const { return !m_scriptSchedule.empty(); }

        bool IsAllowedMap(uint32 mapid) { return m_forbiddenMapIds.count(mapid) == 0 ;}

        // for max speed access
        static float GetMaxVisibleDistanceOnContinents()    { return m_MaxVisibleDistanceOnContinents; }
        static float GetMaxVisibleDistanceInInstances()     { return m_MaxVisibleDistanceInInstances;  }
        static float GetMaxVisibleDistanceInBGArenas()      { return m_MaxVisibleDistanceInBGArenas;   }
        static float GetMaxVisibleDistanceForObject()   { return m_MaxVisibleDistanceForObject;   }
        static float GetMaxVisibleDistanceInFlight()    { return m_MaxVisibleDistanceInFlight;    }
        static float GetVisibleUnitGreyDistance()       { return m_VisibleUnitGreyDistance;       }
        static float GetVisibleObjectGreyDistance()     { return m_VisibleObjectGreyDistance;     }

        //movement anticheat enable flag
        inline bool GetMvAnticheatEnable()             {return m_MvAnticheatEnable;}
        inline bool GetMvAnticheatKick()               {return m_MvAnticheatKick;}
        inline uint32 GetMvAnticheatAlarmCount()       {return m_MvAnticheatAlarmCount;}
        inline uint32 GetMvAnticheatAlarmPeriod()      {return m_MvAnticheatAlarmPeriod;}
        inline unsigned char GetMvAnticheatBan()       {return m_MvAntiCheatBan;}
        inline std::string GetMvAnticheatBanTime()     {return m_MvAnticheatBanTime;}
        inline unsigned char GetMvAnticheatGmLevel()   {return m_MvAnticheatGmLevel;}
        inline bool GetMvAnticheatKill()               {return m_MvAnticheatKill;}
        inline bool GetMvAnticheatWarn()               {return m_MvAnticheatWarn;}

        inline std::string GetWardenBanTime()          {return m_wardenBanTime;}

        // xinef: Global Player Data Storage system
        void LoadGlobalPlayerDataStore();
        uint32 GetGlobalPlayerGUID(std::string const& name) const;
        GlobalPlayerData const* GetGlobalPlayerData(uint32 guid) const;
        bool HasGlobalPlayerData(uint32 guidLow) const;
        void AddGlobalPlayerData(uint32 guid, uint32 accountId, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level, uint16 mailCount, uint32 guildId);
        void UpdateGlobalPlayerData(uint32 guid, uint8 mask, std::string const& name, uint8 level = 0, uint8 gender = 0, uint8 race = 0, uint8 playerClass = 0);
        void UpdateGlobalPlayerMails(uint32 guid, int16 count, bool add = true);
        void UpdateGlobalPlayerGuild(uint32 guid, uint32 guildId);
        void UpdateGlobalPlayerGroup(uint32 guid, uint32 groupId);
        void UpdateGlobalPlayerArenaTeam(uint32 guid, uint8 slot, uint32 arenaTeamId);
        void UpdateGlobalNameData(uint32 guidLow, std::string const& oldName, std::string const& newName);
        void DeleteGlobalPlayerData(uint32 guid, std::string const& name);

        void ProcessCliCommands();
        void QueueCliCommand(CliCommandHolder* commandHolder) { cliCmdQueue.add(commandHolder); }

        void ForceGameEventUpdate();

        void UpdateRealmCharCount(uint32 accid);

        void UpdateAllowedSecurity();

        /// Deny clients?
        bool IsClosed() const;

        /// Close world
        void SetClosed(bool val);

        LocaleConstant GetAvailableDbcLocale(LocaleConstant locale) const { if(m_availableDbcLocaleMask & (1 << locale)) return locale; else return m_defaultDbcLocale; }

        //used World DB version
        void LoadDBVersion();
        char const* GetDBVersion() { return m_DBVersion.c_str(); }

        //used Script version
        void SetScriptsVersion(char const* version) { m_ScriptsVersion = version ? version : "unknown scripting library"; }
        char const* GetScriptsVersion() { return m_ScriptsVersion.c_str(); }

        /** Record current time to start from for the next RecordTimeDiff call */
        void ResetTimeDiffRecord();
        /** Print time diff since last ResetTimeDiffRecord() or since last RecordTimeDiff() (this function also does the reset) */
        void RecordTimeDiff(std::string const& text);
        
        uint32 GetCurrentQuestForPool(uint32 poolId);
        bool IsQuestInAPool(uint32 questId);
        bool IsQuestCurrentOfAPool(uint32 questId);
        bool IsPhishing(std::string msg);
        void LogPhishing(uint32 src, uint32 dst, std::string msg);
        void ResetDailyQuests();
        void LoadAutoAnnounce();
        
        std::vector<ArenaTeam*> getArenaLeaderTeams() { return firstArenaTeams; };
        void updateArenaLeaderTeams(uint8 maxcount, uint8 type = 2, uint32 minimalRating = 1800);
        void updateArenaLeadersTitles();
        //must be between 1 and 3
        CharTitlesEntry const* getArenaLeaderTitle(uint8 rank);
        CharTitlesEntry const* getGladiatorTitle(uint8 rank);

    protected:
        void _UpdateGameTime();
        void ScriptsProcess();
        // callback for UpdateRealmCharacters
        void _UpdateRealmCharCount(PreparedQueryResult resultCharCount);

        void InitDailyQuestResetTime();
        void InitNewDataForQuestPools();
        void LoadQuestPoolsData();
        void UpdateMonitoring(uint32 diff);
    private:

        void UpdateArenaSeasonLogs();

        static volatile bool m_stopEvent;
        static uint8 m_ExitCode;
        uint32 m_ShutdownTimer;
        uint32 m_ShutdownMask;
        std::string m_ShutdownReason;

        bool m_isClosed;

        time_t m_startTime;
        time_t m_gameTime;
        IntervalTimer m_timers[WUPDATE_COUNT];
        uint32 mail_timer;
        uint32 mail_timer_expires;
        uint32 m_updateTime, m_updateTimeSum;
        uint32 m_updateTimeCount;
        uint32 m_currentTime;
        uint32 m_updateTimeMon;

        typedef std::unordered_map<uint32, Weather* > WeatherMap;
        WeatherMap m_weathers;
        SessionMap m_sessions;
        typedef std::unordered_map<uint32, time_t> DisconnectMap;
        DisconnectMap m_disconnects;
        uint32 m_maxActiveSessionCount;
        uint32 m_maxQueuedSessionCount;

        std::multimap<time_t, ScriptAction> m_scriptSchedule;

        std::string m_newCharString;

        float rate_values[MAX_RATES];
        int32 m_configs[CONFIG_VALUE_COUNT];
        int32 m_playerLimit;
        AccountTypes m_allowedSecurityLevel;
        LocaleConstant m_defaultDbcLocale;                     // from config for one from loaded DBC locales
        uint32 m_availableDbcLocaleMask;                       // by loaded DBC
        void DetectDBCLang();
        std::string m_motd;
        std::string m_lastTwitter;
        std::string m_dataPath;
        std::set<uint32> m_forbiddenMapIds;

        // for max speed access
        static float m_MaxVisibleDistanceOnContinents;
        static float m_MaxVisibleDistanceInInstances;
        static float m_MaxVisibleDistanceInBGArenas;
        static float m_MaxVisibleDistanceForObject;
        static float m_MaxVisibleDistanceInFlight;
        static float m_VisibleUnitGreyDistance;
        static float m_VisibleObjectGreyDistance;

        //movement anticheat enable flag
        bool m_MvAnticheatEnable;
        bool m_MvAnticheatKick;
        uint32 m_MvAnticheatAlarmCount;
        uint32 m_MvAnticheatAlarmPeriod;
        unsigned char m_MvAntiCheatBan;
        std::string m_MvAnticheatBanTime;
        unsigned char m_MvAnticheatGmLevel;
        bool m_MvAnticheatKill;
        bool m_MvAnticheatWarn;

        std::string m_wardenBanTime;

       // CLI command holder to be thread safe
        LockedQueue<CliCommandHolder*> cliCmdQueue;

        // our speed ups
        GlobalPlayerDataMap _globalPlayerDataStore; // xinef
        GlobalPlayerNameMap _globalPlayerNameStore; // xinef

        // next daily quests reset time
        time_t m_NextDailyQuestReset;

        //Player Queue
        Queue m_QueuedPlayer;

        //sessions that are added async
        void AddSession_(WorldSession* s);
        LockedQueue<WorldSession*> addSessQueue;

        //used versions
        std::string m_DBVersion;
        std::string m_ScriptsVersion;
        
        std::vector<uint32> m_questInPools;
        std::map<uint32, uint32> m_currentQuestInPools;
        
        // Average timediff
        uint32 fastTdCount;
        uint32 fastTdSum;
        uint32 fastTd;        // Average td on 150 last loops (~30 sec)
        uint32 avgTdCount;
        uint32 avgTdSum;
        uint32 avgTd;        // Average td on 4500 last loops (~15 min). If that variable exceeds Config.World.MaxAverageTimediff, trigger an automatic restart
        
        std::map<uint32, AutoAnnounceMessage*> autoAnnounces;

        std::vector<ArenaTeam*> firstArenaTeams;

        void ProcessQueryCallbacks();
        std::deque<std::future<PreparedQueryResult>> m_realmCharCallbacks;
};

extern uint32 realmID;

#define sWorld World::instance()
#endif
/// @}

