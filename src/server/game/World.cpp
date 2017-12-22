
#include "AccountMgr.h"
#include "AddonMgr.h"
#include "ArenaTeam.h"
#include "AuctionHouseMgr.h"
#include "BattleGroundMgr.h"
#include "CellImpl.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "Common.h"
#include "ConditionMgr.h"
#include "Config.h"
#include "CreatureAIRegistry.h"
#include "CreatureGroups.h"
#include "CreatureTextMgr.h"
#include "DBCStores.h"
#include "GameEventMgr.h"
#include "GameObjectModel.h"
#include "GameTime.h"
#include "GitRevision.h"
#include "GlobalEvents.h"
#include "GridNotifiersImpl.h"
#include "InstanceSaveMgr.h"
#include "ItemEnchantmentMgr.h"
#include "Language.h"
#include "Monitor.h"
#include "Log.h"
#include "LogsDatabaseAccessor.h"
#include "LootMgr.h"
#include "M2Stores.h"
#include "Management/MMapFactory.h"
#include "Management/MMapManager.h"
#include "Management/VMapFactory.h"
#include "Management/VMapManager2.h"
#include "MapManager.h"
#include "Memory.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "Pet.h"
#include "PoolMgr.h"
#include "QueryCallback.h"
#include "ScriptMgr.h"
#include "ScriptReloadMgr.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "SmartAI.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "Transport.h"
#include "TransportMgr.h"
#include "Util.h"
#include "WardenDataStorage.h"
#include "WaypointManager.h"
#include "WaypointMovementGenerator.h"
#include "Weather.h"
#include "WhoListStorage.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#ifdef TESTS
#include "TestMgr.h"
#endif

#ifdef PLAYERBOT
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#endif

volatile bool World::m_stopEvent = false;
uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
volatile uint32 World::m_worldLoopCounter = 0;

float World::m_MaxVisibleDistanceOnContinents = DEFAULT_VISIBILITY_DISTANCE;
float World::m_MaxVisibleDistanceInInstances  = DEFAULT_VISIBILITY_INSTANCE;
float World::m_MaxVisibleDistanceInBGArenas   = DEFAULT_VISIBILITY_BGARENAS;
float World::m_MaxVisibleDistanceForObject    = DEFAULT_VISIBILITY_DISTANCE;

float World::m_MaxVisibleDistanceInFlight     = DEFAULT_VISIBILITY_DISTANCE;

TC_GAME_API int32 World::m_visibility_notify_periodOnContinents = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
TC_GAME_API int32 World::m_visibility_notify_periodInInstances = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
TC_GAME_API int32 World::m_visibility_notify_periodInBGArenas = DEFAULT_VISIBILITY_NOTIFY_PERIOD;

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME      = 1,
    SERVER_MSG_RESTART_TIME       = 2,
    SERVER_MSG_STRING             = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED = 4,
    SERVER_MSG_RESTART_CANCELLED  = 5
};

/// World constructor
World::World()
{
    m_playerLimit = 0;
    m_allowedSecurityLevel = SEC_PLAYER;
    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_maxActiveSessionCount = 0;
    m_maxQueuedSessionCount = 0;
    m_NextDailyQuestReset = 0;

    m_defaultDbcLocale = LOCALE_enUS;
    m_availableDbcLocaleMask = 0;

    m_isClosed = false;

    _guidWarn = false;
    _guidAlert = false;
    _warnDiff = 0;
    _warnShutdownTime = time(nullptr);
}

/// World destructor
World::~World()
{
    ///- Empty the kicked session set
    while (!m_sessions.empty())
    {
        // not remove from queue, prevent loading new sessions
        delete m_sessions.begin()->second;
        m_sessions.erase(m_sessions.begin());
    }

    ///- Empty the WeatherMap
    for (auto & m_weather : m_weathers)
        delete m_weather.second;

    CliCommandHolder* command = nullptr;
    while (cliCmdQueue.next(command))
        delete command;

    VMAP::VMapFactory::clear();
    MMAP::MMapFactory::clear();

    //TODO free addSessQueue
}

/// Find a player in a specified zone
Player* World::FindPlayerInZone(uint32 zone)
{
    ///- circle through active sessions and return the first player found in the zone
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if(!itr->second)
            continue;
        Player *player = itr->second->GetPlayer();
        if(!player)
            continue;
        if( player->IsInWorld() && player->GetZoneId() == zone )
        {
            // Used by the weather system. We return the player to broadcast the change weather message to him and all players in the zone.
            return player;
        }
    }
    return nullptr;
}

bool World::IsClosed() const
{
    return m_isClosed;
}

void World::SetClosed(bool val)
{
    m_isClosed = val;

    // Invert the value, for simplicity for scripters.
    //    sScriptMgr->OnOpenStateChange(!val);
}

void World::TriggerGuidWarning()
{
    // Lock this only to prevent multiple maps triggering at the same time
    std::lock_guard<std::mutex> lock(_guidAlertLock);

    time_t gameTime = GameTime::GetGameTime();
    time_t today = (gameTime / DAY) * DAY;

    // Check if our window to restart today has passed. 5 mins until quiet time
    while (gameTime >= (today + (getIntConfig(CONFIG_RESPAWN_RESTARTQUIETTIME) * HOUR) - 1810))
        today += DAY;

    // Schedule restart for 30 minutes before quiet time, or as long as we have
    _warnShutdownTime = today + (getIntConfig(CONFIG_RESPAWN_RESTARTQUIETTIME) * HOUR) - 1800;

    _guidWarn = true;
    SendGuidWarning();
}

void World::TriggerGuidAlert()
{
    // Lock this only to prevent multiple maps triggering at the same time
    std::lock_guard<std::mutex> lock(_guidAlertLock);

    DoGuidAlertRestart();
    _guidAlert = true;
    _guidWarn = false;
}

void World::DoGuidWarningRestart()
{
    if (m_ShutdownTimer)
        return;

    ShutdownServ(1800, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);
    _warnShutdownTime += HOUR;
}

void World::DoGuidAlertRestart()
{
    if (m_ShutdownTimer)
        return;

    ShutdownServ(300, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, _alertRestartReason);
}

void World::SendGuidWarning()
{
    if (!m_ShutdownTimer && _guidWarn && getIntConfig(CONFIG_RESPAWN_GUIDWARNING_FREQUENCY) > 0)
        SendServerMessage(SERVER_MSG_STRING, _guidWarningMsg.c_str());
    _warnDiff = 0;
}

/// Find a session by its id
WorldSession* World::FindSession(uint32 id) const
{
    auto itr = m_sessions.find(id);

    if(itr != m_sessions.end())
        return itr->second;                                 // also can return NULL for kicked session
    else
        return nullptr;
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
    ///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
    auto itr = m_sessions.find(id);

    if(itr != m_sessions.end() && itr->second)
    {
        if (itr->second->PlayerLoading())
            return false;
        itr->second->KickPlayer();
    }

    return true;
}

void World::AddSession(WorldSession* s)
{
    addSessQueue.add(s);
}

void
World::AddSession_(WorldSession* s)
{
    ASSERT (s);

    //NOTE - Still there is race condition in WorldSession* being used in the Sockets

    ///- kick already loaded player with same account (if any) and remove session
    ///- if player is in loading and want to load again, return
    if (!RemoveSession (s->GetAccountId ()))
    {
        s->KickPlayer ();
        delete s;                                           // session not added yet in session list, so not listed in queue
        return;
    }

    // decrease session counts only at not reconnection case
    bool decrease_session = true;

    // if session already exist, prepare to it deleting at next world update
    // NOTE - KickPlayer() should be called on "old" in RemoveSession()
    {
        SessionMap::const_iterator old = m_sessions.find(s->GetAccountId ());

        if(old != m_sessions.end())
        {
            // prevent decrease sessions count if session queued
            if(RemoveQueuedPlayer(old->second))
                decrease_session = false;
            // not remove replaced session form queue if listed
            delete old->second;
        }
    }

    m_sessions[s->GetAccountId ()] = s;

    uint32 Sessions = GetActiveAndQueuedSessionCount ();
    uint32 pLimit = GetPlayerAmountLimit ();

    //so we don't count the user trying to
    //login as a session and queue the socket that we are using
    if(decrease_session)
        --Sessions;

    if (pLimit > 0 && Sessions >= pLimit && s->GetSecurity () == SEC_PLAYER && !HasRecentlyDisconnected(s) )
    {
        AddQueuedPlayer (s);
        UpdateMaxSessionCounters ();
        //sLog->outDetail ("PlayerQueue: Account id %u is in Queue Position (%u).", s->GetAccountId (), ++QueueSize);
        return;
    }

    s->InitializeSession();

    UpdateMaxSessionCounters ();

    // Updates the population
    if (pLimit > 0)
    {
        float popu = GetActiveSessionCount (); //updated number of users on the server
        popu /= pLimit;
        popu *= 2;
        LoginDatabase.AsyncPQuery("UPDATE realmlist SET population = '%f' WHERE id = '%d'", popu, realm.Id.Realm);
    }
}

