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
#include "IRC_ChatHandler.h"
#include "Log.h"

void TCClient::GetHook(IRC_MSG_DATA *_ICD)
{
    /*
        This filter function sends the required data to a unique individual command hook, defined per command.
    */

    if(!stricmp("PRIVMSG", _ICD->command))
        HandlePrivmsgCommand(_ICD);
    else if(!stricmp("QUIT", _ICD->command)) 
        HandleQuitCommand(_ICD);
    else if(!stricmp("PART", _ICD->command))
        HandlePartCommand(_ICD);
    else if(!stricmp("KICK", _ICD->command))
        HandleKickCommand(_ICD);
    else if(!stricmp("NICK", _ICD->command))
        HandleNickCommand(_ICD);
    else if(!stricmp("JOIN", _ICD->command))
        HandleJoinCommand(_ICD);
    else if(!stricmp("353", _ICD->command))
        HandleNamesCommand(_ICD);

}

void TCClient::HandleJoinCommand(IRC_MSG_DATA *ICD)
{
    char *nick = ICD->sender.nick;
    char *channelname = &ICD->params[1];
    
    IRCChannel* Chan = GetChannel(channelname);
    if(!Chan)
    {
        char *pw = strtok(channelname, " ");
        AddNewChannel(channelname, pw);
        return;
    }

    if(!stricmp(nick, ConnectionMgr.NickName)) // ourselves
        return;

    if(Chan->relayChatMask & 8)
        HandleIRCChatActivity(Chan->wowchannel.c_str(), "a rejoint IRC.", nick);
        
    ConnectionMgr.Output("NAMES #staff");
}

void TCClient::HandlePrivmsgCommand(IRC_MSG_DATA *ICD)
{
    /*
    This function handles an incoming private message.
    Fist we check for CTCP command from another client, then we resume processing the data.
    All IRC-GM commands are done via DoPrivmsg (private or channel messages),
    only if the command prefix is detected.
    */

    char *channel = strtok(ICD->params, " :");
    char *message = &ICD->params[strlen(channel ) + 2];
    std::string fullmsg(message);
    
    ClientData const* user = GetLoggedUser(ICD->sender.nick);
    if(user)
        const_cast<ClientData*>(user)->UpdateIdleTime();

    // core handlers
    if(!strncmp(message, ".", 1))
    {
        char *fulcmd = strdup(message);
        char *cmd = strtok(message, " ");
        char *output = cmd;
        
        for(; *output != '\0'; ++output)
           *output = (char) toupper(*output);

        if(strstr(&cmd[1], "LOGOUT"))
        {
            if(user)
            {
                if(!HandleIRCUserLogOutCommand(ICD->sender.nick)){}
            }
            else DoPrivmsg(ICD->sender.nick, "Error: you are not logged in.");

            free(fulcmd);  
            return;
        }
        else if(strstr(&cmd[1], "LOGIN"))
        {
            if(!user)
            {
                char *username = strtok(NULL, " ");
                char *pass = strtok(NULL, " ");
                
                if(!HandleIRCUserLogInCommand(ICD->sender.nick, username, pass)){}
            }
            else DoPrivmsg(ICD->sender.nick, "Error: you are already logged in.");

            free(fulcmd);
            return;
        }
        else
        {
            if(!user)
            {
                DoPrivmsg(ICD->sender.nick, "Error: you are not logged in. Please type \".login username password\" to gain access to the remote administration interface.");
                free(fulcmd);
                return;
            }

            IRCHandler *ICH = new IRCHandler(ICD->sender.nick, user);
            if(ICH->ParseCommands(fulcmd) == 1)
            {
                std::ostringstream ss;
                ss << ICD->sender.nick << " (Account: " << user->UName;
                ss << ") used command [" << cmd << "] with parameters: [";
                ss << &fulcmd[strlen(cmd)] << " ]";
                sLog.outIRCGM(ss.str().c_str());
            }
            delete ICH;
            free(fulcmd);  
        }  
    }
    else if(!stricmp(message, "INFO")) // Get bot info
    {
        std::ostringstream ss;
        uint32 channelcounter = 0, loggedcounter = 0, gmcounter = 0;
        ChannelMap::const_iterator itr = Channels.begin();
        LoggedUsersList::const_iterator ktr = LoggedUsers.begin();
        for(; itr != Channels.end(); ++itr)
        {
            ++channelcounter;
        }
        for(; ktr != LoggedUsers.end(); ++ktr)
        {
            if((*ktr)->GMLevel > SEC_PLAYER)
                ++gmcounter;
            ++loggedcounter;    
        }
        ss << "I am active on " << channelcounter;
        ss << " channels. ";
        ss << "I also have " << loggedcounter << " users logged into the remote administration interface, ";
        ss << gmcounter << " of which are staffmembers. I have been active for " << secsToTimeString(sWorld.GetUptime());
        DoPrivmsg(ICD->sender.nick, ss.str().c_str());
    }
    else if(!strcmp(channel, ConnectionMgr.NickName))
    {
        DoPPrivmsg(ICD->sender.nick, "Hello there %s. I am a bot running from TrinityIRC. To gain access to administrative commands, type '.login username password'. To find out more about the bot's status, type 'info'.", ICD->sender.nick);
    }
    else
    {
        IRCChannel const *irchan = GetChannel(channel);
        if(irchan && (irchan->relayChatMask & 32))
        {
            HandleIRCChatActivity(irchan->wowchannel.c_str(), fullmsg, ICD->sender.nick);
        }
    }
}


