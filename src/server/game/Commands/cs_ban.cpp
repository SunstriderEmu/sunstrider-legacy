#include "Chat.h"
#include "Language.h"
#include "AccountMgr.h"

bool ChatHandler::HandleUnBanAccountCommand(const char* args)
{
    return HandleUnBanHelper(SANCTION_BAN_ACCOUNT,args);
}

bool ChatHandler::HandleUnBanCharacterCommand(const char* args)
{
    return HandleUnBanHelper(SANCTION_BAN_CHARACTER,args);
}

bool ChatHandler::HandleUnBanIPCommand(const char* args)
{
    return HandleUnBanHelper(SANCTION_BAN_IP,args);
}

bool ChatHandler::HandleBanListIPCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.EscapeString(filter);

    QueryResult result;

    if(filter.empty())
    {
        result = LoginDatabase.Query ("SELECT ip,bandate,unbandate,bannedby,banreason FROM ip_banned"
            " WHERE (bandate=unbandate OR unbandate>UNIX_TIMESTAMP())"
            " ORDER BY unbandate" );
    }
    else
    {
        result = LoginDatabase.PQuery( "SELECT ip,bandate,unbandate,bannedby,banreason FROM ip_banned"
            " WHERE (bandate=unbandate OR unbandate>UNIX_TIMESTAMP()) AND ip " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'")
            " ORDER BY unbandate",filter.c_str() );
    }

    if(!result)
    {
        PSendSysMessage(LANG_BANLIST_NOIP);
        return true;
    }

    PSendSysMessage(LANG_BANLIST_MATCHINGIP);
    // Chat short output
    if(m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            PSendSysMessage("%s",fields[0].GetCString());
        } while (result->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_IPS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_IPS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field *fields = result->Fetch();
            time_t t_ban = fields[1].GetUInt64();
            tm* aTm_ban = localtime(&t_ban);
            if ( fields[1].GetUInt64() == fields[2].GetUInt64() )
            {
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                    fields[0].GetCString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    fields[3].GetCString(), fields[4].GetCString());
            }
            else
            {
                time_t t_unban = fields[2].GetUInt64();
                tm* aTm_unban = localtime(&t_unban);
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                    fields[0].GetCString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                    fields[3].GetCString(), fields[4].GetCString());
            }
        }while( result->NextRow() );
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleBanListCharacterCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok ((char*)args, " ");
    if(!cFilter)
        return false;

    std::string filter = cFilter;
    LoginDatabase.EscapeString(filter);
    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'"),filter.c_str());
    if (!result)
    {
        PSendSysMessage(LANG_BANLIST_NOCHARACTER);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListAccountCommand(const char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate<=UNIX_TIMESTAMP() AND unbandate<>bandate");

    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.EscapeString(filter);

    QueryResult result;

    if(filter.empty())
    {
        result = LoginDatabase.Query("SELECT account.id, username FROM account, account_banned"
            " WHERE account.id = account_banned.id AND active = 1 GROUP BY account.id");
    }
    else
    {
        result = LoginDatabase.PQuery("SELECT account.id, username FROM account, account_banned"
            " WHERE account.id = account_banned.id AND active = 1 AND username " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'")" GROUP BY account.id",
            filter.c_str());
    }

    if (!result)
    {
        PSendSysMessage(LANG_BANLIST_NOACCOUNT);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListHelper(QueryResult result)
{
    PSendSysMessage(LANG_BANLIST_MATCHINGACCOUNT);

    // Chat short output
    if(m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 accountid = fields[0].GetUInt32();

            QueryResult banresult = LoginDatabase.PQuery("SELECT account.username FROM account,account_banned WHERE account_banned.id='%u' AND account_banned.id=account.id",accountid);
            if(banresult)
            {
                Field* fields2 = banresult->Fetch();
                PSendSysMessage("%s",fields2[0].GetCString());
            }
        } while (result->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_ACCOUNTS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_ACCOUNTS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field *fields = result->Fetch();
            uint32 account_id = fields[0].GetUInt32 ();

            std::string account_name;

            // "account" case, name can be get in same query
            if(result->GetFieldCount() > 1)
                account_name = fields[1].GetString();
            // "character" case, name need extract from another DB
            else
                sAccountMgr->GetName (account_id,account_name);

            // No SQL injection. id is uint32.
            QueryResult banInfo = LoginDatabase.PQuery("SELECT bandate,unbandate,bannedby,banreason FROM account_banned WHERE id = %u ORDER BY unbandate", account_id);
            if (banInfo)
            {
                Field *fields2 = banInfo->Fetch();
                do
                {
                    time_t t_ban = fields2[0].GetUInt64();
                    tm* aTm_ban = localtime(&t_ban);

                    if (fields2[0].GetUInt64() == fields2[1].GetUInt64())
                    {
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            fields2[2].GetCString(),fields2[3].GetCString());
                    }
                    else
                    {
                        time_t t_unban = fields2[1].GetUInt64();
                        tm* aTm_unban = localtime(&t_unban);
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                            fields2[2].GetCString(),fields2[3].GetCString());
                    }
                }while ( banInfo->NextRow() );
            }
        }while( result->NextRow() );
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleBanAccountCommand(const char* args)
{
    return HandleBanHelper(SANCTION_BAN_ACCOUNT,args);
}

bool ChatHandler::HandleBanCharacterCommand(const char* args)
{
    return HandleBanHelper(SANCTION_BAN_CHARACTER,args);
}

bool ChatHandler::HandleBanIPCommand(const char* args)
{
    return HandleBanHelper(SANCTION_BAN_IP,args);
}

bool ChatHandler::HandleBanHelper(SanctionType mode, const char* args)
{
    char* cnameOrIP = strtok ((char*)args, " ");
    if (!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    char* duration = strtok (nullptr," ");
    if(!duration || !atoi(duration))
        return false;

    char* reason = strtok (nullptr,"");
    if(!reason)
        return false;

    switch(mode)
    {
        case SANCTION_BAN_ACCOUNT:
            if(!AccountMgr::normalizeString(nameOrIP))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,nameOrIP.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case SANCTION_BAN_CHARACTER:
            if(!normalizePlayerName(nameOrIP))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case SANCTION_BAN_IP:
            if(!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    switch(sWorld->BanAccount(mode, nameOrIP, duration, reason, m_session ? m_session->GetPlayerName() : "", m_session))
    {
        case BAN_SUCCESS:
            if(atoi(duration)>0)
                PSendSysMessage(LANG_BAN_YOUBANNED,nameOrIP.c_str(),secsToTimeString(TimeStringToSecs(duration),true).c_str(),reason);
            else
                PSendSysMessage(LANG_BAN_YOUPERMBANNED,nameOrIP.c_str(),reason);
            break;
        case BAN_SYNTAX_ERROR:
            return false;
        case BAN_NOTFOUND:
            switch(mode)
            {
                default:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"account",nameOrIP.c_str());
                    break;
                case SANCTION_BAN_CHARACTER:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"character",nameOrIP.c_str());
                    break;
                case SANCTION_BAN_IP:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"ip",nameOrIP.c_str());
                    break;
            }
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleUnBanHelper(SanctionType mode, const char* args)
{
    char* cnameOrIP = strtok ((char*)args, " ");
    if(!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    switch(mode)
    {
        case SANCTION_BAN_ACCOUNT:
            if(!AccountMgr::normalizeString(nameOrIP))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,nameOrIP.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case SANCTION_BAN_CHARACTER:
            if(!normalizePlayerName(nameOrIP))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case SANCTION_BAN_IP:
            if(!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    if(sWorld->RemoveBanAccount(mode,nameOrIP, m_session))
        PSendSysMessage(LANG_UNBAN_UNBANNED,nameOrIP.c_str());
    else
        PSendSysMessage(LANG_UNBAN_ERROR,nameOrIP.c_str());

    return true;
}