bool World::HasRecentlyDisconnected(WorldSession* session)
{
    if(!session) return false;

    if(uint32 tolerance = getConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
    {
        for(auto i = m_disconnects.begin(); i != m_disconnects.end(); ++i)
        {
            if(difftime(i->second, time(nullptr)) < tolerance)
            {
                if(i->first == session->GetAccountId())
                    return true;
            }
            else
                m_disconnects.erase(i);
        }
    }
    return false;
 }

int32 World::GetQueuePos(WorldSession* sess)
{
    uint32 position = 1;

    for(auto iter = m_QueuedPlayer.begin(); iter != m_QueuedPlayer.end(); ++iter, ++position)
        if((*iter) == sess)
            return position;

    return 0;
}

void World::AddQueuedPlayer(WorldSession* sess)
{
    sess->SetInQueue(true);
    m_QueuedPlayer.push_back (sess);

    // The 1st SMSG_AUTH_RESPONSE needs to contain other info too.
    sess->SendAuthResponse(AUTH_WAIT_QUEUE, false, GetQueuePos(sess));
}

bool World::RemoveQueuedPlayer(WorldSession* sess)
{
    // sessions count including queued to remove (if removed_session set)
    uint32 sessions = GetActiveSessionCount();

    uint32 position = 1;
    auto iter = m_QueuedPlayer.begin();

    // search to remove and count skipped positions
    bool found = false;

    for(;iter != m_QueuedPlayer.end(); ++iter, ++position)
    {
        if(*iter==sess)
        {
            sess->SetInQueue(false);
            sess->ResetTimeOutTime(false);
            iter = m_QueuedPlayer.erase(iter);
            found = true;                                   // removing queued session
            break;
        }
    }

    // iter point to next socked after removed or end()
    // position store position of removed socket and then new position next socket after removed

    // if session not queued then we need decrease sessions count
    if(!found && sessions)
        --sessions;

    // accept first in queue
    if ((!m_playerLimit || sessions < m_playerLimit) && !m_QueuedPlayer.empty())
    {
        WorldSession* pop_sess = m_QueuedPlayer.front();
        pop_sess->InitializeSession();
        m_QueuedPlayer.pop_front();

        // update iter to point first queued socket or end() if queue is empty now
        iter = m_QueuedPlayer.begin();
        position = 1;
    }

    // update position from iter to end()
    // iter point to first not updated socket, position store new position
    for(; iter != m_QueuedPlayer.end(); ++iter, ++position)
        (*iter)->SendAuthWaitQue(position);

    return found;
}

/// Find a Weather object by the given zoneid
Weather* World::FindWeather(uint32 id) const
{
    auto itr = m_weathers.find(id);

    if(itr != m_weathers.end())
        return itr->second;
    else
        return nullptr;
}

/// Remove a Weather object for the given zoneid
void World::RemoveWeather(uint32 id)
{
    // not called at the moment. Kept for completeness
    auto itr = m_weathers.find(id);

    if(itr != m_weathers.end())
    {
        delete itr->second;
        m_weathers.erase(itr);
    }
}

/// Add a Weather object to the list
Weather* World::AddWeather(uint32 zone_id)
{
    WeatherZoneChances const* weatherChances = sObjectMgr->GetWeatherChances(zone_id);

    // zone not have weather, ignore
    if(!weatherChances)
        return nullptr;

    auto  w = new Weather(zone_id,weatherChances);
    m_weathers[w->GetZone()] = w;
    w->ReGenerate();
    w->UpdateWeather();
    return w;
}

/// Initialize config values
void World::LoadConfigSettings(bool reload)
{
    if(reload)
    {
        std::string configError;
        if(!sConfigMgr->Reload(configError))
        {
            TC_LOG_ERROR("server.loading","World settings reload fail: can't read settings from %s. %s.",sConfigMgr->GetFilename().c_str(),configError.c_str());
            return;
        }
        sLog->LoadFromConfig();
    }

    ///- Read the player limit from the config file
    SetPlayerLimit(sConfigMgr->GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT));

    ///- Get string for new logins (newly created characters)
    SetNewCharString(sConfigMgr->GetStringDefault("PlayerStart.String", ""));

    ///- Send server info on login?
    m_configs[CONFIG_ENABLE_SINFO_LOGIN] = sConfigMgr->GetIntDefault("Server.LoginInfo", 0);

    ///- Read all rates from the config file
    rate_values[RATE_HEALTH]      = sConfigMgr->GetFloatDefault("Rate.Health", 1);
    if(rate_values[RATE_HEALTH] < 0)
    {
        TC_LOG_ERROR("server.loading","Rate.Health (%f) mustbe > 0. Using 1 instead.",rate_values[RATE_HEALTH]);
        rate_values[RATE_HEALTH] = 1;
    }
    rate_values[RATE_POWER_MANA]  = sConfigMgr->GetFloatDefault("Rate.Mana", 1);
    if(rate_values[RATE_POWER_MANA] < 0)
    {
        TC_LOG_ERROR("server.loading","Rate.Mana (%f) mustbe > 0. Using 1 instead.",rate_values[RATE_POWER_MANA]);
        rate_values[RATE_POWER_MANA] = 1;
    }
    rate_values[RATE_POWER_RAGE_INCOME] = sConfigMgr->GetFloatDefault("Rate.Rage.Income", 1);
    rate_values[RATE_POWER_RAGE_LOSS]   = sConfigMgr->GetFloatDefault("Rate.Rage.Loss", 1);
    if(rate_values[RATE_POWER_RAGE_LOSS] < 0)
    {
        TC_LOG_ERROR("server.loading","Rate.Rage.Loss (%f) mustbe > 0. Using 1 instead.",rate_values[RATE_POWER_RAGE_LOSS]);
        rate_values[RATE_POWER_RAGE_LOSS] = 1;
    }
    rate_values[RATE_POWER_FOCUS] = sConfigMgr->GetFloatDefault("Rate.Focus", 1.0f);
    rate_values[RATE_LOYALTY]     = sConfigMgr->GetFloatDefault("Rate.Loyalty", 1.0f);
    rate_values[RATE_SKILL_DISCOVERY] = sConfigMgr->GetFloatDefault("Rate.Skill.Discovery", 1.0f);
    rate_values[RATE_DROP_ITEM_POOR]       = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Poor", 1.0f);
    rate_values[RATE_DROP_ITEM_NORMAL]     = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Normal", 1.0f);
    rate_values[RATE_DROP_ITEM_UNCOMMON]   = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Uncommon", 1.0f);
    rate_values[RATE_DROP_ITEM_RARE]       = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Rare", 1.0f);
    rate_values[RATE_DROP_ITEM_EPIC]       = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Epic", 1.0f);
    rate_values[RATE_DROP_ITEM_LEGENDARY]  = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Legendary", 1.0f);
    rate_values[RATE_DROP_ITEM_ARTIFACT]   = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Artifact", 1.0f);
    rate_values[RATE_DROP_ITEM_REFERENCED] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Referenced", 1.0f);
    rate_values[RATE_DROP_MONEY]  = sConfigMgr->GetFloatDefault("Rate.Drop.Money", 1.0f);
    rate_values[RATE_XP_KILL]     = sConfigMgr->GetFloatDefault("Rate.XP.Kill", 1.0f);
    rate_values[RATE_XP_QUEST]    = sConfigMgr->GetFloatDefault("Rate.XP.Quest", 1.0f);
    rate_values[RATE_XP_EXPLORE]  = sConfigMgr->GetFloatDefault("Rate.XP.Explore", 1.0f);
    rate_values[RATE_XP_PAST_70]  = sConfigMgr->GetFloatDefault("Rate.XP.PastLevel70", 1.0f);
    rate_values[RATE_REPUTATION_GAIN]  = sConfigMgr->GetFloatDefault("Rate.Reputation.Gain", 1.0f);
    rate_values[RATE_CREATURE_AGGRO]  = sConfigMgr->GetFloatDefault("Rate.Creature.Aggro", 1.0f);
    rate_values[RATE_REST_INGAME]                    = sConfigMgr->GetFloatDefault("Rate.Rest.InGame", 1.0f);
    rate_values[RATE_REST_OFFLINE_IN_TAVERN_OR_CITY] = sConfigMgr->GetFloatDefault("Rate.Rest.Offline.InTavernOrCity", 1.0f);
    rate_values[RATE_REST_OFFLINE_IN_WILDERNESS]     = sConfigMgr->GetFloatDefault("Rate.Rest.Offline.InWilderness", 1.0f);
    rate_values[RATE_DAMAGE_FALL]  = sConfigMgr->GetFloatDefault("Rate.Damage.Fall", 1.0f);
    rate_values[RATE_AUCTION_TIME]  = sConfigMgr->GetFloatDefault("Rate.Auction.Time", 1.0f);
    rate_values[RATE_AUCTION_DEPOSIT] = sConfigMgr->GetFloatDefault("Rate.Auction.Deposit", 1.0f);
    rate_values[RATE_AUCTION_CUT] = sConfigMgr->GetFloatDefault("Rate.Auction.Cut", 1.0f);
    rate_values[RATE_HONOR] = sConfigMgr->GetFloatDefault("Rate.Honor",1.0f);
    rate_values[RATE_MINING_AMOUNT] = sConfigMgr->GetFloatDefault("Rate.Mining.Amount",1.0f);
    rate_values[RATE_MINING_NEXT]   = sConfigMgr->GetFloatDefault("Rate.Mining.Next",1.0f);
    rate_values[RATE_INSTANCE_RESET_TIME] = sConfigMgr->GetFloatDefault("Rate.InstanceResetTime",1.0f);
    rate_values[RATE_TALENT] = sConfigMgr->GetFloatDefault("Rate.Talent",1.0f);
    if(rate_values[RATE_TALENT] < 0.0f)
    {
        TC_LOG_ERROR("server.loading","Rate.Talent (%f) mustbe > 0. Using 1 instead.",rate_values[RATE_TALENT]);
        rate_values[RATE_TALENT] = 1.0f;
    }
    rate_values[RATE_CORPSE_DECAY_LOOTED] = sConfigMgr->GetFloatDefault("Rate.Corpse.Decay.Looted",0.5f);

    rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = sConfigMgr->GetFloatDefault("TargetPosRecalculateRange", 0.5f);
    if(rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] < CONTACT_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","TargetPosRecalculateRange (%f) must be >= %f. Using %f instead.",rate_values[RATE_TARGET_POS_RECALCULATION_RANGE],CONTACT_DISTANCE,CONTACT_DISTANCE);
        rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = CONTACT_DISTANCE;
    }
    else if(rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] > NOMINAL_MELEE_RANGE)
    {
        TC_LOG_ERROR("server.loading","TargetPosRecalculateRange (%f) must be <= %f. Using %f instead.",
            rate_values[RATE_TARGET_POS_RECALCULATION_RANGE],NOMINAL_MELEE_RANGE,NOMINAL_MELEE_RANGE);
        rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = NOMINAL_MELEE_RANGE;
    }

    rate_values[RATE_DURABILITY_LOSS_DAMAGE] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Damage",0.5f);
    if(rate_values[RATE_DURABILITY_LOSS_DAMAGE] < 0.0f)
    {
        TC_LOG_ERROR("server.loading","DurabilityLossChance.Damage (%f) must be >=0. Using 0.0 instead.",rate_values[RATE_DURABILITY_LOSS_DAMAGE]);
        rate_values[RATE_DURABILITY_LOSS_DAMAGE] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_ABSORB] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Absorb",0.5f);
    if(rate_values[RATE_DURABILITY_LOSS_ABSORB] < 0.0f)
    {
        TC_LOG_ERROR("server.loading","DurabilityLossChance.Absorb (%f) must be >=0. Using 0.0 instead.",rate_values[RATE_DURABILITY_LOSS_ABSORB]);
        rate_values[RATE_DURABILITY_LOSS_ABSORB] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_PARRY] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Parry",0.05f);
    if(rate_values[RATE_DURABILITY_LOSS_PARRY] < 0.0f)
    {
        TC_LOG_ERROR("server.loading","DurabilityLossChance.Parry (%f) must be >=0. Using 0.0 instead.",rate_values[RATE_DURABILITY_LOSS_PARRY]);
        rate_values[RATE_DURABILITY_LOSS_PARRY] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_BLOCK] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Block",0.05f);
    if(rate_values[RATE_DURABILITY_LOSS_BLOCK] < 0.0f)
    {
        TC_LOG_ERROR("server.loading","DurabilityLossChance.Block (%f) must be >=0. Using 0.0 instead.",rate_values[RATE_DURABILITY_LOSS_BLOCK]);
        rate_values[RATE_DURABILITY_LOSS_BLOCK] = 0.0f;
    }

    ///- Read other configuration items from the config file

    // movement anticheat
    m_MvAnticheatEnable                     = sConfigMgr->GetBoolDefault("Anticheat.Movement.Enable",true);
    m_MvAnticheatKick                       = sConfigMgr->GetBoolDefault("Anticheat.Movement.Kick",false);
    m_MvAnticheatAlarmCount                 = (uint32)sConfigMgr->GetIntDefault("Anticheat.Movement.AlarmCount", 5);
    m_MvAnticheatAlarmPeriod                = (uint32)sConfigMgr->GetIntDefault("Anticheat.Movement.AlarmTime", 5000);
    m_MvAntiCheatBan                        = (unsigned char)sConfigMgr->GetIntDefault("Anticheat.Movement.BanType",0);
    m_MvAnticheatBanTime                    = sConfigMgr->GetStringDefault("Anticheat.Movement.BanTime","3m");
    m_MvAnticheatGmLevel                    = (unsigned char)sConfigMgr->GetIntDefault("Anticheat.Movement.GmLevel",0);
    m_MvAnticheatKill                       = sConfigMgr->GetBoolDefault("Anticheat.Movement.Kill",false);
    m_MvAnticheatWarn                       = sConfigMgr->GetBoolDefault("Anticheat.Movement.Warn",true);

    m_wardenBanTime                         = sConfigMgr->GetStringDefault("Warden.BanTime","180d");

    m_configs[CONFIG_COMPRESSION] = sConfigMgr->GetIntDefault("Compression", 1);
    if(m_configs[CONFIG_COMPRESSION] < 1 || m_configs[CONFIG_COMPRESSION] > 9)
    {
        TC_LOG_ERROR("server.loading","Compression level (%i) must be in range 1..9. Using default compression level (1).",m_configs[CONFIG_COMPRESSION]);
        m_configs[CONFIG_COMPRESSION] = 1;
    }
    m_configs[CONFIG_ADDON_CHANNEL] = sConfigMgr->GetBoolDefault("AddonChannel", true);
    m_configs[CONFIG_GRID_UNLOAD] = sConfigMgr->GetBoolDefault("GridUnload", true);
    m_configs[CONFIG_INTERVAL_SAVE] = sConfigMgr->GetIntDefault("PlayerSaveInterval", 60000);
    m_configs[CONFIG_INTERVAL_DISCONNECT_TOLERANCE] = sConfigMgr->GetIntDefault("DisconnectToleranceInterval", 0);

    m_configs[CONFIG_INTERVAL_MAPUPDATE] = sConfigMgr->GetIntDefault("MapUpdateInterval", 100);
    if(m_configs[CONFIG_INTERVAL_MAPUPDATE] < MIN_MAP_UPDATE_DELAY)
    {
        TC_LOG_ERROR("server.loading","MapUpdateInterval (%i) must be greater %u. Use this minimal value.",m_configs[CONFIG_INTERVAL_MAPUPDATE],MIN_MAP_UPDATE_DELAY);
        m_configs[CONFIG_INTERVAL_MAPUPDATE] = MIN_MAP_UPDATE_DELAY;
    }
    if(reload)
        sMapMgr->SetMapUpdateInterval(m_configs[CONFIG_INTERVAL_MAPUPDATE]);

    m_configs[CONFIG_INTERVAL_CHANGEWEATHER] = sConfigMgr->GetIntDefault("ChangeWeatherInterval", 600000);

    if(reload)
    {
        int32 val = sConfigMgr->GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT);
        if(val!=m_configs[CONFIG_PORT_WORLD])
            TC_LOG_ERROR("server.loading","WorldServerPort option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[CONFIG_PORT_WORLD]);
    }
    else
        m_configs[CONFIG_PORT_WORLD] = sConfigMgr->GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT);

    m_configs[CONFIG_SOCKET_TIMEOUTTIME] = sConfigMgr->GetIntDefault("SocketTimeOutTime", 900000) / 1000;
    m_configs[CONFIG_SOCKET_TIMEOUTTIME_ACTIVE] = sConfigMgr->GetIntDefault("SocketTimeOutTimeActive", 60000) / 1000;

    m_configs[CONFIG_GROUP_XP_DISTANCE] = sConfigMgr->GetIntDefault("MaxGroupXPDistance", 74);
    /// \todo Add MonsterSight and GuarderSight (with meaning) in worldserver.conf or put them as define
    m_configs[CONFIG_SIGHT_MONSTER] = sConfigMgr->GetIntDefault("MonsterSight", 50);

    if(reload)
    {
        int32 val = sConfigMgr->GetIntDefault("GameType", 0);
        if(val!=m_configs[CONFIG_GAME_TYPE])
            TC_LOG_ERROR("server.loading","GameType option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[CONFIG_GAME_TYPE]);
    }
    else
        m_configs[CONFIG_GAME_TYPE] = sConfigMgr->GetIntDefault("GameType", 0);

    if(reload)
    {
        int32 val = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);
        if(val!=m_configs[CONFIG_REALM_ZONE])
            TC_LOG_ERROR("server.loading","RealmZone option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[CONFIG_REALM_ZONE]);
    }
    else
        m_configs[CONFIG_REALM_ZONE] = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);

    m_configs[CONFIG_ALLOW_TWO_SIDE_ACCOUNTS] = sConfigMgr->GetBoolDefault("AllowTwoSide.Accounts", false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT]    = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Chat",false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL] = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Channel",false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP]   = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Group",false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD]   = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Guild",false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION] = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Auction",false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL]    = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Mail",false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_WHO_LIST] = sConfigMgr->GetBoolDefault("AllowTwoSide.WhoList", false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND] = sConfigMgr->GetBoolDefault("AllowTwoSide.AddFriend", false);
    m_configs[CONFIG_ALLOW_TWO_SIDE_TRADE] = sConfigMgr->GetBoolDefault("AllowTwoSide.Trade", false);
    m_configs[CONFIG_STRICT_PLAYER_NAMES]  = sConfigMgr->GetIntDefault("StrictPlayerNames",  1);
    m_configs[CONFIG_STRICT_CHARTER_NAMES] = sConfigMgr->GetIntDefault("StrictCharterNames", 1);
    m_configs[CONFIG_STRICT_PET_NAMES]     = sConfigMgr->GetIntDefault("StrictPetNames",     1);

    m_configs[CONFIG_CHARACTERS_CREATING_DISABLED] = sConfigMgr->GetIntDefault("CharactersCreatingDisabled", 0);

    m_configs[CONFIG_CHARACTERS_PER_REALM] = sConfigMgr->GetIntDefault("CharactersPerRealm", 10);
    if(m_configs[CONFIG_CHARACTERS_PER_REALM] < 1 || m_configs[CONFIG_CHARACTERS_PER_REALM] > 10)
    {
        TC_LOG_ERROR("server.loading","CharactersPerRealm (%i) must be in range 1..10. Set to 10.",m_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_configs[CONFIG_CHARACTERS_PER_REALM] = 10;
    }

    // must be after CONFIG_CHARACTERS_PER_REALM
    m_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = sConfigMgr->GetIntDefault("CharactersPerAccount", 50);
    if(m_configs[CONFIG_CHARACTERS_PER_ACCOUNT] < m_configs[CONFIG_CHARACTERS_PER_REALM])
    {
        TC_LOG_ERROR("server.loading","CharactersPerAccount (%i) can't be less than CharactersPerRealm (%i).",m_configs[CONFIG_CHARACTERS_PER_ACCOUNT],m_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = m_configs[CONFIG_CHARACTERS_PER_REALM];
    }

    m_configs[CONFIG_SKIP_CINEMATICS] = sConfigMgr->GetIntDefault("SkipCinematics", 0);
    if(m_configs[CONFIG_SKIP_CINEMATICS] < 0 || m_configs[CONFIG_SKIP_CINEMATICS] > 2)
    {
        TC_LOG_ERROR("server.loading","SkipCinematics (%i) must be in range 0..2. Set to 0.",m_configs[CONFIG_SKIP_CINEMATICS]);
        m_configs[CONFIG_SKIP_CINEMATICS] = 0;
    }

    if(reload)
    {
        int32 val = sConfigMgr->GetIntDefault("MaxPlayerLevel", 70);
        if(val!=m_configs[CONFIG_MAX_PLAYER_LEVEL])
            TC_LOG_ERROR("server.loading","MaxPlayerLevel option can't be changed at config reload, using current value (%u).",m_configs[CONFIG_MAX_PLAYER_LEVEL]);
    }
    else
        m_configs[CONFIG_MAX_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("MaxPlayerLevel", 70);

    if(m_configs[CONFIG_MAX_PLAYER_LEVEL] > MAX_LEVEL)
    {
        TC_LOG_ERROR("server.loading","MaxPlayerLevel (%i) must be in range 1..%u. Set to %u.",m_configs[CONFIG_MAX_PLAYER_LEVEL],MAX_LEVEL,MAX_LEVEL);
        m_configs[CONFIG_MAX_PLAYER_LEVEL] = MAX_LEVEL;
    }

    m_configs[CONFIG_START_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("StartPlayerLevel", 1);
    if(m_configs[CONFIG_START_PLAYER_LEVEL] < 1)
    {
        TC_LOG_ERROR("server.loading","StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to 1.",m_configs[CONFIG_START_PLAYER_LEVEL],m_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_configs[CONFIG_START_PLAYER_LEVEL] = 1;
    }
    else if(m_configs[CONFIG_START_PLAYER_LEVEL] > m_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        TC_LOG_ERROR("server.loading","StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to %u.",m_configs[CONFIG_START_PLAYER_LEVEL],m_configs[CONFIG_MAX_PLAYER_LEVEL],m_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_configs[CONFIG_START_PLAYER_LEVEL] = m_configs[CONFIG_MAX_PLAYER_LEVEL];
    }

    m_configs[CONFIG_START_PLAYER_MONEY] = sConfigMgr->GetIntDefault("StartPlayerMoney", 0);
    if(m_configs[CONFIG_START_PLAYER_MONEY] < 0)
    {
        TC_LOG_ERROR("server.loading","StartPlayerMoney (%i) must be in range 0..%u. Set to %u.",m_configs[CONFIG_START_PLAYER_MONEY],MAX_MONEY_AMOUNT,0);
        m_configs[CONFIG_START_PLAYER_MONEY] = 0;
    }
    else if(m_configs[CONFIG_START_PLAYER_MONEY] > MAX_MONEY_AMOUNT)
    {
        TC_LOG_ERROR("server.loading","StartPlayerMoney (%i) must be in range 0..%u. Set to %u.",
            m_configs[CONFIG_START_PLAYER_MONEY],MAX_MONEY_AMOUNT,MAX_MONEY_AMOUNT);
        m_configs[CONFIG_START_PLAYER_MONEY] = MAX_MONEY_AMOUNT;
    }

    m_configs[CONFIG_MAX_HONOR_POINTS] = sConfigMgr->GetIntDefault("MaxHonorPoints", 75000);
    if(m_configs[CONFIG_MAX_HONOR_POINTS] < 0)
    {
        TC_LOG_ERROR("server.loading","MaxHonorPoints (%i) can't be negative. Set to 0.",m_configs[CONFIG_MAX_HONOR_POINTS]);
        m_configs[CONFIG_MAX_HONOR_POINTS] = 0;
    }

    m_configs[CONFIG_START_HONOR_POINTS] = sConfigMgr->GetIntDefault("StartHonorPoints", 0);
    if(m_configs[CONFIG_START_HONOR_POINTS] < 0)
    {
        TC_LOG_ERROR("server.loading","StartHonorPoints (%i) must be in range 0..MaxHonorPoints(%u). Set to %u.",
            m_configs[CONFIG_START_HONOR_POINTS],m_configs[CONFIG_MAX_HONOR_POINTS],0);
        m_configs[CONFIG_MAX_HONOR_POINTS] = 0;
    }
    else if(m_configs[CONFIG_START_HONOR_POINTS] > m_configs[CONFIG_MAX_HONOR_POINTS])
    {
        TC_LOG_ERROR("server.loading","StartHonorPoints (%i) must be in range 0..MaxHonorPoints(%u). Set to %u.",
            m_configs[CONFIG_START_HONOR_POINTS],m_configs[CONFIG_MAX_HONOR_POINTS],m_configs[CONFIG_MAX_HONOR_POINTS]);
        m_configs[CONFIG_START_HONOR_POINTS] = m_configs[CONFIG_MAX_HONOR_POINTS];
    }

    std::string s_pvp_ranks = sConfigMgr->GetStringDefault("PvPRank.HKPerRank", "1000,1500,2500,3500,5000,6500,9000,13000,18000,25000,35000,50000,75000,100000");
    char *c_pvp_ranks = const_cast<char*>(s_pvp_ranks.c_str());
    for (int i = 0; i !=HKRANKMAX; i++)
    {
        if(i==0)
            pvp_ranks[0] = 0;
        else if(i==1)
            pvp_ranks[1] = atoi(strtok (c_pvp_ranks, ","));
        else
            pvp_ranks[i] = atoi(strtok (nullptr, ","));
    }

    std::string s_leaderTeams = sConfigMgr->GetStringDefault("Arena.NewTitleDistribution.StaticLeaders", "0,0,0,0,0,0,0,0,0,0,0,0");
    Tokens leadertokens = StrSplit(s_leaderTeams, ",");
    for (int i = 0; i < leadertokens.size(); i++)
        confStaticLeaders[i] = atoi(leadertokens[i].c_str());
    for (int i = leadertokens.size(); i < 12; i++)
        confStaticLeaders[i] = 0;

    confGladiators.clear();
    std::string s_Gladiators = sConfigMgr->GetStringDefault("Arena.NewTitleDistribution.Gladiators", ""); //format : "<playerguid> <rank [1-3]>, <playerguid2> <rank>,..."
    Tokens gladtokens = StrSplit(s_Gladiators, ",");
    for (auto & gladtoken : gladtokens)
    {
        Tokens subTokens = StrSplit(gladtoken, " ");
        if(subTokens.size() != 2)
        {
            TC_LOG_ERROR("server.loading","ERROR in config file in Arena.NewTitleDistribution.Gladiators, skipped this entry.");
            continue;
        }
        uint32 playerguid = atoi(subTokens[0].c_str());
        if(playerguid == 0)
        {
            TC_LOG_ERROR("server.loading","ERROR in config file in Arena.NewTitleDistribution.Gladiators, skipped this entry.");
            continue;
        }
        uint32 rank = atoi(subTokens[1].c_str());
        Gladiator glad(playerguid, rank);
        confGladiators.push_back(glad);
    }

    m_configs[CONFIG_MAX_ARENA_POINTS] = sConfigMgr->GetIntDefault("MaxArenaPoints", 5000);
    if(m_configs[CONFIG_MAX_ARENA_POINTS] < 0)
    {
        TC_LOG_ERROR("server.loading","MaxArenaPoints (%i) can't be negative. Set to 0.",m_configs[CONFIG_MAX_ARENA_POINTS]);
        m_configs[CONFIG_MAX_ARENA_POINTS] = 0;
    }

    m_configs[CONFIG_START_ARENA_POINTS] = sConfigMgr->GetIntDefault("StartArenaPoints", 0);
    if(m_configs[CONFIG_START_ARENA_POINTS] < 0)
    {
        TC_LOG_ERROR("server.loading","StartArenaPoints (%i) must be in range 0..MaxArenaPoints(%u). Set to %u.",
            m_configs[CONFIG_START_ARENA_POINTS],m_configs[CONFIG_MAX_ARENA_POINTS],0);
        m_configs[CONFIG_START_ARENA_POINTS] = 0;
    }
    else if(m_configs[CONFIG_START_ARENA_POINTS] > m_configs[CONFIG_MAX_ARENA_POINTS])
    {
        TC_LOG_ERROR("server.loading","StartArenaPoints (%i) must be in range 0..MaxArenaPoints(%u). Set to %u.",
            m_configs[CONFIG_START_ARENA_POINTS],m_configs[CONFIG_MAX_ARENA_POINTS],m_configs[CONFIG_MAX_ARENA_POINTS]);
        m_configs[CONFIG_START_ARENA_POINTS] = m_configs[CONFIG_MAX_ARENA_POINTS];
    }

    m_configs[CONFIG_ALL_TAXI_PATHS] = sConfigMgr->GetBoolDefault("AllFlightPaths", false);
    m_configs[CONFIG_INSTANT_TAXI]   = sConfigMgr->GetBoolDefault("InstantFlightPaths", false);

    m_configs[CONFIG_INSTANCE_IGNORE_LEVEL] = sConfigMgr->GetBoolDefault("Instance.IgnoreLevel", false);
    m_configs[CONFIG_INSTANCE_IGNORE_RAID]  = sConfigMgr->GetBoolDefault("Instance.IgnoreRaid", false);

    m_configs[CONFIG_BATTLEGROUND_CAST_DESERTER]              = sConfigMgr->GetBoolDefault("Battleground.CastDeserter", true);
    m_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE]     = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.Enable", true);
    m_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY] = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.PlayerOnly", false);
    m_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_WORLDONLY]  = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.WorldOnly", true);
    m_configs[CONFIG_BATTLEGROUND_ARENA_RATED_ENABLE]         = sConfigMgr->GetBoolDefault("Battleground.Arena.Rated.Enable", true);
    m_configs[CONFIG_BATTLEGROUND_ARENA_CLOSE_AT_NIGHT_MASK]  = sConfigMgr->GetIntDefault("Battleground.Arena.NightClose.Mask", 1);
    m_configs[CONFIG_BATTLEGROUND_ARENA_ALTERNATE_RATING]     = sConfigMgr->GetBoolDefault("Battleground.Arena.Alternate.Rating", false);
    m_configs[CONFIG_BATTLEGROUND_ARENA_ANNOUNCE]             = sConfigMgr->GetBoolDefault("Battleground.Arena.Announce", true);

    m_configs[CONFIG_CHARDELETE_KEEP_DAYS] = sConfigMgr->GetIntDefault("CharDelete.KeepDays", 30);

    m_configs[CONFIG_CAST_UNSTUCK] = sConfigMgr->GetBoolDefault("CastUnstuck", true);
    m_configs[CONFIG_INSTANCE_RESET_TIME_HOUR]  = sConfigMgr->GetIntDefault("Instance.ResetTimeHour", 9); //9AM on retail in 2008
    m_configs[CONFIG_INSTANCE_UNLOAD_DELAY] = sConfigMgr->GetIntDefault("Instance.UnloadDelay", 1800000);

    m_configs[CONFIG_MAX_PRIMARY_TRADE_SKILL] = sConfigMgr->GetIntDefault("MaxPrimaryTradeSkill", 2);
    m_configs[CONFIG_MIN_PETITION_SIGNS] = sConfigMgr->GetIntDefault("MinPetitionSigns", 9);
    if(m_configs[CONFIG_MIN_PETITION_SIGNS] > 9)
    {
        TC_LOG_ERROR("server.loading","MinPetitionSigns (%i) must be in range 0..9. Set to 9.",m_configs[CONFIG_MIN_PETITION_SIGNS]);
        m_configs[CONFIG_MIN_PETITION_SIGNS] = 9;
    }

    m_configs[CONFIG_GM_LOGIN_STATE]       = sConfigMgr->GetIntDefault("GM.LoginState",2);
    m_configs[CONFIG_GM_VISIBLE_STATE]     = sConfigMgr->GetIntDefault("GM.Visible", 2);
    m_configs[CONFIG_GM_CHAT]              = sConfigMgr->GetIntDefault("GM.Chat",2);
    m_configs[CONFIG_GM_WISPERING_TO]      = sConfigMgr->GetIntDefault("GM.WhisperingTo",2);
    m_configs[CONFIG_GM_LEVEL_IN_GM_LIST]  = sConfigMgr->GetIntDefault("GM.InGMList.Level", SEC_GAMEMASTER3);
    m_configs[CONFIG_GM_LEVEL_IN_WHO_LIST] = sConfigMgr->GetIntDefault("GM.InWhoList.Level", SEC_GAMEMASTER3);
    m_configs[CONFIG_START_GM_LEVEL]       = sConfigMgr->GetIntDefault("GM.StartLevel", 1);
    m_configs[CONFIG_GM_DEFAULT_GUILD]     = sConfigMgr->GetIntDefault("GM.DefaultGuild", 0);
    m_configs[CONFIG_GM_FORCE_GUILD]       = sConfigMgr->GetIntDefault("GM.ForceGuild", 0);
    m_configs[CONFIG_ALLOW_GM_GROUP]       = sConfigMgr->GetBoolDefault("GM.AllowInvite", false);
    m_configs[CONFIG_ALLOW_GM_FRIEND]      = sConfigMgr->GetBoolDefault("GM.AllowFriend", false);
    if(m_configs[CONFIG_START_GM_LEVEL] < m_configs[CONFIG_START_PLAYER_LEVEL])
    {
        TC_LOG_ERROR("server.loading","GM.StartLevel (%i) must be in range StartPlayerLevel(%u)..%u. Set to %u.",
            m_configs[CONFIG_START_GM_LEVEL],m_configs[CONFIG_START_PLAYER_LEVEL], MAX_LEVEL, m_configs[CONFIG_START_PLAYER_LEVEL]);
        m_configs[CONFIG_START_GM_LEVEL] = m_configs[CONFIG_START_PLAYER_LEVEL];
    }
    else if(m_configs[CONFIG_START_GM_LEVEL] > MAX_LEVEL)
    {
        TC_LOG_ERROR("server.loading","GM.StartLevel (%i) must be in range 1..%u. Set to %u.", m_configs[CONFIG_START_GM_LEVEL], MAX_LEVEL, MAX_LEVEL);
        m_configs[CONFIG_START_GM_LEVEL] = MAX_LEVEL;
    }
    if (m_configs[CONFIG_GM_DEFAULT_GUILD] && m_configs[CONFIG_GM_FORCE_GUILD] && (m_configs[CONFIG_GM_DEFAULT_GUILD] != m_configs[CONFIG_GM_FORCE_GUILD]))
    {
        TC_LOG_ERROR("server.loading", "GM.ForcedGuild conflicts with GM.DefaultGuild. Keeping only GM.ForcedGuild.");
        m_configs[CONFIG_GM_DEFAULT_GUILD] = 0;
    }

    m_configs[CONFIG_GROUP_VISIBILITY] = sConfigMgr->GetIntDefault("Visibility.GroupMode", 1);

    m_configs[CONFIG_LOG_BG_STATS] = sConfigMgr->GetIntDefault("DBLog.bgstats", -1);
    m_configs[CONFIG_LOG_BOSS_DOWNS] = sConfigMgr->GetIntDefault("DBLog.BossDowns", -1);
    m_configs[CONFIG_LOG_CHAR_DELETE] = sConfigMgr->GetIntDefault("DBLog.char.Delete", -1);
    m_configs[CONFIG_GM_LOG_CHAR_DELETE] = sConfigMgr->GetIntDefault("DBLog.gm.char.Delete", -1);
    m_configs[CONFIG_LOG_CHAR_CHAT] = sConfigMgr->GetIntDefault("DBLog.char.Chat", 30);
    m_configs[CONFIG_GM_LOG_CHAR_CHAT] = sConfigMgr->GetIntDefault("DBLog.gm.char.Chat", -1);
    m_configs[CONFIG_LOG_CHAR_GUILD_MONEY] = sConfigMgr->GetIntDefault("DBLog.char.GuildMoney", 30);
    m_configs[CONFIG_GM_LOG_CHAR_GUILD_MONEY] = sConfigMgr->GetIntDefault("DBLog.gm.char.GuildMoney", -1);
    m_configs[CONFIG_LOG_CHAR_ITEM_DELETE] = sConfigMgr->GetIntDefault("DBLog.char.item.Delete", 30);
    m_configs[CONFIG_GM_LOG_CHAR_ITEM_DELETE] = sConfigMgr->GetIntDefault("DBLog.gm.char.item.Delete", -1);
    m_configs[CONFIG_LOG_CHAR_ITEM_GUILD_BANK] = sConfigMgr->GetIntDefault("DBLog.char.item.GuildBank", 30);
    m_configs[CONFIG_GM_LOG_CHAR_ITEM_GUILD_BANK] = sConfigMgr->GetIntDefault("DBLog.gm.char.item.GuildBank", -1);
    m_configs[CONFIG_LOG_CHAR_ITEM_VENDOR] = sConfigMgr->GetIntDefault("DBLog.char.item.vendor", 30);
    m_configs[CONFIG_GM_LOG_CHAR_ITEM_VENDOR] = sConfigMgr->GetIntDefault("DBLog.gm.char.item.vendor", -1);
    m_configs[CONFIG_LOG_CHAR_ITEM_AUCTION] = sConfigMgr->GetIntDefault("DBLog.char.item.auction", 30);
    m_configs[CONFIG_GM_LOG_CHAR_ITEM_AUCTION] = sConfigMgr->GetIntDefault("DBLog.gm.char.item.auction", -1);
    m_configs[CONFIG_LOG_CHAR_ITEM_TRADE] = sConfigMgr->GetIntDefault("DBLog.char.item.trade", 30);
    m_configs[CONFIG_GM_LOG_CHAR_ITEM_TRADE] = sConfigMgr->GetIntDefault("DBLog.gm.char.item.trade", -1);
    m_configs[CONFIG_LOG_CHAR_ITEM_ENCHANT] = sConfigMgr->GetIntDefault("DBLog.char.item.enchant", 30);
    m_configs[CONFIG_GM_LOG_CHAR_ITEM_ENCHANT] = sConfigMgr->GetIntDefault("DBLog.gm.char.item.enchant", -1);
    m_configs[CONFIG_LOG_CHAR_MAIL] = sConfigMgr->GetIntDefault("DBLog.char.mail", 30);
    m_configs[CONFIG_GM_LOG_CHAR_MAIL] = sConfigMgr->GetIntDefault("DBLog.gm.char.mail", -1);
    m_configs[CONFIG_LOG_CHAR_RENAME] = sConfigMgr->GetIntDefault("DBLog.char.rename", 30);
    m_configs[CONFIG_GM_LOG_CHAR_RENAME] = sConfigMgr->GetIntDefault("DBLog.gm.char.rename", -1);
    m_configs[CONFIG_LOG_GM_COMMANDS] = sConfigMgr->GetIntDefault("DBLog.GMCommands", -1);
    m_configs[CONFIG_LOG_SANCTIONS] = sConfigMgr->GetIntDefault("DBLog.sanctions", -1);
    m_configs[CONFIG_LOG_CONNECTION_IP] = sConfigMgr->GetIntDefault("DBLog.connectionip",-1);
    m_configs[CONFIG_GM_LOG_CONNECTION_IP] = sConfigMgr->GetIntDefault("DBLog.gm.connectionip", -1);

    m_configs[CONFIG_MAIL_DELIVERY_DELAY] = sConfigMgr->GetIntDefault("MailDeliveryDelay",HOUR);

    m_configs[CONFIG_UPTIME_UPDATE] = sConfigMgr->GetIntDefault("UpdateUptimeInterval", 10);
    if(m_configs[CONFIG_UPTIME_UPDATE]<=0)
    {
        TC_LOG_ERROR("server.loading","UpdateUptimeInterval (%i) must be > 0, set to default 10.",m_configs[CONFIG_UPTIME_UPDATE]);
        m_configs[CONFIG_UPTIME_UPDATE] = 10;
    }
    if(reload)
    {
        m_timers[WUPDATE_UPTIME].SetInterval(m_configs[CONFIG_UPTIME_UPDATE]*MINUTE*1000);
        m_timers[WUPDATE_UPTIME].Reset();
    }

    m_configs[CONFIG_SKILL_CHANCE_ORANGE] = sConfigMgr->GetIntDefault("SkillChance.Orange",100);
    m_configs[CONFIG_SKILL_CHANCE_YELLOW] = sConfigMgr->GetIntDefault("SkillChance.Yellow",75);
    m_configs[CONFIG_SKILL_CHANCE_GREEN]  = sConfigMgr->GetIntDefault("SkillChance.Green",25);
    m_configs[CONFIG_SKILL_CHANCE_GREY]   = sConfigMgr->GetIntDefault("SkillChance.Grey",0);

    m_configs[CONFIG_SKILL_CHANCE_MINING_STEPS]  = sConfigMgr->GetIntDefault("SkillChance.MiningSteps",75);
    m_configs[CONFIG_SKILL_CHANCE_SKINNING_STEPS]   = sConfigMgr->GetIntDefault("SkillChance.SkinningSteps",75);

    m_configs[CONFIG_SKILL_PROSPECTING] = sConfigMgr->GetBoolDefault("SkillChance.Prospecting",false);

    m_configs[CONFIG_SKILL_GAIN_CRAFTING]  = sConfigMgr->GetIntDefault("SkillGain.Crafting", 1);
    if(m_configs[CONFIG_SKILL_GAIN_CRAFTING] < 0)
    {
        TC_LOG_ERROR("server.loading","SkillGain.Crafting (%i) can't be negative. Set to 1.",m_configs[CONFIG_SKILL_GAIN_CRAFTING]);
        m_configs[CONFIG_SKILL_GAIN_CRAFTING] = 1;
    }

    m_configs[CONFIG_SKILL_GAIN_DEFENSE]  = sConfigMgr->GetIntDefault("SkillGain.Defense", 1);
    if(m_configs[CONFIG_SKILL_GAIN_DEFENSE] < 0)
    {
        TC_LOG_ERROR("server.loading","SkillGain.Defense (%i) can't be negative. Set to 1.",m_configs[CONFIG_SKILL_GAIN_DEFENSE]);
        m_configs[CONFIG_SKILL_GAIN_DEFENSE] = 1;
    }

    m_configs[CONFIG_SKILL_GAIN_GATHERING]  = sConfigMgr->GetIntDefault("SkillGain.Gathering", 1);
    if(m_configs[CONFIG_SKILL_GAIN_GATHERING] < 0)
    {
        TC_LOG_ERROR("server.loading","SkillGain.Gathering (%i) can't be negative. Set to 1.",m_configs[CONFIG_SKILL_GAIN_GATHERING]);
        m_configs[CONFIG_SKILL_GAIN_GATHERING] = 1;
    }

    m_configs[CONFIG_SKILL_GAIN_WEAPON]  = sConfigMgr->GetIntDefault("SkillGain.Weapon", 1);
    if(m_configs[CONFIG_SKILL_GAIN_WEAPON] < 0)
    {
        TC_LOG_ERROR("server.loading","SkillGain.Weapon (%i) can't be negative. Set to 1.",m_configs[CONFIG_SKILL_GAIN_WEAPON]);
        m_configs[CONFIG_SKILL_GAIN_WEAPON] = 1;
    }

    m_configs[CONFIG_MAX_OVERSPEED_PINGS] = sConfigMgr->GetIntDefault("MaxOverspeedPings",2);
    if(m_configs[CONFIG_MAX_OVERSPEED_PINGS] != 0 && m_configs[CONFIG_MAX_OVERSPEED_PINGS] < 2)
    {
        TC_LOG_ERROR("server.loading","MaxOverspeedPings (%i) must be in range 2..infinity (or 0 to disable check. Set to 2.",m_configs[CONFIG_MAX_OVERSPEED_PINGS]);
        m_configs[CONFIG_MAX_OVERSPEED_PINGS] = 2;
    }

    m_configs[CONFIG_WEATHER] = sConfigMgr->GetBoolDefault("ActivateWeather",true);

    m_configs[CONFIG_ALWAYS_MAX_SKILL_FOR_LEVEL] = sConfigMgr->GetBoolDefault("AlwaysMaxSkillForLevel", false);

    m_configs[CONFIG_TRANSPORT_DOCKING_SOUNDS] = sConfigMgr->GetBoolDefault("Transports.DockingSounds", false);

    if(reload)
    {
        int32 val = sConfigMgr->GetIntDefault("Expansion", 1);
        if(val!=m_configs[CONFIG_EXPANSION])
            TC_LOG_ERROR("server.loading","Expansion option can't be changed at worldserver.conf reload, using current value (%u).", m_configs[CONFIG_EXPANSION]);
    }
    else
        m_configs[CONFIG_EXPANSION] = sConfigMgr->GetIntDefault("Expansion",1);

    m_configs[CONFIG_CHATFLOOD_MESSAGE_COUNT] = sConfigMgr->GetIntDefault("ChatFlood.MessageCount", 10);
    m_configs[CONFIG_CHATFLOOD_MESSAGE_DELAY] = sConfigMgr->GetIntDefault("ChatFlood.MessageDelay", 1);
    m_configs[CONFIG_CHATFLOOD_MUTE_TIME]     = sConfigMgr->GetIntDefault("ChatFlood.MuteTime", 10);

    m_configs[CONFIG_EVENT_ANNOUNCE] = sConfigMgr->GetIntDefault("Event.Announce", 0);

    m_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS] = sConfigMgr->GetIntDefault("CreatureFamilyAssistanceRadius", 10);
    m_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY]  = sConfigMgr->GetIntDefault("CreatureFamilyAssistanceDelay", 1500);
    m_configs[CONFIG_CREATURE_FAMILY_FLEE_DELAY] = sConfigMgr->GetIntDefault("CreatureFamilyFleeDelay", 7000);
    m_configs[CONFIG_CREATURE_UNREACHABLE_TARGET_EVADE_TIME] = sConfigMgr->GetIntDefault("CreatureUnreachableTarget.EvadeHomeTimer", 10000);
    m_configs[CONFIG_CREATURE_UNREACHABLE_TARGET_EVADE_ATTACKS_TIME] = sConfigMgr->GetIntDefault("CreatureUnreachableTarget.EvadeAttacksTimer", 3000);
    m_configs[CONFIG_CREATURE_STOP_FOR_PLAYER] = sConfigMgr->GetIntDefault("Creature.MovingStopTimeForPlayer", 1 * MINUTE * IN_MILLISECONDS);
    // note: disable value (-1) will assigned as 0xFFFFFFF, to prevent overflow at calculations limit it to max possible player level MAX_LEVEL(100)
    m_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = sConfigMgr->GetIntDefault("Quests.LowLevelHideDiff", 4);
    if(m_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = MAX_LEVEL;
    m_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = sConfigMgr->GetIntDefault("Quests.HighLevelHideDiff", 7);
    if(m_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = MAX_LEVEL;

    m_configs[CONFIG_RESTRICTED_LFG_CHANNEL] = sConfigMgr->GetBoolDefault("Channel.RestrictedLfg", true);
    m_configs[CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL] = sConfigMgr->GetBoolDefault("Channel.SilentlyGMJoin", false);

    m_configs[CONFIG_TALENTS_INSPECTING] = sConfigMgr->GetBoolDefault("TalentsInspecting", true);
    m_configs[CONFIG_CHAT_FAKE_MESSAGE_PREVENTING] = sConfigMgr->GetBoolDefault("ChatFakeMessagePreventing", true);

    // Respawn Settings
    m_configs[CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY] = sConfigMgr->GetBoolDefault("SaveRespawnTimeImmediately", true);
    if (!m_configs[CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY])
    {
        TC_LOG_WARN("server.loading", "SaveRespawnTimeImmediately triggers assertions when disabled, overridden to Enabled");
        m_configs[CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY] = true;
    }
    m_configs[CONFIG_RESPAWN_MINCHECKINTERVALMS] = sConfigMgr->GetIntDefault("Respawn.MinCheckIntervalMS", 5000);
    m_configs[CONFIG_RESPAWN_DYNAMIC_ESCORTNPC] = sConfigMgr->GetBoolDefault("Respawn.DynamicEscortNPC", true);
    m_configs[CONFIG_RESPAWN_DYNAMICMODE] = sConfigMgr->GetIntDefault("Respawn.DynamicMode", 1);
    if (m_configs[CONFIG_RESPAWN_DYNAMICMODE] > 1)
    {
        TC_LOG_ERROR("server.loading", "Invalid value for Respawn.DynamicMode (%u). Set to 1.", m_configs[CONFIG_RESPAWN_DYNAMICMODE]);
        m_configs[CONFIG_RESPAWN_DYNAMICMODE] = 1;
    }
    m_configs[CONFIG_RESPAWN_GUIDWARNLEVEL] = sConfigMgr->GetIntDefault("Respawn.GuidWarnLevel", 12000000);
    if (m_configs[CONFIG_RESPAWN_GUIDWARNLEVEL] > 16777215)
    {
        TC_LOG_ERROR("server.loading", "Respawn.GuidWarnLevel (%u) cannot be greater than maximum GUID (16777215). Set to 12000000.", m_configs[CONFIG_RESPAWN_GUIDWARNLEVEL]);
        m_configs[CONFIG_RESPAWN_GUIDWARNLEVEL] = 12000000;
    }
    m_configs[CONFIG_RESPAWN_GUIDALERTLEVEL] = sConfigMgr->GetIntDefault("Respawn.GuidAlertLevel", 16000000);
    if (m_configs[CONFIG_RESPAWN_GUIDALERTLEVEL] > 16777215)
    {
        TC_LOG_ERROR("server.loading", "Respawn.GuidWarnLevel (%u) cannot be greater than maximum GUID (16777215). Set to 16000000.", m_configs[CONFIG_RESPAWN_GUIDALERTLEVEL]);
        m_configs[CONFIG_RESPAWN_GUIDALERTLEVEL] = 16000000;
    }
    m_configs[CONFIG_RESPAWN_RESTARTQUIETTIME] = sConfigMgr->GetIntDefault("Respawn.RestartQuietTime", 3);
    if (m_configs[CONFIG_RESPAWN_RESTARTQUIETTIME] > 23)
    {
        TC_LOG_ERROR("server.loading", "Respawn.RestartQuietTime (%u) must be an hour, between 0 and 23. Set to 3.", m_configs[CONFIG_RESPAWN_RESTARTQUIETTIME]);
        m_configs[CONFIG_RESPAWN_RESTARTQUIETTIME] = 3;
    }
    m_configs[CONFIG_RESPAWN_DYNAMICRATE_CREATURE] = sConfigMgr->GetFloatDefault("Respawn.DynamicRateCreature", 15.0f);
    if (m_configs[CONFIG_RESPAWN_DYNAMICRATE_CREATURE] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "Respawn.DynamicRateCreature (%u) must be positive. Set to 20.", m_configs[CONFIG_RESPAWN_DYNAMICRATE_CREATURE]);
        m_configs[CONFIG_RESPAWN_DYNAMICRATE_CREATURE] = 20.0f;
    }
    m_configs[CONFIG_RESPAWN_DYNAMICMINIMUM_CREATURE] = sConfigMgr->GetIntDefault("Respawn.DynamicMinimumCreature", 30);
    m_configs[CONFIG_RESPAWN_DYNAMICRATE_GAMEOBJECT] = sConfigMgr->GetFloatDefault("Respawn.DynamicRateGameObject", 15.0f);
    if (m_configs[CONFIG_RESPAWN_DYNAMICRATE_GAMEOBJECT] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "Respawn.DynamicRateGameObject (%i) must be positive. Set to 10.", m_configs[CONFIG_RESPAWN_DYNAMICRATE_GAMEOBJECT]);
        m_configs[CONFIG_RESPAWN_DYNAMICRATE_GAMEOBJECT] = 10.0f;
    }
    m_configs[CONFIG_RESPAWN_DYNAMICMINIMUM_GAMEOBJECT] = sConfigMgr->GetIntDefault("Respawn.DynamicMinimumGameObject", 10);
    _guidWarningMsg = sConfigMgr->GetStringDefault("Respawn.WarningMessage", "There will be an unscheduled server restart at 03:00. The server will be available again shortly after.");
    _alertRestartReason = sConfigMgr->GetStringDefault("Respawn.AlertRestartReason", "Urgent Maintenance");
    m_configs[CONFIG_RESPAWN_GUIDWARNING_FREQUENCY] = sConfigMgr->GetIntDefault("Respawn.WarningFrequency", 1800);

    m_configs[CONFIG_CORPSE_DECAY_NORMAL] = sConfigMgr->GetIntDefault("Corpse.Decay.NORMAL", 60);
    m_configs[CONFIG_CORPSE_DECAY_RARE] = sConfigMgr->GetIntDefault("Corpse.Decay.RARE", 300);
    m_configs[CONFIG_CORPSE_DECAY_ELITE] = sConfigMgr->GetIntDefault("Corpse.Decay.ELITE", 300);
    m_configs[CONFIG_CORPSE_DECAY_RAREELITE] = sConfigMgr->GetIntDefault("Corpse.Decay.RAREELITE", 300);
    m_configs[CONFIG_CORPSE_DECAY_WORLDBOSS] = sConfigMgr->GetIntDefault("Corpse.Decay.WORLDBOSS", 3600);

    m_configs[CONFIG_DEATH_SICKNESS_LEVEL] = sConfigMgr->GetIntDefault("Death.SicknessLevel", 11);
    m_configs[CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP] = sConfigMgr->GetBoolDefault("Death.CorpseReclaimDelay.PvP", true);
    m_configs[CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE] = sConfigMgr->GetBoolDefault("Death.CorpseReclaimDelay.PvE", true);
    m_configs[CONFIG_DEATH_BONES_WORLD]       = sConfigMgr->GetBoolDefault("Death.Bones.World", true);
    m_configs[CONFIG_DEATH_BONES_BG_OR_ARENA] = sConfigMgr->GetBoolDefault("Death.Bones.BattlegroundOrArena", true);

    m_configs[CONFIG_THREAT_RADIUS] = sConfigMgr->GetIntDefault("ThreatRadius", 60);

    // always use declined names in the russian client
    m_configs[CONFIG_DECLINED_NAMES_USED] =
        (m_configs[CONFIG_REALM_ZONE] == REALM_ZONE_RUSSIAN) ? true : sConfigMgr->GetBoolDefault("DeclinedNames", false);

    m_configs[CONFIG_LISTEN_RANGE_SAY]       = sConfigMgr->GetIntDefault("ListenRange.Say", 25);
    m_configs[CONFIG_LISTEN_RANGE_TEXTEMOTE] = sConfigMgr->GetIntDefault("ListenRange.TextEmote", 25);
    m_configs[CONFIG_LISTEN_RANGE_YELL]      = sConfigMgr->GetIntDefault("ListenRange.Yell", 300);

    m_configs[CONFIG_ARENA_MAX_RATING_DIFFERENCE] = sConfigMgr->GetIntDefault("Arena.MaxRatingDifference", 0);
    m_configs[CONFIG_ARENA_RATING_DISCARD_TIMER] = sConfigMgr->GetIntDefault("Arena.RatingDiscardTimer",300000);
    m_configs[CONFIG_ARENA_RATED_UPDATE_TIMER] = sConfigMgr->GetIntDefault("Arena.RatedUpdateTimer", 5 * SECOND * IN_MILLISECONDS);
    m_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS] = sConfigMgr->GetBoolDefault("Arena.AutoDistributePoints", false);
    m_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS] = sConfigMgr->GetIntDefault("Arena.AutoDistributeInterval", 7);

    m_configs[CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER] = sConfigMgr->GetIntDefault("Battleground.PrematureFinishTimer", 300000);
    m_configs[CONFIG_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH] = sConfigMgr->GetIntDefault("Battleground.PremadeGroupWaitForMatch", 30 * SECOND * IN_MILLISECONDS);
    m_configs[CONFIG_BATTLEGROUND_INVITATION_TYPE] = sConfigMgr->GetIntDefault("Battleground.InvitationType", 0);
    m_configs[CONFIG_BATTLEGROUND_TIMELIMIT_WARSONG] = sConfigMgr->GetIntDefault("Battleground.TimeLimit.Warsong", 0);
    m_configs[CONFIG_BATTLEGROUND_TIMELIMIT_ARENA] = sConfigMgr->GetIntDefault("Battleground.TimeLimit.Arena", 0);

    m_configs[CONFIG_INSTANT_LOGOUT] = sConfigMgr->GetIntDefault("InstantLogout", SEC_GAMEMASTER1);

    m_configs[CONFIG_GROUPLEADER_RECONNECT_PERIOD] = sConfigMgr->GetIntDefault("GroupLeaderReconnectPeriod", 180);

    //visibility on continents
    m_MaxVisibleDistanceOnContinents      = sConfigMgr->GetFloatDefault("Visibility.Distance.Continents",     DEFAULT_VISIBILITY_DISTANCE);
    if (m_MaxVisibleDistanceOnContinents < 45*sWorld->GetRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Continents can't be less max aggro radius %f", 45*sWorld->GetRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceOnContinents = 45*sWorld->GetRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceOnContinents  >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Continents can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceOnContinents = MAX_VISIBILITY_DISTANCE ;
    }

    //visibility in instances
    m_MaxVisibleDistanceInInstances        = sConfigMgr->GetFloatDefault("Visibility.Distance.Instances",       DEFAULT_VISIBILITY_INSTANCE);
    if (m_MaxVisibleDistanceInInstances < 45*sWorld->GetRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Instances can't be less max aggro radius %f",45*sWorld->GetRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInInstances = 45*sWorld->GetRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInInstances >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Instances can't be greater %f",MAX_VISIBILITY_DISTANCE );
        m_MaxVisibleDistanceInInstances = MAX_VISIBILITY_DISTANCE ;
    }

    //visibility in BG/Arenas
    m_MaxVisibleDistanceInBGArenas        = sConfigMgr->GetFloatDefault("Visibility.Distance.BGArenas",       DEFAULT_VISIBILITY_BGARENAS);
    if (m_MaxVisibleDistanceInBGArenas < 45*sWorld->GetRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.BGArenas can't be less max aggro radius %f",45*sWorld->GetRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInBGArenas = 45*sWorld->GetRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInBGArenas + MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.BGArenas can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInBGArenas = MAX_VISIBILITY_DISTANCE ;
    }

    m_MaxVisibleDistanceForObject    = sConfigMgr->GetFloatDefault("Visibility.Distance.Object",   DEFAULT_VISIBILITY_DISTANCE);
    if(m_MaxVisibleDistanceForObject < INTERACTION_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Object can't be less max aggro radius %f",float(INTERACTION_DISTANCE));
        m_MaxVisibleDistanceForObject = INTERACTION_DISTANCE;
    }
    else if(m_MaxVisibleDistanceForObject  >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Object can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceForObject = MAX_VISIBILITY_DISTANCE ;
    }

    m_MaxVisibleDistanceInFlight    = sConfigMgr->GetFloatDefault("Visibility.Distance.InFlight",      DEFAULT_VISIBILITY_DISTANCE);
    if(m_MaxVisibleDistanceInFlight > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.InFlight can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInFlight = MAX_VISIBILITY_DISTANCE;
    }

    m_visibility_notify_periodOnContinents = sConfigMgr->GetIntDefault("Visibility.Notify.Period.OnContinents", DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    m_visibility_notify_periodInInstances = sConfigMgr->GetIntDefault("Visibility.Notify.Period.InInstances", DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    m_visibility_notify_periodInBGArenas = sConfigMgr->GetIntDefault("Visibility.Notify.Period.InBGArenas", DEFAULT_VISIBILITY_NOTIFY_PERIOD);

    ///- Read the "Data" directory from the config file
    std::string dataPath = sConfigMgr->GetStringDefault("DataDir","./");
    if( dataPath.at(dataPath.length()-1)!='/' && dataPath.at(dataPath.length()-1)!='\\' )
        dataPath.append("/");

    if(reload)
    {
        if(dataPath != m_dataPath)
            TC_LOG_ERROR("server.loading", "DataDir option can't be changed at worldserver.conf reload, using current value (%s).", m_dataPath.c_str());
    }
    else
    {
        m_dataPath = dataPath;
        TC_LOG_INFO("server.loading", "Using DataDir %s", m_dataPath.c_str());
    }

    bool enableLOS = sConfigMgr->GetBoolDefault("vmap.enableLOS", true);
    bool enableHeight = sConfigMgr->GetBoolDefault("vmap.enableHeight", true);
    VMAP::VMapFactory::createOrGetVMapManager()->setEnableLineOfSightCalc(enableLOS);
    VMAP::VMapFactory::createOrGetVMapManager()->setEnableHeightCalc(enableHeight);
    TC_LOG_INFO("server.loading", "WORLD: VMap support included. LineOfSight:%i, getHeight:%i",enableLOS, enableHeight);
    TC_LOG_INFO("server.loading", "WORLD: VMap data directory is: %svmaps",m_dataPath.c_str());

    m_configs[CONFIG_PREMATURE_BG_REWARD] = sConfigMgr->GetBoolDefault("Battleground.PrematureReward", true);
    m_configs[CONFIG_START_ALL_EXPLORED] = sConfigMgr->GetBoolDefault("PlayerStart.MapsExplored", false);
    m_configs[CONFIG_START_ALL_REP] = sConfigMgr->GetBoolDefault("PlayerStart.AllReputation", false);
    m_configs[CONFIG_ALWAYS_MAXSKILL] = sConfigMgr->GetBoolDefault("AlwaysMaxWeaponSkill", false);
    m_configs[CONFIG_PVP_TOKEN_ENABLE] = sConfigMgr->GetBoolDefault("PvPToken.Enable", false);
    m_configs[CONFIG_PVP_TOKEN_MAP_TYPE] = sConfigMgr->GetIntDefault("PvPToken.MapAllowType", 4);
    m_configs[CONFIG_PVP_TOKEN_ID] = sConfigMgr->GetIntDefault("PvPToken.ItemID", 29434);
    m_configs[CONFIG_PVP_TOKEN_COUNT] = sConfigMgr->GetIntDefault("PvPToken.ItemCount", 1);
    if(m_configs[CONFIG_PVP_TOKEN_COUNT] < 1)
        m_configs[CONFIG_PVP_TOKEN_COUNT] = 1;
    m_configs[CONFIG_PVP_TOKEN_ZONE_ID] = sConfigMgr->GetIntDefault("PvPToken.ZoneID", 0);
    m_configs[CONFIG_NO_RESET_TALENT_COST] = sConfigMgr->GetBoolDefault("NoResetTalentsCost", false);
    m_configs[CONFIG_SHOW_KICK_IN_WORLD] = sConfigMgr->GetBoolDefault("ShowKickInWorld", false);
    m_configs[CONFIG_INTERVAL_LOG_UPDATE] = sConfigMgr->GetIntDefault("RecordUpdateTimeDiffInterval", 60000);
    m_configs[CONFIG_MIN_LOG_UPDATE] = sConfigMgr->GetIntDefault("MinRecordUpdateTimeDiff", 10);
    m_configs[CONFIG_NUMTHREADS] = sConfigMgr->GetIntDefault("MapUpdate.Threads", 4);

    m_configs[CONFIG_WORLDCHANNEL_MINLEVEL] = sConfigMgr->GetIntDefault("WorldChannel.MinLevel", 10);

    m_configs[CONFIG_MONITORING_ENABLED] = sConfigMgr->GetBoolDefault("Monitor.Enabled", false);
    m_configs[CONFIG_MONITORING_GENERALINFOS_UPDATE] = sConfigMgr->GetIntDefault("Monitor.GeneralInfo.Update", 20);
    m_configs[CONFIG_MONITORING_KEEP_DURATION] = sConfigMgr->GetIntDefault("Monitor.KeepDays", 0);
    m_configs[CONFIG_MONITORING_ABNORMAL_WORLD_UPDATE_DIFF] = sConfigMgr->GetIntDefault("Monitor.AbnormalDiff.World", 500);
    m_configs[CONFIG_MONITORING_ABNORMAL_MAP_UPDATE_DIFF] = sConfigMgr->GetIntDefault("Monitor.AbnormalDiff.Map", 400);
    m_configs[CONFIG_MONITORING_ALERT_THRESHOLD_COUNT] = sConfigMgr->GetIntDefault("Monitor.LagAlertThreshold.Count", 10);
    m_configs[CONFIG_MONITORING_LAG_AUTO_REBOOT_COUNT] = sConfigMgr->GetIntDefault("Monitor.LagAutoReboot.Count", 8000);
    m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST] = sConfigMgr->GetBoolDefault("Monitor.DynamicViewDist.Enable", 0);
    m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_MINDIST] = sConfigMgr->GetIntDefault("Monitor.DynamicViewDist.MinDistance", 60);
    if (m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_MINDIST] < 60)
    {
        TC_LOG_ERROR("server.loading", "Monitor.DynamicLoS.MinDistance must be at least 60 yards, setting it to 60");
        m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_MINDIST] = 60;
    }
    m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_IDEAL_DIFF] = sConfigMgr->GetIntDefault("Monitor.DynamicViewDist.IdealDiff", 200);
    if (m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_IDEAL_DIFF] < 100)
    {
        TC_LOG_ERROR("server.loading", "Monitor.DynamicLoS.IdealDiff must be at least 100, setting it to default value (200)");
        m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_IDEAL_DIFF] = 200;
    }
    m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_TRIGGER_DIFF] = sConfigMgr->GetIntDefault("Monitor.DynamicViewDist.TriggerDiff", 400);
    if (m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_TRIGGER_DIFF] <= m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_IDEAL_DIFF])
    {
        TC_LOG_ERROR("server.loading", "Monitor.DynamicViewDist.TriggerDiff must be greater than DynamicLoS.IdealDiff. Disabling CONFIG_MONITORING_DYNAMIC_LOS");
        m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST] = false;
    }
    if (m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_TRIGGER_DIFF] < 150)
    {
        TC_LOG_ERROR("server.loading", "Monitor.DynamicViewDist.TriggerDiff must be greater than 150. Setting it to default value (400)");
        m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_TRIGGER_DIFF] = 400;
    }
    m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_CHECK_INTERVAL] = sConfigMgr->GetIntDefault("Monitor.DynamicViewDist.CheckInterval", 60);
    m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_AVERAGE_COUNT] = sConfigMgr->GetIntDefault("Monitor.DynamicViewDist.AverageCount", 500);
    if (m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_AVERAGE_COUNT] < 1)
    {
        TC_LOG_ERROR("server.loading", "Monitor.DynamicViewDist.AverageCount must be greater than 0. Setting it to default value (500)");
        m_configs[CONFIG_MONITORING_DYNAMIC_VIEWDIST_AVERAGE_COUNT] = 500;
    }


    std::string forbiddenmaps = sConfigMgr->GetStringDefault("ForbiddenMaps", "");
    auto  forbiddenMaps = new char[forbiddenmaps.length() + 1];
    forbiddenMaps[forbiddenmaps.length()] = 0;
    strncpy(forbiddenMaps, forbiddenmaps.c_str(), forbiddenmaps.length());
    const char * delim = ",";
    char * token = strtok(forbiddenMaps, delim);
    while(token != nullptr)
    {
        int32 mapid = strtol(token, nullptr, 10);
        m_forbiddenMapIds.insert(mapid);
        token = strtok(nullptr,delim);
    }
    delete[] forbiddenMaps;

    m_configs[CONFIG_BUGGY_QUESTS_AUTOCOMPLETE] = sConfigMgr->GetBoolDefault("BuggyQuests.AutoComplete", false);

    m_configs[CONFIG_AUTOANNOUNCE_ENABLED] = sConfigMgr->GetBoolDefault("AutoAnnounce.Enable", false);

    // warden
    m_configs[CONFIG_WARDEN_ENABLED] = sConfigMgr->GetBoolDefault("Warden.Enabled", true);
    m_configs[CONFIG_WARDEN_KICK] = sConfigMgr->GetBoolDefault("Warden.Kick", false);
    m_configs[CONFIG_WARDEN_NUM_CHECKS] = sConfigMgr->GetIntDefault("Warden.NumChecks", 3);
    m_configs[CONFIG_WARDEN_CLIENT_CHECK_HOLDOFF] = sConfigMgr->GetIntDefault("Warden.ClientCheckHoldOff", 30);
    m_configs[CONFIG_WARDEN_CLIENT_RESPONSE_DELAY] = sConfigMgr->GetIntDefault("Warden.ClientResponseDelay", 15);
    m_configs[CONFIG_WARDEN_DB_LOG] = sConfigMgr->GetBoolDefault("Warden.DBLogs", true);

    m_configs[CONFIG_WHISPER_MINLEVEL] = sConfigMgr->GetIntDefault("Whisper.MinLevel", 1);

    m_configs[CONFIG_ARENA_SPECTATOR_ENABLE] = sConfigMgr->GetBoolDefault("ArenaSpectator.Enable", true);
    m_configs[CONFIG_ARENA_SPECTATOR_MAX] = sConfigMgr->GetIntDefault("ArenaSpectator.Max", 10);
    m_configs[CONFIG_ARENA_SPECTATOR_GHOST] = sConfigMgr->GetBoolDefault("ArenaSpectator.Ghost", true);
    m_configs[CONFIG_ARENA_SPECTATOR_STEALTH] = sConfigMgr->GetBoolDefault("ArenaSpectator.Stealth", false);

    m_configs[CONFIG_ARENA_SEASON] = sConfigMgr->GetIntDefault("Arena.Season", 0);
    m_configs[CONFIG_ARENA_NEW_TITLE_DISTRIB] = sConfigMgr->GetBoolDefault("Arena.NewTitleDistribution.Enabled", false);
    m_configs[CONFIG_ARENA_NEW_TITLE_DISTRIB_MIN_RATING] = sConfigMgr->GetIntDefault("Arena.NewTitleDistribution.MinRating", 1800);

    m_configs[CONFIG_ARENA_DECAY_ENABLED] = sConfigMgr->GetBoolDefault("Arena.Decay.Enabled", false);
    m_configs[CONFIG_ARENA_DECAY_MINIMUM_RATING] = sConfigMgr->GetIntDefault("Arena.Decay.MinRating", 1800);
    m_configs[CONFIG_ARENA_DECAY_VALUE] = sConfigMgr->GetIntDefault("Arena.Decay.Value", 20);
    m_configs[CONFIG_ARENA_DECAY_CONSECUTIVE_WEEKS] = sConfigMgr->GetIntDefault("Arena.Decay.ConsecutiveWeeks", 2);

    //packet spoof punishment
    m_configs[CONFIG_PACKET_SPOOF_POLICY] = sConfigMgr->GetIntDefault("PacketSpoof.Policy", (uint32)WorldSession::DosProtection::POLICY_KICK);
    m_configs[CONFIG_PACKET_SPOOF_BANMODE] = sConfigMgr->GetIntDefault("PacketSpoof.BanMode", (uint32)SANCTION_BAN_ACCOUNT);
    if (m_configs[CONFIG_PACKET_SPOOF_BANMODE] == SANCTION_BAN_CHARACTER || m_configs[CONFIG_PACKET_SPOOF_BANMODE] > SANCTION_BAN_IP)
        m_configs[CONFIG_PACKET_SPOOF_BANMODE] = SANCTION_BAN_ACCOUNT;

    m_configs[CONFIG_SPAM_REPORT_THRESHOLD] = sConfigMgr->GetIntDefault("Spam.Report.Threshold", 3);
    m_configs[CONFIG_SPAM_REPORT_PERIOD] = sConfigMgr->GetIntDefault("Spam.Report.Period", 120); // In seconds
    m_configs[CONFIG_SPAM_REPORT_COOLDOWN] = sConfigMgr->GetIntDefault("Spam.Report.Cooldown", 120); // In seconds

    m_configs[CONFIG_FACTION_CHANGE_ENABLED] = sConfigMgr->GetBoolDefault("Faction.Change.Enabled", false);
    m_configs[CONFIG_FACTION_CHANGE_A2H] = sConfigMgr->GetBoolDefault("Faction.Change.AllianceToHorde", false);
    m_configs[CONFIG_FACTION_CHANGE_H2A] = sConfigMgr->GetBoolDefault("Faction.Change.HordeToAlliance", false);
    m_configs[CONFIG_FACTION_CHANGE_H2A_COST] = sConfigMgr->GetIntDefault("Faction.Change.AllianceToHorde.Cost", 4);
    m_configs[CONFIG_FACTION_CHANGE_H2A_COST] = sConfigMgr->GetIntDefault("Faction.Change.HordeToAlliance.Cost", 4);
    m_configs[CONFIG_RACE_CHANGE_COST] = sConfigMgr->GetIntDefault("Race.Change.Cost", 4);

    m_configs[CONFIG_ARENASERVER_ENABLED] = sConfigMgr->GetBoolDefault("ArenaServer.Enabled", false);
    m_configs[CONFIG_ARENASERVER_USE_CLOSESCHEDULE] = sConfigMgr->GetBoolDefault("ArenaServer.UseCloseSchedule", false);
    m_configs[CONFIG_ARENASERVER_PLAYER_REPARTITION_THRESHOLD] = sConfigMgr->GetIntDefault("ArenaServer.RepartitionThreshold", 0);

    m_configs[CONFIG_BETASERVER_ENABLED] = sConfigMgr->GetIntDefault("BetaServer.Enabled", false);

    m_configs[CONFIG_DEBUG_DISABLE_MAINHAND] = sConfigMgr->GetBoolDefault("Debug.DisableMainHand", 0);
    m_configs[CONFIG_DEBUG_DISABLE_ARMOR] = sConfigMgr->GetBoolDefault("Debug.DisableArmor", 0);
    m_configs[CONFIG_DEBUG_LOG_LAST_PACKETS] = sConfigMgr->GetBoolDefault("Debug.LogLastPackets", 0);
    m_configs[CONFIG_DEBUG_LOG_ALL_PACKETS] = sConfigMgr->GetBoolDefault("Debug.LogAllPackets", 0);
    m_configs[CONFIG_DEBUG_DISABLE_CREATURES_LOADING] = sConfigMgr->GetBoolDefault("Debug.DisableCreaturesLoading", 0);
    m_configs[CONFIG_DEBUG_DISABLE_GAMEOBJECTS_LOADING] = sConfigMgr->GetBoolDefault("Debug.DisableGameObjectsLoading", 0);
    m_configs[CONFIG_DEBUG_DISABLE_TRANSPORTS] = sConfigMgr->GetBoolDefault("Debug.DisableTransports", 0);


    if (int32 clientCacheId = sConfigMgr->GetIntDefault("ClientCacheVersion", 0))
    {
        // overwrite DB/old value
        if (clientCacheId > 0)
        {
            m_configs[CONFIG_CLIENTCACHE_VERSION] = clientCacheId;
            TC_LOG_INFO("server.loading", "Client cache version set to: %u", clientCacheId);
        }
        else
            TC_LOG_ERROR("server.loading", "ClientCacheVersion can't be negative %d, ignored.", clientCacheId);
    }

    LoadSanctuaryAndFFAZones();
    LoadFishingWords();

    // call ScriptMgr if we're reloading the configuration
   /* if (reload)
        sScriptMgr->OnConfigLoad(reload);*/

    m_configs[CONFIG_HOTSWAP_ENABLED] = sConfigMgr->GetBoolDefault("HotSwap.Enabled", true);
    m_configs[CONFIG_HOTSWAP_RECOMPILER_ENABLED] = sConfigMgr->GetBoolDefault("HotSwap.EnableReCompiler", true);
    m_configs[CONFIG_HOTSWAP_EARLY_TERMINATION_ENABLED] = sConfigMgr->GetBoolDefault("HotSwap.EnableEarlyTermination", true);
    m_configs[CONFIG_HOTSWAP_BUILD_FILE_RECREATION_ENABLED] = sConfigMgr->GetBoolDefault("HotSwap.EnableBuildFileRecreation", true);
    m_configs[CONFIG_HOTSWAP_INSTALL_ENABLED] = sConfigMgr->GetBoolDefault("HotSwap.EnableInstall", true);
    m_configs[CONFIG_HOTSWAP_PREFIX_CORRECTION_ENABLED] = sConfigMgr->GetBoolDefault("HotSwap.EnablePrefixCorrection", true);

    m_configs[CONFIG_DB_PING_INTERVAL] = sConfigMgr->GetIntDefault("MaxPingTime", 5);
}

