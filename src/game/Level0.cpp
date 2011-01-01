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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "AccountMgr.h"
#include "SystemConfig.h"
#include "revision.h"
#include "Util.h"

#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Config/ConfigEnv.h"

bool ChatHandler::HandleHelpCommand(const char* args)
{
    char* cmd = strtok((char*)args, " ");
    if(!cmd)
    {
        ShowHelpForCommand(getCommandTable(), "help");
        ShowHelpForCommand(getCommandTable(), "");
    }
    else
    {
        if(!ShowHelpForCommand(getCommandTable(), cmd))
            SendSysMessage(LANG_NO_HELP_CMD);
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* args)
{
    ShowHelpForCommand(getCommandTable(), "");
    return true;
}

bool ChatHandler::HandleAccountCommand(const char* /*args*/)
{
    uint32 gmlevel = m_session->GetSecurity();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, gmlevel);
    return true;
}

bool ChatHandler::HandleStartCommand(const char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    if(chr->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }
    
    if(chr->InBattleGround())
    {
        SendSysMessage(LANG_YOU_IN_BATTLEGROUND);
        SetSentErrorMessage(true);
        return false;
    }
    
    if (chr->HasAura(9454)) // Char is freezed by GM
    {
        SendSysMessage("Impossible lorsque vous êtes gelé.");
        SetSentErrorMessage(true);
        return false;
    }

    // cast spell Stuck
    //chr->CastSpell(chr,7355,false);
    if (chr->isAlive())
        chr->Kill(chr);
    chr->RepopAtGraveyard();
    return true;
}

bool ChatHandler::HandleServerInfoCommand(const char* /*args*/)
{
    uint32 activeClientsNum = sWorld.GetActiveSessionCount();
    uint32 queuedClientsNum = sWorld.GetQueuedSessionCount();
    uint32 maxActiveClientsNum = sWorld.GetMaxActiveSessionCount();
    uint32 maxQueuedClientsNum = sWorld.GetMaxQueuedSessionCount();
    std::string str = secsToTimeString(sWorld.GetUptime());
    uint32 updateTime = sWorld.GetUpdateTime();

    PSendSysMessage(_FULLVERSION);
    //if(m_session)
    //    full = _FULLVERSION(REVISION_DATE,REVISION_TIME,"|cffffffff|Hurl:" REVISION_ID "|h" REVISION_ID "|h|r");
    //else
    //    full = _FULLVERSION(REVISION_DATE,REVISION_TIME,REVISION_ID);

    //SendSysMessage(full);
    //PSendSysMessage(LANG_USING_SCRIPT_LIB,sWorld.GetScriptsVersion());
    //PSendSysMessage(LANG_USING_WORLD_DB,sWorld.GetDBVersion());
    //PSendSysMessage(LANG_CONNECTED_USERS, activeClientsNum, maxActiveClientsNum, queuedClientsNum, maxQueuedClientsNum);
    PSendSysMessage("Online Players: %u (Max: %u)", activeClientsNum, maxActiveClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());
    PSendSysMessage("Update time diff: %u.", updateTime);

    return true;
}

