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

#include "Channel.h"
#include "ChannelMgr.h"
#include "ObjectMgr.h"
#include "World.h"
#include "SocialMgr.h"
#include "Chat.h"

Channel::Channel(const std::string& name, uint32 channel_id)
: m_name(name), m_announce(true), m_moderate(false), m_channelId(channel_id), m_ownerGUID(0), m_password(""), m_flags(0)
{
    // set special flags if built-in channel
    ChatChannelsEntry const* ch = GetChannelEntryFor(channel_id);
    if(ch)                                                  // it's built-in channel
    {
        m_channelId = ch->ChannelID;                         // built-in channel
        m_announce = false;                                 // no join/leave announces

        m_flags |= CHANNEL_FLAG_GENERAL;                    // for all built-in channels

        if(ch->flags & CHANNEL_DBC_FLAG_TRADE)              // for trade channel
            m_flags |= CHANNEL_FLAG_TRADE;

        if(ch->flags & CHANNEL_DBC_FLAG_CITY_ONLY2)         // for city only channels
            m_flags |= CHANNEL_FLAG_CITY;

        if(ch->flags & CHANNEL_DBC_FLAG_LFG)                // for LFG channel
            m_flags |= CHANNEL_FLAG_LFG;
        else                                                // for all other channels
            m_flags |= CHANNEL_FLAG_NOT_LFG;
    }
    else                                                    // it's custom channel
    {
        m_flags |= CHANNEL_FLAG_CUSTOM;
        
        // Load GM bans on channels
        gmbanned.clear();
        std::string safe_name = name;
        CharacterDatabase.EscapeString(safe_name);
        QueryResult result = CharacterDatabase.PQuery("SELECT accountid, expire FROM channel_ban WHERE channel = '%s' AND expire > " UI64FMTD " ORDER BY expire", safe_name.c_str(), time(NULL));
        if (result) {
            do {
                Field *fields = result->Fetch();
                gmbanned[fields[0].GetUInt32()] = fields[1].GetUInt32();
            } while (result->NextRow());
        }
        
        if (m_name == "2v2" || m_name == "3v3" || m_name == "5v5" || m_name == "pvp") //Don't announce arrivals for these chans
            m_announce = false;
    }
}

bool Channel::IsBannedByGM(uint64 const guid)
{
    uint64 accountId = sObjectMgr->GetPlayerAccountIdByGUID(guid);
    if (!accountId) {
        TC_LOG_ERROR("FIXME","Channel::IsBanned: Unknown account for player " UI64FMTD , guid);
        return false;
    }
    
    /*for (GMBannedList::const_iterator itrtmp = gmbanned.begin(); itrtmp != gmbanned.end(); itrtmp++) {
        TC_LOG_INFO("TIME is %lu, guid is " UI64FMTD ", expire is " UI64FMTD , time(NULL), itrtmp->first, itrtmp->second);
    }*/
    
    GMBannedList::const_iterator itr = gmbanned.find(accountId);
    if (itr != gmbanned.end()) {        // Account is banned, check expiration date
        if (itr->second > time(NULL))
            return true;
    }
    
    return false;
}

void Channel::Join(uint64 p, const char *pass)
{
    WorldPacket data;
    if(IsOn(p))
    {
        if(!IsConstant())                                   // non send error message for built-in channels
        {
            MakePlayerAlreadyMember(&data, p);
            SendToOne(&data, p);
        }
        return;
    }

    if(IsBanned(p))
    {
        MakeBanned(&data);
        SendToOne(&data, p);
        return;
    }

    if (IsBannedByGM(p))
    {
        MakeBanned(&data);
        SendToOne(&data, p);
        return;
    }

    if(m_password.length() > 0 && strcmp(pass, m_password.c_str()))
    {
        MakeWrongPassword(&data);
        SendToOne(&data, p);
        return;
    }

    Player *plr = sObjectMgr->GetPlayer(p);

    if(plr)
    {
        if(HasFlag(CHANNEL_FLAG_LFG) &&
            sWorld->getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && plr->GetSession()->GetSecurity() == SEC_PLAYER &&
            (plr->GetGroup() || plr->m_lookingForGroup.Empty()) )
        {
            MakeNotInLfg(&data);
            SendToOne(&data, p);
            return;
        }

        if(plr->GetGuildId() && (GetFlags() == 0x38))
            return;

        plr->JoinedChannel(this);
    }

    if(m_announce && (!plr || plr->GetSession()->GetSecurity() < SEC_GAMEMASTER1 || !sWorld->getConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL) ) && this->GetName() != "world" && this->GetName() != "pvp") //announce auto-deactivated for the world channel
    {
        MakeJoined(&data, p);
        SendToAll(&data);
    }

    data.clear();

    PlayerInfo pinfo;
    pinfo.player = p;
    pinfo.flags = 0;
    pinfo.invisible = (plr ? plr->GetSession()->GetSecurity() > SEC_PLAYER : false) && sWorld->getConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL);
    players[p] = pinfo;

    MakeYouJoined(&data);
    SendToOne(&data, p);

    JoinNotify(p);
    
    // if no owner first logged will become
    if(!IsConstant() && !m_ownerGUID)
    {
        SetOwner(p, (players.size() > 1 ? true : false));
        players[p].SetModerator(true);
    }
}