/// Initialize the World
void World::SetInitialWorldSettings()
{
    ///- Initialize start time
    uint32 serverStartingTime = GetMSTime();

    ///- Initialize the random number generator
    srand((uint32)time(nullptr));

    ///- Initialize detour memory management
    dtAllocSetCustom(dtCustomAlloc, dtCustomFree);

    ///- Initialize VMapManager function pointers (to untangle game/collision circular deps)
    if (VMAP::VMapManager2* vmmgr2 = dynamic_cast<VMAP::VMapManager2*>(VMAP::VMapFactory::createOrGetVMapManager()))
    {
        vmmgr2->GetLiquidFlagsPtr = &GetLiquidFlags;
        //vmmgr2->IsVMAPDisabledForPtr = &DisableMgr::IsVMAPDisabledFor;
    }

    ///- Initialize config settings
    LoadConfigSettings();

    ///- Initialize motd and twitter
    LoadMotdAndTwitter();

    ///- Init highest guids before any table loading to prevent using not initialized guids in some code.
    sObjectMgr->SetHighestGuids();

    ///- Check the existence of the map files for all races' startup areas.
    if(    !MapManager::ExistMapAndVMap(0,-6240.32f, 331.033f)
        || !MapManager::ExistMapAndVMap(0,-8949.95f,-132.493f)
        || !MapManager::ExistMapAndVMap(0,-8949.95f,-132.493f)
        || !MapManager::ExistMapAndVMap(1,-618.518f,-4251.67f)
        || !MapManager::ExistMapAndVMap(0, 1676.35f, 1677.45f)
        || !MapManager::ExistMapAndVMap(1, 10311.3f, 832.463f)
        || !MapManager::ExistMapAndVMap(1,-2917.58f,-257.98f)
        || (m_configs[CONFIG_EXPANSION]
           && (!MapManager::ExistMapAndVMap(530,10349.6f,-6357.29f) || !MapManager::ExistMapAndVMap(530,-3961.64f,-13931.2f)) )
      )
    {
        TC_LOG_ERROR("server.loading", "Correct *.map files not found in path '%smaps' or *.vmap/*vmdir files in '%svmaps'. Please place *.map/*.vmap/*.vmdir files in appropriate directories or correct the DataDir value in the worldserver.conf file.", m_dataPath.c_str(), m_dataPath.c_str());
        exit(1);
    }

    ///- Initialize pool manager
    sPoolMgr->Initialize();

    ///- Loading strings. Getting no records means core load has to be canceled because no error message can be output.
    TC_LOG_INFO("server.loading", "Loading Trinity strings..." );
    if (!sObjectMgr->LoadTrinityStrings())
        exit(1);                                            // Error message displayed in function already

    ///- Update the realm entry in the database with the realm type from the config file
    //No SQL injection as values are treated as integers

    // not send custom type REALM_FFA_PVP to realm list
    uint32 server_type = IsFFAPvPRealm() ? REALM_TYPE_PVP : getConfig(CONFIG_GAME_TYPE);
    uint32 realm_zone = getConfig(CONFIG_REALM_ZONE);
    LoginDatabase.PExecute("UPDATE realmlist SET icon = %u, timezone = %u WHERE id = '%d'", server_type, realm_zone, realm.Id.Realm);

    ///- Remove the bones after a restart
    CharacterDatabase.PExecute("DELETE FROM corpse WHERE corpse_type = '0'");

    ///- Load the DBC files
    TC_LOG_INFO("server.loading","Initialize data stores...");
    LoadDBCStores(m_dataPath);
    DetectDBCLang();

    // Load cinematic cameras
    LoadM2Cameras(m_dataPath);

    std::vector<uint32> mapIds;
    for (uint32 mapId = 0; mapId < sMapStore.GetNumRows(); mapId++)
        if (sMapStore.LookupEntry(mapId))
            mapIds.push_back(mapId);

    if (VMAP::VMapManager2* vmmgr2 = dynamic_cast<VMAP::VMapManager2*>(VMAP::VMapFactory::createOrGetVMapManager()))
        vmmgr2->InitializeThreadUnsafe(mapIds);

    MMAP::MMapManager* mmmgr = MMAP::MMapFactory::createOrGetMMapManager();
    mmmgr->InitializeThreadUnsafe(mapIds);

    TC_LOG_INFO("server.loading","Loading Item Extended Cost Data...");
    sObjectMgr->LoadItemExtendedCost();

    TC_LOG_INFO("server.loading","Loading Spell templates...");
    sObjectMgr->LoadSpellTemplates();

    TC_LOG_INFO("server.loading", "Loading SkillLineAbilityMultiMap Data..." );
    sSpellMgr->LoadSkillLineAbilityMap();

    TC_LOG_INFO("server.loading", "Loading Spell Required Data..." );
    sSpellMgr->LoadSpellRequired();

    ///- Initialize static helper structures
    AIRegistry::Initialize();

    TC_LOG_INFO("server.loading", "Loading SpellInfo store...");  //must be after all SpellEntry's alterations
    sSpellMgr->LoadSpellInfoStore(false);

    TC_LOG_INFO("server.loading", "Loading Spell Elixir types..." ); //must be after SpellInfo
    sSpellMgr->LoadSpellElixirs();

    TC_LOG_INFO("server.loading", "Loading Spell Rank Data...." ); //must be after LoadSkillLineAbilityMap and after SpellInfo
    sSpellMgr->LoadSpellRanks();

    TC_LOG_INFO("server.loading", "Loading Spell Learn Skills..." );
    sSpellMgr->LoadSpellLearnSkills();                        // must be after LoadSpellChains and after SpellInfo

    TC_LOG_INFO("server.loading", "Loading Spell Learn Spells..." );  //must be after SpellInfo
    sSpellMgr->LoadSpellLearnSpells();

    TC_LOG_INFO("server.loading", "Loading Spell Bonus Data...");
    sSpellMgr->LoadSpellBonusess();

    TC_LOG_INFO("server.loading", "Loading Threat Spells Definitions..."); //must be after SpellInfo
    sSpellMgr->LoadSpellThreats();

    TC_LOG_INFO("server.loading", "Loading Spell Proc Event conditions..." ); //must be after spellInfo
    sSpellMgr->LoadSpellProcEvents();

    TC_LOG_INFO("server.loading", "Loading Script Names...");
    sObjectMgr->LoadScriptNames();

    TC_LOG_INFO("server.loading", "Loading InstanceTemplate" );
    sObjectMgr->LoadInstanceTemplate();

    // sunwell: Global Storage, should be loaded asap
    TC_LOG_INFO("server.loading", "Load Global Player Data...");
    sCharacterCache->LoadCharacterCacheStorage();

    ///- Clean up and pack instances
    // Must be called before `creature_respawn`/`gameobject_respawn` tables
    TC_LOG_INFO("server.loading", "Loading instances..." );
    sInstanceSaveMgr->LoadInstances();                              // must be called before `creature_respawn`/`gameobject_respawn` tables

//    TC_LOG_INFO("server.loading", "Packing instances..." );
//    sInstanceSaveMgr->PackInstances();

    TC_LOG_INFO("server.loading", "Loading Broadcast texts...");
    sObjectMgr->LoadBroadcastTexts();
    sObjectMgr->LoadBroadcastTextLocales();

    TC_LOG_INFO("server.loading", "Loading Localization strings..." );
    sObjectMgr->LoadCreatureLocales();
    sObjectMgr->LoadGameObjectLocales();
    sObjectMgr->LoadItemLocales();
    sObjectMgr->LoadQuestLocales();
    sObjectMgr->LoadGossipTextLocales();
    sObjectMgr->LoadPageTextLocales();
    sObjectMgr->LoadGossipMenuItemsLocales();
    sObjectMgr->SetDBCLocaleIndex(GetDefaultDbcLocale());        // Get once for all the locale index of DBC language (console/broadcasts)

    TC_LOG_INFO("server.loading", "Loading Page Texts..." );
    sObjectMgr->LoadPageTexts();

    TC_LOG_INFO("server.loading", "Loading Game Object Templates..." );   // must be after LoadPageTexts
    sObjectMgr->LoadGameObjectTemplate();

    TC_LOG_INFO("server.loading", "Loading NPC Texts..." );
    sObjectMgr->LoadGossipText();

    TC_LOG_INFO("server.loading", "Loading Enchant Spells Proc datas...");
    sSpellMgr->LoadSpellEnchantProcData();

    TC_LOG_INFO("server.loading", "Loading Item Random Enchantments Table..." );
    LoadRandomEnchantmentsTable();

    TC_LOG_INFO("server.loading", "Loading Items..." );                   // must be after LoadRandomEnchantmentsTable and LoadPageTexts
    sObjectMgr->LoadItemTemplates();

    TC_LOG_INFO("server.loading", "Loading Item Texts..." );
    sObjectMgr->LoadItemTexts();

    TC_LOG_INFO("server.loading", "Loading Creature Model Based Info Data..." );
    sObjectMgr->LoadCreatureModelInfo();

    TC_LOG_INFO("server.loading", "Loading Creature templates..." );
    sObjectMgr->LoadCreatureTemplates(false);

    TC_LOG_INFO("server.loading", "Loading Equipment templates...");
    sObjectMgr->LoadEquipmentTemplates();

    TC_LOG_INFO("server.loading", "Loading Creature template addons...");
    sObjectMgr->LoadCreatureTemplateAddons();

    TC_LOG_INFO("server.loading", "Loading Creature Reputation OnKill Data..." );
    sObjectMgr->LoadReputationOnKill();

    TC_LOG_INFO("server.loading", "Loading Points Of Interest Data...");
    sObjectMgr->LoadPointsOfInterest();

    TC_LOG_INFO("server.loading", "Loading Pet Create Spells..." );
    sObjectMgr->LoadPetCreateSpells();

    TC_LOG_INFO("server.loading", "Loading Creature Base Stats...");
    sObjectMgr->LoadCreatureClassLevelStats();

    TC_LOG_INFO("server.loading", "Loading Spawn Group Templates...");
    sObjectMgr->LoadSpawnGroupTemplates();

    TC_LOG_INFO("server.loading", "Loading instance spawn groups...");
    sObjectMgr->LoadInstanceSpawnGroups();

    if(!getConfig(CONFIG_DEBUG_DISABLE_CREATURES_LOADING))
    {
        TC_LOG_INFO("server.loading", "Loading Creature Data..." );
        sObjectMgr->LoadCreatures();

        TC_LOG_INFO("server.loading", "Loading Creature Addon Data..." );
        sObjectMgr->LoadCreatureAddons();                            // must be after LoadCreatureTemplates() and LoadCreatures()
    }

    if (!getConfig(CONFIG_DEBUG_DISABLE_GAMEOBJECTS_LOADING))
    {
        TC_LOG_INFO("server.loading", "Loading Gameobject Data...");
        sObjectMgr->LoadGameObjects();
    }

    TC_LOG_INFO("server.loading", "Loading Spawn Group Data...");
    sObjectMgr->LoadSpawnGroups();

    TC_LOG_INFO("server.loading", "Loading Linked Respawn...");
    sObjectMgr->LoadLinkedRespawn();                     // must be after LoadCreatures(), LoadGameObjects()

    TC_LOG_INFO("server.loading","Loading Transport templates...");
    sTransportMgr->LoadTransportTemplates();

    TC_LOG_INFO("server.loading", "Loading Objects Pooling Data...");
    sPoolMgr->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Game Event Data...");
    sGameEventMgr->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Weather Data..." );
    sObjectMgr->LoadWeatherZoneChances();

    TC_LOG_INFO("server.loading","Loading GameObject models...");
    LoadGameObjectModelList(sWorld->GetDataPath());

    TC_LOG_INFO("server.loading", "Loading SpellInfo diminishing infos...");
    sSpellMgr->LoadSpellInfoDiminishing();

    TC_LOG_INFO("server.loading", "Loading SpellInfo immunity infos...");
    sSpellMgr->LoadSpellInfoImmunities();

    TC_LOG_INFO("server.loading", "Loading Quests..." );
    sObjectMgr->LoadQuests();                                    // must be loaded after DBCs, creature_template, item_template, gameobject tables

    TC_LOG_INFO("server.loading", "Loading Quests Relations..." );
    sObjectMgr->LoadQuestRelations();                            // must be after quest load

    TC_LOG_INFO("server.loading", "Loading AreaTrigger definitions..." );
    sObjectMgr->LoadAreaTriggerTeleports();

    TC_LOG_INFO("server.loading", "Loading Access Requirements..." );
    sObjectMgr->LoadAccessRequirements();                        // must be after item template load

    TC_LOG_INFO("server.loading", "Loading Quest Area Triggers..." );
    sObjectMgr->LoadQuestAreaTriggers();                         // must be after LoadQuests

    TC_LOG_INFO("server.loading", "Loading SpellArea Data...");  // must be after quest load
    sSpellMgr->LoadSpellAreas();

    TC_LOG_INFO("server.loading", "Loading Tavern Area Triggers..." );
    sObjectMgr->LoadTavernAreaTriggers();

    TC_LOG_INFO("server.loading", "Loading AreaTrigger script names..." );
    sObjectMgr->LoadAreaTriggerScripts();

    TC_LOG_INFO("server.loading", "Loading Graveyard-zone links...");
    sObjectMgr->LoadGraveyardZones();

    TC_LOG_INFO("server.loading", "Loading Spell target coordinates..." );
    sSpellMgr->LoadSpellTargetPositions();

    TC_LOG_INFO("server.loading", "Loading SpellAffect definitions..." );
    sSpellMgr->LoadSpellAffects();

    TC_LOG_INFO("server.loading", "Loading spell pet auras..." );
    sSpellMgr->LoadSpellPetAuras();

    TC_LOG_INFO("server.loading","Overriding SpellItemEnchantment...");
    sSpellMgr->OverrideSpellItemEnchantment();

    TC_LOG_INFO("server.loading", "Loading player Create Info & Level Stats..." );
    sObjectMgr->LoadPlayerInfo();

    TC_LOG_INFO("server.loading", "Loading Exploration BaseXP Data..." );
    sObjectMgr->LoadExplorationBaseXP();

    TC_LOG_INFO("server.loading", "Loading Pet Name Parts..." );
    sObjectMgr->LoadPetNames();

    TC_LOG_INFO("server.loading", "Loading the max pet number..." );
    sObjectMgr->LoadPetNumber();

    TC_LOG_INFO("server.loading", "Loading pet level stats..." );
    sObjectMgr->LoadPetLevelInfo();

    TC_LOG_INFO("server.loading", "Loading Disabled Spells..." );
    sObjectMgr->LoadSpellDisabledEntrys();

    TC_LOG_INFO("server.loading", "Loading Loot Tables..." );
    LoadLootTables();

    TC_LOG_INFO("server.loading", "Loading Skill Discovery Table..." );
    LoadSkillDiscoveryTable();

    TC_LOG_INFO("server.loading", "Loading Skill Extra Item Table..." );
    LoadSkillExtraItemTable();

    TC_LOG_INFO("server.loading", "Loading Skill Fishing base level requirements..." );
    sObjectMgr->LoadFishingBaseSkillLevel();

    ///- Load dynamic data tables from the database
    TC_LOG_INFO("server.loading", "Loading Auctions..." );
    sAuctionMgr->LoadAuctionItems();
    sAuctionMgr->LoadAuctions();

//    TC_LOG_INFO("server.loading", "Loading Guilds..." );
//    sObjectMgr->LoadGuilds();

    TC_LOG_INFO("server.loading", "Loading ArenaTeams..." );
    sObjectMgr->LoadArenaTeams();

    TC_LOG_INFO("server.loading", "Loading Groups..." );
    sObjectMgr->LoadGroups();

    TC_LOG_INFO("server.loading", "Loading ReservedNames..." );
    sObjectMgr->LoadReservedPlayersNames();

    TC_LOG_INFO("server.loading", "Loading GameObject for quests..." );
    sObjectMgr->LoadGameObjectForQuests();

    TC_LOG_INFO("server.loading", "Loading BattleMasters..." );
    sObjectMgr->LoadBattleMastersEntry();

    TC_LOG_INFO("server.loading", "Loading GameTeleports..." );
    sObjectMgr->LoadGameTele();

    TC_LOG_INFO("server.loading", "Loading Npc gossip menus..." );
    sObjectMgr->LoadGossipMenu();

    TC_LOG_INFO("server.loading", "Loading Npc Options..." );
    sObjectMgr->LoadGossipMenuItems();

    TC_LOG_INFO("server.loading", "Loading Npc gossips Id..." );
    sObjectMgr->LoadCreatureGossip();                                 // must be after load Creature and menus

    TC_LOG_INFO("server.loading", "Loading vendors..." );
    sObjectMgr->LoadVendors();                                   // must be after load CreatureTemplate and ItemTemplate

    TC_LOG_INFO("server.loading", "Loading trainers..." );
    sObjectMgr->LoadTrainerSpell();                              // must be after load CreatureTemplate

    TC_LOG_INFO("server.loading", "Loading Waypoints..." );
    sWaypointMgr->Load();

    TC_LOG_INFO("server.loading","Loading SmartAI Waypoints...");
    sSmartWaypointMgr->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Creature Formations..." );
    sCreatureGroupMgr->LoadCreatureFormations();

    TC_LOG_INFO("server.loading","Loading Conditions...");
    sConditionMgr->LoadConditions();

    TC_LOG_INFO("server.loading", "Loading GM tickets...");
    sObjectMgr->LoadGMTickets();

    TC_LOG_INFO("server.loading", "Loading client addons...");
    AddonMgr::LoadFromDB();

    ///- Handle outdated emails (delete/return)
    TC_LOG_INFO("server.loading", "Returning old mails..." );
    sObjectMgr->ReturnOrDeleteOldMails(false);

    TC_LOG_INFO("server.loading","*** Faction change system ***");
    TC_LOG_INFO("server.loading","Loading faction change items...");
    sObjectMgr->LoadFactionChangeItems();
    TC_LOG_INFO("server.loading","Loading faction change spells...");
    sObjectMgr->LoadFactionChangeSpells();
    TC_LOG_INFO("server.loading","Loading faction change titles...");
    sObjectMgr->LoadFactionChangeTitles();
    TC_LOG_INFO("server.loading","Loading faction change quests...");
    sObjectMgr->LoadFactionChangeQuests();
    TC_LOG_INFO("server.loading","Loading faction change reputations (generic)...");
    sObjectMgr->LoadFactionChangeReputGeneric();

    TC_LOG_INFO("server.loading", "Loading spell script names...");
    sObjectMgr->LoadSpellScriptNames();

    TC_LOG_INFO("server.loading","Loading Creature Texts...");
    sCreatureTextMgr->LoadCreatureTexts();

    TC_LOG_INFO("server.loading", "Loading Creature Text Locales...");
    sCreatureTextMgr->LoadCreatureTextLocales();

    ///- Load and initialize scripts
    TC_LOG_INFO("server.loading", "Loading Scripts..." );
    sObjectMgr->LoadQuestStartScripts();                         // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sObjectMgr->LoadQuestEndScripts();                           // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sObjectMgr->LoadSpellScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadGameObjectScripts();                         // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadEventScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadWaypointScripts();

    TC_LOG_INFO("server.loading", "Initializing Scripts..." );
    sScriptMgr->Initialize(_TRINITY_SCRIPT_CONFIG);
//TC    sScriptMgr->OnConfigLoad(false);                                // must be done after the ScriptMgr has been properly initialized

    TC_LOG_INFO("server.loading","Loading SmartAI scripts...");
    sSmartScriptMgr->LoadSmartAIFromDB();

    ///- Initialize game time and timers
    TC_LOG_INFO("server.loading", "Initialize game time and timers");
    GameTime::UpdateGameTimers();

    tm local;
    time_t curr;
    time(&curr);
    local=*(localtime(&curr));                              // dereference and assign
    char isoDate[128];
    sprintf( isoDate, "%04d-%02d-%02d %02d:%02d:%02d",
        local.tm_year+1900, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);

    LoginDatabase.PExecute("INSERT INTO uptime (realmid, starttime, uptime, revision, maxplayers) VALUES('%u', " UI64FMTD ", 0, '%s', 0)",
        realm.Id.Realm, uint64(GameTime::GetStartTime()), isoDate, GitRevision::GetFullVersion());

    m_timers[WUPDATE_WEATHERS].SetInterval(1*IN_MILLISECONDS);
    m_timers[WUPDATE_AUCTIONS].SetInterval(MINUTE*IN_MILLISECONDS);    //set auction update interval to 1 minute
    m_timers[WUPDATE_UPTIME].SetInterval(m_configs[CONFIG_UPTIME_UPDATE]*MINUTE*IN_MILLISECONDS);
                                                            //Update "uptime" table based on configuration entry in minutes.
    m_timers[WUPDATE_CORPSES].SetInterval(20*MINUTE*IN_MILLISECONDS);  //erase corpses every 20 minutes
    m_timers[WUPDATE_ANNOUNCES].SetInterval(MINUTE*IN_MILLISECONDS); // Check announces every minute

    m_timers[WUPDATE_PINGDB].SetInterval(getIntConfig(CONFIG_DB_PING_INTERVAL)*MINUTE*IN_MILLISECONDS);    // Mysql ping time in minutes

    m_timers[WUPDATE_ARENASEASONLOG].SetInterval(MINUTE*1000);

    m_timers[WUPDATE_CHECK_FILECHANGES].SetInterval(500);

    m_timers[WUPDATE_WHO_LIST].SetInterval(5 * IN_MILLISECONDS); // update who list cache every 5 seconds

    //to set mailtimer to return mails every day between 4 and 5 am
    //mailtimer is increased when updating auctions
    //one second is 1000 -(tested on win system)
    tm localTm;
    time_t gameTime = GameTime::GetGameTime();
    localtime_r(&gameTime, &localTm);
    mail_timer = ((((localTm.tm_hour + 20) % 24)* HOUR * IN_MILLISECONDS) / m_timers[WUPDATE_AUCTIONS].GetInterval());
                                                            //1440
    mail_timer_expires = ( (DAY * IN_MILLISECONDS) / (m_timers[WUPDATE_AUCTIONS].GetInterval()));

    ///- Initialize MapManager
    TC_LOG_INFO("server.loading", "Starting Map System" );
    sMapMgr->Initialize();

    // Load Warden Data
    TC_LOG_INFO("server.loading","Loading Warden Data..." );
    WardenDataStorage.Init();

    ///- Initialize Battlegrounds
    TC_LOG_INFO("server.loading", "Starting Battleground System" );
    sBattlegroundMgr->LoadBattlegroundTemplates();
    sBattlegroundMgr->InitAutomaticArenaPointDistribution();

    ///- Initialize outdoor pvp
    TC_LOG_INFO("server.loading", "Starting Outdoor PvP System" );
    sOutdoorPvPMgr->InitOutdoorPvP();

    TC_LOG_INFO("server.loading", "Loading Transports..." );
    if (!getConfig(CONFIG_DEBUG_DISABLE_TRANSPORTS))
        sTransportMgr->SpawnContinentTransports();

    TC_LOG_INFO("server.loading","Deleting expired bans..." );
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    TC_LOG_INFO("server.loading","Calculate next daily quest reset time..." );
    InitDailyQuestResetTime();

    TC_LOG_INFO("server.loading","Starting Game Event system..." );
    uint32 nextGameEvent = sGameEventMgr->Initialize();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);    //depend on next event

    // Delete all characters which have been deleted X days before
    Player::DeleteOldCharacters();

    //TC_LOG_INFO("server.loading","Initialize AuctionHouseBot...");
    //auctionbot.Initialize();

    TC_LOG_INFO("server.loading", "Initializing Opcodes...");
    opcodeTable.Initialize();

    TC_LOG_INFO("server.loading","Initialize Quest Pools...");
    LoadQuestPoolsData();

    TC_LOG_INFO("server.loading","Loading automatic announces...");
    LoadAutoAnnounce();

    TC_LOG_INFO("server.loading","Cleaning up old logs...");
    if(m_configs[CONFIG_MONITORING_ENABLED])
        sLogsDatabaseAccessor->CleanupOldMonitorLogs();

    sLogsDatabaseAccessor->CleanupOldLogs();

    #ifdef PLAYERBOT
    sPlayerbotAIConfig.Initialize();
    #endif

    uint32 serverStartedTime = GetMSTimeDiffToNow(serverStartingTime);
    TC_LOG_INFO("server.loading","World initialized in %u.%u seconds.", (serverStartedTime / 1000), (serverStartedTime % 1000));
}

