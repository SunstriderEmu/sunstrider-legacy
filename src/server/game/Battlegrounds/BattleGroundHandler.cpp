
#include "Common.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "Chat.h"
#include "BattleGroundMgr.h"
#include "BattleGroundWS.h"
#include "BattleGroundEY.h"
#include "BattleGround.h"
#include "ArenaTeam.h"
#include "Language.h"
#include "World.h"
#include "GameTime.h"

void WorldSession::HandleBattleMasterHelloOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;

    Creature* unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->IsBattleMaster())                             // it's not battlemaster
        return;

    // Stop the npc if moving
    unit->PauseMovement(sWorld->getIntConfig(CONFIG_CREATURE_STOP_FOR_PLAYER));
    unit->SetHomePosition(unit->GetPosition());

    BattlegroundTypeId bgTypeId = sObjectMgr->GetBattleMasterBG(unit->GetEntry());

    if(!_player->GetBGAccessByLevel(bgTypeId))
    {
        // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    SendBattleGroundList(guid, bgTypeId);
}

void WorldSession::SendBattleGroundList( ObjectGuid guid, BattlegroundTypeId bgTypeId )
{
    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundListPacket(&data, guid, _player, bgTypeId);
    SendPacket( &data );
}

void WorldSession::_HandleBattlegroundJoin(BattlegroundTypeId bgTypeId, uint32 instanceId, bool joinAsGroup)
{
    Group* grp = nullptr;
    bool isPremade = false;

    // can do this, since it's battleground, not arena
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, 0);
#ifdef LICH_KING
    BattlegroundQueueTypeId bgQueueTypeIdRandom = BattlegroundMgr::BGQueueTypeId(BATTLEGROUND_RB, 0);
#endif

    // ignore if player is already in BG
    if(_player->InBattleground())
        return;

    // get bg instance or bg template if instance not found
    Battleground * bg = nullptr;
    if(instanceId)
        bg = sBattlegroundMgr->GetBattlegroundThroughClientInstance(instanceId, bgTypeId);

    if (!bg)
        bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);

    if (!bg)
    {
        TC_LOG_ERROR("bg.battleground","Battleground: no available bg / template found");
        return;
    }

    // expected bracket entry
    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->GetLevel());
    if (!bracketEntry)
        return;

    // check queueing conditions
    if(!joinAsGroup)
    {
#ifdef LICH_KING
        if (GetPlayer()->isUsingLfg())
        {
            // player is using dungeon finder or raid finder
            WorldPacket data;
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, ERR_LFG_CANT_USE_BATTLEGROUND);
            GetPlayer()->SendDirectMessage(&data);
            return;
        }

        if (_player->GetBattlegroundQueueIndex(bgQueueTypeIdRandom) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            // player is already in random queue
            WorldPacket data;
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, ERR_IN_RANDOM_BG);
            _player->SendDirectMessage(&data);
            return;
        }

        if (_player->InBattlegroundQueue() && bgTypeId == BATTLEGROUND_RB)
        {
            // player is already in queue, can't start random queue
            WorldPacket data;
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, ERR_IN_NON_RANDOM_BG);
            _player->SendDirectMessage(&data);
            return;
        }
#endif

        // check Deserter debuff
        if( !_player->CanJoinToBattleground(bg) )
        {
            WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            _player->SendDirectMessage(&data);
            return;
        }

        // check if already in queue
        if (_player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;

        // check if has free queue slots
        if (!_player->HasFreeBattlegroundQueueId())
        {
            //NOT SURE FOR BC. Old code only returned here with no feedback to client
            WorldPacket data;
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, ERR_BATTLEGROUND_TOO_MANY_QUEUES);
            _player->SendDirectMessage(&data);
            return;
        }

        // check Freeze debuff
        if (_player->HasAura(9454))
            return;

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);

        GroupQueueInfo* ginfo = bgQueue.AddGroup(_player, nullptr, bgTypeId, bracketEntry, 0, false, isPremade, 0, 0);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        // already checked if queueSlot is valid, now just get it
        uint32 queueSlot = _player->AddBattlegroundQueueId(bgQueueTypeId);

        WorldPacket data;
        // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0, ginfo->ArenaType, 0);
        SendPacket(&data);
        TC_LOG_DEBUG("bg.battleground", "Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s",
            bgQueueTypeId, bgTypeId, _player->GetGUID().GetCounter(), _player->GetName().c_str());
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if(!grp)
            return;
        //must be leader to tag as group
        if(grp->GetLeaderGUID() != _player->GetGUID())
            return;

        GroupJoinBattlegroundResult err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), false, 0);
        isPremade = (grp->GetMembersCount() >= bg->GetMinPlayersPerTeam());

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        GroupQueueInfo* ginfo = nullptr;
        uint32 avgTime = 0;

        if (err > 0)
        {
            TC_LOG_DEBUG("bg.battleground", "Battleground: the following players are joining as group:");
            ginfo = bgQueue.AddGroup(_player, grp, bgTypeId, bracketEntry, 0, false, isPremade, 0, 0);
            avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        }

        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member)
                continue;   // this should never happen

            WorldPacket data;

            if (err <= 0)
            {
                sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, err);
                member->SendDirectMessage(&data);
                continue;
            }

            // add to queue
            uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);

            // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0, ginfo->ArenaType, 0);
            member->SendDirectMessage(&data);
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, err);
            member->SendDirectMessage(&data);
            TC_LOG_DEBUG("bg.battleground", "Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s",
                bgQueueTypeId, bgTypeId, member->GetGUID().GetCounter(), member->GetName().c_str());
        }
        TC_LOG_DEBUG("bg.battleground", "Battleground: group end");
    }
    sBattlegroundMgr->ScheduleQueueUpdate(0, 0, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
}

void WorldSession::HandleBattlemasterJoinOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    uint32 bgTypeId_;
    uint32 instanceId;
    uint8 joinAsGroup;

    //LK OK

    recvData >> guid;                                      // battlemaster guid
    recvData >> bgTypeId_;                                  // battleground type id (DBC id)
    recvData >> instanceId;                                // instance id, 0 if First Available selected
    recvData >> joinAsGroup;                               // join as group

    if (!sBattlemasterListStore.LookupEntry(bgTypeId_))
    {
        TC_LOG_ERROR("network", "Battleground: invalid bgtype (%u) received. possible cheater? player guid %u", bgTypeId_, _player->GetGUID().GetCounter());
        return;
    }

    BattlegroundTypeId bgTypeId = BattlegroundTypeId(bgTypeId_);

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->IsBattleMaster())                             // it's not battlemaster
        return;

    _HandleBattlegroundJoin(BattlegroundTypeId(bgTypeId), instanceId, joinAsGroup);
}

void WorldSession::HandleBattlegroundPlayerPositionsOpcode( WorldPacket & /*recvData*/ )
{
    Battleground *bg = _player->GetBattleground();
    if(!bg)   // can't be received if player not in battleground
        return;

    uint32 flagCarrierCount = 0;

    Player* allianceFlagCarrier = nullptr;
    Player* hordeFlagCarrier = nullptr;
    //Remove this switch when we move GetFlagPickerGUID to Battleground class
    switch (bg->GetTypeID())
    {
    case BATTLEGROUND_WS:
        allianceFlagCarrier = ObjectAccessor::FindPlayer(((BattlegroundWS*)bg)->GetAllianceFlagPickerGUID());
        hordeFlagCarrier    = ObjectAccessor::FindPlayer(((BattlegroundWS*)bg)->GetHordeFlagPickerGUID());
        break;
    case BATTLEGROUND_EY:
        allianceFlagCarrier = ObjectAccessor::FindPlayer(((BattlegroundEY*)bg)->GetFlagPickerGUID()); //not actually always alliance... But not important and keeping this to keep compat with TC
        break;
    }

    if(allianceFlagCarrier)
        ++flagCarrierCount;

    if(hordeFlagCarrier)
        ++flagCarrierCount;

    WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4+16* flagCarrierCount)); //LK OK
    // Used to send several player positions (found used in AV)
    data << uint32(0);  // CGBattlefieldInfo__m_numPlayerPositions
    /*
    for (CGBattlefieldInfo__m_numPlayerPositions)
        data << guid << posx << posy;
    */
    data << flagCarrierCount;
    if(allianceFlagCarrier)
    {
        data << (uint64)allianceFlagCarrier->GetGUID();
        data << (float)allianceFlagCarrier->GetPositionX();
        data << (float)allianceFlagCarrier->GetPositionY();
    }
    if(hordeFlagCarrier)
    {
        data << (uint64)hordeFlagCarrier->GetGUID();
        data << (float)hordeFlagCarrier->GetPositionX();
        data << (float)hordeFlagCarrier->GetPositionY();
    }

    SendPacket(&data);
}

void WorldSession::HandlePVPLogDataOpcode( WorldPacket & /*recvData*/ )
{
//    TC_LOG_DEBUG("network", "WORLD: Recvd MSG_PVP_LOG_DATA Message");

    Battleground *bg = _player->GetBattleground();
    if(!bg || (bg->IsArena() && bg->GetStatus() != STATUS_WAIT_LEAVE))
        return;

    WorldPacket data;
    bg->BuildPvPLogDataPacket(data);
    SendPacket(&data);

//    TC_LOG_DEBUG("network", "WORLD: Sent MSG_PVP_LOG_DATA Message");
}

