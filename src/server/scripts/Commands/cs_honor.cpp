#include "Chat.h"
#include "Language.h"

class honor_commandscript : public CommandScript
{
public:
    honor_commandscript() : CommandScript("honor_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> honorCommandTable =
        {
            { "add",            SEC_GAMEMASTER2,     false, &HandleAddHonorCommand,            "" },
            { "addkill",        SEC_GAMEMASTER2,     false, &HandleHonorAddKillCommand,        "" },
            { "update",         SEC_GAMEMASTER2,     false, &HandleUpdateHonorFieldsCommand,   "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "honor",          SEC_GAMEMASTER2,  false, nullptr,                              "", honorCommandTable },
        };
        return commandTable;
    }

    static bool HandleAddHonorCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        Player *target = handler->GetSelectedPlayerOrSelf();
        if (!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 amount = (uint32)atoi(args);
        target->RewardHonor(nullptr, 1, amount);
        return true;
    }

    static bool HandleHonorAddKillCommand(ChatHandler* handler, char const* /*args*/)
    {
        Unit *target = handler->GetSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->GetSession()->GetPlayer()->RewardHonor(target, 1);
        return true;
    }

    static bool HandleUpdateHonorFieldsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *target = handler->GetSelectedPlayerOrSelf();
        if (!target)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->UpdateHonorFields();
        return true;
    }
};

void AddSC_honor_commandscript()
{
    new honor_commandscript();
}

