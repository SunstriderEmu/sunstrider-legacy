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
#include "../shared/Config/ConfigEnv.h"

inline QueryResult* TCClient::GetDBConfig(const char* field) const
{
    return WorldDatabase.PQuery("SELECT `configuration_value` FROM `irc_configurations` where `configuration_index` = '%s'", field);
}

void TCClient::LoadConfigs()
{
    QueryResult *result = WorldDatabase.PQuery("SELECT * FROM `irc_channels`");

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            
            char* name = strdup(fields[1].GetString());
            IRCChannel *chan = new IRCChannel;
            chan->password = strdup(fields[2].GetString());
            chan->wowchannel = fields[3].GetCppString();
            chan->joinmsg = strdup(fields[4].GetString());
            chan->relayChatMask = fields[5].GetUInt8();
            Channels[name] = chan;
        }
        while( result->NextRow() );
        
        delete result;
    }
    else
        sLog.outIRC("No channels found in the database configuration table `irc_channels`.");

    ConnectionMgr.Host = GetDBConfig("Host")->Fetch()->GetString();
    ConnectionMgr.Port = GetDBConfig("Port")->Fetch()->GetUInt32();
    ConnectionMgr.NickName = GetDBConfig("Nickname")->Fetch()->GetString();
    ConnectionMgr.UserName = GetDBConfig("User")->Fetch()->GetString();
    ConnectionMgr.NickServPassword = GetDBConfig("Password")->Fetch()->GetString();
    ConnectionMgr.RealName = GetDBConfig("Realname")->Fetch()->GetString();
    
    Configs.ClientPingTimeOut = (GetDBConfig("Clientside.Ping.Timeout")->Fetch()->GetBool());
    if(Configs.ClientPingTimeOut)
        Configs.PingTimeOutTime = GetDBConfig("Clientside.Ping.TimeoutTime")->Fetch()->GetUInt64();
    Configs.Use_SSL = GetDBConfig("SSL Connection")->Fetch()->GetBool();
    Configs.irc_char_guid = GetDBConfig("GameNameGUID")->Fetch()->GetUInt64();
    Configs.userIdleInterval = GetDBConfig("UserIdleInterval")->Fetch()->GetUInt32();
    Configs.securityLevelModes[SEC_PLAYER] = GetDBConfig("LoginChannelMode_0")->Fetch()->GetString();
    Configs.securityLevelModes[SEC_MODERATOR] = GetDBConfig("LoginChannelMode_1")->Fetch()->GetString();
    Configs.securityLevelModes[SEC_GAMEMASTER] = GetDBConfig("LoginChannelMode_2")->Fetch()->GetString();
    Configs.securityLevelModes[SEC_ADMINISTRATOR] = GetDBConfig("LoginChannelMode_3")->Fetch()->GetString();
    Configs.securityLevelModes[SEC_CONSOLE] = GetDBConfig("LoginChannelMode_4")->Fetch()->GetString();
}