void WorldSession::HandleBattlefieldListOpcode( WorldPacket &recvData )
{
    uint32 bgTypeId;
    recvData >> bgTypeId;                                  // id from DBC

    uint8 fromWhere = 0;
#ifdef LICH_KING
    recvData >> fromWhere;                                 // 0 - battlemaster (lua: ShowBattlefieldList), 1 - UI (lua: RequestBattlegroundInstanceInfo)

    uint8 canGainXP;
    recvData >> canGainXP;                                 // players with locked xp have their own bg queue on retail
#endif

    if(bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
    {
        TC_LOG_ERROR("bg.battleground","Battleground: invalid bgtype received: %u.", bgTypeId);
        return;
    }

    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgTypeId);

    if (!bl)
    {
        TC_LOG_DEBUG("bg.battleground", "BattlegroundHandler: invalid bgtype (%u) with player (Name: %s, GUID: %u) received.", bgTypeId, _player->GetName().c_str(), _player->GetGUID().GetCounter());
        return;
    }

    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundListPacket(&data, _player->GetGUID(), _player, BattlegroundTypeId(bgTypeId), fromWhere);
    SendPacket( &data );
}

void WorldSession::HandleBattleFieldPortOpcode( WorldPacket &recvData )
{
    uint8 type;                                             // arenatype if arena
    uint8 unk2;                                             // unk, can be 0x0 (may be if was invited?) and 0x1
    uint32 bgTypeId_;                                       // type id from dbc
    uint16 unk;                                             // 0x1F90 constant?
    uint8 action;                                           // enter battle 0x1, leave queue 0x0

    recvData >> type >> unk2 >> bgTypeId_ >> unk >> action; //LK OK
    if (!sBattlemasterListStore.LookupEntry(bgTypeId_))
    {
        TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Invalid BgType!",
            GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action);
        return;
    }

    if (!_player->InBattlegroundQueue())
    {
        TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Player not in queue!",
            GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action);
        return;
    }

    BattlegroundTypeId bgTypeId = BattlegroundTypeId(bgTypeId_);
    if(bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
    {
        //this code block may or may not be needed... this is an old hack
        /*
        TC_LOG_ERROR("network","Battleground: invalid bgtype received: %u.", bgTypeId_);
        // update battleground slots for the player to fix his UI and sent data.
        // this is a HACK, I don't know why the client starts sending invalid packets in the first place.
        // it usually happens with extremely high latency (if debugging / stepping in the code for example)
        if(_player->InBattlegroundQueue())
        {
            // update all queues, send invitation info if player is invited, queue info if queued
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
            {
                uint32 queue_id = _player->GetBattlegroundQueueId(i);
                if(!queue_id)
                    continue;
                auto itrPlayerStatus = sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());
                // if the player is not in queue, contine
                if(itrPlayerStatus == sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end())
                    continue;

                // no group information, this should never happen
                if(!itrPlayerStatus->second.GroupInfo)
                    continue;

                Battleground * bg = nullptr;

                // get possibly needed data from groupinfo
                bgTypeId = itrPlayerStatus->second.GroupInfo->BgTypeId;
                uint8 arenatype = itrPlayerStatus->second.GroupInfo->ArenaType;
                uint8 israted = itrPlayerStatus->second.GroupInfo->IsRated;
                uint8 status = 0;

                if(!itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID)
                {
                    // not invited to bg, get template
                    bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
                    status = STATUS_WAIT_QUEUE;
                }
                else
                {
                    // get the bg we're invited to
                    bg = sBattlegroundMgr->GetBattleground(itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID);
                    status = STATUS_WAIT_JOIN;
                }

                // if bg not found, then continue
                if(!bg)
                    continue;

                // don't invite if already in the instance
                if(_player->InBattleground() && _player->GetBattleground() && _player->GetBattleground()->GetInstanceID() == bg->GetInstanceID())
                    continue;

                // re - invite player with proper data
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, itrPlayerStatus->second.GroupInfo->Team?itrPlayerStatus->second.GroupInfo->Team:_player->GetTeam(), i, status, INVITE_ACCEPT_WAIT_TIME, 0, arenatype, israted);
                SendPacket(&data);
            }
        }
        */
        TC_LOG_ERROR("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Wrong bg type id %u received!",
            GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action, uint32(bgTypeId_));
        return;
    }

    /*
    //get GroupQueueInfo from BattlegroundQueue
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, type);
    BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
    //we must use temporary variable, because GroupQueueInfo pointer can be deleted in BattlegroundQueue::RemovePlayer() function
    GroupQueueInfo ginfo;
    if (!bgQueue.GetPlayerGroupInfoData(_player->GetGUID(), &ginfo))
    {
        TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Player not in queue (No player Group Info)!",
            GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action);
        return;
    }

    // get the bg what we were invited to
    BattlegroundQueue::QueuedPlayersMap::iterator itrPlayerStatus;
    bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bgTypeId,type);
    itrPlayerStatus = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());

    if(itrPlayerStatus == sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end())
    {
        TC_LOG_ERROR("network","Battleground: itrplayerstatus not found.");
        return;
    }
    instanceId = itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID;

    // if action == 1, then instanceId is _required_
    if(!instanceId && action == 1)
    {
        TC_LOG_ERROR("bg.battleground","Battleground: instance not found.");
        return;
    }

    Battleground *bg = sBattlegroundMgr->GetBattleground(instanceId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if(!bg && action == 0)
        bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);

    if(!bg)
    {
        TC_LOG_ERROR("bg.battleground","Battleground: bg not found.");
        return;
    }

    bgTypeId = bg->GetTypeID();

    if(_player->InBattlegroundQueue())
    {
        uint32 queueSlot = 0;
        uint32 team = 0;
        uint32 arenatype = 0;
        uint32 israted = 0;
        uint32 rating = 0;
        uint32 opponentsRating = 0;
        // get the team info from the queue
        auto pitr = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());
        if(pitr !=sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end()
            && pitr->second.GroupInfo )
        {
            team = pitr->second.GroupInfo->Team;
            arenatype = pitr->second.GroupInfo->ArenaType;
            israted = pitr->second.GroupInfo->IsRated;
            rating = pitr->second.GroupInfo->ArenaTeamRating;
            opponentsRating = pitr->second.GroupInfo->OpponentsTeamRating;
        }
        else
        {
            TC_LOG_ERROR("battleground","Battleground: Invalid player queue info!");
            return;
        }
        // if player is trying to enter battleground (not arena) and he has deserter debuff, we must just remove him from queue
        if (arenatype == 0 && !_player->CanJoinToBattleground())
            action = 0;

        WorldPacket data;
        switch(action)
        {
            case 1:                                     // port to battleground
                if(!_player->IsInvitedForBattlegroundQueueType(bgQueueTypeId))
                    return;                                     // cheating?
                // resurrect the player
                if(!_player->IsAlive())
                {
                    _player->ResurrectPlayer(1.0f);
                    _player->SpawnCorpseBones();
                }
                // stop taxi flight at port
                if(_player->IsInFlight())
                {
                    _player->GetMotionMaster()->MovementExpired();
                    _player->CleanupAfterTaxiFlight();
                }

                queueSlot = _player->GetBattlegroundQueueIndex(bgQueueTypeId);
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                _player->SendDirectMessage(&data);
                // remove battleground queue status from BGmgr
                sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), false);
                // this is still needed here if battleground "jumping" shouldn't add deserter debuff
                // also this required to prevent stuck at old battleground after SetBattlegroundId set to new
                if( Battleground *currentBg = _player->GetBattleground() )
                    currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true);

                // set the destination instance id
                _player->SetBattlegroundId(bg->GetInstanceID());
                // set the destination team
                _player->SetBGTeam(team);
                // clear AFK
                if(_player->IsAFK())
                    _player->ToggleAFK();
                sBattlegroundMgr->SendToBattleground(_player, instanceId);
                break;
            case 0:                                     // leave queue
                queueSlot = _player->GetBattlegroundQueueIndex(bgQueueTypeId);
                // if player leaves rated arena match before match start, it is counted as he played but he lost
                if (israted)
                {
                    ArenaTeam * at = sObjectMgr->GetArenaTeamById(team);
                    if (at)
                    {
                        at->MemberLost(_player, opponentsRating);
                        at->SaveToDB();
                    }
                }
                _player->RemoveBattlegroundQueueId(bgQueueTypeId); // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_NONE, 0, 0);
                sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), true);
                // player left queue, we should update it, maybe now his group fits in
                sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].BattlegroundQueueUpdate(bgTypeId,_player->GetBattlegroundQueueIdFromLevel(),arenatype,israted,rating);
                SendPacket(&data);
                break;
            default:
                TC_LOG_ERROR("bg.battleground","Battleground port: unknown action %u", action);
                break;
        }
    }
    */
 //get GroupQueueInfo from BattlegroundQueue
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, type);
    BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
    //we must use temporary variable, because GroupQueueInfo pointer can be deleted in BattlegroundQueue::RemovePlayer() function
    GroupQueueInfo ginfo;
    if (!bgQueue.GetPlayerGroupInfoData(_player->GetGUID(), &ginfo))
    {
        TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Player not in queue (No player Group Info)!",
            GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action);
        return;
    }
    // if action == 1, then instanceId is required
    if (!ginfo.IsInvitedToBGInstanceGUID && action == 1)
    {
        TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Player is not invited to any bg!",
            GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action);
        return;
    }

    Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
    if (!bg)
    {
        if (action)
        {
            TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u. Cant find BG with id %u!",
                GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action, ginfo.IsInvitedToBGInstanceGUID);
            return;
        }

        bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
        {
            TC_LOG_ERROR("network", "BattlegroundHandler: bg_template not found for type id %u.", bgTypeId);
            return;
        }
    }

    TC_LOG_DEBUG("bg.battleground", "CMSG_BATTLEFIELD_PORT %s ArenaType: %u, Unk: %u, BgType: %u, Action: %u.",
        GetPlayerInfo().c_str(), type, unk2, bgTypeId_, action);

    // expected bracket entry
    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->GetLevel());
    if (!bracketEntry)
        return;

    //some checks if player isn't cheating - it is not exactly cheating, but we cannot allow it
    if (action == 1 && ginfo.ArenaType == 0)
    {
        //if player is trying to enter battleground (not arena!) and he has deserter debuff, we must just remove him from queue
        if (!_player->CanJoinToBattleground(bg))
        {
            //send bg command result to show nice message
            WorldPacket data2;
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data2, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            _player->SendDirectMessage(&data2);
            action = 0;
            TC_LOG_DEBUG("bg.battleground", "Player %s (%u) has a deserter debuff, do not port him to battleground!", _player->GetName().c_str(), _player->GetGUID().GetCounter());
        }
        //if player don't match battleground max level, then do not allow him to enter! (this might happen when player leveled up during his waiting in queue
        if (_player->GetLevel() > bg->GetMaxLevel())
        {
            TC_LOG_ERROR("network", "Player %s (%u) has level (%u) higher than maxlevel (%u) of battleground (%u)! Do not port him to battleground!",
                _player->GetName().c_str(), _player->GetGUID().GetCounter(), _player->GetLevel(), bg->GetMaxLevel(), bg->GetTypeID());
            action = 0;
        }
    }
    uint32 queueSlot = _player->GetBattlegroundQueueIndex(bgQueueTypeId);
    WorldPacket data;
    if (action)
    {
        // check Freeze debuff
        if (_player->HasAura(9454))
            return;

        if (!_player->IsInvitedForBattlegroundQueueType(bgQueueTypeId))
            return;                                 // cheating?

        if (!_player->InBattleground())
            _player->SetBattlegroundEntryPoint();

        // resurrect the player
        if (!_player->IsAlive())
        {
            _player->ResurrectPlayer(1.0f);
            _player->SpawnCorpseBones();
        }
        // stop taxi flight at port
        if (_player->IsInFlight())
        {
            _player->GetMotionMaster()->MovementExpired();
            _player->CleanupAfterTaxiFlight();
        }

        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime(), bg->GetArenaType(), ginfo.Team);
        _player->SendDirectMessage(&data);

        // remove battleground queue status from BGmgr
        bgQueue.RemovePlayer(_player->GetGUID(), false);
        // this is still needed here if battleground "jumping" shouldn't add deserter debuff
        // also this is required to prevent stuck at old battleground after SetBattlegroundId set to new
        if (Battleground* currentBg = _player->GetBattleground())
            currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true);

        // set the destination instance id
        _player->SetBattlegroundId(bg->GetInstanceID(), bgTypeId);
        // set the destination team
        _player->SetBGTeam(ginfo.Team);
        // sunstrider: clear AFK
        if (_player->IsAFK())
            _player->ToggleAFK();

        // bg->HandleBeforeTeleportToBattleground(_player);
        sBattlegroundMgr->SendToBattleground(_player, ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
        // add only in HandleMoveWorldPortAck()
        // bg->AddPlayer(_player, team);
        TC_LOG_DEBUG("bg.battleground", "Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.", _player->GetName().c_str(), _player->GetGUID().GetCounter(), bg->GetInstanceID(), bg->GetTypeID(), bgQueueTypeId);
    }
    else // leave queue
    {
        if (bg->IsArena() && bg->GetStatus() > STATUS_WAIT_QUEUE)
            return;
        // if player leaves rated arena match before match start, it is counted as he played but he lost
        if (ginfo.IsRated && ginfo.IsInvitedToBGInstanceGUID)
        {
            ArenaTeam* at = sObjectMgr->GetArenaTeamById(ginfo.Team); //sArenaTeamMgr->GetArenaTeamById(ginfo.Team);
            if (at)
            {
                TC_LOG_DEBUG("bg.battleground", "UPDATING memberLost's personal arena rating for %u by opponents rating: %u, because he has left queue!", _player->GetGUID().GetCounter(), ginfo.OpponentsTeamRating);
                at->MemberLost(_player, ginfo.OpponentsMatchmakerRating);
                at->SaveToDB();
            }
        }
        _player->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_NONE, 0, 0, 0, 0);
        bgQueue.RemovePlayer(_player->GetGUID(), true);
        // player left queue, we should update it - do not update Arena Queue
        if (!ginfo.ArenaType)
            sBattlegroundMgr->ScheduleQueueUpdate(ginfo.ArenaMatchmakerRating, ginfo.ArenaType, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
        SendPacket(&data);
        TC_LOG_DEBUG("bg.battleground", "Battleground: player %s (%u) left queue for bgtype %u, queue type %u.", _player->GetName().c_str(), _player->GetGUID().GetCounter(), bg->GetTypeID(), bgQueueTypeId);

        // track if player refuses to join the BG after being invited
        /* TC
        if (bg->isBattleground() && sWorld->getBoolConfig(CONFIG_BATTLEGROUND_TRACK_DESERTERS) &&
                (bg->GetStatus() == STATUS_IN_PROGRESS || bg->GetStatus() == STATUS_WAIT_JOIN))
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_DESERTER_TRACK);
            stmt->setUInt32(0, _player->GetGUID().GetCounter());
            stmt->setUInt8(1, BG_DESERTION_TYPE_LEAVE_QUEUE);
            CharacterDatabase.Execute(stmt);
        }
        */
    }
}

