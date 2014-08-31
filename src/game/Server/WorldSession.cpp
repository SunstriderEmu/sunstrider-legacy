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
#include "BattlegroundMgr.h"
#include "OutdoorPvPMgr.h"
#include "Language.h"                                       // for CMSG_CANCEL_MOUNT_AURA handler
#include "Chat.h"
#include "SocialMgr.h"
#include "ScriptCalls.h"
#include "ScriptMgr.h"
#include "IRCMgr.h"
#include "WardenWin.h"
#include "WardenMac.h"
#include "BigNumber.h"

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
WorldSession::WorldSession(uint32 id,  std::shared_ptr<WorldSocket> sock, AccountTypes sec, uint8 expansion, time_t mute_time, LocaleConstant locale, uint32 recruiter, bool isARecruiter, bool mailChange) :
LookingForGroup_auto_join(false), 
LookingForGroup_auto_add(false), 
m_muteTime(mute_time),
_player(NULL), 
m_Socket(sock),
_security(sec),
_accountId(id), 
m_expansion(expansion),
m_sessionDbcLocale(sWorld->GetAvailableDbcLocale(locale)), 
m_sessionDbLocaleIndex(sObjectMgr->GetIndexForLocale(locale)),
_logoutTime(0), 
m_inQueue(false), 
m_playerLoading(false), 
m_playerLogout(false), 
m_playerRecentlyLogout(false), 
m_latency(0), 
m_clientTimeDelay(0),
m_mailChange(mailChange),
_Warden(NULL), 
lastCheatWarn(time(NULL)),
expireTime(60000) // 1 min after socket loss, session is deleted
{
    if (sock)
    {
        m_Address = sock->GetRemoteIpAddress().to_string();
        ResetTimeOutTime();
        LoginDatabase.PExecute("UPDATE account SET online = 1 WHERE id = %u;", GetAccountId());
    }

    InitializeQueryCallbackParameters();
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
        m_Socket = nullptr;
    }
    
    if (_Warden)
        delete _Warden;

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
    TC_LOG_ERROR("FIXME","Client (account %u) send packet %s (%u) with size %u but expected %u (attempt crash server?), skipped",
        GetAccountId(),LookupOpcodeName(packet.GetOpcode()),packet.GetOpcode(),packet.size(),size);
}

/// Get the player name
std::string const& WorldSession::GetPlayerName() const
{
    return GetPlayer() ? GetPlayer()->GetName() : "<none>";
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket const* packet)
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
        TC_LOG_DEBUG("misc","Send all time packets count: " UI64FMTD " bytes: " UI64FMTD " avr.count/sec: %f avr.bytes/sec: %f time: %u",sendPacketCount,sendPacketBytes,float(sendPacketCount)/fullTime,float(sendPacketBytes)/fullTime,uint32(fullTime));
        TC_LOG_DEBUG("misc","Send last min packets count: " UI64FMTD " bytes: " UI64FMTD " avr.count/sec: %f avr.bytes/sec: %f",sendLastPacketCount,sendLastPacketBytes,float(sendLastPacketCount)/minTime,float(sendLastPacketBytes)/minTime);

        lastTime = cur_time;
        sendLastPacketCount = 1;
        sendLastPacketBytes = packet->wpos();               // wpos is real written size
    }

    #endif                                                  // !MANGOS_DEBUG

    sScriptMgr->OnPacketSend(this, *packet);

    m_Socket->AsyncWrite(*packet);
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket* new_packet)
{
    _recvQueue.add(new_packet);
}

/// Logging helper for unexpected opcodes
void WorldSession::logUnexpectedOpcode(WorldPacket* packet, const char *reason)
{
    TC_LOG_DEBUG("FIXME", "SESSION: received unexpected opcode %s (0x%.4X) %s",
        LookupOpcodeName(packet->GetOpcode()),
        packet->GetOpcode(),
        reason);
}

