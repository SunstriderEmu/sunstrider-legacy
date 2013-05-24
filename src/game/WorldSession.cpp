/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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
    \ingroup u2w
*/

#include "WorldSocket.h"                                    // must be first to make ACE happy with ACE includes in it
#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "Guild.h"
#include "World.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "BattleGroundMgr.h"
#include "OutdoorPvPMgr.h"
#include "Language.h"                                       // for CMSG_CANCEL_MOUNT_AURA handler
#include "Chat.h"
#include "SocialMgr.h"
#include "ScriptCalls.h"
#include "../scripts/ScriptMgr.h"
#include "Config/ConfigEnv.h"
#include "IRC.h"
#include "WardenWin.h"
#include "WardenMac.h"

bool MapSessionFilter::Process(WorldPacket * packet)
{
    OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];
    //let's check if our opcode can be really processed in Map::Update()
    if(opHandle.packetProcessing == PROCESS_INPLACE)
        return true;
        
    //we do not process thread-unsafe packets
    if(opHandle.packetProcessing == PROCESS_THREADUNSAFE)
        return false;

    Player * plr = m_pSession->GetPlayer();
    if(!plr)
        return false;

    //in Map::Update() we do not process packets where player is not in world!
    return plr->IsInWorld();
}

//we should process ALL packets when player is not in world/logged in
//OR packet handler is not thread-safe!
bool WorldSessionFilter::Process(WorldPacket* packet)
{
    OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];
    //check if packet handler is supposed to be safe
    if(opHandle.packetProcessing == PROCESS_INPLACE)
        return true;
        
    //thread-unsafe packets should be processed in World::UpdateSessions()
    if(opHandle.packetProcessing == PROCESS_THREADUNSAFE)
        return true;

    //no player attached? -> our client! ^^
    Player * plr = m_pSession->GetPlayer();
    if(!plr)
        return true;

    //lets process all packets for non-in-the-world player
    return (plr->IsInWorld() == false);
}

/// WorldSession constructor
WorldSession::WorldSession(uint32 id, WorldSocket *sock, uint32 sec, uint8 expansion, time_t mute_time, LocaleConstant locale, uint32 gid, bool mailChange) :
LookingForGroup_auto_join(false), LookingForGroup_auto_add(false), m_muteTime(mute_time),
_player(NULL), m_Socket(sock),_security(sec), _groupid(gid), _accountId(id), m_expansion(expansion),
m_sessionDbcLocale(sWorld.GetAvailableDbcLocale(locale)), m_sessionDbLocaleIndex(objmgr.GetIndexForLocale(locale)),
_logoutTime(0), m_inQueue(false), m_playerLoading(false), m_playerLogout(false), m_playerRecentlyLogout(false), m_latency(0), m_mailChange(mailChange), m_Warden(NULL)
{
    if (sock)
    {
        m_Address = sock->GetRemoteAddress ();
        sock->AddReference ();
        LoginDatabase.PExecute("UPDATE account SET online = 1 WHERE id = %u;", GetAccountId());
    }
}

/// WorldSession destructor
WorldSession::~WorldSession()
{
    ///- unload player if not unloaded
    if (_player)
        LogoutPlayer (true);

    /// - If have unclosed socket, close it
    if (m_Socket)
    {
        m_Socket->CloseSocket ();
        m_Socket->RemoveReference ();
        m_Socket = NULL;
    }
    
    if (m_Warden)
        delete m_Warden;

    ///- empty incoming packet queue
    while(!_recvQueue.empty())
    {
        WorldPacket *packet = _recvQueue.next ();
        delete packet;
    }
    LoginDatabase.PExecute("UPDATE account SET online = 0 WHERE id = %u;", GetAccountId());
    CharacterDatabase.PExecute("UPDATE characters SET online = 0 WHERE account = %u;", GetAccountId());
}

void WorldSession::SizeError(WorldPacket const& packet, uint32 size) const
{
    sLog.outError("Client (account %u) send packet %s (%u) with size %u but expected %u (attempt crash server?), skipped",
        GetAccountId(),LookupOpcodeName(packet.GetOpcode()),packet.GetOpcode(),packet.size(),size);
}