void WorldSession::HandleBattlefieldLeaveOpcode( WorldPacket & /*recvData*/ )
{
    /*TC has this for LK. Not sure for BC
    recvData.read_skip<uint8>();                           // unk1
    recvData.read_skip<uint8>();                           // unk2
    recvData.read_skip<uint32>();                          // BattlegroundTypeId
    recvData.read_skip<uint16>();                          // unk3
    */

    if(Battleground *bg = _player->GetBattleground())
    {
        // not allow leave battleground in combat
        if(_player->IsInCombat())
            if(bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

        if (bg->isSpectator(_player->GetGUID()))
        {
            _player->CancelSpectate();

            if (_player->TeleportToBGEntryPoint())
            {
                _player->SetSpectate(false);
                bg->RemoveSpectator(_player->GetGUID());
            }
        }
        else
            _player->LeaveBattleground();
    }
}

void WorldSession::HandleBattlefieldStatusOpcode( WorldPacket & /*recvData*/ )
{
    // empty opcode
    TC_LOG_DEBUG("network", "WORLD: Battleground status");

    WorldPacket data;
    // we must update all queues here
    Battleground* bg = nullptr;
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattlegroundQueueTypeId bgQueueTypeId = _player->GetBattlegroundQueueTypeId(i);
        if (!bgQueueTypeId)
            continue;
        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(bgQueueTypeId);
        uint8 arenaType = BattlegroundMgr::BGArenaType(bgQueueTypeId);
        if (bgTypeId == _player->GetBattlegroundTypeId())
        {
            bg = _player->GetBattleground();
            //i cannot check any variable from player class because player class doesn't know if player is in 2v2 / 3v3 or 5v5 arena
            //so i must use bg pointer to get that information
            if (bg && bg->GetArenaType() == arenaType)
            {
                // this line is checked, i only don't know if GetStartTime is changing itself after bg end!
                // send status in Battleground
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, i, STATUS_IN_PROGRESS, bg->GetEndTime(), bg->GetStartTime(), arenaType, _player->GetBGTeam());
                SendPacket(&data);
                continue;
            }
        }
        //we are sending update to player about queue - he can be invited there!
        //get GroupQueueInfo for queue status
        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        GroupQueueInfo ginfo;
        if (!bgQueue.GetPlayerGroupInfoData(_player->GetGUID(), &ginfo))
            continue;
        if (ginfo.IsInvitedToBGInstanceGUID)
        {
            bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
            if (!bg)
                continue;
            uint32 remainingTime = GetMSTimeDiff(GameTime::GetGameTimeMS(), ginfo.RemoveInviteTime);
            // send status invited to Battleground
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, i, STATUS_WAIT_JOIN, remainingTime, 0, arenaType, 0);
            SendPacket(&data);
        }
        else
        {
            bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bg)
                continue;

            // expected bracket entry
            PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->GetLevel());
            if (!bracketEntry)
                continue;

            uint32 avgTime = bgQueue.GetAverageQueueWaitTime(&ginfo, bracketEntry->GetBracketId());
            // send status in Battleground Queue
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, i, STATUS_WAIT_QUEUE, avgTime, GetMSTimeDiff(ginfo.JoinTime, GameTime::GetGameTimeMS()), arenaType, 0);
            SendPacket(&data);
        }
    }
}

