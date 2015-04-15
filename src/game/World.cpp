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

/** \file
    \ingroup world
*/

#include "Common.h"
#include "Memory.h"
#include "SystemConfig.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Weather.h"
#include "Player.h"
#include "SkillExtraItems.h"
#include "SkillDiscovery.h"
#include "World.h"
#include "AccountMgr.h"
#include "AuctionHouseMgr.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "LootMgr.h"
#include "ItemEnchantmentMgr.h"
#include "MapManager.h"
#include "ScriptCalls.h"
#include "CreatureAIRegistry.h"
#include "BattleGroundMgr.h"
#include "OutdoorPvPMgr.h"
#include "TemporarySummon.h"
#include "WaypointMovementGenerator.h"
#include "Management/VMapFactory.h"
#include "Management/MMapManager.h"
#include "GlobalEvents.h"
#include "GameEvent.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "InstanceSaveMgr.h"
#include "WaypointManager.h"
#include "Util.h"
#include "Language.h"
#include "CreatureGroups.h"
#include "Transport.h"
#include "CreatureTextMgr.h"
#include "ConditionMgr.h"
#include "SmartAI.h"
#include "WardenDataStorage.h"
#include "ArenaTeam.h"
#include "Management/MMapFactory.h"
#include "TransportMgr.h"
#include "ConfigMgr.h"
#include "ScriptMgr.h"
#include "AddonMgr.h"

volatile bool World::m_stopEvent = false;
uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
volatile uint32 World::m_worldLoopCounter = 0;

float World::m_MaxVisibleDistanceOnContinents = DEFAULT_VISIBILITY_DISTANCE;
float World::m_MaxVisibleDistanceInInstances  = DEFAULT_VISIBILITY_INSTANCE;
float World::m_MaxVisibleDistanceInBGArenas   = DEFAULT_VISIBILITY_BGARENAS;
float World::m_MaxVisibleDistanceForObject    = DEFAULT_VISIBILITY_DISTANCE;

float World::m_MaxVisibleDistanceInFlight     = DEFAULT_VISIBILITY_DISTANCE;
float World::m_VisibleUnitGreyDistance        = 0;
float World::m_VisibleObjectGreyDistance      = 0;

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME      = 1,
    SERVER_MSG_RESTART_TIME       = 2,
    SERVER_MSG_STRING             = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED = 4,
    SERVER_MSG_RESTART_CANCELLED  = 5
};

struct ScriptAction
{
    uint64 sourceGUID;
    uint64 targetGUID;
    uint64 ownerGUID;                                       // owner of source if source is item
    ScriptInfo const* script;                               // pointer to static script data
};

/// World constructor
World::World()
{
    m_playerLimit = 0;
    m_allowedSecurityLevel = SEC_PLAYER;
    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_gameTime=time(NULL);
    m_startTime=m_gameTime;
    m_maxActiveSessionCount = 0;
    m_maxQueuedSessionCount = 0;
    m_NextDailyQuestReset = 0;

    m_defaultDbcLocale = LOCALE_enUS;
    m_availableDbcLocaleMask = 0;

    m_updateTimeSum = 0;
    m_updateTimeCount = 0;
    m_updateTimeMon = 0;
    
    uint32 fastTdCount = 0;
    uint32 fastTdSum = 0;
    uint32 fastTd = 0;
    uint32 avgTdCount = 0;
    uint32 avgTdSum = 0;
    uint32 avgTd = 0;

    m_isClosed = false;
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
    for (WeatherMap::iterator itr = m_weathers.begin(); itr != m_weathers.end(); ++itr)
        delete itr->second;

    CliCommandHolder* command = NULL;
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
    return NULL;
}

bool World::IsClosed() const
{
    return m_isClosed;
}

void World::SetClosed(bool val)
{
    m_isClosed = val;

    // Invert the value, for simplicity for scripters.
    sScriptMgr->OnOpenStateChange(!val);
}

