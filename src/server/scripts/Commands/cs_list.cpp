#include "Chat.h"
#include "Language.h"
#include "SpellAuraEffects.h"
#include "GameTime.h"

class list_commandscript : public CommandScript
{
public:
    list_commandscript() : CommandScript("list_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> listCommandTable =
        {
            { "creature",       SEC_GAMEMASTER3,  true,  &HandleListCreatureCommand,        "" },
            { "item",           SEC_GAMEMASTER3,  true,  &HandleListItemCommand,            "" },
            { "object",         SEC_GAMEMASTER3,  true,  &HandleListObjectCommand,          "" },
            { "auras",          SEC_GAMEMASTER3,  false, &HandleListAurasCommand,           "" },
            { "spawnpoints",    SEC_GAMEMASTER3,  false, &HandleListSpawnPointsCommand,     "" },
            { "respawns",       SEC_GAMEMASTER3,  false, &HandleListRespawnsCommand,        "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "list",           SEC_GAMEMASTER3,  true,  nullptr,                        "", listCommandTable },
        };
        return commandTable;
    }

    static bool HandleListCreatureCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!cId)
            return false;

        uint32 cr_id = atol(cId);
        if (!cr_id)
        {
            handler->PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cr_id);
        if (!cInfo)
        {
            handler->PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* c_count = strtok(nullptr, " ");
        int count = c_count ? atol(c_count) : 10;

        if (count < 0)
            return false;

        uint32 cr_count = 0;
        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_COUNT_BY_ID);
        stmt->setUInt32(0, cr_id);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        if (result)
        {
            cr_count = (*result)[0].GetUInt64();
        }

        stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_POS_BY_ID);
        stmt->setUInt32(0, cr_id);
        result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt32();
                float x = fields[1].GetFloat();
                float y = fields[2].GetFloat();
                float z = fields[3].GetFloat();
                int mapid = fields[4].GetUInt16();

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, cInfo->Name.c_str(), x, y, z, mapid);
                else
                    handler->PSendSysMessage(LANG_CREATURE_LIST_CONSOLE, guid, cInfo->Name.c_str(), x, y, z, mapid);
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_LISTCREATUREMESSAGE, cr_id, cr_count);
        return true;
    }

    static bool HandleListItemCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* cId = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!cId)
            return false;

        uint32 item_id = atol(cId);
        if (!item_id)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(item_id);
        if (!itemProto)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* c_count = strtok(nullptr, " ");
        int count = c_count ? atol(c_count) : 10;

        if (count < 0)
            return false;

        QueryResult result;

        // inventory case
        uint32 inv_count = 0;
        result = CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM character_inventory WHERE item_template='%u'", item_id);
        if (result)
        {
            inv_count = (*result)[0].GetUInt64();
        }

        result = CharacterDatabase.PQuery(
            //          0        1             2             3        4                  5
            "SELECT ci.item, cibag.slot AS bag, ci.slot, ci.guid, characters.account,characters.name "
            "FROM character_inventory AS ci LEFT JOIN character_inventory AS cibag ON (cibag.item=ci.bag),characters "
            "WHERE ci.item_template='%u' AND ci.guid = characters.guid LIMIT %u ",
            item_id, uint32(count));

        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                ObjectGuid::LowType item_guid = fields[0].GetUInt32();
                uint32 item_bag = fields[1].GetUInt32();
                uint32 item_slot = fields[2].GetUInt32();
                ObjectGuid::LowType owner_guid = fields[3].GetUInt32();
                uint32 owner_acc = fields[4].GetUInt32();
                std::string owner_name = fields[5].GetString();

                char const* item_pos = nullptr;
                if (Player::IsEquipmentPos(item_bag, item_slot))
                    item_pos = "[equipped]";
                else if (Player::IsInventoryPos(item_bag, item_slot))
                    item_pos = "[in inventory]";
                else if (Player::IsBankPos(item_bag, item_slot))
                    item_pos = "[in bank]";
                else
                    item_pos = "";

                handler->PSendSysMessage(LANG_ITEMLIST_SLOT,
                    item_guid, owner_name.c_str(), owner_guid, owner_acc, item_pos);
            } while (result->NextRow());

            int64 res_count = result->GetRowCount();

            if (count > res_count)
                count -= res_count;
            else if (count)
                count = 0;
        }

        // mail case
        uint32 mail_count = 0;

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_MAIL_COUNT_ITEM);
        stmt->setUInt32(0, item_id);
        PreparedQueryResult result2 = CharacterDatabase.Query(stmt);

        if (result2)
            mail_count = (*result2)[0].GetUInt64();
        if (count > 0)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_MAIL_ITEMS_BY_ENTRY);
            stmt->setUInt32(0, item_id);
            stmt->setUInt32(1, count);
            result2 = CharacterDatabase.Query(stmt);
        }
        else
            result2 = PreparedQueryResult(nullptr);

        if (result2)
        {
            do
            {
                Field* fields = result2->Fetch();
                ObjectGuid::LowType itemGuid = fields[0].GetUInt32();
                ObjectGuid::LowType itemSender = fields[1].GetUInt32();
                uint32 itemReceiver = fields[2].GetUInt32();
                uint32 itemSenderAccountId = fields[3].GetUInt32();
                std::string itemSenderName = fields[4].GetString();
                uint32 itemReceiverAccount = fields[5].GetUInt32();
                std::string itemReceiverName = fields[6].GetString();

                char const* itemPos = "[in mail]";

                handler->PSendSysMessage(LANG_ITEMLIST_MAIL, itemGuid, itemSenderName.c_str(), itemSender, itemSenderAccountId, itemReceiverName.c_str(), itemReceiver, itemReceiverAccount, itemPos);
            } while (result2->NextRow());

            uint32 resultCount = uint32(result2->GetRowCount());

            if (count > resultCount)
                count -= resultCount;
            else if (count)
                count = 0;
        }

        // auction case
        uint32 auc_count = 0;
        result = CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM auctionhouse WHERE item_template='%u'", item_id);
        if (result)
        {
            auc_count = (*result)[0].GetUInt32();
        }

        if (count > 0)
        {
            result = CharacterDatabase.PQuery(
                //           0                      1                       2                   3
                "SELECT  auctionhouse.itemguid, auctionhouse.itemowner, characters.account, characters.name "
                "FROM auctionhouse,characters WHERE auctionhouse.item_template='%u' AND characters.guid = auctionhouse.itemowner LIMIT %u",
                item_id, uint32(count));
        }
        else
            result = nullptr;

        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                ObjectGuid::LowType item_guid = fields[0].GetUInt32();
                uint32 owner = fields[1].GetUInt32();
                uint32 owner_acc = fields[2].GetUInt32();
                std::string owner_name = fields[3].GetString();

                char const* item_pos = "[in auction]";

                handler->PSendSysMessage(LANG_ITEMLIST_AUCTION, item_guid, owner_name.c_str(), owner, owner_acc, item_pos);
            } while (result->NextRow());
        }


        // guild bank case
        uint32 guildCount = 0;

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_BANK_COUNT_ITEM);
        stmt->setUInt32(0, item_id);
        result2 = CharacterDatabase.Query(stmt);

        if (result2)
            guildCount = (*result2)[0].GetUInt64();

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_BANK_ITEM_BY_ENTRY);
        stmt->setUInt32(0, item_id);
        stmt->setUInt32(1, count);
        result2 = CharacterDatabase.Query(stmt);

        if (result2)
        {
            do
            {
                Field* fields = result2->Fetch();
                uint32 itemGuid = fields[0].GetUInt32();
                uint32 guildGuid = fields[1].GetUInt32();
                std::string guildName = fields[2].GetString();

                char const* itemPos = "[in guild bank]";

                handler->PSendSysMessage(LANG_ITEMLIST_GUILD, itemGuid, guildName.c_str(), guildGuid, itemPos);
            } while (result2->NextRow());

            uint32 resultCount = uint32(result2->GetRowCount());

            if (count > resultCount)
                count -= resultCount;
            else if (count)
                count = 0;
        }

        if (inv_count + mail_count + auc_count + guildCount == 0)
        {
            handler->SendSysMessage(LANG_COMMAND_NOITEMFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_COMMAND_LISTITEMMESSAGE, item_id, inv_count + mail_count + auc_count + guildCount, inv_count, mail_count, auc_count, guildCount);

        return true;
    }

    static bool HandleListObjectCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        if (!cId)
            return false;

        uint32 go_id = atol(cId);
        if (!go_id)
        {
            handler->PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        GameObjectTemplate const * gInfo = sObjectMgr->GetGameObjectTemplate(go_id);
        if (!gInfo)
        {
            handler->PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* c_count = strtok(nullptr, " ");
        int count = c_count ? atol(c_count) : 10;

        if (count < 0)
            return false;

        QueryResult result;

        uint32 obj_count = 0;
        result = WorldDatabase.PQuery("SELECT COUNT(guid) FROM gameobject WHERE id='%u'", go_id);
        if (result)
        {
            obj_count = (*result)[0].GetUInt64();
        }

        if (handler->GetSession())
        {
            Player* pl = handler->GetSession()->GetPlayer();
            result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, id, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), go_id, uint32(count));
        }
        else
            result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM gameobject WHERE id = '%u' LIMIT %u",
                go_id, uint32(count));

        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt32();
                float x = fields[1].GetFloat();
                float y = fields[2].GetFloat();
                float z = fields[3].GetFloat();
                int mapid = fields[4].GetUInt16();
                uint32 id = fields[5].GetUInt32();

                if (handler->GetSession())
                    handler->PSendSysMessage(LANG_GO_LIST_CHAT, guid, id, guid, gInfo->name.c_str(), x, y, z, mapid);
                else
                    handler->PSendSysMessage(LANG_GO_LIST_CONSOLE, guid, gInfo->name.c_str(), x, y, z, mapid);
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_LISTOBJMESSAGE, go_id, obj_count);
        return true;
    }

    static bool HandleListAurasCommand(ChatHandler* handler, char const* args)
    {
        Unit* unit = handler->GetSelectedUnit();
        if (!unit)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char const* talentStr = handler->GetTrinityString(LANG_TALENT);
        char const* passiveStr = handler->GetTrinityString(LANG_PASSIVE);

        Unit::AuraApplicationMap const& auras = unit->GetAppliedAuras();
        handler->PSendSysMessage(LANG_COMMAND_TARGET_LISTAURAS, auras.size());
        for (Unit::AuraApplicationMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        {
            bool talent = GetTalentSpellCost(itr->second->GetBase()->GetId()) > 0;

            AuraApplication const* aurApp = itr->second;
            Aura const* aura = aurApp->GetBase();
            char const* name = aura->GetSpellInfo()->SpellName[handler->GetSessionDbcLocale()];

            std::ostringstream ss_name;
            //       ss_name << "|cffffffff|Hspell:" << aura->GetId() << "|h[" << name << "]|h|r"; //message does not appear, what's wrong? link format is the same on LK and BC
            ss_name << name;

            handler->PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, aura->GetId(), (handler->GetSession() ? ss_name.str().c_str() : name),
                aurApp->GetEffectMask(), aura->GetCharges(), aura->GetStackAmount(), aurApp->GetSlot(),
                aura->GetDuration(), aura->GetMaxDuration(), (aura->IsPassive() ? passiveStr : ""),
                (talent ? talentStr : ""), aura->GetCasterGUID().IsPlayer() ? "player" : "creature",
                aura->GetCasterGUID().GetCounter());

            //not working... why?
            /*std::ostringstream test;
            test << "|cffffffff|Hspell:" << aura->GetId() << "|h[" << name << "]|h|r";
            handler->SendSysMessage(test.str().c_str());*/
        }

        for (uint16 i = 0; i < TOTAL_AURAS; ++i)
        {
            Unit::AuraEffectList const& auraList = unit->GetAuraEffectsByType(AuraType(i));
            if (auraList.empty())
                continue;

            handler->PSendSysMessage(LANG_COMMAND_TARGET_LISTAURATYPE, auraList.size(), i);

            for (Unit::AuraEffectList::const_iterator itr = auraList.begin(); itr != auraList.end(); ++itr)
                handler->PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, (*itr)->GetId(), (*itr)->GetEffIndex(), (*itr)->GetAmount());
        }

        return true;
    }

    static bool HandleListSpawnPointsCommand(ChatHandler* handler, char const* args)
    {
        Player const* player = handler->GetSession()->GetPlayer();
        Map const* map = player->GetMap();
        uint32 const mapId = map->GetId();
        bool const showAll = map->IsBattlegroundOrArena() || map->IsDungeon();
        handler->PSendSysMessage("Listing all spawn points in map %u (%s)%s:", mapId, map->GetMapName(), showAll ? "" : " within 5000yd");
        for (auto const& pair : sObjectMgr->GetAllCreatureData())
        {
            CreatureData const& data = pair.second;
            if (data.spawnPoint.GetMapId() != mapId)
                continue;
            CreatureTemplate const* cTemp = sObjectMgr->GetCreatureTemplate(data.GetFirstSpawnEntry());
            if (!cTemp)
                continue;
            if (showAll || data.spawnPoint.IsInDist2d(player, 5000.0))
                handler->PSendSysMessage("Type: %u | SpawnId: %u | Entry: %u (%s) | X: %.3f | Y: %.3f | Z: %.3f", uint32(data.type), data.spawnId, data.GetFirstSpawnEntry(), cTemp->Name.c_str(), data.spawnPoint.GetPositionX(), data.spawnPoint.GetPositionY(), data.spawnPoint.GetPositionZ());
        }
        for (auto const& pair : sObjectMgr->GetAllGameObjectData())
        {
            GameObjectData const& data = pair.second;
            if (data.spawnPoint.GetMapId() != mapId)
                continue;
            GameObjectTemplate const* goTemp = sObjectMgr->GetGameObjectTemplate(data.id);
            if (!goTemp)
                continue;
            if (showAll || data.spawnPoint.IsInDist2d(player, 5000.0))
                handler->PSendSysMessage("Type: %u | SpawnId: %u | Entry: %u (%s) | X: %.3f | Y: %.3f | Z: %.3f", uint32(data.type), data.spawnId, data.id, goTemp->name.c_str(), data.spawnPoint.GetPositionX(), data.spawnPoint.GetPositionY(), data.spawnPoint.GetPositionZ());
        }
        return true;
    }

    static char const* GetZoneName(uint32 zoneId, LocaleConstant locale)
    {
        AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(zoneId);
        return zoneEntry ? zoneEntry->area_name[locale] : "<unknown zone>";
    }
    static bool HandleListRespawnsCommand(ChatHandler* handler, char const* args)
    {
        Player const* player = handler->GetSession()->GetPlayer();
        Map const* map = player->GetMap();
        uint32 range = 0;
        if (*args)
            range = atoi((char*)args);

        std::vector<RespawnInfo*> respawns;
        //LocaleConstant locale = handler->GetSession()->handler->GetSessionDbcLocale();
        char const* stringOverdue = "overdue"; // sObjectMgr->handler->GetTrinityString(LANG_LIST_RESPAWNS_OVERDUE, locale);
        char const* stringCreature = "creatures"; //sObjectMgr->handler->GetTrinityString(LANG_LIST_RESPAWNS_CREATURES, locale);
        char const* stringGameobject = "gameobjects"; //sObjectMgr->handler->GetTrinityString(LANG_LIST_RESPAWNS_GAMEOBJECTS, locale);

        uint32 zoneId = player->GetZoneId();
        if (range)
            handler->PSendSysMessage(/*LANG_LIST_RESPAWNS_RANGE*/"Listing %s respawns within %uyd", stringCreature, range);
        else
            handler->PSendSysMessage(/*LANG_LIST_RESPAWNS_ZONE*/"Listing %s respawns for %s (zone %u)", stringCreature, GetZoneName(zoneId, handler->GetSessionDbcLocale()), zoneId);

        handler->PSendSysMessage(/*LANG_LIST_RESPAWNS_LISTHEADER*/"SpawnID | Entry | GridXY| Zone | Respawn time (Full)");
        map->GetRespawnInfo(respawns, SPAWN_TYPEMASK_CREATURE, range ? 0 : zoneId);
        for (RespawnInfo* ri : respawns)
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(ri->spawnId);
            if (!data)
                continue;
            if (range && !player->IsInDist(data->spawnPoint, range))
                continue;
            uint32 gridY = ri->gridId / MAX_NUMBER_OF_GRIDS;
            uint32 gridX = ri->gridId % MAX_NUMBER_OF_GRIDS;

            std::string respawnTime = ri->respawnTime > GameTime::GetGameTime() ? secsToTimeString(uint64(ri->respawnTime - GameTime::GetGameTime()), true) : stringOverdue;
            handler->PSendSysMessage("%u | %u | [%02u,%02u] | %s (%u) | %s", ri->spawnId, ri->entry, gridX, gridY, GetZoneName(ri->zoneId, handler->GetSessionDbcLocale()), ri->zoneId, map->IsSpawnGroupActive(data->spawnGroupData->groupId) ? respawnTime.c_str() : "inactive");
        }

        respawns.clear();
        if (range)
            handler->PSendSysMessage(/*LANG_LIST_RESPAWNS_RANGE*/"Listing %s respawns within %uyd", stringGameobject, range);
        else
            handler->PSendSysMessage(/*LANG_LIST_RESPAWNS_ZONE*/"Listing %s respawns for %s (zone %u)", stringGameobject, GetZoneName(zoneId, handler->GetSessionDbcLocale()), zoneId);

        handler->PSendSysMessage(/*LANG_LIST_RESPAWNS_LISTHEADER*/"SpawnID | Entry | GridXY| Zone | Respawn time (Full)");
        map->GetRespawnInfo(respawns, SPAWN_TYPEMASK_GAMEOBJECT, range ? 0 : zoneId);
        for (RespawnInfo* ri : respawns)
        {
            GameObjectData const* data = sObjectMgr->GetGameObjectData(ri->spawnId);
            if (!data)
                continue;
            if (range && !player->IsInDist(data->spawnPoint, range))
                continue;
            uint32 gridY = ri->gridId / MAX_NUMBER_OF_GRIDS;
            uint32 gridX = ri->gridId % MAX_NUMBER_OF_GRIDS;

            std::string respawnTime = ri->respawnTime > GameTime::GetGameTime() ? secsToTimeString(uint64(ri->respawnTime - GameTime::GetGameTime()), true) : stringOverdue;
            handler->PSendSysMessage("%u | %u | [% 02u, % 02u] | %s (%u) | %s", ri->spawnId, ri->entry, gridX, gridY, GetZoneName(ri->zoneId, handler->GetSessionDbcLocale()), ri->zoneId, map->IsSpawnGroupActive(data->spawnGroupData->groupId) ? respawnTime.c_str() : "inactive");
        }
        return true;
    }
};

void AddSC_list_commandscript()
{
    new list_commandscript();
}
