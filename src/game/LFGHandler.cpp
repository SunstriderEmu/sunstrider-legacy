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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "WorldSession.h"
#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "Player.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "World.h"

static void AttemptJoin(Player* _player)
{
    // skip not can autojoin cases and player group case
    if(!_player->m_lookingForGroup.canAutoJoin() || _player->GetGroup())
        return;

    auto lock = HashMapHolder<Player>::GetLock();
    lock->lock();
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator iter = players.begin(); iter != players.end(); ++iter)
    {
        Player *plr = iter->second;

        // skip enemies and self
        if(!plr || plr==_player || plr->GetTeam() != _player->GetTeam())
            continue;

        // skip not auto add, not group leader cases
        if(!plr->GetSession()->LookingForGroup_auto_add || (plr->GetGroup() && plr->GetGroup()->GetLeaderGUID()!=plr->GetGUID()))
            continue;

        // skip non auto-join or empty slots, or non compatible slots
        if(!plr->m_lookingForGroup.more.canAutoJoin() || !_player->m_lookingForGroup.HaveInSlot(plr->m_lookingForGroup.more))
            continue;

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        // attempt create group, or skip
        if(!plr->GetGroup())
        {
            Group* group = new Group;
            if(!group->Create(plr->GetGUID(), plr->GetName(), trans))
            {
                delete group;
                continue;
            }

            sObjectMgr->AddGroup(group);
        }

        // stop at success join
        if(plr->GetGroup()->AddMember(_player->GetGUID(), _player->GetName(), trans))
        {
            if( sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && _player->GetSession()->GetSecurity() == SEC_PLAYER )
                _player->LeaveLFGChannel();
            break;
        }
        // full
        else
        {
            if( sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && plr->GetSession()->GetSecurity() == SEC_PLAYER )
                plr->LeaveLFGChannel();
        }
        CharacterDatabase.CommitTransaction(trans);
    }
}

static void AttemptAddMore(Player* _player)
{
    // skip not group leader case
    if(_player->GetGroup() && _player->GetGroup()->GetLeaderGUID()!=_player->GetGUID())
        return;

    if(!_player->m_lookingForGroup.more.canAutoJoin())
        return;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator iter = players.begin(); iter != players.end(); ++iter)
    {
        Player *plr = iter->second;

        // skip enemies and self
        if(!plr || plr==_player || plr->GetTeam() != _player->GetTeam())
            continue;

        // skip not auto join or in group
        if(!plr->GetSession()->LookingForGroup_auto_join || plr->GetGroup() )
            continue;

        if(!plr->m_lookingForGroup.HaveInSlot(_player->m_lookingForGroup.more))
            continue;

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        // attempt create group if need, or stop attempts
        if(!_player->GetGroup())
        {
            Group* group = new Group;
            if(!group->Create(_player->GetGUID(), _player->GetName(), trans))
            {
                delete group;
                return;                                     // can't create group (??)
            }

            sObjectMgr->AddGroup(group);
        }

        // stop at join fail (full)
        if(!_player->GetGroup()->AddMember(plr->GetGUID(), plr->GetName(), trans) )
        {
            if( sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && _player->GetSession()->GetSecurity() == SEC_PLAYER )
                _player->LeaveLFGChannel();

            break;
        }
        CharacterDatabase.CommitTransaction(trans);

        // joined
        if( sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && plr->GetSession()->GetSecurity() == SEC_PLAYER )
            plr->LeaveLFGChannel();

        // and group full
        if(_player->GetGroup()->IsFull() )
        {
            if( sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && _player->GetSession()->GetSecurity() == SEC_PLAYER )
                _player->LeaveLFGChannel();

            break;
        }
    }
}

void WorldSession::HandleLfgAutoJoinOpcode( WorldPacket & /*recvData*/ )
{
    
    
    LookingForGroup_auto_join = true;

    if(!_player)                                            // needed because STATUS_AUTHED
        return;

    AttemptJoin(_player);
}

void WorldSession::HandleLfgCancelAutoJoinOpcode( WorldPacket & /*recvData*/ )
{
    
    
    LookingForGroup_auto_join = false;
}