void Channel::Leave(uint64 p, bool send)
{
    if(!IsOn(p))
    {
        if(send)
        {
            WorldPacket data;
            MakeNotMember(&data);
            SendToOne(&data, p);
        }
    }
    else
    {
        Player *plr = sObjectMgr->GetPlayer(p);
        if(send)
        {
            WorldPacket data;
            MakeYouLeft(&data);
            SendToOne(&data, p);
            if(plr)
                plr->LeftChannel(this);
            data.clear();
        }

        bool changeowner = players[p].IsOwner();

        players.erase(p);
        if(m_announce && (!plr || plr->GetSession()->GetSecurity() == SEC_PLAYER || !sWorld->getConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL) ) && this->GetName() != "world" && this->GetName() != "pvp") //announce auto-deactivated for the world & pvp channel
        {
            WorldPacket data;
            MakeLeft(&data, p);
            SendToAll(&data);
        }

        LeaveNotify(p);

        if(changeowner)
        {
            uint64 newowner = !players.empty() ? players.begin()->second.player : 0;
            SetOwner(newowner);
        }
    }
}

void Channel::KickOrBan(uint64 good, std::string const& badname, bool ban)
{
    uint32 sec = 0;
    Player *gplr = sObjectAccessor->FindConnectedPlayer(good);
    if(gplr)
        sec = gplr->GetSession()->GetSecurity();
        
    if (this->GetName() == "world" && sec < SEC_GAMEMASTER1)
        return;

    if(!IsOn(good) && sec < SEC_GAMEMASTER1)
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, good);
    }
    else if(!players[good].IsModerator() && sec < SEC_GAMEMASTER1)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, good);
    }
    else
    {
        Player *bad = sObjectAccessor->FindConnectedPlayerByName(badname);
        if(bad == NULL || !IsOn(bad->GetGUID()))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, badname);
            SendToOne(&data, good);
        }
        else if(sec < SEC_GAMEMASTER1 && bad->GetGUID() == m_ownerGUID && good != m_ownerGUID)
        {
            WorldPacket data;
            MakeNotOwner(&data);
            SendToOne(&data, good);
        }
        else
        {
            bool changeowner = (m_ownerGUID == bad->GetGUID());

            WorldPacket data;

            if(ban && !IsBanned(bad->GetGUID()))
            {
                banned.insert(bad->GetGUID());
                MakePlayerBanned(&data, bad->GetGUID(), good);
            }
            else
                MakePlayerKicked(&data, bad->GetGUID(), good);

            if (GetName() != "world")
                SendToAll(&data);
            players.erase(bad->GetGUID());
            bad->LeftChannel(this);

            if(changeowner)
            {
                uint64 newowner = !players.empty() ? good : false;
                SetOwner(newowner);
            }
        }
    }
}

