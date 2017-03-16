#include "Chat.h"
#include "Language.h"

// Enables or disables hiding of the staff badge
bool ChatHandler::HandleGMChatCommand(const char* args)
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

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleGMListIngameCommand(const char* /*args*/)
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
                SendSysMessage(LANG_GMS_ON_SRV);
                first = false;
            }

            SendSysMessage(itr->second->GetName().c_str());
        }
    }

    if(first)
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

/// Display the list of GMs
bool ChatHandler::HandleGMListFullCommand(const char* /*args*/)
{
    ///- Get the accounts with GM Level >0
    QueryResult result = LoginDatabase.Query( "SELECT username,gmlevel FROM account WHERE gmlevel > 0" );
    if(result)
    {
        SendSysMessage(LANG_GMLIST);
        SendSysMessage("========================");
        SendSysMessage(LANG_GMLIST_HEADER);
        SendSysMessage("========================");

        ///- Circle through them. Display username and GM level
        do
        {
            Field *fields = result->Fetch();
            PSendSysMessage("|%15s|%6s|", fields[0].GetCString(),fields[1].GetCString());
        }while( result->NextRow() );

        PSendSysMessage("========================");
    }
    else
        PSendSysMessage(LANG_GMLIST_EMPTY);
    return true;
}

bool ChatHandler::HandleGMFlyModeCommand(const char* args)
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
        SendSysMessage(LANG_USE_BOL);
        return false;
    }

    //+replay handling?

    PSendSysMessage(LANG_COMMAND_FLYMODE_STATUS, unit->GetName().c_str(), args);
    return true;
}

bool ChatHandler::HandleGMStats(const char* args)
{
    uint32 accId = m_session->GetAccountId();
    
    QueryResult timeResult = CharacterDatabase.Query("SELECT GmWeekBeginTime FROM saved_variables");
    if (!timeResult) {
        PSendSysMessage("Error with GmWeekBeginTime.");
        return true;
    }
    
    Field* timeFields = timeResult->Fetch();
    uint64 beginTime = timeFields[0].GetUInt64();

    QueryResult countResult = CharacterDatabase.PQuery("SELECT COUNT(*) FROM gm_tickets WHERE timestamp > " UI64FMTD " AND closed = %u", beginTime, accId);
    if (!countResult) {
        PSendSysMessage("No information found for this account.");
        return true;
    }
    
    Field* countFields = countResult->Fetch();
    uint32 count = countFields[0].GetUInt64();
    
    //PSendSysMessage("Vous avez fermé %u tickets depuis le début de la semaine.", count);
    PSendSysMessage("You closed %u tickets since the beginning of the week.", count);
    
    return true;
}

//Enable\Dissable GM Mode
bool ChatHandler::HandleGMmodeCommand(const char* args)
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
        #ifdef _DEBUG_VMAPS
        VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
        vMapManager->processCommand("stoplog");
        #endif
        return true;
    } else  if (argstr == "off")
    {
        m_session->GetPlayer()->SetGameMaster(false);
        m_session->SendNotification(LANG_GM_OFF);
        #ifdef _DEBUG_VMAPS
        VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
        vMapManager->processCommand("startlog");
        #endif
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

//Enable\Dissable Invisible mode
bool ChatHandler::HandleGMVisibleCommand(const char* args)
{
    if (!*args)
    {
        PSendSysMessage(LANG_YOU_ARE, m_session->GetPlayer()->IsGMVisible() ?  GetTrinityString(LANG_VISIBLE) : GetTrinityString(LANG_INVISIBLE));
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

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}