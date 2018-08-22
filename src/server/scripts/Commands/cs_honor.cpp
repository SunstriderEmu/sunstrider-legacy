#include "Chat.h"
#include "Language.h"

static bool HandleAddHonorCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    Player *target = GetSelectedPlayerOrSelf();
    if(!target)
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    uint32 amount = (uint32)atoi(args);
    target->RewardHonor(nullptr, 1, amount);
    return true;
}

static bool HandleHonorAddKillCommand(const char* /*args*/)
{
    

    Unit *target = GetSelectedUnit();
    if(!target)
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->RewardHonor(target, 1);
    return true;
}

static bool HandleUpdateHonorFieldsCommand(const char* /*args*/)
{
    Player *target = GetSelectedPlayerOrSelf();
    if(!target)
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    target->UpdateHonorFields();
    return true;
}