/// Get the player name
char const* WorldSession::GetPlayerName() const
{
    return GetPlayer() ? GetPlayer()->GetName() : "<none>";
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket const* packet, bool withDelayed /*= false*/)
{
    if (!m_Socket)
        return;

    #ifdef TRINITY_DEBUG

    // Code for network use statistic
    static uint64 sendPacketCount = 0;
    static uint64 sendPacketBytes = 0;

    static time_t firstTime = time(NULL);
    static time_t lastTime = firstTime;                     // next 60 secs start time

    static uint64 sendLastPacketCount = 0;
    static uint64 sendLastPacketBytes = 0;

    time_t cur_time = time(NULL);

    if((cur_time - lastTime) < 60)
    {
        sendPacketCount+=1;
        sendPacketBytes+=packet->size();

        sendLastPacketCount+=1;
        sendLastPacketBytes+=packet->size();
    }
    else
    {
        uint64 minTime = uint64(cur_time - lastTime);
        uint64 fullTime = uint64(lastTime - firstTime);
        sLog.outDetail("Send all time packets count: " I64FMTD " bytes: " I64FMTD " avr.count/sec: %f avr.bytes/sec: %f time: %u",sendPacketCount,sendPacketBytes,float(sendPacketCount)/fullTime,float(sendPacketBytes)/fullTime,uint32(fullTime));
        sLog.outDetail("Send last min packets count: " I64FMTD " bytes: " I64FMTD " avr.count/sec: %f avr.bytes/sec: %f",sendLastPacketCount,sendLastPacketBytes,float(sendLastPacketCount)/minTime,float(sendLastPacketBytes)/minTime);

        lastTime = cur_time;
        sendLastPacketCount = 1;
        sendLastPacketBytes = packet->wpos();               // wpos is real written size
    }

    #endif                                                  // !MANGOS_DEBUG

    // Delay packets about arena fighters if we are spectator
    if (_player && _player->IsInWorld() && _player->isSpectator() && !withDelayed) {
        switch (packet->GetOpcode()) {
        case MSG_MOVE_START_FORWARD:
        case MSG_MOVE_START_BACKWARD:
        case MSG_MOVE_STOP:
        case MSG_MOVE_START_STRAFE_LEFT:
        case MSG_MOVE_START_STRAFE_RIGHT:
        case MSG_MOVE_STOP_STRAFE:
        case MSG_MOVE_JUMP:
        case MSG_MOVE_START_TURN_LEFT:
        case MSG_MOVE_START_TURN_RIGHT:
        case MSG_MOVE_STOP_TURN:
        case MSG_MOVE_START_PITCH_UP:
        case MSG_MOVE_START_PITCH_DOWN:
        case MSG_MOVE_STOP_PITCH:
        case MSG_MOVE_SET_RUN_MODE:
        case MSG_MOVE_SET_WALK_MODE:
        case MSG_MOVE_FALL_LAND:
        case MSG_MOVE_START_SWIM:
        case MSG_MOVE_STOP_SWIM:
        case MSG_MOVE_SET_FACING:
        case MSG_MOVE_SET_PITCH:
        case MSG_MOVE_HEARTBEAT:
        case CMSG_MOVE_FALL_RESET:
        case CMSG_MOVE_SET_FLY:
        case MSG_MOVE_START_ASCEND:
        case MSG_MOVE_STOP_ASCEND:
        case CMSG_MOVE_CHNG_TRANSPORT:
        case MSG_MOVE_START_DESCEND:
        case SMSG_FORCE_WALK_SPEED_CHANGE:
        case SMSG_FORCE_RUN_SPEED_CHANGE:
        case SMSG_FORCE_RUN_BACK_SPEED_CHANGE:
        case SMSG_FORCE_SWIM_SPEED_CHANGE:
        case SMSG_FORCE_SWIM_BACK_SPEED_CHANGE:
        case SMSG_FORCE_TURN_RATE_CHANGE:
        case SMSG_FORCE_FLIGHT_SPEED_CHANGE:
        case SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE: // For these, pack guid is at the beginning of the data
            // Nothing to do, server doesn't send movement data for ourself
        case SMSG_SPELLSTEALLOG: // Target pack guid is second but we don't care, these packets can only originate from fighters since we cannot do anything but wander
        case SMSG_SPELLINSTAKILLLOG:
        case SMSG_PLAY_SPELL_IMPACT:
        case SMSG_SPELLLOGMISS:
        case SMSG_SPELLLOGEXECUTE:
        case SMSG_PERIODICAURALOG:
        case SMSG_SPELLDAMAGESHIELD:
        case SMSG_SPELLNONMELEEDAMAGELOG:
        case SMSG_RESURRECT_FAILED:
        case SMSG_PROCRESIST:
        case SMSG_DISPEL_FAILED:
        case SMSG_SPELLORDAMAGE_IMMUNE:
        case SMSG_SPELLDISPELLOG:
        case SMSG_DAMAGE_CALC_LOG:
        case SMSG_STANDSTATE_UPDATE:
        case SMSG_UPDATE_WORLD_STATE:  
        case SMSG_PET_CAST_FAILED:
        case SMSG_ATTACKSTART:
        case SMSG_ATTACKSTOP:
        case SMSG_SPELLHEALLOG:
        case SMSG_SPELLENERGIZELOG:
        case SMSG_MOUNTRESULT:
        case SMSG_DISMOUNTRESULT:
        case SMSG_AURACASTLOG:
        case SMSG_SPELL_DELAYED:
        case SMSG_PLAY_SPELL_VISUAL:
        case SMSG_PARTYKILLLOG:
        case SMSG_DESTROY_OBJECT:
        case SMSG_MONSTER_MOVE:
        case SMSG_FORCE_MOVE_ROOT:
        case SMSG_FORCE_MOVE_UNROOT:
        case SMSG_MOVE_KNOCK_BACK:
        case SMSG_MOVE_SET_HOVER:
        case SMSG_MOVE_UNSET_HOVER:
        case SMSG_MOVE_FEATHER_FALL:
        case SMSG_MOVE_NORMAL_FALL:
        case SMSG_EMOTE:
        case SMSG_TEXT_EMOTE:
        case SMSG_CAST_FAILED:
        case SMSG_SPELL_START:
        case SMSG_SPELL_GO:
        case SMSG_SPELL_FAILURE:
        case SMSG_SPELL_COOLDOWN:
        case SMSG_COOLDOWN_EVENT:
        case SMSG_UPDATE_AURA_DURATION:
        case CMSG_STANDSTATECHANGE:
        //case SMSG_UPDATE_OBJECT:
        //case SMSG_COMPRESSED_UPDATE_OBJECT:
        {
            DelayedPacket dp;
            dp.pkt = *packet;
            dp.time = getMSTime();
            m_delayedPackets.push(dp);
            return;
        }
        default:
            break;
        }
    }
    
    if (m_Socket->SendPacket (*packet) == -1)
        m_Socket->CloseSocket ();
}