/// Find a session by its id
WorldSession* World::FindSession(uint32 id) const
{
    SessionMap::const_iterator itr = m_sessions.find(id);

    if(itr != m_sessions.end())
        return itr->second;                                 // also can return NULL for kicked session
    else
        return NULL;
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
    ///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
    SessionMap::iterator itr = m_sessions.find(id);

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
World::AddSession_ (WorldSession* s)
{
    PROFILE;
    
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
    uint32 QueueSize = GetQueueSize (); //number of players in the queue

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
    
    s->SendAuthResponse(AUTH_OK, true);
    s->SendAddonsInfo();
#ifdef LICH_KING
    s->SendClientCacheVersion(sWorld->getIntConfig(CONFIG_CLIENTCACHE_VERSION));
    s->SendTutorialsData();
#endif

    UpdateMaxSessionCounters ();

    // Updates the population
    if (pLimit > 0)
    {
        float popu = GetActiveSessionCount (); //updated number of users on the server
        popu /= pLimit;
        popu *= 2;
        LoginDatabase.PExecute ("UPDATE realmlist SET population = '%f' WHERE id = '%d'", popu, realmID);
    }
}

bool World::HasRecentlyDisconnected(WorldSession* session)
{
    if(!session) return false;

    if(uint32 tolerance = getConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
    {
        for(DisconnectMap::iterator i = m_disconnects.begin(); i != m_disconnects.end(); ++i)
        {
            if(difftime(i->second, time(NULL)) < tolerance)
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

    for(Queue::iterator iter = m_QueuedPlayer.begin(); iter != m_QueuedPlayer.end(); ++iter, ++position)
        if((*iter) == sess)
            return position;

    return 0;
}

void World::AddQueuedPlayer(WorldSession* sess)
{
    sess->SetInQueue(true);
    m_QueuedPlayer.push_back (sess);

    // The 1st SMSG_AUTH_RESPONSE needs to contain other info too.
    WorldPacket packet (SMSG_AUTH_RESPONSE, 1 + 4 + 1 + 4 + 1);
    packet << uint8 (AUTH_WAIT_QUEUE);
    packet << uint32 (0); // unknown random value...
    packet << uint8 (0);
    packet << uint32 (0);
    packet << uint8 (sess->Expansion () ? 1 : 0); // 0 - normal, 1 - TBC, must be set in database manually for each account
    packet << uint32(GetQueuePos (sess));
    sess->SendPacket (&packet);

    //sess->SendAuthWaitQue (GetQueuePos (sess));
}

bool World::RemoveQueuedPlayer(WorldSession* sess)
{
    // sessions count including queued to remove (if removed_session set)
    uint32 sessions = GetActiveSessionCount();

    uint32 position = 1;
    Queue::iterator iter = m_QueuedPlayer.begin();

    // search to remove and count skipped positions
    bool found = false;

    for(;iter != m_QueuedPlayer.end(); ++iter, ++position)
    {
        if(*iter==sess)
        {
            sess->SetInQueue(false);
            sess->ResetTimeOutTime();
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
        pop_sess->SetInQueue(false);
        pop_sess->ResetTimeOutTime();
        pop_sess->SendAuthWaitQue(0);
        pop_sess->SendAddonsInfo();

#ifdef LICH_KING
        pop_sess->SendClientCacheVersion(sWorld->getIntConfig(CONFIG_CLIENTCACHE_VERSION));
        pop_sess->SendAccountDataTimes(GLOBAL_CACHE_MASK);
        pop_sess->SendTutorialsData();
#endif

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
    WeatherMap::const_iterator itr = m_weathers.find(id);

    if(itr != m_weathers.end())
        return itr->second;
    else
        return 0;
}

/// Remove a Weather object for the given zoneid
void World::RemoveWeather(uint32 id)
{
    // not called at the moment. Kept for completeness
    WeatherMap::iterator itr = m_weathers.find(id);

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
        return NULL;

    Weather* w = new Weather(zone_id,weatherChances);
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

    ///- Read the player limit and the Message of the day from the config file
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

    rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = sConfigMgr->GetFloatDefault("TargetPosRecalculateRange",1.5f);
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
    m_configs[CONFIG_INTERVAL_SAVE] = sConfigMgr->GetIntDefault("PlayerSaveInterval", 900000);
    m_configs[CONFIG_INTERVAL_DISCONNECT_TOLERANCE] = sConfigMgr->GetIntDefault("DisconnectToleranceInterval", 0);

    m_configs[CONFIG_INTERVAL_GRIDCLEAN] = sConfigMgr->GetIntDefault("GridCleanUpDelay", 300000);
    if(m_configs[CONFIG_INTERVAL_GRIDCLEAN] < MIN_GRID_DELAY)
    {
        TC_LOG_ERROR("server.loading","GridCleanUpDelay (%i) must be greater %u. Use this minimal value.",m_configs[CONFIG_INTERVAL_GRIDCLEAN],MIN_GRID_DELAY);
        m_configs[CONFIG_INTERVAL_GRIDCLEAN] = MIN_GRID_DELAY;
    }
    if(reload)
        sMapMgr->SetGridCleanUpDelay(m_configs[CONFIG_INTERVAL_GRIDCLEAN]);

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
        uint32 val = sConfigMgr->GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT);
        if(val!=m_configs[CONFIG_PORT_WORLD])
            TC_LOG_ERROR("server.loading","WorldServerPort option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[CONFIG_PORT_WORLD]);
    }
    else
        m_configs[CONFIG_PORT_WORLD] = sConfigMgr->GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT);

    m_configs[CONFIG_SOCKET_TIMEOUTTIME] = sConfigMgr->GetIntDefault("SocketTimeOutTime", 900000);

    if(reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("SocketSelectTime", DEFAULT_SOCKET_SELECT_TIME);
        if(val!=m_configs[CONFIG_SOCKET_SELECTTIME])
            TC_LOG_ERROR("server.loading","SocketSelectTime option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[DEFAULT_SOCKET_SELECT_TIME]);
    }
    else
        m_configs[CONFIG_SOCKET_SELECTTIME] = sConfigMgr->GetIntDefault("SocketSelectTime", DEFAULT_SOCKET_SELECT_TIME);

    m_configs[CONFIG_GROUP_XP_DISTANCE] = sConfigMgr->GetIntDefault("MaxGroupXPDistance", 74);
    /// \todo Add MonsterSight and GuarderSight (with meaning) in worldserver.conf or put them as define
    m_configs[CONFIG_SIGHT_MONSTER] = sConfigMgr->GetIntDefault("MonsterSight", 50);
    m_configs[CONFIG_SIGHT_GUARDER] = sConfigMgr->GetIntDefault("GuarderSight", 50);

    if(reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("GameType", 0);
        if(val!=m_configs[CONFIG_GAME_TYPE])
            TC_LOG_ERROR("server.loading","GameType option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[CONFIG_GAME_TYPE]);
    }
    else
        m_configs[CONFIG_GAME_TYPE] = sConfigMgr->GetIntDefault("GameType", 0);

    if(reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);
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
    m_configs[CONFIG_ALLOW_TWO_SIDE_TRADE] = sConfigMgr->GetBoolDefault("AllowTwoSide.trade", false);
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
        uint32 val = sConfigMgr->GetIntDefault("MaxPlayerLevel", 60);
        if(val!=m_configs[CONFIG_MAX_PLAYER_LEVEL])
            TC_LOG_ERROR("server.loading","MaxPlayerLevel option can't be changed at config reload, using current value (%u).",m_configs[CONFIG_MAX_PLAYER_LEVEL]);
    }
    else
        m_configs[CONFIG_MAX_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("MaxPlayerLevel", 60);

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
            pvp_ranks[i] = atoi(strtok (NULL, ","));
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
    for (int i = 0; i < gladtokens.size(); i++)
    {
        Tokens subTokens = StrSplit(gladtokens[i], " ");
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

    m_configs[CONFIG_CAST_UNSTUCK] = sConfigMgr->GetBoolDefault("CastUnstuck", true);
    m_configs[CONFIG_INSTANCE_RESET_TIME_HOUR]  = sConfigMgr->GetIntDefault("Instance.ResetTimeHour", 4);
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
    m_configs[CONFIG_GM_LOG_TRADE]         = sConfigMgr->GetBoolDefault("GM.LogTrade", false);
    m_configs[CONFIG_START_GM_LEVEL]       = sConfigMgr->GetIntDefault("GM.StartLevel", 1);
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

    m_configs[CONFIG_GROUP_VISIBILITY] = sConfigMgr->GetIntDefault("Visibility.GroupMode",0);

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

    m_configs[CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY] = sConfigMgr->GetBoolDefault("SaveRespawnTimeImmediately",true);
    m_configs[CONFIG_WEATHER] = sConfigMgr->GetBoolDefault("ActivateWeather",true);

    m_configs[CONFIG_ALWAYS_MAX_SKILL_FOR_LEVEL] = sConfigMgr->GetBoolDefault("AlwaysMaxSkillForLevel", false);

    if(reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("Expansion",1);
        if(val!=m_configs[CONFIG_EXPANSION])
            TC_LOG_ERROR("server.loading","Expansion option can't be changed at worldserver.conf reload, using current value (%u).",m_configs[CONFIG_EXPANSION]);
    }
    else
        m_configs[CONFIG_EXPANSION] = sConfigMgr->GetIntDefault("Expansion",1);

    m_configs[CONFIG_CHATFLOOD_MESSAGE_COUNT] = sConfigMgr->GetIntDefault("ChatFlood.MessageCount",10);
    m_configs[CONFIG_CHATFLOOD_MESSAGE_DELAY] = sConfigMgr->GetIntDefault("ChatFlood.MessageDelay",1);
    m_configs[CONFIG_CHATFLOOD_MUTE_TIME]     = sConfigMgr->GetIntDefault("ChatFlood.MuteTime",10);

    m_configs[CONFIG_EVENT_ANNOUNCE] = sConfigMgr->GetIntDefault("Event.Announce",0);

    m_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS] = sConfigMgr->GetIntDefault("CreatureFamilyAssistanceRadius",10);
    m_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY]  = sConfigMgr->GetIntDefault("CreatureFamilyAssistanceDelay",1500);
    m_configs[CONFIG_CREATURE_FAMILY_FLEE_DELAY] = sConfigMgr->GetIntDefault("CreatureFamilyFleeDelay",7000);
    m_configs[CONFIG_CREATURE_MAX_UNREACHABLE_TARGET_TIME] = sConfigMgr->GetIntDefault("CreatureMaxUnreachableTargetTime",10000);

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
    m_configs[CONFIG_CHAT_FAKE_MESSAGE_PREVENTING] = sConfigMgr->GetBoolDefault("ChatFakeMessagePreventing", false);

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
    m_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS] = sConfigMgr->GetBoolDefault("Arena.AutoDistributePoints", false);
    m_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS] = sConfigMgr->GetIntDefault("Arena.AutoDistributeInterval", 7);

    m_configs[CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER] = sConfigMgr->GetIntDefault("Battleground.PrematureFinishTimer", 0);
    m_configs[CONFIG_BATTLEGROUND_TIMELIMIT_WARSONG] = sConfigMgr->GetIntDefault("Battleground.TimeLimit.Warsong", 0);
    m_configs[CONFIG_BATTLEGROUND_TIMELIMIT_ARENA] = sConfigMgr->GetIntDefault("Battleground.TimeLimit.Arena", 0);

    m_configs[CONFIG_INSTANT_LOGOUT] = sConfigMgr->GetIntDefault("InstantLogout", SEC_GAMEMASTER1);
    
    m_configs[CONFIG_GROUPLEADER_RECONNECT_PERIOD] = sConfigMgr->GetIntDefault("GroupLeaderReconnectPeriod", 180);

    m_VisibleUnitGreyDistance = sConfigMgr->GetFloatDefault("Visibility.Distance.Grey.Unit", 1);
    if(m_VisibleUnitGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Grey.Unit can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_VisibleUnitGreyDistance = MAX_VISIBILITY_DISTANCE;
    }
    m_VisibleObjectGreyDistance = sConfigMgr->GetFloatDefault("Visibility.Distance.Grey.Object", 10);
    if(m_VisibleObjectGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Grey.Object can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_VisibleObjectGreyDistance = MAX_VISIBILITY_DISTANCE;
    }

    //visibility on continents
    m_MaxVisibleDistanceOnContinents      = sConfigMgr->GetFloatDefault("Visibility.Distance.Continents",     DEFAULT_VISIBILITY_DISTANCE);
    if (m_MaxVisibleDistanceOnContinents < 45*sWorld->GetRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Continents can't be less max aggro radius %f", 45*sWorld->GetRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceOnContinents = 45*sWorld->GetRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceOnContinents + m_VisibleUnitGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Continents can't be greater %f",MAX_VISIBILITY_DISTANCE - m_VisibleUnitGreyDistance);
        m_MaxVisibleDistanceOnContinents = MAX_VISIBILITY_DISTANCE - m_VisibleUnitGreyDistance;
    }

    //visibility in instances
    m_MaxVisibleDistanceInInstances        = sConfigMgr->GetFloatDefault("Visibility.Distance.Instances",       DEFAULT_VISIBILITY_INSTANCE);
    if (m_MaxVisibleDistanceInInstances < 45*sWorld->GetRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Instances can't be less max aggro radius %f",45*sWorld->GetRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInInstances = 45*sWorld->GetRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInInstances + m_VisibleUnitGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Instances can't be greater %f",MAX_VISIBILITY_DISTANCE - m_VisibleUnitGreyDistance);
        m_MaxVisibleDistanceInInstances = MAX_VISIBILITY_DISTANCE - m_VisibleUnitGreyDistance;
    }

    //visibility in BG/Arenas
    m_MaxVisibleDistanceInBGArenas        = sConfigMgr->GetFloatDefault("Visibility.Distance.BGArenas",       DEFAULT_VISIBILITY_BGARENAS);
    if (m_MaxVisibleDistanceInBGArenas < 45*sWorld->GetRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.BGArenas can't be less max aggro radius %f",45*sWorld->GetRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInBGArenas = 45*sWorld->GetRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInBGArenas + m_VisibleUnitGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.BGArenas can't be greater %f",MAX_VISIBILITY_DISTANCE - m_VisibleUnitGreyDistance);
        m_MaxVisibleDistanceInBGArenas = MAX_VISIBILITY_DISTANCE - m_VisibleUnitGreyDistance;
    }

    m_MaxVisibleDistanceForObject    = sConfigMgr->GetFloatDefault("Visibility.Distance.Object",   DEFAULT_VISIBILITY_DISTANCE);
    if(m_MaxVisibleDistanceForObject < INTERACTION_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Object can't be less max aggro radius %f",float(INTERACTION_DISTANCE));
        m_MaxVisibleDistanceForObject = INTERACTION_DISTANCE;
    }
    else if(m_MaxVisibleDistanceForObject + m_VisibleObjectGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.Object can't be greater %f",MAX_VISIBILITY_DISTANCE-m_VisibleObjectGreyDistance);
        m_MaxVisibleDistanceForObject = MAX_VISIBILITY_DISTANCE - m_VisibleObjectGreyDistance;
    }

    m_MaxVisibleDistanceInFlight    = sConfigMgr->GetFloatDefault("Visibility.Distance.InFlight",      DEFAULT_VISIBILITY_DISTANCE);
    if(m_MaxVisibleDistanceInFlight + m_VisibleObjectGreyDistance > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading","Visibility.Distance.InFlight can't be greater %f",MAX_VISIBILITY_DISTANCE-m_VisibleObjectGreyDistance);
        m_MaxVisibleDistanceInFlight = MAX_VISIBILITY_DISTANCE - m_VisibleObjectGreyDistance;
    }

    ///- Read the "Data" directory from the config file
    std::string dataPath = sConfigMgr->GetStringDefault("DataDir","./");
    if( dataPath.at(dataPath.length()-1)!='/' && dataPath.at(dataPath.length()-1)!='\\' )
        dataPath.append("/");

    if(reload)
    {
        if(dataPath!=m_dataPath)
            TC_LOG_ERROR("server.loading","DataDir option can't be changed at worldserver.conf reload, using current value (%s).",m_dataPath.c_str());
    }
    else
    {
        m_dataPath = dataPath;
        TC_LOG_INFO("server.loading","Using DataDir %s",m_dataPath.c_str());
    }

    m_configs[CONFIG_VMAP_INDOOR_CHECK] = sConfigMgr->GetBoolDefault("vmap.enableIndoorCheck", true);
    m_configs[CONFIG_VMAP_INDOOR_INST_CHECK] = sConfigMgr->GetBoolDefault("vmap.enableIndoorCheckInstanceOnly", false);
    bool enableLOS = sConfigMgr->GetBoolDefault("vmap.enableLOS", true);
    bool enableHeight = sConfigMgr->GetBoolDefault("vmap.enableHeight", true);
    std::string ignoreMapIds = sConfigMgr->GetStringDefault("vmap.ignoreMapIds", "");
    VMAP::VMapFactory::createOrGetVMapManager()->setEnableLineOfSightCalc(enableLOS);
    VMAP::VMapFactory::createOrGetVMapManager()->setEnableHeightCalc(enableHeight);
    TC_LOG_INFO("server.loading", "WORLD: VMap support included. LineOfSight:%i, getHeight:%i",enableLOS, enableHeight);
    TC_LOG_INFO("server.loading", "WORLD: VMap data directory is: %svmaps",m_dataPath.c_str());
    TC_LOG_INFO("server.loading", "WORLD: VMap config keys are: vmap.enableLOS, vmap.enableHeight, vmap.ignoreMapIds");
    
    m_configs[CONFIG_BOOL_MMAP_ENABLED] = sConfigMgr->GetBoolDefault("mmap.enabled", 1);
    std::string mmapIgnoreMapIds = sConfigMgr->GetStringDefault("mmap.ignoreMapIds", "");
    MMAP::MMapFactory::preventPathfindingOnMaps(mmapIgnoreMapIds.c_str());
    TC_LOG_INFO("server.loading", "WORLD: mmap pathfinding %sabled", getConfig(CONFIG_BOOL_MMAP_ENABLED) ? "en" : "dis");

    m_configs[CONFIG_PET_LOS] = sConfigMgr->GetBoolDefault("vmap.petLOS", true);
    
    m_configs[CONFIG_PREMATURE_BG_REWARD] = sConfigMgr->GetBoolDefault("Battleground.PrematureReward", true);
    m_configs[CONFIG_START_ALL_SPELLS] = sConfigMgr->GetBoolDefault("PlayerStart.AllSpells", false);
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
    m_configs[CONFIG_NUMTHREADS] = sConfigMgr->GetIntDefault("MapUpdate.Threads",1);
    
    m_configs[CONFIG_WORLDCHANNEL_MINLEVEL] = sConfigMgr->GetIntDefault("WorldChannel.MinLevel", 10);
    
    m_configs[CONFIG_MAX_AVERAGE_TIMEDIFF] = sConfigMgr->GetIntDefault("World.MaxAverage.TimeDiff", 420);

    m_configs[CONFIG_MONITORING_ENABLED] = sConfigMgr->GetBoolDefault("Monitor.enabled", false);
    m_configs[CONFIG_MONITORING_UPDATE] = sConfigMgr->GetIntDefault("Monitor.update", 20000);

    std::string forbiddenmaps = sConfigMgr->GetStringDefault("ForbiddenMaps", "");
    char * forbiddenMaps = new char[forbiddenmaps.length() + 1];
    forbiddenMaps[forbiddenmaps.length()] = 0;
    strncpy(forbiddenMaps, forbiddenmaps.c_str(), forbiddenmaps.length());
    const char * delim = ",";
    char * token = strtok(forbiddenMaps, delim);
    while(token != NULL)
    {
        int32 mapid = strtol(token, NULL, 10);
        m_forbiddenMapIds.insert(mapid);
        token = strtok(NULL,delim);
    }
    delete[] forbiddenMaps;
    
    m_configs[CONFIG_PLAYER_GENDER_CHANGE_DELAY]    = sConfigMgr->GetIntDefault("Player.Change.Gender.Delay", 14);
    

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

    m_configs[CONFIG_GUIDDISTRIB_NEWMETHOD] = sConfigMgr->GetBoolDefault("GuidDistribution.NewMethod", true);
    m_configs[CONFIG_GUIDDISTRIB_PROPORTION] = sConfigMgr->GetIntDefault("GuidDistribution.Proportion", 90);

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

    m_configs[CONFIG_IRC_ENABLED] = sConfigMgr->GetBoolDefault("IRC.Enabled", false);
    m_configs[CONFIG_IRC_COMMANDS] = sConfigMgr->GetBoolDefault("IRC.Commands", false);
    
    //packet spoof punishment
    m_configs[CONFIG_PACKET_SPOOF_POLICY] = sConfigMgr->GetIntDefault("PacketSpoof.Policy", (uint32)WorldSession::DosProtection::POLICY_KICK);
    m_configs[CONFIG_PACKET_SPOOF_BANMODE] = sConfigMgr->GetIntDefault("PacketSpoof.BanMode", (uint32)BAN_ACCOUNT);
    if (m_configs[CONFIG_PACKET_SPOOF_BANMODE] == BAN_CHARACTER || m_configs[CONFIG_PACKET_SPOOF_BANMODE] > BAN_IP)
        m_configs[CONFIG_PACKET_SPOOF_BANMODE] = BAN_ACCOUNT;

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
    m_configs[CONFIG_ARENASERVER_USE_CLOSESCHEDULE] = sConfigMgr->GetBoolDefault("ArenaServer.UseCloseSchedule", true);
    m_configs[CONFIG_ARENASERVER_PLAYER_REPARTITION_THRESHOLD] = sConfigMgr->GetIntDefault("ArenaServer.PlayerRepartitionThreshold", 0);

    m_configs[CONFIG_DEBUG_DISABLE_MAINHAND] = sConfigMgr->GetBoolDefault("Debug.DisableMainHand", 0);
    m_configs[CONFIG_DEBUG_DISABLE_ARMOR] = sConfigMgr->GetBoolDefault("Debug.DisableArmor", 0);
    m_configs[CONFIG_DEBUG_LOG_LAST_PACKETS] = sConfigMgr->GetBoolDefault("Debug.LogLastPackets", 0);
    m_configs[CONFIG_DEBUG_LOG_ALL_PACKETS] = sConfigMgr->GetBoolDefault("Debug.LogAllPackets", 0);
    m_configs[CONFIG_DEBUG_DISABLE_CREATURES_LOADING] = sConfigMgr->GetBoolDefault("Debug.DisableCreaturesLoading", 0);
    

    m_configs[CONFIG_ARMORY_ENABLE] = sConfigMgr->GetBoolDefault("Armory.Enable", true);

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

    // call ScriptMgr if we're reloading the configuration
    if (reload)
        sScriptMgr->OnConfigLoad(reload);
}

extern void LoadGameObjectModelList();

/// Initialize the World
void World::SetInitialWorldSettings()
{
    ///- Initialize start time
    uint32 serverStartingTime = GetMSTime();

    ///- Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    ///- Initialize detour memory management
    dtAllocSetCustom(dtCustomAlloc, dtCustomFree);

    ///- Initialize config settings
    LoadConfigSettings();
    
    ///- Initialize motd and twitter
    LoadMotdAndTwitter();

    ///- Init highest guids before any table loading to prevent using not initialized guids in some code.
    sObjectMgr->SetHighestGuids();

    ///- Check the existence of the map files for all races' startup areas.
    if(   !MapManager::ExistMapAndVMap(0,-6240.32f, 331.033f)
        ||!MapManager::ExistMapAndVMap(0,-8949.95f,-132.493f)
        ||!MapManager::ExistMapAndVMap(0,-8949.95f,-132.493f)
        ||!MapManager::ExistMapAndVMap(1,-618.518f,-4251.67f)
        ||!MapManager::ExistMapAndVMap(0, 1676.35f, 1677.45f)
        ||!MapManager::ExistMapAndVMap(1, 10311.3f, 832.463f)
        ||!MapManager::ExistMapAndVMap(1,-2917.58f,-257.98f)
        ||m_configs[CONFIG_EXPANSION] && (
        !MapManager::ExistMapAndVMap(530,10349.6f,-6357.29f) || !MapManager::ExistMapAndVMap(530,-3961.64f,-13931.2f) ) )
    {
        TC_LOG_ERROR("server.loading", "Correct *.map files not found in path '%smaps' or *.vmap/*vmdir files in '%svmaps'. Please place *.map/*.vmap/*.vmdir files in appropriate directories or correct the DataDir value in the worldserver.conf file.", m_dataPath.c_str(), m_dataPath.c_str());
        exit(1);
    }

    ///- Loading strings. Getting no records means core load has to be canceled because no error message can be output.
    TC_LOG_INFO("server.loading", "" );
    TC_LOG_INFO("server.loading", "Loading Trinity strings..." );
    if (!sObjectMgr->LoadTrinityStrings())
        exit(1);                                            // Error message displayed in function already

    ///- Update the realm entry in the database with the realm type from the config file
    //No SQL injection as values are treated as integers

    // not send custom type REALM_FFA_PVP to realm list
    uint32 server_type = IsFFAPvPRealm() ? REALM_TYPE_PVP : getConfig(CONFIG_GAME_TYPE);
    uint32 realm_zone = getConfig(CONFIG_REALM_ZONE);
    LoginDatabase.PExecute("UPDATE realmlist SET icon = %u, timezone = %u WHERE id = '%d'", server_type, realm_zone, realmID);

    ///- Remove the bones after a restart
    CharacterDatabase.PExecute("DELETE FROM corpse WHERE corpse_type = '0'");

    ///- Load the DBC files
    TC_LOG_INFO("server.loading","Initialize data stores...");
    LoadDBCStores(m_dataPath);
    DetectDBCLang();
    TC_LOG_INFO("server.loading"," ");

    TC_LOG_INFO("server.loading","Loading Spell templates...");
    sObjectMgr->LoadSpellTemplates();

    TC_LOG_INFO("server.loading", "Loading Script Names...");
    sObjectMgr->LoadScriptNames();

    TC_LOG_INFO("server.loading", "Loading InstanceTemplate" );
    sObjectMgr->LoadInstanceTemplate();
    sObjectMgr->LoadInstanceTemplateAddon();

    TC_LOG_INFO("server.loading", "Loading SkillLineAbilityMultiMap Data..." );
    sSpellMgr->LoadSkillLineAbilityMap();

    ///- Clean up and pack instances
    TC_LOG_INFO("server.loading", "Cleaning up instances..." );
    sInstanceSaveManager.CleanupInstances();                              // must be called before `creature_respawn`/`gameobject_respawn` tables

//    TC_LOG_INFO("server.loading", "Packing instances..." );
//    sInstanceSaveManager.PackInstances();

    TC_LOG_INFO("server.loading", "Loading Localization strings..." );
    sObjectMgr->LoadCreatureLocales();
    sObjectMgr->LoadGameObjectLocales();
    sObjectMgr->LoadItemLocales();
    sObjectMgr->LoadQuestLocales();
    sObjectMgr->LoadNpcTextLocales();
    sObjectMgr->LoadPageTextLocales();
    sObjectMgr->LoadNpcOptionLocales();
    sObjectMgr->SetDBCLocaleIndex(GetDefaultDbcLocale());        // Get once for all the locale index of DBC language (console/broadcasts)

    TC_LOG_INFO("server.loading", "Loading Page Texts..." );
    sObjectMgr->LoadPageTexts();

    TC_LOG_INFO("server.loading", "Loading Game Object Templates..." );   // must be after LoadPageTexts
    sObjectMgr->LoadGameObjectTemplate();

    TC_LOG_INFO("server.loading", "Loading Spell Chain Data..." );
    sSpellMgr->LoadSpellChains();

    TC_LOG_INFO("server.loading", "Loading Spell Required Data..." );
    sSpellMgr->LoadSpellRequired();

    TC_LOG_INFO("server.loading", "Loading Spell Elixir types..." );
    sSpellMgr->LoadSpellElixirs();

    TC_LOG_INFO("server.loading", "Loading Spell Learn Skills..." );
    sSpellMgr->LoadSpellLearnSkills();                        // must be after LoadSpellChains

    TC_LOG_INFO("server.loading", "Loading Spell Learn Spells..." );
    sSpellMgr->LoadSpellLearnSpells();

    TC_LOG_INFO("server.loading", "Loading Spell Proc Event conditions..." );
    sSpellMgr->LoadSpellProcEvents();

    TC_LOG_INFO("server.loading", "Loading Aggro Spells Definitions...");
    sSpellMgr->LoadSpellThreats();

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

    TC_LOG_INFO("server.loading", "Loading Equipment templates...");
    sObjectMgr->LoadEquipmentTemplates();

    TC_LOG_INFO("server.loading", "Loading Creature templates..." );
    sObjectMgr->LoadCreatureTemplates();

    TC_LOG_INFO("server.loading", "Loading Creature template addons...");
    sObjectMgr->LoadCreatureTemplateAddons();

    TC_LOG_INFO("server.loading", "Loading SpellsScriptTarget...");
    sSpellMgr->LoadSpellScriptTarget();                       // must be after LoadCreatureTemplates and LoadGameObjectTemplate
    
    TC_LOG_INFO("server.loading","Loading Spell scripts...");
    sObjectMgr->LoadSpellScriptsNew();

    TC_LOG_INFO("server.loading", "Loading Creature Reputation OnKill Data..." );
    sObjectMgr->LoadReputationOnKill();

    TC_LOG_INFO("server.loading", "Loading Pet Create Spells..." );
    sObjectMgr->LoadPetCreateSpells();

    TC_LOG_INFO("server.loading", "Loading Creature Base Stats...");
    sObjectMgr->LoadCreatureClassLevelStats();

    if(!getConfig(CONFIG_DEBUG_DISABLE_CREATURES_LOADING))
    {
        TC_LOG_INFO("server.loading", "Loading Creature Data..." );
        sObjectMgr->LoadCreatures();
    
        TC_LOG_INFO("server.loading", "Loading Creature Linked Respawn..." );
        sObjectMgr->LoadCreatureLinkedRespawn();                     // must be after LoadCreatures()

        TC_LOG_INFO("server.loading", "Loading Creature Addon Data..." );
        sObjectMgr->LoadCreatureAddons();                            // must be after LoadCreatureTemplates() and LoadCreatures()

        TC_LOG_INFO("server.loading", "Loading Creature Respawn Data..." );   // must be after PackInstances()
        sObjectMgr->LoadCreatureRespawnTimes();
    }

    TC_LOG_INFO("server.loading", "Loading Gameobject Data..." );
    sObjectMgr->LoadGameobjects();

    TC_LOG_INFO("server.loading","Loading Transport templates...");
    sTransportMgr->LoadTransportTemplates();

    TC_LOG_INFO("server.loading", "Loading Gameobject Respawn Data..." ); // must be after PackInstances()
    sObjectMgr->LoadGameobjectRespawnTimes();

    TC_LOG_INFO("server.loading", "Loading Game Event Data...");
    gameeventmgr.LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Weather Data..." );
    sObjectMgr->LoadWeatherZoneChances();

    TC_LOG_INFO("server.loading", "Loading spell extra attributes..." );
    sSpellMgr->LoadSpellCustomAttr();
    
    TC_LOG_INFO("server.loading","Loading GameObject models...");
    LoadGameObjectModelList();
    
    TC_LOG_INFO("server.loading","Loading overriden area flags data...");
    sObjectMgr->LoadAreaFlagsOverridenData();

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

    TC_LOG_INFO("server.loading", "Loading linked spells..." );
    sSpellMgr->LoadSpellLinked();

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

    TC_LOG_INFO("server.loading", "Loading Player Corpses..." );
    sObjectMgr->LoadCorpses();

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
    sAHMgr.LoadAuctionItems();
    sAHMgr.LoadAuctions();

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

    TC_LOG_INFO("server.loading", "Loading Npc Text Id..." );
    sObjectMgr->LoadNpcTextId();                                 // must be after load Creature and NpcText

    TC_LOG_INFO("server.loading", "Loading Npc Options..." );
    sObjectMgr->LoadNpcOptions();

    TC_LOG_INFO("server.loading", "Loading vendors..." );
    sObjectMgr->LoadVendors();                                   // must be after load CreatureTemplate and ItemTemplate

    TC_LOG_INFO("server.loading", "Loading trainers..." );
    sObjectMgr->LoadTrainerSpell();                              // must be after load CreatureTemplate

    TC_LOG_INFO("server.loading", "Loading Waypoints..." );
    sWaypointMgr->Load();
    
    TC_LOG_INFO("server.loading","Loading SmartAI Waypoints...");
    sSmartWaypointMgr->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Creature Formations..." );
    sCreatureGroupMgr.LoadCreatureFormations();
    
    TC_LOG_INFO("server.loading","Loading Conditions...");
    sConditionMgr.LoadConditions();

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
    
    TC_LOG_INFO("server.loading","Loading Creature Texts...");
    sCreatureTextMgr.LoadCreatureTexts();

    ///- Load and initialize scripts
    TC_LOG_INFO("server.loading", "Loading Scripts..." );
    sObjectMgr->LoadQuestStartScripts();                         // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sObjectMgr->LoadQuestEndScripts();                           // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sObjectMgr->LoadSpellScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadGameObjectScripts();                         // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadEventScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadWaypointScripts();

    TC_LOG_INFO("server.loading", "Loading Scripts text locales..." );    // must be after Load*Scripts calls
    sObjectMgr->LoadDbScriptStrings();

    TC_LOG_INFO("server.loading", "Initializing Scripts..." );
    if(!LoadScriptingModule())
        exit(1);
        
    TC_LOG_INFO("server.loading","Loading SmartAI scripts...");
    sSmartScriptMgr->LoadSmartAIFromDB();

    ///- Initialize game time and timers
    m_gameTime = time(NULL);
    m_startTime=m_gameTime;

    tm local;
    time_t curr;
    time(&curr);
    local=*(localtime(&curr));                              // dereference and assign
    char isoDate[128];
    sprintf( isoDate, "%04d-%02d-%02d %02d:%02d:%02d",
        local.tm_year+1900, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);

    LoginDatabase.PExecute("INSERT INTO uptime (realmid, starttime, uptime, revision, maxplayers) VALUES('%u', " UI64FMTD ", 0, '%s', 0)",
        realmID, uint64(m_startTime), isoDate, _FULLVERSION);

    m_timers[WUPDATE_OBJECTS].SetInterval(1);
    m_timers[WUPDATE_SESSIONS].SetInterval(1);
    m_timers[WUPDATE_WEATHERS].SetInterval(1000);
    m_timers[WUPDATE_AUCTIONS].SetInterval(MINUTE*1000);    //set auction update interval to 1 minute
    m_timers[WUPDATE_UPTIME].SetInterval(m_configs[CONFIG_UPTIME_UPDATE]*MINUTE*1000);
                                                            //Update "uptime" table based on configuration entry in minutes.
    m_timers[WUPDATE_CORPSES].SetInterval(20*MINUTE*1000);  //erase corpses every 20 minutes
    m_timers[WUPDATE_ANNOUNCES].SetInterval(MINUTE*1000); // Check announces every minute

    m_timers[WUPDATE_ARENASEASONLOG].SetInterval(MINUTE*1000);

    //to set mailtimer to return mails every day between 4 and 5 am
    //mailtimer is increased when updating auctions
    //one second is 1000 -(tested on win system)
    mail_timer = ((((localtime( &m_gameTime )->tm_hour + 20) % 24)* HOUR * 1000) / m_timers[WUPDATE_AUCTIONS].GetInterval() );
                                                            //1440
    mail_timer_expires = ( (DAY * 1000) / (m_timers[WUPDATE_AUCTIONS].GetInterval()));

    ///- Initilize static helper structures
    AIRegistry::Initialize();
    Player::InitVisibleBits();

    ///- Initialize MapManager
    TC_LOG_INFO("server.loading", "Starting Map System" );
    sMapMgr->Initialize();
    
    // Load Warden Data
    TC_LOG_INFO("server.loading","Loading Warden Data..." );
    WardenDataStorage.Init();

    ///- Initialize Battlegrounds
    TC_LOG_INFO("server.loading", "Starting Battleground System" );
    sBattlegroundMgr->CreateInitialBattlegrounds();
    sBattlegroundMgr->InitAutomaticArenaPointDistribution();

    ///- Initialize outdoor pvp
    TC_LOG_INFO("server.loading", "Starting Outdoor PvP System" );
    sOutdoorPvPMgr->InitOutdoorPvP();

    TC_LOG_INFO("server.loading", "Loading Transports..." );
    sTransportMgr->SpawnContinentTransports();

    TC_LOG_INFO("server.loading","Deleting expired bans..." );
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    TC_LOG_INFO("server.loading","Calculate next daily quest reset time..." );
    InitDailyQuestResetTime();

    TC_LOG_INFO("server.loading","Starting Game Event system..." );
    uint32 nextGameEvent = gameeventmgr.Initialize();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);    //depend on next event

    //TC_LOG_INFO("server.loading","Initialize AuctionHouseBot...");
    //auctionbot.Initialize();
    
    TC_LOG_INFO("server.loading","Initialize Quest Pools...");
    LoadQuestPoolsData();
    
    TC_LOG_INFO("server.loading","Loading automatic announces...");
    LoadAutoAnnounce();

    TC_LOG_INFO("server.loading","Cleaning up old logs...");
    if(m_configs[CONFIG_MONITORING_ENABLED])
        CleanupOldMonitorLogs(); 
    CleanupOldLogs();

    LoadCharacterNameData();

    uint32 serverStartedTime = GetMSTimeDiffToNow(serverStartingTime);
    TC_LOG_INFO("server.loading","World initialized in %u.%u seconds.", (serverStartedTime / 1000), (serverStartedTime % 1000));
}

void World::DetectDBCLang()
{
    uint32 m_lang_confid = sConfigMgr->GetIntDefault("DBC.Locale", 255);

    if(m_lang_confid != 255 && m_lang_confid >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("server.loading","Incorrect DBC.Locale! Must be >= 0 and < %d (set to 0)",TOTAL_LOCALES);
        m_lang_confid = LOCALE_enUS;
    }

    ChrRacesEntry const* race = sChrRacesStore.LookupEntry(1);

    std::string availableLocalsStr;

    int default_locale = TOTAL_LOCALES;
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

    TC_LOG_INFO("Using %s DBC Locale as default. All available DBC locales: %s",localeNames[m_defaultDbcLocale],availableLocalsStr.empty() ? "<none>" : availableLocalsStr.c_str());
}

void World::RecordTimeDiff(const char *text, ...)
{
    if(m_updateTimeCount != 1)
        return;
    if(!text)
    {
        m_currentTime = GetMSTime();
        return;
    }

    uint32 thisTime = GetMSTime();
    uint32 diff = GetMSTimeDiff(m_currentTime, thisTime);

    if(diff > m_configs[CONFIG_MIN_LOG_UPDATE])
    {
        va_list ap;
        char str [256];
        va_start(ap, text);
        vsnprintf(str,256,text, ap );
        va_end(ap);
        TC_LOG_INFO("FIXME","Difftime %s: %u.", str, diff);
    }

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
    m_updateTime = uint32(diff);

    if(m_configs[CONFIG_MONITORING_ENABLED])
    {
        if (m_configs[CONFIG_MONITORING_UPDATE])
        {
            if (m_updateTimeMon > m_configs[CONFIG_MONITORING_UPDATE])
            {
                UpdateMonitoring(diff);
                m_updateTimeMon = 0;
            }
            m_updateTimeMon += diff;
        }
    }

    if(m_configs[CONFIG_INTERVAL_LOG_UPDATE])
    {
        if(m_updateTimeSum > m_configs[CONFIG_INTERVAL_LOG_UPDATE])
        {
            TC_LOG_DEBUG("FIXME","Update time diff: %u. Players online: %u.", m_updateTimeSum / m_updateTimeCount, GetActiveSessionCount());
            m_updateTimeSum = m_updateTime;
            m_updateTimeCount = 1;
        }
        else
        {
            m_updateTimeSum += m_updateTime;
            ++m_updateTimeCount;
        }
    }
    
    ///- Record average timediff for last 150 loops
    fastTdCount++;
    fastTdSum += diff;

    if (fastTdCount >= 150) {        // Record avg time diff
        avgTdCount++;
        avgTdSum += fastTdSum;
        
        fastTd = (uint32)fastTdSum/fastTdCount;
        fastTdCount = 0;
        fastTdSum = 0;
    }
        
    if (avgTdCount >= 10) {        // Check every ~15 mins if restart is needed
        avgTd = (uint32)avgTdSum/(avgTdCount*150);
        if (avgTd > m_configs[CONFIG_MAX_AVERAGE_TIMEDIFF] && !sWorld->IsShuttingDown()) {
            // Trigger restart
            sWorld->ShutdownServ(900, SHUTDOWN_MASK_RESTART, "Auto-restart triggered due to abnormal server load.");
        }

        avgTdCount = 0;
        avgTdSum = 0;
    }

    ///- Update the different timers
    for(int i = 0; i < WUPDATE_COUNT; i++)
        if(m_timers[i].GetCurrent()>=0)
            m_timers[i].Update(diff);
    else m_timers[i].SetCurrent(0);

    ///- Update the game time and check for shutdown time
    _UpdateGameTime();

    /// Handle daily quests reset time
    if(m_gameTime > m_NextDailyQuestReset)
    {
        ResetDailyQuests();
        InitNewDataForQuestPools();
        m_NextDailyQuestReset += DAY;
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
        sAHMgr.Update();
    }

    /// <li> Handle session updates when the timer has passed
    if (m_timers[WUPDATE_SESSIONS].Passed())
    {
        m_timers[WUPDATE_SESSIONS].Reset();

        RecordTimeDiff(NULL);
        UpdateSessions(diff);
        RecordTimeDiff("UpdateSessions");

        // Update groups
        RecordTimeDiff(NULL);
        for (ObjectMgr::GroupSet::iterator itr = sObjectMgr->GetGroupSetBegin(); itr != sObjectMgr->GetGroupSetEnd(); ++itr)
            (*itr)->Update(diff);
        RecordTimeDiff("UpdateGroups");
    }

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
        uint32 tmpDiff = (m_gameTime - m_startTime);
        uint32 maxClientsNum = GetMaxActiveSessionCount();

        m_timers[WUPDATE_UPTIME].Reset();
        LoginDatabase.PExecute("UPDATE uptime SET uptime = %u, maxplayers = %u WHERE realmid = %u AND starttime = " UI64FMTD, tmpDiff, maxClientsNum, realmID, uint64(m_startTime));
    }

    /// <li> Handle all other objects
    if (m_timers[WUPDATE_OBJECTS].Passed())
    {
        m_timers[WUPDATE_OBJECTS].Reset();
        ///- Update objects when the timer has passed (maps, transport, creatures,...)
        sMapMgr->Update(diff);                // As interval = 0

        RecordTimeDiff(NULL);
        ///- Process necessary scripts
        if (!m_scriptSchedule.empty())
            ScriptsProcess();
        RecordTimeDiff("UpdateScriptsProcess");

        sBattlegroundMgr->Update(diff);
        RecordTimeDiff("UpdateBattlegroundMgr");


        sOutdoorPvPMgr->Update(diff);
        RecordTimeDiff("UpdateOutdoorPvPMgr");

    }

    RecordTimeDiff(NULL);

    ///- Erase corpses once every 20 minutes
    if (m_timers[WUPDATE_CORPSES].Passed())
    {
        m_timers[WUPDATE_CORPSES].Reset();

        CorpsesErase();
    }

    if (m_timers[WUPDATE_ARENASEASONLOG].Passed())
    {
        m_timers[WUPDATE_ARENASEASONLOG].Reset();

        UpdateArenaSeasonLogs();
    }
    
    // execute callbacks from sql queries that were queued recently
    ProcessQueryCallbacks();
    RecordTimeDiff("ProcessQueryCallbacks");

    ///- Announce if a timer has passed
    if (m_timers[WUPDATE_ANNOUNCES].Passed()) 
    {
        m_timers[WUPDATE_ANNOUNCES].Reset();
        
        if (getConfig(CONFIG_AUTOANNOUNCE_ENABLED)) {
            time_t curTime = time(NULL);
            for (std::map<uint32, AutoAnnounceMessage*>::iterator itr = autoAnnounces.begin(); itr != autoAnnounces.end(); itr++) {
                if (itr->second->nextAnnounce <= curTime) {
                    SendWorldText(LANG_AUTO_ANN, itr->second->message.c_str());
                    itr->second->nextAnnounce += DAY;
                }
            }
        }
    }

    ///- Process Game events when necessary
    if (m_timers[WUPDATE_EVENTS].Passed())
    {
        m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
        uint32 nextGameEvent = gameeventmgr.Update();
        m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
        m_timers[WUPDATE_EVENTS].Reset();
    }

    /// </ul>
    ///- Move all creatures with "delayed move" and remove and delete all objects with "delayed remove"
    //sMapMgr->DoDelayedMovesAndRemoves();

    // update the instance reset times
    sInstanceSaveManager.Update();

    // And last, but not least handle the issued cli commands
    ProcessCliCommands();

    sScriptMgr->OnWorldUpdate(diff);
}

void World::ForceGameEventUpdate()
{
    m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
    uint32 nextGameEvent = gameeventmgr.Update();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
    m_timers[WUPDATE_EVENTS].Reset();
}

/// Put scripts in the execution queue
void World::ScriptsStart(ScriptMapMap const& scripts, uint32 id, Object* source, Object* target, bool start)
{
    ///- Find the script map
    ScriptMapMap::const_iterator s = scripts.find(id);
    if (s == scripts.end())
        return;

    // prepare static data
    uint64 sourceGUID = source ? source->GetGUID() : (uint64)0; //some script commands doesn't have source
    uint64 targetGUID = target ? target->GetGUID() : (uint64)0;
    uint64 ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : (uint64)0;

    ///- Schedule script execution for all scripts in the script map
    ScriptMap const *s2 = &(s->second);
    bool immedScript = false;
    for (ScriptMap::const_iterator iter = s2->begin(); iter != s2->end(); ++iter)
    {
        ScriptAction sa;
        sa.sourceGUID = sourceGUID;
        sa.targetGUID = targetGUID;
        sa.ownerGUID  = ownerGUID;

        sa.script = &iter->second;
        //TC_LOG_INFO("SCRIPT: Inserting script with source guid " UI64FMTD " target guid " UI64FMTD " owner guid " UI64FMTD " script id %u", sourceGUID, targetGUID, ownerGUID, id);
        m_scriptSchedule.insert(std::pair<time_t, ScriptAction>(m_gameTime + iter->first, sa));
        if (iter->first == 0)
            immedScript = true;
    }
    ///- If one of the effects should be immediate, launch the script execution
    if (start && immedScript)
        ScriptsProcess();
}

void World::ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target)
{
    // NOTE: script record _must_ exist until command executed

    // prepare static data
    uint64 sourceGUID = source ? source->GetGUID() : (uint64)0;
    uint64 targetGUID = target ? target->GetGUID() : (uint64)0;
    uint64 ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : (uint64)0;

    ScriptAction sa;
    sa.sourceGUID = sourceGUID;
    sa.targetGUID = targetGUID;
    sa.ownerGUID  = ownerGUID;

    sa.script = &script;
    //TC_LOG_INFO("SCRIPTCMD: Inserting script with source guid " UI64FMTD " target guid " UI64FMTD " owner guid " UI64FMTD " script id %u", sourceGUID, targetGUID, ownerGUID, script.id);
    m_scriptSchedule.insert(std::pair<time_t, ScriptAction>(m_gameTime + delay, sa));

    ///- If effects should be immediate, launch the script execution
    if(delay == 0)
        ScriptsProcess();
}

