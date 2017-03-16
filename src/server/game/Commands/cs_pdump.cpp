#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "PlayerDump.h"

bool ChatHandler::HandleLoadPDumpCommand(const char *args)
{
    char * file = strtok((char*)args, " ");
    if(!file)
        return false;

    char * account = strtok(nullptr, " ");
    if(!account)
        return false;

    std::string account_name = account;
    if(!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account_id = sAccountMgr->GetId(account_name);
    if(!account_id)
    {
        account_id = atoi(account);                             // use original string
        if(!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    if(!sAccountMgr->GetName(account_id,account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    char* guid_str = nullptr;
    char* name_str = strtok(nullptr, " ");

    std::string name;
    if(name_str)
    {
        name = name_str;
        // normalize the name if specified and check if it exists
        if(!normalizePlayerName(name))
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
            SetSentErrorMessage(true);
            return false;
        }

        if(!ObjectMgr::CheckPlayerName(name,true))
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
            SetSentErrorMessage(true);
            return false;
        }

        guid_str = strtok(nullptr, " ");
    }

    uint32 guid = 0;

    if(guid_str)
    {
        guid = atoi(guid_str);
        if(!guid)
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_GUID);
            SetSentErrorMessage(true);
            return false;
        }

        if(sCharacterCache->GetCharacterAccountIdByGuid(guid))
        {
            PSendSysMessage(LANG_CHARACTER_GUID_IN_USE,guid);
            SetSentErrorMessage(true);
            return false;
        }
    }

    switch(PlayerDumpReader().LoadDump(file, account_id, name, guid))
    {
        case DUMP_SUCCESS:
            PSendSysMessage(LANG_COMMAND_IMPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            PSendSysMessage(LANG_FILE_OPEN_FAIL,file);
            SetSentErrorMessage(true);
            return false;
        case DUMP_FILE_BROKEN:
            PSendSysMessage(LANG_DUMP_BROKEN,file);
            SetSentErrorMessage(true);
            return false;
        case DUMP_TOO_MANY_CHARS:
            PSendSysMessage(LANG_ACCOUNT_CHARACTER_LIST_FULL,account_name.c_str(),account_id);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_COMMAND_IMPORT_FAILED);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleWritePDumpCommand(const char *args)
{
    char* file = strtok((char*)args, " ");
    char* p2 = strtok(nullptr, " ");

    if(!file || !p2)
        return false;

    uint32 guid;
    // character name can't start from number
    if (isNumeric(p2[0]))
        guid = atoi(p2);
    else
    {
        std::string name = p2;

        if (!normalizePlayerName (name))
        {
            SendSysMessage (LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage (true);
            return false;
        }

        guid = sCharacterCache->GetCharacterGuidByName(name);
    }

    if(!sCharacterCache->GetCharacterAccountIdByGuid(guid))
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    switch(PlayerDumpWriter().WriteDump(file, guid))
    {
        case DUMP_SUCCESS:
            PSendSysMessage(LANG_COMMAND_EXPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            PSendSysMessage(LANG_FILE_OPEN_FAIL,file);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_COMMAND_EXPORT_FAILED);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}
