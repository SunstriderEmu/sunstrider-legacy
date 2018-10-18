#include "Chat.h"
#include "Language.h"
#include "SpellHistory.h"

class cheat_commandscript : public CommandScript
{
public:
    cheat_commandscript() : CommandScript("cheat_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> cheatCommandTable =
        {
            { "god",            SEC_GAMEMASTER2,  false, &HandleGodModeCheatCommand,            "" },
            { "casttime",       SEC_GAMEMASTER3,  false, &HandleCasttimeCheatCommand,           "" },
            { "cooldown",       SEC_GAMEMASTER3,  false, &HandleCoolDownCheatCommand,           "" },
            { "power",          SEC_GAMEMASTER3,  false, &HandlePowerCheatCommand,              "" },
            { "taxi",           SEC_GAMEMASTER3,  false, &HandleTaxiCheatCommand,               "" },
            { "explore",        SEC_GAMEMASTER3,  false, &HandleExploreCheatCommand,            "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "cheat",          SEC_GAMEMASTER2,  false, nullptr,                               "", cheatCommandTable },
        };
        return commandTable;
    }

    //Enable On\OFF all taxi paths
    static bool HandleTaxiCheatCommand(ChatHandler* handler, char const* args)
    {
        //default to "on" if no input specified
        std::string argstr = *args ? (char*)args : "on";

        Player *chr = handler->GetSelectedPlayerOrSelf();

        if (argstr == "on")
        {
            chr->SetTaxiCheater(true);
            handler->PSendSysMessage(LANG_YOU_GIVE_TAXIS, chr->GetName().c_str());
            if (handler->needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_ADDED, handler->GetName().c_str());
            return true;
        }

        if (argstr == "off")
        {
            chr->SetTaxiCheater(false);
            handler->PSendSysMessage(LANG_YOU_REMOVE_TAXIS, chr->GetName().c_str());
            if (handler->needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_REMOVED, handler->GetName().c_str());

            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleGodModeCheatCommand(ChatHandler* handler, char const* args)
    {
        if (!handler->GetSession() || !handler->GetSession()->GetPlayer())
            return false;

        std::string argstr = (char*)args;

        if (!*args)
            argstr = (handler->GetSession()->GetPlayer()->GetCommandStatus(CHEAT_GOD)) ? "off" : "on";

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOff(CHEAT_GOD);
            handler->SendSysMessage("Godmode is OFF. You can take damage.");
            return true;
        }
        else if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOn(CHEAT_GOD);
            handler->SendSysMessage("Godmode is ON. You won't take damage.");
            return true;
        }

        return false;
    }

    static bool HandleCasttimeCheatCommand(ChatHandler* handler, char const* args)
    {
        if (!handler->GetSession() || !handler->GetSession()->GetPlayer())
            return false;

        std::string argstr = (char*)args;

        if (!*args)
            argstr = (handler->GetSession()->GetPlayer()->GetCommandStatus(CHEAT_CASTTIME)) ? "off" : "on";

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOff(CHEAT_CASTTIME);
            handler->SendSysMessage("CastTime Cheat is OFF. Your spells will have a casttime.");
            return true;
        }
        else if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOn(CHEAT_CASTTIME);
            handler->SendSysMessage("CastTime Cheat is ON. Your spells won't have a casttime.");
            return true;
        }

        return false;
    }

    static bool HandleCoolDownCheatCommand(ChatHandler* handler, char const* args)
    {
        if (!handler->GetSession() || !handler->GetSession()->GetPlayer())
            return false;

        std::string argstr = (char*)args;

        if (!*args)
            argstr = (handler->GetSession()->GetPlayer()->GetCommandStatus(CHEAT_COOLDOWN)) ? "off" : "on";

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOff(CHEAT_COOLDOWN);
            handler->SendSysMessage("Cooldown Cheat is OFF. You are on the global cooldown.");
            return true;
        }
        else if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->GetSpellHistory()->ResetAllCooldowns();
            handler->GetSession()->GetPlayer()->SetCommandStatusOn(CHEAT_COOLDOWN);
            handler->SendSysMessage("Cooldown Cheat is ON. You are not on the global cooldown.");
            return true;
        }

        return false;
    }

    static bool HandlePowerCheatCommand(ChatHandler* handler, char const* args)
    {
        if (!handler->GetSession() || !handler->GetSession()->GetPlayer())
            return false;

        std::string argstr = (char*)args;

        if (!*args)
            argstr = (handler->GetSession()->GetPlayer()->GetCommandStatus(CHEAT_POWER)) ? "off" : "on";

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOff(CHEAT_POWER);
            handler->SendSysMessage("Power Cheat is OFF. You need mana/rage/energy to use spells.");
            return true;
        }
        else if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetCommandStatusOn(CHEAT_POWER);
            handler->SendSysMessage("Power Cheat is ON. You don't need mana/rage/energy to use spells.");
            return true;
        }

        return false;
    }

    static bool HandleExploreCheatCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            int flag = atoi((char*)args);

        Player *chr = handler->GetSelectedPlayerOrSelf();
        if (chr == nullptr)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (flag != 0)
        {
            handler->PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, chr->GetName().c_str());
            if (handler->needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_ALL, handler->GetName().c_str());
        }
        else
        {
            handler->PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName().c_str());
            if (handler->needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_NOTHING, handler->GetName().c_str());
        }

        for (uint8 i = 0; i < 128; i++)
        {
            if (flag != 0)
                handler->GetSession()->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0xFFFFFFFF);
            else
                handler->GetSession()->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0);
        }

        return true;
    }
};

void AddSC_cheat_commandscript()
{
    new cheat_commandscript();
}
