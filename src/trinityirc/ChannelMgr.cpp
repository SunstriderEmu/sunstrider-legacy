/*
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
 /*
 *  TrinityIRC by Machiavelli (machiaveltman@gmail.com),
 *  based on MangChat by Cybrax (cybraxvd@gmail.com).
 *  Please give proper credit for the authors' hard work.
 */ 
 
#include "IRC.h"
#include "ObjectAccessor.h"
#include "../shared/WorldPacket.h"
#include "../game/ChannelMgr.h"
#include "Guild.h"
#include "ObjectMgr.h"
#include "Config/ConfigEnv.h"

void TCClient::JoinAllChannels()
{
    for(ChannelMap::const_iterator itr = Channels.begin(); itr != Channels.end(); ++itr)
    {
        if(itr->first)
        {
            assert(itr->second);
            DoJoin(itr->first, itr->second->password);
            if(itr->second->joinmsg != NULL)
                DoPrivmsg(itr->first, itr->second->joinmsg);
                
            sLog.outIRC("Joined channel: %s", itr->first);
        }
    }
}


IRCChannel * TCClient::GetChannel(char* channelname)
{
    for(ChannelMap::iterator itr = Channels.begin(); itr != Channels.end(); ++itr)
    {
        if(!stricmp(itr->first, channelname))
            return itr->second;
    }
    return NULL;
}

IRCChannel * TCClient::GetChannelByGameName(std::string name) const
{
    for(ChannelMap::const_iterator itr = Channels.begin(); itr != Channels.end(); ++itr)
    {
        if(itr->second->wowchannel.compare(name) == 0)
        {
            return itr->second;
        }
    }
    return (NULL);
}

const char* TCClient::GetChannelNameByChannel(IRCChannel const* channel) const
{
    for(ChannelMap::const_iterator itr = Channels.begin(); itr != Channels.end(); ++itr)
    {
        if(itr->second->wowchannel.compare(channel->wowchannel) == 0)
        {
            return itr->first;
        }
    }
    return (NULL);
}

void TCClient::RemoveChannel(char* channelname)
{
    for(ChannelMap::iterator itr = Channels.begin(); itr != Channels.end();) // we can't do .find here due to case sensitivity
    {
        if(!stricmp(itr->first, channelname))
        {
            free(itr->first);       // strdup - free
            delete itr->second;     // new - delete
            Channels.erase(itr);
            return;
        }
        ++itr;
    }
    assert(1); // Shouldn't happen
}

std::string TCClient::FillChatPrefix(std::string prefix, std::string channelname, const char* name, uint32 security, uint32 team, bool join)
{
    /*if(security > SEC_PLAYER)
        prefix.append("\0037{Staff}\0037 ");*/

    if (!join)
        prefix.append("[");
    prefix.append(name);
    if (!join)
        prefix.append("] ");
    
    /*switch(team)
    {
        case 67:
        {
            //prefix.append("\0034");
            prefix.append("[");
            prefix.append(name);
            //prefix.append("\003");
            prefix.append("]");
        }
        break;        
        case 469:
        {   
            prefix.append("\00312");
            prefix.append(name);
            prefix.append("\003");
            
        }
        break;
    }*/
    
    return prefix;
}

void TCClient::HandleGameChannelActivity(std::string channelname, const char* Name, uint32 security, uint32 team, uint8 Action)
{
    IRCChannel * chan = GetChannelByGameName(channelname);
    if(chan)
    {
        if(Action == WOW_CHAN_JOIN && !(chan->relayChatMask & 1))
            return;
            
        if(Action == WOW_CHAN_LEAVE && !(chan->relayChatMask & 2))
            return;
        
        std::string prefix = "";
        prefix = (FillChatPrefix(prefix, channelname, Name, security, team, true));
        
        switch(Action)
        {
            case WOW_CHAN_JOIN:
            prefix.append(" a rejoint le canal ");
            break;
            case WOW_CHAN_LEAVE:
            prefix.append(" a quitté le canal ");
            break;
        }
        
        prefix.append(channelname.c_str());
        prefix.append(".");
        
        DoPrivmsg(GetChannelNameByChannel(chan), prefix.c_str());
    }
}