void WorldSession::SendPacketDelayed(WorldPacket const* packet)
{
    DelayedPacket dp;
    dp.pkt = *packet;
    dp.time = getMSTime();
    m_delayedPackets.push(dp);
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket* new_packet)
{
    _recvQueue.add(new_packet);
}

/// Logging helper for unexpected opcodes
void WorldSession::logUnexpectedOpcode(WorldPacket* packet, const char *reason)
{
    sLog.outError( "SESSION: received unexpected opcode %s (0x%.4X) %s",
        LookupOpcodeName(packet->GetOpcode()),
        packet->GetOpcode(),
        reason);
}

/// Update the WorldSession (triggered by World update)
bool WorldSession::Update(uint32 diff, PacketFilter& updater)
{
    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// not process packets if socket already closed
    WorldPacket *packet;
    while (!_recvQueue.empty() && m_Socket && !m_Socket->IsClosed () && _recvQueue.next(packet, updater))
    {
        /*#if 1
        sLog.outError( "MOEP: %s (0x%.4X)",
                        LookupOpcodeName(packet->GetOpcode()),
                        packet->GetOpcode());
        #endif*/

        if(packet->GetOpcode() >= NUM_MSG_TYPES)
        {
            sLog.outError( "SESSION: received non-existed opcode %s (0x%.4X)",
                LookupOpcodeName(packet->GetOpcode()),
                packet->GetOpcode());
        }
        else
        {
            OpcodeHandler& opHandle = opcodeTable[packet->GetOpcode()];
            switch (opHandle.status)
            {
                case STATUS_LOGGEDIN:
                    if(!_player)
                    {
                        // skip STATUS_LOGGEDIN opcode unexpected errors if player logout sometime ago - this can be network lag delayed packets
                        if(!m_playerRecentlyLogout)
                            logUnexpectedOpcode(packet, "the player has not logged in yet");
                    }
                    else if(_player->IsInWorld())
                        (this->*opHandle.handler)(*packet);
                    // lag can cause STATUS_LOGGEDIN opcodes to arrive after the player started a transfer
                    break;
                case STATUS_TRANSFER_PENDING:
                    if(!_player)
                        logUnexpectedOpcode(packet, "the player has not logged in yet");
                    else if(_player->IsInWorld())
                        logUnexpectedOpcode(packet, "the player is still in world");
                    else
                        (this->*opHandle.handler)(*packet);
                    break;
                case STATUS_AUTHED:
                    // prevent cheating with skip queue wait
                    if(m_inQueue)
                    {
                        logUnexpectedOpcode(packet, "the player not pass queue yet");
                        break;
                    }

                    m_playerRecentlyLogout = false;
                    (this->*opHandle.handler)(*packet);
                    break;
                case STATUS_NEVER:
                    break;
                    /* sLog.outError( "SESSION: received not allowed opcode %s (0x%.4X)",
                        LookupOpcodeName(packet->GetOpcode()),
                        packet->GetOpcode());
                    break; */
            }
        }

        delete packet;
    }
    
    ///- Send delayed packets (arena spectator mode)
    if (!m_delayedPackets.empty()) {
        DelayedPacket dp = m_delayedPackets.front();
        while (getMSTimeDiffToNow(dp.time) > sWorld.getConfig(CONFIG_ARENA_SPECTATOR_DELAY)) {
            SendPacket(&dp.pkt, true);
            m_delayedPackets.pop();
            if (m_delayedPackets.empty())
                break;
            
            dp = m_delayedPackets.front();
        }
    }

    ///- Cleanup socket pointer if need
    if (m_Socket && m_Socket->IsClosed ())
    {
        m_Socket->RemoveReference ();
        m_Socket = NULL;
    }
    
    if (m_Socket && !m_Socket->IsClosed())
    {
    	if (m_Warden)
            m_Warden->Update();
    }

    ///- If necessary, log the player out
    //check if we are safe to proceed with logout
    //logout procedure should happen only in World::UpdateSessions() method!!!
    if(updater.ProcessLogout())
    {
        ///- If necessary, log the player out
        time_t currTime = time(NULL);
        if (!m_Socket || (ShouldLogOut(currTime) && !m_playerLoading))
            LogoutPlayer(true);

        if (!m_Socket)
            return false;                                       //Will remove this session from the world session map
    }

    return true;
}