void World::DetectDBCLang()
{
    uint32 m_lang_confid = sConfigMgr->GetIntDefault("DBC.Locale", 255);

    if(m_lang_confid != 255 && m_lang_confid >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("server.loading","Incorrect DBC.Locale! Must be >= 0 and < %d (set to 0). Falling back to enUS.", TOTAL_LOCALES);
        m_lang_confid = LOCALE_enUS;
    }

    ChrRacesEntry const* race = sChrRacesStore.LookupEntry(1);

    std::string availableLocalsStr;

    uint8 default_locale = TOTAL_LOCALES;
    for (int i = MAX_LOCALE; i >= 0; --i)
    {
        if ( strlen(race->name[i]) > 0)                     // check by race names
        {
            default_locale = i;
            m_availableDbcLocaleMask |= (1 << i);
            availableLocalsStr += localeNames[i];
            availableLocalsStr += " ";
        }
    }

    if( default_locale != m_lang_confid && m_lang_confid < TOTAL_LOCALES &&
        (m_availableDbcLocaleMask & (1 << m_lang_confid)) )
    {
        default_locale = m_lang_confid;
    }

    if(default_locale >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("server.loading","Unable to determine your DBC Locale! (corrupt DBC?)");
        exit(1);
    }

    m_defaultDbcLocale = LocaleConstant(default_locale);

    TC_LOG_INFO("server.loading", "Using %s DBC Locale as default. All available DBC locales: %s",localeNames[m_defaultDbcLocale],availableLocalsStr.empty() ? "<none>" : availableLocalsStr.c_str());
}