void Channel::UnBan(uint64 good, std::string const& badname)
{
    uint32 sec = 0;
    Player *gplr = sObjectMgr->GetPlayer(good);
    if(gplr)
        sec = gplr->GetSession()->GetSecurity();

    if(!IsOn(good))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, good);
    }
    else if(!players[good].IsModerator() && sec < SEC_GAMEMASTER2)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, good);
    }
    else
    {
        Player *bad = sObjectAccessor->FindConnectedPlayerByName(badname);
        if(bad == NULL || !IsBanned(bad->GetGUID()))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, badname);
            SendToOne(&data, good);
        }
        else
        {
            banned.erase(bad->GetGUID());

            WorldPacket data;
            MakePlayerUnbanned(&data, bad->GetGUID(), good);
            SendToAll(&data);
        }
    }
}

void Channel::Password(uint64 p, const char *pass)
{
    uint32 sec = 0;
    Player *plr = sObjectMgr->GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();
    
    if (this->GetName() == "world" && sec < SEC_GAMEMASTER2)
        return;

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if(!players[p].IsModerator() && sec < SEC_GAMEMASTER2)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        m_password = pass;

        WorldPacket data;
        MakePasswordChanged(&data, p);
        SendToAll(&data);
    }
}

void Channel::SetMode(uint64 p, const char *p2n, bool mod, bool set)
{
    uint32 sec = 0;
    Player *plr = sObjectMgr->GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if(!players[p].IsModerator() && sec < SEC_GAMEMASTER2)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        Player *newp = sObjectAccessor->FindConnectedPlayerByName(p2n);
        if(!newp)
        {
            WorldPacket data;
            MakePlayerNotFound(&data, p2n);
            SendToOne(&data, p);
            return;
        }

        PlayerInfo inf = players[newp->GetGUID()];
        if(p == m_ownerGUID && newp->GetGUID() == m_ownerGUID && mod)
            return;

        if(!IsOn(newp->GetGUID()))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, p2n);
            SendToOne(&data, p);
            return;
        }

        // allow make moderator from another team only if both is GMs
        // at this moment this only way to show channel post for GM from another team
        if( plr && (plr->GetSession()->GetSecurity() < SEC_GAMEMASTER2 || newp->GetSession()->GetSecurity() < SEC_GAMEMASTER2) &&
            plr->GetTeam() != newp->GetTeam() && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL) )
        {
            WorldPacket data;
            MakePlayerNotFound(&data, p2n);
            SendToOne(&data, p);
            return;
        }

        if(m_ownerGUID == newp->GetGUID() && m_ownerGUID != p)
        {
            WorldPacket data;
            MakeNotOwner(&data);
            SendToOne(&data, p);
            return;
        }

        if(mod)
            SetModerator(newp->GetGUID(), set);
        else
            SetMute(newp->GetGUID(), set);
    }
}

void Channel::SetOwner(uint64 p, const char *newname)
{
    uint32 sec = 0;
    Player *plr = sObjectMgr->GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
        return;
    }

    if(sec < SEC_GAMEMASTER2 && p != m_ownerGUID)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, p);
        return;
    }

    Player *newp = sObjectAccessor->FindConnectedPlayerByName(newname);
    if(newp == NULL || !IsOn(newp->GetGUID()))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, p);
        return;
    }

    if(plr && newp->GetTeam() != plr->GetTeam() && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, p);
        return;
    }

    players[newp->GetGUID()].SetModerator(true);
    SetOwner(newp->GetGUID());
}

void Channel::SendWhoOwner(uint64 p)
{
    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else
    {
        WorldPacket data;
        MakeChannelOwner(&data);
        SendToOne(&data, p);
    }
}

void Channel::List(Player* player)
{
    uint64 p = player->GetGUID();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else
    {
        WorldPacket data(SMSG_CHANNEL_LIST, 1+(GetName().size()+1)+1+4+GetNumPlayers()*(8+1));
        data << uint8(1);                                   // channel type?
        data << GetName();                                  // channel name
        data << uint8(GetFlags());                          // channel flags?

        size_t pos = data.wpos();
        data << uint32(0);                                  // size of list, placeholder

        //uint32 gmLevelInWhoList = sWorld->getConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);
        uint32 count  = 0;

        for(PlayerList::iterator i = players.begin(); i != players.end(); ++i)
        {
            /*
            Player *plr = sObjectMgr->GetPlayer(i->first);

            // PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
            // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
            if (plr && (player->GetSession()->GetSecurity() > SEC_PLAYER || plr->GetSession()->GetSecurity() <= gmLevelInWhoList) && 
                    plr->IsVisibleGloballyFor(player))
                    */
            if(!(i->second.invisible))
            {
                data << uint64(i->first);
                data << uint8(i->second.flags);             // flags seems to be changed...
                ++count;
            }
        }

        data.put<uint32>(pos,count);

        SendToOne(&data, p);
    }
}

