#include "Chat.h"
#include "Language.h"
#include "BattleGround.h"

static bool HandleSpectateVersion(const char *args)
{
    if(!sWorld->getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
    {
        handler->PSendSysMessage("Arena spectator deactivated.");
        return true;
    }

    ARGS_CHECK

    std::string version = args;

    handler->PSendSysMessage("Addon Spectator Version: %s", version.c_str());

    return true;
}

static bool HandleSpectateCancelCommand(const char* /*args*/)
{
    if(!sWorld->getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
    {
        //PSendSysMessage("Arena spectator désactivé.");
        handler->PSendSysMessage("Arena spectator deactivated.");
        return true;
    }

    Player* player =  GetSession()->GetPlayer();

    Battleground *bg = player->GetBattleground();
    if (!bg)
        return true;

    if (!bg->isSpectator(player->GetGUID()))
        return true;

    player->CancelSpectate();

    if (player->TeleportToBGEntryPoint())
    {
        player->SetSpectate(false);
        bg->RemoveSpectator(player->GetGUID());
    }

    return true;
}

static bool HandleSpectateFromCommand(const char *args)
{
    if(!sWorld->getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
    {
        //PSendSysMessage("Arena spectator désactivé.");
        handler->PSendSysMessage("Arena spectator deactivated.");
        return true;
    }

    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
        return false;

    Player* player = GetSession()->GetPlayer();

    if (!target)
    {
        //PSendSysMessage("Le joueur ciblé est introuvable.");
        handler->PSendSysMessage("Targeted player not found.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (!player->isSpectator())
    {
        //PSendSysMessage("Vous n'êtes pas spectateur.");
        handler->PSendSysMessage("You are not a spectator.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (target->isSpectator())
    {
        //PSendSysMessage("Vous ne pouvez pas faire cela car le joueur ciblé est aussi spectateur.");
        handler->PSendSysMessage("You cannot do this if the targeted player is also a spectator.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (player->GetMap() != target->GetMap())
    {
        //PSendSysMessage("Vous ne pouvez pas faire cela car vous n'êtes pas dans la même arène que le joueur ciblé.");
        handler->PSendSysMessage("You cannot do this if you are not in the same arena as the targeted player.");
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (Battleground* bg = target->GetBattleground())
    {
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
        {
            //PSendSysMessage("Vous ne pouvez pas faire cela car l'arène n'a pas encore commencé.");
            handler->PSendSysMessage("You cannot do this when the arena has not started.");
            handler->SetSentErrorMessage(true);
            return false;
        }
    }

    if (player->getSpectateFrom())
    {
        if (target == player->getSpectateFrom())
            player->getSpectateFrom()->RemovePlayerFromVision(player);
        else
        {
            player->getSpectateFrom()->RemovePlayerFromVision(player);
            target->AddPlayerToVision(player);
        }
        return true;
    }
    else
        target->AddPlayerToVision(player);

    return true;
}

static bool HandleSpectateInitCommand(const char *args)
{
    if(!sWorld->getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
        return true;

    if (Player* player = GetSession()->GetPlayer())
        player->SendDataForSpectator();

    return true;
}