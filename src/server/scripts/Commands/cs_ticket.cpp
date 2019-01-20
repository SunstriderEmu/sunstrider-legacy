#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "Realm.h"
#include "TicketMgr.h"
#include "World.h"
#include "WorldSession.h"

class ticket_commandscript : public CommandScript
{
public:
    ticket_commandscript() : CommandScript("ticket_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
#ifdef LICH_KING
        static std::vector<ChatCommand> ticketResponseCommandTable =
        {
            { "append",   SEC_GAMEMASTER1, true,  &HandleGMTicketResponseAppendCommand,   "" },
            { "appendln", SEC_GAMEMASTER1, true,  &HandleGMTicketResponseAppendLnCommand, "" },
        };
#endif
        static std::vector<ChatCommand> ticketCommandTable =
        {
            { "assign",        SEC_GAMEMASTER1, true, &HandleGMTicketAssignToCommand,          "" },
            { "close",         SEC_GAMEMASTER1, true, &HandleGMTicketCloseByIdCommand,         "" },
            { "closedlist",    SEC_GAMEMASTER1, true, &HandleGMTicketListClosedCommand,        "" },
            { "comment",       SEC_GAMEMASTER1, true, &HandleGMTicketCommentCommand,           "" },
            { "complete",      SEC_GAMEMASTER1, true, &HandleGMTicketCompleteCommand,          "" },
            { "delete",        SEC_GAMEMASTER1, true, &HandleGMTicketDeleteByIdCommand,        "" },
            { "escalate",      SEC_GAMEMASTER1, true, &HandleGMTicketEscalateCommand,          "" },
            { "escalatedlist", SEC_GAMEMASTER1, true, &HandleGMTicketListEscalatedCommand,     "" },
            { "list",          SEC_GAMEMASTER1, true, &HandleGMTicketListCommand,              "" },
            { "onlinelist",    SEC_GAMEMASTER1, true, &HandleGMTicketListOnlineCommand,        "" },
            { "reset",         SEC_GAMEMASTER1, true, &HandleGMTicketResetCommand,             "" },
#ifdef LICH_KING
            { "response",      SEC_GAMEMASTER1, true, nullptr,              "", ticketResponseCommandTable },
#else
            { "response",      SEC_GAMEMASTER1, true, &HandleNotTBC,                           "" },
#endif
            { "togglesystem",  SEC_GAMEMASTER1, true, &HandleToggleGMTicketSystem,             "" },
            { "unassign",      SEC_GAMEMASTER1, true, &HandleGMTicketUnAssignCommand,          "" },
            { "viewid",        SEC_GAMEMASTER1, true, &HandleGMTicketGetByIdCommand,           "" },
            { "viewname",      SEC_GAMEMASTER1, true, &HandleGMTicketGetByNameCommand,         "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "ticket",        SEC_GAMEMASTER1, false, nullptr, "", ticketCommandTable },
        };
        return commandTable;
    }

    static bool HandleNotTBC(ChatHandler* handler, char const* args)
    {
        handler->SendSysMessage("Only for TLK");
        return true;
    }

    static bool HandleGMTicketAssignToCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* ticketIdStr = strtok((char*)args, " ");
        uint32 ticketId = atoi(ticketIdStr);

        char* targetStr = strtok(nullptr, " ");
        if (!targetStr)
            return false;

        std::string target(targetStr);
        if (!normalizePlayerName(target))
            return false;

        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        ObjectGuid targetGuid = sCharacterCache->GetCharacterGuidByName(target);
        uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(targetGuid);
        // Target must exist and have administrative rights
        /* TC
        if (!AccountMgr::HasPermission(accountId, rbac::RBAC_PERM_COMMANDS_BE_ASSIGNED_TICKET, realm.Id.Realm))
        */
        if(AccountMgr::GetSecurity(accountId) <= SEC_PLAYER)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_A);
            return true;
        }

        // If already assigned, leave
        if (ticket->IsAssignedTo(targetGuid))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_B, ticket->GetId());
            return true;
        }

        // If assigned to different player other than current, leave
        //! Console can override though
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (player && ticket->IsAssignedNotTo(player->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->GetId(), target.c_str());
            return true;
        }

        // Assign ticket
        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetAssignedTo(targetGuid, AccountMgr::IsAdminAccount(AccountMgr::GetSecurity(accountId, realm.Id.Realm)));
        ticket->SaveToDB(trans);
        sTicketMgr->UpdateLastChange();

        std::string msg = ticket->FormatMessageString(*handler, nullptr, target.c_str(), nullptr, nullptr, nullptr);
        handler->SendGlobalGMSysMessage(msg.c_str());
        return true;
    }

    static bool HandleGMTicketCloseByIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 ticketId = atoi(args);
        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed() || ticket->IsCompleted())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        // Ticket should be assigned to the player who tries to close it.
        // Console can override though
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (player && ticket->IsAssignedNotTo(player->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETCANNOTCLOSE, ticket->GetId());
            return true;
        }

        sTicketMgr->ResolveAndCloseTicket(ticket->GetId(), player ? player->GetGUID() : ObjectGuid(uint64(0)));
        sTicketMgr->UpdateLastChange();

        std::string msg = ticket->FormatMessageString(*handler, player ? player->GetName().c_str() : "Console", nullptr, nullptr, nullptr, nullptr);
        handler->SendGlobalGMSysMessage(msg.c_str());

        // Inform player, who submitted this ticket, that it is closed
        if (Player* submitter = ticket->GetPlayer())
        {
            WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
            data << uint32(GMTICKET_RESPONSE_TICKET_DELETED);
            submitter->SendDirectMessage(&data);
        }
        return true;
    }

    static bool HandleGMTicketCommentCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* ticketIdStr = strtok((char*)args, " ");
        uint32 ticketId = atoi(ticketIdStr);

        char* comment = strtok(nullptr, "\n");
        if (!comment)
            return false;

        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed())
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        // Cannot comment ticket assigned to someone else
        //! Console excluded
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (player && ticket->IsAssignedNotTo(player->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->GetId());
            return true;
        }

        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetComment(comment);
        ticket->SaveToDB(trans);
        sTicketMgr->UpdateLastChange();

        std::string msg = [&] {
            std::string const assignedName = ticket->GetAssignedToName();
            return ticket->FormatMessageString(*handler, nullptr,
                assignedName.empty() ? nullptr : assignedName.c_str(), nullptr, nullptr, nullptr);
        }();

        msg += handler->PGetParseString(LANG_COMMAND_TICKETLISTADDCOMMENT, player ? player->GetName().c_str() : "Console", comment);
        handler->SendGlobalGMSysMessage(msg.c_str());

        return true;
    }

    static bool HandleGMTicketListClosedCommand(ChatHandler* handler, char const* /*args*/)
    {
        sTicketMgr->ShowClosedList(*handler);
        return true;
    }

    static bool HandleGMTicketCompleteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* ticketIdStr = strtok((char*)args, " ");
        uint32 ticketId = atoi(ticketIdStr);

        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed() || ticket->IsCompleted())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