void Channel::Announce(uint64 p)
{
    uint32 sec = 0;
    Player *plr = sObjectMgr->GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if(!players[p].IsModerator() && sec < SEC_GAMEMASTER2)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        m_announce = !m_announce;

        WorldPacket data;
        if(m_announce)
            MakeAnnouncementsOn(&data, p);
        else
            MakeAnnouncementsOff(&data, p);
        SendToAll(&data);
    }
}

void Channel::Moderate(uint64 p)
{
    uint32 sec = 0;
    Player *plr = sObjectMgr->GetPlayer(p);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if(!players[p].IsModerator() && sec < SEC_GAMEMASTER2)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        m_moderate = !m_moderate;

        WorldPacket data;
        if(m_moderate)
            MakeModerationOn(&data, p);
        else
            MakeModerationOff(&data, p);
        SendToAll(&data);
    }
}

void Channel::Say(uint64 playerGUID, const char *what, Language lang)
{
    if(!what)
        return;

    if (sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        lang = LANG_UNIVERSAL;

    uint32 sec = 0;
    Player *plr = sObjectMgr->GetPlayer(playerGUID);
    if(plr)
        sec = plr->GetSession()->GetSecurity();

    if(!IsOn(playerGUID))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, playerGUID);
    }
    else if(players[playerGUID].IsMuted())
    {
        WorldPacket data;
        MakeMuted(&data);
        SendToOne(&data, playerGUID);
    }
    else if(m_moderate && !players[playerGUID].IsModerator() && sec < SEC_GAMEMASTER2)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, playerGUID);
    }
    else if (this->GetName() == "world" && plr && plr->GetLevel() < sWorld->getConfig(CONFIG_WORLDCHANNEL_MINLEVEL))
        ChatHandler(plr).PSendSysMessage("You must be at least level %u to speak on this channel.", sWorld->getConfig(CONFIG_WORLDCHANNEL_MINLEVEL));
    else if (m_name == "2v2" || m_name == "3v3" || m_name == "5v5" || m_name == "pvp")
        return;
    else
    {
        if(!plr)
            return;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, lang, plr, nullptr, what, 0, m_name);
        SendToAll(&data, !players[playerGUID].IsModerator() ? playerGUID : false);
        // if player is horde, put this on gmworlda, alliance side (and vice-versa)
        if (plr && this->GetName() == "world") {
            ChannelMgr* cMgrOther = channelMgr(plr->GetTeam());
            if (!cMgrOther)
                return;

            std::string gmchannelName = plr->GetTeam() == TEAM_HORDE ? "gmworldh" : "gmworlda"; 
            if (Channel* chan = cMgrOther->GetJoinChannel(gmchannelName, 0)) {
                WorldPacket data2;
                ChatHandler::BuildChatPacket(data2, CHAT_MSG_CHANNEL, lang, plr, nullptr, what, 0, gmchannelName);
                chan->SendToAll(&data2, 0);
            }
        }
    }
}

void Channel::Invite(uint64 playerGUID, const char *newname)
{
    if(!IsOn(playerGUID))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, playerGUID);
        return;
    }

    Player *newp = sObjectAccessor->FindConnectedPlayerByName(newname);
    if(!newp)
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, playerGUID);
        return;
    }

    Player *plr = sObjectMgr->GetPlayer(playerGUID);
    if (!plr)
        return;

    if (newp->GetTeam() != plr->GetTeam() && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakeInviteWrongFaction(&data);
        SendToOne(&data, playerGUID);
        return;
    }

    if(IsOn(newp->GetGUID()))
    {
        WorldPacket data;
        MakePlayerAlreadyMember(&data, newp->GetGUID());
        SendToOne(&data, playerGUID);
        return;
    }

    WorldPacket data;
    if(!newp->GetSocial()->HasIgnore(GUID_LOPART(playerGUID)))
    {
        MakeInvite(&data, playerGUID);
        SendToOne(&data, newp->GetGUID());
        data.clear();
    }
    MakePlayerInvited(&data, newp->GetName());
    SendToOne(&data, playerGUID);
}

