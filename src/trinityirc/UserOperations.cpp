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


bool TCClient::HandleIRCUserLogInCommand(char* nickname, char* username, char* password)
{
    if(!nickname)
    {
        sLog.outIRC("Error: HandleIRCUserLogInCommand(); could not resolve nickname.");
        return false;
    } 
    if(!username || !password)
    {
        DoPrivmsg(nickname, "Correct syntax is \".login username password\".");
        return false;
    }
    
    if(!LogUserIn(nickname, username, password))
        return false;

    return true;
}

bool TCClient::LogUserIn(char* nickname, char* username, char* password)
{
    std::string strusr(username);
    LoginDatabase.escape_string(strusr);

    QueryResult *result = LoginDatabase.PQuery("SELECT `account`.`id`, `gmlevel`, `sha_pass_hash` FROM `account`, `account_access` WHERE `username` = '%s' AND `account`.`id` = `account_access`.`id`", strusr.c_str());

    if (!result)
    {
        DoPrivmsg(nickname, "ERROR: Please make sure your username was entered correctly.");
        return false;
    }

    Field *fields = result->Fetch();
    uint32 gmlevel = fields[1].GetUInt32();
    const char* sha_pass_hash = fields[2].GetString();
    unsigned char* hash_buff = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
    memset(hash_buff, 0, EVP_MAX_MD_SIZE);

    /* "UPPER(username)":"UPPER(password)" */
    char *output = password;
    for(; *output != '\0'; ++output)
        *output = (char) toupper(*output);
    output = username;
    for(; *output != '\0'; ++output)
        *output = (char) toupper(*output);

    /* SHA1 Hash */
    EVP_MD_CTX mdctx;
    const EVP_MD *md = EVP_sha1();
    unsigned int md_len;
    OpenSSL_add_all_digests();

    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, username, strlen(username));
    EVP_DigestUpdate(&mdctx, ":", 1);
    EVP_DigestUpdate(&mdctx, password, strlen(password));
    EVP_DigestFinal_ex(&mdctx, hash_buff, &md_len);

    char *strout = (char*)malloc(EVP_MAX_MD_SIZE);
    memset(strout, 0, EVP_MAX_MD_SIZE);

    for(int i = 0, j = 0; j < md_len; ++j, i+=2)
        sprintf(&strout[i], "%02x", hash_buff[j]);

    free(hash_buff);

    if(!strcmp(sha_pass_hash, strout))
    {
        ClientData * NewClient = new ClientData;
        NewClient->id       = fields[0].GetUInt32();
        NewClient->UName    = strusr;
        NewClient->GMLevel  = gmlevel;
        NewClient->nickname = strdup(nickname);
        LoggedUsers.push_back(NewClient);

        DoPPrivmsg(nickname, "Logged in successfully as %s. Your GM level is %d. ", username, gmlevel);

        free(strout);
        delete result;
        return true;
    }

    DoPrivmsg(nickname, "ERROR: Please make sure your password was entered correctly.");
    free(strout);
    delete result;
    return false;
}

bool TCClient::HandleIRCUserLogOutCommand(const char* nickname)
{
    if(!nickname)
    {
        sLog.outIRC("Error: HandleIRCUserLogOutCommand(); could not resolve nickname.");
        return false;
    }

    if(LogUserOut(nickname))
    {
        DoPrivmsg(nickname, "Logout succesful!");
        return true;
    }

    sLog.outIRC("Error: HandleIRCUserLogOutCommand(); Logout not successful.");
    return false;
}

bool TCClient::LogUserOut(const char *nickname)
{
    for(LoggedUsersList::iterator itr = LoggedUsers.begin(); itr != LoggedUsers.end(); ++itr)
    {
        if(!stricmp((*itr)->nickname, nickname))
        {
            delete (*itr);
            LoggedUsers.erase(itr);
            return true;
        }
    }

    return false;
}

ClientData const* TCClient::GetLoggedUser(const char *nickname)
{
    for(LoggedUsersList::const_iterator itr = LoggedUsers.begin(); itr != LoggedUsers.end(); ++itr)
    {
        if(!stricmp((*itr)->nickname, nickname))
        return (*itr);
    }
    return NULL;
}