void TCClient::HandleGameChatActivity(std::string channelname, std::string message, const char* name, uint32 security, uint32 team)
{
    IRCChannel const *chan = GetChannelByGameName(channelname);
    if(chan)
    {
        if(!(chan->relayChatMask & 4))
            return;
        
        //std::string prefix = "\002\003<"+channelname+">\002\003";
        std::string prefix = "";
        prefix = (FillChatPrefix(prefix, channelname, name, security, team));
        //prefix.append(": ");
        
        std::wstring buf;
        
        message = Delink(message);
        message = WoWcol2IRC(message);
        prefix.append(message);
        
        /*if(Utf8toWStr(prefix, buf))
        {
            std::string s(buf.begin(), buf.end());
            s.assign(buf.begin(), buf.end());
            prefix = s;
        }*/
        
        DoPrivmsg(GetChannelNameByChannel(chan), prefix.c_str());
    }
}

void TCClient::HandleIRCChatActivity(const char* channelname, std::string message, char *nick)
{
    if (!strncmp(message.c_str(), "!who", 4)) { // Special handling
        Guild *guild = objmgr.GetGuildById(sConfig.GetIntDefault("IRC.Guild.Id", 0));
        if (guild)
            HandleGameChatActivity("de guilde", guild->GetOnlineMembersName().c_str(), "Connectés", 0, 0);
        return;
    }

    std::string realmsg = "[";
    realmsg.append(nick);
    realmsg.append("] ");
    message = IRCcol2WoW(message);
    realmsg.append(message);
    std::wstring buf;
    /*if(Utf8toWStr(realmsg, buf))
    {
        std::string s(buf.begin(), buf.end());
        s.assign(buf.begin(), buf.end());
        realmsg = s;
    }*/
    
    ClientData const *user = GetLoggedUser(nick);
    uint8 chatbadge = (uint8)0;
    if(user && user->GMLevel > SEC_PLAYER)
        chatbadge = (uint8)4;
        
    if (!strncmp(channelname, "de guilde", 9)) {
        Guild *guild = objmgr.GetGuildById(sConfig.GetIntDefault("IRC.Guild.Id", 0));
        if (guild)
            guild->BroadcastToGuildFromIRC(realmsg);

        return;
    }

    HashMapHolder<Player>::MapType& m = ObjectAccessor::Instance().GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (itr->second && itr->second->GetSession()->GetPlayer() && itr->second->GetSession()->GetPlayer()->IsInWorld())
        {
            if(ChannelMgr* cMgr = channelMgr(itr->second->GetSession()->GetPlayer()->GetTeam()))
            {
                if(Channel *chn = cMgr->GetChannel(channelname, itr->second->GetSession()->GetPlayer()))
                {
                    WorldPacket data(SMSG_MESSAGECHAT, 1+4+8+4+(strlen(channelname))+1+8+4+(realmsg.size())+1);
                    data << (uint8)CHAT_MSG_CHANNEL;
                    data << (uint32)LANG_UNIVERSAL;
                    data << (uint64)Configs.irc_char_guid;;  //player   GUID                      // 2.1.0
                    data << uint32(0);                                  // 2.1.0
                    data << channelname;
                    data << (uint64)Configs.irc_char_guid;; // player   GUID
                    data << (uint32)(realmsg.size()+1);
                    data << realmsg.c_str();
                    data << chatbadge;
                    itr->second->GetSession()->SendPacket(&data);
                }
            }
        }
    }

}