bool ChatHandler::HandleDismountCommand(const char* /*args*/)
{
    //If player is not mounted, so go out :)
    if (!m_session->GetPlayer( )->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(m_session->GetPlayer( )->isInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->Unmount();
    m_session->GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* /*args*/)
{
    Player *player=m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if(m_session->GetSecurity())
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
        return true;
    }

    // save or plan save after 20 sec (logout delay) if current next save time more this value and _not_ output any messages to prevent cheat planning
    uint32 save_interval = sWorld.getConfig(CONFIG_INTERVAL_SAVE);
    if(save_interval==0 || save_interval > 20*1000 && player->GetSaveTimer() <= save_interval - 20*1000)
        player->SaveToDB();

    return true;
}

bool ChatHandler::HandleGMListIngameCommand(const char* /*args*/)
{
    bool first = true;

    HashMapHolder<Player>::MapType &m = HashMapHolder<Player>::GetContainer();
    HashMapHolder<Player>::MapType::iterator itr = m.begin();
    for(; itr != m.end(); ++itr)
    {
        AccountTypes itr_sec = (AccountTypes)itr->second->GetSession()->GetSecurity();

        if ((itr->second->isGameMaster() || itr_sec > SEC_PLAYER && itr_sec <= sWorld.getConfig(CONFIG_GM_LEVEL_IN_GM_LIST)) &&
            (!m_session || itr->second->IsVisibleGloballyFor(m_session->GetPlayer())))
        {
            if(first)
            {
                SendSysMessage(LANG_GMS_ON_SRV);
                first = false;
            }

            SendSysMessage(itr->second->GetName());
        }
    }

    if(first)
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

bool ChatHandler::HandlePasswordCommand(const char* args)
{
    if(!*args)
        return false;

    char *old_pass = strtok ((char*)args, " ");
    char *new_pass = strtok (NULL, " ");
    char *new_pass_c  = strtok (NULL, " ");

    if (!old_pass || !new_pass || !new_pass_c)
        return false;

    std::string password_old = old_pass;
    std::string password_new = new_pass;
    std::string password_new_c = new_pass_c;

    if (strcmp(new_pass, new_pass_c) != 0)
    {
        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage (true);
        return false;
    }

    if (!accmgr.CheckPassword (m_session->GetAccountId(), password_old))
    {
        SendSysMessage (LANG_COMMAND_WRONGOLDPASSWORD);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = accmgr.ChangePassword(m_session->GetAccountId(), password_new);

    switch(result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        case AOR_NAME_NOT_EXIST:                            // not possible case, don't want get account name for output
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleLockAccountCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "on")
    {
        LoginDatabase.PExecute( "UPDATE account SET locked = '1' WHERE id = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
        return true;
    }

    if (argstr == "off")
    {
        LoginDatabase.PExecute( "UPDATE account SET locked = '0' WHERE id = '%d'",m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    return true;
}

/// Display the 'Message of the day' for the realm
bool ChatHandler::HandleServerMotdCommand(const char* /*args*/)
{
    PSendSysMessage(LANG_MOTD_CURRENT, sWorld.GetMotd());
    return true;
}

/*bool ChatHandler::HandleCharRecoveryCommand (const char *)
{
    Player *player = m_session->GetPlayer();

    if (player->isInCombat())
    {
        PSendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);

        return false;
    }

    if (player->InBattleGround())
    {
        PSendSysMessage(LANG_YOU_IN_BATTLEGROUND);
        SetSentErrorMessage(true);

        return false;
    }

    uint32 account_id = m_session->GetAccountId();
    uint32 player_class = player->getClass();
    uint32 player_race = player->getRace();
    uint32 player_guid = player->GetGUID();

    // Recherche de la récupération correspondante
    QueryResult *query = CharacterDatabase.PQuery("SELECT id, niveau, coef, price, valid, done, set_type FROM character_recovery WHERE compte = %u AND classe = %u ORDER BY date DESC LIMIT 0,1", account_id, player_class);

    if (!query)
    {
        PSendSysMessage(LANG_NO_RECUP);
        SetSentErrorMessage(true);

        return false;
    }

    // Attention : l'ordre des champs doit être le même que celui de la requête
    Field *fields = query->Fetch();
    uint32 recovery_id = fields[0].GetUInt32();
    uint8 player_level = fields[1].GetUInt8();
    float coef = fields[2].GetFloat();
    uint8 price = fields[3].GetUInt8();
    uint8 valid = fields[4].GetUInt8();
    uint32 done = fields[5].GetUInt32();
    uint8 set_type = fields[6].GetUInt8();

    if (valid != 1)
    {
    	PSendSysMessage(LANG_RECUP_NOT_VALID);
        SetSentErrorMessage(true);

        return false;
    }

    if (done != 0)
    {
        PSendSysMessage(LANG_RECUP_ALREADY_DONE);
        SetSentErrorMessage(true);

        return false;
    }

    uint8 credits = 0;

    // Si la récupéartion n'est pas gratuite, on vérifie le nombre de crédits
    // du joueurs
    if (price != 0)
    {
        query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

        if (!query)
        {
            PSendSysMessage(LANG_NO_CREDIT_EVER);
            SetSentErrorMessage(true);

            return false;
        }
        else
        {
            fields = query->Fetch();
            credits = fields[0].GetUInt8();

            if (credits < price)
            {
                PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
                SetSentErrorMessage(true);

                return false;
            }

            delete query;
        }
    }

    // Début de la récupération
    player_level = coef * player_level;

    // On lui donne le niveau demandé et on reset son XP
    if (player_level < 1 || player_level > 70)
    {
        PSendSysMessage(LANG_RECUP_WRONG_LEVEL);
        SetSentErrorMessage(true);

        return false;
    }
	else if (player_level != player->getLevel())
	{
    	player->GiveLevel(player_level);
    	player->SetUInt32Value(PLAYER_XP, 0);
	}

    // On lui donne de l'argent
    uint32 money = sConfig.GetIntDefault("Recovery.Money", 2000) * 10000;

    if (money >= 0 && money < MAX_MONEY_AMOUNT)
        player->ModifyMoney(money);

    // Ajout des sorts nécessaire
    query = WorldDatabase.PQuery("SELECT spell_id FROM `recovery_spell` WHERE (`class` = %u OR `class` = 0) AND `level` <= %u ORDER BY `level` ASC", player_class, player_level);

    if (query)
    {
	    uint32 spell_id;

        do
        {
            fields = query->Fetch();
            spell_id = fields[0].GetUInt32();

            player->learnSpell(spell_id);
        } while (query->NextRow());
    }

    delete query;

    // Ajout des objets nécessaire
    // On lui enlève tous ses objets
    Item *item = NULL;
    uint8 msg_store, msg_unequip;
    ItemPosCountVec dest;

    for (uint32 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        msg_store = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        msg_unequip = player->CanUnequipItem(i, false);

        if (item && msg_store == EQUIP_ERR_OK && msg_unequip == EQUIP_ERR_OK)
        {
            player->RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
            player->StoreItem(dest, item, true);
        }
    }

    // On détermine dans quel intervalle de niveau est le joueur
    uint32 player_interval = player_level / 10;
    // On récupère les objets à ajouter au joueur
    query = WorldDatabase.PQuery("SELECT item_id, quantity FROM `recovery_item` WHERE (race = %u OR race = 0) AND (class = %u OR class = 0) AND (level_interval = %u OR level_interval = 0) AND (set_type = %u OR set_type = 0) ORDER BY `order` ASC", player_race, player_class, player_interval, set_type);

    if (query)
    {
        uint32 item_id;
        uint32 quantity;

        do
        {
            fields = query->Fetch();
            item_id = fields[0].GetUInt32();
            quantity = fields[1].GetUInt32();

            player->StoreNewItemInBestSlots(item_id, quantity);
        } while (query->NextRow());
    }

    delete query;

    // Ajout des métiers
    // Sélection des métiers à ajouter
    query = CharacterDatabase.PQuery("SELECT `skill_id`, `level` FROM `character_recovery_skills` WHERE `recovery_id` = %u", recovery_id);
    std::list<std::pair<uint32, uint32> > skill_list;

    if (query)
    {
        // On récupère toutes les compétences à ajouter au joueur
        do
        {
            fields = query->Fetch();

            skill_list.push_back(std::pair<uint32, uint32>(fields[0].GetUInt32(), fields[1].GetUInt32()));
        } while (query->NextRow());

        // On supprime la requête précédente
        delete query;
        
        uint32 skill_id;
        uint32 skill_level;
        uint32 skill_level_max;
        uint32 spell_id;

        // On lit toutes les compétences à lui ajouter
        for (std::list<std::pair<uint32, uint32> >::iterator i = skill_list.begin(); i != skill_list.end(); ++i)
        {
            skill_id = i->first;
            skill_level = i->second;

            if (!player->HasSkill(skill_id) && skill_level > 0)
            {
                // On récupère tous les sorts associés aux compétences
                query = WorldDatabase.PQuery("SELECT `spell_id` FROM `recovery_skill` WHERE `skill_id` = %u AND `level` <= %u ORDER BY `level` ASC", skill_id, skill_level);

                if (query)
                {
                    // On ajoute tous les sorts
                    do
                    {
                        fields = query->Fetch();
                        spell_id = fields[0].GetUInt32();

                        if (spell_id > 0)
                            player->learnSpell(spell_id);
                    } while (query->NextRow());
                }

                delete query;
            }
            
            // On augmente la compétence au niveau du joueur
            if (player->GetSkillValue(skill_id) < skill_level)
            {
                skill_level_max = player->GetPureMaxSkillValue(skill_id);
                player->SetSkill(skill_id, skill_level, skill_level_max);
            }
        }
    }

    // Mise à jour des compétences du joueur
    player->UpdateSkillsToMaxSkillsForLevel();

    // Téléportation du joueur
    if(player->isInFlight())
    {
        player->GetMotionMaster()->MovementExpired();
        player->m_taxi.ClearTaxiDestinations();
    }

    player->TeleportTo(530, -1911.114502, 5406.552734, 2.511920, 0.5155464);
    player->SaveToDB();

    // On lui enlève ses crédits
    if (price != 0)
        LoginDatabase.PExecuteLog("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Récupérations' WHERE id = %u", credits - price, time(NULL), account_id);

    CharacterDatabase.PExecuteLog("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", player->GetGUID(), "Récupération", time(NULL));

    // On met à jour les champs de récupération
    CharacterDatabase.PExecuteLog("UPDATE character_recovery SET done = %u WHERE id = %u", player_guid, recovery_id);

    return true;
}*/

bool ChatHandler::HandleRecupReputations(Player *player, std::string reputs)
{
    std::string tempstr = reputs;
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
        while ((cutAt = tempstr.find_first_of(":")) != tempstr.npos) {
            if (cutAt > 0) {
                v.push_back(tempstr.substr(0, cutAt));
            }
            tempstr = tempstr.substr(cutAt + 1);
        }

        if (tempstr.length() > 0) {
            v.push_back(tempstr);
        }

        /* v[0] == faction id, v[1] == rank */

        uint32 factionId = atoi(v[0].c_str());
        int32 amount = 0;

        switch(atoi(v[1].c_str())) {
            /*  Neutre  0 -> 3000   Amical
                Amical  0 -> 6000   Honore
                Honore  0 -> 12000  Revere
                Revere  0 -> 24000  Exalte
            */
        case 1: /* amical */
            amount = 3000;
            break;
        case 2: /* honore */
            amount = 9000;
            break;
        case 3: /* revere */
            amount = 21000;
            break;
        case 4: /* exalte */
            amount = 45000;
            break;
        }

        if (amount <= 0)
            continue;

        FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionId);

        if (!factionEntry) {
            PSendSysMessage(LANG_COMMAND_FACTION_UNKNOWN, factionId);
            SetSentErrorMessage(true);
            return false;
        }

        if (factionEntry->reputationListID < 0) {
            PSendSysMessage(LANG_COMMAND_FACTION_NOREP_ERROR, factionEntry->name[m_session->GetSessionDbcLocale()], factionId);
            SetSentErrorMessage(true);
            return false;
        }

        player->SetFactionReputation(factionEntry, amount);
        PSendSysMessage(LANG_COMMAND_MODIFY_REP, factionEntry->name[m_session->GetSessionDbcLocale()], factionId, player->GetName(), player->GetReputation(factionId));
    }

    return true;
}

bool ChatHandler::HandleRecupParseCommand(Player *player, std::string command, uint32 metier_level)
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
            uint32 itemsetId = atoi(v[1].c_str());
            bool error = false;

            if (itemsetId == 0) {
                PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                SetSentErrorMessage(true);
                return false;
            }

            QueryResult *result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE itemset = %u", itemsetId);

            if (!result) {
                PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                SetSentErrorMessage(true);
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
                    PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, 1);
                }
            } while(result->NextRow() || error);
            delete result;
        } else if (v[0] == "additem") {
            /* additem, v[1] == item ID, v[2] == item count */
            uint32 itemId = atol(v[1].c_str());
            uint32 count = atoi(v[2].c_str());

            ItemPrototype const *pProto = objmgr.GetItemPrototype(itemId);
            if (!pProto) {
                PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
                SetSentErrorMessage(true);
                return false;
            }

            uint32 noSpaceForCount = 0;

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
            if (msg != EQUIP_ERR_OK)
                count -= noSpaceForCount;

            if (count == 0 || dest.empty()) {
                PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                SetSentErrorMessage(true);
                return false;
            }

            Item *item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

            if (count > 0 && item)
                player->SendNewItem(item, count, true, true);

            if (noSpaceForCount > 0) {
                PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                return false;
            }
        } else if (v[0] == "learn") {
            /* learn, v[1] == spell ID */
            uint32 spell = atol(v[1].c_str());
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer())) {
                PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
                SetSentErrorMessage(true);
                return false;
            }

            if (!player->HasSpell(spell))
                player->learnSpell(spell);
        } else if (v[0] == "money") {
            /* money, v[1] == money count, in pc */
            uint32 money = atoi(v[1].c_str());
            uint32 current_money = player->GetMoney();

            if (money > 0 && (current_money + money) < MAX_MONEY_AMOUNT) {
                player->ModifyMoney(money);
                PSendSysMessage(LANG_YOU_GET_MONEY);
            }
        } else if (v[0] == "setskill") {
            /* skill, v[1] == skill ID */
            int32 skill = atoi(v[1].c_str());
            if (skill <= 0) {
                PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
                SetSentErrorMessage(true);
                return false;
            }

            int32 maxskill = ((int)(metier_level/75)+1)*75;
            if (maxskill > 375)
                maxskill = 375;

            SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
            if (!sl) {
                PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
                SetSentErrorMessage(true);
                return false;
            }

            if (!player->GetSkillValue(skill)) {
                PSendSysMessage(LANG_SET_SKILL_ERROR, player->GetName(), skill, sl->name[0]);
                SetSentErrorMessage(true);
                return false;
            }

            player->SetSkill(skill, metier_level, maxskill);
            PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], player->GetName(), metier_level, maxskill);
        }
    }

    return true;
}

