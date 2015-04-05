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

void WorldSession::HandleBattlemasterHelloOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;
    recvData >> guid;

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->isBattleMaster())                             // it's not battlemaster
        return;

    // Stop the npc if moving
    unit->StopMoving();

    uint32 bgTypeId = sObjectMgr->GetBattleMasterBG(unit->GetEntry());

    if(!_player->GetBGAccessByLevel(bgTypeId))
    {
                                                            // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    SendBattlegGroundList(guid, bgTypeId);
}

void WorldSession::SendBattlegGroundList( uint64 guid, uint32 bgTypeId )
{
    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundListPacket(&data, guid, _player, bgTypeId);
    SendPacket( &data );
}

void WorldSession::_HandleBattlegroundJoin(uint32 bgTypeId,uint32 instanceId,bool joinAsGroup)
{
    Group * grp;

    // can do this, since it's battleground, not arena
    uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bgTypeId, 0);

    // ignore if player is already in BG
    if(_player->InBattleground())
        return;

    // get bg instance or bg template if instance not found
    Battleground * bg = 0;
    if(instanceId)
        Battleground *bg = sBattlegroundMgr->GetBattleground(instanceId);

    if(!bg && !(bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId)))
    {
        TC_LOG_ERROR("FIXME","Battleground: no available bg / template found");
        return;
    }

    // check queueing conditions
    if(!joinAsGroup)
    {
        // check Deserter debuff
        if( !_player->CanJoinToBattleground() )
        {
            WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
            data << (uint32) 0xFFFFFFFE;
            _player->GetSession()->SendPacket(&data);
            return;
        }
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
        //must be leader to tag as group
        if(grp->GetLeaderGUID() != _player->GetGUID())
            return;
        uint32 err = grp->CanJoinBattlegroundQueue(bgTypeId, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), false, 0);
        if (err != BG_JOIN_ERR_OK)
        {
            SendBattlegroundOrArenaJoinError(err);
            return;
        }
    }
    // if we're here, then the conditions to join a bg are met. We can proceed in joining.

    // _player->GetGroup() was already checked, grp is already initialized
    if(joinAsGroup /* && _player->GetGroup()*/)
    {
        GroupQueueInfo * ginfo = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, 0, false, 0);
        for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->GetSource();
            if(!member) continue;   // this should never happen

            uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);           // add to queue

            // store entry point coords (same as leader entry point)
            member->SetBattlegroundEntryPoint(member->GetMapId(),member->GetPositionX(),member->GetPositionY(),member->GetPositionZ(),member->GetOrientation());

            WorldPacket data;
                                                            // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].GetAvgTime(), 0);
            member->GetSession()->SendPacket(&data);
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, bgTypeId);
            member->GetSession()->SendPacket(&data);
            sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddPlayer(member, ginfo);
        }

        sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattlegroundQueueIdFromLevel());
    }
    else
    {
        // already checked if queueSlot is valid, now just get it
        uint32 queueSlot = _player->AddBattlegroundQueueId(bgQueueTypeId);
        // store entry point coords
        _player->SetBattlegroundEntryPoint(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());
        
        GroupQueueInfo * ginfo = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, 0, false, 0);

        WorldPacket data;
        // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].GetAvgTime(), 0);
        SendPacket(&data);

        sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddPlayer(_player, ginfo);
        sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattlegroundQueueIdFromLevel());
    }
}

void WorldSession::HandleBattlemasterJoinOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8+4+4+1);

    uint64 guid;
    uint32 bgTypeId;
    uint32 instanceId;
    uint8 joinAsGroup;

    recvData >> guid;                                      // battlemaster guid
    recvData >> bgTypeId;                                  // battleground type id (DBC id)
    recvData >> instanceId;                                // instance id, 0 if First Available selected
    recvData >> joinAsGroup;                               // join as group

    if(bgTypeId >= MAX_BATTLEGROUND_TYPES)
    {
        TC_LOG_ERROR("FIXME","Battleground: invalid bgtype received. possible cheater? player guid %u",_player->GetGUIDLow());
        return;
    }

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->isBattleMaster())                             // it's not battlemaster
        return;

    _HandleBattlegroundJoin(bgTypeId,instanceId,joinAsGroup);
}

