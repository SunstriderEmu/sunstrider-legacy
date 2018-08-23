#include "Chat.h"
#include "Language.h"

class gm_commandscript : public CommandScript
{
public:
    gm_commandscript() : CommandScript("gm_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> gmCommandTable =
        {
            { "chat",           SEC_GAMEMASTER1,    false, &HandleGMChatCommand,              "" },
            { "ingame",         SEC_PLAYER,         true,  &HandleGMListIngameCommand,        "" },
            { "list",           SEC_GAMEMASTER3,    true,  &HandleGMListFullCommand,          "" },
            { "visible",        SEC_GAMEMASTER1,    false, &HandleGMVisibleCommand,           "" },
            { "fly",            SEC_GAMEMASTER3,    false, &HandleGMFlyModeCommand,           "" },
            { "stats",          SEC_GAMEMASTER1,    false, &HandleGMStats,                    "" },
            { "",               SEC_GAMEMASTER1,    false, &HandleGMmodeCommand,              "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "gm",             SEC_GAMEMASTER1,  true,  nullptr,                                        "", gmCommandTable },
        };
        return commandTable;
    }

    // Enables or disables hiding of the staff badge
    static bool HandleGMChatCommand(ChatHandler* handler, char const* args)
    {
        if(!*args)
        {
            if(handler->GetSession()->GetPlayer()->IsGMChat())
                handler->GetSession()->SendNotification(LANG_GM_CHAT_ON);
            else
                handler->GetSession()->SendNotification(LANG_GM_CHAT_OFF);
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetGMChat(true);
            handler->GetSession()->SendNotification(LANG_GM_CHAT_ON);
            return true;
        }

        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetGMChat(false);
            handler->GetSession()->SendNotification(LANG_GM_CHAT_OFF);
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleGMListIngameCommand(ChatHandler* handler, char const* /*args*/)
    {
        bool first = true;

        HashMapHolder<Player>::MapType &m = HashMapHolder<Player>::GetContainer();
        auto itr = m.begin();
        for(; itr != m.end(); ++itr)
        {
            AccountTypes itr_sec = (AccountTypes)itr->second->GetSession()->GetSecurity();

            //if target has gm mode enabled or has a listable gm account level
            //and target is visible to player (this check disabled if no session)
            if ((itr->second->IsGameMaster() || (itr_sec > SEC_PLAYER && itr_sec <= sWorld->getConfig(CONFIG_GM_LEVEL_IN_GM_LIST))) 
                && (!handler->GetSession() || itr->second->IsVisibleGloballyFor(handler->GetSession()->GetPlayer())))
            {
                if(first)
                {
                    handler->SendSysMessage(LANG_GMS_ON_SRV);
                    first = false;
                }

                handler->SendSysMessage(itr->second->GetName().c_str());
            }
        }

        if(first)
            handler->SendSysMessage(LANG_GMS_NOT_LOGGED);

        return true;
    }

    /// Display the list of GMs
    static bool HandleGMListFullCommand(ChatHandler* handler, char const* /*args*/)
    {
        ///- Get the accounts with GM Level >0
        QueryResult result = LoginDatabase.Query( "SELECT username,gmlevel FROM account WHERE gmlevel > 0" );
        if(result)
        {
            handler->SendSysMessage(LANG_GMLIST);
            handler->SendSysMessage("========================");
            handler->SendSysMessage(LANG_GMLIST_HEADER);
            handler->SendSysMessage("========================");

            ///- Circle through them. Display username and GM level
            do
            {
                Field *fields = result->Fetch();
                handler->PSendSysMessage("|%15s|%6s|", fields[0].GetCString(),fields[1].GetCString());
            }while( result->NextRow() );

            handler->PSendSysMessage("========================");
        }
        else
            handler->PSendSysMessage(LANG_GMLIST_EMPTY);
        return true;
    }

    static bool HandleGMFlyModeCommand(ChatHandler* handler, char const* args)
    {
        Unit *unit = handler->GetSelectedUnit();
        if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
            unit = handler->GetSession()->GetPlayer();

        WorldPacket data(12);
        if (strncmp(args, "on", 3) == 0)
        {
            ((Player*)(unit))->SetFlying(true);
        }
        else if (strncmp(args, "off", 4) == 0)
        {
            ((Player*)(unit))->SetFlying(false);
        }
        else
        {
            handler->SendSysMessage(LANG_USE_BOL);
            return false;
        }

        //+replay handling?

        handler->PSendSysMessage(LANG_COMMAND_FLYMODE_STATUS, unit->GetName().c_str(), args);
        return true;
    }

    static bool HandleGMStats(ChatHandler* handler, char const* args)
    {
        uint32 accId = handler->GetSession()->GetAccountId();
    
        QueryResult timeResult = CharacterDatabase.Query("SELECT GmWeekBeginTime FROM saved_variables");
        if (!timeResult) {
            handler->PSendSysMessage("Error with GmWeekBeginTime.");
            return true;
        }
    
        Field* timeFields = timeResult->Fetch();
        uint64 beginTime = timeFields[0].GetUInt64();

        QueryResult countResult = CharacterDatabase.PQuery("SELECT COUNT(*) FROM gm_tickets WHERE timestamp > " UI64FMTD " AND closed = %u", beginTime, accId);
        if (!countResult) {
            handler->PSendSysMessage("No information found for this account.");
            return true;
        }
    
        Field* countFields = countResult->Fetch();
        uint32 count = countFields[0].GetUInt64();
    
        //PSendSysMessage("Vous avez fermé %u tickets depuis le début de la semaine.", count);
        handler->PSendSysMessage("You closed %u tickets since the beginning of the week.", count);
    
        return true;
    }

    //Enable\Dissable GM Mode
    static bool HandleGMmodeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            if (handler->GetSession()->GetPlayer()->IsGameMaster())
                handler->GetSession()->SendNotification(LANG_GM_ON);
            else
                handler->GetSession()->SendNotification(LANG_GM_OFF);
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetGameMaster(true);
            handler->GetSession()->SendNotification(LANG_GM_ON);
            handler->GetSession()->GetPlayer()->UpdateTriggerVisibility();
#ifdef _DEBUG_VMAPS
            VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
            vMapManager->processCommand("stoplog");
#endif
            return true;
        }
        else  if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetGameMaster(false);
            handler->GetSession()->SendNotification(LANG_GM_OFF);
            handler->GetSession()->GetPlayer()->UpdateTriggerVisibility();
#ifdef _DEBUG_VMAPS
            VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
            vMapManager->processCommand("startlog");
#endif
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    //Enable\Dissable Invisible mode
    static bool HandleGMVisibleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(LANG_YOU_ARE, handler->GetSession()->GetPlayer()->IsGMVisible() ? handler->GetTrinityString(LANG_VISIBLE) : handler->GetTrinityString(LANG_INVISIBLE));
            return true;
        }

        std::string argstr = (char*)args;

        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetGMVisible(true);
            handler->GetSession()->SendNotification(LANG_INVISIBLE_VISIBLE);
            return true;
        }

        if (argstr == "off")
        {
            handler->GetSession()->SendNotification(LANG_INVISIBLE_INVISIBLE);
            handler->GetSession()->GetPlayer()->SetGMVisible(false);
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }
};

void AddSC_gm_commandscript()
{
    new gm_commandscript();
}
