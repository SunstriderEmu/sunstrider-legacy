#include "Chat.h"
#include "Language.h"

static bool HandleGroupLeaderCommand(ChatHandler* handler, char const* args)
{
    Player* plr  = nullptr;
    Group* group = nullptr;
    ObjectGuid guid;
    char* cname  = strtok((char*)args, " ");

    if(GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
        if(group && group->GetLeaderGUID() != guid)
            group->ChangeLeader(guid);

    return true;
}

static bool HandleGroupDisbandCommand(ChatHandler* handler, char const* args)
{
    Player* plr  = nullptr;
    Group* group = nullptr;
    ObjectGuid guid;
    char* cname  = strtok((char*)args, " ");

    if(GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
        if(group)
            group->Disband();

    return true;
}

static bool HandleGroupRemoveCommand(ChatHandler* handler, char const* args)
{
    Player* plr  = nullptr;
    Group* group = nullptr;
    ObjectGuid guid;
    char* cname  = strtok((char*)args, " ");

    if(GetPlayerGroupAndGUIDByName(cname, plr, group, guid, true))
        if(group)
            group->RemoveMember(guid);

    return true;
}
