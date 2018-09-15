/*
 * Copyright (C) 2005-2008 MaNGOS
 *
 * Copyright (C) 2008 Trinity
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Language.h"
#include "WorldPacket.h"
#include "Common.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "World.h"

#define ValidateLinksAndMaybeKick(str)                                                                              \
{                                                                                                                   \
    if (!Trinity::Hyperlinks::ValidateLinks(str))                                                                   \
    {                                                                                                               \
        TC_LOG_ERROR("network", "Player %s (GUID: %u) tried to add an invalid link to a GM ticket - corrected",     \
            GetPlayer()->GetName().c_str(), GetPlayer()->GetGUID().GetCounter());                                   \
                                                                                                                    \
        if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))                                            \
        {                                                                                                           \
            KickPlayer();                                                                                           \
            return;                                                                                                 \
        }                                                                                                           \
    }                                                                                                               \
}

void WorldSession::HandleGMTicketCreateOpcode( WorldPacket & recvData )
{
    if(GM_Ticket *ticket = sObjectMgr->GetGMTicketByPlayer(GetPlayer()->GetGUID()))
    {
      WorldPacket data( SMSG_GMTICKET_CREATE, 4 );
      data << uint32(1); // 1 - You already have GM ticket
      SendPacket( &data );
      return;
    }

    uint32 map;
    float x, y, z;
    std::string ticketText, ticketText2;

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);
    recvData >> map;
    recvData >> x;
    recvData >> y;
    recvData >> z;
    recvData >> ticketText;

    ValidateLinksAndMaybeKick(ticketText);

    recvData >> ticketText2;

    ValidateLinksAndMaybeKick(ticketText2);

    auto ticket = new GM_Ticket;
    ticket->guid = sObjectMgr->GenerateGMTicketId();
    ticket->playerGuid = GetPlayer()->GetGUID();
    ticket->message = ticketText;
    ticket->createtime = time(nullptr);
    ticket->map = map;
    ticket->pos_x = x;
    ticket->pos_y = y;
    ticket->pos_z = z;
    ticket->timestamp = time(nullptr);
    ticket->closed = 0;
    ticket->assignedToGM = ObjectGuid::Empty;
    ticket->comment = "";

    sObjectMgr->AddOrUpdateGMTicket(*ticket, true);

    sWorld->SendGMText(LANG_COMMAND_TICKETNEW, GetPlayer()->GetName().c_str(), ticket->guid);
}

void WorldSession::HandleGMTicketUpdateOpcode( WorldPacket & recvData)
{
    WorldPacket data(SMSG_GMTICKET_UPDATETEXT, 4);

    std::string message;
    recvData >> message;

    ValidateLinksAndMaybeKick(message);

    GM_Ticket *ticket = sObjectMgr->GetGMTicketByPlayer(GetPlayer()->GetGUID());
    if(!ticket)
    {
        data << uint32(1);
        SendPacket(&data);
        return;
    }

    ticket->message = message;
    ticket->timestamp = time(nullptr);

    sObjectMgr->AddOrUpdateGMTicket(*ticket);

    data << uint32(2);
    SendPacket(&data);

    sWorld->SendGMText(LANG_COMMAND_TICKETUPDATED, GetPlayer()->GetName().c_str(), ticket->guid);
}

void WorldSession::HandleGMTicketDeleteOpcode( WorldPacket & /*recvData*/)
{
    GM_Ticket* ticket = sObjectMgr->GetGMTicketByPlayer(GetPlayer()->GetGUID());
    if(ticket)
    {
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        SendPacket(&data);

        sWorld->SendGMText(LANG_COMMAND_TICKETPLAYERABANDON, GetPlayer()->GetName().c_str(), ticket->guid );
        sObjectMgr->RemoveGMTicket(ticket, GetPlayer()->GetGUID(), false);
        SendGMTicketGetTicket(0x0A, nullptr);
    }
}

void WorldSession::HandleGMTicketGetTicketOpcode( WorldPacket & /*recvData*/)
{
    WorldPacket data( SMSG_QUERY_TIME_RESPONSE, 4+4 );
    data << (uint32)time(nullptr);
    data << (uint32)0;
    SendPacket( &data );

    GM_Ticket *ticket = sObjectMgr->GetGMTicketByPlayer(GetPlayer()->GetGUID());

    if(ticket)
      SendGMTicketGetTicket(0x06, ticket->message.c_str());
    else
      SendGMTicketGetTicket(0x0A, nullptr);
}

void WorldSession::HandleGMTicketSystemStatusOpcode( WorldPacket & /*recvData*/)
{
    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);
    data << uint32(1);
    SendPacket(&data);
}

void WorldSession::SendGMTicketGetTicket(uint32 status, char const* text)
{
    int len = text ? strlen(text) : 0;
    WorldPacket data( SMSG_GMTICKET_GETTICKET, (4+len+1+4+2+4+4) );
    data << uint32(status); // standard 0x0A, 0x06 if text present
    if(status == 6)
    {
        data << text; // ticket text
        data << uint8(0x7); // ticket category
        data << float(0); // tickets in queue?
        data << float(0); // if > "tickets in queue" then "We are currently experiencing a high volume of petitions."
        data << float(0); // 0 - "Your ticket will be serviced soon", 1 - "Wait time currently unavailable"
        data << uint8(0); // if == 2 and next field == 1 then "Your ticket has been escalated"
        data << uint8(0); // const
    }
    SendPacket( &data );
}

void WorldSession::HandleGMSurveySubmit(WorldPacket& recvData)
{
    //sun: packet structure confirmed for TBC via binary reversing
    /*
    uint32 nextSurveyID = sTicketMgr->GetNextSurveyID();
    // just put the survey into the database
    uint32 mainSurvey; // GMSurveyCurrentSurvey.dbc, column 1 (all 9) ref to GMSurveySurveys.dbc
    recvData >> mainSurvey;

    std::unordered_set<uint32> surveyIds;
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // sub_survey1, r1, comment1, sub_survey2, r2, comment2, sub_survey3, r3, comment3, sub_survey4, r4, comment4, sub_survey5, r5, comment5, sub_survey6, r6, comment6, sub_survey7, r7, comment7, sub_survey8, r8, comment8, sub_survey9, r9, comment9, sub_survey10, r10, comment10,
    for (uint8 i = 0; i < 10; i++)
    {
        uint32 subSurveyId; // ref to i'th GMSurveySurveys.dbc field (all fields in that dbc point to fields in GMSurveyQuestions.dbc)
        recvData >> subSurveyId;
        if (!subSurveyId)
            break;

        uint8 rank; // probably some sort of ref to GMSurveyAnswers.dbc
        recvData >> rank;
        std::string comment; // comment ("Usage: GMSurveyAnswerSubmit(question, rank, comment)")
        recvData >> comment;

        // make sure the same sub survey is not added to DB twice
        if (!surveyIds.insert(subSurveyId).second)
            continue;

        ValidateLinksAndMaybeKick(comment);

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GM_SUBSURVEY);
        stmt->setUInt32(0, nextSurveyID);
        stmt->setUInt32(1, subSurveyId);
        stmt->setUInt32(2, rank);
        stmt->setString(3, comment);
        trans->Append(stmt);
    }

    std::string comment; // just a guess
    recvData >> comment;

    ValidateLinksAndMaybeKick(comment);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GM_SURVEY);
    stmt->setUInt32(0, GetPlayer()->GetGUID().GetCounter());
    stmt->setUInt32(1, nextSurveyID);
    stmt->setUInt32(2, mainSurvey);
    stmt->setString(3, comment);

    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
    */
}
