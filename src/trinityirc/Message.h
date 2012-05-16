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
 
 
#ifndef IRC_MSG_DATA_H
#define IRC_MSG_DATA_H

struct IRC_SENDER
{
    IRC_SENDER()
    {
        nick = (char*)"";
        ident = (char*)"";
        host = (char*)"";
    } 
   
    char *nick;
    char *ident;
    char *host;

};

struct IRC_MSG_DATA
{
    IRC_MSG_DATA()
    {
        command = (char*)"";
        params = (char*)"";
    }
    
    IRC_SENDER sender; // Contains the senders name and host and identity (nickname!username@host);
    char *command;     // Contains the command receieved from the other party.
    char *params;      // Contains the remainder of the data we receieved.

};


#endif
