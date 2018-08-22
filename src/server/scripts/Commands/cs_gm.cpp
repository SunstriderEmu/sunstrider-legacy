#include "Chat.h"
#include "Language.h"

// Enables or disables hiding of the staff badge
static bool HandleGMChatCommand(ChatHandler* handler, char const* args)
{
    if(!*args)
    {
        if(m_session->GetPlayer()->IsGMChat())
            m_session->SendNotification(LANG_GM_CHAT_ON);
        else
            m_session->SendNotification(LANG_GM_CHAT_OFF);
        return true;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGMChat(true);
        m_session->SendNotification(LANG_GM_CHAT_ON);
        return true;
    }

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetGMChat(false);
        m_session->SendNotification(LANG_GM_CHAT_OFF);
        return true;
    }

    handler->SendSysMessage(LANG_USE_BOL);
    handler->SetSentErrorMessage(true);
    return false;
}

static bool HandleGMListIngameCommand(const char* /*args*/)
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
            && (!m_session || itr->second->IsVisibleGloballyFor(m_session->GetPlayer())))
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
static bool HandleGMListFullCommand(const char* /*args*/)
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
    Unit *unit = GetSelectedUnit();
    if (!unit || (unit->GetTypeId() != TYPEID_PLAYER))
        unit = m_session->GetPlayer();

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
    uint32 accId = m_session->GetAccountId();
    
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
    if(!*args)
    {
        if(m_session->GetPlayer()->IsGameMaster())
            m_session->SendNotification(LANG_GM_ON);
        else
            m_session->SendNotification(LANG_GM_OFF);
        return true;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGameMaster(true);
        m_session->SendNotification(LANG_GM_ON);
        m_session->GetPlayer()->UpdateTriggerVisibility();
        #ifdef _DEBUG_VMAPS
        VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
        vMapManager->processCommand("stoplog");
        #endif
        return true;
    } else  if (argstr == "off")
    {
        m_session->GetPlayer()->SetGameMaster(false);
        m_session->SendNotification(LANG_GM_OFF);
        m_session->GetPlayer()->UpdateTriggerVisibility();
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
        handler->PSendSysMessage(LANG_YOU_ARE, m_session->GetPlayer()->IsGMVisible() ?  GetTrinityString(LANG_VISIBLE) : GetTrinityString(LANG_INVISIBLE));
        return true;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGMVisible(true);
        m_session->SendNotification(LANG_INVISIBLE_VISIBLE);
        return true;
    }

    if (argstr == "off")
    {
        m_session->SendNotification(LANG_INVISIBLE_INVISIBLE);
        m_session->GetPlayer()->SetGMVisible(false);
        return true;
    }

    handler->SendSysMessage(LANG_USE_BOL);
    handler->SetSentErrorMessage(true);
    return false;
}