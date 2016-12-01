/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "UpdateMask.h"
#include "NPCHandler.h"

void WorldSession::SendNameQueryOpcode(uint64 guid)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    GlobalPlayerData const* nameData = sWorld->GetGlobalPlayerData(GUID_LOPART(guid));

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8 + 1 + 4 + 4 + 4 + 1));
#ifdef BUILD_335_SUPPORT
    if (GetClientBuild() == BUILD_335)
    {
        data.appendPackGUID(guid);
        if (!nameData)
        {
            data << uint8(1);                           // name unknown
            SendPacket(&data);
            return;
        }

        data << uint8(0);                               // name known
    } else
#endif
    {
        if (!nameData)
            return; //simply ignore request
                    // guess size

        data << guid;
    }

    data << nameData->name;
    data << uint8(0);                               // realm name - only set for cross realm interaction (such as Battlegrounds)
    data << uint32(nameData->race);
    data << uint32(nameData->gender);
    data << uint32(nameData->playerClass);

    if (DeclinedName const* names = (player ? player->GetDeclinedNames() : nullptr))
    {
        data << uint8(1);                           // Name is declined
        for (const auto & i : names->name)
            data << i;
    }
    else
        data << uint8(0);                           // Name is not declined

    SendPacket(&data);
}

void WorldSession::HandleNameQueryOpcode( WorldPacket & recvData )
{
    uint64 guid;
    recvData >> guid;

    SendNameQueryOpcode(guid);
}

void WorldSession::HandleQueryTimeOpcode( WorldPacket & /*recvData*/ )
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4+4);
    data << uint32(time(nullptr));
    data << uint32(sWorld->GetNextDailyQuestsResetTime() - time(nullptr));
    SendPacket(&data);
}

