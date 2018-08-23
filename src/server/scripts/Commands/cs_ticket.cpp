#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"

class ticket_commandscript : public CommandScript
{
public:
    ticket_commandscript() : CommandScript("ticket_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> ticketCommandTable =
        {
            { "list",           SEC_GAMEMASTER1,      false, &HandleGMTicketListCommand,             "" },
            { "onlinelist",     SEC_GAMEMASTER1,      false, &HandleGMTicketListOnlineCommand,       "" },
            { "viewname",       SEC_GAMEMASTER1,      false, &HandleGMTicketGetByNameCommand,        "" },
            { "viewid",         SEC_GAMEMASTER1,      false, &HandleGMTicketGetByIdCommand,          "" },
            { "close",          SEC_GAMEMASTER1,      false, &HandleGMTicketCloseByIdCommand,        "" },
            { "closedlist",     SEC_GAMEMASTER1,      false, &HandleGMTicketListClosedCommand,       "" },
            { "delete",         SEC_GAMEMASTER3,      false, &HandleGMTicketDeleteByIdCommand,       "" },
            { "assign",         SEC_GAMEMASTER1,      false, &HandleGMTicketAssignToCommand,         "" },
            { "unassign",       SEC_GAMEMASTER1,      false, &HandleGMTicketUnAssignCommand,         "" },
            { "comment",        SEC_GAMEMASTER1,      false, &HandleGMTicketCommentCommand,          "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "ticket",         SEC_GAMEMASTER1,      false, nullptr,                                "", ticketCommandTable },
        };
        return commandTable;
    }

    static void SendTicket(ChatHandler* handler, GM_Ticket const* ticket, time_t currentTime, bool showMessage = false, bool showComment = false, bool showAge = false, bool showAssign = false, bool globalMessage = false)
    {
        std::string gmname;
        std::stringstream ss;
        ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
        CharacterCacheEntry const* data = sCharacterCache->GetCharacterCacheByGuid(ticket->playerGuid);

        ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTNAME, data ? data->name.c_str() : "<name not found>", data ? data->name.c_str() : "<name not found>");
        if (showAge)
        {
            ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTAGECREATE, (secsToTimeString(time(nullptr) - ticket->createtime, true, false)).c_str());
            ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTAGE, (secsToTimeString(time(nullptr) - ticket->timestamp, true, false)).c_str());
        }
        if (showAssign)
        {
            data = sCharacterCache->GetCharacterCacheByGuid(ticket->assignedToGM);
            ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTASSIGNEDTO, data ? data->name.c_str() : "<name not found>");
        }
        if (showMessage)
            ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTMESSAGE, ticket->message.c_str());
        if (showComment)
            ss << handler->PGetParseString(LANG_COMMAND_TICKETLISTCOMMENT, ticket->comment.c_str());

        if (globalMessage)
            handler->SendGlobalGMSysMessage(ss.str().c_str());
        else
            handler->SendSysMessage(ss.str().c_str());

        return;
    }

    static bool HandleGMTicketList(ChatHandler* handler, bool onlineOnly, bool closedOnly)
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

            if (onlineOnly && !ObjectAccessor::FindPlayer((*itr)->playerGuid))
                continue;

            SendTicket(handler, *itr, now, false, false);
        }
        return true;
    }

    static bool HandleGMTicketListCommand(ChatHandler* handler, char const* args)
    {
        handler->SendSysMessage(LANG_COMMAND_TICKETSHOWLIST);
        return HandleGMTicketList(handler, false, false);
    }


    static bool HandleGMTicketListOnlineCommand(ChatHandler* handler, char const* args)
    {
        handler->SendSysMessage(LANG_COMMAND_TICKETSHOWONLINELIST);
        return HandleGMTicketList(handler, true, false);
    }

    static bool HandleGMTicketListClosedCommand(ChatHandler* handler, char const* args)
    {
        handler->SendSysMessage(LANG_COMMAND_TICKETSHOWCLOSEDLIST);
        return HandleGMTicketList(handler, false, true);
    }

    static bool HandleGMTicketGetByIdCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            uint64 tguid = atoi(args);
        GM_Ticket *ticket = sObjectMgr->GetGMTicket(tguid);
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        SendTicket(handler, ticket, time(nullptr), true, true);
        return true;
    }

    static bool HandleGMTicketGetByNameCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player *plr = ObjectAccessor::FindConnectedPlayerByName(args);
        if (!plr)
        {
            handler->SendSysMessage(LANG_NO_PLAYERS_FOUND);
            return true;
        }

        GM_Ticket *ticket = sObjectMgr->GetGMTicketByPlayer(plr->GetGUID());
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        SendTicket(handler, ticket, time(nullptr), true, true);
        return true;
    }

    static bool HandleGMTicketCloseByIdCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            uint64 tguid = atoi(args);
        GM_Ticket *ticket = sObjectMgr->GetGMTicket(tguid);
        if (!ticket || ticket->closed != 0)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }
        if (ticket && ticket->assignedToGM != 0 && (!handler->GetSession() || ticket->assignedToGM != handler->GetSession()->GetPlayer()->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETCANNOTCLOSE, ticket->guid);
            return true;
        }
        SendTicket(handler, ticket, time(nullptr), false, false, false, false, true);

        Player *plr = ObjectAccessor::FindPlayer(ticket->playerGuid);
        ticket->timestamp = time(nullptr);
        sObjectMgr->RemoveGMTicket(ticket, handler->GetSession() ? handler->GetSession()->GetAccountId() : 0);

        if (!plr || !plr->IsInWorld())
            return true;

        // send abandon ticket
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        plr->SendDirectMessage(&data);
        return true;
    }

    static bool HandleGMTicketAssignToCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* tguid = strtok((char*)args, " ");
        uint64 ticketGuid = atoi(tguid);
        char* targetgm = strtok(nullptr, " ");

        if (!targetgm)
            return false;

        std::string targm = targetgm;

        if (!normalizePlayerName(targm))
            return true;

        Player *cplr = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        std::string gmname;
        GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

        if (!ticket || ticket->closed != 0)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }
        ObjectGuid tarGUID = sCharacterCache->GetCharacterGuidByName(targm.c_str());
        uint64 accid = sCharacterCache->GetCharacterAccountIdByGuid(tarGUID);
        QueryResult result = LoginDatabase.PQuery("SELECT `gmlevel` FROM `account` WHERE `id` = '%u'", accid);
        if (!tarGUID || !result || result->Fetch()->GetUInt32() < SEC_GAMEMASTER1)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_A);
            return true;
        }

        if (ticket->assignedToGM == tarGUID)
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_B, ticket->guid);
            return true;
        }
        sCharacterCache->GetCharacterNameByGuid(tarGUID, gmname);
        if (ticket->assignedToGM != 0 && (!cplr || ticket->assignedToGM != cplr->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->guid, gmname.c_str());
            return true;
        }

        ticket->assignedToGM = tarGUID;
        sObjectMgr->AddOrUpdateGMTicket(*ticket);
        SendTicket(handler, ticket, 0, false, false, false, true, true);
        return true;
    }

    static bool HandleGMTicketUnAssignCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            uint64 ticketGuid = atoi(args);
        Player* cplr = handler->GetSession()->GetPlayer();
        GM_Ticket* ticket = sObjectMgr->GetGMTicket(ticketGuid);

        if (!ticket || ticket->closed != 0)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }
        if (ticket->assignedToGM == 0)
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETNOTASSIGNED, ticket->guid);
            return true;
        }

        std::string gmname;
        sCharacterCache->GetCharacterNameByGuid(ticket->assignedToGM, gmname);
        Player *plr = ObjectAccessor::FindPlayer(ticket->assignedToGM);
        if (plr && plr->IsInWorld() && plr->GetSession()->GetSecurity() > cplr->GetSession()->GetSecurity())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETUNASSIGNSECURITY);
            return true;
        }

        SendTicket(handler, ticket, 0, false, false, false, true, true);
        std::string str = handler->GetTrinityStringVA(LANG_COMMAND_TICKETLISTUNASSIGNED, cplr->GetName().c_str());
        handler->SendGlobalGMSysMessage(str.c_str());

        ticket->assignedToGM.Clear();
        sObjectMgr->AddOrUpdateGMTicket(*ticket);
        return true;
    }

    static bool HandleGMTicketCommentCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* tguid = strtok((char*)args, " ");
        uint64 ticketGuid = atoi(tguid);
        char* comment = strtok(nullptr, "\n");

        if (!comment)
            return false;

        Player *cplr = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

        if (!ticket || ticket->closed != 0)
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }
        if (ticket->assignedToGM != 0 && (!cplr || ticket->assignedToGM != cplr->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->guid);
            return true;
        }

        ticket->comment = comment;
        sObjectMgr->AddOrUpdateGMTicket(*ticket);

        SendTicket(handler, ticket, false, false, false, false, true, true);
        std::string str = handler->PGetParseString(LANG_COMMAND_TICKETLISTADDCOMMENT, cplr ? cplr->GetName().c_str() : "", ticket->comment.c_str());
        handler->SendGlobalGMSysMessage(str.c_str());
        return true;
    }

    static bool HandleGMTicketDeleteByIdCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
            uint64 ticketGuid = atoi(args);
        GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticketGuid);

        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }
        if (ticket->closed == 0)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETCLOSEFIRST);
            return true;
        }

        SendTicket(handler, ticket, 0, false, false, false, false, true);
        std::string str = handler->PGetParseString(LANG_COMMAND_TICKETDELETED, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetName().c_str() : "");
        handler->SendGlobalGMSysMessage(str.c_str());
        Player *plr = ObjectAccessor::FindPlayer(ticket->playerGuid);
        sObjectMgr->RemoveGMTicket(ticket, -1, true);
        if (plr && plr->IsInWorld())
        {
            // Force abandon ticket
            WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
            data << uint32(9);
            plr->SendDirectMessage(&data);
        }

        ticket = nullptr;
        return true;
    }
};

void AddSC_ticket_commandscript()
{
    new ticket_commandscript();
}