void World::ResetTimeDiffRecord()
{
    if (m_updateTimeCount != 1)
        return;

    m_currentTime = GetMSTime();
}

void World::RecordTimeDiff(std::string const& text)
{
    if (m_updateTimeCount != 1)
        return;

    uint32 thisTime = GetMSTime();
    uint32 diff = GetMSTimeDiff(m_currentTime, thisTime);

    if (diff > m_configs[CONFIG_MIN_LOG_UPDATE])
        TC_LOG_INFO("misc", "Difftime %s: %u.", text.c_str(), diff);

    m_currentTime = thisTime;
}

uint32 World::GetCurrentQuestForPool(uint32 poolId)
{
    std::map<uint32, uint32>::const_iterator itr = m_currentQuestInPools.find(poolId);
    if (itr != m_currentQuestInPools.end())
        return itr->second;

    return 0;
}

bool World::IsQuestInAPool(uint32 questId)
{
    for (std::vector<uint32>::const_iterator itr = m_questInPools.begin(); itr != m_questInPools.end(); itr++) {
        if (*itr == questId)
            return true;
    }

    return false;
}

bool World::IsQuestCurrentOfAPool(uint32 questId)
{
    for (std::map<uint32, uint32>::const_iterator itr = m_currentQuestInPools.begin(); itr != m_currentQuestInPools.end(); itr++) {
        if (itr->second == questId)
            return true;
    }

    return false;
}

