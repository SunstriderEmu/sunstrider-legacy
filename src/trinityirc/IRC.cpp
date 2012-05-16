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
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( TCClient );        /** Main Client Instance **/
INSTANTIATE_SINGLETON_1( IRCConnect );      /** Socket Helper Class Instance **/

void TCClient::run()
{
    LoadConfigs();

    ACE_Based::Thread::Sleep(500);
    
    m_active = true;
    sLog.outString("TrinityIRC by Machiavelli (based on Mangchat by Cybrax) activated.");
    uint16 cCount = 1;
    while(m_active && !World::IsStopped())
    {
        if(ConnectionMgr.Connect())
        {
            Configs.Use_SSL ? ConnectionMgr.SSLReceive() : ConnectionMgr.Receive();
            if(Configs.ClientPingTimeOut)
            {
                if((LastPingTime - time(NULL)) > PingInterval)
                {
                    sLog.outIRC("Client sided ping timeout time of %d reached.", PingInterval);
                    ConnectionMgr.Disconnect();
                }
            } 
        }
        else
        {
            ++cCount;
            ACE_Based::Thread::Sleep(3000);
        }
    }
}

TCClient::~TCClient()
{
    sLog.outIRC("Deconstructing TCClient class.");
    if (m_connected)
        ConnectionMgr.Output("QUIT :TCClient deconstructor called. Shutting down.");
    if (Channels.begin() != Channels.end())
    {
        for(ChannelMap::iterator itr = Channels.begin(); itr != Channels.end();)
        {
            delete itr->second;
            Channels.erase(itr++);
        }
    }
}