void WorldSession::InitWarden(BigNumber *K, std::string os)
{
    if (os == "Win" || os == "niW")                                        // Windows
        m_Warden = (WardenBase*)new WardenWin();
    else                                                    // MacOS
        m_Warden = (WardenBase*)new WardenMac();

    m_Warden->Init(this, K);
}

/// %Log the player out
void WorldSession::LogoutPlayer(bool Save)
{
    // finish pending transfers before starting the logout
    while(_player && _player->IsBeingTeleported())
        HandleMoveWorldportAckOpcode();

    m_playerLogout = true;

    if (_player)
    {
        if (uint64 lguid = GetPlayer()->GetLootGUID())
            DoLootRelease(lguid);

        ///- If the player just died before logging out, make him appear as a ghost
        //FIXME: logout must be delayed in case lost connection with client in time of combat
        if (_player->GetDeathTimer())
        {
            _player->getHostilRefManager().deleteReferences();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }
        else if (!_player->getAttackers().empty())
        {
            _player->CombatStop();
            _player->getHostilRefManager().setOnlineOfflineState(false);
            _player->RemoveAllAurasOnDeath();

            // build set of player who attack _player or who have pet attacking of _player
            std::set<Player*> aset;
            for(Unit::AttackerSet::const_iterator itr = _player->getAttackers().begin(); itr != _player->getAttackers().end(); ++itr)
            {
                Unit* owner = (*itr)->GetOwner();           // including player controlled case
                if(owner)
                {
                    if(owner->GetTypeId()==TYPEID_PLAYER)
                        aset.insert(owner->ToPlayer());
                }
                else
                if((*itr)->GetTypeId()==TYPEID_PLAYER)
                    aset.insert((*itr)->ToPlayer());
            }

            _player->SetPvPDeath(!aset.empty());
            _player->KillPlayer();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();

            // give honor to all attackers from set like group case
            for(std::set<Player*>::const_iterator itr = aset.begin(); itr != aset.end(); ++itr)
                (*itr)->RewardHonor(_player,aset.size());

            // give bg rewards and update counters like kill by first from attackers
            // this can't be called for all attackers.
            if(!aset.empty())
                if(BattleGround *bg = _player->GetBattleGround())
                    bg->HandleKillPlayer(_player,*aset.begin());
        }
        else if(_player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        {
            // this will kill character by SPELL_AURA_SPIRIT_OF_REDEMPTION
            _player->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);
            //_player->SetDeathPvP(*); set at SPELL_AURA_SPIRIT_OF_REDEMPTION apply time
            _player->KillPlayer();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }

        //drop a flag if player is carrying it
        if(BattleGround *bg = _player->GetBattleGround())
        {
            if (!bg->isArena())
                bg->EventPlayerLoggedOut(_player);

            _player->LeaveBattleground();
        }

        sOutdoorPvPMgr.HandlePlayerLeaveZone(_player,_player->GetZoneId());

        for (int i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            if(int32 bgTypeId = _player->GetBattleGroundQueueId(i))
            {
                _player->RemoveBattleGroundQueueId(bgTypeId);
                sBattleGroundMgr.m_BattleGroundQueues[ bgTypeId ].RemovePlayer(_player->GetGUID(), true);
            }
        }

        ///- If the player is in a guild, update the guild roster and broadcast a logout message to other guild members
        Guild *guild = objmgr.GetGuildById(_player->GetGuildId());
        if(guild)
        {
            guild->LoadPlayerStatsByGuid(_player->GetGUID());
            guild->UpdateLogoutTime(_player->GetGUID());

            WorldPacket data(SMSG_GUILD_EVENT, (1+1+12+8)); // name limited to 12 in character table.
            data<<(uint8)GE_SIGNED_OFF;
            data<<(uint8)1;
            data<<_player->GetName();
            data<<_player->GetGUID();
            guild->BroadcastPacket(&data);
            if (guild->GetId() == sConfig.GetIntDefault("IRC.Guild.Id", 0))
                sIRC.HandleGameChannelActivity("de guilde", _player->GetName(), _player->GetSession()->GetSecurity(), _player->GetTeam(), WOW_CHAN_LEAVE);
        }

        ///- Remove pet
        _player->RemovePet(NULL,PET_SAVE_AS_CURRENT, true);

        ///- empty buyback items and save the player in the database
        // some save parts only correctly work in case player present in map/player_lists (pets, etc)
        if(Save)
        {
            uint32 eslot;
            for(int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; j++)
            {
                eslot = j - BUYBACK_SLOT_START;
                _player->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+eslot*2,0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+eslot,0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+eslot,0);
            }
            _player->SaveToDB();
        }

        ///- Leave all channels before player delete...
        _player->CleanupChannels();

        ///- If the player is in a group (or invited), remove him. If the group if then only 1 person, disband the group.
        _player->UninviteFromGroup();

        // remove player from the group if he is:
        // a) in group; b) not in raid group; c) logging out normally (not being kicked or disconnected)
        //if(_player->GetGroup() && !_player->GetGroup()->isRaidGroup() && m_Socket)
        //    _player->RemoveFromGroup();

        ///- Remove the player from the world
        // the player may not be in the world when logging out
        // e.g if he got disconnected during a transfer to another map
        // calls to GetMap in this case may cause crashes
        if(_player->IsInWorld()) _player->GetMap()->Remove(_player, false);
        // RemoveFromWorld does cleanup that requires the player to be in the accessor
        ObjectAccessor::Instance().RemoveObject(_player);

        ///- Inform the group about leaving and send update to other members
        if(_player->GetGroup())
        {
            _player->GetGroup()->CheckLeader(_player->GetGUID(), true); //logout check leader
            _player->GetGroup()->SendUpdate();
        }


        ///- Broadcast a logout message to the player's friends
        sSocialMgr.SendFriendStatus(_player, FRIEND_OFFLINE, _player->GetGUIDLow(), true);

        ///- Delete the player object
        _player->CleanupsBeforeDelete();                    // do some cleanup before deleting to prevent crash at crossreferences to already deleted data

        sSocialMgr.RemovePlayerSocial (_player->GetGUIDLow ());
        delete _player;
        _player = NULL;

        ///- Send the 'logout complete' packet to the client
        WorldPacket data( SMSG_LOGOUT_COMPLETE, 0 );
        SendPacket( &data );

        ///- Since each account can only have one online character at any given time, ensure all characters for active account are marked as offline
        //No SQL injection as AccountId is uint32
        CharacterDatabase.PExecute("UPDATE characters SET online = 0 WHERE account = '%u'",
            GetAccountId());
        sLog.outDebug( "SESSION: Sent SMSG_LOGOUT_COMPLETE Message" );
    }
    
    //Hook for OnLogout Event
    sScriptMgr.OnLogout(_player);

    m_playerLogout = false;
    m_playerRecentlyLogout = true;
    LogoutRequest(0);
}