void Channel::SetOwner(uint64 guid, bool exclaim)
{
    if(m_ownerGUID)
    {
        // [] will re-add player after it possible removed
        PlayerList::iterator p_itr = players.find(m_ownerGUID);
        if(p_itr != players.end())
            p_itr->second.SetOwner(false);
    }

    m_ownerGUID = guid;
    if(m_ownerGUID)
    {
        uint8 oldFlag = GetPlayerFlags(m_ownerGUID);
        players[m_ownerGUID].SetOwner(true);

        WorldPacket data;
        MakeModeChange(&data, m_ownerGUID, oldFlag);
        SendToAll(&data);

        if(exclaim)
        {
            MakeOwnerChanged(&data, m_ownerGUID);
            SendToAll(&data);
        }
    }
}

void Channel::SendToAll(WorldPacket *data, uint64 p)
{
    for(PlayerList::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player *plr = sObjectMgr->GetPlayer(i->first);
        if(plr)
        {
            if(!p || !plr->GetSocial()->HasIgnore(GUID_LOPART(p)))
                plr->GetSession()->SendPacket(data);
        }
    }
}

void Channel::SendToAllButOne(WorldPacket *data, uint64 who)
{
    for(PlayerList::iterator i = players.begin(); i != players.end(); ++i)
    {
        if(i->first != who)
        {
            Player *plr = sObjectMgr->GetPlayer(i->first);
            if(plr)
                plr->GetSession()->SendPacket(data);
        }
    }
}

void Channel::SendToOne(WorldPacket *data, uint64 who)
{
    Player *plr = sObjectMgr->GetPlayer(who);
    if(plr)
        plr->GetSession()->SendPacket(data);
}

void Channel::Voice(uint64 guid1, uint64 guid2)
{

}

void Channel::DeVoice(uint64 guid1, uint64 guid2)
{

}

// done
void Channel::MakeNotifyPacket(WorldPacket *data, uint8 notify_type)
{
    data->Initialize(SMSG_CHANNEL_NOTIFY, 1+m_name.size()+1);
    *data << uint8(notify_type);
    *data << m_name;
}

// done 0x00
void Channel::MakeJoined(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_JOINED_NOTICE);
    *data << uint64(guid);
}

// done 0x01
void Channel::MakeLeft(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_LEFT_NOTICE);
    *data << uint64(guid);
}

// done 0x02
void Channel::MakeYouJoined(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_YOU_JOINED_NOTICE);
    *data << uint8(GetFlags());
    *data << uint32(GetChannelId());
    *data << uint32(0);
}

// done 0x03
void Channel::MakeYouLeft(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_YOU_LEFT_NOTICE);
    *data << uint32(GetChannelId());
    *data << uint8(0);                                      // can be 0x00 and 0x01
}

// done 0x04
void Channel::MakeWrongPassword(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_WRONG_PASSWORD_NOTICE);
}

// done 0x05
void Channel::MakeNotMember(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_MEMBER_NOTICE);
}

// done 0x06
void Channel::MakeNotModerator(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATOR_NOTICE);
}

// done 0x07
void Channel::MakePasswordChanged(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PASSWORD_CHANGED_NOTICE);
    *data << uint64(guid);
}

// done 0x08
void Channel::MakeOwnerChanged(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_OWNER_CHANGED_NOTICE);
    *data << uint64(guid);
}

// done 0x09
void Channel::MakePlayerNotFound(WorldPacket *data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_FOUND_NOTICE);
    *data << name;
}

// done 0x0A
void Channel::MakeNotOwner(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_OWNER_NOTICE);
}

// done 0x0B
void Channel::MakeChannelOwner(WorldPacket *data)
{
    std::string name = "";

    if(!sObjectMgr->GetPlayerNameByGUID(m_ownerGUID, name) || name.empty())
        name = "PLAYER_NOT_FOUND";

    MakeNotifyPacket(data, CHAT_CHANNEL_OWNER_NOTICE);
    *data << ((IsConstant() || !m_ownerGUID) ? "Nobody" : name);
}