/// Update the WorldSession (triggered by World update)
bool WorldSession::Update(uint32 diff, PacketFilter& updater)
{
    /// Update Timeout timer.
    UpdateTimeOutTime(diff);

    ///- Before we process anything:
    /// If necessary, kick the player from the character select screen
    if (IsConnectionIdle())
        m_Socket->CloseSocket();

    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// not process packets if socket already closed
    WorldPacket* packet = nullptr;

    //! Delete packet after processing by default
    bool deletePacket = true;
    //! To prevent infinite loop
    WorldPacket* firstDelayedPacket = NULL;

    //! If _recvQueue.peek() == firstDelayedPacket it means that in this Update call, we've processed all
    //! *properly timed* packets, and we're now at the part of the queue where we find
    //! delayed packets that were re-enqueued due to improper timing. To prevent an infinite
    //! loop caused by re-enqueueing the same packets over and over again, we stop updating this session
    //! and continue updating others. The re-enqueued packets will be handled in the next Update call for this session.
    uint32 processedPackets = 0;

    while (!_recvQueue.empty() && m_Socket && _recvQueue.peek(true) != firstDelayedPacket && _recvQueue.next(packet, updater))
    {
        #if 0
        TC_LOG_INFO( "network","MOEP: %s (0x%.4X)",
                        LookupOpcodeName(packet->GetOpcode()),
                        packet->GetOpcode());
        #endif

        if(packet->GetOpcode() >= NUM_MSG_TYPES)
        {
            TC_LOG_ERROR( "network","SESSION: received non-existed opcode %s (0x%.4X)",
                LookupOpcodeName(packet->GetOpcode()),
                packet->GetOpcode());
            sScriptMgr->OnUnknownPacketReceive(this, *packet);
        }
        else
        {
            OpcodeHandler& opHandle = opcodeTable[packet->GetOpcode()];
            try
            {
            switch (opHandle.status)
            {
                case STATUS_LOGGEDIN:
                    if(!_player)
                    {
                        // skip STATUS_LOGGEDIN opcode unexpected errors if player logout sometime ago - this can be network lag delayed packets
                        if(!m_playerRecentlyLogout)
                        {
                            //! Prevent infinite loop
                                if (!firstDelayedPacket)
                                    firstDelayedPacket = packet;
                                //! Because checking a bool is faster than reallocating memory
                                deletePacket = false;
                                QueuePacket(packet);
                                //! Log
                                TC_LOG_DEBUG("network", "Re-enqueueing packet with opcode %s with with status STATUS_LOGGEDIN. "
                                    "Player is currently not in world yet.", GetOpcodeNameForLogging(packet->GetOpcode()).c_str());
                         //   logUnexpectedOpcode(packet, "the player has not logged in yet");
                        }
                    }
                    else if(_player->IsInWorld())
                    {
                        sScriptMgr->OnPacketReceive(this, *packet);
                        (this->*opHandle.handler)(*packet);
                        LogUnprocessedTail(packet);
                    }
                    // lag can cause STATUS_LOGGEDIN opcodes to arrive after the player started a transfer
                    break;
                case STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT:
                        if (!_player && !m_playerRecentlyLogout && !m_playerLogout) // There's a short delay between _player = null and m_playerRecentlyLogout = true during logout
                            LogUnexpectedOpcode(packet, "STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT",
                                "the player has not logged in yet and not recently logout");
                        else
                        {
                            // not expected _player or must checked in packet handler
                            sScriptMgr->OnPacketReceive(this, *packet);
                            (this->*opHandle.handler)(*packet);
                            LogUnprocessedTail(packet);
                        }
                        break;
                case STATUS_TRANSFER:
                    if(!_player)
                        logUnexpectedOpcode(packet, "the player has not logged in yet");
                    else if(_player->IsInWorld())
                        logUnexpectedOpcode(packet, "the player is still in world");
                    else
                    {
                        sScriptMgr->OnPacketReceive(this, *packet);
                        (this->*opHandle.handler)(*packet);
                        LogUnprocessedTail(packet);
                    }
                    break;
                case STATUS_AUTHED:
                    // prevent cheating with skip queue wait
                    if(m_inQueue)
                    {
                        logUnexpectedOpcode(packet, "the player not pass queue yet");
                        break;
                    }

                    // some auth opcodes can be recieved before STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT opcodes
                    // however when we recieve CMSG_CHAR_ENUM we are surely no longer during the logout process.
                    if (packet->GetOpcode() == CMSG_CHAR_ENUM)
                        m_playerRecentlyLogout = false;

                    sScriptMgr->OnPacketReceive(this, *packet);
                    (this->*opHandle.handler)(*packet);
                    LogUnprocessedTail(packet);
                    break;
                case STATUS_NEVER:
                    TC_LOG_ERROR("network.opcode", "Received not allowed opcode %s from %s", GetOpcodeNameForLogging(packet->GetOpcode()).c_str()
                            , GetPlayerInfo().c_str());
                     break;
                case STATUS_UNHANDLED:
                    TC_LOG_DEBUG("network.opcode", "Received not handled opcode %s from %s", GetOpcodeNameForLogging(packet->GetOpcode()).c_str()
                            , GetPlayerInfo().c_str());
                    break;
            }
            }
            catch (ByteBufferException const&)
            {
                TC_LOG_ERROR("misc", "WorldSession::Update ByteBufferException occured while parsing a packet (opcode: %u) from client %s, accountid=%i. Skipped packet.",
                        packet->GetOpcode(), GetRemoteAddress().c_str(), GetAccountId());
                packet->hexlike();
            }
        }

        if (deletePacket)
            delete packet;

        deletePacket = true;

#define MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE 100
        processedPackets++;

        //process only a max amout of packets in 1 Update() call.
        //Any leftover will be processed in next update
        if (processedPackets > MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE)
            break;
    }

    if (m_Socket && m_Socket->IsOpen() && _Warden)
        _Warden->Update();

    ProcessQueryCallbacks();

    ///- If necessary, log the player out
    //check if we are safe to proceed with logout
    //logout procedure should happen only in World::UpdateSessions() method!!!
    if(updater.ProcessLogout())
    {
        ///- If necessary, log the player out
        time_t currTime = time(NULL);
        if (!m_Socket || (ShouldLogOut(currTime) && !m_playerLoading))
            LogoutPlayer(true);

        ///- Cleanup socket pointer if need
        if (m_Socket && !m_Socket->IsOpen())
        {
            expireTime -= expireTime > diff ? diff : expireTime;
            if (expireTime < diff || forceExit || !GetPlayer())
            {
                m_Socket = nullptr;
            }
        }

        if (!m_Socket)
            return false;                                       //Will remove this session from the world session map
    }

    return true;
}