//LK OK, but missing battlefield handling
void WorldSession::HandleAreaSpiritHealerQueryOpcode( WorldPacket & recvData )
{
    Battleground *bg = _player->GetBattleground();
    if(!bg)
        return;

    ObjectGuid guid;
    recvData >> guid;

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->IsSpiritService())                            // it's not spirit service
        return;

    sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, guid);
}

//LK OK, but missing battlefield handling
void WorldSession::HandleAreaSpiritHealerQueueOpcode( WorldPacket & recvData )
{
    Battleground *bg = _player->GetBattleground();
    if(!bg)
        return;

    ObjectGuid guid;
    recvData >> guid;

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->IsSpiritService())                            // it's not spirit service
        return;

    bg->AddPlayerToResurrectQueue(guid, _player->GetGUID());
}

//LK OK
void WorldSession::HandleBattlemasterJoinArena( WorldPacket & recvData )
{     
    // ignore if we already in BG or BG queue
    if(_player->InBattleground())
        return;

    ObjectGuid guid;                                            // arena Battlemaster guid
    uint8 arenaslot;                                        // 2v2, 3v3 or 5v5
    uint8 asGroup;                                          // asGroup
    uint8 isRated;                                          // isRated
    Group * grp = nullptr;

    recvData >> guid >> arenaslot >> asGroup >> isRated;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BATTLEMASTER);
    if (!unit)
        return;

    if (isRated && !sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_RATED_ENABLE)) {
        ChatHandler(GetPlayer()).PSendSysMessage(LANG_RATED_ARENA_DISABLED);
        return;
    }
    
    if(!_player->IsGameMaster())
    {
        // Close rated arena during the night to block wintraders
        bool closeAtNight = sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_CLOSE_AT_NIGHT_MASK) & 1;
        bool alsoCloseSkirmish = sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_CLOSE_AT_NIGHT_MASK) & 2;
        time_t curTime = time(nullptr);
        tm localTm = *localtime(&curTime);
        if (closeAtNight && (isRated || alsoCloseSkirmish))
        {
            if (localTm.tm_wday == 0 || localTm.tm_wday == 6) { // Saturday (6) or Sunday (0)
                if (localTm.tm_hour > 3 && localTm.tm_hour < 7) {
                    ChatHandler(GetPlayer()).PSendSysMessage(LANG_RATED_ARENA_CLOSED_DURING_NIGHT);
                    return;
                }
            }
            else {
                if (localTm.tm_hour > 2 && localTm.tm_hour < 8) {
                    ChatHandler(GetPlayer()).PSendSysMessage(LANG_RATED_ARENA_CLOSED_DURING_NIGHT);
                    return;
                }
            }
        }
        //Arena server (WM Tournoi) is open wedsnesday, saturday & sunday from 14 to 22 pm
        if(sWorld->getConfig(CONFIG_ARENASERVER_ENABLED) && sWorld->getConfig(CONFIG_ARENASERVER_USE_CLOSESCHEDULE)) 
        { 
            if ( (localTm.tm_wday != 3 && localTm.tm_wday != 6 && localTm.tm_wday != 0)
                 || localTm.tm_hour < 14 
                 || localTm.tm_hour > 22
                ) 
            {
                ChatHandler(GetPlayer()).PSendSysMessage(LANG_ARENASERVER_CLOSED);
                return;
            }
        }
    }

    uint8 arenatype = 0;
    uint32 arenaRating = 0;
    uint32 matchmakerRating = 0;

    switch(arenaslot)
    {
        case 0:
            arenatype = ARENA_TYPE_2v2;
            break;
        case 1:
            arenatype = ARENA_TYPE_3v3;
            break;
        case 2:
            arenatype = ARENA_TYPE_5v5;
            break;
        default:
            TC_LOG_ERROR("bg.battleground","Unknown arena type %u at HandleBattlemasterJoinArena()", arenaslot);
            return;
    }

    if(!_player->IsGameMaster() && sWorld->getConfig(CONFIG_ARENASERVER_ENABLED) && arenatype != ARENA_TYPE_3v3)
    {
        ChatHandler(GetPlayer()).PSendSysMessage(LANG_ARENASERVER_ONLY_3V3);
        return;
    }

    //check existance
    Battleground* bg = nullptr;
    if( !(bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA)) )
    {
        TC_LOG_ERROR("bg.battleground","Battleground: template bg (all arenas) not found");
        return;
    }

    BattlegroundTypeId bgTypeId = bg->GetTypeID();
    BattlegroundQueueTypeId bgQueueTypeId = BattlegroundMgr::BGQueueTypeId(bgTypeId, arenatype);
    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bg->GetMapId(), _player->GetLevel());
    if (!bracketEntry)
        return;

    GroupJoinBattlegroundResult err = ERR_GROUP_JOIN_BATTLEGROUND_FAIL;

    // check queueing conditions
    if(!asGroup)
    {
        // check if already in queue
        if (_player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;
        // check if has free queue slots
        if(!_player->HasFreeBattlegroundQueueId())
            return;
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if(!grp)
            return;
        if (grp->GetLeaderGUID() != _player->GetGUID())
            return;

        err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, arenatype, arenatype, isRated != 0, arenaslot);
    }

    uint32 ateamId = 0;

    if(isRated)
    {
#ifndef LICH_KING
        //LK allow tagging alone in rated?
        if (!grp)
        { 
            TC_LOG_ERROR("network.opcode", "WorldSession::HandleBattlemasterJoinArena received isRated option without asGroup option from player %s. This guy is probably playing with packets.", _player->GetName().c_str());
            return;
        }
#endif
        ateamId = _player->GetArenaTeamId(arenaslot);
        // check real arenateam existence only here (if it was moved to group->CanJoin .. () then we would have to get it twice)
        ArenaTeam * at = sObjectMgr->GetArenaTeamById(ateamId); //sArenaTeamMgr->GetArenaTeamById(ateamId);
        if(!at)
        {
            _player->GetSession()->SendNotInArenaTeamPacket(arenatype);
            return;
        }
        // get the team rating for queueing
        arenaRating = at->GetRating();
        //TC matchmakerRating = at->GetAverageMMR(grp);
        matchmakerRating = arenaRating; //equals for now, we may want to review this later

        if (arenaRating <= 0)
            arenaRating = 1;

        if(sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ANNOUNCE))
        {
            // Announce arena tags on a dedicated channel
            std::ostringstream msg;
            std::string channel;
            std::string pvpchannel = "pvp";
            std::string ttype;
            switch (arenatype) {
                case 2: ttype = "2v2"; channel = "2v2"; break;
                case 3: ttype = "3v3"; channel = "3v3"; break;
                case 5: ttype = "5v5"; channel = "5v5"; break;
                default: 
                    ttype = "?";
                    TC_LOG_ERROR("bg.battleground","Invalid arena type.");
                    break;
            }

            //print rank approximation
            //msg << "TAG: [" << ttype << "] (" << arenaRating/50*50 << " - " << ((arenaRating/50)+1)*50 << ")";

            if (matchmakerRating >= 2200)
                msg << "TAG: [" << ttype << "] (2200+)";
            else if (matchmakerRating >= 1900)
                msg << "TAG: [" << ttype << "] (1900+)";
            else if (matchmakerRating >= 1500)
                msg << "TAG: [" << ttype << "] (1500+)";
            else
                msg << "Tag: [" << ttype << "] (1500-)";

            if(!channel.empty())
                ChatHandler::SendMessageWithoutAuthor(channel.c_str(), msg.str().c_str());

            ChatHandler::SendMessageWithoutAuthor(pvpchannel.c_str(), msg.str().c_str());
        }

    }

    BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
    if (asGroup)
    {
        uint32 avgTime = 0;

        if (err > 0)
        {
            TC_LOG_DEBUG("bg.battleground", "Battleground: arena join as group start");
            if (isRated)
            {
                TC_LOG_DEBUG("bg.battleground", "Battleground: arena team id %u, leader %s queued with matchmaker rating %u for type %u", _player->GetArenaTeamId(arenaslot), _player->GetName().c_str(), matchmakerRating, arenatype);
                bg->SetRated(true);
            }
            else
                bg->SetRated(false);

            GroupQueueInfo* ginfo = bgQueue.AddGroup(_player, grp, bgTypeId, bracketEntry, arenatype, isRated != 0, false, arenaRating, matchmakerRating, ateamId);
            avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        }

        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member)
                continue;

            WorldPacket data;

            if (err <= 0)
            {
                sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, err);
                member->SendDirectMessage(&data);
                continue;
            }

            // add to queue
            uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);

            // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0, arenatype, 0);
            member->SendDirectMessage(&data);
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, err);
            member->SendDirectMessage(&data);
            TC_LOG_DEBUG("bg.battleground", "Battleground: player joined queue for arena as group bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, member->GetGUID().GetCounter(), member->GetName().c_str());
        }
    }
    else
    {
        GroupQueueInfo* ginfo = bgQueue.AddGroup(_player, nullptr, bgTypeId, bracketEntry, arenatype, isRated != 0, false, arenaRating, matchmakerRating, ateamId);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
        uint32 queueSlot = _player->AddBattlegroundQueueId(bgQueueTypeId);

        WorldPacket data;
        // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0, arenatype, 0);
        SendPacket(&data);
        TC_LOG_DEBUG("bg.battleground", "Battleground: player joined queue for arena, skirmish, bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, _player->GetGUID().GetCounter(), _player->GetName().c_str());
    }
    sBattlegroundMgr->ScheduleQueueUpdate(matchmakerRating, arenatype, bgQueueTypeId, bgTypeId, bracketEntry->GetBracketId());
}

void WorldSession::HandleReportPvPAFK( WorldPacket & recvData )
{
    ObjectGuid playerGuid;
    recvData >> playerGuid;
    Player *reportedPlayer = ObjectAccessor::FindPlayer(playerGuid);

    if(!reportedPlayer)
    {
        TC_LOG_ERROR("bg.battleground","WorldSession::HandleReportPvPAFK: player reported by %s not found.",_player->GetName().c_str());
        return;
    }

    reportedPlayer->ReportedAfkBy(_player);
}
