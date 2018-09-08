
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "QueryPackets.h"
#include "CharacterCache.h"
#include "GameTime.h"

void WorldSession::SendNameQueryOpcode(ObjectGuid guid)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    CharacterCacheEntry const* nameData = sCharacterCache->GetCharacterCacheByGuid(guid.GetCounter());

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8 + 1 + 4 + 4 + 4 + 1));
#ifdef LICH_KING
    data.appendPackGUID(guid);
    if (!nameData)
    {
        data << uint8(1);                           // name unknown
        SendPacket(&data);
        return;
    }

    data << uint8(0);                               // name known
#else 
    if (!nameData)
        return; //simply ignore request
            // guess size

    data << guid;
#endif

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
    ObjectGuid guid;
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
    data << uint32(GameTime::GetGameTime());
    data << uint32(sWorld->GetNextDailyQuestsResetTime() - GameTime::GetGameTime());
    SendPacket(&data);
}

/// Only _static_ data send in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPackets::Query::QueryCreature& query)
{
    if (CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(query.CreatureID))
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name.c_str(), query.CreatureID);
        if (sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
            SendPacket(&ci->QueryData[static_cast<uint32>(GetSessionDbLocaleIndex())]);
        else
        {
            WorldPacket response = ci->BuildQueryData(GetSessionDbLocaleIndex());
            SendPacket(&response);
        }
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_CREATURE_QUERY - NO CREATURE INFO! (ENTRY: %u)",
            query.CreatureID);

        WorldPackets::Query::QueryCreatureResponse response;
        response.CreatureID = query.CreatureID;
        SendPacket(response.Write());
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
}

/// Only _static_ data send in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPackets::Query::QueryGameObject& query)
{
    if (GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(query.GameObjectID))
    {
        if (sWorld->getBoolConfig(CONFIG_CACHE_DATA_QUERIES))
            SendPacket(&info->QueryData[static_cast<uint32>(GetSessionDbLocaleIndex())]);
        else
        {
            WorldPacket response = info->BuildQueryData(GetSessionDbLocaleIndex());
            SendPacket(&response);
        }
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: CMSG_GAMEOBJECT_QUERY - Missing gameobject info for (ENTRY: %u)",
            query.GameObjectID);

        WorldPackets::Query::QueryGameObjectResponse response;
        response.GameObjectID = query.GameObjectID;
        SendPacket(response.Write());
        TC_LOG_DEBUG("network", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket & /*recvData*/)
{
    uint8 found = uint8(_player->HasCorpse());

    WorldPacket data(MSG_CORPSE_QUERY, (1+found*(5*4)));
    data << uint8(found);
    if(found)
    {
        WorldLocation corpseLocation = _player->GetCorpseLocation();
        uint32 corpseMapID = corpseLocation.GetMapId();
        uint32 mapID = corpseLocation.GetMapId();
        float x = corpseLocation.GetPositionX();
        float y = corpseLocation.GetPositionY();
        float z = corpseLocation.GetPositionZ();

        // if corpse at different map
        if (mapID != _player->GetMapId())
        {
            // search entrance map for proper show entrance
            if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapID))
            {
                if (corpseMapEntry->IsDungeon() && corpseMapEntry->entrance_map >= 0)
                {
                    // if corpse map have entrance
                    if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->entrance_map))
                    {
                        mapID = corpseMapEntry->entrance_map;
                        x = corpseMapEntry->entrance_x;
                        y = corpseMapEntry->entrance_y;
                        z = entranceMap->GetHeight(GetPlayer()->GetPhaseMask(), x, y, MAX_HEIGHT);
                    }
                }
            }
        }

        data << int32(mapID);
        data << float(x);
        data << float(y);
        data << float(z);
        data << int32(corpseMapID);
#ifdef LICH_KING
        data << uint32(0);  //unk
#endif
    }
    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode( WorldPacket & recvData )
{
    uint32 textID;
    ObjectGuid guid;

    recvData >> textID;
    //TC_LOG_DEBUG("network", "WORLD: CMSG_NPC_TEXT_QUERY TextId: %u", textID);

    recvData >> guid;

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