void WorldSession::ProcessQueryCallbacks()
{
    PreparedQueryResult result;

    //! HandleCharEnumOpcode
    if (_charEnumCallback.valid() && _charEnumCallback.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        result = _charEnumCallback.get();
        HandleCharEnum(result);
    }

    if (_charCreateCallback.IsReady())
    {
        _charCreateCallback.GetResult(result);
        HandleCharCreateCallback(result, _charCreateCallback.GetParam());
    }

    //! HandlePlayerLoginOpcode
    if (_charLoginCallback.valid() && _charLoginCallback.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        SQLQueryHolder* param = _charLoginCallback.get();
        HandlePlayerLogin((LoginQueryHolder*)param);
    }

    //! HandleAddFriendOpcode
    if (_addFriendCallback.IsReady())
    {
        std::string param = _addFriendCallback.GetParam();
        _addFriendCallback.GetResult(result);
        HandleAddFriendOpcodeCallBack(result, param);
        _addFriendCallback.FreeResult();
    }

    //- HandleCharRenameOpcode
    if (_charRenameCallback.IsReady())
    {
        std::string param = _charRenameCallback.GetParam();
        _charRenameCallback.GetResult(result);
        HandleCharRenameOpcodeCallBack(result, param);
        _charRenameCallback.FreeResult();
    }

    //- HandleCharAddIgnoreOpcode
    if (_addIgnoreCallback.valid() && _addIgnoreCallback.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        result = _addIgnoreCallback.get();
        HandleAddIgnoreOpcodeCallBack(result);
    }

    //- SendStabledPet
    if (_sendStabledPetCallback.IsReady())
    {
        uint64 param = _sendStabledPetCallback.GetParam();
        _sendStabledPetCallback.GetResult(result);
        SendStablePetCallback(result, param);
        _sendStabledPetCallback.FreeResult();
    }

    //- HandleStablePet
    if (_stablePetCallback.valid() && _stablePetCallback.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        result = _stablePetCallback.get();
        HandleStablePetCallback(result);
    }

    //- HandleUnstablePet
    if (_unstablePetCallback.IsReady())
    {
        uint32 param = _unstablePetCallback.GetParam();
        _unstablePetCallback.GetResult(result);
        HandleUnstablePetCallback(result, param);
        _unstablePetCallback.FreeResult();
    }

    //- HandleStableSwapPet
    if (_stableSwapCallback.IsReady())
    {
        uint32 param = _stableSwapCallback.GetParam();
        _stableSwapCallback.GetResult(result);
        HandleStableSwapPetCallback(result, param);
        _stableSwapCallback.FreeResult();
    }
}