std::string TCClient::WoWcol2IRC(std::string msg)
{
    std::size_t pos;
    char IRCCol[17][6] = { "\xF", "\xF", "\x3\x31\x34", "\x3\x30\x33", "\x3\x31\x32", "\x3\x30\x36", "\x3\x30\x37", "\x3\x30\x34", "\x3\x30\x34", "\x3\x31\x34", "\x3\x31\x32", "\x3\x30\x37", "\x3\x30\x34", "\x3\x30\x33", "\x3\x31\x32", "\x3\x31\x32", "\x3\x30\x37"};
    char WoWCol[17][12] = { "|r", "|cffffffff", "|cff9d9d9d", "|cff1eff00", "|cff0070dd", "|cffa335ee", "|cffff8000", "|cffe6cc80", "|cffffd000", "|cff808080", "|cff71d5ff", "|cffffff00", "|cffff2020", "|cff40c040", "|cff4e96f7", "|cff71d5ff", "|cffff8040"};
    for (int i=0; i<=16; i++)
    {
        while ((pos = msg.find(WoWCol[i])) != std::string::npos)
        {
            if (i == 0)
                msg.replace(pos, 2, IRCCol[i]);
            else
                msg.replace(pos, 11, IRCCol[i]);
        }
    }
    return msg;
}

// This function converts the characters used by IRC to identify colour to a format the client can understand.
std::string TCClient::IRCcol2WoW(std::string msg)
{
    std::size_t pos;
    char IRCCol[18][4] = { "\x3\x30", "\x3\x31", "\x3\x32", "\x3\x33", "\x3\x34", "\x3\x35", "\x3\x36", "\x3\x37", "\x3\x38", "\x3\x39", "\x3\x31\x30", "\x3\x31\x31", "\x3\x31\x32", "\x3\x31\x33", "\x3\x31\x34", "\x3\x31\x35", "\x3\x30\x37", "\x3\x30\x37"};
    char IRCCol2[10][4] = { "\x3\x30\x30", "\x3\x30\x31", "\x3\x30\x32", "\x3\x30\x33", "\x3\x30\x34", "\x3\x30\x35", "\x3\x30\x36", "\x3\x30\x37", "\x3\x30\x38", "\x3\x30\x39"};
    char WoWcol[18][12] = { "|cffffffff", "|cff000000", "|cff00007f", "|cff009300", "|cffff0000", "|cff7f0000", "|cff9c009c", "|cfffc9300", "|cffffff00", "|cff00fc00", "|cff009393", "|cff00ffff", "|cff0000fc", "|cffff00ff", "|cff7f7f7f", "|cffd2d2d2", "|cff808080", "|cff71d5ff"};
                                                                                                                                                                                                                                                            
    
    for (int i=15; i>=0; i--)
    {
        if (i<10)
        {
            while ((pos = msg.find(IRCCol2[i])) != std::string::npos)
            {
                msg.replace(pos, 3, WoWcol[i]);
            }
            while ((pos = msg.find(IRCCol[i])) != std::string::npos)
            {
                msg.replace(pos, 2, WoWcol[i]);
            }
        }
        else
        {
            while ((pos = msg.find(IRCCol[i])) != std::string::npos)
            {
                msg.replace(pos, 3, WoWcol[i]);
            }
        }

        // Remove Bold, Reverse, Underline from IRC
        char Checker[3][3] = {"\x2","\x16","\x1F"}; // This is the Hex part not Dec. In Decimal its (2,22,31)
        for(int I=0; I < 3; I++)
        {
            while ((pos = msg.find(Checker[I])) != std::string::npos)
            {    
                msg.replace(pos, 1, "");
            }
        }
        // Finished Removing !

    }

    while ((pos = msg.find("\x3")) != std::string::npos)
    {
        msg.replace(pos, 1, "|r");
    }
    while ((pos = msg.find("\xF")) != std::string::npos)
    {
        msg.replace(pos, 1, "|r");
    }

    return msg;
}

std::string TCClient::Delink(std::string msg)
{
    std::size_t pos;
    while((pos = msg.find("|Hitem")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Henchant")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Hquest")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Hspell")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Htalent")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Hachievement")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Htrade")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    while((pos = msg.find("|Hglyph")) != std::string::npos)
    {
        std::size_t find1 = msg.find("|h", pos);
        msg.replace(pos, find1 - pos + 2, "\x2");
        msg.replace(msg.find("|h", pos), 2, "\x2");
    }
    return msg;
}

void TCClient::AddNewChannel(char* name, const char* password)
{
    IRCChannel *chan = new IRCChannel(password);
    char* namecpy = strdup(name);
    Channels[namecpy] = chan;
}
