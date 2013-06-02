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
 
#include "IRC.h"

void TCClient::DoNotice(const char* target, const char* message)
{
    if (!m_connected)
        return;
        
    ConnectionMgr.Output("NOTICE %s :%s ", target, message);
}

void TCClient::DoPNotice(const char* target, const char* message, ...)
{
    if (!m_connected)
        return;
        
    va_list ap;
    char ircMsg[513];
    va_start(ap, message);
    vsnprintf(ircMsg, 512, message, ap );
    va_end(ap);
    
    ConnectionMgr.Output("NOTICE %s :%s", target, ircMsg);
    }

void TCClient::DoPrivmsg(const char* target, const char* message)
{
    if (!m_connected)
        return;
        
    ConnectionMgr.Output("PRIVMSG %s :%s", target, message);
}

void TCClient::DoPPrivmsg(const char* target, const char* message, ...)
{
    if (!m_connected)
        return;
        
    va_list ap;
    char ircMsg[513];
    va_start(ap, message);
    vsnprintf(ircMsg, 512, message, ap );
    va_end(ap);
    
    ConnectionMgr.Output("PRIVMSG %s :%s", target, ircMsg);
}


void TCClient::DoJoin(const char* channel, const char* password)
{
    if (!m_connected)
        return;
    
    if(password)
        ConnectionMgr.Output("JOIN %s %s", channel, password);
    else
        ConnectionMgr.Output("JOIN %s", channel);    
}

void TCClient::DoPart(const char* channel)
{
    if (!m_connected)
        return;
        
    ConnectionMgr.Output("PART %s", channel);
    RemoveChannel((char*)channel);
}

void TCClient::DoKick(const char* channel, const char* nick)
{
    if (!m_connected)
        return;
        
    ConnectionMgr.Output("KICK %s %s", channel, nick);
}

void TCClient::DoKick(const char* channel, const char* nick, const char* message)
{
    if (!m_connected)
        return;
        
    ConnectionMgr.Output("KICK %s %s :%s", channel, nick, message);
}

void TCClient::DoMode(const char* channel, const char* prefix, const char* modes, const char* targets)
{
    if (!m_connected)
        return;
        
    if (!targets)
        ConnectionMgr.Output("MODE %s %s%s", channel, prefix, modes);
    else
        ConnectionMgr.Output("MODE %s %s%s %s", channel, prefix, modes, targets);
}

void TCClient::DoNick(const char* newnick)
{
    if (!m_connected)
        return;
        
    ConnectionMgr.Output("NICK %s", newnick);
}

void TCClient::DoQuit(const char* quit_message)
{
    if (m_connected)
    {
        if (quit_message)
            ConnectionMgr.Output("QUIT :%s", quit_message);
        else
            ConnectionMgr.Output("QUIT");
    }
}

std::string TCClient::nicklist()
{
    if (!m_connected)
        return "";
        
    return m_nicklist;
}
