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
#include "BattleGround.h"
#include "Guild.h"
#include "ArenaTeam.h"
#include "PlayerDump.h"
#include "AuctionHouseMgr.h"

#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Config/ConfigEnv.h"

#include <sstream>

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
    
    if (chr->HasAura(9454)) // Char is freezed by GM
    {
        SendSysMessage("Impossible lorsque vous êtes gelé.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if(chr->InBattleGround())
    {
        if (chr->isAlive())
            SendSysMessage("Inutilisable en champ de bataille lorsque vous �tes en vie.");
        else {
            BattleGround* bg = chr->GetBattleGround();
            if (bg) {
                WorldSafeLocsEntry const* closestGrave = bg->GetClosestGraveYard(chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), chr->GetTeam());
                if (closestGrave)
                    chr->TeleportTo(bg->GetMapId(), closestGrave->x, closestGrave->y, closestGrave->z, chr->GetOrientation());
            }
        }
        
        return true;
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

    PSendSysMessage(_FULLVERSION);
    PSendSysMessage("Joueurs en ligne: %u (Max: %u)", activeClientsNum, maxActiveClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());
    PSendSysMessage("Update time diff lissé: %u.", sWorld.GetFastTimeDiff());
    PSendSysMessage("Update time diff instantané: %u.", sWorld.GetUpdateTime());
    if (sWorld.IsShuttingDown())
        PSendSysMessage("Arret du serveur dans %s", secsToTimeString(sWorld.GetShutDownTimeLeft()).c_str());

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

    if (!sAccountMgr.CheckPassword (m_session->GetAccountId(), password_old))
    {
        SendSysMessage (LANG_COMMAND_WRONGOLDPASSWORD);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = sAccountMgr.ChangePassword(m_session->GetAccountId(), password_new);

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
        LoginDatabase.PExecute("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Récupérations' WHERE id = %u", credits - price, time(NULL), account_id);

    CharacterDatabase.PExecute("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", player->GetGUID(), "Récupération", time(NULL));

    // On met à jour les champs de récupération
    CharacterDatabase.PExecute("UPDATE character_recovery SET done = %u WHERE id = %u", player_guid, recovery_id);

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

bool ChatHandler::HandleRecupParseCommand(Player *player, std::string command, uint32 metier_level, bool equip)
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
                PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                return false;
            }
        } else if (v[0] == "learn") {
            /* learn, v[1] == spell ID */
            uint32 spell = atol(v[1].c_str());
            SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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

            player->SetSkill(skill, metier_level ? 1 : NULL, maxskill);
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

    QueryResult *query = CharacterDatabase.PQuery("SELECT classe,faction,metier1,metier1_level,metier2,metier2_level,reputs,phase,id,stuff,metier3,metier3_level,guid,stufflevel FROM recups WHERE account = %u AND active = 1 ORDER BY id DESC LIMIT 1", account_id);
    if (!query) {
        PSendSysMessage(LANG_NO_RECUP_AVAILABLE);
        SetSentErrorMessage(true);
        return false;
    }

    Field *fields = query->Fetch();
    uint32 classe, faction, metier1, metier1_level, metier2, metier2_level, phase, recupID, stuff, metier3, metier3_level;
    uint32 recupguid, stufflevel;
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
    stufflevel = fields[13].GetUInt32();

    delete query;

    /* additionnal checks */

    if ((player->GetTeam() == ALLIANCE && faction != 1) || (player->GetTeam() == HORDE && faction != 2) || player->getClass() != classe || phase < 1 || phase > 2) {
        PSendSysMessage(LANG_RECUP_WRONG_DATA);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 noSpaceForCount = 0;
    ItemPosCountVec dest;
    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 21841, 7, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK || noSpaceForCount > 0)
    {
        PSendSysMessage(LANG_RECUP_NO_SPACE);
        SetSentErrorMessage(true);
        return false;
    }

    if (player->HasSpell(2259) || player->HasSpell(3908) || player->HasSpell(2366) || player->HasSpell(8613) || player->HasSpell(7411) ||
        player->HasSpell(2018) || player->HasSpell(4036) || player->HasSpell(25229) || player->HasSpell(2575) || player->HasSpell(2108)) {
        PSendSysMessage(LANG_RECUP_SKILL_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    /* at this point, recup data is assumed to be valid and we can fetch commands to execute */

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

    query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE classe = %u AND (faction = %u OR faction = 0) AND stufflevel = %u AND phase = 2 AND (stuff = %u OR stuff = -1)", classe, faction, stufflevel, stuff);
    if (!query) {
        PSendSysMessage(LANG_RECUP_CORRUPT);
        SetSentErrorMessage(true);
        return false;
    }

    LoginDatabase.PExecute("UPDATE account_credits SET amount = amount-2 WHERE id = %u", account_id);

    player->GiveLevel(70);
    player->SetUInt32Value(PLAYER_XP, 0);
    PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName(), 70);

    if (!ChatHandler::HandleRecupParseCommand(player, command, 0, true)) {
        PSendSysMessage(LANG_RECUP_CMD_FAILED);
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE recups SET phase=2 WHERE id = %u", recupID);
    CharacterDatabase.PExecute("UPDATE recups SET guid=%lu WHERE id=%u", pGUID, recupID);
/*    PSendSysMessage(LANG_RECUP_PHASE1_SUCCESS);

    if (recupguid != pGUID) {
        PSendSysMessage(LANG_RECUP_WRONG_CHAR);
        SetSentErrorMessage(true);
        return false;
    }*/

    /* first, add all stuff items (set, offset, weapons, etc) */

    do {
        fields = query->Fetch();
        std::string command_p2 = fields[0].GetString();

        if (!ChatHandler::HandleRecupParseCommand(player, command_p2, 0)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }
    } while (query->NextRow());

    delete query;

    /* next, give profession skills */
    if (metier1)
    {
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
    }

    if (metier2)
    {
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
    if (player->IsBeingTeleported() == true) {
        PSendSysMessage(LANG_IS_TELEPORTED, player->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_TELEPORTING_TO, player->GetName(),"", "Shattrath");

    player->TeleportTo(530, -1838.160034, 5301.790039, -12.428000, 5.951700);

    /* at this point, recup is completed */

    CharacterDatabase.PExecute("UPDATE recups SET active=0 WHERE id = %u", recupID);
    PSendSysMessage(LANG_RECUP_PHASE2_SUCCESS);

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

    query = WorldDatabase.PQuery("SELECT actions, cost, name FROM shop_orders WHERE name = '%s' AND cost <= %u AND (class = %u OR class = 0) AND (level_min <= %u OR level_min = 0) AND (level_max >= %u OR level_max = 0) AND (race = %u OR race = 0) ORDER BY level_min DESC LIMIT 1", safe_args.c_str(), credits, player->getClass(), plevel, plevel, player->getRace());

    if (!query) 
    {
        PSendSysMessage("Commande inconnue ou crédits insuffisants. Vérifiez que vous avez correctement entré la commande.");
        SetSentErrorMessage(true);
        return false;
    }

    fields = query->Fetch();
    std::string script = fields[0].GetString();
    std::string actions = script;
    const char* buyName = fields[2].GetString();
    uint32 cost = fields[1].GetUInt32();
    bool can_take_credits = true;

    delete query;
    
    // Check that the player has enough free slots in inventory
    // 8 for "set T1", "set T2"
    // 9 for "set S0"
    // 15 for "perso"
    uint32 freeSlots = player->GetEmptyBagSlotsCount();
    uint32 requiredSlots = 0;
    
    if (!strncmp(buyName, "set T1", 5))
        requiredSlots = 8;
    else if (!strncmp(buyName, "set T2", 5))
        requiredSlots = 8;
    else if (!strncmp(buyName, "set S0", 5))
        requiredSlots = 9;
    else if (!strncmp(buyName, "perso", 5))
        requiredSlots = 15;

    if (freeSlots < requiredSlots) {
        PSendSysMessage("Vous n'avez pas assez d'emplacements d'inventaire libres pour cette commande (%u requis).", requiredSlots);
        SetSentErrorMessage(true);
        return false;
    }
    
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

            /*if (!tele) {
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
            }*/

            PSendSysMessage(LANG_TELEPORTING_TO, player->GetName(),"", "Shattrath");

            if (player->isInFlight()) {
                player->GetMotionMaster()->MovementExpired();
                player->m_taxi.ClearTaxiDestinations();
            } else
                player->SaveRecallPosition();

            //player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
            player->TeleportTo(530, -1838.160034, 5301.790039, -12.428000, 5.951700);
        } else if (v[0] == "set") {
            if (v[1] == "new_name") {
                int is_allowed = atoi(v[2].c_str());

                if (is_allowed == 1) {
                    player->SetAtLoginFlag(AT_LOGIN_RENAME);
                    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '1' WHERE guid = %u", player->GetGUID());
                    PSendSysMessage(LANG_RENAME_PLAYER);
                } else {
                    player->SetAtLoginFlag(AT_LOGIN_NONE);
                    CharacterDatabase.PExecute("UPDATE characters SET at_login = 0 WHERE guid = %u", player->GetGUID());
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
            SpellEntry const* spellInfo = spellmgr.LookupSpell(spell);
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
        LoginDatabase.PExecute("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", credits - cost, time(NULL), account_id);
        CharacterDatabase.PExecute("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", player->GetGUID(), actions.c_str(), time(NULL));
        player->SaveToDB();

        return true;
    } else {
        sLog.outString("Erreur boutique - actions : %s - GUID : %u cout : %u - heure : %u", actions.c_str(), player->GetGUID(), cost, time(NULL));
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
            else {
                if (Quest const* qtemplate = objmgr.GetQuestTemplate(sWorld.GetCurrentQuestForPool(1)))
                    PSendSysMessage("Heroday: \"%s\".", qtemplate->GetTitle().c_str());
            }
        }
        else
            PSendSysMessage("Erreur lors de la récupération de la quête journalière.");
    }
    else {
        if (Quest const* qtemplate = objmgr.GetQuestTemplate(sWorld.GetCurrentQuestForPool(1)))
            PSendSysMessage("Heroday: \"%s\".", qtemplate->GetTitle().c_str());
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
    uint64 account_id = m_session->GetAccountId();
    QueryResult *result = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!result) {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
        return false;
    }

    Field *fields = result->Fetch();
    uint32 credits = fields[0].GetUInt32();

    delete result;

    if (credits < 1) {
        PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
        SetSentErrorMessage(true);
        return false;
    }

    result = CharacterDatabase.PQuery("SELECT guid, account, race, gender, playerBytes, playerBytes2 FROM characters WHERE name = '%s'", safeTargetName.c_str());
    
    if (!result)
        return false;
    
    fields = result->Fetch();
    
    uint32 t_guid = fields[0].GetUInt32();
    uint32 t_account = fields[1].GetUInt32();
    uint32 t_race = fields[2].GetUInt32();
    uint8 t_gender = fields[3].GetUInt8();
    uint32 t_playerBytes = fields[4].GetUInt32();
    uint32 t_playerBytes2 = fields[5].GetUInt32();
    
    delete result;
    
    uint32 m_race = m_session->GetPlayer()->GetRace();
    uint32 m_gender = m_session->GetPlayer()->GetGender();
    
    if (t_race != m_race /*|| t_gender != m_gender */|| t_guid == m_session->GetPlayer()->GetGUIDLow() || t_account != m_session->GetAccountId())
        return false;
        
    if (m_session->GetPlayer()->GetLastGenderChange() > (time(NULL) - sWorld.getConfig(CONFIG_PLAYER_GENDER_CHANGE_DELAY) * 86400)) {
        if (t_gender != m_gender) {
            uint32 delta = time(NULL) - m_session->GetPlayer()->GetLastGenderChange();
            uint32 days = uint32(delta / 86400.0f);
            uint32 hours = uint32((delta - (days * 86400)) / 3600.0f);
            uint32 minutes = uint32((delta - (days * 86400) - (hours * 3600)) / 60.0f);
            PSendSysMessage("Vous ne pouvez pas faire plus d'un changement de sexe tous les %u jours. Dernier changement il y a %u jours %u heures %u minutes.",
                sWorld.getConfig(CONFIG_PLAYER_GENDER_CHANGE_DELAY), days, hours, minutes);
            return true;
        }
    }

    uint32 bankBags = m_session->GetPlayer()->GetByteValue(PLAYER_BYTES_2, 2);

    m_session->GetPlayer()->SetUInt32Value(PLAYER_BYTES, t_playerBytes);
    m_session->GetPlayer()->SetUInt32Value(PLAYER_BYTES_2, t_playerBytes2);
    m_session->GetPlayer()->SetByteValue(PLAYER_BYTES_2, 2, bankBags);
    m_session->GetPlayer()->SetGender(t_gender);
    if (t_gender != m_gender) {
        m_session->GetPlayer()->SetLastGenderChange(time(NULL));
        //CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '4' WHERE guid ='%u'", m_session->GetPlayer()->GetGUIDLow()); //TODO: to be discussed
    }

    LoginDatabase.PExecute("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", credits - 1, time(NULL), account_id);

    m_session->GetPlayer()->SaveToDB();
//    m_session->KickPlayer();
    m_session->GetPlayer()->m_kickatnextupdate = true;
    
    return true;
}

bool ChatHandler::HandleRaceOrFactionChange(const char* args)
{
    if (!args || !*args)
        return false;
        
    char* targetName = strtok((char*)args, "");
    std::string safeTargetName = targetName;
    CharacterDatabase.escape_string(safeTargetName);
    uint64 account_id = m_session->GetAccountId();
    QueryResult* result = NULL;
    Field* fields = NULL;
    
    if (!sWorld.getConfig(CONFIG_FACTION_CHANGE_ENABLED)) {
        PSendSysMessage("Le changement de race/faction est actuellement désactivé.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (!m_session->GetPlayer()->isAlive()) {
        PSendSysMessage("Vous devez être en vie pour effectuer un changement de race ou faction.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->isInCombat()) {
        PSendSysMessage("Impossible en combat.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->GetBattleGround()) {
        PSendSysMessage("Impossible en champ de bataille ou en arène.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->GetGroup()) {
        PSendSysMessage("Veuillez quitter votre groupe pour effectuer le changement.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->GetInstanceId() != 0) {
        PSendSysMessage("Impossible en instance.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->getLevel() < 10) {
        PSendSysMessage(LANG_FACTIONCHANGE_LEVEL_MIN);
        SetSentErrorMessage(true);
        return false;
    }

    result = CharacterDatabase.PQuery("SELECT guid, account, race, gender, playerBytes, playerBytes2 FROM characters WHERE name = '%s'", safeTargetName.c_str());
    
    if (!result) {
        PSendSysMessage("Personnage cible non trouvé.");
        SetSentErrorMessage(true);
        return false;
    }
    
    fields = result->Fetch();
    
    Player* plr = m_session->GetPlayer();
    
    // My values
    uint32 m_guid = plr->GetGUIDLow();
    uint32 m_account = m_session->GetAccountId();
    uint32 m_class = plr->getClass();
    uint32 m_race = plr->getRace();
    uint8 m_gender = plr->getGender();
    uint64 m_fullGUID = plr->GetGUID();
    
    // Target values
    uint32 t_guid = fields[0].GetUInt32();
    uint32 t_account = fields[1].GetUInt32();
    uint32 t_race = fields[2].GetUInt32();
    uint8 t_gender = fields[3].GetUInt32();
    uint32 t_playerBytes = fields[4].GetUInt32();
    uint32 t_playerBytes2 = fields[5].GetUInt32();
    
    delete result;
    
    uint32 dest_team = BG_TEAM_ALLIANCE;
    // Search each faction is targeted
    switch (t_race)
    {
        case RACE_ORC:
        case RACE_TAUREN:
        case RACE_UNDEAD_PLAYER:
        case RACE_TROLL:
        case RACE_BLOODELF:
            dest_team = BG_TEAM_HORDE;
            break;
        default:
            break;
    }

    PlayerInfo const* targetInfo = objmgr.GetPlayerInfo(t_race, m_class);
    if (!targetInfo) {
        PSendSysMessage("La race du personnage cible est incompatible avec votre classe.");
        SetSentErrorMessage(true);
        return false;
    }
    
    PlayerInfo const* myInfo = objmgr.GetPlayerInfo(m_race, m_class);
    bool factionChange = (Player::TeamForRace(m_race) != Player::TeamForRace(t_race));
    
    // Check if this transfer is currently allowed
    if (factionChange) {
        if (dest_team == BG_TEAM_ALLIANCE && !sWorld.getConfig(CONFIG_FACTION_CHANGE_H2A)) {
            PSendSysMessage("Le changement de faction n'est actuellement pas autorisé dans le sens Horde -> Alliance.");
            SetSentErrorMessage(true);
            return false;
        } else if (dest_team == BG_TEAM_HORDE && !sWorld.getConfig(CONFIG_FACTION_CHANGE_A2H)) {
            PSendSysMessage("Le changement de faction n'est actuellement pas autorisé dans le sens Alliance -> Horde.");
            SetSentErrorMessage(true);
            return false;
        }
    }
    
    uint32 cost = 4;
    if (m_session->GetSecurity() <= SEC_PLAYER) {
        result = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

        if (!result) {
            PSendSysMessage(LANG_NO_CREDIT_EVER);
            SetSentErrorMessage(true);
            return false;
        }

        fields = result->Fetch();
        uint32 credits = fields[0].GetUInt32();

        delete result;

        if (credits < cost) {
            PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
            SetSentErrorMessage(true);
            return false;
        }
    }
    
    // Check guild and arena team, friends are removed after the SaveToDB() call
    // Guild
    if (factionChange) {
        Guild* guild = objmgr.GetGuildById(plr->GetGuildId());
        if (guild) {
            PSendSysMessage("Vous êtes actuellement dans une guilde. Veuillez la quitter pour effectuer le changement de faction.");
            SetSentErrorMessage(true);
            return false;
        }
    }
    
    // Arena teams
    if (factionChange) {
        result = CharacterDatabase.PQuery("SELECT arena_team_member.arenaTeamId FROM arena_team_member JOIN arena_team ON arena_team_member.arenaTeamId = arena_team.arenaTeamId WHERE guid = %u", plr->GetGUIDLow());

        if (result) {
            PSendSysMessage("Vous êtes actuellement dans une ou plusieurs équipes d'arène. Veuillez les quitter pour effectuer le changement de faction.");
            SetSentErrorMessage(true);
            return false;
        }
    
        delete result;
    }
    
    // Dump player by safety
    std::ostringstream oss;
    std::string fname = sConfig.GetStringDefault("LogsDir", "");
    oss << fname;
    if (fname.length() > 0 && fname.at(fname.length()-1) != '/')
        oss << "/";
    oss << "chardump_factionchange/" << account_id << "_" << GUID_LOPART(m_session->GetPlayer()->GetGUID()) << "_" << m_session->GetPlayer()->GetName();
    PlayerDumpWriter().WriteDump(oss.str().c_str(), GUID_LOPART(m_session->GetPlayer()->GetGUID()));
    
    WorldLocation loc;
    uint32 area_id = 0;
    if (factionChange) {
        if (Player::TeamForRace(t_race) == ALLIANCE) {
            loc = WorldLocation(0, -8866.468750, 671.831238, 97.903374, 2.154216);
            area_id = 1519; // Stormwind
        }
        else {
            loc = WorldLocation(1, 1632.54, -4440.77, 15.4584, 1.0637);
            area_id = 1637; // Orgrimmar
        }
    }
    
    // Homebind
    if (factionChange)
        plr->SetHomebindToLocation(loc, area_id);
    
    uint32 bankBags = plr->GetByteValue(PLAYER_BYTES_2, 2);
    plr->SetByteValue(UNIT_FIELD_BYTES_0, 0, t_race);
    plr->SetRace(t_race);
    plr->SetUInt32Value(PLAYER_BYTES, t_playerBytes);
    plr->SetUInt32Value(PLAYER_BYTES_2, t_playerBytes2);
    plr->SetByteValue(PLAYER_BYTES_2, 2, bankBags);
    plr->SetGender(t_gender);
    
    // Reset flys at next login
    if (factionChange)
        plr->SetAtLoginFlag(AT_LOGIN_RESET_FLYS);
    
    // Remove previous race starting spells
    std::list<CreateSpellPair>::const_iterator spell_itr;
    for (spell_itr = myInfo->spell.begin(); spell_itr != myInfo->spell.end(); ++spell_itr) {
        uint16 tspell = spell_itr->first;
        if (tspell)
            plr->removeSpell(tspell,spell_itr->second);
    }
    // Add new race starting spells
    for (spell_itr = targetInfo->spell.begin(); spell_itr != targetInfo->spell.end(); ++spell_itr) {
        uint16 tspell = spell_itr->first;
        if (tspell) {
            if (!spell_itr->second)               // don't care about passive spells or loading case
                plr->addSpell(tspell,spell_itr->second);
            else                                            // but send in normal spell in game learn case
                plr->learnSpell(tspell);
        }
    }
    
    // Reset current quests
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; slot++) {
        if (uint32 quest = plr->GetQuestSlotQuestId(slot)) {
            plr->TakeQuestSourceItem(quest, true);

            plr->SetQuestStatus( quest, QUEST_STATUS_NONE);
        }

        plr->SetQuestSlot(slot, 0);
    }
    
    // Titles
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = objmgr.factionchange_titles.begin(); it != objmgr.factionchange_titles.end(); ++it) {
            CharTitlesEntry const* title_alliance = sCharTitlesStore.LookupEntry(it->first);
            CharTitlesEntry const* title_horde = sCharTitlesStore.LookupEntry(it->second);
            
            if (!title_alliance || !title_horde)
                continue;

            if (dest_team == BG_TEAM_ALLIANCE) {
                if (plr->HasTitle(title_horde)) {
                    plr->RemoveTitle(title_horde);
                    plr->SetTitle(title_alliance);
                }
            }
            else {
                if (plr->HasTitle(title_alliance)) {
                    plr->RemoveTitle(title_alliance);
                    plr->SetTitle(title_horde);
                }
            }
        }
    }
    
    // Reputations
    result = WorldDatabase.PQuery("SELECT faction_from, faction_to FROM player_factionchange_reputations WHERE race_from = %u AND race_to = %u", m_race, t_race);
    if (result) {
        do {
            Field* fields = result->Fetch();
            
            uint32 from = fields[0].GetUInt32();
            uint32 to = fields[1].GetUInt32();
            
            if (!from)
                continue;
            
            if (!to)
                plr->DropFactionReputation(from);
            else
                plr->SwapFactionReputation(from, to);
        } while (result->NextRow());
    }
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = objmgr.factionchange_reput_generic.begin(); it != objmgr.factionchange_reput_generic.end(); ++it) {
            uint32 faction_alliance = it->first;
            uint32 faction_horde = it->second;

            if (dest_team == BG_TEAM_ALLIANCE) {
                if (faction_alliance == 0)
                    plr->DropFactionReputation(faction_horde);
                else if (faction_horde == 0)
                    plr->DropFactionReputation(faction_alliance);
                else
                    plr->SwapFactionReputation(faction_alliance, faction_horde);
            }
            else {
                if (faction_horde == 0)
                    plr->DropFactionReputation(faction_alliance);
                else if (faction_alliance == 0)
                    plr->DropFactionReputation(faction_horde);
                else
                    plr->SwapFactionReputation(faction_horde, faction_alliance);
            }
        }
    }
    
    // Spells
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = objmgr.factionchange_spells.begin(); it != objmgr.factionchange_spells.end(); ++it) {
            uint32 spell_alliance = it->first;
            uint32 spell_horde = it->second;

            if (dest_team == BG_TEAM_ALLIANCE) {
                if (spell_alliance == 0) {
                    if (plr->HasSpell(spell_horde))
                        plr->removeSpell(spell_horde);
                } else {
                    if (plr->HasSpell(spell_horde)) {
                        plr->removeSpell(spell_horde);
                        plr->learnSpell(spell_alliance);
                    }
                }
            }
            else {
                if (spell_horde == 0) {
                    if (plr->HasSpell(spell_alliance))
                        plr->removeSpell(spell_alliance);
                } else {
                    if (plr->HasSpell(spell_alliance)) {
                        plr->removeSpell(spell_alliance);
                        plr->learnSpell(spell_horde);
                    }
                }
            }
        }
    }
    // Spells, race specific
    result = WorldDatabase.PQuery("SELECT spell1, spell2 FROM player_factionchange_spells_specific WHERE race1 = %u AND race2 = %u", m_race, t_race);
    if (result) {
        do {
            Field* fields = result->Fetch();
            
            uint32 from = fields[0].GetUInt32();
            uint32 to = fields[1].GetUInt32();

            if (to == 0) {
                if (plr->HasSpell(from))
                    plr->removeSpell(from);
            } else {
                if (plr->HasSpell(from)) {
                    plr->removeSpell(from);
                    plr->learnSpell(to);
                }
            }
        } while (result->NextRow());
    }
    result = WorldDatabase.PQuery("SELECT spell2, spell1 FROM player_factionchange_spells_specific WHERE race2 = %u AND race1 = %u", m_race, t_race);
    if (result) {
        do {
            Field* fields = result->Fetch();
            
            uint32 from = fields[0].GetUInt32();
            uint32 to = fields[1].GetUInt32();

            if (to == 0) {
                if (plr->HasSpell(from))
                    plr->removeSpell(from);
            } else {
                if (plr->HasSpell(from)) {
                    plr->removeSpell(from);
                    plr->learnSpell(to);
                }
            }
        } while (result->NextRow());
    }
    
    // Special case : Devouring Plague (Undead-only spell)
    if (plr->HasSpell(2944))
        plr->removeSpell(2944); // Remove rank 1, it should remove all 7 ranks
    
    // Items
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = objmgr.factionchange_items.begin(); it != objmgr.factionchange_items.end(); ++it) {
            uint32 item_alliance = it->first;
            uint32 item_horde = it->second;

            if (dest_team == BG_TEAM_ALLIANCE) {
                if (item_alliance == 0) {
                    uint32 count = plr->GetItemCount(item_horde, true);
                    if (count != 0)
                        plr->DestroyItemCount(item_horde, count, true, false, true);
                } else {
                    uint32 count = plr->GetItemCount(item_horde, true);
                    if (count != 0) {
                        plr->DestroyItemCount(item_horde, count, true, false, true);
                        ItemPosCountVec dest;
                        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item_alliance, count, false);
                        if (msg == EQUIP_ERR_OK)
                            plr->StoreNewItem(dest, item_alliance, count, true);
                        else {
                            if (Item* newItem = Item::CreateItem(item_alliance, count, plr)) {
                                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                                newItem->SaveToDB(trans);
                                CharacterDatabase.CommitTransaction(trans);

                                MailItemsInfo mi;
                                mi.AddItem(newItem->GetGUIDLow(), newItem->GetEntry(), newItem);
                                std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                                WorldSession::SendMailTo(plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUIDLow(), plr->GetGUIDLow(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
                            }
                        }
                    }
                }
            }
            else {
                if (item_horde == 0) {
                    uint32 count = plr->GetItemCount(item_alliance, true);
                    if (count != 0)
                        plr->DestroyItemCount(item_alliance, count, true, false, true);
                } else {
                    uint32 count = plr->GetItemCount(item_alliance, true);
                    if (count != 0) {
                        plr->DestroyItemCount(item_alliance, count, true, false, true);
                        ItemPosCountVec dest;
                        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item_horde, count, false);
                        if (msg == EQUIP_ERR_OK)
                            plr->StoreNewItem(dest, item_horde, count, true);
                        else {
                            if (Item* newItem = Item::CreateItem(item_horde, count, plr)) {
                                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                                newItem->SaveToDB(trans);
                                CharacterDatabase.CommitTransaction(trans);

                                MailItemsInfo mi;
                                mi.AddItem(newItem->GetGUIDLow(), newItem->GetEntry(), newItem);
                                std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                                WorldSession::SendMailTo(plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUIDLow(), plr->GetGUIDLow(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Items, race specific
    result = WorldDatabase.PQuery("SELECT item1, item2 FROM player_factionchange_items_race_specific WHERE race1 = %u AND race2 = %u", m_race, t_race);
    if (result) {
        do {
            Field* fields = result->Fetch();
            
            uint32 from = fields[0].GetUInt32();
            uint32 to = fields[1].GetUInt32();

            if (to == 0) {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0)
                    plr->DestroyItemCount(from, count, true, false, true);
            } else {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0) {
                    plr->DestroyItemCount(from, count, true, false, true);
                    ItemPosCountVec dest;
                    uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, to, count, false);
                    if (msg == EQUIP_ERR_OK)
                        plr->StoreNewItem( dest, to, count, true);
                    else {
                        if (Item* newItem = Item::CreateItem(to, count, plr)) {
                            SQLTransaction trans = CharacterDatabase.BeginTransaction();
                            newItem->SaveToDB(trans);
                            CharacterDatabase.CommitTransaction(trans);

                            MailItemsInfo mi;
                            mi.AddItem(newItem->GetGUIDLow(), newItem->GetEntry(), newItem);
                            std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                            WorldSession::SendMailTo(plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUIDLow(), plr->GetGUIDLow(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
                        }
                    }
                }
            }
        } while (result->NextRow());
    }
    result = WorldDatabase.PQuery("SELECT item2, item1 FROM player_factionchange_items_race_specific WHERE race2 = %u AND race1 = %u", m_race, t_race);
    if (result) {
        do {
            Field* fields = result->Fetch();
            
            uint32 from = fields[0].GetUInt32();
            uint32 to = fields[1].GetUInt32();

            if (to == 0) {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0)
                    plr->DestroyItemCount(from, count, true, false, true);
            } else {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0) {
                    plr->DestroyItemCount(from, count, true, false, true);
                    ItemPosCountVec dest;
                    uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, to, count, false);
                    if (msg == EQUIP_ERR_OK)
                        plr->StoreNewItem(dest, to, count, true);
                    else {
                        if (Item* newItem = Item::CreateItem(to, count, plr)) {
                            SQLTransaction trans = CharacterDatabase.BeginTransaction();
                            newItem->SaveToDB(trans);
                            CharacterDatabase.CommitTransaction(trans);

                            MailItemsInfo mi;
                            mi.AddItem(newItem->GetGUIDLow(), newItem->GetEntry(), newItem);
                            std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                            WorldSession::SendMailTo(plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUIDLow(), plr->GetGUIDLow(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
                        }
                    }
                }
            }
        } while (result->NextRow());
    }
    
    // Act like auctions are expired
    sAHMgr.RemoveAllAuctionsOf(plr->GetGUIDLow());
    plr->RemoveAllAuras();

    // Remove instance tag
    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++) {
        Player::BoundInstancesMap &binds = plr->GetBoundInstances(i);
        for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ) {
            if (itr->first != plr->GetMapId())
                plr->UnbindInstance(itr, i);
            else
                ++itr;
        }
    }

    if (m_session->GetSecurity() <= SEC_PLAYER) {
        LoginDatabase.PExecute("UPDATE account_credits SET amount = amount - %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", cost, time(NULL), account_id);
        CharacterDatabase.PExecute("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", plr->GetGUID(), "Changement de faction", time(NULL));
    }
    
    plr->SaveToDB();
    plr->m_kickatnextupdate = true;
    
    //***********************************************************************//
    //* BEYOND THIS LINE, ONLY STUFF THAT WILL NOT BE SAVED WITH SaveToDB() *//
    //***********************************************************************//
    
    // Quests
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = objmgr.factionchange_quests.begin(); it != objmgr.factionchange_quests.end(); ++it) {
            uint32 quest_alliance = it->first;
            uint32 quest_horde = it->second;

            if (dest_team == BG_TEAM_ALLIANCE) {
                if (quest_alliance == 0)
                    CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = %u AND quest = %u", plr->GetGUIDLow(), quest_horde);
                else
                    CharacterDatabase.PExecute("UPDATE character_queststatus SET quest = %u WHERE guid = %u AND quest = %u", quest_alliance, plr->GetGUIDLow(), quest_horde);
            }
            else {
                if (quest_horde == 0)
                    CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = %u AND quest = %u", plr->GetGUIDLow(), quest_alliance);
                else
                    CharacterDatabase.PExecute("UPDATE character_queststatus SET quest = %u WHERE guid = %u AND quest = %u", quest_horde, plr->GetGUIDLow(), quest_alliance);
            }
        }
    }

    // Friend list
    if (factionChange)
        CharacterDatabase.PExecute("DELETE FROM character_social WHERE guid = %u OR friend = %u", plr->GetGUIDLow(), plr->GetGUIDLow());

    // Relocation
    if (factionChange) {
        switch (t_race) {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_DRAENEI:
            // Stormwind
            Player::SavePositionInDB(0, -8866.468750f, 671.831238f, 97.903374f, 2.154216f, 1519, m_fullGUID);
            break;
        case RACE_ORC:
        case RACE_UNDEAD_PLAYER:
        case RACE_TAUREN:
        case RACE_TROLL:
        case RACE_BLOODELF:
            // Orgrimmar
            Player::SavePositionInDB(1, 1632.54f, -4440.77f, 15.4584f, 1.0637f, 1637, m_fullGUID);
            break;
        }
    }
    
    return true;
}

bool ChatHandler::HandleSpectateVersion(const char *args)
{
    if(!sWorld.getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
    {
        PSendSysMessage("Arena spectator désactivé.");
        return true;
    }

    if (!args || !*args)
        return false;

    std::string version = args;

    PSendSysMessage("Addon Spectator Version : %s", version.c_str());

    return true;
}

bool ChatHandler::HandleSpectateCancelCommand(const char* /*args*/)
{
    if(!sWorld.getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
    {
        PSendSysMessage("Arena spectator désactivé.");
        return true;
    }

    Player* player =  GetSession()->GetPlayer();

    BattleGround *bg = player->GetBattleGround();
    if (!bg)
    {
    	 PSendSysMessage("Vous n'êtes pas dans une arène.");
    	 SetSentErrorMessage(true);
    	 return false;
    }

    if (!bg->isSpectator(player->GetGUID()))
    {
        PSendSysMessage("Vous n'êtes pas spectateur.");
        SetSentErrorMessage(true);
        return false;
    }

    player->CancelSpectate();

    uint32 map = player->GetBattleGroundEntryPointMap();
    float positionX = player->GetBattleGroundEntryPointX();
    float positionY = player->GetBattleGroundEntryPointY();
    float positionZ = player->GetBattleGroundEntryPointZ();
    float positionO = player->GetBattleGroundEntryPointO();
    if (player->TeleportTo(map, positionX, positionY, positionZ, positionO))
    {
        player->SetSpectate(false);
        bg->RemoveSpectator(player->GetGUID());
    }

    return true;
}

bool ChatHandler::HandleSpectateFromCommand(const char *args)
{
    if(!sWorld.getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
    {
        PSendSysMessage("Arena spectator désactivé.");
        return true;
    }

    Player* target;
    uint64 target_guid;
    std::string target_name;
    if (!extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
        return false;

    Player* player = GetSession()->GetPlayer();

    if (!target)
    {
        PSendSysMessage("Le joueur ciblé est introuvable.");
        SetSentErrorMessage(true);
        return false;
    }

    if (!player->isSpectator())
    {
        PSendSysMessage("Vous n'êtes pas spectateur.");
        SetSentErrorMessage(true);
        return false;
    }

    if (target->isSpectator())
    {
        PSendSysMessage("Vous ne pouvez pas faire cela car le joueur ciblé est aussi spectateur.");
        SetSentErrorMessage(true);
        return false;
    }

    if (player->GetMap() != target->GetMap())
    {
        PSendSysMessage("Vous ne pouvez pas faire cela car vous n'êtes pas dans la même arène que le joueur ciblé.");
        SetSentErrorMessage(true);
        return false;
    }

    if (BattleGround* bg = target->GetBattleGround())
    {
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
        {
            PSendSysMessage("Vous ne pouvez pas faire cela car l'arène n'a pas encore commencé.");
            SetSentErrorMessage(true);
            return false;
        }
    }

    if (player->getSpectateFrom())
    {
        if (target == player->getSpectateFrom())
    	    player->getSpectateFrom()->RemovePlayerFromVision(player);
        else
        {
        	player->getSpectateFrom()->RemovePlayerFromVision(player);
        	target->AddPlayerToVision(player);
        }
        return true;
    }
    else
        target->AddPlayerToVision(player);

    return true;
}

bool ChatHandler::HandleSpectateInitCommand(const char *args)
{
    if(!sWorld.getConfig(CONFIG_ARENA_SPECTATOR_ENABLE))
        return true;

    if (Player* player = GetSession()->GetPlayer())
    	player->SendDataForSpectator();

    return true;
}

bool ChatHandler::HandleUpdateTitleCommand(const char *args)
{
    if (Player * player = GetSession()->GetPlayer()) {
        player->UpdateKnownTitles();
        return true;
    }
    return false;
}

bool ChatHandler::HandleReportLagCommand(const char* args)
{
    time_t now = time(NULL);
    Player* player = GetSession()->GetPlayer();
    if (now - player->lastLagReport > 10) { // Spam prevention
        sLog.outString("[LAG] Player %s (GUID: %u - IP: %s) reported lag - Current timediff: %u",
                player->GetName(), player->GetGUIDLow(), GetSession()->GetRemoteAddress().c_str(), sWorld.GetUpdateTime());
        player->lastLagReport = now;
    }

    return true;
}