void World::LoadQuestPoolsData()
{
    m_questInPools.clear();
    m_currentQuestInPools.clear();
    QueryResult result = WorldDatabase.PQuery("SELECT quest_id FROM quest_pool");
    if (!result)
        return;

    do {
        Field* fields = result->Fetch();
        uint32 questId = fields[0].GetUInt32();
        m_questInPools.push_back(questId);
    } while (result->NextRow());

    result = WorldDatabase.PQuery("SELECT pool_id, quest_id FROM quest_pool_current");
    if (!result)
        return;

    do {
        Field* fields = result->Fetch();
        uint32 poolId = fields[0].GetUInt32();
        uint32 questId = fields[1].GetUInt32();
        m_currentQuestInPools[poolId] = questId;
    } while (result->NextRow());
}

/// Update the World !
void World::Update(time_t diff)
{
    ///- Update the game time and check for shutdown time
    _UpdateGameTime();
    time_t currentGameTime = GameTime::GetGameTime();

    sMonitor->StartedWorldLoop();

    if(m_configs[CONFIG_INTERVAL_LOG_UPDATE])
    {
        if(m_updateTimeSum > m_configs[CONFIG_INTERVAL_LOG_UPDATE])
        {
            TC_LOG_DEBUG("misc","Update time diff: %u. Players online: %u.", m_updateTimeSum / m_updateTimeCount, GetActiveSessionCount());
            m_updateTimeSum = m_updateTime;
            m_updateTimeCount = 1;
        }
        else
        {
            m_updateTimeSum += m_updateTime;
            ++m_updateTimeCount;
        }
    }

    ///- Update the different timers
    for(auto & m_timer : m_timers)
        if(m_timer.GetCurrent()>=0)
            m_timer.Update(diff);
    else m_timer.SetCurrent(0);

    ///- Update Who List Storage
    if (m_timers[WUPDATE_WHO_LIST].Passed())
    {
        m_timers[WUPDATE_WHO_LIST].Reset();
        sWhoListStorageMgr->Update();
    }

    ///- Update the game time and check for shutdown time
    _UpdateGameTime();

    /// Handle daily quests reset time
    if(currentGameTime > m_NextDailyQuestReset)
    {
        ResetDailyQuests();
        InitNewDataForQuestPools();
        InitDailyQuestResetTime(false);
    }

    /// <ul><li> Handle auctions when the timer has passed
    if (m_timers[WUPDATE_AUCTIONS].Passed())
    {
        m_timers[WUPDATE_AUCTIONS].Reset();

        ///- Update mails (return old mails with item, or delete them)
        //(tested... works on win)
        if (++mail_timer > mail_timer_expires)
        {
            mail_timer = 0;
            sObjectMgr->ReturnOrDeleteOldMails(true);
        }
        ///-Handle expired auctions
        sAuctionMgr->Update();
    }

    #ifdef PLAYERBOT
    sRandomPlayerbotMgr.UpdateAI(diff);
    sRandomPlayerbotMgr.UpdateSessions(diff);
    #endif

    /// <li> Handle file changes
    if (m_timers[WUPDATE_CHECK_FILECHANGES].Passed())
    {
        sScriptReloadMgr->Update();
        m_timers[WUPDATE_CHECK_FILECHANGES].Reset();
    }

    /// <li> Handle session updates
    ResetTimeDiffRecord();
    UpdateSessions(diff);
    RecordTimeDiff("UpdateSessions");

    // Update groups
    for (auto itr = sObjectMgr->GetGroupSetBegin(); itr != sObjectMgr->GetGroupSetEnd(); ++itr)
        (*itr)->Update(diff);
    RecordTimeDiff("UpdateGroups");

    /// <li> Handle weather updates when the timer has passed
    if (m_timers[WUPDATE_WEATHERS].Passed())
    {
        m_timers[WUPDATE_WEATHERS].Reset();

        ///- Send an update signal to Weather objects
        WeatherMap::iterator itr, next;
        for (itr = m_weathers.begin(); itr != m_weathers.end(); itr = next)
        {
            next = itr;
            ++next;

            ///- and remove Weather objects for zones with no player
                                                            //As interval > WorldTick
            if(!itr->second->Update(m_timers[WUPDATE_WEATHERS].GetInterval()))
            {
                delete itr->second;
                m_weathers.erase(itr);
            }
        }
    }
    /// <li> Update uptime table
    if (m_timers[WUPDATE_UPTIME].Passed())
    {
        uint32 tmpDiff = GameTime::GetUptime();
        uint32 maxClientsNum = GetMaxActiveSessionCount();

        m_timers[WUPDATE_UPTIME].Reset();
        LoginDatabase.PExecute("UPDATE uptime SET uptime = %u, maxplayers = %u WHERE realmid = %u AND starttime = " UI64FMTD, tmpDiff, maxClientsNum, realm.Id.Realm, uint64(m_startTime));
    }

    ///- Update objects (maps, transport, creatures,...)
    sMapMgr->Update(diff);
    RecordTimeDiff("UpdateMapMgr");

    sBattlegroundMgr->Update(diff);
    RecordTimeDiff("UpdateBattlegroundMgr");

    sOutdoorPvPMgr->Update(diff);
    RecordTimeDiff("UpdateOutdoorPvPMgr");

#ifdef TESTS
    sTestMgr->Update(diff);
    RecordTimeDiff("UpdatesTestMgr");
#endif

    ///- Erase corpses once every 20 minutes
    if (m_timers[WUPDATE_CORPSES].Passed())
    {
        m_timers[WUPDATE_CORPSES].Reset();

        sMapMgr->DoForAllMaps([](Map* map)
        {
            map->RemoveOldCorpses();
        });
    }

    if (m_timers[WUPDATE_ARENASEASONLOG].Passed())
    {
        m_timers[WUPDATE_ARENASEASONLOG].Reset();

        UpdateArenaSeasonLogs();
    }

    // execute callbacks from sql queries that were queued recently
    ResetTimeDiffRecord();
    ProcessQueryCallbacks();
    RecordTimeDiff("ProcessQueryCallbacks");

    ///- Announce if a timer has passed
    if (m_timers[WUPDATE_ANNOUNCES].Passed())
    {
        m_timers[WUPDATE_ANNOUNCES].Reset();

        if (getConfig(CONFIG_AUTOANNOUNCE_ENABLED)) {
            time_t curTime = time(nullptr);
            for (auto & autoAnnounce : autoAnnounces) {
                if (autoAnnounce.second->nextAnnounce <= curTime) {
                    SendWorldText(LANG_AUTO_ANN, autoAnnounce.second->message.c_str());
                    autoAnnounce.second->nextAnnounce += DAY;
                }
            }
        }
    }

    ///- Process Game events when necessary
    if (m_timers[WUPDATE_EVENTS].Passed())
    {
        m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
        uint32 nextGameEvent = sGameEventMgr->Update();
        m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
        m_timers[WUPDATE_EVENTS].Reset();
    }

    ///- Ping to keep MySQL connections alive
    if (m_timers[WUPDATE_PINGDB].Passed())
    {
        m_timers[WUPDATE_PINGDB].Reset();
        TC_LOG_DEBUG("misc", "Ping MySQL to keep connection alive");
        CharacterDatabase.KeepAlive();
        LoginDatabase.KeepAlive();
        WorldDatabase.KeepAlive();
        LogsDatabase.KeepAlive();
    }

    // update the instance reset times
    sInstanceSaveMgr->Update();

    // Check for shutdown warning
    if (_guidWarn && !_guidAlert)
    {
        _warnDiff += diff;
        if (GameTime::GetGameTime() >= _warnShutdownTime)
            DoGuidWarningRestart();
        else if (_warnDiff > getIntConfig(CONFIG_RESPAWN_GUIDWARNING_FREQUENCY) * IN_MILLISECONDS)
            SendGuidWarning();
    }

    // And last, but not least handle the issued cli commands
    ProcessCliCommands();

    sMonitor->FinishedWorldLoop();
    sMonitor->Update(diff);

   // sScriptMgr->OnWorldUpdate(diff);
}

