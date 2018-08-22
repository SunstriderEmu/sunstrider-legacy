#include "Chat.h"
#include "Language.h"
#include "SpellHistory.h"

//Enable On\OFF all taxi paths
static bool HandleTaxiCheatCommand(ChatHandler* handler, char const* args)
{
    //default to "on" if no input specified
    std::string argstr = *args ? (char*)args : "on";

    Player *chr = GetSelectedPlayerOrSelf();

    if (argstr == "on")
    {
        chr->SetTaxiCheater(true);
        handler->PSendSysMessage(LANG_YOU_GIVE_TAXIS, chr->GetName().c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_ADDED, GetName().c_str());
        return true;
    }

    if (argstr == "off")
    {
        chr->SetTaxiCheater(false);
        handler->PSendSysMessage(LANG_YOU_REMOVE_TAXIS, chr->GetName().c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_REMOVED, GetName().c_str());

        return true;
    }

    handler->SendSysMessage(LANG_USE_BOL);
    handler->SetSentErrorMessage(true);
    return false;
}

static bool HandleGodModeCheatCommand(const char *args)
{
    if (!m_session || !m_session->GetPlayer())
        return false;

    std::string argstr = (char*)args;

    if (!*args)
        argstr = (m_session->GetPlayer()->GetCommandStatus(CHEAT_GOD)) ? "off" : "on";

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetCommandStatusOff(CHEAT_GOD);
        handler->SendSysMessage("Godmode is OFF. You can take damage.");
        return true;
    }
    else if (argstr == "on")
    {
        m_session->GetPlayer()->SetCommandStatusOn(CHEAT_GOD);
        handler->SendSysMessage("Godmode is ON. You won't take damage.");
        return true;
    }

    return false;
}

static bool HandleCasttimeCheatCommand(const char *args)
{
    if (!m_session || !m_session->GetPlayer())
        return false;

    std::string argstr = (char*)args;

    if (!*args)
        argstr = (m_session->GetPlayer()->GetCommandStatus(CHEAT_CASTTIME)) ? "off" : "on";

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetCommandStatusOff(CHEAT_CASTTIME);
        handler->SendSysMessage("CastTime Cheat is OFF. Your spells will have a casttime.");
        return true;
    }
    else if (argstr == "on")
    {
        m_session->GetPlayer()->SetCommandStatusOn(CHEAT_CASTTIME);
        handler->SendSysMessage("CastTime Cheat is ON. Your spells won't have a casttime.");
        return true;
    }

    return false;
}

static bool HandleCoolDownCheatCommand(const char *args)
{
    if (!m_session || !m_session->GetPlayer())
        return false;

    std::string argstr = (char*)args;

    if (!*args)
        argstr = (m_session->GetPlayer()->GetCommandStatus(CHEAT_COOLDOWN)) ? "off" : "on";

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetCommandStatusOff(CHEAT_COOLDOWN);
        handler->SendSysMessage("Cooldown Cheat is OFF. You are on the global cooldown.");
        return true;
    }
    else if (argstr == "on")
    {
        m_session->GetPlayer()->GetSpellHistory()->ResetAllCooldowns();
        m_session->GetPlayer()->SetCommandStatusOn(CHEAT_COOLDOWN);
        handler->SendSysMessage("Cooldown Cheat is ON. You are not on the global cooldown.");
        return true;
    }

    return false;
}

static bool HandlePowerCheatCommand(const char *args)
{
    if (!m_session || !m_session->GetPlayer())
        return false;

    std::string argstr = (char*)args;

    if (!*args)
        argstr = (m_session->GetPlayer()->GetCommandStatus(CHEAT_POWER)) ? "off" : "on";

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetCommandStatusOff(CHEAT_POWER);
        handler->SendSysMessage("Power Cheat is OFF. You need mana/rage/energy to use spells.");
        return true;
    }
    else if (argstr == "on")
    {
        m_session->GetPlayer()->SetCommandStatusOn(CHEAT_POWER);
        handler->SendSysMessage("Power Cheat is ON. You don't need mana/rage/energy to use spells.");
        return true;
    }

    return false;
}

static bool HandleWaterwalkCheatCommand(ChatHandler* handler, char const* args)
{
    Player *player = GetSelectedPlayerOrSelf();
    if(!player)
    {
        handler->PSendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (strncmp(args, "on", 3) == 0)
        player->SetMovement(MOVE_WATER_WALK);               // ON
    else if (strncmp(args, "off", 4) == 0)
        player->SetMovement(MOVE_LAND_WALK);                // OFF
    else
    {
        handler->SendSysMessage(LANG_USE_BOL);
        return false;
    }

    handler->PSendSysMessage(LANG_YOU_SET_WATERWALK, args, player->GetName().c_str());
    if(needReportToTarget(player))
        handler->PSendSysMessage(LANG_YOUR_WATERWALK_SET, args, GetName().c_str());
    return true;

}

static bool HandleExploreCheatCommand(ChatHandler* handler, char const* args)
{
    ARGS_CHECK

    int flag = atoi((char*)args);

    Player *chr = GetSelectedPlayerOrSelf();
    if (chr == nullptr)
    {
        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (flag != 0)
    {
        handler->PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, chr->GetName().c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_ALL,GetName().c_str());
    }
    else
    {
        handler->PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName().c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_NOTHING,GetName().c_str());
    }

    for (uint8 i=0; i<128; i++)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0);
        }
    }

    return true;
}