void WorldSession::HandleBattlegroundPlayerPositionsOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
    Battleground *bg = _player->GetBattleground();
    if(!bg)                                                 // can't be received if player not in battleground
        return;

    if(bg->GetTypeID() == BATTLEGROUND_WS)
    {
        uint32 count1 = 0;
        uint32 count2 = 0;

        Player *ap = sObjectMgr->GetPlayer(((BattlegroundWS*)bg)->GetAllianceFlagPickerGUID());
        if(ap) ++count2;

        Player *hp = sObjectMgr->GetPlayer(((BattlegroundWS*)bg)->GetHordeFlagPickerGUID());
        if(hp) ++count2;

        WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4+16*count1+16*count2));
        data << count1;                                     // alliance flag holders count
        data << count2;                                     // horde flag holders count
        if(ap)
        {
            data << (uint64)ap->GetGUID();
            data << (float)ap->GetPositionX();
            data << (float)ap->GetPositionY();
        }
        if(hp)
        {
            data << (uint64)hp->GetGUID();
            data << (float)hp->GetPositionX();
            data << (float)hp->GetPositionY();
        }

        SendPacket(&data);
    }
    else if (bg->GetTypeID() == BATTLEGROUND_EY) {
        Player* picker = sObjectMgr->GetPlayer(((BattlegroundEY*)bg)->GetFlagPickerGUID());                                 
        WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, 4 + 4 + 16 * (picker ? 1 : 0));
        data << (uint32)0;
        data << (uint32)(picker ? 1 : 0);

        if (picker) {                              
            data << (uint64)picker->GetGUID();
            data << (float)picker->GetPositionX();
            data << (float)picker->GetPositionY();                            
        }
        
        SendPacket(&data);
    }
}

void WorldSession::HandlePVPLogDataOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
    Battleground *bg = _player->GetBattleground();
    if(!bg || (bg->isArena() && bg->GetStatus() != STATUS_WAIT_LEAVE))
        return;

    WorldPacket data;
    sBattlegroundMgr->BuildPvpLogDataPacket(&data, bg);
    SendPacket(&data);
}

void WorldSession::HandleBattlefieldListOpcode( WorldPacket &recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 4);

    uint32 bgTypeId;
    recvData >> bgTypeId;                                  // id from DBC

    if(bgTypeId >= MAX_BATTLEGROUND_TYPES)
    {
        TC_LOG_ERROR("FIXME","Battleground: invalid bgtype received: %u.", bgTypeId);
        return;
    }

    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgTypeId);

    if(!bl)
        return;

    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundListPacket(&data, _player->GetGUID(), _player, bgTypeId);
    SendPacket( &data );
}

void WorldSession::HandleBattleFieldPortOpcode( WorldPacket &recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 1+1+4+2+1);

    uint8 type;                                             // arenatype if arena
    uint8 unk2;                                             // unk, can be 0x0 (may be if was invited?) and 0x1
    uint32 instanceId;
    uint32 bgTypeId;                                        // type id from dbc
    uint16 unk;                                             // 0x1F90 constant?
    uint8 action;                                           // enter battle 0x1, leave queue 0x0

    recvData >> type >> unk2 >> bgTypeId >> unk >> action;

    if(bgTypeId >= MAX_BATTLEGROUND_TYPES)
    {
        TC_LOG_ERROR("FIXME","Battleground: invalid bgtype received: %u.", bgTypeId);
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
                BattlegroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());
                // if the player is not in queue, contine
                if(itrPlayerStatus == sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end())
                    continue;

                // no group information, this should never happen
                if(!itrPlayerStatus->second.GroupInfo)
                    continue;

                Battleground * bg = NULL;

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
                    Battleground * bg = sBattlegroundMgr->GetBattleground(itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID);
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
        return;
    }

    uint32 bgQueueTypeId = 0;
    // get the bg what we were invited to
    BattlegroundQueue::QueuedPlayersMap::iterator itrPlayerStatus;
    bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bgTypeId,type);
    itrPlayerStatus = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());

    if(itrPlayerStatus == sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end())
    {
        TC_LOG_ERROR("FIXME","Battleground: itrplayerstatus not found.");
        return;
    }
    instanceId = itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID;

    // if action == 1, then instanceId is _required_
    if(!instanceId && action == 1)
    {
        TC_LOG_ERROR("FIXME","Battleground: instance not found.");
        return;
    }

    Battleground *bg = sBattlegroundMgr->GetBattleground(instanceId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if(!bg && action == 0)
        bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);

    if(!bg)
    {
        TC_LOG_ERROR("FIXME","Battleground: bg not found.");
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
        BattlegroundQueue::QueuedPlayersMap::iterator pitr = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());
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
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_IN_PROGRESS, 0, bg->GetElapsedTime());
                _player->GetSession()->SendPacket(&data);
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
                /*
                if player leaves rated arena match before match start, it is counted as he played but he lost
                */
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
                sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId,_player->GetBattlegroundQueueIdFromLevel(),arenatype,israted,rating);
                SendPacket(&data);
                break;
            default:
                TC_LOG_ERROR("FIXME","Battleground port: unknown action %u", action);
                break;
        }
    }
}

void WorldSession::HandleBattlefieldLeaveOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;

    if(Battleground *bg = _player->GetBattleground())
    {
        // not allow leave battleground in combat
        if(_player->IsInCombat())
            if(bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

        if (bg->isSpectator(_player->GetGUID()))
        {
            _player->CancelSpectate();

            uint32 map = _player->GetBattlegroundEntryPointMap();
            float positionX = _player->GetBattlegroundEntryPointX();
            float positionY = _player->GetBattlegroundEntryPointY();
            float positionZ = _player->GetBattlegroundEntryPointZ();
            float positionO = _player->GetBattlegroundEntryPointO();
            if (_player->TeleportTo(map, positionX, positionY, positionZ, positionO))
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
    PROFILE;
    
    WorldPacket data;

    // TODO: we must put player back to battleground in case disconnect (< 5 minutes offline time) or teleport player on login(!) from battleground map to entry point
    if(_player->InBattleground())
    {
        Battleground *bg = _player->GetBattleground();
        if(bg)
        {
            uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
            uint32 queueSlot = _player->GetBattlegroundQueueIndex(bgQueueTypeId);
            if((bg->GetStatus() <= STATUS_IN_PROGRESS))
            {
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_IN_PROGRESS, 0, bg->GetElapsedTime());
                SendPacket(&data);
            }
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
            {
                uint32 queue_id = _player->GetBattlegroundQueueId(i);       // battlegroundqueueid stores the type id, not the instance id, so this is definitely wrong
                uint8 arenatype = sBattlegroundMgr->BGArenaType(queue_id);
                uint8 isRated = 0;
                if (i == queueSlot || !queue_id)                            // we need to get the instance ids
                    continue;
                BattlegroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());
                if(itrPlayerStatus == sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end())
                    continue;
                if(itrPlayerStatus->second.GroupInfo)
                {
                    arenatype = itrPlayerStatus->second.GroupInfo->ArenaType;
                    isRated = itrPlayerStatus->second.GroupInfo->IsRated;
                }
                Battleground *bg2 = sBattlegroundMgr->GetBattlegroundTemplate(sBattlegroundMgr->BGTemplateId(queue_id)); //  try this
                if(bg2)
                {
                    //in this call is small bug, this call should be filled by player's waiting time in queue
                    //this call nulls all timers for client :
                    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg2, _player->GetTeam(), i, STATUS_WAIT_QUEUE, sBattlegroundMgr->m_BattlegroundQueues[queue_id].GetAvgTime(), 0,arenatype,isRated);
                    SendPacket(&data);
                }
            }
        }
    }
    else
    {
        // we should update all queues? .. i'm not sure if this code is correct
        for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            uint32 queue_id = _player->GetBattlegroundQueueId(i);
            if(!queue_id)
                continue;
            uint32 bgTypeId = sBattlegroundMgr->BGTemplateId(queue_id);
            uint8 arenatype = sBattlegroundMgr->BGArenaType(queue_id);
            uint8 isRated = 0;
            Battleground *bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            BattlegroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].find(_player->GetGUID());
            if(itrPlayerStatus == sBattlegroundMgr->m_BattlegroundQueues[queue_id].m_QueuedPlayers[_player->GetBattlegroundQueueIdFromLevel()].end())
                continue;
            if(itrPlayerStatus->second.GroupInfo)
            {
                arenatype = itrPlayerStatus->second.GroupInfo->ArenaType;
                isRated = itrPlayerStatus->second.GroupInfo->IsRated;
            }
            if(bg && queue_id)
            {
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), i, STATUS_WAIT_QUEUE, sBattlegroundMgr->m_BattlegroundQueues[queue_id].GetAvgTime(), 0, arenatype, isRated);
                SendPacket(&data);
            }
        }
    }
