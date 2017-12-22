#include "Chat.h"
#include "Language.h"

bool ChatHandler::HandleListCreatureCommand(const char* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hcreature_entry");
    if(!cId)
        return false;

    uint32 cr_id = atol(cId);
    if(!cr_id)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cr_id);
    if(!cInfo)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(nullptr, " ");
    int count = c_count ? atol(c_count) : 10;

    if(count < 0)
        return false;

    QueryResult result;

    uint32 cr_count = 0;
    result=WorldDatabase.PQuery("SELECT COUNT(guid) FROM creature WHERE id='%u'",cr_id);
    if(result)
    {
        cr_count = (*result)[0].GetUInt64();
    }

    if(m_session)
    {
        Player* pl = m_session->GetPlayer();
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM creature WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), cr_id,uint32(count));
    }
    else
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM creature WHERE id = '%u' LIMIT %u",
            cr_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            int mapid = fields[4].GetUInt16();

            if  (m_session)
                PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, cInfo->Name.c_str(), x, y, z, mapid);
            else
                PSendSysMessage(LANG_CREATURE_LIST_CONSOLE, guid, cInfo->Name.c_str(), x, y, z, mapid);
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_LISTCREATUREMESSAGE,cr_id,cr_count);
    return true;
}

bool ChatHandler::HandleListItemCommand(const char* args)
{
    ARGS_CHECK

    char* cId = extractKeyFromLink((char*)args,"Hitem");
    if(!cId)
        return false;

    uint32 item_id = atol(cId);
    if(!item_id)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(item_id);
    if(!itemProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(nullptr, " ");
    int count = c_count ? atol(c_count) : 10;

    if(count < 0)
        return false;

    QueryResult result;

    // inventory case
    uint32 inv_count = 0;
    result = CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM character_inventory WHERE item_template='%u'", item_id);
    if(result)
    {
        inv_count = (*result)[0].GetUInt64();
    }

    result=CharacterDatabase.PQuery(
    //          0        1             2             3        4                  5
        "SELECT ci.item, cibag.slot AS bag, ci.slot, ci.guid, characters.account,characters.name "
        "FROM character_inventory AS ci LEFT JOIN character_inventory AS cibag ON (cibag.item=ci.bag),characters "
        "WHERE ci.item_template='%u' AND ci.guid = characters.guid LIMIT %u ",
        item_id, uint32(count));

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 item_bag = fields[1].GetUInt32();
            uint32 item_slot = fields[2].GetUInt32();
            uint32 owner_guid = fields[3].GetUInt32();
            uint32 owner_acc = fields[4].GetUInt32();
            std::string owner_name = fields[5].GetString();

            char const* item_pos = nullptr;
            if(Player::IsEquipmentPos(item_bag,item_slot))
                item_pos = "[equipped]";
            else if(Player::IsInventoryPos(item_bag,item_slot))
                item_pos = "[in inventory]";
            else if(Player::IsBankPos(item_bag,item_slot))
                item_pos = "[in bank]";
            else
                item_pos = "";

            PSendSysMessage(LANG_ITEMLIST_SLOT,
                item_guid,owner_name.c_str(),owner_guid,owner_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if(count > res_count)
            count-=res_count;
        else if(count)
            count = 0;
    }

    // mail case
    uint32 mail_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM mail_items WHERE item_template='%u'", item_id);
    if(result)
    {
        mail_count = (*result)[0].GetUInt32();
    }

    if(count > 0)
    {
        result=CharacterDatabase.PQuery(
        //          0                     1            2              3               4            5               6
            "SELECT mail_items.item_guid, mail.sender, mail.receiver, char_s.account, char_s.name, char_r.account, char_r.name "
            "FROM mail,mail_items,characters as char_s,characters as char_r "
            "WHERE mail_items.item_template='%u' AND char_s.guid = mail.sender AND char_r.guid = mail.receiver AND mail.id=mail_items.mail_id LIMIT %u",
            item_id,uint32(count));
    }
    else
        result = nullptr;

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid        = fields[0].GetUInt32();
            uint32 item_s           = fields[1].GetUInt32();
            uint32 item_r           = fields[2].GetUInt32();
            uint32 item_s_acc       = fields[3].GetUInt32();
            std::string item_s_name = fields[4].GetString();
            uint32 item_r_acc       = fields[5].GetUInt32();
            std::string item_r_name = fields[6].GetString();

            char const* item_pos = "[in mail]";

            PSendSysMessage(LANG_ITEMLIST_MAIL,
                item_guid,item_s_name.c_str(),item_s,item_s_acc,item_r_name.c_str(),item_r,item_r_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if(count > res_count)
            count-=res_count;
        else if(count)
            count = 0;
    }

    // auction case
    uint32 auc_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM auctionhouse WHERE item_template='%u'",item_id);
    if(result)
    {
        auc_count = (*result)[0].GetUInt32();
    }

    if(count > 0)
    {
        result=CharacterDatabase.PQuery(
        //           0                      1                       2                   3
            "SELECT  auctionhouse.itemguid, auctionhouse.itemowner, characters.account, characters.name "
            "FROM auctionhouse,characters WHERE auctionhouse.item_template='%u' AND characters.guid = auctionhouse.itemowner LIMIT %u",
            item_id,uint32(count));
    }
    else
        result = nullptr;

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid       = fields[0].GetUInt32();
            uint32 owner           = fields[1].GetUInt32();
            uint32 owner_acc       = fields[2].GetUInt32();
            std::string owner_name = fields[3].GetString();

            char const* item_pos = "[in auction]";

            PSendSysMessage(LANG_ITEMLIST_AUCTION, item_guid, owner_name.c_str(), owner, owner_acc,item_pos);
        } while (result->NextRow());
    }

    // guild bank case
    uint32 guild_count = 0;
    result=CharacterDatabase.PQuery("SELECT COUNT(item_entry) FROM guild_bank_item WHERE item_entry='%u'",item_id);
    if(result)
    {
        guild_count = (*result)[0].GetUInt32();
    }

    result=CharacterDatabase.PQuery(
        //      0             1           2
        "SELECT gi.item_guid, gi.guildid, guild.name "
        "FROM guild_bank_item AS gi, guild WHERE gi.item_entry='%u' AND gi.guildid = guild.guildid LIMIT %u ",
        item_id,uint32(count));

    if(result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 guild_guid = fields[1].GetUInt32();
            std::string guild_name = fields[2].GetString();

            char const* item_pos = "[in guild bank]";

            PSendSysMessage(LANG_ITEMLIST_GUILD,item_guid,guild_name.c_str(),guild_guid,item_pos);
        } while (result->NextRow());

        //int64 res_count = result->GetRowCount();
    }

    if(inv_count+mail_count+auc_count+guild_count == 0)
    {
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_COMMAND_LISTITEMMESSAGE,item_id,inv_count+mail_count+auc_count+guild_count,inv_count,mail_count,auc_count,guild_count);

    return true;
}

