#include "RecupMgr.h"
#include "Chat.h"
#include "Language.h"

bool RecupMgr::HandleRecupParseCommand(Player *player, std::string command, uint32 metier_level, bool equip, ChatHandler* chatHandler)
{
	std::string tempstr = command;
    std::vector<std::string> v, vline;
    std::vector<std::string>::iterator i;

    int cutAt;
    while ((cutAt = tempstr.find_first_of(";")) != tempstr.npos) {
        if (cutAt > 0) {
            vline.push_back(tempstr.substr(0, cutAt));
        }
        tempstr = tempstr.substr(cutAt + 1);
    }

    if (tempstr.length() > 0) {
        vline.push_back(tempstr);
    }

    for (i = vline.begin(); i != vline.end(); i++) {
        v.clear();
        tempstr = *i;
        while ((cutAt = tempstr.find_first_of(" ")) != tempstr.npos) {
            if (cutAt > 0) {
                v.push_back(tempstr.substr(0, cutAt));
            }
            tempstr = tempstr.substr(cutAt + 1);
        }

        if (tempstr.length() > 0) {
            v.push_back(tempstr);
        }

        if (v[0] == "additemset") {
            /* additemset, v[1] == set ID */
            if(v.size() < 2) 
            {
                if(chatHandler) chatHandler->SendSysMessage("Command error.");
                continue;
            }
            uint32 itemsetId = atoi(v[1].c_str());
            bool error = false;

            if (itemsetId == 0) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            QueryResult result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE itemset = %u", itemsetId);

            if (!result) {
                if(chatHandler) chatHandler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            do {
                Field *fields = result->Fetch();
                uint32 itemId = fields[0].GetUInt32();

                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                if (msg == EQUIP_ERR_OK) {
                    Item *item = player->StoreNewItem(dest, itemId, true);
                    player->SendNewItem(item, 1, true, true);
                } else {
                    player->SendEquipError(msg, NULL, NULL);
                    if (chatHandler) chatHandler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, 1);
                }
            } while(result->NextRow() || error);

        } else if (v[0] == "additem") {
            /* additem, v[1] == item ID, v[2] == item count */
            if(v.size() < 3) 
            {
                if (chatHandler) chatHandler->SendSysMessage("Command error.");
                continue;
            }

            uint32 itemId = atol(v[1].c_str());
            uint32 count = atoi(v[2].c_str());

            ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemId);
            if (!pProto) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            uint32 noSpaceForCount = 0;

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
            if (msg != EQUIP_ERR_OK)
                count -= noSpaceForCount;

            if (count == 0 || dest.empty()) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            if (equip)
            {
                player->StoreNewItemInBestSlots(itemId, count);
            }
            else
            {
                Item *item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

                if (count > 0 && item)
                    player->SendNewItem(item, count, true, true);
            }

            if (noSpaceForCount > 0) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                return false;
            }
        } else if (v[0] == "learn") {
            /* learn, v[1] == spell ID */
            if(v.size() < 2) 
            {
                if (chatHandler) chatHandler->SendSysMessage("Command error.");
                continue;
            }
            uint32 spell = atol(v[1].c_str());
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player)) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            if (!player->HasSpell(spell))
                player->LearnSpell(spell);
        } else if (v[0] == "money") {
            /* money, v[1] == money count, in pc */
            uint32 money = atoi(v[1].c_str());
            uint32 current_money = player->GetMoney();

            if (money > 0 && (current_money + money) < MAX_MONEY_AMOUNT) {
                player->ModifyMoney(money);
                if (chatHandler) chatHandler->PSendSysMessage(LANG_YOU_GET_MONEY);
            }
        } else if (v[0] == "setskill") {
            /* skill, v[1] == skill ID */
            if(v.size() < 2) 
            {
                if (chatHandler) chatHandler->SendSysMessage("Command error.");
                continue;
            }
            int32 skill = atoi(v[1].c_str());
            if (skill <= 0) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            int32 maxskill = ((int)(metier_level/75)+1)*75;
            if (maxskill > 375)
                maxskill = 375;

            SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
            if (!sl) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            if (!player->GetSkillValue(skill)) {
                if (chatHandler) chatHandler->PSendSysMessage(LANG_SET_SKILL_ERROR, player->GetName().c_str(), skill, sl->name[0]);
                if (chatHandler) chatHandler->SetSentErrorMessage(true);
                return false;
            }

            player->SetSkill(skill, metier_level ? metier_level : 1, maxskill);
            if (chatHandler) chatHandler->PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], player->GetName().c_str(), metier_level, maxskill);
        }
    }

    return true;
}

bool RecupMgr::Recup(Player* player, RecupEquipmentType type, RecupStuffLevel level)
{
    auto query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE classe = %u AND (faction = %u OR faction = 0) AND stufflevel = %u AND phase = 2 AND (stuff = %u OR stuff = -1)", player->GetClass(), 0, uint32(level), uint32(type));
    if (!query)
    {
        TC_LOG_ERROR("misc", "RecupMgr::Recup: could not get data from recups_data");
        return false;
    }

    auto fields = query->Fetch();
    std::string command = fields[0].GetString();

    if (!RecupMgr::HandleRecupParseCommand(player, command, 0, true, nullptr))
    {
        TC_LOG_ERROR("misc", "RecupMgr::Recup: Could not parse recup command");
        return false;
    }
    return true;
}