void TCClient::HandleQuitCommand(IRC_MSG_DATA *ICD)
{
    LogUserOut(ICD->sender.nick);
}

void TCClient::HandlePartCommand(IRC_MSG_DATA *ICD)
{
    LogUserOut(ICD->sender.nick);
    char* channelname = strtok(ICD->params, " ");
    char *nick = ICD->sender.nick;
    IRCChannel* chan = GetChannel(channelname);
    if(chan)
        HandleIRCChatActivity(chan->wowchannel.c_str(), "a quitté le canal IRC.", nick);
        
    ConnectionMgr.Output("NAMES #staff");
}

void TCClient::HandleKickCommand(IRC_MSG_DATA *ICD)
{
    /*
        The following handles the KICK command.
        First part checks if we were kicked. If yes, loop trough the channel list container to find a channel
        by the channel name in the KICK command. When the channel has been found, we re-join the channel,
        and if the password to the channel has been specified earlier, we append this to the DoJoin command.
    */

    char *tmpdata = ICD->params;
    char *channel = strtok(tmpdata, " ");
    char *kicked = strtok(NULL, " ");
    
    sLog.outIRC("%s was kicked from %s, by %s", kicked, channel, ICD->sender.nick);
    
    IRCChannel *chn = GetChannel(channel);
    
    if(chn && !stricmp(ConnectionMgr.NickName, kicked) )
    {
        DoJoin(channel, chn->password);
        DoPrivmsg(channel, "Please don't kick me again, you will have your privileges revoked.");
        sLog.outIRC("Rejoined channel %s.", channel);
    }
    
    if(LogUserOut(kicked))
        DoPPrivmsg(channel, "%s succesfuly logged out.", kicked);
        
    ConnectionMgr.Output("NAMES #staff");
}

void TCClient::HandleNickCommand(IRC_MSG_DATA *ICD)
{
    if(!stricmp(ICD->sender.nick, ConnectionMgr.NickName))
    {
        ConnectionMgr.NickName = strdup(&ICD->params[1]);
        return;
    }
    
    for(LoggedUsersList::iterator itr = LoggedUsers.begin(); itr != LoggedUsers.end(); ++itr)
        if(!stricmp((*itr)->nickname, ICD->sender.nick))
            LogUserOut((*itr)->nickname);
            
    ConnectionMgr.Output("NAMES #staff");
}

void TCClient::HandleNamesCommand(IRC_MSG_DATA *ICD)
{
    char* params = strtok(ICD->params, ":");
    params = strtok(NULL, "");
    
    m_nicklist = std::string(params);
}
