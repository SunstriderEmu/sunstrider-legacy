#include "RecupMgr.h"
#include "Chat.h"
#include "Language.h"

bool RecupMgr::HandleRecupParseCommand(Player* player, std::string const command, uint32 profession_level, bool equip, ChatHandler* chatHandler)
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

            if (itemsetId == 0) 
            {
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                    chatHandler->SetSentErrorMessage(true);
                }
                return false;
            }

            QueryResult result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE itemset = %u", itemsetId);

            if (!result) {
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                    chatHandler->SetSentErrorMessage(true);
                }
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
                    player->SendEquipError(msg, nullptr, nullptr);
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
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
                    chatHandler->SetSentErrorMessage(true);
                }
                return false;
            }

            uint32 noSpaceForCount = 0;

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
            if (msg != EQUIP_ERR_OK)
                count -= noSpaceForCount;

            if (count == 0 || dest.empty()) 
            {
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                    chatHandler->SetSentErrorMessage(true);
                }
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

            if (noSpaceForCount > 0) 
            {
                if (chatHandler) 
                    chatHandler->PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                return false;
            }
        } else if (v[0] == "learn") {
            /* learn, v[1] == spell ID */
            if(v.size() < 2) 
            {
                if (chatHandler) 
                    chatHandler->SendSysMessage("Command error.");
                continue;
            }
            uint32 spell = atol(v[1].c_str());
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player)) {
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
                    chatHandler->SetSentErrorMessage(true);
                }
                return false;
            }

            if (!player->HasSpell(spell))
                player->LearnSpell(spell, false);
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
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
                    chatHandler->SetSentErrorMessage(true);
                }
                return false;
            }

            int32 maxskill = ((int)(profession_level /75)+1)*75;
            if (maxskill > 375)
                maxskill = 375;

            SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
            if (!sl) {
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
                    chatHandler->SetSentErrorMessage(true);
                }
                return false;
            }

            if (!player->GetSkillValue(skill)) {
                if (chatHandler)
                {
                    chatHandler->PSendSysMessage(LANG_SET_SKILL_ERROR, player->GetName().c_str(), skill, sl->name[0]);
                    chatHandler->SetSentErrorMessage(true);
                }
                return false;
            }

            //from 0 to 4
            uint16 step = profession_level ? ((profession_level - 1) / 75) : 1;
            player->SetSkill(skill, step, profession_level ? profession_level : 1, maxskill);
            if (chatHandler) 
                chatHandler->PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], player->GetName().c_str(), profession_level, maxskill);
        }
    }

    return true;
}

bool RecupMgr::Recup(Player* player, RecupEquipmentType type, RecupStuffTier tier)
{
    auto query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE classe = %u AND (faction = %u OR faction = 0) AND tier = %u AND phase = 2 AND (stuff = %u OR stuff = -1)", player->GetClass(), 0, uint32(tier), uint32(type));
    if (!query)
    {
        TC_LOG_ERROR("misc", "RecupMgr::Recup: could not get data from recups_data");
        return false;
    }

    player->LearnAllClassProficiencies();
    player->UpdateSkillsToMaxSkillsForLevel();

    auto fields = query->Fetch();
    std::string command = fields[0].GetString();

    if (!RecupMgr::HandleRecupParseCommand(player, command, 0, true, nullptr))
    {
        TC_LOG_ERROR("misc", "RecupMgr::Recup: Could not parse recup command");
        return false;
    }
    return true;
}

bool RecupMgr::RecupProfession(Player* player, RecupProfessionType profession, uint32 maxSkill /*= 375*/)
{
    if (maxSkill > 375)
        return false;

    switch (profession)
    {
    case RECUP_PROFESSION_FIRST_AID:
        player->SetSkill(129, 4, maxSkill, maxSkill); //first aid
        player->LearnSpell(27028, false); //first aid spell
        player->LearnSpell(27033, false); //bandage
        if(maxSkill >= 300)
            player->StoreNewItemInBestSlots(21990, 20); // netherweave bandages
        break;
    case RECUP_PROFESSION_COOKING:
        player->SetSkill(185, 4, maxSkill, maxSkill);
        player->LearnSpell(33359, false);
        break;
    case RECUP_PROFESSION_FISHING:
        player->SetSkill(356, 4, maxSkill, maxSkill);
        player->LearnSpell(33095, false);
        player->StoreNewItemInBestSlots(6256, 1); // Fishing pole
        break;
    case RECUP_PROFESSION_ENCHANTMENT:
        player->SetSkill(333, 4, maxSkill, maxSkill);
        player->LearnSpell(28029, false);
        if (maxSkill >= 300)
            player->StoreNewItemInBestSlots(16207, 1); // Runed arcanite rod
        break;
    case RECUP_PROFESSION_MINING:
        player->SetSkill(182, 4, maxSkill, maxSkill);
        player->LearnSpell(29354, false);
        player->StoreNewItemInBestSlots(2901, 1); //mining pick
        break;
    case RECUP_PROFESSION_FORGE:
        player->SetSkill(164, 4, maxSkill, maxSkill);
        player->LearnSpell(29844, false);
        break;
    case RECUP_PROFESSION_ENGINEERING:
        player->SetSkill(202, 4, maxSkill, maxSkill);
        player->LearnSpell(30350, false);
        break;
    case RECUP_PROFESSION_LEATHERWORKING:
        player->SetSkill(165, 4, maxSkill, maxSkill);
        player->LearnSpell(32549, false);
        break;
    case RECUP_PROFESSION_SKINNING:
        player->SetSkill(393, 4, maxSkill, maxSkill);
        player->LearnSpell(32678, false);
        break;
    case RECUP_PROFESSION_ALCHEMY:
        player->SetSkill(171, 4, maxSkill, maxSkill);
        player->LearnSpell(28596, false);
        break;
    case RECUP_PROFESSION_TAILORING:
        player->SetSkill(197, 4, maxSkill, maxSkill);
        player->LearnSpell(26790, false);
        break;
    case RECUP_PROFESSION_JEWELCRAFTING:
        player->SetSkill(755, 4, maxSkill, maxSkill);
        player->LearnSpell(28897, false);
        break;
    case RECUP_PROFESSION_HERBALISM:
        player->SetSkill(182, 4, maxSkill, maxSkill);
        player->LearnSpell(28695, false);
        break;
    case RECUP_PROFESSION_LOCKPICKING:
        player->SetSkill(633, 2, 225, 225);
        player->LearnSpell(6463, false);
        break;
    default:
        return false;
    }
    return true;
}