void WorldSession::HandleLfmAutoAddMembersOpcode( WorldPacket & /*recvData*/ )
{
    
    
    LookingForGroup_auto_add = true;

    if(!_player)                                            // needed because STATUS_AUTHED
        return;

    AttemptAddMore(_player);
}

void WorldSession::HandleLfmCancelAutoAddmembersOpcode( WorldPacket & /*recvData*/ )
{
    
    
    LookingForGroup_auto_add = false;
}

void WorldSession::HandleLfgClearOpcode( WorldPacket & /*recvData */ )
{
    
    
    for(int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
        _player->m_lookingForGroup.slots[i].Clear();

    if( sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && _player->GetSession()->GetSecurity() == SEC_PLAYER )
        _player->LeaveLFGChannel();
}

void WorldSession::HandleLfmSetNoneOpcode( WorldPacket & /*recvData */)
{
    
    
    _player->m_lookingForGroup.more.Clear();
}

void WorldSession::HandleLfmSetOpcode( WorldPacket & recvData )
{
    
    
    

    uint32 temp, entry, type;
    recvData >> temp;

    entry = ( temp & 0xFFFF);
    type = ( (temp >> 24) & 0xFFFF);

    _player->m_lookingForGroup.more.Set(entry,type);

    if(LookingForGroup_auto_add)
        AttemptAddMore(_player);

    SendLfgResult(type, entry, 1);
}

void WorldSession::HandleLfgSetCommentOpcode( WorldPacket & recvData )
{
    
    
    

    std::string comment;
    recvData >> comment;

    _player->m_lookingForGroup.comment = comment;
}

void WorldSession::HandleLookingForGroup(WorldPacket& recvData)
{
    
    
    

    uint32 type, entry, unk;

    recvData >> type >> entry >> unk;

    if(LookingForGroup_auto_add)
        AttemptAddMore(_player);

    if(LookingForGroup_auto_join)
        AttemptJoin(_player);

    SendLfgResult(type, entry, 0);
}

void WorldSession::SendLfgResult(uint32 type, uint32 entry, uint8 lfg_type)
{
    uint32 number = 0;

    // start prepare packet;
    WorldPacket data(MSG_LOOKING_FOR_GROUP);
    data << uint32(type);                                   // type
    data << uint32(entry);                                  // entry from LFGDungeons.dbc
    data << uint32(0);                                      // count, placeholder
    data << uint32(0);                                      // count again, strange, placeholder

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator iter = players.begin(); iter != players.end(); ++iter)
    {
        Player *plr = iter->second;

        if(!plr || plr->GetTeam() != _player->GetTeam())
            continue;

        if(!plr->m_lookingForGroup.HaveInSlot(entry,type))
            continue;

        ++number;

        data << plr->GetPackGUID();                    // packed guid
        data << plr->GetLevel();                            // level
        data << plr->GetZoneId();                           // current zone
        data << lfg_type;                                   // 0x00 - LFG, 0x01 - LFM

        for(uint8 j = 0; j < MAX_LOOKING_FOR_GROUP_SLOT; ++j)
        {
            data << uint32( plr->m_lookingForGroup.slots[j].entry | (plr->m_lookingForGroup.slots[j].type << 24) );
        }
        data << plr->m_lookingForGroup.comment;

        Group *group = plr->GetGroup();
        if(group)
        {
            data << group->GetMembersCount()-1;             // count of group members without group leader
            for(GroupReference *itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->GetSource();
                if(member && member->GetGUID() != plr->GetGUID())
                {
                    data << member->GetPackGUID();     // packed guid
                    data << member->GetLevel();             // player level
                }
            }
        }
        else
        {
            data << uint32(0x00);
        }
    }

    // fill count placeholders
    data.put<uint32>(4+4,  number);
    data.put<uint32>(4+4+4,number);

    SendPacket(&data);
}

void WorldSession::HandleSetLfgOpcode( WorldPacket & recvData )
{
    
    
    

    uint32 slot, temp, entry, type;
    recvData >> slot >> temp;

    entry = ( temp & 0xFFFF);
    type = ( (temp >> 24) & 0xFFFF);

    if(slot >= MAX_LOOKING_FOR_GROUP_SLOT)
        return;

    _player->m_lookingForGroup.slots[slot].Set(entry,type);

    if(LookingForGroup_auto_join)
        AttemptJoin(_player);

    SendLfgResult(type, entry, 0);
}