#ifdef LICH_KING
        char* response = strtok(nullptr, "\n");
        if (response)
        {
            // Cannot add response to ticket, assigned to someone else
            //! Console excluded
            Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
            if (player && ticket->IsAssignedNotTo(player->GetGUID()))
            {
                handler->PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->GetId());
                return true;
            }

            ticket->AppendResponse(response);
        }

        if (Player* player = ticket->GetPlayer())
            ticket->SendResponse(player->GetSession());
#else
        //not tested. Need to notify player the ticket is closed
        if (Player* player = ticket->GetPlayer())
            sTicketMgr->SendTicket(player->GetSession(), nullptr);
#endif

        Player* gm = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;

        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetResolvedBy(gm ? gm->GetGUID() : ObjectGuid(uint64(0)));
        ticket->SetCompleted();
        ticket->SaveToDB(trans);

        std::string msg = ticket->FormatMessageString(*handler, nullptr, nullptr,
            nullptr, nullptr, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetName().c_str() : "Console");
        handler->SendGlobalGMSysMessage(msg.c_str());
        sTicketMgr->UpdateLastChange();
        return true;
    }

    static bool HandleGMTicketDeleteByIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 ticketId = atoi(args);
        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        if (!ticket->IsClosed())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETCLOSEFIRST);
            return true;
        }

        std::string msg = ticket->FormatMessageString(*handler, nullptr, nullptr, nullptr, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetName().c_str() : "Console", nullptr);
        handler->SendGlobalGMSysMessage(msg.c_str());

        sTicketMgr->RemoveTicket(ticket->GetId());
        sTicketMgr->UpdateLastChange();

        if (Player* player = ticket->GetPlayer())
        {
            // Force abandon ticket
            WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
            data << uint32(GMTICKET_RESPONSE_TICKET_DELETED);
            player->SendDirectMessage(&data);
        }

        return true;
    }

    static bool HandleGMTicketEscalateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 ticketId = atoi(args);
        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed() || ticket->IsCompleted() || ticket->GetEscalatedStatus() != TICKET_UNASSIGNED)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        ticket->SetEscalatedStatus(TICKET_IN_ESCALATION_QUEUE);

        if (Player* player = ticket->GetPlayer())
            sTicketMgr->SendTicket(player->GetSession(), ticket);

        sTicketMgr->UpdateLastChange();
        return true;
    }

    static bool HandleGMTicketListEscalatedCommand(ChatHandler* handler, char const* /*args*/)
    {
        sTicketMgr->ShowEscalatedList(*handler);
        return true;
    }

    static bool HandleGMTicketListCommand(ChatHandler* handler, char const* /*args*/)
    {
        sTicketMgr->ShowList(*handler, false);
        return true;
    }

    static bool HandleGMTicketListOnlineCommand(ChatHandler* handler, char const* /*args*/)
    {
        sTicketMgr->ShowList(*handler, true);
        return true;
    }

    static bool HandleGMTicketResetCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (sTicketMgr->GetOpenTicketCount())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETPENDING);
            return true;
        }
        else
        {
            sTicketMgr->ResetTickets();
            handler->SendSysMessage(LANG_COMMAND_TICKETRESET);
        }

        return true;
    }

    static bool HandleToggleGMTicketSystem(ChatHandler* handler, char const* /*args*/)
    {
        bool status = !sTicketMgr->GetStatus();
        sTicketMgr->SetStatus(status);
        handler->PSendSysMessage(status ? LANG_ALLOW_TICKETS : LANG_DISALLOW_TICKETS);
        return true;
    }

    static bool HandleGMTicketUnAssignCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 ticketId = atoi(args);
        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }
        // Ticket must be assigned
        if (!ticket->IsAssigned())
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETNOTASSIGNED, ticket->GetId());
            return true;
        }

        // Get security level of player, whom this ticket is assigned to
        uint32 security = SEC_PLAYER;
        Player* assignedPlayer = ticket->GetAssignedPlayer();
        if (assignedPlayer)
            security = assignedPlayer->GetSession()->GetSecurity();
        else
        {
            ObjectGuid guid = ticket->GetAssignedToGUID();
            uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid);
            security = AccountMgr::GetSecurity(accountId, realm.Id.Realm);
        }

        // Check security
        //! If no m_session present it means we're issuing this command from the console
        uint32 mySecurity = handler->GetSession() ? handler->GetSession()->GetSecurity() : SEC_CONSOLE;
        if (security > mySecurity)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETUNASSIGNSECURITY);
            return true;
        }

        std::string assignedTo = ticket->GetAssignedToName(); // copy assignedto name because we need it after the ticket has been unnassigned
        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetUnassigned();
        ticket->SaveToDB(trans);
        sTicketMgr->UpdateLastChange();

        std::string msg = ticket->FormatMessageString(*handler, nullptr, assignedTo.c_str(),
            handler->GetSession() ? handler->GetSession()->GetPlayer()->GetName().c_str() : "Console", nullptr, nullptr);
        handler->SendGlobalGMSysMessage(msg.c_str());

        return true;
    }

    static bool HandleGMTicketGetByIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 ticketId = atoi(args);
        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed() || ticket->IsCompleted())
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetViewed();
        ticket->SaveToDB(trans);

        handler->SendSysMessage(ticket->FormatMessageString(*handler, true).c_str());
        return true;
    }

    static bool HandleGMTicketGetByNameCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string name(args);
        if (!normalizePlayerName(name))
            return false;

        // Detect target's GUID
        ObjectGuid guid;
        if (Player* player = ObjectAccessor::FindPlayerByName(name))
            guid = player->GetGUID();
        else
            guid = sCharacterCache->GetCharacterGuidByName(name);

        // Target must exist
        if (guid.IsEmpty())
        {
            handler->SendSysMessage(LANG_NO_PLAYERS_FOUND);
            return true;
        }

        // Ticket must exist
        GmTicket* ticket = sTicketMgr->GetTicketByPlayer(guid);
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->SetViewed();
        ticket->SaveToDB(trans);

        handler->SendSysMessage(ticket->FormatMessageString(*handler, true).c_str());
        return true;
    }

