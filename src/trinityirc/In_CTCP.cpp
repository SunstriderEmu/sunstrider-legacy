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
#include "SystemConfig.h"

bool TCClient::HandleCTCP(const char * nick,  const char * msg)
{
    if(strstr(msg, "\001VERSION\001"))
    {
        DoNotice(nick, _FULLVERSION);
        sLog.outIRC("CTCP Version request from %s, replying.", nick);
        return true;
    }
    
    else if(strstr(msg, "\001CLIENTINFO\001"))
    {
        DoNotice(nick, "TrinityIRC by Machiavelli (machiaveltman@gmail.com) (C) 2008-2009. Licensed under the GNU General Public License 2.0. Modified by DrThum (C) 2012.");
        sLog.outIRC("CTCP Clientinfo request from %s, replying.", nick);
        return true;
    }

    return false;
}