void World::ForceGameEventUpdate()
{
    m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
    uint32 nextGameEvent = sGameEventMgr->Update();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
    m_timers[WUPDATE_EVENTS].Reset();
}

/// Send a packet to all players (except self if mentioned)
void World::SendGlobalMessage(WorldPacket *packet, WorldSession *self, uint32 team)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team) )
        {
            itr->second->SendPacket(packet);
        }
    }
}

void World::SendGlobalGMMessage(WorldPacket *packet, WorldSession *self, uint32 team)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second != self &&
            itr->second->GetSecurity() >SEC_PLAYER &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team) )
        {
            itr->second->SendPacket(packet);
        }
    }
}

/// Send a System Message to all players (except self if mentioned)
void World::SendWorldText(int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for(auto & m_session : m_sessions)
    {
        if(!m_session.second || !m_session.second->GetPlayer() || !m_session.second->GetPlayer()->IsInWorld() )
            continue;

        LocaleConstant loc_idx = m_session.second->GetSessionDbcLocale();
        uint32 cache_idx = loc_idx+1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if(data_cache.size() < cache_idx+1 || data_cache[cache_idx].empty())
        {
            if(data_cache.size() < cache_idx+1)
                data_cache.resize(cache_idx+1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr->GetTrinityString(string_id,loc_idx);

            char buf[1000];

            va_list argptr;
            va_start( argptr, string_id );
            vsnprintf( buf,1000, text, argptr );
            va_end( argptr );

            char* pos = &buf[0];

            while(char* line = ChatHandler::LineFromMessage(pos))
            {
                auto  data = new WorldPacket();
                ChatHandler::BuildChatPacket(*data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for(auto & i : *data_list)
            m_session.second->SendPacket(i);
    }

    // free memory
    for(auto & i : data_cache)
        for(auto & j : i)
            delete j;
}

void World::SendGMText(int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for(auto & m_session : m_sessions)
    {
        if(!m_session.second || !m_session.second->GetPlayer() || !m_session.second->GetPlayer()->IsInWorld() )
            continue;

        LocaleConstant loc_idx = m_session.second->GetSessionDbcLocale();
        uint32 cache_idx = loc_idx+1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if(data_cache.size() < cache_idx+1 || data_cache[cache_idx].empty())
        {
            if(data_cache.size() < cache_idx+1)
                data_cache.resize(cache_idx+1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr->GetTrinityString(string_id,loc_idx);

            char buf[1000];

            va_list argptr;
            va_start( argptr, string_id );
            vsnprintf( buf,1000, text, argptr );
            va_end( argptr );

            char* pos = &buf[0];

            while(char* line = ChatHandler::LineFromMessage(pos))
            {
                auto  data = new WorldPacket();
                ChatHandler::BuildChatPacket(*data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for(auto & i : *data_list)
            if(m_session.second->GetSecurity() > SEC_PLAYER)
            m_session.second->SendPacket(i);
    }

    // free memory
    for(auto & i : data_cache)
        for(auto & j : i)
            delete j;
}

/// Send a System Message to all players (except self if mentioned)
void World::SendGlobalText(const char* text, WorldSession *self)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(text);
    char* pos = buf;

    while(char* line = ChatHandler::LineFromMessage(pos))
    {
        ChatHandler::BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        SendGlobalMessage(&data, self);
    }

    free(buf);
}

/// Send a packet to all players (or players selected team) in the zone (except self if mentioned)
void World::SendZoneMessage(uint32 zone, WorldPacket *packet, WorldSession *self, uint32 team)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second->GetPlayer()->GetZoneId() == zone &&
            itr->second != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team) )
        {
            itr->second->SendPacket(packet);
        }
    }
}

/// Send a System Message to all players in the zone (except self if mentioned)
void World::SendZoneText(uint32 zone, const char* text, WorldSession *self, uint32 team)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, text);
    SendZoneMessage(zone, &data, self,team);
}

/// Kick (and save) all players
void World::KickAll()
{
    m_QueuedPlayer.clear();                                 // prevent send queue update packet and login queued sessions

    // session not removed at kick and will removed in next update tick
    for (auto & m_session : m_sessions)
        m_session.second->KickPlayer();
}

/// Kick (and save) all players with security level less `sec`
void World::KickAllLess(AccountTypes sec)
{
    // session not removed at kick and will removed in next update tick
    for (auto & m_session : m_sessions)
        if(m_session.second->GetSecurity() < sec)
            m_session.second->KickPlayer();
}

/// Kick (and save) the designated player
bool World::KickPlayer(const std::string& playerName)
{
    SessionMap::iterator itr;

    // session not removed at kick and will removed in next update tick
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if(!itr->second)
            continue;
        Player *player = itr->second->GetPlayer();
        if(!player)
            continue;
        if( player->IsInWorld() )
        {
            if (playerName == player->GetName())
            {
                itr->second->KickPlayer();
                return true;
            }
        }
    }
    return false;
}

/// Ban an account or ban an IP address, duration is in seconds if positive, otherwise permban
BanReturn World::BanAccount(SanctionType mode, std::string const& _nameOrIP, uint32 duration_secs, std::string const& _reason, std::string const& author, WorldSession const* author_session)
{
    std::string nameOrIP(_nameOrIP);
    LoginDatabase.EscapeString(nameOrIP);
    std::string reason(_reason);
    LoginDatabase.EscapeString(reason);
    std::string safe_author=author;
    LoginDatabase.EscapeString(safe_author);

    //AccountTypes authorSecurity = author_session ? AccountTypes(author_session->GetSecurity()) : SEC_ADMINISTRATOR;

    QueryResult resultAccounts = nullptr;                     //used for kicking

//    uint32 authorGUID = sCharacterCache->GetCharacterGuidByName(safe_author);

    ///- Update the database with ban information
    switch(mode)
    {
        case SANCTION_BAN_IP:
            //No SQL injection as strings are escaped
            resultAccounts = LoginDatabase.PQuery("SELECT id FROM account WHERE last_ip = '%s'",nameOrIP.c_str());
            LoginDatabase.AsyncPQuery("INSERT INTO ip_banned VALUES ('%s',UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+%u,'%s','%s')",nameOrIP.c_str(),duration_secs,safe_author.c_str(),reason.c_str());
            break;
        case SANCTION_BAN_ACCOUNT:
            //No SQL injection as string is escaped
            resultAccounts = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'",nameOrIP.c_str());
            break;
        case SANCTION_BAN_CHARACTER:
            //No SQL injection as string is escaped
            resultAccounts = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name = '%s'",nameOrIP.c_str());
            break;
        default:
            return BAN_SYNTAX_ERROR;
    }

    if(!resultAccounts)
    {
        if(mode== SANCTION_BAN_IP)
            return BAN_SUCCESS;                             // ip correctly banned but nobody affected (yet)
        else
            return BAN_NOTFOUND;                                // Nobody to ban
    }

    ///- Disconnect all affected players (for IP it can be several)
    do
    {
        Field* fieldsAccount = resultAccounts->Fetch();
        uint32 account = fieldsAccount->GetUInt32();
        //also reset mail change
        LoginDatabase.AsyncPQuery("UPDATE account SET newMail = '', newMailTS = 0 WHERE id = %u",account);

        if(mode!= SANCTION_BAN_IP)
        {
            //No SQL injection as strings are escaped
            LoginDatabase.AsyncPQuery("INSERT INTO account_banned VALUES ('%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s', '1')",
                account,duration_secs,safe_author.c_str(),reason.c_str());
            LogsDatabaseAccessor::Sanction(author_session, account, 0, SANCTION_BAN_ACCOUNT, duration_secs, reason);
        }
        else {
            // Log ip ban for each account on that IP
            LogsDatabaseAccessor::Sanction(author_session, account, 0, SANCTION_BAN_IP, duration_secs, reason);
        }

        if (WorldSession* sess = FindSession(account))
            if(std::string(sess->GetPlayerName()) != author)
                sess->KickPlayer();
    }
    while( resultAccounts->NextRow() );

    return BAN_SUCCESS;
}

/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
//Todo: Drop "author" in favor of session
BanReturn World::BanAccount(SanctionType mode, std::string const& nameOrIP, std::string const& duration, std::string const& reason, std::string const& author, WorldSession const* author_session)
{
    uint32 duration_secs = TimeStringToSecs(duration);
    return BanAccount(mode, nameOrIP, duration_secs, reason, author, author_session);
}

/// Remove a ban from an account or IP address
bool World::RemoveBanAccount(SanctionType mode, std::string nameOrIP, WorldSession const* unbanAuthor)
{
    if (mode == SANCTION_BAN_IP)
    {
        LoginDatabase.EscapeString(nameOrIP);
        LoginDatabase.AsyncPQuery("DELETE FROM ip_banned WHERE ip = '%s'",nameOrIP.c_str());

        LogsDatabaseAccessor::RemoveSanction(unbanAuthor, 0, 0, nameOrIP, SANCTION_BAN_IP);
    }
    else
    {
        uint32 account = 0;
        if (mode == SANCTION_BAN_ACCOUNT)
            account = sAccountMgr->GetId (nameOrIP);
        else if (mode == SANCTION_BAN_CHARACTER)
            account = sCharacterCache->GetCharacterAccountIdByName (nameOrIP);

        if (!account)
            return false;

        //NO SQL injection as account is uint32
        LoginDatabase.AsyncPQuery("UPDATE account_banned SET active = '0' WHERE id = '%u'",account);
        LogsDatabaseAccessor::RemoveSanction(unbanAuthor, account, 0, "", mode);
    }
    return true;
}