bool ChatHandler::HandleRecupCommand(const char* args)
{
    Player *player = m_session->GetPlayer();
    uint64 account_id = m_session->GetAccountId();
    uint32 pGUID = player->GetGUID();

    QueryResult *query = CharacterDatabase.PQuery("SELECT classe,faction,metier1,metier1_level,metier2,metier2_level,reputs,phase,id,stuff,metier3,metier3_level,guid FROM recups WHERE account = %u AND active = 1 ORDER BY id DESC LIMIT 1", account_id);
    if (!query) {
        PSendSysMessage(LANG_NO_RECUP_AVAILABLE);
        SetSentErrorMessage(true);
        return false;
    }

    Field *fields = query->Fetch();
    uint32 classe, faction, metier1, metier1_level, metier2, metier2_level, phase, recupID, stuff, metier3, metier3_level;
    uint32 recupguid;
    std::string reputs;

    classe = fields[0].GetUInt32();
    faction = fields[1].GetUInt32();
    metier1 = fields[2].GetUInt32();
    metier1_level = fields[3].GetUInt32();
    metier2 = fields[4].GetUInt32();
    metier2_level = fields[5].GetUInt32();

    reputs = fields[6].GetString();

    phase = fields[7].GetUInt32();
    recupID = fields[8].GetUInt32();
    stuff = fields[9].GetUInt32();
    metier3 = fields[10].GetUInt32();
    metier3_level = fields[11].GetUInt32();
    recupguid = fields[12].GetUInt64();

    delete query;

    /* additionnal checks */

    if ((player->GetTeam() == ALLIANCE && faction != 1) || (player->GetTeam() == HORDE && faction != 2) || player->getClass() != classe || phase < 1 || phase > 2) {
        PSendSysMessage(LANG_RECUP_WRONG_DATA);
        SetSentErrorMessage(true);
        return false;
    }

    /* at this point, recup data is assumed to be valid and we can fetch commands to execute */

    if (phase == 1) {
        int level = 70 - player->getLevel();

        query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

        if (!query) {
            PSendSysMessage(LANG_NO_CREDIT_EVER);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        int credits_count = fields[0].GetUInt32();
        delete query;

        if (credits_count < 2) {
            PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
            SetSentErrorMessage(true);
            return false;
        }

        LoginDatabase.PExecuteLog("UPDATE account_credits SET amount = amount-2 WHERE id = %u", account_id);

        player->GiveLevel(70);
//        player->UpdateSkillsToMaxSkillsForLevel();
        player->SetUInt32Value(PLAYER_XP, 0);
        PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName(), 70);

        if (classe == 3)
            query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE phase = 1 AND classe = 3");
        else
            query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE phase = 1 AND classe = 0");

        if (!query) {
            PSendSysMessage(LANG_RECUP_CORRUPT);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        std::string command = fields[0].GetString();
        delete query;

        if (!ChatHandler::HandleRecupParseCommand(player, command, 0)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        CharacterDatabase.PExecuteLog("UPDATE recups SET phase=2 WHERE id = %u", recupID);
        CharacterDatabase.PExecuteLog("UPDATE recups SET guid=%lu WHERE id=%u", pGUID, recupID);
        PSendSysMessage(LANG_RECUP_PHASE1_SUCCESS);
    } else {
        if (recupguid != pGUID) {
            PSendSysMessage(LANG_RECUP_WRONG_CHAR);
            SetSentErrorMessage(true);
            return false;
        }

        if (player->HasSpell(2259) || player->HasSpell(3908) || player->HasSpell(2366) || player->HasSpell(8613) || player->HasSpell(7411) ||
            player->HasSpell(2018) || player->HasSpell(4036) || player->HasSpell(25229) || player->HasSpell(2575) || player->HasSpell(2108)) {
            PSendSysMessage(LANG_RECUP_SKILL_EXIST);
            SetSentErrorMessage(true);
            return false;
        }

        /* first, add all stuff items (set, offset, weapons, etc) */
        query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE classe = %u AND faction = %u AND phase = 2 AND stuff = %u", classe, faction, stuff);
        if (!query) {
            PSendSysMessage(LANG_RECUP_CORRUPT);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        std::string command = fields[0].GetString();
        delete query;

        if (!ChatHandler::HandleRecupParseCommand(player, command, 0)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        /* next, give profession skills */
        query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE metier = %u AND metier != 0", metier1);
        if (!query) {
            PSendSysMessage(LANG_RECUP_CORRUPT);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        command = fields[0].GetString();
        delete query;

        if (!ChatHandler::HandleRecupParseCommand(player, command, metier1_level)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE metier = %u AND metier != 0", metier2);
        if (!query) {
            PSendSysMessage(LANG_RECUP_CORRUPT);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        command = fields[0].GetString();
        delete query;

        if (!ChatHandler::HandleRecupParseCommand(player, command, metier2_level)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        if (metier3 != 0) {
            query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE metier = %u AND metier != 0", metier3);
            if (!query) {
                PSendSysMessage(LANG_RECUP_CORRUPT);
                SetSentErrorMessage(true);
                return false;
            }

            fields = query->Fetch();
            command = fields[0].GetString();
            delete query;

            if (!ChatHandler::HandleRecupParseCommand(player, command, metier3_level)) {
                PSendSysMessage(LANG_RECUP_CMD_FAILED);
                SetSentErrorMessage(true);
                return false;
            }
        }

        /* next, give money and mount skill */
        query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE phase=2 AND metier=0 AND classe=0 AND faction=0");
        if (!query) {
            PSendSysMessage(LANG_RECUP_CORRUPT);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        command = fields[0].GetString();
        delete query;

        if (!ChatHandler::HandleRecupParseCommand(player, command, 0)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        /* next, give weapon skills */
        query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE phase=3 AND classe = %u", classe);
        if (!query)  {
            PSendSysMessage(LANG_RECUP_CORRUPT);
            SetSentErrorMessage(true);
            return false;
        }

        fields = query->Fetch();
        command = fields[0].GetString();
        delete query;

        if (!ChatHandler::HandleRecupParseCommand(player, command, 0))  {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        /* finally, give reputations */
        if (!ChatHandler::HandleRecupReputations(player, reputs)) {
            PSendSysMessage(LANG_RECUP_REPUT_FAILED);
            SetSentErrorMessage(true);
            return false;
        }

        /* upgrade skills */

        player->UpdateSkillsToMaxSkillsForLevel();

        /* tele to shattrath */
        char telename[8] = {0};
        strncpy(telename, "shat", 7);
        GameTele const* tele = extractGameTeleFromLink(telename);

        if (!tele) {
            PSendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
            SetSentErrorMessage(true);
            return false;
        }

        MapEntry const* me = sMapStore.LookupEntry(tele->mapId);
        if (!me || me->IsBattleGroundOrArena()) {
            PSendSysMessage(LANG_CANNOT_TELE_TO_BG);
            SetSentErrorMessage(true);
            return false;
        }

        if (player->IsBeingTeleported() == true) {
            PSendSysMessage(LANG_IS_TELEPORTED, player->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, player->GetName(),"", tele->name.c_str());

        if (player->isInFlight()) {
            player->GetMotionMaster()->MovementExpired();
            player->m_taxi.ClearTaxiDestinations();
        } else
            player->SaveRecallPosition();

        player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);

        /* at this point, recup is completed */

        CharacterDatabase.PExecuteLog("UPDATE recups SET active=0 WHERE id = %u", recupID);
        PSendSysMessage(LANG_RECUP_PHASE2_SUCCESS);
    }

    player->SaveToDB();
    return true;
}

bool ChatHandler::HandleViewCreditsCommand(const char *args)
{
    uint64 account_id = m_session->GetAccountId();

    QueryResult *query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!query)
    {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
    }
    else 
    {
        Field *field = query->Fetch();
        uint32 credits = field[0].GetUInt32();
        delete query;

        PSendSysMessage(LANG_CREDIT_INFO, credits);
    }

    return true;
}

bool ChatHandler::HandleBuyInShopCommand(const char *args)
{
    if (!args || *args == '\0')
        return false;

    Player *player = m_session->GetPlayer();
    uint64 account_id = m_session->GetAccountId();
    QueryResult *query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!query) {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
        return false;
    }

    int plevel = player->getLevel();
    Field *fields = query->Fetch();
    uint32 credits = fields[0].GetUInt32();

    delete query;

    std::string safe_args = args;
    WorldDatabase.escape_string(safe_args);

    query = WorldDatabase.PQuery("SELECT actions, cost FROM shop_orders WHERE name = '%s' AND cost <= %u AND (class = %u OR class = 0) AND (level_min <= %u OR level_min = 0) AND (level_max >= %u OR level_max = 0) AND (race = %u OR race = 0) ORDER BY level_min DESC LIMIT 1", safe_args.c_str(), credits, player->getClass(), plevel, plevel, player->getRace());

    if (!query) 
    {
        PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
        SetSentErrorMessage(true);
        return false;
    }

    fields = query->Fetch();
    std::string script = fields[0].GetString();
    std::string actions = script;
    uint32 cost = fields[1].GetUInt32();
    bool can_take_credits = true;

    delete query;

    std::vector<std::string> v, vline;
    std::vector<std::string>::iterator i;
    std::string tempstr;

    int cutAt;
    tempstr = script;
    while ((cutAt = tempstr.find_first_of("\n")) != tempstr.npos) {
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

        if (v[0] == "teleport") {
            char telename[8] = {0};
            strncpy(telename, v[1].c_str(), 7);
            GameTele const* tele = extractGameTeleFromLink(telename);

            if (!tele) {
                PSendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
                SetSentErrorMessage(true);
                continue;
            }

            MapEntry const* me = sMapStore.LookupEntry(tele->mapId);
            if (!me || me->IsBattleGroundOrArena()) {
                PSendSysMessage(LANG_CANNOT_TELE_TO_BG);
                SetSentErrorMessage(true);
                return false;
            }

            if (player->IsBeingTeleported() == true) {
                PSendSysMessage(LANG_IS_TELEPORTED, player->GetName());
                SetSentErrorMessage(true);
                return false;
            }

            PSendSysMessage(LANG_TELEPORTING_TO, player->GetName(),"", tele->name.c_str());

            if (player->isInFlight()) {
                player->GetMotionMaster()->MovementExpired();
                player->m_taxi.ClearTaxiDestinations();
            } else
                player->SaveRecallPosition();

            player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
        } else if (v[0] == "set") {
            if (v[1] == "new_name") {
                int is_allowed = atoi(v[2].c_str());

                if (is_allowed == 1) {
                    player->SetAtLoginFlag(AT_LOGIN_RENAME);
                    CharacterDatabase.PExecuteLog("UPDATE characters SET at_login = at_login | '1' WHERE guid = %u", player->GetGUID());
                    PSendSysMessage(LANG_RENAME_PLAYER);
                } else {
                    player->SetAtLoginFlag(AT_LOGIN_NONE);
                    CharacterDatabase.PExecuteLog("UPDATE characters SET at_login = 0 WHERE guid = %u", player->GetGUID());
                }

                can_take_credits = true;
            } else if (v[1] == "level") {
                int level = atoi(v[2].c_str());
                if (level > 70) level = 70;
                if (level < 1) level = 1;

                player->GiveLevel(level);

                int reset = atoi(v[2].c_str());
                if (reset == 1) player->SetUInt32Value(PLAYER_XP, 0);

                PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName(), level);

                can_take_credits = true;
            }
        } else if (v[0] == "learn") {
            /* learn, v[1] == spell ID */
            uint32 spell = atol(v[1].c_str());
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer())) {
                PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
                SetSentErrorMessage(true);
                return false;
            }

            if (!player->HasSpell(spell))
                player->learnSpell(spell);
        } else if (v[0] == "add") {
            if (v[1] == "level") {
                int add_levels = atoi(v[2].c_str());
                int level = player->getLevel() + add_levels;

                if (level > 70) level = 70;
                if (level < 1) level = 1;

                player->GiveLevel(level);

                int maxskill = atoi(v[3].c_str());
                if (maxskill == 1)
                    player->UpdateSkillsToMaxSkillsForLevel();

                int reset = atoi(v[4].c_str());
                if (reset == 1)
                    player->SetUInt32Value(PLAYER_XP, 0);

                PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName(), level);
                can_take_credits = true;
            } else if (v[1] == "money") {
                uint32 money = atoi(v[2].c_str());
                uint32 current_money = player->GetMoney();

                if (money > 0 && (current_money + money) < MAX_MONEY_AMOUNT) {
                    player->ModifyMoney(money);
                    can_take_credits = true;
                }
            } else if (v[1] == "item") {
                uint32 itemId = 0;
                uint32 count = 0;

                if (v[2] == "random")
                {
                    QueryResult *query = WorldDatabase.PQuery("SELECT item_id, count FROM shop_items_random WHERE category = %u ORDER BY RAND() LIMIT 1", atoi(v[3].c_str()));

                    if (query)
                    {
                        Field *field = query->Fetch();
                        itemId = field[0].GetUInt32();
                        count = field[1].GetUInt32();

                        delete query;
                    }
                    else
                    {
                        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
                        SetSentErrorMessage(true);

                        return false;
                    }
                }
                else if (v[2] == "equiped")
                {
                  itemId = atol(v[3].c_str());
                  count = atoi(v[4].c_str());
                }
                else
                {
                    itemId = atol(v[2].c_str());
                    count = atoi(v[3].c_str());
                }

                ItemPrototype const *pProto = objmgr.GetItemPrototype(itemId);
                if (!pProto) {
                    PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
                    SetSentErrorMessage(true);
                    return false;
                }

                uint32 noSpaceForCount = 0;

                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
                if (msg != EQUIP_ERR_OK)
                    count -= noSpaceForCount;

                if (count == 0 || dest.empty()) {
                    PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                    SetSentErrorMessage(true);
                    continue;
                }

                if (v[2] == "equiped")
                {
                  player->StoreNewItemInBestSlots(itemId, count);
                }
                else
                {
                  Item *item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

                  if (count > 0 && item) {
                    player->SendNewItem(item, count, true, true);
                    can_take_credits = true;
                  }
                }

                if (noSpaceForCount > 0) {
                    PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                    continue;
                }
            } else if (v[1] == "itemset") {
                uint32 itemsetId = atol(v[2].c_str());
                bool error = false;

                if (itemsetId == 0) {
                    PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                    SetSentErrorMessage(true);
                    return false;
                }

                QueryResult *result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE itemset = %u", itemsetId);

                if (!result) {
                    PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                    SetSentErrorMessage(true);
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
                        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, 1);
                    }
                } while (result->NextRow() || error);

                can_take_credits = true;
            }
        }
    }

    if (can_take_credits == true) {
        LoginDatabase.PExecuteLog("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", credits - cost, time(NULL), account_id);
        CharacterDatabase.PExecuteLog("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", player->GetGUID(), actions.c_str(), time(NULL));
        player->SaveToDB();

        return true;
    } else {
        sLog.outString("Erreur boutique - actions : %s - GUID : %u cout : %u - heure : %u", actions, player->GetGUID(), cost, time(NULL));
        return false;
    }
}

bool ChatHandler::HandleHerodayCommand(const char* args)
{
    int loc_idx = m_session->GetSessionDbLocaleIndex();
    if (loc_idx >= 0) {
        QuestLocale const* pQuest = objmgr.GetQuestLocale(sWorld.GetCurrentQuestForPool(1));
        if (pQuest) {
            if (pQuest->Title.size() > loc_idx && !pQuest->Title[loc_idx].empty())
                PSendSysMessage("La quête héroïque du jour est : \"%s\".", pQuest->Title[loc_idx].c_str());
        }
        else
            PSendSysMessage("Erreur lors de la récupération de la quête journalière.");
    }
    
    return true;
}

bool ChatHandler::HandleReskinCommand(const char* args)
{
    if (!args || !*args)
        return false;
        
    char* targetName = strtok((char*)args, "");
    std::string safeTargetName = targetName;
    CharacterDatabase.escape_string(safeTargetName);
    
    QueryResult *result = CharacterDatabase.PQuery("SELECT guid, account, race, gender, playerBytes, playerBytes2 FROM characters WHERE name = '%s'", safeTargetName.c_str());
    
    if (!result)
        return false;
    
    Field *fields = result->Fetch();
    
    uint32 t_guid = fields[0].GetUInt32();
    uint32 t_account = fields[1].GetUInt32();
    uint32 t_race = fields[2].GetUInt32();
    uint32 t_gender = fields[3].GetUInt32();
    uint32 t_playerBytes = fields[4].GetUInt32();
    uint32 t_playerBytes2 = fields[5].GetUInt32();
    
    uint32 m_race = m_session->GetPlayer()->GetRace();
    uint32 m_gender = m_session->GetPlayer()->GetGender();
    
    if (t_race != m_race || t_gender != m_gender || t_guid == m_session->GetPlayer()->GetGUIDLow() || t_account != m_session->GetAccountId())
        return false;

    m_session->GetPlayer()->SetUInt32Value(PLAYER_BYTES, t_playerBytes);
    m_session->GetPlayer()->SetUInt32Value(PLAYER_BYTES_2, t_playerBytes2);
    
    m_session->GetPlayer()->SaveToDB();
    m_session->KickPlayer();
    
    return true;
}
