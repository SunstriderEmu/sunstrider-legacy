#include "Chat.h"
#include "Language.h"

bool ChatHandler::HandleAddHonorCommand(const char* args)
{
    ARGS_CHECK

    Player *target = GetSelectedPlayerOrSelf();
    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 amount = (uint32)atoi(args);
    target->RewardHonor(nullptr, 1, amount);
    return true;
}

bool ChatHandler::HandleHonorAddKillCommand(const char* /*args*/)
{
    

    Unit *target = GetSelectedUnit();
    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->RewardHonor(target, 1);
    return true;
}

bool ChatHandler::HandleUpdateHonorFieldsCommand(const char* /*args*/)
{
    Player *target = GetSelectedPlayerOrSelf();
    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    target->UpdateHonorFields();
    return true;
}
