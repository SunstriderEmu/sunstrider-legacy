#include "Chat.h"
#include "Language.h"

/// Set/Unset the expansion level for an account
bool ChatHandler::HandleAccountSetAddonCommand(const char* args)
{
    ///- Get the command line arguments
    char *szAcc = strtok((char*)args," ");
    char *szExp = strtok(nullptr," ");

    if(!szAcc)
        return false;

    std::string account_name;
    uint32 account_id;

    if(!szExp)
    {
        Player* player = GetSelectedPlayerOrSelf();
        if(!player)
            return false;

        account_id = player->GetSession()->GetAccountId();
        sAccountMgr->GetName(account_id,account_name);
        szExp = szAcc;
    }
    else
    {
        ///- Convert Account name to Upper Format
        account_name = szAcc;
        if(!AccountMgr::normalizeString(account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        account_id = sAccountMgr->GetId(account_name);
        if(!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    int lev=atoi(szExp);                                    //get int anyway (0 if error)
    if(lev < 0)
        return false;

    // No SQL injection
    LoginDatabase.PExecute("UPDATE account SET expansion = '%d' WHERE id = '%u'",lev,account_id);
    PSendSysMessage(LANG_ACCOUNT_SETADDON,account_name.c_str(),account_id,lev);
    return true;
}


/// Create an account
bool ChatHandler::HandleAccountCreateCommand(char const* args)
{
    ARGS_CHECK

    std::string email;

    ///- %Parse the command line arguments
    char* accountName = strtok((char*)args, " ");
    char* password = strtok(nullptr, " ");
    char* possibleEmail = strtok(nullptr, " ' ");
    if (possibleEmail)
        email = possibleEmail;

    if (!accountName || !password)
        return false;

    AccountOpResult result = sAccountMgr->CreateAccount(std::string(accountName), std::string(password), email);
    switch (result)
    {
        case AOR_OK:
            PSendSysMessage(LANG_ACCOUNT_CREATED, accountName);
            if (GetSession())
            {
                TC_LOG_INFO("entities.player.character", "Account: %d (IP: %s) Character:[%s] (GUID: %u) created Account %s (Email: '%s')",
                    GetSession()->GetAccountId(), GetSession()->GetRemoteAddress().c_str(),
                    GetSession()->GetPlayer()->GetName().c_str(), GetSession()->GetPlayer()->GetGUIDLow(),
                    accountName, email.c_str());
            }
            break;
        case AOR_NAME_TOO_LONG:
            SendSysMessage(LANG_ACCOUNT_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        case AOR_NAME_ALREADY_EXIST:
            SendSysMessage(LANG_ACCOUNT_ALREADY_EXIST);
            SetSentErrorMessage(true);
            return false;
        case AOR_DB_INTERNAL_ERROR:
            PSendSysMessage(LANG_ACCOUNT_NOT_CREATED_SQL_ERROR, accountName);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_ACCOUNT_NOT_CREATED, accountName);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

/// Delete a user account and all associated characters in this realm
/// \todo This function has to be enhanced to respect the login/realm split (delete char, delete account chars in realm, delete account chars in realm then delete account
bool ChatHandler::HandleAccountDeleteCommand(const char* args)
{
    ARGS_CHECK

    ///- Get the account name from the command line
    char *account_name_str=strtok ((char*)args," ");
    if (!account_name_str)
        return false;

    std::string account_name = account_name_str;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account_id = sAccountMgr->GetId(account_name);
    if(!account_id)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    /// Commands not recommended call from chat, but support anyway
    if(m_session)
    {
        uint32 targetSecurity = sAccountMgr->GetSecurity(account_id);

        /// can delete only for account with less security
        /// This is also reject self apply in fact
        if (targetSecurity >= m_session->GetSecurity())
        {
            SendSysMessage (LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage (true);
            return false;
        }
    }

    AccountOpResult result = sAccountMgr->DeleteAccount(account_id);
    switch(result)
    {
        case AOR_OK:
            PSendSysMessage(LANG_ACCOUNT_DELETED,account_name.c_str());
            break;
        case AOR_NAME_NOT_EXIST:
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        case AOR_DB_INTERNAL_ERROR:
            PSendSysMessage(LANG_ACCOUNT_NOT_DELETED_SQL_ERROR,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_ACCOUNT_NOT_DELETED,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleAccountCommand(const char* /*args*/)
{
    uint32 gmlevel = m_session->GetSecurity();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, gmlevel);
    return true;
}

//Set a new mail and check if a change is pending
bool ChatHandler::HandleAccountMailChangeCommand(const char* args)
{
    ARGS_CHECK

    char* sAccount = strtok((char*)args, " ");
    char* mail = strtok(nullptr, " ");

    if (!sAccount || !mail)
        return false;

    std::string account_name = sAccount;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetAccountId = sAccountMgr->GetId(account_name);
    if (!targetAccountId)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

     LoginDatabase.PExecute("UPDATE account SET email='%s', newMail=NULL, newMailTS='0' WHERE id=%u", mail, targetAccountId);
     //TODO translate
     PSendSysMessage("Mail changed.");
     return true;
}

/// Set password for account
bool ChatHandler::HandleAccountSetPasswordCommand(const char* args)
{
    ARGS_CHECK

    ///- Get the command line arguments
    char *szAccount = strtok ((char*)args," ");
    char *szPassword1 =  strtok (nullptr," ");
    char *szPassword2 =  strtok (nullptr," ");

    if (!szAccount||!szPassword1 || !szPassword2)
        return false;

    std::string account_name = szAccount;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetAccountId = sAccountMgr->GetId(account_name);
    if (!targetAccountId)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetSecurity = sAccountMgr->GetSecurity(targetAccountId);

    /// m_session==NULL only for console
    uint32 plSecurity = m_session ? m_session->GetSecurity() : SEC_CONSOLE;

    /// can set password only for target with less security
    /// This is also reject self apply in fact
    if (targetSecurity >= plSecurity)
    {
        SendSysMessage (LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage (true);
        return false;
    }

    if (strcmp(szPassword1,szPassword2))
    {
        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = sAccountMgr->ChangePassword(targetAccountId, szPassword1);

    switch(result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_NAME_NOT_EXIST:
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleAccountSetGmLevelCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_CMD_SYNTAX);
        SetSentErrorMessage(true);
        return false;
    }

    std::string targetAccountName;
    uint32 targetAccountId = 0;
    uint32 targetSecurity = 0;
    uint32 gm = 0;
    char* arg1 = strtok((char*)args, " ");
    char* arg2 = strtok(nullptr, " ");
    char* arg3 = strtok(nullptr, " ");
    bool isAccountNameGiven = true;

    if (!arg3)
    {
        if (!GetSelectedPlayer())
            return false;
        isAccountNameGiven = false;
    }

    // Check for second parameter
    if (!isAccountNameGiven && !arg2)
        return false;

    // Check for account
    if (isAccountNameGiven)
    {
        targetAccountName = arg1;
        if (!AccountMgr::normalizeString(targetAccountName) || !AccountMgr::GetId(targetAccountName))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, targetAccountName.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    // Check for invalid specified GM level.
    gm = (isAccountNameGiven) ? atoi(arg2) : atoi(arg1);
    if (gm > SEC_CONSOLE)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    // handler->getSession() == NULL only for console
    targetAccountId = (isAccountNameGiven) ? AccountMgr::GetId(targetAccountName) : GetSelectedPlayer()->GetSession()->GetAccountId();
    int32 gmRealmID = (isAccountNameGiven) ? atoi(arg3) : atoi(arg2);
    uint32 playerSecurity;
    if (GetSession())
        playerSecurity = AccountMgr::GetSecurity(GetSession()->GetAccountId(), gmRealmID);
    else
        playerSecurity = SEC_CONSOLE;

    // can set security level only for target with less security and to less security that we have
    // This also restricts setting handler's own security.
    targetSecurity = AccountMgr::GetSecurity(targetAccountId, gmRealmID);
    if (targetSecurity >= playerSecurity || gm >= playerSecurity)
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return false;
    }

    // Check and abort if the target gm has a higher rank on one of the realms and the new realm is -1
    if (gmRealmID == -1 && !AccountMgr::IsConsoleAccount(playerSecurity))
    {
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ACCESS_GMLEVEL_TEST);

        stmt->setUInt32(0, targetAccountId);
        stmt->setUInt8(1, uint8(gm));

        PreparedQueryResult result = LoginDatabase.Query(stmt);

        if (result)
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage(true);
            return false;
        }
    }

    // Check if provided realmID has a negative value other than -1
    if (gmRealmID < -1)
    {
        SendSysMessage("You have not chosen -1 or the current realmID that you are on.");
        SetSentErrorMessage(true);
        return false;
    }

    rbac::RBACData* rbac = nullptr;
    //rbac = isAccountNameGiven ? NULL : GetSelectedPlayer()->GetSession()->GetRBACData(); //TODO RBAC
    sAccountMgr->UpdateAccountAccess(rbac, targetAccountId, uint8(gm), gmRealmID);

    PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
    return true;
}
