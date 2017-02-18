#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"

std::string ChatHandler::PGetParseString(int32 entry, ...)
{
        const char *format = GetTrinityString(entry);
        va_list ap;
        char str [1024];
        va_start(ap, entry);
        vsnprintf(str,1024,format, ap );
        va_end(ap);
        return (std::string)str;
}

void ChatHandler::SendTicket(GM_Ticket const* ticket, time_t currentTime, bool showMessage, bool showComment, bool showAge, bool showAssign, bool globalMessage)
{
    std::string gmname;
    std::stringstream ss;
    ss << PGetParseString(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    CharacterCacheEntry const* data = sCharacterCache->GetCharacterCacheByGuid(ticket->playerGuid);

    ss << PGetParseString(LANG_COMMAND_TICKETLISTNAME, data ? data->name.c_str() : "<name not found>", data ? data->name.c_str() : "<name not found>");
    if (showAge)
    {
        ss << PGetParseString(LANG_COMMAND_TICKETLISTAGECREATE, (secsToTimeString(time(nullptr) - ticket->createtime, true, false)).c_str());
        ss << PGetParseString(LANG_COMMAND_TICKETLISTAGE, (secsToTimeString(time(nullptr) - ticket->timestamp, true, false)).c_str());
    }
    if (showAssign)
    {
        data = sCharacterCache->GetCharacterCacheByGuid(ticket->assignedToGM);
        ss << PGetParseString(LANG_COMMAND_TICKETLISTASSIGNEDTO, data ? data->name.c_str() : "<name not found>");
    }
    if (showMessage)
        ss << PGetParseString(LANG_COMMAND_TICKETLISTMESSAGE, ticket->message.c_str());
    if (showComment)
        ss << PGetParseString(LANG_COMMAND_TICKETLISTCOMMENT, ticket->comment.c_str());

    if (globalMessage)
        SendGlobalGMSysMessage(ss.str().c_str());
    else
        SendSysMessage(ss.str().c_str());

    return;
}

bool ChatHandler::HandleGMTicketList(bool onlineOnly, bool closedOnly)
{
    time_t now = time(nullptr);
    for (auto itr = sObjectMgr->m_GMTicketList.begin(); itr != sObjectMgr->m_GMTicketList.end(); ++itr)
    {
        bool closed = ((*itr)->closed != 0);
        //skip closed if closedOnly is not set 
        if (!closedOnly && closed)
            continue;

        //skip non closed if closedOnly
        if (closedOnly && !closed)
            continue;

        if ((*itr)->closed != 0)
            continue;

        if (onlineOnly && !sObjectMgr->GetPlayer((*itr)->playerGuid))
            continue;

        SendTicket(*itr, now, false, false);
    }
    return true;
}

bool ChatHandler::HandleGMTicketListCommand(const char* args)
{
  SendSysMessage(LANG_COMMAND_TICKETSHOWLIST);
  return HandleGMTicketList(false, false);
}


bool ChatHandler::HandleGMTicketListOnlineCommand(const char* args)
{
  SendSysMessage(LANG_COMMAND_TICKETSHOWONLINELIST);
  return HandleGMTicketList(true, false);
}

bool ChatHandler::HandleGMTicketListClosedCommand(const char* args)
{
  SendSysMessage(LANG_COMMAND_TICKETSHOWCLOSEDLIST);
  return HandleGMTicketList(false, true);
}

bool ChatHandler::HandleGMTicketGetByIdCommand(const char* args)
{
    ARGS_CHECK

    uint64 tguid = atoi(args);
    GM_Ticket *ticket = sObjectMgr->GetGMTicket(tguid);
    if(!ticket) 
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    SendTicket(ticket, time(nullptr), true, true);
    return true;
}

bool ChatHandler::HandleGMTicketGetByNameCommand(const char* args)
{
    ARGS_CHECK

    Player *plr = sObjectAccessor->FindConnectedPlayerByName(args);
    if(!plr)
    {
        SendSysMessage(LANG_NO_PLAYERS_FOUND);
        return true;
    }
  
    GM_Ticket *ticket = sObjectMgr->GetGMTicketByPlayer(plr->GetGUID()); 
    if(!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    SendTicket(ticket, time(nullptr), true, true);
    return true;
}

bool ChatHandler::HandleGMTicketCloseByIdCommand(const char* args)
{
  ARGS_CHECK

  uint64 tguid = atoi(args);
  GM_Ticket *ticket = sObjectMgr->GetGMTicket(tguid);
  if(!ticket || ticket->closed != 0)
  {
    SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
    return true;
  }
  if(ticket && ticket->assignedToGM != 0 && (!m_session || ticket->assignedToGM != m_session->GetPlayer()->GetGUID()))
  {
    PSendSysMessage(LANG_COMMAND_TICKETCANNOTCLOSE, ticket->guid);
    return true;
  }
  SendTicket(ticket, time(nullptr), false, false, false, false, true);

  Player *plr = sObjectMgr->GetPlayer(ticket->playerGuid);
  ticket->timestamp = time(nullptr);
  sObjectMgr->RemoveGMTicket(ticket, m_session ? m_session->GetAccountId() : 0); 

  if(!plr || !plr->IsInWorld())
    return true;

  // send abandon ticket
  WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
  data << uint32(9);
  plr->SendDirectMessage( &data );
  return true;
}

bool ChatHandler::HandleGMTicketAssignToCommand(const char* args)
{
    ARGS_CHECK

  char* tguid = strtok((char*)args, " ");
  uint64 ticketGuid = atoi(tguid);
  char* targetgm = strtok( nullptr, " ");

  if(!targetgm)
    return false;

  std::string targm = targetgm;

  if(!normalizePlayerName(targm))
    return true;

  Player *cplr = m_session ? m_session->GetPlayer() : nullptr;
  std::string gmname;
  GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

  if(!ticket || ticket->closed != 0)
  {
    SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
    return true;
  }
  uint64 tarGUID = sCharacterCache->GetCharacterGuidByName(targm.c_str());
  uint64 accid = sCharacterCache->GetCharacterAccountIdByGuid(tarGUID);
  QueryResult result = LoginDatabase.PQuery("SELECT `gmlevel` FROM `account` WHERE `id` = '%u'", accid);
  if(!tarGUID|| !result || result->Fetch()->GetUInt32() < SEC_GAMEMASTER1)
  {
    SendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_A);
    return true;
  }

  if(ticket->assignedToGM == tarGUID)
  {
    PSendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_B, ticket->guid);
    return true;
  }
  sCharacterCache->GetCharacterNameByGuid(tarGUID, gmname);
  if(ticket->assignedToGM != 0 && (!cplr || ticket->assignedToGM != cplr->GetGUID()))
  {
    PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->guid, gmname.c_str());
    return true;
  }

  ticket->assignedToGM = tarGUID;
  sObjectMgr->AddOrUpdateGMTicket(*ticket);
  SendTicket(ticket, 0, false, false, false, true, true);
  return true;
}