/// Kick a player out of the World
void WorldSession::KickPlayer()
{
    if (m_Socket)
        m_Socket->CloseSocket ();
}

/// Cancel channeling handler

void WorldSession::SendAreaTriggerMessage(const char* Text, ...)
{
    va_list ap;
    char szStr [1024];
    szStr[0] = '\0';

    va_start(ap, Text);
    vsnprintf( szStr, 1024, Text, ap );
    va_end(ap);

    uint32 length = strlen(szStr)+1;
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 4+length);
    data << length;
    data << szStr;
    SendPacket(&data);
}

void WorldSession::SendNotification(const char *format,...)
{
    if(format)
    {
        va_list ap;
        char szStr [1024];
        szStr[0] = '\0';
        va_start(ap, format);
        vsnprintf( szStr, 1024, format, ap );
        va_end(ap);

        WorldPacket data(SMSG_NOTIFICATION, (strlen(szStr)+1));
        data << szStr;
        SendPacket(&data);
    }
}

void WorldSession::SendNotification(int32 string_id,...)
{
    char const* format = GetTrinityString(string_id);
    if(format)
    {
        va_list ap;
        char szStr [1024];
        szStr[0] = '\0';
        va_start(ap, string_id);
        vsnprintf( szStr, 1024, format, ap );
        va_end(ap);

        WorldPacket data(SMSG_NOTIFICATION, (strlen(szStr)+1));
        data << szStr;
        SendPacket(&data);
    }
}