/// Only _static_ data send in this packet !!!
void WorldSession::HandleCreatureQueryOpcode( WorldPacket & recvData )
{
    uint32 entry;
    recvData >> entry;
    /* //Not used, but 335 client send guid too:
    uint64 guid;
    if(GetClientBuild() == BUILD_335)
        recvData >> guid;
    */

    CreatureTemplate const *ci = sObjectMgr->GetCreatureTemplate(entry);
    if (ci)
    {
        std::string Name, SubName;
        Name = ci->Name;
        SubName = ci->SubName;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if (loc_idx >= 0)
        {
            CreatureLocale const *cl = sObjectMgr->GetCreatureLocale(entry);
            if (cl)
            {
                ObjectMgr::GetLocaleString(cl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(cl->SubName, loc_idx, SubName);
            }
        }
//        TC_LOG_DEBUG("network","WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name.c_str(), entry);
        // guess size
        WorldPacket data( SMSG_CREATURE_QUERY_RESPONSE, 100 );
        data << (uint32)entry;                              // creature entry
        data << Name;
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4, always empty
        data << SubName;
        data << ci->IconName;                               // "Directions" for guard, string for Icons 2.3.0
        data << (uint32)ci->type_flags;                     // flags          wdbFeild7=wad flags1
        data << (uint32)ci->type;
        data << (uint32)ci->family;                         // family         wdbFeild9
        data << (uint32)ci->rank;                           // rank           wdbFeild10
#ifdef BUILD_335_SUPPORT
        if(GetClientBuild() == BUILD_335)
        {
#ifdef LICH_KING
            data << uint32(ci->KillCredit[0]);              // new in 3.1, kill credit
            data << uint32(ci->KillCredit[1]);              // new in 3.1, kill credit
#else
            data << uint32(0);
            data << uint32(0);
#endif
        } else 
#endif
        {
            data << (uint32)0;                              // unknown        wdbFeild11
            data << (uint32)ci->PetSpellDataId;             // Id from CreatureSpellData.dbc    wdbField12
        }
        data << (uint32)ci->Modelid1;                       // Modelid1
        data << (uint32)ci->Modelid2;                       // Modelid2
        data << (uint32)ci->Modelid3;                       // Modelid3
        data << (uint32)ci->Modelid4;                       // Modelid4
        data << float(ci->ModHealth);                       // dmg/hp modifier
        data << float(ci->ModMana);                         // dmg/mana modifier
        data << (uint8)ci->RacialLeader;

#ifdef BUILD_335_SUPPORT
        if(GetClientBuild() == BUILD_335)
        {
#ifdef LICH_KING
            CreatureQuestItemList const* items = sObjectMgr->GetCreatureQuestItemList(entry);
            if (items)
                for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
                    data << (i < items->size() ? uint32((*items)[i]) : uint32(0));
            else
                for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
                    data << uint32(0);

            data << uint32(ci->movementId);                     // CreatureMovementInfo.dbc
#else
            for (uint32 i = 0; i < 6; ++i)
                data << uint32(0);

            data << uint32(0);
#endif
        }
#endif

        SendPacket( &data );
    }
    else
    {
        uint64 guid;
        recvData >> guid;

        TC_LOG_ERROR("network","WORLD: CMSG_CREATURE_QUERY - NO CREATURE INFO! (GUID: %u, ENTRY: %u)",
            GUID_LOPART(guid), entry);
        WorldPacket data( SMSG_CREATURE_QUERY_RESPONSE, 4 );
        data << uint32(entry | 0x80000000);
        SendPacket( &data );
    }
    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
}

/// Only _static_ data send in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recvData )
{
    uint32 entryID;
    recvData >> entryID;
    /* //Not used, but 335 client send guid too:
    uint64 guid;
    if(GetClientBuild() == BUILD_335)
        recvData >> guid;
    */

    const GameObjectTemplate *info = sObjectMgr->GetGameObjectTemplate(entryID);
    if(info)
    {
        std::string Name;
        std::string IconName;
        std::string CastBarCaption;

        Name = info->name;
        IconName = info->IconName;
        CastBarCaption = info->castBarCaption;

        LocaleConstant loc_idx = GetSessionDbcLocale();
        if (loc_idx >= 0)
        {
            GameObjectLocale const *gl = sObjectMgr->GetGameObjectLocale(entryID);
            if (gl)
            {
                if (gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                    Name = gl->Name[loc_idx];
                if (gl->CastBarCaption.size() > loc_idx && !gl->CastBarCaption[loc_idx].empty())
                    CastBarCaption = gl->CastBarCaption[loc_idx];
            }
        }
       // TC_LOG_DEBUG("network.opcode","WORLD: CMSG_GAMEOBJECT_QUERY '%s' - Entry: %u. ", info->name.c_str(), entryID);
        WorldPacket data ( SMSG_GAMEOBJECT_QUERY_RESPONSE, 150 );
        data << uint32(entryID);
        data << uint32(info->type);
        data << uint32(info->displayId);
        data << Name;
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4
        data << IconName;                                   // 2.0.3, string
        data << CastBarCaption;                             // 2.0.3, string. Text will appear in Cast Bar when using GO (ex: "Collecting")
        data << uint8(0);                                   // 2.0.3, probably string
        data.append(info->raw.data,MAX_GAMEOBJECT_DATA);

#ifdef BUILD_335_SUPPORT
        if (GetClientBuild() == BUILD_335)
        {
#ifdef LICH_KING
            for (uint32 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
                data << uint32(info->questItems[i]);              // itemId[6], quest drop
#else
            for (uint32 i = 0; i < 6; ++i)
                data << uint32(0);
#endif
        }
#endif

        SendPacket( &data );
    }
    else
    {

        uint64 guid;
        recvData >> guid;

        TC_LOG_ERROR("FIXME",  "WORLD: CMSG_GAMEOBJECT_QUERY - Missing gameobject info for (GUID: %u, ENTRY: %u)",
            GUID_LOPART(guid), entryID );
        WorldPacket data ( SMSG_GAMEOBJECT_QUERY_RESPONSE, 4 );
        data << uint32(entryID | 0x80000000);
        SendPacket( &data );
    }
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket & /*recvData*/)
{
    Corpse *corpse = GetPlayer()->GetCorpse();

    uint8 found = 1;
    if(!corpse)
        found = 0;

    WorldPacket data(MSG_CORPSE_QUERY, (1+found*(5*4)));
    data << uint8(found);
    if(found)
    {
        data << corpse->GetMapId();
        data << corpse->GetPositionX();
        data << corpse->GetPositionY();
        data << corpse->GetPositionZ();
        data << _player->GetMapId();
        //data << uint32(0);                                      // added on LK
    }
    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode( WorldPacket & recvData )
{
    

    uint32 textID;
    uint64 guid;

    recvData >> textID;
    TC_LOG_DEBUG("network", "WORLD: CMSG_NPC_TEXT_QUERY TextId: %u", textID);

    recvData >> guid;
    //needed? GetPlayer()->SetTarget(guid);

    GossipText const* gossip = sObjectMgr->GetGossipText(textID);

    WorldPacket data( SMSG_NPC_TEXT_UPDATE, 100 );          // guess size
    data << textID;

    if (!gossip)
    {
        for(uint32 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << float(0);
            if(GetSessionDbcLocale() == LOCALE_frFR)
            {
                data << "Salutations $N";
                data << "Salutations $N";
            } else {
                data << "Greetings $N";
                data << "Greetings $N";
            }
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
        }
    }
    else
    {
        std::string text0[MAX_GOSSIP_TEXT_OPTIONS], text1[MAX_GOSSIP_TEXT_OPTIONS];
        LocaleConstant locale = GetSessionDbLocaleIndex();

        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            BroadcastText const* bct = sObjectMgr->GetBroadcastText(gossip->Options[i].BroadcastTextID);
            if (bct)
            {
                text0[i] = bct->GetText(locale, GENDER_MALE, true);
                text1[i] = bct->GetText(locale, GENDER_FEMALE, true);
            }
            else
            {
                text0[i] = gossip->Options[i].Text_0;
                text1[i] = gossip->Options[i].Text_1;
            }

            if (locale != DEFAULT_LOCALE && !bct)
            {
                if (NpcTextLocale const* npcTextLocale = sObjectMgr->GetNpcTextLocale(textID))
                {
                    ObjectMgr::GetLocaleString(npcTextLocale->Text_0[i], locale, text0[i]);
                    ObjectMgr::GetLocaleString(npcTextLocale->Text_1[i], locale, text1[i]);
                }
            }

            data << gossip->Options[i].Probability;

            if (text0[i].empty())
                data << text1[i];
            else
                data << text0[i];

            if (text1[i].empty())
                data << text0[i];
            else
                data << text1[i];

            data << gossip->Options[i].Language;

            for (auto Emote : gossip->Options[i].Emotes)
            {
                data << Emote._Delay;
                data << Emote._Emote;
            }
        }
    }

    SendPacket( &data );

    //TC_LOG_DEBUG("network", "WORLD: Sent SMSG_NPC_TEXT_UPDATE");
}

void WorldSession::HandlePageTextQueryOpcode( WorldPacket & recvData )
{
    uint32 pageID;

    recvData >> pageID;
    //LK, dunno if BC. Don't need it anyway //recvData.read_skip<uint64>();                          // guid

    //TC_LOG_DEBUG("network","WORLD: Received CMSG_PAGE_TEXT_QUERY for pageID '%u'", pageID);

    while (pageID)
    {
        PageText const* pPage = sObjectMgr->GetPageText(pageID);
                                                            // guess size
        WorldPacket data( SMSG_PAGE_TEXT_QUERY_RESPONSE, 50 );
        data << pageID;

        if (!pPage)
        {
            data << "Item page missing.";
            data << uint32(0);
            pageID = 0;
        }
        else
        {
            std::string Text = pPage->Text;

            LocaleConstant loc_idx = GetSessionDbcLocale();
            if (loc_idx >= 0)
            {
                PageTextLocale const *pl = sObjectMgr->GetPageTextLocale(pageID);
                if (pl)
                {
                    if (pl->Text.size() > loc_idx && !pl->Text[loc_idx].empty())
                        Text = pl->Text[loc_idx];
                }
            }

            data << Text;
            data << uint32(pPage->NextPage);
            pageID = pPage->NextPage;
        }
        SendPacket( &data );
    }
}