#ifdef LICH_KING
    static bool _HandleGMTicketResponseAppendCommand(char const* args, bool newLine, ChatHandler* handler)
    {
        if (!*args)
            return false;

        char* ticketIdStr = strtok((char*)args, " ");
        uint32 ticketId = atoi(ticketIdStr);

        char* response = strtok(nullptr, "\n");
        if (!response)
            return false;

        GmTicket* ticket = sTicketMgr->GetTicket(ticketId);
        if (!ticket || ticket->IsClosed())
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        // Cannot add response to ticket, assigned to someone else
        //! Console excluded
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (player && ticket->IsAssignedNotTo(player->GetGUID()))
        {
            handler->PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->GetId());
            return true;
        }

        SQLTransaction trans = SQLTransaction(nullptr);
        ticket->AppendResponse(response);
        if (newLine)
            ticket->AppendResponse("\n");
        ticket->SaveToDB(trans);

        return true;
    }

    static bool HandleGMTicketResponseAppendCommand(ChatHandler* handler, char const* args)
    {
        return _HandleGMTicketResponseAppendCommand(args, false, handler);
    }

    static bool HandleGMTicketResponseAppendLnCommand(ChatHandler* handler, char const* args)
    {
        return _HandleGMTicketResponseAppendCommand(args, true, handler);
    }
#endif
};

void AddSC_ticket_commandscript()
{
    new ticket_commandscript();
}