bool ChatHandler::HandleListObjectCommand(const char* args)
{
    ARGS_CHECK

    // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject_entry");
    if(!cId)
        return false;

    uint32 go_id = atol(cId);
    if(!go_id)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    GameObjectTemplate const * gInfo = sObjectMgr->GetGameObjectTemplate(go_id);
    if(!gInfo)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(nullptr, " ");
    int count = c_count ? atol(c_count) : 10;

    if(count < 0)
        return false;

    QueryResult result;

    uint32 obj_count = 0;
    result=WorldDatabase.PQuery("SELECT COUNT(guid) FROM gameobject WHERE id='%u'",go_id);
    if(result)
    {
        obj_count = (*result)[0].GetUInt64();
    }

    if(m_session)
    {
        Player* pl = m_session->GetPlayer();
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, id, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),go_id,uint32(count));
    }
    else
        result = WorldDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM gameobject WHERE id = '%u' LIMIT %u",
            go_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            int mapid = fields[4].GetUInt16();
            uint32 id = fields[5].GetUInt32();

            if (m_session)
                PSendSysMessage(LANG_GO_LIST_CHAT, guid, id, guid, gInfo->name.c_str(), x, y, z, mapid);
            else
                PSendSysMessage(LANG_GO_LIST_CONSOLE, guid, gInfo->name.c_str(), x, y, z, mapid);
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_LISTOBJMESSAGE,go_id,obj_count);
    return true;
}

bool ChatHandler::HandleListAurasCommand (const char * /*args*/)
{
    Unit *unit = GetSelectedUnit();
    if(!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    char const* talentStr = GetTrinityString(LANG_TALENT);
    char const* passiveStr = GetTrinityString(LANG_PASSIVE);

    Unit::AuraMap const& uAuras = unit->GetAuras();
    PSendSysMessage(LANG_COMMAND_TARGET_LISTAURAS, uAuras.size());
    for (auto itr = uAuras.begin(); itr != uAuras.end(); ++itr)
    {
        bool talent = GetTalentSpellCost(itr->second->GetId()) > 0;
        PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, itr->second->GetId(), itr->second->GetEffIndex(),
            itr->second->GetModifier()->m_auraname, itr->second->GetDuration(), itr->second->GetMaxDuration(),
            itr->second->GetSpellInfo()->SpellName[GetSessionDbcLocale()],
            (itr->second->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
            IS_PLAYER_GUID(itr->second->GetCasterGUID()) ? "player" : "creature",GUID_LOPART(itr->second->GetCasterGUID()));
    }
    for (int i = 0; i < TOTAL_AURAS; i++)
    {
        Unit::AuraList const& uAuraList = unit->GetAurasByType(AuraType(i));
        if (uAuraList.empty()) continue;
        PSendSysMessage(LANG_COMMAND_TARGET_LISTAURATYPE, uAuraList.size(), i);
        for (auto itr : uAuraList)
        {
            bool talent = GetTalentSpellCost(itr->GetId()) > 0;
            PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, itr->GetId(), itr->GetEffIndex(),
                itr->GetSpellInfo()->SpellName[GetSessionDbcLocale()],(itr->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
                IS_PLAYER_GUID(itr->GetCasterGUID()) ? "player" : "creature",GUID_LOPART(itr->GetCasterGUID()));
        }
    }
    return true;
}