void WorldSession::InitWarden(BigNumber *K, std::string os)
{
    if (os == "Win" || os == "niW")                                        // Windows
        _Warden = (WardenBase*)new WardenWin();
    else                                                    // MacOS
        _Warden = (WardenBase*)new WardenMac();

    _Warden->Init(this, K);
}

/// %Log the player out
void WorldSession::LogoutPlayer(bool Save)
{
    // finish pending transfers before starting the logout
    while (_player && _player->IsBeingTeleportedFar())
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
            _player->GetHostilRefManager().deleteReferences();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }
        else if (!_player->getAttackers().empty())
        {
            _player->CombatStop();
            _player->GetHostilRefManager().setOnlineOfflineState(false);
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
                if(Battleground *bg = _player->GetBattleground())
                    bg->HandleKillPlayer(_player,*aset.begin());
        }
        else if(_player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        {
            // this will kill character by SPELL_AURA_SPIRIT_OF_REDEMPTION
            _player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
            //_player->SetDeathPvP(*); set at SPELL_AURA_SPIRIT_OF_REDEMPTION apply time
            _player->KillPlayer();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }

        //drop a flag if player is carrying it
        if(Battleground *bg = _player->GetBattleground())
        {
            if (!bg->isArena())
                bg->EventPlayerLoggedOut(_player);
            
            if (bg->isArena())
                _player->LeaveBattleground();
        }

        sOutdoorPvPMgr.HandlePlayerLeaveZone(_player,_player->GetZoneId());

        for (int i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            if(int32 bgTypeId = _player->GetBattlegroundQueueId(i))
            {
                _player->RemoveBattlegroundQueueId(bgTypeId);
                sBattlegroundMgr.m_BattlegroundQueues[ bgTypeId ].RemovePlayer(_player->GetGUID(), true);
            }
        }

        // Repop at GraveYard or other player far teleport will prevent saving player because of not present map
        // Teleport player immediately for correct player save
        while (_player->IsBeingTeleportedFar())
            HandleMoveWorldportAckOpcode();

        ///- If the player is in a guild, update the guild roster and broadcast a logout message to other guild members
        Guild *guild = sObjectMgr->GetGuildById(_player->GetGuildId());
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
            
            if (sWorld->getConfig(CONFIG_IRC_ENABLED))
                sIRCMgr->onIngameGuildLeft(guild->GetId(), guild->GetName(), _player->GetName());
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
        sObjectAccessor->RemoveObject(_player);

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
    }
    
    //Hook for OnLogout Event
    sScriptMgr->OnPlayerLogout(_player);

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
    return sObjectMgr->GetTrinityString(entry,GetSessionDbLocaleIndex());
}