bool ChatHandler::HandleGMTicketUnAssignCommand(const char* args)
{
    ARGS_CHECK

  uint64 ticketGuid = atoi(args);
  Player *cplr = m_session->GetPlayer();
  GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

  if(!ticket|| ticket->closed != 0)
  {
    SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
    return true;
  }
  if(ticket->assignedToGM == 0)
  {
    PSendSysMessage(LANG_COMMAND_TICKETNOTASSIGNED, ticket->guid);
    return true;
  }

  std::string gmname;
  sCharacterCache->GetCharacterNameByGuid(ticket->assignedToGM, gmname);
  Player *plr = sObjectMgr->GetPlayer(ticket->assignedToGM);
  if(plr && plr->IsInWorld() && plr->GetSession()->GetSecurity() > cplr->GetSession()->GetSecurity())
  {
    SendSysMessage(LANG_COMMAND_TICKETUNASSIGNSECURITY);
    return true;
  }

  SendTicket(ticket, 0, false, false, false, true, true);
  std::string str = GetTrinityStringVA(LANG_COMMAND_TICKETLISTUNASSIGNED, cplr->GetName().c_str());
  SendGlobalGMSysMessage(str.c_str());

  ticket->assignedToGM = 0;
  sObjectMgr->AddOrUpdateGMTicket(*ticket);
  return true;
}

bool ChatHandler::HandleGMTicketCommentCommand(const char* args)
{
    ARGS_CHECK

  char* tguid = strtok((char*)args, " ");
  uint64 ticketGuid = atoi(tguid);
  char* comment = strtok( nullptr, "\n");

  if(!comment)
    return false;

  Player *cplr = m_session ? m_session->GetPlayer() : nullptr;
  GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

  if(!ticket || ticket->closed != 0)
  {
    PSendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
    return true;
  }
  if(ticket->assignedToGM != 0 && (!cplr || ticket->assignedToGM != cplr->GetGUID()))
  {
    PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->guid);
    return true;
  }

  ticket->comment = comment;
  sObjectMgr->AddOrUpdateGMTicket(*ticket);

  SendTicket(ticket, false, false, false, false, true, true);
  std::string str = PGetParseString(LANG_COMMAND_TICKETLISTADDCOMMENT, cplr ? cplr->GetName().c_str() : "", ticket->comment.c_str());
  SendGlobalGMSysMessage(str.c_str());
  return true;
}

bool ChatHandler::HandleGMTicketDeleteByIdCommand(const char* args)
{
    ARGS_CHECK
  uint64 ticketGuid = atoi(args);
  GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

  if(!ticket)
  {
    SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
    return true;
  }
  if(ticket->closed == 0)
  {
    SendSysMessage(LANG_COMMAND_TICKETCLOSEFIRST);
    return true;
  }

  SendTicket(ticket, 0, false, false, false, false, true);
  std::string str = PGetParseString(LANG_COMMAND_TICKETDELETED, m_session ? m_session->GetPlayer()->GetName().c_str() : "");
  SendGlobalGMSysMessage(str.c_str());
  Player *plr = sObjectMgr->GetPlayer(ticket->playerGuid);
  sObjectMgr->RemoveGMTicket(ticket, -1, true);
  if(plr && plr->IsInWorld())
  {
    // Force abandon ticket
    WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
    data << uint32(9);
    plr->SendDirectMessage( &data );
  }

  ticket = nullptr;
  return true;
}
