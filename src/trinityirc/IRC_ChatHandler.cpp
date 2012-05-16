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
 
#include "IRC_ChatHandler.h"

bool IRCHandler::isAvailable(ChatCommand const& cmd) const
{
    return cmd.AllowConsole && user->GMLevel >= cmd.SecurityLevel; 
}

const char *IRCHandler::GetTrinityString(int32 entry) const
{
   return objmgr.GetTrinityStringForDBCLocale(entry);
} 

char const* IRCHandler::GetName() const
{
    return nickname;
}

void IRCHandler::SendSysMessage(const char *str)
{
    std::string msg(str);
    size_t pos;
    
    while((pos = msg.find("\r")) != std::string::npos)
    {
        msg.replace(pos, 1, ", ");
    }
    while((pos = msg.find("\n")) != std::string::npos)
    {
        msg.replace(pos, 1, " ");
    }
    
    std::string ret;
    int i = 0;
    int size = msg.size();
    bool sendret = false;
    
    for(std::string::iterator itr = msg.begin(); itr != msg.end(); ++i, ++itr, --size)
    {
        if(size < 200)
        {
            if(!sendret)
                sIRC.DoPrivmsg(nickname, msg.c_str());
            else
                sIRC.DoPrivmsg(nickname, ret.c_str());    
            return;
        }
        if(size == 200)
        {    
            ret = msg.substr(0, i);
            sIRC.DoPrivmsg(nickname, ret.c_str());
            ret = msg.substr(i);
            i = 0;
            size = ret.size();
            sendret = true;
        }
    }
}

std::string IRCHandler::GetNameLink() const
{
    return nickname;
}

bool IRCHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

LocaleConstant IRCHandler::GetSessionDbcLocale() const
{
    return LocaleConstant(sWorld.GetDefaultDbcLocale());
}

int IRCHandler::GetSessionDbLocaleIndex() const
{
    return objmgr.GetDBCLocaleIndex();
}