/*    else              // not sure if it needed...
    {
        for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, NULL, _player->GetTeam(),i , STATUS_NONE, 0, 0);
            SendPacket(&data);
        }
    }*/
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    Battleground *bg = _player->GetBattleground();
    if(!bg)
        return;

    uint64 guid;
    recvData >> guid;

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->isSpiritService())                            // it's not spirit service
        return;

    sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, guid);
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode( WorldPacket & recvData )
{
    CHECK_PACKET_SIZE(recvData, 8);

    Battleground *bg = _player->GetBattleground();
    if(!bg)
        return;

    uint64 guid;
    recvData >> guid;

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->isSpiritService())                            // it's not spirit service
        return;

    bg->AddPlayerToResurrectQueue(guid, _player->GetGUID());
}

void WorldSession::HandleBattlemasterJoinArena( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8+1+1+1);

    // ignore if we already in BG or BG queue
    if(_player->InBattleground())
        return;

    uint64 guid;                                            // arena Battlemaster guid
    uint8 type;                                             // 2v2, 3v3 or 5v5
    uint8 asGroup;                                          // asGroup
    uint8 isRated;                                          // isRated
    Group * grp;

    recvData >> guid >> type >> asGroup >> isRated;

    Creature *unit = ObjectAccessor::GetCreature(*_player, guid);
    if(!unit)
        return;

    if(!unit->isBattleMaster())                             // it's not battle master
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
        time_t curTime = time(NULL);
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

    switch(type)
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
            TC_LOG_ERROR("FIXME","Unknown arena type %u at HandleBattlemasterJoinArena()", type);
            return;
    }

    if(!_player->IsGameMaster() && sWorld->getConfig(CONFIG_ARENASERVER_ENABLED) && arenatype != ARENA_TYPE_3v3)
    {
        ChatHandler(GetPlayer()).PSendSysMessage(LANG_ARENASERVER_ONLY_3V3);
        return;
    }

    //check existance
    Battleground* bg = NULL;
    if( !(bg = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA)) )
    {
        TC_LOG_ERROR("FIXME","Battleground: template bg (all arenas) not found");
        return;
    }

    uint8 bgTypeId = bg->GetTypeID();
    uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bgTypeId, arenatype);

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
        uint32 err = grp->CanJoinBattlegroundQueue(bgTypeId, bgQueueTypeId, arenatype, arenatype, (bool)isRated, type);
        if (err != BG_JOIN_ERR_OK)
        {
            SendBattlegroundOrArenaJoinError(err);
            return;
        }
    }

    uint32 ateamId = 0;

    if(isRated)
    {
        ateamId = _player->GetArenaTeamId(type);
        // check real arenateam existence only here (if it was moved to group->CanJoin .. () then we would have to get it twice)
        ArenaTeam * at = sObjectMgr->GetArenaTeamById(ateamId);
        if(!at)
        {
            _player->GetSession()->SendNotInArenaTeamPacket(arenatype);
            return;
        }
        // get the team rating for queueing
        arenaRating = at->GetRating();
        // the arenateam id must match for everyone in the group
        // get the personal ratings for queueing
        uint32 avg_pers_rating = 0;
        uint32 max_pers_rating = 0;
        for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->GetSource();
            
            uint32 cur_pers_rating = member->GetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (type*6) + 5);

            // max personal rating
            if (cur_pers_rating > max_pers_rating)
                max_pers_rating = cur_pers_rating;

            // calc avg personal rating
            avg_pers_rating += member->GetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (type*6) + 5);
        }

        if( arenatype )
            avg_pers_rating /= arenatype;

        if(sWorld->getConfig(CONFIG_BATTLEGROUND_ARENA_ANNOUNCE))
        {
            // Announce arena tags on a dedicated channel
            std::ostringstream msg;
            char const* channel;
            char const* pvpchannel = "pvp";
            char const* ttype;
            switch (arenatype) {
                case 2: ttype = "2v2"; channel = "2v2"; break;
                case 3: ttype = "3v3"; channel = "3v3"; break;
                case 5: ttype = "5v5"; channel = "5v5"; break;
                default: TC_LOG_ERROR("FIXME","Invalid arena type.");
            }

            //msg << "TAG: [" << ttype << "] (" << arenaRating/50*50 << " - " << ((arenaRating/50)+1)*50 << ")";

            if (arenaRating >= 2200)
                msg << "TAG: [" << ttype << "] (2200+)";
            else if (arenaRating >= 1900)
                msg << "TAG: [" << ttype << "] (1900+)";
            else if (arenaRating >= 1500)
                msg << "TAG: [" << ttype << "] (1500+)";
            else
                msg << "Tag: [" << ttype << "] (1500-)";

            ChatHandler(_player).SendMessageWithoutAuthor(channel, msg.str().c_str());
            ChatHandler(_player).SendMessageWithoutAuthor(pvpchannel, msg.str().c_str());
        }

        // if avg personal rating is more than 150 points below the teams rating, the team will be queued against an opponent matching or similar to the maximal personal rating in the team
        if(avg_pers_rating + 150 < arenaRating) {
            //arenaRating = avg_pers_rating;
            arenaRating = max_pers_rating;
        }
    }

    if(asGroup)
    {
        GroupQueueInfo * ginfo = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, arenatype, isRated, arenaRating, ateamId);
        for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->GetSource();
            if(!member) continue;

            uint32 queueSlot = member->AddBattlegroundQueueId(bgQueueTypeId);// add to queue

            // store entry point coords (same as leader entry point)
            member->SetBattlegroundEntryPoint(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());

            WorldPacket data;
            // send status packet (in queue)
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, member->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].GetAvgTime(), 0, arenatype, isRated);
            member->GetSession()->SendPacket(&data);
            sBattlegroundMgr->BuildGroupJoinedBattlegroundPacket(&data, bgTypeId);
            member->GetSession()->SendPacket(&data);
            sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddPlayer(member, ginfo);
        }

        sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattlegroundQueueIdFromLevel(), arenatype, isRated, arenaRating);
    }
    else
    {
        uint32 queueSlot = _player->AddBattlegroundQueueId(bgQueueTypeId);

        // store entry point coords
        _player->SetBattlegroundEntryPoint(_player->GetMapId(),_player->GetPositionX(),_player->GetPositionY(),_player->GetPositionZ(),_player->GetOrientation());
        GroupQueueInfo * ginfo = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, arenatype, isRated, arenaRating);

        WorldPacket data;
        // send status packet (in queue)
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].GetAvgTime(), 0, arenatype, isRated);
        SendPacket(&data);
        sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].AddPlayer(_player, ginfo);
        sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattlegroundQueueIdFromLevel(), arenatype, isRated, arenaRating);
    }
}

