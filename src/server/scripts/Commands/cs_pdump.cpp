#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "PlayerDump.h"
#include "AccountMgr.h"

class pdump_commandscript : public CommandScript
{
public:
    pdump_commandscript() : CommandScript("pdump_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> pdumpCommandTable =
        {
            { "load",           SEC_GAMEMASTER3,  true, &HandleLoadPDumpCommand,           "" },
            { "write",          SEC_GAMEMASTER3,  true, &HandleWritePDumpCommand,          "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "pdump",          SEC_GAMEMASTER3,  true,  nullptr,                          "", pdumpCommandTable },
        };
        return commandTable;
    }

    static bool HandleLoadPDumpCommand(ChatHandler* handler, char const* args)
    {
        char * file = strtok((char*)args, " ");
        if (!file)
            return false;

        char * account = strtok(nullptr, " ");
        if (!account)
            return false;

        std::string account_name = account;
        if (!AccountMgr::normalizeString(account_name))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_name.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 account_id = sAccountMgr->GetId(account_name);
        if (!account_id)
        {
            account_id = atoi(account);                             // use original string
            if (!account_id)
            {
                handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_name.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!sAccountMgr->GetName(account_id, account_name))
        {
            handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_name.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* guid_str = nullptr;
        char* name_str = strtok(nullptr, " ");

        std::string name;
        if (name_str)
        {
            name = name_str;
            // normalize the name if specified and check if it exists
            if (!normalizePlayerName(name))
            {
                handler->PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (!ObjectMgr::CheckPlayerName(name, true))
            {
                handler->PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
                handler->SetSentErrorMessage(true);
                return false;
            }

            guid_str = strtok(nullptr, " ");
        }

        uint32 guid = 0;

        if (guid_str)
        {
            guid = atoi(guid_str);
            if (!guid)
            {
                handler->PSendSysMessage(LANG_INVALID_CHARACTER_GUID);
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (sCharacterCache->GetCharacterAccountIdByGuid(ObjectGuid(HighGuid::Player, guid)))
            {
                handler->PSendSysMessage(LANG_CHARACTER_GUID_IN_USE, guid);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        switch (PlayerDumpReader().LoadDump(file, account_id, name, guid))
        {
        case DUMP_SUCCESS:
            handler->PSendSysMessage(LANG_COMMAND_IMPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            handler->PSendSysMessage(LANG_FILE_OPEN_FAIL, file);
            handler->SetSentErrorMessage(true);
            return false;
        case DUMP_FILE_BROKEN:
            handler->PSendSysMessage(LANG_DUMP_BROKEN, file);
            handler->SetSentErrorMessage(true);
            return false;
        case DUMP_TOO_MANY_CHARS:
            handler->PSendSysMessage(LANG_ACCOUNT_CHARACTER_LIST_FULL, account_name.c_str(), account_id);
            handler->SetSentErrorMessage(true);
            return false;
        default:
            handler->PSendSysMessage(LANG_COMMAND_IMPORT_FAILED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleWritePDumpCommand(ChatHandler* handler, char const* args)
    {
        char* file = strtok((char*)args, " ");
        char* p2 = strtok(nullptr, " ");

        if (!file || !p2)
            return false;

        uint32 guid;
        // character name can't start from number
        if (isNumeric(p2[0]))
            guid = atoi(p2);
        else
        {
            std::string name = p2;

            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            guid = sCharacterCache->GetCharacterGuidByName(name);
        }

        if (!sCharacterCache->GetCharacterAccountIdByGuid(ObjectGuid(HighGuid::Player, guid)))
        {
            handler->PSendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        switch (PlayerDumpWriter().WriteDump(file, guid))
        {
        case DUMP_SUCCESS:
            handler->PSendSysMessage(LANG_COMMAND_EXPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            handler->PSendSysMessage(LANG_FILE_OPEN_FAIL, file);
            handler->SetSentErrorMessage(true);
            return false;
        default:
            handler->PSendSysMessage(LANG_COMMAND_EXPORT_FAILED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }
};

void AddSC_pdump_commandscript()
{
    new pdump_commandscript();
}