/// Process queued scripts
void World::ScriptsProcess()
{
    if (m_scriptSchedule.empty())
        return;

    ///- Process overdue queued scripts
    std::multimap<time_t, ScriptAction>::iterator iter = m_scriptSchedule.begin();
                                                            // ok as multimap is a *sorted* associative container
    while (!m_scriptSchedule.empty() && (iter->first <= m_gameTime))
    {
        ScriptAction const& step = iter->second;

        Object* source = NULL;

        if(step.sourceGUID)
        {
            switch(GUID_HIPART(step.sourceGUID))
            {
                case HIGHGUID_ITEM:
                    // case HIGHGUID_CONTAINER: ==HIGHGUID_ITEM
                    {
                        Player* player = HashMapHolder<Player>::Find(step.ownerGUID);
                        if(player)
                            source = player->GetItemByGuid(step.sourceGUID);
                        break;
                    }
                case HIGHGUID_UNIT:
                    source = HashMapHolder<Creature>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_PET:
                    source = HashMapHolder<Pet>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_PLAYER:
                    source = HashMapHolder<Player>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_GAMEOBJECT:
                    source = HashMapHolder<GameObject>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_CORPSE:
                    source = HashMapHolder<Corpse>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_MO_TRANSPORT:
                    /*
                    for (MapManager::TransportSet::iterator iter = sMapMgr->m_Transports.begin(); iter != sMapMgr->m_Transports.end(); ++iter)
                    {
                        if((*iter)->GetGUID() == step.sourceGUID)
                        {
                            source = reinterpret_cast<Object*>(*iter);
                            break;
                        }
                    }
                    break;*/
                default:
                    TC_LOG_ERROR("FIXME","*_script source with unsupported high guid value %u",GUID_HIPART(step.sourceGUID));
                    break;
            }
        }

        //if(source && !source->IsInWorld()) source = NULL;

        Object* target = NULL;

        if(step.targetGUID)
        {
            switch(GUID_HIPART(step.targetGUID))
            {
                case HIGHGUID_UNIT:
                    target = HashMapHolder<Creature>::Find(step.targetGUID);
                    break;
                case HIGHGUID_PET:
                    target = HashMapHolder<Pet>::Find(step.targetGUID);
                    break;
                case HIGHGUID_PLAYER:                       // empty GUID case also
                    target = HashMapHolder<Player>::Find(step.targetGUID);
                    break;
                case HIGHGUID_GAMEOBJECT:
                    target = HashMapHolder<GameObject>::Find(step.targetGUID);
                    break;
                case HIGHGUID_CORPSE:
                    target = HashMapHolder<Corpse>::Find(step.targetGUID);
                    break;
                default:
                    TC_LOG_ERROR("FIXME","*_script source with unsupported high guid value %u",GUID_HIPART(step.targetGUID));
                    break;
            }
        }
        
        if (!source && !target)
            TC_LOG_ERROR("FIXME","World::ScriptProcess: no source neither target for this script, if this is the last line before a crash, then you'd better return here.");

        //if(target && !target->IsInWorld()) target = NULL;

        if (GUID_HIPART(step.sourceGUID) == 16256 || GUID_HIPART(step.targetGUID) == 16256) {
            TC_LOG_ERROR("FIXME","Source high GUID seems to be corrupted, skipping this script. Source GUID: " UI64FMTD ", target GUID: " UI64FMTD ", owner GUID: " UI64FMTD ", script info address: %p.", step.sourceGUID, step.targetGUID, step.ownerGUID, step.script);
            if (m_scriptSchedule.size() == 1) {
                m_scriptSchedule.clear();
                break;
            }
        }

        switch (step.script->command)
        {
            case SCRIPT_COMMAND_TALK:
            {
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TALK call for NULL creature.");
                    break;
                }

                if(source->GetTypeId()!=TYPEID_UNIT)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TALK call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }
                if(step.script->datalong > 3)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TALK invalid chat type (%u), skipping.",step.script->datalong);
                    break;
                }

                uint64 unit_target = target ? target->GetGUID() : 0;

                //datalong 0=normal say, 1=whisper, 2=yell, 3=emote text
                switch(step.script->datalong)
                {
                    case 0:                                 // Say
                        (source->ToCreature())->Say(step.script->dataint, LANG_UNIVERSAL, unit_target);
                        break;
                    case 1:                                 // Whisper
                        if(!unit_target)
                        {
                            TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TALK attempt to whisper (%u) NULL, skipping.",step.script->datalong);
                            break;
                        }
                        (source->ToCreature())->Whisper(step.script->dataint,unit_target);
                        break;
                    case 2:                                 // Yell
                        (source->ToCreature())->Yell(step.script->dataint, LANG_UNIVERSAL, unit_target);
                        break;
                    case 3:                                 // Emote text
                        (source->ToCreature())->TextEmote(step.script->dataint, unit_target);
                        break;
                    default:
                        break;                              // must be already checked at load
                }
                break;
            }

            case SCRIPT_COMMAND_EMOTE:
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_EMOTE call for NULL creature.");
                    break;
                }

                if(source->GetTypeId()!=TYPEID_UNIT)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_EMOTE call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                (source->ToCreature())->HandleEmoteCommand(step.script->datalong);
                break;
            case SCRIPT_COMMAND_FIELD_SET:
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_FIELD_SET call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_FIELD_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->SetUInt32Value(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_MOVE_TO:
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_MOVE_TO call for NULL creature.");
                    break;
                }

                if(source->GetTypeId()!=TYPEID_UNIT)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_MOVE_TO call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }
                ((Unit *)source)->MonsterMoveWithSpeed(step.script->x, step.script->y, step.script->z, step.script->datalong2 );
                ((Unit *)source)->GetMap()->CreatureRelocation((source->ToCreature()), step.script->x, step.script->y, step.script->z, 0);
                break;
            case SCRIPT_COMMAND_FLAG_SET:
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_FLAG_SET call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_FLAG_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->SetFlag(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_FLAG_REMOVE:
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_FLAG_REMOVE call for NULL object.");
                    break;
                }
                if(step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_FLAG_REMOVE call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->RemoveFlag(step.script->datalong, step.script->datalong2);
                break;

            case SCRIPT_COMMAND_TELEPORT_TO:
            {
                // accept player in any one from target/source arg
                if (!target && !source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TELEPORT_TO call for NULL object.");
                    break;
                }

                                                            // must be only Player
                if((!target || target->GetTypeId() != TYPEID_PLAYER) && (!source || source->GetTypeId() != TYPEID_PLAYER))
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TELEPORT_TO call for non-player (TypeIdSource: %u)(TypeIdTarget: %u), skipping.", source ? source->GetTypeId() : 0, target ? target->GetTypeId() : 0);
                    break;
                }

                Player* pSource = target && target->GetTypeId() == TYPEID_PLAYER ? target->ToPlayer() : source->ToPlayer();

                pSource->TeleportTo(step.script->datalong, step.script->x, step.script->y, step.script->z, step.script->o);
                break;
            }

            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            {
                if(!step.script->datalong)                  // creature not specified
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for NULL creature.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for NULL world object.");
                    break;
                }

                WorldObject* summoner = dynamic_cast<WorldObject*>(source);

                if(!summoner)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for non-WorldObject (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                float x = step.script->x;
                float y = step.script->y;
                float z = step.script->z;
                float o = step.script->o;

                Creature* pCreature = summoner->SummonCreature(step.script->datalong, x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,step.script->datalong2);
                if (!pCreature)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_TEMP_SUMMON failed for creature (entry: %u).",step.script->datalong);
                    break;
                }

                break;
            }

            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            {
                if(!step.script->datalong)                  // gameobject not specified
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for NULL gameobject.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for NULL world object.");
                    break;
                }

                WorldObject* summoner = dynamic_cast<WorldObject*>(source);

                if(!summoner)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for non-WorldObject (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                GameObject *go = NULL;
                int32 time_to_despawn = step.script->datalong2<5 ? 5 : (int32)step.script->datalong2;

                CellPair p(Trinity::ComputeCellPair(summoner->GetPositionX(), summoner->GetPositionY()));
                Cell cell(p);
                cell.data.Part.reserved = ALL_DISTRICT;

                Trinity::GameObjectWithDbGUIDCheck go_check(*summoner,step.script->datalong);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(go,go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *summoner->GetMap());

                if ( !go )
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }

                if( go->GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE ||
                    go->GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE ||
                    go->GetGoType()==GAMEOBJECT_TYPE_DOOR        ||
                    go->GetGoType()==GAMEOBJECT_TYPE_BUTTON      ||
                    go->GetGoType()==GAMEOBJECT_TYPE_TRAP )
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_RESPAWN_GAMEOBJECT can not be used with gameobject of type %u (guid: %u).", uint32(go->GetGoType()), step.script->datalong);
                    break;
                }

                if( go->isSpawned() )
                    break;                                  //gameobject already spawned

                go->SetLootState(GO_READY);
                go->SetRespawnTime(time_to_despawn);        //despawn object in ? seconds

                go->GetMap()->Add(go);
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:
            {
                if(!step.script->datalong)                  // door not specified
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_OPEN_DOOR call for NULL door.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_OPEN_DOOR call for NULL unit.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))          // must be any Unit (creature or player)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_OPEN_DOOR call for non-unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *door = NULL;
                int32 time_to_close = step.script->datalong2 < 15 ? 15 : (int32)step.script->datalong2;

                CellPair p(Trinity::ComputeCellPair(caster->GetPositionX(), caster->GetPositionY()));
                Cell cell(p);
                cell.data.Part.reserved = ALL_DISTRICT;

                Trinity::GameObjectWithDbGUIDCheck go_check(*caster,step.script->datalong);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(door,go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *caster->GetMap());

                if ( !door )
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_OPEN_DOOR failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }
                if ( door->GetGoType() != GAMEOBJECT_TYPE_DOOR )
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_OPEN_DOOR failed for non-door(GoType: %u).", door->GetGoType());
                    break;
                }

                if( !door->GetGoState() )
                    break;                                  //door already  open

                door->UseDoorOrButton(time_to_close);

                if(target && target->isType(TYPEMASK_GAMEOBJECT) && ((GameObject*)target)->GetGoType()==GAMEOBJECT_TYPE_BUTTON)
                    ((GameObject*)target)->UseDoorOrButton(time_to_close);
                break;
            }
            case SCRIPT_COMMAND_CLOSE_DOOR:
            {
                if(!step.script->datalong)                  // guid for door not specified
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CLOSE_DOOR call for NULL door.");
                    break;
                }

                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CLOSE_DOOR call for NULL unit.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))          // must be any Unit (creature or player)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CLOSE_DOOR call for non-unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *door = NULL;
                int32 time_to_open = step.script->datalong2 < 15 ? 15 : (int32)step.script->datalong2;

                CellPair p(Trinity::ComputeCellPair(caster->GetPositionX(), caster->GetPositionY()));
                Cell cell(p);
                cell.data.Part.reserved = ALL_DISTRICT;

                Trinity::GameObjectWithDbGUIDCheck go_check(*caster,step.script->datalong);
                Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(door,go_check);

                TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > object_checker(checker);
                cell.Visit(p, object_checker, *caster->GetMap());

                if ( !door )
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CLOSE_DOOR failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }
                if ( door->GetGoType() != GAMEOBJECT_TYPE_DOOR )
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CLOSE_DOOR failed for non-door(GoType: %u).", door->GetGoType());
                    break;
                }

                if( door->GetGoState() )
                    break;                                  //door already closed

                door->UseDoorOrButton(time_to_open);

                if(target && target->isType(TYPEMASK_GAMEOBJECT) && ((GameObject*)target)->GetGoType()==GAMEOBJECT_TYPE_BUTTON)
                    ((GameObject*)target)->UseDoorOrButton(time_to_open);

                break;
            }
            case SCRIPT_COMMAND_QUEST_EXPLORED:
            {
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_QUEST_EXPLORED call for NULL source.");
                    break;
                }

                if(!target)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_QUEST_EXPLORED call for NULL target.");
                    break;
                }

                // when script called for item spell casting then target == (unit or GO) and source is player
                WorldObject* worldObject;
                Player* player;

                if(target->GetTypeId()==TYPEID_PLAYER)
                {
                    if(source->GetTypeId()!=TYPEID_UNIT && source->GetTypeId()!=TYPEID_GAMEOBJECT)
                    {
                        TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_QUEST_EXPLORED call for non-creature and non-gameobject (TypeId: %u), skipping.",source->GetTypeId());
                        break;
                    }

                    worldObject = (WorldObject*)source;
                    player = target->ToPlayer();
                }
                else
                {
                    if(target->GetTypeId()!=TYPEID_UNIT && target->GetTypeId()!=TYPEID_GAMEOBJECT)
                    {
                        TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_QUEST_EXPLORED call for non-creature and non-gameobject (TypeId: %u), skipping.",target->GetTypeId());
                        break;
                    }

                    if(source->GetTypeId()!=TYPEID_PLAYER)
                    {
                        TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_QUEST_EXPLORED call for non-player(TypeId: %u), skipping.",source->GetTypeId());
                        break;
                    }

                    worldObject = (WorldObject*)target;
                    player = source->ToPlayer();
                }

                // quest id and flags checked at script loading
                if( (worldObject->GetTypeId()!=TYPEID_UNIT || ((Unit*)worldObject)->IsAlive()) &&
                    (step.script->datalong2==0 || worldObject->IsWithinDistInMap(player,float(step.script->datalong2))) )
                    player->AreaExploredOrEventHappens(step.script->datalong);
                else
                    player->FailQuest(step.script->datalong);

                break;
            }

            case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            {
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_ACTIVATE_OBJECT must have source caster.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_ACTIVATE_OBJECT source caster isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if(!target)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_ACTIVATE_OBJECT call for NULL gameobject.");
                    break;
                }

                if(target->GetTypeId()!=TYPEID_GAMEOBJECT)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_ACTIVATE_OBJECT call for non-gameobject (TypeId: %u), skipping.",target->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *go = (GameObject*)target;

                go->Use(caster);
                break;
            }

            case SCRIPT_COMMAND_REMOVE_AURA:
            {
                Object* cmdTarget = step.script->datalong2 ? source : target;

                if(!cmdTarget)
                {
                    TC_LOG_ERROR("SCRIPT_COMMAND_REMOVE_AURA call for NULL %s.",step.script->datalong2 ? "source" : "target");
                    break;
                }

                if(!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("SCRIPT_COMMAND_REMOVE_AURA %s isn't unit (TypeId: %u), skipping.",step.script->datalong2 ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                ((Unit*)cmdTarget)->RemoveAurasDueToSpell(step.script->datalong);
                break;
            }

            case SCRIPT_COMMAND_CAST_SPELL:
            {
                if(!source)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL must have source caster.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL source caster isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Object* cmdTarget = step.script->datalong2 ? source : target;

                if(!cmdTarget)
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL (ID: %u) call for NULL %s.",step.script->id, step.script->datalong2 ? "source" : "target");
                    break;
                }

                if(!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("FIXME","SCRIPT_COMMAND_CAST_SPELL %s isn't unit (TypeId: %u), skipping.",step.script->datalong2 ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                Unit* spellTarget = (Unit*)cmdTarget;

                //TODO: when GO cast implemented, code below must be updated accordingly to also allow GO spell cast
                ((Unit*)source)->CastSpell(spellTarget,step.script->datalong,false);

                break;
            }

            case SCRIPT_COMMAND_LOAD_PATH:
            {
                if(!source)
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE is tried to apply to NON-existing unit.");
                    break;
                }

                if(!source->isType(TYPEMASK_UNIT))
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE source mover isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if(!sWaypointMgr->GetPath(step.script->datalong))
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_START_MOVE source mover has an invalid path (%u), skipping.", step.script->datalong);
                    break;
                }

                dynamic_cast<Unit*>(source)->GetMotionMaster()->MovePath(step.script->datalong);
                break;
            }

            case SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT:
            {
                if(!step.script->datalong || !step.script->datalong2)
                {
                    TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_CALLSCRIPT calls invallid db_script_id or lowguid not present: skipping.");
                    break;
                }
                //our target
                Creature* target = NULL;

                if(source) //using grid searcher
                {
                    CellPair p(Trinity::ComputeCellPair(((Unit*)source)->GetPositionX(), ((Unit*)source)->GetPositionY()));
                    Cell cell(p);
                    cell.data.Part.reserved = ALL_DISTRICT;

                    //TC_LOG_DEBUG("FIXME","Attempting to find Creature: Db GUID: %i", step.script->datalong);
                    Trinity::CreatureWithDbGUIDCheck target_check(((Unit*)source), step.script->datalong);
                    Trinity::CreatureSearcher<Trinity::CreatureWithDbGUIDCheck> checker(target,target_check);

                    TypeContainerVisitor<Trinity::CreatureSearcher <Trinity::CreatureWithDbGUIDCheck>, GridTypeMapContainer > unit_checker(checker);
                    cell.Visit(p, unit_checker, *((Unit *)source)->GetMap());
                }
                else //check hashmap holders
                {
                    if(CreatureData const* data = sObjectMgr->GetCreatureData(step.script->datalong))
                        target = ObjectAccessor::GetObjectInWorld<Creature>(data->mapid, data->posX, data->posY, MAKE_NEW_GUID(step.script->datalong, data->id, HIGHGUID_UNIT), target);
                }
                //TC_LOG_DEBUG("FIXME","attempting to pass target...");
                if(!target)
                    break;
                //TC_LOG_DEBUG("FIXME","target passed");
                //Lets choose our ScriptMap map
                ScriptMapMap *datamap = NULL;
                switch(step.script->dataint)
                {
                    case 1://QUEST END SCRIPTMAP
                        datamap = &sQuestEndScripts;
                        break;
                    case 2://QUEST START SCRIPTMAP
                        datamap = &sQuestStartScripts;
                        break;
                    case 3://SPELLS SCRIPTMAP
                        datamap = &sSpellScripts;
                        break;
                    case 4://GAMEOBJECTS SCRIPTMAP
                        datamap = &sGameObjectScripts;
                        break;
                    case 5://EVENTS SCRIPTMAP
                        datamap = &sEventScripts;
                        break;
                    case 6://WAYPOINTS SCRIPTMAP
                        datamap = &sWaypointScripts;
                        break;
                    default:
                        TC_LOG_ERROR("sql.sql","SCRIPT_COMMAND_CALLSCRIPT ERROR: no scriptmap present... ignoring");
                        break;
                }
                //if no scriptmap present...
                if(!datamap)
                    break;

                uint32 script_id = step.script->datalong2;
                //insert script into schedule but do not start it
                ScriptsStart(*datamap, script_id, target, NULL, false);
                break;
            }

            case SCRIPT_COMMAND_PLAYSOUND:
            {
                if(!source)
                    break;
                //datalong sound_id, datalong2 onlyself
                ((WorldObject*)source)->SendPlaySound(step.script->datalong, step.script->datalong2);
                break;
            }

            case SCRIPT_COMMAND_KILL:
            {
                if(!source || (source->ToCreature())->IsDead())
                    break;

                (source->ToCreature())->DealDamage((source->ToCreature()), (source->ToCreature())->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

                switch(step.script->dataint)
                {
                case 0: break; //return false not remove corpse
                case 1: (source->ToCreature())->RemoveCorpse(); break;
                }
                break;
            }
            
            case SCRIPT_COMMAND_KILL_CREDIT:
            {
                if (!source || ((Unit*)source)->GetTypeId() != TYPEID_PLAYER)
                    break;
                if (step.script->datalong2)
                    source->ToPlayer()->CastedCreatureOrGO(step.script->datalong, 0, step.script->datalong2);
                else
                    source->ToPlayer()->KilledMonster(step.script->datalong, 0);
                break;
            }

            default:
                TC_LOG_ERROR("sql.sql","Unknown script command %u called.",step.script->command);
                break;
        }

        m_scriptSchedule.erase(iter);

        iter = m_scriptSchedule.begin();
    }
    return;
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

    for(SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if(!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld() )
            continue;

        LocaleConstant loc_idx = itr->second->GetSessionDbcLocale();
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
                WorldPacket* data = new WorldPacket();
                ChatHandler::BuildChatPacket(*data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for(int i = 0; i < data_list->size(); ++i)
            itr->second->SendPacket((*data_list)[i]);
    }

    // free memory
    for(int i = 0; i < data_cache.size(); ++i)
        for(int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

void World::SendGMText(int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for(SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if(!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld() )
            continue;

        LocaleConstant loc_idx = itr->second->GetSessionDbcLocale();
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
                WorldPacket* data = new WorldPacket();
                ChatHandler::BuildChatPacket(*data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for(int i = 0; i < data_list->size(); ++i)
            if(itr->second->GetSecurity() > SEC_PLAYER)
            itr->second->SendPacket((*data_list)[i]);
    }

    // free memory
    for(int i = 0; i < data_cache.size(); ++i)
        for(int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
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
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        itr->second->KickPlayer();
}

/// Kick (and save) all players with security level less `sec`
void World::KickAllLess(AccountTypes sec)
{
    // session not removed at kick and will removed in next update tick
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if(itr->second->GetSecurity() < sec)
            itr->second->KickPlayer();
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
BanReturn World::BanAccount(BanMode mode, std::string const& _nameOrIP, uint32 duration_secs, std::string const& _reason, std::string const& author)
{
    std::string nameOrIP(_nameOrIP);
    LoginDatabase.EscapeString(nameOrIP);
    std::string reason(_reason);
    LoginDatabase.EscapeString(reason);
    std::string safe_author=author;
    LoginDatabase.EscapeString(safe_author);

    QueryResult resultAccounts = NULL;                     //used for kicking
    
    uint32 authorGUID = sObjectMgr->GetPlayerLowGUIDByName(safe_author);

    ///- Update the database with ban information
    switch(mode)
    {
        case BAN_IP:
            //No SQL injection as strings are escaped
            resultAccounts = LoginDatabase.PQuery("SELECT id FROM account WHERE last_ip = '%s'",nameOrIP.c_str());
            LoginDatabase.PExecute("INSERT INTO ip_banned VALUES ('%s',UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+%u,'%s','%s')",nameOrIP.c_str(),duration_secs,safe_author.c_str(),reason.c_str());
            break;
        case BAN_ACCOUNT:
            //No SQL injection as string is escaped
            resultAccounts = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'",nameOrIP.c_str());
            break;
        case BAN_CHARACTER:
            //No SQL injection as string is escaped
            resultAccounts = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name = '%s'",nameOrIP.c_str());
            break;
        default:
            return BAN_SYNTAX_ERROR;
    }

    if(!resultAccounts)
    {
        if(mode==BAN_IP)
            return BAN_SUCCESS;                             // ip correctly banned but nobody affected (yet)
        else
            return BAN_NOTFOUND;                                // Nobody to ban
    }

    ///- Disconnect all affected players (for IP it can be several)
    do
    {
        Field* fieldsAccount = resultAccounts->Fetch();
        uint32 account = fieldsAccount->GetUInt32();
        LoginDatabase.PExecute("UPDATE account SET email_temp = '', email_ts = 0 WHERE id = %u",account);

        if(mode!=BAN_IP)
        {
            //No SQL injection as strings are escaped
            LoginDatabase.PExecute("INSERT INTO account_banned VALUES ('%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s', '1')",
                account,duration_secs,safe_author.c_str(),reason.c_str());
            LogsDatabase.PExecute("INSERT INTO sanctions VALUES (%u, %u, %u, %u, " UI64FMTD ", \"%s\")", account, authorGUID, uint32(SANCTION_BAN_ACCOUNT), duration_secs, uint64(time(NULL)), reason.c_str());
        }
        else {
            // Log ip ban for each account on that IP
            LogsDatabase.PExecute("INSERT INTO sanctions VALUES (%u, %u, %u, %u, " UI64FMTD ", \"%s\")", account, authorGUID, uint32(SANCTION_BAN_IP), duration_secs, uint64(time(NULL)), reason.c_str());
        }

        if (WorldSession* sess = FindSession(account))
            if(std::string(sess->GetPlayerName()) != author)
                sess->KickPlayer();
    }
    while( resultAccounts->NextRow() );

    return BAN_SUCCESS;
}

/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
BanReturn World::BanAccount(BanMode mode, std::string const& nameOrIP, std::string const& duration, std::string const& reason, std::string const& author)
{
    uint32 duration_secs = TimeStringToSecs(duration);
    return BanAccount(mode, nameOrIP, duration_secs, reason, author);
}

/// Remove a ban from an account or IP address
bool World::RemoveBanAccount(BanMode mode, std::string nameOrIP)
{
    if (mode == BAN_IP)
    {
        LoginDatabase.EscapeString(nameOrIP);
        LoginDatabase.PExecute("DELETE FROM ip_banned WHERE ip = '%s'",nameOrIP.c_str());
    }
    else
    {
        uint32 account = 0;
        if (mode == BAN_ACCOUNT)
            account = sAccountMgr->GetId (nameOrIP);
        else if (mode == BAN_CHARACTER)
            account = sObjectMgr->GetPlayerAccountIdByPlayerName (nameOrIP);

        if (!account)
            return false;

        //NO SQL injection as account is uint32
        LoginDatabase.PExecute("UPDATE account_banned SET active = '0' WHERE id = '%u'",account);
    }
    return true;
}

/// Update the game time
void World::_UpdateGameTime()
{
    ///- update the time
    time_t thisTime = time(NULL);
    uint32 elapsed = uint32(thisTime - m_gameTime);
    m_gameTime = thisTime;

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

            ShutdownMsg(false, NULL, m_ShutdownReason);
        }
    }
}

/// Shutdown the server
void World::ShutdownServ(uint32 time, uint32 options, const char* reason)
{
    // ignore if server shutdown at next tick
    if(m_stopEvent)
        return;

    m_ShutdownMask = options;
    m_ShutdownReason = reason;

    ///- If the shutdown time is 0, set m_stopEvent (except if shutdown is 'idle' with remaining sessions)
    if(time==0)
    {
        if(!(options & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount()==0)
            m_stopEvent = true;                             // exist code already set
        else
            m_ShutdownTimer = 1;                            //So that the session count is re-evaluated at next world tick
    }
    ///- Else set the shutdown timer and warn users
    else
    {
        m_ShutdownTimer = time;
        ShutdownMsg(true, NULL, m_ShutdownReason);
    }

//    sScriptMgr->OnShutdownInitiate(ShutdownExitCode(exitcode), ShutdownMask(options));
}

/// Display a shutdown message to the user(s)
void World::ShutdownMsg(bool show, Player* player, std::string reason)
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
        TC_LOG_DEBUG("FIXME","Server is %s in %s",(m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shutting down"),str.c_str());
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

    TC_LOG_DEBUG("FIXME","Server %s cancelled.",(m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"));
}

/// Send a server message to the user(s)
void World::SendServerMessage(uint32 type, const char *text, Player* player)
{
    WorldPacket data(SMSG_SERVER_MESSAGE, 50);              // guess size
    data << uint32(type);
    if(type <= SERVER_MSG_STRING)
        data << text;

    if(player)
        player->GetSession()->SendPacket(&data);
    else
        SendGlobalMessage( &data );
}

void World::UpdateSessions(uint32 diff)
{
    ///- Add new sessions
    WorldSession* sess = NULL;
    while (addSessQueue.next(sess))
        AddSession_ (sess);

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
                m_disconnects[itr->second->GetAccountId()] = time(NULL);
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
    HashMapHolder<Player>::MapType& onlinePlayers = sObjectAccessor->GetPlayers();
    for (auto itr : onlinePlayers)
    {
        if(itr.second)
            itr.second->UpdateArenaTitles();
    }
}

// This handles the issued and queued CLI commands
void World::ProcessCliCommands()
{
    CliCommandHolder::Print* zprint = NULL;
    void* callbackArg = NULL;
    CliCommandHolder* command = NULL;
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
    m_realmCharCallbacks.push_back(CharacterDatabase.AsyncQuery(stmt));
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
        stmt->setUInt32(1, realmID);
        LoginDatabase.Execute(stmt);

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
        stmt->setUInt8(0, charCount);
        stmt->setUInt32(1, accountId);
        stmt->setUInt32(2, realmID);
        LoginDatabase.Execute(stmt);
    }
}

void World::InitDailyQuestResetTime()
{
    time_t mostRecentQuestTime;

    QueryResult result = CharacterDatabase.Query("SELECT MAX(time) FROM character_queststatus_daily");
    if(result)
    {
        Field *fields = result->Fetch();

        mostRecentQuestTime = (time_t)fields[0].GetUInt64();
    }
    else
        mostRecentQuestTime = 0;

    // client built-in time for reset is 6:00 AM
    // FIX ME: client not show day start time
    time_t curTime = time(NULL);
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
     QueryResult result = LoginDatabase.PQuery("SELECT allowedSecurityLevel from realmlist WHERE id = '%d'", realmID);
     if (result)
     {
        m_allowedSecurityLevel = AccountTypes(result->Fetch()->GetUInt16());
     }
}

void World::ResetDailyQuests()
{
    TC_LOG_DEBUG("FIXME","Daily quests reset for all characters.");
    
    // Every 1st of the month, delete data for quests 9884, 9885, 9886, 9887
    time_t curTime = time(NULL);
    tm localTm = *localtime(&curTime);
    bool reinitConsortium = false;
    if (localTm.tm_mday == 1) {
        reinitConsortium = true;
        CharacterDatabase.Execute("DELETE FROM character_queststatus WHERE quest IN (9884, 9885, 9886, 9887)");
    }
    
    CharacterDatabase.Execute("DELETE FROM character_queststatus_daily");
    for(SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr) {
        if(itr->second->GetPlayer()) {
            itr->second->GetPlayer()->ResetDailyQuestStatus();
            if (reinitConsortium) {
                itr->second->GetPlayer()->SetQuestStatus(9884, QUEST_STATUS_NONE);
                itr->second->GetPlayer()->SetQuestStatus(9885, QUEST_STATUS_NONE);
                itr->second->GetPlayer()->SetQuestStatus(9886, QUEST_STATUS_NONE);
                itr->second->GetPlayer()->SetQuestStatus(9887, QUEST_STATUS_NONE);
            }
        }
    }
}

void World::CleanupOldMonitorLogs()
{
    TC_LOG_DEBUG("FIXME","Cleaning old logs from monitoring system. ( > 1 month old)");
    
    time_t now = time(NULL);
    time_t limit = now - (1 * MONTH);
    
    SQLTransaction trans = LogsDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM mon_players WHERE time < %u", limit);
    trans->PAppend("DELETE FROM mon_timediff WHERE time < %u", limit);
    trans->PAppend("DELETE FROM mon_maps WHERE time < %u", limit);
    trans->PAppend("DELETE FROM mon_races WHERE time < %u", limit);
    trans->PAppend("DELETE FROM mon_classes WHERE time < %u", limit);
    LogsDatabase.CommitTransaction(trans);
}

void World::CleanupOldLogs()
{
    TC_LOG_DEBUG("FIXME","Cleaning old logs for deleted chars and items. ( > 1 month old)");

    time_t now = time(NULL);
    time_t limit = now - (1 * MONTH);
    
    SQLTransaction trans = LogsDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM char_delete WHERE time < %u", limit);
    trans->PAppend("DELETE FROM item_delete WHERE time < %u", limit);
    trans->PAppend("DELETE FROM item_mail WHERE time < %u",   limit);
    LogsDatabase.CommitTransaction(trans);
}

void World::InitNewDataForQuestPools()
{
    TC_LOG_DEBUG("FIXME","Init new current quest in pools.");
    QueryResult result = WorldDatabase.PQuery("SELECT pool_id FROM quest_pool_current");
    if (!result) {
        TC_LOG_ERROR("FIXME","World::InitNewDataForQuestPools: No quest_pool found!");
        return;
    }
    
    do {
        Field* fields = result->Fetch();
        uint32 poolId = fields[0].GetUInt32();
        
        QueryResult resquests = WorldDatabase.PQuery("SELECT quest_id FROM quest_pool WHERE pool_id = %u", poolId);
        if (!resquests) {
            TC_LOG_ERROR("FIXME","World::InitNewDataForQuestPools: No quest in pool (%u)!", poolId);
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

bool World::IsZoneSanctuary(uint32 zoneid)
{
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

    for (it = v.begin(); it != v.end(); it++) {
        if (atoi((*it).c_str()) == zoneid)
            return true;
    }

    return false;
}

bool World::IsZoneFFA(uint32 zoneid)
{
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

    for (it = v.begin(); it != v.end(); it++) {
        if (atoi((*it).c_str()) == zoneid)
            return true;
    }

    return false;
}

bool World::IsPhishing(std::string msg)
{
    std::string badstr = sConfigMgr->GetStringDefault("PhishingWords", "");
    std::vector<std::string> v;
    std::vector<std::string>::iterator itr;
    std::string tempstr;
    int cutAt;

    if (badstr.length() == 0)
        return false;

    tempstr = badstr;
    while ((cutAt = tempstr.find_first_of(",")) != tempstr.npos) {
        if (cutAt > 0)
            v.push_back(tempstr.substr(0, cutAt));
        tempstr = tempstr.substr(cutAt + 1);
    }

    if (tempstr.length() > 1)
        v.push_back(tempstr);

    for (itr = v.begin(); itr != v.end(); itr++) {
        if (msg.find(*itr) != msg.npos)
            return true;
    }

    return false;
}

void World::LogPhishing(uint32 src, uint32 dst, std::string msg)
{
    std::string msgsafe = msg;
    LogsDatabase.EscapeString(msgsafe);
    LogsDatabase.PExecute("INSERT INTO phishing (srcguid, dstguid, time, data) VALUES ('%u', '%u', UNIX_TIMESTAMP(), '%s')", src, dst, msgsafe.c_str());
}

void World::LoadMotdAndTwitter()
{
    /*
    QueryResult motdRes = LoginDatabase.PQuery("SELECT motd, last_twitter FROM realmlist WHERE id = %u", realmID);
    if (!motdRes) {
        TC_LOG_ERROR("FIXME","Could not get motd from database for realm %u", realmID);       // I think that should never happen
        return;
    }
    
    Field *fields = motdRes->Fetch();
    */
    m_motd = "tofixmotd"; //fields[0].GetString();
    m_lastTwitter = "tofixtwiter"; //fields[1].GetString();
}

void World::UpdateMonitoring(uint32 diff)
{
    FILE *fp;
    std::string monpath;
    std::string filename;
    char data[64];
    time_t now = time(NULL);

    monpath = sConfigMgr->GetStringDefault("Monitor.path", "");
    monpath += "/";
    
    SQLTransaction trans = LogsDatabase.BeginTransaction();

    /* players */

    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.players", "players");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    sprintf(data, "%lu %lu", GetActiveSessionCount(), GetQueuedSessionCount());
    trans->PAppend("INSERT INTO mon_players (time, active, queued) VALUES (%u, %u, %u)", (uint32)now, GetActiveSessionCount(), GetQueuedSessionCount());
    fputs(data, fp);
    fclose(fp);

    /* time diff */

    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.timediff", "timediff");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    sprintf(data, "%lu", fastTd);
    trans->PAppend("INSERT INTO mon_timediff (time, diff) VALUES (%u, %u)", (uint32)now, fastTd);
    fputs(data, fp);
    fclose(fp);

    /* maps */

    std::string maps = "eastern kalimdor outland karazhan hyjal ssc blacktemple tempestkeep zulaman warsong arathi eye alterac arenas sunwell";
    std::stringstream cnts;
    int arena_cnt = 0;
    arena_cnt += sMapMgr->GetNumPlayersInMap(562); /* nagrand */
    arena_cnt += sMapMgr->GetNumPlayersInMap(559); /* blade's edge */
    arena_cnt += sMapMgr->GetNumPlayersInMap(572); /* lordaeron */

    cnts << sMapMgr->GetNumPlayersInMap(0) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(1) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(530) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(532) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(534) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(548) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(564) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(550) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(568) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(489) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(529) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(566) << " ";
    cnts << sMapMgr->GetNumPlayersInMap(30) << " ";
    cnts << arena_cnt << " ";
    cnts << sMapMgr->GetNumPlayersInMap(580);
    
    int mapIds[14] = { 0, 1, 530, 532, 534, 548, 564, 550, 568, 489, 529, 566, 30, 580 };
    for (int i = 0; i < 14; i++)
        trans->PAppend("INSERT INTO mon_maps (time, map, players) VALUES (%u, %u, %u)", (uint32)now, mapIds[i], sMapMgr->GetNumPlayersInMap(mapIds[i]));
    // arenas
    trans->PAppend("INSERT INTO mon_maps (time, map, players) VALUES (%u, 559, %u)", (uint32)now, arena_cnt); // Nagrand!

    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.maps", "maps");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    fputs(maps.c_str(), fp);
    fputs("\n", fp);
    fputs(cnts.str().c_str(), fp);
    fclose(fp);

    /* battleground queue time */

    std::string bgs = "alterac warsong arathi eye 2v2 3v3 5v5";
    std::stringstream bgs_wait;

    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_AV].GetAvgTime() << " ";
    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_WS].GetAvgTime() << " ";
    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_AB].GetAvgTime() << " ";
    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_EY].GetAvgTime() << " ";
    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_2v2].GetAvgTime() << " ";
    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_3v3].GetAvgTime() << " ";
    bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_5v5].GetAvgTime();

    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.bgwait", "bgwait");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    fputs(bgs.c_str(), fp);
    fputs("\n", fp);
    fputs(bgs_wait.str().c_str(), fp);
    fclose(fp);

    /* max creature guid */

    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.creatureguid", "creatureguid");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    sprintf(data, "%lu", sObjectMgr->GetMaxCreatureGUID());
    fputs(data, fp);
    fclose(fp);
    
    /* races && classes */

    std::string races = "human orc dwarf nightelf undead tauren gnome troll bloodelf draenei";
    std::stringstream ssraces;
    
    std::string classes = "warrior paladin hunter rogue priest shaman mage warlock druid";
    std::stringstream ssclasses;
    
    uint32 racesCount[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32 classesCount[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    HashMapHolder<Player>::MapType& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr) {
        racesCount[itr->second->GetRace()]++;
        classesCount[itr->second->GetClass()]++;
    }
    
    ssraces << racesCount[1] << " " << racesCount[2] << " " << racesCount[3] << " ";
    ssraces << racesCount[4] << " " << racesCount[5] << " " << racesCount[6] << " ";
    ssraces << racesCount[7] << " " << racesCount[8] << " " << racesCount[10] << " ";
    ssraces << racesCount[11];
    
    ssclasses << classesCount[1] << " " << classesCount[2] << " " << classesCount[3] << " ";
    ssclasses << classesCount[4] << " " << classesCount[5] << " " << classesCount[7] << " ";
    ssclasses << classesCount[8] << " " << classesCount[9] << " " << classesCount[11] << " ";
    
    for (int i = 1; i < 12; i++) {
        if (i != 9) 
            trans->PAppend("INSERT INTO mon_races (time, race, players) VALUES (%u, %u, %u)", (uint32)now, i, racesCount[i]);
    }
    
    for (int i = 1; i < 12; i++) {
        if (i != 6 && i != 10)
            trans->PAppend("INSERT INTO mon_classes (time, `class`, players) VALUES (%u, %u, %u)", (uint32)now, i, classesCount[i]);
    }
    
    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.races", "races");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    fputs(races.c_str(), fp);
    fputs("\n", fp);
    fputs(ssraces.str().c_str(), fp);
    fclose(fp);
    
    filename = monpath;
    filename += sConfigMgr->GetStringDefault("Monitor.classes", "classes");
    if ((fp = fopen(filename.c_str(), "w")) == NULL)
        return;
    fputs(classes.c_str(), fp);
    fputs("\n", fp);
    fputs(ssclasses.str().c_str(), fp);
    fclose(fp);
    
    LogsDatabase.CommitTransaction(trans); // TODO: drop records from more than 8 days ago in a method like daily quests reinit
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
        
        AutoAnnounceMessage* ann = new AutoAnnounceMessage;
        ann->message = fields[1].GetString();
        uint32 hour = fields[2].GetUInt32();
        uint32 mins = fields[3].GetUInt32();

        time_t curTime = time(NULL);
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
        return NULL;

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
        return NULL;

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
    PreparedQueryResult result;

    for (std::deque<std::future<PreparedQueryResult>>::iterator itr = m_realmCharCallbacks.begin(); itr != m_realmCharCallbacks.end(); )
    {
        if ((*itr).wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            ++itr;
            continue;
        }

        result = (*itr).get();
        _UpdateRealmCharCount(result);
        itr = m_realmCharCallbacks.erase(itr);
    }
}


/**
* @brief Loads several pieces of information on server startup with the low GUID
* There is no further database query necessary.
* These are a number of methods that work into the calling function.
*
* @param guid Requires a lowGUID to call
* @return Name, Gender, Race, Class and Level of player character
* Example Usage:
* @code
*    CharacterNameData const* nameData = sWorld->GetCharacterNameData(lowGUID);
*    if (!nameData)
*        return;
*
* std::string playerName = nameData->m_name;
* uint8 playerGender = nameData->m_gender;
* uint8 playerRace = nameData->m_race;
* uint8 playerClass = nameData->m_class;
* uint8 playerLevel = nameData->m_level;
* @endcode
**/

void World::LoadCharacterNameData()
{
    TC_LOG_INFO("server.loading", "Loading character name data");

    QueryResult result = CharacterDatabase.Query("SELECT guid, name, race, gender, class, level FROM characters WHERE deleteDate IS NULL");
    if (!result)
    {
        TC_LOG_INFO("server.loading", "No character name data loaded, empty query");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        AddCharacterNameData(fields[0].GetUInt32(), fields[1].GetString(),
            fields[3].GetUInt8() /*gender*/, fields[2].GetUInt8() /*race*/, fields[4].GetUInt8() /*class*/, fields[5].GetUInt8() /*level*/);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded name data for %u characters", count);
}

void World::AddCharacterNameData(uint32 guid, std::string const& name, uint8 gender, uint8 race, uint8 playerClass, uint8 level)
{
    CharacterNameData& data = _characterNameDataMap[guid];
    data.m_name = name;
    data.m_race = race;
    data.m_gender = gender;
    data.m_class = playerClass;
    data.m_level = level;
}

void World::UpdateCharacterNameData(uint32 guid, std::string const& name, uint8 gender /*= GENDER_NONE*/, uint8 race /*= RACE_NONE*/)
{
    std::map<uint32, CharacterNameData>::iterator itr = _characterNameDataMap.find(guid);
    if (itr == _characterNameDataMap.end())
        return;

    itr->second.m_name = name;

    if (gender != GENDER_NONE)
        itr->second.m_gender = gender;

    if (race != RACE_NONE)
        itr->second.m_race = race;

    InvalidatePlayerDataToAllClient(MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER));
}

void World::UpdateCharacterNameDataLevel(uint32 guid, uint8 level)
{
    std::map<uint32, CharacterNameData>::iterator itr = _characterNameDataMap.find(guid);
    if (itr == _characterNameDataMap.end())
        return;

    itr->second.m_level = level;
}

CharacterNameData const* World::GetCharacterNameData(uint32 guid) const
{
    std::map<uint32, CharacterNameData>::const_iterator itr = _characterNameDataMap.find(guid);
    if (itr != _characterNameDataMap.end())
        return &itr->second;
    else
        return NULL;
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