const char * WorldSession::GetTrinityString( int32 entry ) const
{
    return objmgr.GetTrinityString(entry,GetSessionDbLocaleIndex());
}

void WorldSession::Handle_NULL( WorldPacket& recvPacket )
{
    sLog.outError( "SESSION: received unhandled opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_EarlyProccess( WorldPacket& recvPacket )
{
    sLog.outError( "SESSION: received opcode %s (0x%.4X) that must be processed in WorldSocket::OnRead",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_ServerSide( WorldPacket& recvPacket )
{
    sLog.outError( "SESSION: received server-side opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_Deprecated( WorldPacket& recvPacket )
{
    sLog.outError( "SESSION: received deprecated opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::SendAuthWaitQue(uint32 position)
{
    if(position == 0)
    {
        WorldPacket packet( SMSG_AUTH_RESPONSE, 1 );
        packet << uint8( AUTH_OK );
        SendPacket(&packet);
    }
    else
    {
        WorldPacket packet( SMSG_AUTH_RESPONSE, 5 );
        packet << uint8( AUTH_WAIT_QUEUE );
        packet << uint32 (position);
        SendPacket(&packet);
    }
}

void WorldSession::ReadMovementInfo(WorldPacket &data, MovementInfo *mi, uint32* flags)
{
    data >> *flags;
    data >> mi->unk1;
    data >> mi->time;
    data >> mi->x;
    data >> mi->y;
    data >> mi->z;
    data >> mi->o;

    if ((*flags) & MOVEMENTFLAG_ONTRANSPORT)
    {
        data >> mi->t_guid;
        data >> mi->t_x;
        data >> mi->t_y;
        data >> mi->t_z;
        data >> mi->t_o;
        data >> mi->t_time;
    }

    if ((*flags) & (MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING2))
        data >> mi->s_pitch;

    data >> mi->fallTime;

    if ((*flags) & MOVEMENTFLAG_JUMPING)
    {
        data >> mi->j_unk;
        data >> mi->j_sinAngle;
        data >> mi->j_cosAngle;
        data >> mi->j_xyspeed;
    }

    if ((*flags) & MOVEMENTFLAG_SPLINE_ELEVATION)
        data >> mi->u_unk1;
}
