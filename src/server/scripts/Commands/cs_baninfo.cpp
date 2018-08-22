#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "AccountMgr.h"

static bool HandleBanInfoAccountCommand(ChatHandler* handler, char const* args)
{
    char* cname = strtok((char*)args, "");
    if(!cname)
        return false;

    std::string account_name = cname;
    if(!AccountMgr::normalizeString(account_name))
    {
        handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        handler->SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sAccountMgr->GetId(account_name);
    if(!accountid)
    {
        handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }

    return HandleBanInfoHelper(accountid,account_name.c_str());
}

static bool HandleBanInfoCharacterCommand(ChatHandler* handler, char const* args)
{
    char* cname = strtok ((char*)args, "");
    if(!cname)
        return false;

    std::string name = cname;
    if(!normalizePlayerName(name))
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(name);
    if(!accountid)
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if(!sAccountMgr->GetName(accountid,accountname))
    {
        handler->PSendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }

    return HandleBanInfoHelper(accountid,accountname.c_str());
}

static bool HandleBanInfoHelper(uint32 accountid, char const* accountname)
{
    QueryResult result = LoginDatabase.PQuery("SELECT FROM_UNIXTIME(bandate), unbandate-bandate, active, unbandate,banreason,bannedby FROM account_banned WHERE id = '%u' ORDER BY bandate ASC",accountid);
    if(!result)
    {
        handler->PSendSysMessage(LANG_BANINFO_NOACCOUNTBAN, accountname);
        return true;
    }

    handler->PSendSysMessage(LANG_BANINFO_BANHISTORY,accountname);
    do
    {
        Field* fields = result->Fetch();

        time_t unbandate = time_t(fields[3].GetUInt64());
        bool active = false;
        if(fields[2].GetBool() && (fields[1].GetUInt64() == (uint64)0 ||unbandate >= time(nullptr)) )
            active = true;
        bool permanent = (fields[1].GetUInt64() == (uint64)0);
        std::string bantime = permanent?GetTrinityString(LANG_BANINFO_INFINITE):secsToTimeString(fields[1].GetUInt64(), true);
        handler->PSendSysMessage(LANG_BANINFO_HISTORYENTRY,
            fields[0].GetCString(), bantime.c_str(), active ? GetTrinityString(LANG_BANINFO_YES):GetTrinityString(LANG_BANINFO_NO), fields[4].GetCString(), fields[5].GetCString());
    }while (result->NextRow());

    return true;
}

static bool HandleBanInfoIPCommand(ChatHandler* handler, char const* args)
{
    char* cIP = strtok ((char*)args, "");
    if(!cIP)
        return false;

    if (!IsIPAddress(cIP))
        return false;

    std::string IP = cIP;

    LoginDatabase.EscapeString(IP);
    QueryResult result = LoginDatabase.PQuery("SELECT ip, FROM_UNIXTIME(bandate), FROM_UNIXTIME(unbandate), unbandate-UNIX_TIMESTAMP(), banreason,bannedby,unbandate-bandate FROM ip_banned WHERE ip = '%s'",IP.c_str());
    if(!result)
    {
        handler->PSendSysMessage(LANG_BANINFO_NOIP);
        return true;
    }

    Field *fields = result->Fetch();
    bool permanent = !fields[6].GetUInt64();
    handler->PSendSysMessage(LANG_BANINFO_IPENTRY,
        fields[0].GetCString(), fields[1].GetCString(), permanent ? GetTrinityString(LANG_BANINFO_NEVER):fields[2].GetCString(),
        permanent ? GetTrinityString(LANG_BANINFO_INFINITE):secsToTimeString(fields[3].GetUInt64(), true).c_str(), fields[4].GetCString(), fields[5].GetCString());

    return true;
}