/// Update the game time
void World::_UpdateGameTime()
{
    ///- update the time
    time_t lastGameTime = GameTime::GetGameTime();
    GameTime::UpdateGameTimers();

    uint32 elapsed = uint32(GameTime::GetGameTime() - lastGameTime);

    ///- if there is a shutdown timer
    if(!m_stopEvent && m_ShutdownTimer > 0 && elapsed > 0)
    {
        ///- ... and it is overdue, stop the world (set m_stopEvent)
        if( m_ShutdownTimer <= elapsed )
        {
            if(!(m_ShutdownMask & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount()==0)
                m_stopEvent = true;                         // exist code already set
            else
                m_ShutdownTimer = 1;                        // minimum timer value to wait idle state
        }
        ///- ... else decrease it and if necessary display a shutdown countdown to the users
        else
        {
            m_ShutdownTimer -= elapsed;
            ShutdownMsg(false, nullptr, m_ShutdownReason);
        }
    }
}

/// Shutdown the server
void World::ShutdownServ(uint32 time, uint32 options, uint8 exitcode, const std::string& reason)
{
    // ignore if server shutdown at next tick
    if(m_stopEvent)
        return;

    m_ShutdownMask = options;
    m_ShutdownReason = reason;
    m_ExitCode = exitcode;

    ///- If the shutdown time is 0, set m_stopEvent (except if shutdown is 'idle' with remaining sessions)
    if(time==0)
    {
        if(!(options & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount() == 0)
            m_stopEvent = true;                             // exist code already set
        else
            m_ShutdownTimer = 1;                            //So that the session count is re-evaluated at next world tick
    }
    ///- Else set the shutdown timer and warn users
    else
    {
        m_ShutdownTimer = time;
        ShutdownMsg(true, nullptr, m_ShutdownReason);
    }

    #ifdef PLAYERBOT
    sRandomPlayerbotMgr.LogoutAllBots();
    #endif

//    sScriptMgr->OnShutdownInitiate(ShutdownExitCode(exitcode), ShutdownMask(options));
}

/// Display a shutdown message to the user(s)
void World::ShutdownMsg(bool show, Player* player, const std::string& reason)
{
    // not show messages for idle shutdown mode
    if(m_ShutdownMask & SHUTDOWN_MASK_IDLE)
        return;

    ///- Display a message every 12 hours, hours, 5 minutes, minute, 5 seconds and finally seconds
    if ( show ||
        (m_ShutdownTimer < 10) ||
                                                            // < 30 sec; every 5 sec
        (m_ShutdownTimer<30        && (m_ShutdownTimer % 5         )==0) ||
                                                            // < 5 min ; every 1 min
        (m_ShutdownTimer<5*MINUTE  && (m_ShutdownTimer % MINUTE    )==0) ||
                                                            // < 30 min ; every 5 min
        (m_ShutdownTimer<30*MINUTE && (m_ShutdownTimer % (5*MINUTE))==0) ||
                                                            // < 12 h ; every 1 h
        (m_ShutdownTimer<12*HOUR   && (m_ShutdownTimer % HOUR      )==0) ||
                                                            // > 12 h ; every 12 h
        (m_ShutdownTimer>12*HOUR   && (m_ShutdownTimer % (12*HOUR) )==0))
    {
        std::string str = secsToTimeString(m_ShutdownTimer);

        uint32 msgid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? SERVER_MSG_RESTART_TIME : SERVER_MSG_SHUTDOWN_TIME;

        std::stringstream sst;
        std::string msgToSend;
        sst << str.c_str() << ": " << reason;
        msgToSend = sst.str();

        SendServerMessage(msgid, msgToSend.c_str(), player);
        TC_LOG_DEBUG("misc","Server is %s in %s",(m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shutting down"),str.c_str());
    }
}

/// Cancel a planned server shutdown
void World::ShutdownCancel()
{
    // nothing cancel or too later
    if(!m_ShutdownTimer || m_stopEvent)
        return;

    uint32 msgid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? SERVER_MSG_RESTART_CANCELLED : SERVER_MSG_SHUTDOWN_CANCELLED;

    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_ExitCode = SHUTDOWN_EXIT_CODE;                       // to default value
    SendServerMessage(msgid);

    TC_LOG_DEBUG("misc","Server %s cancelled.",(m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"));
}

/// Send a server message to the user(s)
void World::SendServerMessage(uint32 type, const char *text, Player* player)
{
    WorldPacket data(SMSG_SERVER_MESSAGE, 50);              // guess size
    data << uint32(type);
    if(type <= SERVER_MSG_STRING)
        data << text;

    if(player)
        player->SendDirectMessage(&data);
    else
        SendGlobalMessage( &data );
}

void World::UpdateSessions(uint32 diff)
{
    ///- Add new sessions
    WorldSession* sess = nullptr;
    while (addSessQueue.next(sess))
        AddSession_(sess);

    ///- Then send an update signal to remaining ones
    for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
    {
        next = itr;
        ++next;

        ///- and remove not active sessions from the list
        WorldSession* pSession = itr->second;
        WorldSessionFilter updater(pSession);

        if (!pSession->Update(diff, updater))    // As interval = 0
        {
            if (!RemoveQueuedPlayer(itr->second) && itr->second && getIntConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
                m_disconnects[itr->second->GetAccountId()] = time(nullptr);
            RemoveQueuedPlayer(pSession);
            m_sessions.erase(itr);
            delete pSession;

        }
    }
}

bool compareRank (ArenaTeam* first, ArenaTeam* second)
{
    return first->GetStats().rank < second->GetStats().rank;
}

// /!\ There can be multiple team with rank 1 and so on
void World::updateArenaLeaderTeams(uint8 maxcount, uint8 type, uint32 minimalRating)
{
    firstArenaTeams.clear();
    for (auto i = sObjectMgr->GetArenaTeamMapBegin(); i != sObjectMgr->GetArenaTeamMapEnd(); ++i)
    {
        if(ArenaTeam* team = i->second)
            if (team->GetType() == type && team->GetStats().rank != 0 && team->GetStats().rank <= maxcount && team->GetStats().rating > minimalRating)
                firstArenaTeams.push_back(team);
    }

    std::sort(firstArenaTeams.begin(), firstArenaTeams.end(), compareRank);
}

void World::updateArenaLeadersTitles()
{
    //get 3 first teams
    std::vector<ArenaTeam*> oldLeaderTeams = firstArenaTeams;
    if(sWorld->getConfig(CONFIG_ARENA_SEASON) != 0)
        updateArenaLeaderTeams(3,ARENA_TEAM_2v2,sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB_MIN_RATING));
    else // else we are in an interseason, leader teams are fixed and defined in conf file
    {
        return; //player title are updated at login
    }

    bool leadChanged = false;
    if(firstArenaTeams.size() != oldLeaderTeams.size())
        leadChanged = true;
    else {
        for(uint8 i = 0; i < firstArenaTeams.size(); i++)
            if(oldLeaderTeams[i] != firstArenaTeams[i])
            {
                leadChanged = true;
                break;
            }
    }

    if(!leadChanged)
        return; //nothing to do

    //update all online players
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& onlinePlayers = const_cast<HashMapHolder<Player>::MapType&>(ObjectAccessor::GetPlayers());
    for (auto itr : onlinePlayers)
    {
        if(itr.second)
            itr.second->UpdateArenaTitles();
    }
}

// This handles the issued and queued CLI commands
void World::ProcessCliCommands()
{
    CliCommandHolder::Print* zprint = nullptr;
    void* callbackArg = nullptr;
    CliCommandHolder* command = nullptr;
    while (cliCmdQueue.next(command))
    {
        TC_LOG_INFO("misc", "CLI command under processing...");
        zprint = command->m_print;
        callbackArg = command->m_callbackArg;
        CliHandler handler(callbackArg, zprint);
        handler.ParseCommands(command->m_command);
        if (command->m_commandFinished)
            command->m_commandFinished(callbackArg, !handler.HasSentErrorMessage());
        delete command;
    }
}

void World::UpdateRealmCharCount(uint32 accountId)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_COUNT);
    stmt->setUInt32(0, accountId);
    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&World::_UpdateRealmCharCount, this, std::placeholders::_1)));
}

void World::_UpdateRealmCharCount(PreparedQueryResult resultCharCount)
{
    if (resultCharCount)
    {
        Field* fields = resultCharCount->Fetch();
        uint32 accountId = fields[0].GetUInt32();
        uint8 charCount = uint8(fields[1].GetUInt64());

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM);
        stmt->setUInt32(0, accountId);
        stmt->setUInt32(1, realm.Id.Realm);
        LoginDatabase.Execute(stmt);

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
        stmt->setUInt8(0, charCount);
        stmt->setUInt32(1, accountId);
        stmt->setUInt32(2, realm.Id.Realm);
        LoginDatabase.Execute(stmt);
    }
}

void World::InitDailyQuestResetTime(bool loading)
{
    time_t mostRecentQuestTime = 0;

    if(loading)
    {
        QueryResult result = CharacterDatabase.Query("SELECT MAX(time) FROM character_queststatus_daily");
        if(result)
        {
            Field *fields = result->Fetch();
            mostRecentQuestTime = (time_t)fields[0].GetUInt64();
        }
    }

    // client built-in time for reset is 6:00 AM
    // FIX ME: client not show day start time
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    localTm.tm_hour = 6;
    localTm.tm_min  = 0;
    localTm.tm_sec  = 0;

    // current day reset time
    time_t curDayResetTime = mktime(&localTm);

    // last reset time before current moment
    time_t resetTime = (curTime < curDayResetTime) ? curDayResetTime - DAY : curDayResetTime;

    // need reset (if we have quest time before last reset time (not processed by some reason)
    if(mostRecentQuestTime && mostRecentQuestTime <= resetTime)
        m_NextDailyQuestReset = mostRecentQuestTime;
    else
    {
        // plan next reset time
        m_NextDailyQuestReset = (curTime >= curDayResetTime) ? curDayResetTime + DAY : curDayResetTime;
    }
}

void World::UpdateAllowedSecurity()
{
     QueryResult result = LoginDatabase.PQuery("SELECT allowedSecurityLevel from realmlist WHERE id = '%d'", realm.Id.Realm);
     if (result)
     {
        m_allowedSecurityLevel = AccountTypes(result->Fetch()->GetUInt16());
     }
}

void World::ResetDailyQuests()
{
    TC_LOG_DEBUG("misc","Daily quests reset for all characters.");

    // Every 1st of the month, delete data for quests 9884, 9885, 9886, 9887
    time_t curTime = time(nullptr);
    tm localTm = *localtime(&curTime);
    bool reinitConsortium = false;
    if (localTm.tm_mday == 1) {
        reinitConsortium = true;
        CharacterDatabase.AsyncQuery("DELETE FROM character_queststatus WHERE quest IN (9884, 9885, 9886, 9887)");
    }

    CharacterDatabase.AsyncQuery("DELETE FROM character_queststatus_daily");
    for(auto & m_session : m_sessions) {
        if(m_session.second->GetPlayer()) {
            m_session.second->GetPlayer()->ResetDailyQuestStatus();
            if (reinitConsortium) {
                m_session.second->GetPlayer()->SetQuestStatus(9884, QUEST_STATUS_NONE);
                m_session.second->GetPlayer()->SetQuestStatus(9885, QUEST_STATUS_NONE);
                m_session.second->GetPlayer()->SetQuestStatus(9886, QUEST_STATUS_NONE);
                m_session.second->GetPlayer()->SetQuestStatus(9887, QUEST_STATUS_NONE);
            }
        }
    }

    // change available dailies
    //TC sPoolMgr->ChangeDailyQuests();
}

void World::InitNewDataForQuestPools()
{
    TC_LOG_DEBUG("misc","Init new current quest in pools.");
    QueryResult result = WorldDatabase.PQuery("SELECT pool_id FROM quest_pool_current");
    if (!result) {
        TC_LOG_ERROR("misc","World::InitNewDataForQuestPools: No quest_pool found!");
        return;
    }

    do {
        Field* fields = result->Fetch();
        uint32 poolId = fields[0].GetUInt32();

        QueryResult resquests = WorldDatabase.PQuery("SELECT quest_id FROM quest_pool WHERE pool_id = %u", poolId);
        if (!resquests) {
            TC_LOG_ERROR("misc","World::InitNewDataForQuestPools: No quest in pool (%u)!", poolId);
            continue;
        }

        std::vector<uint32> questIds;
        do {
            Field* fieldquests = resquests->Fetch();
            uint32 questId = fieldquests[0].GetUInt32();
            if (questId)
                questIds.push_back(questId);
        } while (resquests->NextRow());

        uint32 randomIdx = rand()%questIds.size();
        uint32 chosenQuestId = questIds.at(randomIdx);
        WorldDatabase.PQuery("UPDATE quest_pool_current SET quest_id = %u WHERE pool_id = %u", chosenQuestId, poolId);
    } while (result->NextRow());

    LoadQuestPoolsData();
}

void World::SetPlayerLimit(int32 limit)
{
    m_playerLimit = limit;
}

void World::UpdateMaxSessionCounters()
{
    m_maxActiveSessionCount = std::max(m_maxActiveSessionCount,uint32(m_sessions.size()-m_QueuedPlayer.size()));
    m_maxQueuedSessionCount = std::max(m_maxQueuedSessionCount,uint32(m_QueuedPlayer.size()));
}

void World::LoadDBVersion()
{
    QueryResult result = WorldDatabase.Query("SELECT db_version, cache_id FROM version LIMIT 1");
    if(result)
    {
        Field* fields = result->Fetch();

        m_DBVersion = fields[0].GetString();
        // will be overwrite by config values if different and non-0
        m_configs[CONFIG_CLIENTCACHE_VERSION] = fields[1].GetUInt32();
    }
    else
        m_DBVersion = "unknown world database";
}

void World::LoadSanctuaryAndFFAZones()
{
    {
        configSanctuariesZones.clear();

        std::string zonestr = sConfigMgr->GetStringDefault("SanctuaryZone", "3703");
        std::vector<std::string> v;
        std::vector<std::string>::iterator it;
        std::string tempstr;

        int cutAt;
        tempstr = zonestr;
        while ((cutAt = tempstr.find_first_of(",")) != tempstr.npos) {
            if (cutAt > 0) {
                v.push_back(tempstr.substr(0, cutAt));
            }
            tempstr = tempstr.substr(cutAt + 1);
        }

        if (tempstr.length() > 1) {
            v.push_back(tempstr);
        }

        for (auto it : v) {
            uint32 zoneId = atoi(it.c_str());
            configSanctuariesZones.emplace(zoneId);
        }
    }

    {
        configFFAZones.clear();

        std::string zonestr = sConfigMgr->GetStringDefault("FFAZone", "");
        std::vector<std::string> v;
        std::vector<std::string>::iterator it;
        std::string tempstr;

        int cutAt;
        tempstr = zonestr;
        while ((cutAt = tempstr.find_first_of(",")) != tempstr.npos) {
            if (cutAt > 0) {
                v.push_back(tempstr.substr(0, cutAt));
            }
            tempstr = tempstr.substr(cutAt + 1);
        }

        if (tempstr.length() > 1) {
            v.push_back(tempstr);
        }

        for (auto it : v) {
            uint32 zoneId = atoi(it.c_str());
            configFFAZones.emplace(zoneId);
        }
    }
}

bool World::IsZoneSanctuary(uint32 zoneid) const
{
    auto itr = configSanctuariesZones.find(zoneid);
    return itr != configSanctuariesZones.end();
}

bool World::IsZoneFFA(uint32 zoneid) const
{
    auto itr = configFFAZones.find(zoneid);
    return itr != configFFAZones.end();
}

void World::LoadFishingWords()
{
    fishingWords.clear();

    std::string badstr = sConfigMgr->GetStringDefault("PhishingWords", "");
    std::vector<std::string>::iterator itr;
    std::string tempstr;
    int cutAt;

    if (badstr.length() == 0)
        return;

    tempstr = badstr;
    while ((cutAt = tempstr.find_first_of(",")) != tempstr.npos)
    {
        if (cutAt > 0)
            fishingWords.push_back(tempstr.substr(0, cutAt));

        tempstr = tempstr.substr(cutAt + 1);
    }

    if (tempstr.length() > 1)
        fishingWords.push_back(tempstr);
}

bool World::IsPhishing(std::string msg)
{
    for (auto itr : fishingWords) {
        if (msg.find(itr) != msg.npos)
            return true;
    }

    return false;
}

void World::LogPhishing(uint32 src, uint32 dst, std::string msg)
{
    std::string msgsafe = msg;
    LogsDatabase.EscapeString(msgsafe);
    LogsDatabase.AsyncPQuery("INSERT INTO phishing (srcguid, dstguid, time, data) VALUES ('%u', '%u', UNIX_TIMESTAMP(), '%s')", src, dst, msgsafe.c_str());
}

void World::LoadMotdAndTwitter()
{
    SetMotd(sConfigMgr->GetStringDefault("Motd", "Welcome to Sunstrider!"));
}

void World::LoadAutoAnnounce()
{
    QueryResult result = WorldDatabase.Query("SELECT id, message, hour, minute FROM auto_ann_by_time");
    if (!result)
        return;

    autoAnnounces.clear();

    uint32 count = 0;

    do {
        Field* fields = result->Fetch();

        auto  ann = new AutoAnnounceMessage;
        ann->message = fields[1].GetString();
        uint32 hour = fields[2].GetUInt32();
        uint32 mins = fields[3].GetUInt32();

        time_t curTime = time(nullptr);
        tm localTm = *localtime(&curTime);
        localTm.tm_hour = hour;
        localTm.tm_min  = mins;
        localTm.tm_sec  = 0;

        // current day reset time
        time_t curDayAnnounceTime = mktime(&localTm);

        ann->nextAnnounce = (curTime >= curDayAnnounceTime) ? curDayAnnounceTime + DAY : curDayAnnounceTime;

        autoAnnounces[fields[0].GetUInt32()] = ann;
        count++;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading","Loaded %u automatic announces.", count);
}

CharTitlesEntry const* World::getGladiatorTitle(uint8 rank)
{
    if(rank < 1 || rank > 3)
        return nullptr;

    uint8 id = 0;
    switch(rank)
    {
    case 1:   id = 42; break; // Gladiator
    case 2:   id = 62; break; // Merciless Gladiator
    case 3:   id = 71; break; // Vengeful Gladiator
    }

    return sCharTitlesStore.LookupEntry(id);
}

CharTitlesEntry const* World::getArenaLeaderTitle(uint8 rank)
{
    if(rank < 1 || rank > 3)
        return nullptr;

    uint8 id = 0;
    switch(rank)
    {
    case 1:   id = 45; break; // Compétiteur
    case 2:   id = 43; break; // Duelliste
    case 3:   id = 44; break; // Rival
    }

    return sCharTitlesStore.LookupEntry(id);
}

/*
Update arena_season_stats. This table keeps count of how much time a team kept a rank.
This should be called every minute.

Table structure :
teamid, time1, time2, time3
*/
void World::UpdateArenaSeasonLogs()
{
    for(uint8 i = 1; i <= 3; i++)
    {
        if(firstArenaTeams.size() < i)
            break;

        if (QueryResult result = LogsDatabase.PQuery("SELECT null FROM arena_season_stats WHERE teamid = %u;",firstArenaTeams[i-1]->GetId()))
        { //entry already exist
            LogsDatabase.PQuery("UPDATE arena_season_stats SET time%u = time%u + 1 WHERE teamid = %u;",i,i,firstArenaTeams[i-1]->GetId());
        } else { //else create a new one
            LogsDatabase.PQuery("REPLACE INTO arena_season_stats (teamid,time%u) VALUES (%u,1);",i,firstArenaTeams[i-1]->GetId());
        }
    }
}

void World::ProcessQueryCallbacks()
{
    _queryProcessor.ProcessReadyQueries();
}

void World::InvalidatePlayerDataToAllClient(uint64 guid)
{
    WorldPacket data(SMSG_INVALIDATE_PLAYER, 8);
    data << guid;
    SendGlobalMessage(&data);
}

void World::SendZoneUnderAttack(uint32 zoneId, Team team)
{
    WorldPacket data(SMSG_ZONE_UNDER_ATTACK,4);
    data << uint32(zoneId);
    SendGlobalMessage(&data,nullptr,team);
}

Realm realm;
