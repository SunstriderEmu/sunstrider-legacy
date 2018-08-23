#include "Chat.h"
#include "Language.h"

class group_commandscript : public CommandScript
{
public:
    group_commandscript() : CommandScript("group_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> groupCommandTable =
        {
            { "leader",         SEC_GAMEMASTER3,     false, &HandleGroupLeaderCommand,         "" },
            { "disband",        SEC_GAMEMASTER3,     false, &HandleGroupDisbandCommand,        "" },
            { "remove",         SEC_GAMEMASTER3,     false, &HandleGroupRemoveCommand,         "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "group",          SEC_GAMEMASTER3,     false, nullptr,                           "", groupCommandTable },
        };
        return commandTable;
    }

    static bool HandleGroupLeaderCommand(ChatHandler* handler, char const* args)
    {
        Player* plr = nullptr;
        Group* group = nullptr;
        ObjectGuid guid;
        char* cname = strtok((char*)args, " ");

        if (handler->GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
            if (group && group->GetLeaderGUID() != guid)
                group->ChangeLeader(guid);

        return true;
    }

    static bool HandleGroupDisbandCommand(ChatHandler* handler, char const* args)
    {
        Player* plr = nullptr;
        Group* group = nullptr;
        ObjectGuid guid;
        char* cname = strtok((char*)args, " ");

        if (handler->GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
            if (group)
                group->Disband();

        return true;
    }

    static bool HandleGroupRemoveCommand(ChatHandler* handler, char const* args)
    {
        Player* plr = nullptr;
        Group* group = nullptr;
        ObjectGuid guid;
        char* cname = strtok((char*)args, " ");

        if (handler->GetPlayerGroupAndGUIDByName(cname, plr, group, guid, true))
            if (group)
                group->RemoveMember(guid);

        return true;
    }
};

void AddSC_group_commandscript()
{
    new group_commandscript();
}