// done 0x0C
void Channel::MakeModeChange(WorldPacket *data, uint64 guid, uint8 oldflags)
{
    MakeNotifyPacket(data, CHAT_MODE_CHANGE_NOTICE);
    *data << uint64(guid);
    *data << uint8(oldflags);
    *data << uint8(GetPlayerFlags(guid));
}

// done 0x0D
void Channel::MakeAnnouncementsOn(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_ON_NOTICE);
    *data << uint64(guid);
}

// done 0x0E
void Channel::MakeAnnouncementsOff(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_OFF_NOTICE);
    *data << uint64(guid);
}

// done 0x0F
void Channel::MakeModerationOn(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_MODERATION_ON_NOTICE);
    *data << uint64(guid);
}

// done 0x10
void Channel::MakeModerationOff(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_MODERATION_OFF_NOTICE);
    *data << uint64(guid);
}

// done 0x11
void Channel::MakeMuted(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_MUTED_NOTICE);
}

// done 0x12
void Channel::MakePlayerKicked(WorldPacket *data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_KICKED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

// done 0x13
void Channel::MakeBanned(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_BANNED_NOTICE);
}

// done 0x14
void Channel::MakePlayerBanned(WorldPacket *data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_BANNED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

// done 0x15
void Channel::MakePlayerUnbanned(WorldPacket *data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_UNBANNED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

// done 0x16
void Channel::MakePlayerNotBanned(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_BANNED_NOTICE);
    *data << uint64(guid);
}

// done 0x17
void Channel::MakePlayerAlreadyMember(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_ALREADY_MEMBER_NOTICE);
    *data << uint64(guid);
}

// done 0x18
void Channel::MakeInvite(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_INVITE_NOTICE);
    *data << uint64(guid);
}

// done 0x19
void Channel::MakeInviteWrongFaction(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_INVITE_WRONG_FACTION_NOTICE);
}

// done 0x1A
void Channel::MakeWrongFaction(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_WRONG_FACTION_NOTICE);
}

// done 0x1B
void Channel::MakeInvalidName(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_INVALID_NAME_NOTICE);
}

// done 0x1C
void Channel::MakeNotModerated(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATED_NOTICE);
}

// done 0x1D
void Channel::MakePlayerInvited(WorldPacket *data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITED_NOTICE);
    *data << name;
}

// done 0x1E
void Channel::MakePlayerInviteBanned(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITE_BANNED_NOTICE);
    *data << uint64(guid);
}

// done 0x1F
void Channel::MakeThrottled(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_THROTTLED_NOTICE);
}

// done 0x20
void Channel::MakeNotInArea(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_IN_AREA_NOTICE);
}

// done 0x21
void Channel::MakeNotInLfg(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_IN_LFG_NOTICE);
}

// done 0x22
void Channel::MakeVoiceOn(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_VOICE_ON_NOTICE);
    *data << uint64(guid);
}

// done 0x23
void Channel::MakeVoiceOff(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_VOICE_OFF_NOTICE);
    *data << uint64(guid);
}

void Channel::JoinNotify(uint64 guid)
{
    WorldPacket data;

    if(IsConstant())
        data.Initialize(SMSG_USERLIST_ADD, 8+1+1+4+GetName().size()+1);
    else
        data.Initialize(SMSG_USERLIST_UPDATE, 8+1+1+4+GetName().size()+1);

    data << uint64(guid);
    data << uint8(GetPlayerFlags(guid));
    data << uint8(GetFlags());
    data << uint32(GetNumPlayers());
    data << GetName();
    SendToAll(&data);
}

void Channel::LeaveNotify(uint64 guid)
{
    WorldPacket data(SMSG_USERLIST_REMOVE, 8+1+4+GetName().size()+1);
    data << uint64(guid);
    data << uint8(GetFlags());
    data << uint32(GetNumPlayers());
    data << GetName();
    SendToAll(&data);
}

void Channel::RemoveGMBan(uint64 accountid)
{
    gmbanned[accountid] = time(NULL);
}

uint32 Channel::GetNumPlayers()
{ 
    uint32 falseCount = 0;
    for(auto itr : players)
    {
        if(!(itr.second.invisible))
            falseCount++;
    }
    return falseCount;

}