void WorldSession::Handle_NULL( WorldPacket& recvPacket )
{
    TC_LOG_ERROR("FIXME", "SESSION: received unhandled opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_EarlyProccess( WorldPacket& recvPacket )
{
    TC_LOG_ERROR("FIXME", "SESSION: received opcode %s (0x%.4X) that must be processed in WorldSocket::OnRead",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_ServerSide( WorldPacket& recvPacket )
{
    TC_LOG_ERROR("FIXME", "SESSION: received server-side opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_Deprecated( WorldPacket& recvPacket )
{
    TC_LOG_ERROR("FIXME", "SESSION: received deprecated opcode %s (0x%.4X)",
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

void WorldSession::ReadMovementInfo(WorldPacket &data, MovementInfo *mi)
{
    data >> mi->flags;
    data >> mi->flags2;
    data >> mi->time;
    data >> mi->pos.PositionXYZOStream();

    if (mi->HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
    {
        data >> mi->transport.guid;
        data >> mi->transport.pos.PositionXYZOStream();
        data >> mi->transport.time;
    }

    if (mi->HasMovementFlag(MovementFlags(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING)))
        data >> mi->pitch;

    data >> mi->fallTime;

    if (mi->HasMovementFlag(MOVEMENTFLAG_FALLING))
    {
        data >> mi->jump.zspeed;
        data >> mi->jump.sinAngle;
        data >> mi->jump.cosAngle;
        data >> mi->jump.xyspeed;
    }

    if (mi->HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION))
        data >> mi->splineElevation;
}

void WorldSession::WriteMovementInfo(WorldPacket* data, MovementInfo* mi)
{
    *data << mi->flags;
    *data << mi->flags2;
    *data << mi->time;
    *data << mi->pos.PositionXYZOStream();

    if (mi->HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
    {
       *data << mi->transport.guid;
       *data << mi->transport.pos.PositionXYZOStream();
       *data << mi->transport.time;
    }

    if (mi->HasMovementFlag(MovementFlags(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING)))
        *data << mi->pitch;

    *data << mi->fallTime;

    if (mi->HasMovementFlag(MOVEMENTFLAG_FALLING))
    {
        *data << mi->jump.zspeed;
        *data << mi->jump.sinAngle;
        *data << mi->jump.cosAngle;
        *data << mi->jump.xyspeed;
    }

    if (mi->HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION))
        *data << mi->splineElevation;
}

std::string WorldSession::GetLocalizedItemName(const ItemPrototype* proto)
{
    std::string name = proto->Name1;
    int loc_idx = GetSessionDbLocaleIndex();
    if ( loc_idx >= 0 )
    {
        ItemLocale const *il = sObjectMgr->GetItemLocale(proto->ItemId);
        if (il)
        {
            if (il->Name.size() > size_t(loc_idx) && !il->Name[loc_idx].empty())
                name = il->Name[loc_idx];
        }
    }
    return name;
}

std::string WorldSession::GetLocalizedItemName(uint32 itemId)
{
    if(const ItemPrototype* proto = sObjectMgr->GetItemPrototype(itemId))
        return GetLocalizedItemName(proto);
    else
        return std::string();
}

void WorldSession::InitializeQueryCallbackParameters()
{
    // Callback parameters that have pointers in them should be properly
    // initialized to NULL here.
    _charCreateCallback.SetParam(NULL);
}

std::string WorldSession::GetPlayerInfo() const
{
    std::ostringstream ss;

    ss << "[Player: " << GetPlayerName()
       << " (Guid: " << (_player != NULL ? _player->GetGUID() : 0)
       << ", Account: " << GetAccountId() << ")]";

    return ss.str();
}