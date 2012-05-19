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

bool TCClient::HandleRFCCode(IRC_MSG_DATA *_ICD)
{
    switch(atoi(_ICD->command))
    {   
        case 001: // Welcome to the IRC server
        {
            DoPPrivmsg("NICKSERV", "IDENTIFY %s", ConnectionMgr.NickServPassword);
            JoinAllChannels();
            return true;
        }
        case 433: // Nickname is in use
        {
            DoPNotice("NICKSERV", "GHOST %s %s", ConnectionMgr.NickName, ConnectionMgr.NickServPassword);
            return true;
        }

        /*  Codes that don't require to or cannot be handled by us, return true. */

        case 002: // Host information
        case 003: // Server information
        case 005: // ^
        case 042: // Unique assigned ID
        case 251: // There are x users and y invisible on z servers.
        case 252: // x operator(s) online.
        case 254: // x channels formed
        case 255: // I have x clients and 1 servers
        case 265: // Current Local Users: x Max: y
        case 266: // Current Global Users: x Max: y
        case 332: // Channel topic
        //case 353: // Receive names of users on channel
        case 366: // end of /names
        case 372: // MOTD
        case 376: // End of MOTD
        case 401:
        case 439: // Please wait while we process your connection
        return true;
        
        /*     If for some reason a command with length == 3 isn't a supported RFC command
               return false and continue processing                                        */
        default:
            return false;
        break;
    }
}