void WorldSession::HandleReportPvPAFK( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 playerGuid;
    recvData >> playerGuid;
    Player *reportedPlayer = sObjectMgr->GetPlayer(playerGuid);

    if(!reportedPlayer)
    {
        TC_LOG_ERROR("FIXME","WorldSession::HandleReportPvPAFK: player reported by %s not found.",_player->GetName().c_str());
        return;
    }

    reportedPlayer->ReportedAfkBy(_player);
}

void WorldSession::SendBattlegroundOrArenaJoinError(uint8 err)
{
    WorldPacket data;
    int32 msg;
    switch (err)
    {
        case BG_JOIN_ERR_OFFLINE_MEMBER:
            msg = LANG_BG_GROUP_OFFLINE_MEMBER;
            break;
        case BG_JOIN_ERR_GROUP_TOO_MANY:
            msg = LANG_BG_GROUP_TOO_LARGE;
            break;
        case BG_JOIN_ERR_MIXED_FACTION:
            msg = LANG_BG_GROUP_MIXED_FACTION;
            break;
        case BG_JOIN_ERR_MIXED_LEVELS:
            msg = LANG_BG_GROUP_MIXED_LEVELS;
            break;
        case BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE:
            msg = LANG_BG_GROUP_MEMBER_ALREADY_IN_QUEUE;
            break;
        case BG_JOIN_ERR_GROUP_DESERTER:
            msg = LANG_BG_GROUP_MEMBER_DESERTER;
            break;
        case BG_JOIN_ERR_ALL_QUEUES_USED:
            msg = LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS;
            break;
        case BG_JOIN_ERR_GROUP_NOT_ENOUGH:
        case BG_JOIN_ERR_MIXED_ARENATEAM:
        default:
            return;
            break;
    }
    ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, nullptr, nullptr, GetTrinityString(msg));
    SendPacket(&data);
    return;
}

