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

#ifndef _TC_IRC_H
#define _TC_IRC_H

#include "../game/World.h"
#include "Log.h"
#include "Database/DatabaseEnv.h"
#include "Message.h"
#include <openssl/ssl.h>

// Variables TrinityIRC uses to establish connection.
// Nickname!Username@host * Realname
// Password is for server and nickserv authentication.

enum UserModes
{
    MODE_NONE       = 0,
    MODE_VOICE      = 1,
    MODE_HALFOP     = 2,
    MODE_OP         = 4,
    MODE_FOUNDER    = 8,
};

struct ClientData
{
    ClientData() : GMLevel(0), lastmsg(time(NULL) - 36000) {}

    void UpdateIdleTime()
    {
        lastmsg = time(NULL);
    }
    
    ~ClientData()
    {
        if(nickname)
            free(nickname);
    }
    
    uint32 id;
    std::string UName;
    char* nickname;
    uint32 GMLevel;
    time_t lastmsg;
};
typedef std::list<ClientData*> LoggedUsersList;

struct IRCChannel
{
    IRCChannel() : password(NULL), joinmsg(NULL) {};
    IRCChannel(const char *_password) // .irc join case
    {
        if(_password)
            password = strdup(_password);
        else
            password = (char*)"";
         
        relayChatMask = 0;
        joinmsg = NULL;
    }
    ~IRCChannel()
    {
        if(password != NULL)
            free(password);
        if(joinmsg != NULL)
            free(joinmsg);
    }
    
    char *password;
    char *joinmsg;
    std::string wowchannel;
    uint8 relayChatMask;
};
typedef UNORDERED_MAP<char*, IRCChannel*> ChannelMap;

class TCClient : public ACE_Based::Runnable
{
   public:
        TCClient() : m_active(false), m_connected(false), PingInterval(0), LastPingTime(0) {}
        ~TCClient();

        void run();

        /* Members showing current status. */
        bool m_active;
        bool m_connected;
        std::string m_nicklist;
        /* Functions for loading configs. */
        void LoadConfigs();
        QueryResult* GetDBConfig(const char* field) const;


        /***********************************************\
        #           OUTGOING COMMAND HANDLERS           #
        \***********************************************/
        void DoQuit(const char* message);
        void DoNotice(const char* target, const char* message);
        void DoPNotice(const char* target, const char* message, ...);
        void DoPrivmsg(const char* target, const char* message);
        void DoPPrivmsg(const char* target, const char* message, ...);
        void DoJoin(const char* channel, const char* password);
        void DoPart(const char* channel);
        void DoKick(const char* channel, const char* nick);
        void DoKick(const char* channel, const char* nick, const char* message);
        void DoMode(const char* channel, const char* prefix, const char* modes, const char* targets);
        void DoNick(const char* newnick);
        std::string nicklist();

        /* Channel and Logged user storage. */
        ChannelMap Channels;
        LoggedUsersList LoggedUsers;

     protected:
        /***********************************************\
        #          CHANNEL STORAGE FUNCTIONS            #
        \***********************************************/
        void JoinAllChannels();
        void AddNewChannel(char* name, const char* password);
        void RemoveChannel(char* channelname);
        IRCChannel * GetChannel(char* channelname);
        IRCChannel * GetChannelByGameName(std::string name) const;
        const char* GetChannelNameByChannel(IRCChannel const* channel) const;

    public:
        /***********************************************\
        #        INTERCOMMUNICATION RELAY SYSTEM        #
        \***********************************************/
        void HandleGameChannelActivity(std::string channelname, const char* name, uint32 security, uint32 team, uint8 Action);
        void HandleGameChatActivity(std::string channelname, std::string message, const char* name, uint32 security, uint32 team);
        void HandleIRCChatActivity(const char* channelname, std::string message, char* nick);
        std::string FillChatPrefix(std::string prefix, std::string channelname, const char* name, uint32 security, uint32 team, bool join = false);
        std::string IRCcol2WoW(std::string msg);
        std::string WoWcol2IRC(std::string msg);
        std::string Delink(std::string msg);

        /***********************************************\
        #       REMOTE ADMINISTRATION MANAGEMENT        #
        \***********************************************/
        ClientData const* GetLoggedUser(const char *username);
        bool HandleIRCUserLogInCommand(char* nickname, char* username, char* password);
        bool HandleIRCUserLogOutCommand(const char* nickname);
        bool LogUserOut(const char *nickname);
        bool LogUserIn(char* nickname, char* username, char* password);

   protected:
        /***********************************************\
        #         INCOMING IRC COMMAND HANDLERS         #
        \***********************************************/
        void GetHook(IRC_MSG_DATA *__ICD);
        bool HandleRFCCode(IRC_MSG_DATA *__ICD);
        bool HandleCTCP(const char *nick, const char *msg);
        void HandlePrivmsgCommand(IRC_MSG_DATA *ICD);
        void HandleNickCommand(IRC_MSG_DATA *ICD);
        void HandleKickCommand(IRC_MSG_DATA *ICD);
        void HandleQuitCommand(IRC_MSG_DATA *ICD);
        void HandleModeCommand(IRC_MSG_DATA *ICD);
        void HandleJoinCommand(IRC_MSG_DATA *ICD);
        void HandlePartCommand(IRC_MSG_DATA *ICD);
        void HandleNamesCommand(IRC_MSG_DATA* ICD);
        
    public:        
        void Decode(char *data);
        void HandleIncoming(char *data);

        time_t LastPingTime;
        uint32 PingInterval;
        
        /***********************************************\
        #           CONFIGURATIONS STRUCTURE            #
        \***********************************************/
        struct sConfigStruct
        {
            bool Use_SSL;

            bool ClientPingTimeOut;
            time_t PingTimeOutTime;

            uint64 irc_char_guid;
            uint32 userIdleInterval;

            const char *securityLevelModes[5];

        } Configs;
};
#define sIRC Trinity::Singleton<TCClient>::Instance()

/***********************************************\
#           SOCKET OPERATIONS CLASS             #
\***********************************************/

class IRCConnect
{
    friend class TCClient;
    public:
        IRCConnect(){ SendAttempt = 0; }
        ~IRCConnect(){}

        bool Connect();
        bool SSLConnect();    /*SSL*/
        int Receive();
        int SSLReceive();     /*SSL*/
        void Disconnect();

        void Output(const char* data, ...);
        void Transmit(char* data);

        void SetConnected(bool value) { sIRC.m_connected = value; }
        bool IsConnected() { return sIRC.m_connected; }

    protected:
        const char* Host;
        uint32 Port;
        int irc_socket;
        struct sockaddr_in sout;
        struct hostent *he;
        SSL_CTX *ctx;       /* SSL */
        SSL *ssl;           /* SSL */
        SSL_METHOD *method; /* SSL */
        uint16 SendAttempt;

        const char* NickName;
        const char* UserName;
        const char* RealName;
        const char* NickServPassword;
        const char* NetworkPassword;
};

#define ConnectionMgr Trinity::Singleton<IRCConnect>::Instance()

enum Activity
{
    WOW_CHAN_JOIN,
    WOW_CHAN_LEAVE,
    IRC_CHAN_JOIN,
    IRC_CHAN_LEAVE,
    MSG_IRC_WOW,
    MSG_WOW_IRC
};

#endif
