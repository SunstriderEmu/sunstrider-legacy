#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "PlayerDump.h"
#include "AuctionHouseMgr.h"
#include "BattleGround.h"

bool ChatHandler::HandleViewCreditsCommand(const char *args)
{
    uint64 account_id = m_session->GetAccountId();

    QueryResult query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!query)
    {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
    }
    else 
    {
        Field *field = query->Fetch();
        uint32 credits = field[0].GetUInt32();

        PSendSysMessage(LANG_CREDIT_INFO, credits);
    }

    return true;
}

bool ChatHandler::HandleBuyInShopCommand(const char *args)
{
    ARGS_CHECK

    Player *player = m_session->GetPlayer();
    uint64 account_id = m_session->GetAccountId();
    QueryResult query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!query) {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
        return false;
    }

    int plevel = player->GetLevel();
    Field *fields = query->Fetch();
    uint32 credits = fields[0].GetUInt32();

    std::string safe_args = args;
    WorldDatabase.EscapeString(safe_args);

    query = WorldDatabase.PQuery("SELECT actions, cost, name FROM shop_orders WHERE name = '%s' AND cost <= %u AND (class = %u OR class = 0) AND (level_min <= %u OR level_min = 0) AND (level_max >= %u OR level_max = 0) AND (race = %u OR race = 0) ORDER BY level_min DESC LIMIT 1", safe_args.c_str(), credits, player->GetClass(), plevel, plevel, player->GetRace());

    if (!query) 
    {
        PSendSysMessage("Unknown command, unsufficient credits or incorrect level. Make sure you are using the right command.");
        SetSentErrorMessage(true);
        return false;
    }

    fields = query->Fetch();
    std::string script = fields[0].GetString();
    std::string actions = script;
    const char* buyName = fields[2].GetCString();
    uint32 cost = fields[1].GetUInt32();
    bool can_take_credits = true;
    
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
        PSendSysMessage("You must have %u free slots in your inventory to perform this command.", requiredSlots);
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
            //GameTele const* tele = extractGameTeleFromLink(telename);

            /*if (!tele) {
                PSendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
                SetSentErrorMessage(true);
                continue;
            }

            MapEntry const* me = sMapStore.LookupEntry(tele->mapId);
            if (!me || me->IsBattlegroundOrArena()) {
                PSendSysMessage(LANG_CANNOT_TELE_TO_BG);
                SetSentErrorMessage(true);
                return false;
            }

            if (player->IsBeingTeleported() == true) {
                PSendSysMessage(LANG_IS_TELEPORTED, player->GetName().c_str());
                SetSentErrorMessage(true);
                return false;
            }*/

            PSendSysMessage(LANG_TELEPORTING_TO, player->GetName().c_str(),"", "Shattrath");

            if (player->IsInFlight()) {
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
                    PSendSysMessage(LANG_RENAME_PLAYER, player->GetName().c_str());
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

                PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName().c_str(), level);

                can_take_credits = true;
            }
        } else if (v[0] == "learn") {
            /* learn, v[1] == spell ID */
            uint32 spell = atol(v[1].c_str());
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer())) {
                PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
                SetSentErrorMessage(true);
                return false;
            }

            if (!player->HasSpell(spell))
                player->LearnSpell(spell, false);
        } else if (v[0] == "add") {
            if (v[1] == "level") {
                int add_levels = atoi(v[2].c_str());
                int level = player->GetLevel() + add_levels;

                if (level > 70) level = 70;
                if (level < 1) level = 1;

                player->GiveLevel(level);

                int maxskill = atoi(v[3].c_str());
                if (maxskill == 1)
                    player->UpdateSkillsToMaxSkillsForLevel();

                int reset = atoi(v[4].c_str());
                if (reset == 1)
                    player->SetUInt32Value(PLAYER_XP, 0);

                PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName().c_str(), level);
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
                    QueryResult query2 = WorldDatabase.PQuery("SELECT item_id, count FROM shop_items_random WHERE category = %u ORDER BY RAND() LIMIT 1", atoi(v[3].c_str()));

                    if (query2)
                    {
                        Field *field = query2->Fetch();
                        itemId = field[0].GetUInt32();
                        count = field[1].GetUInt32();
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

                ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemId);
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

                QueryResult result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE itemset = %u", itemsetId);

                if (!result) {
                    PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
                    SetSentErrorMessage(true);
                    return false;
                }

                do {
                    Field* fields2 = result->Fetch();
                    uint32 itemId = fields2[0].GetUInt32();

                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
                    if (msg == EQUIP_ERR_OK) 
                    {
                        Item *item = player->StoreNewItem(dest, itemId, true);
                        player->SendNewItem(item, 1, true, true);
                    }
                    else 
                    {
                        player->SendEquipError(msg, nullptr, nullptr);
                        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, 1);
                    }
                } while (result->NextRow() || error);

                can_take_credits = true;
            }
        }
    }

    if (can_take_credits == true) {
        LoginDatabase.PExecute("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", credits - cost, time(nullptr), account_id);
     //   CharacterDatabase.PExecute("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", player->GetGUID(), actions.c_str(), time(NULL));
        player->SaveToDB();

        return true;
    } else {
        TC_LOG_INFO("command", "Shop error - actions : %s - GUID : %u price : %u - hour : %u", actions.c_str(), player->GetGUID().GetCounter(), cost, uint32(time(nullptr)));
        return false;
    }
}

bool ChatHandler::HandleReskinCommand(const char* args)
{
    ARGS_CHECK
        
    char* targetName = strtok((char*)args, "");
    std::string safeTargetName = targetName;
    CharacterDatabase.EscapeString(safeTargetName);
    uint64 account_id = m_session->GetAccountId();
    QueryResult result = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!result) {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
        return false;
    }

    Field *fields = result->Fetch();
    uint32 credits = fields[0].GetUInt32();

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
    uint32 t_race = fields[2].GetUInt8();
    uint8 t_gender = fields[3].GetUInt8();
    uint32 t_playerBytes = fields[4].GetUInt32();
    uint32 t_playerBytes2 = fields[5].GetUInt32();
    
    uint32 m_race = m_session->GetPlayer()->GetRace();
    
    if (t_race != m_race || t_guid == m_session->GetPlayer()->GetGUID().GetCounter() || t_account != m_session->GetAccountId())
        return false;

    uint32 bankBags = m_session->GetPlayer()->GetByteValue(PLAYER_BYTES_2, 2);

    m_session->GetPlayer()->SetUInt32Value(PLAYER_BYTES, t_playerBytes);
    m_session->GetPlayer()->SetUInt32Value(PLAYER_BYTES_2, t_playerBytes2);
    m_session->GetPlayer()->SetByteValue(PLAYER_BYTES_2, 2, bankBags);
    m_session->GetPlayer()->SetGender(t_gender);

    LoginDatabase.PExecute("UPDATE account_credits SET amount = %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", credits - 1, time(nullptr), account_id);
   // CharacterDatabase.PExecute("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", m_session->GetPlayer()->GetGUID(), "reskin", time(NULL));

    m_session->GetPlayer()->SaveToDB();
//    m_session->KickPlayer();
    m_session->GetPlayer()->m_kickatnextupdate = true;
    
    sCharacterCache->UpdateCharacterData(m_session->GetPlayer()->GetGUID(), PLAYER_UPDATE_DATA_GENDER, "", 0, m_session->GetPlayer()->GetGender());

    return true;
}

bool ChatHandler::HandleRaceOrFactionChange(const char* args)
{
    ARGS_CHECK
        
    char* targetName = strtok((char*)args, " ");
    char* cForce  = strtok (nullptr, " "); //skip same account check (for players that already have max characters count on their account)
    std::string safeTargetName = targetName;
    CharacterDatabase.EscapeString(safeTargetName);
    uint64 account_id = m_session->GetAccountId();
    QueryResult result = nullptr;
    Field* fields = nullptr;
    bool force = (cForce && strcmp(cForce,"force") == 0);
    
    if (!sWorld->getConfig(CONFIG_FACTION_CHANGE_ENABLED)) {
        //TODO translate
        PSendSysMessage("Le changement de race/faction est actuellement désactivé.");
        //PSendSysMessage("Race/Faction change is deactivated.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (!m_session->GetPlayer()->IsAlive()) {
        //TODO translate
        PSendSysMessage("Vous devez être en vie pour effectuer un changement de race ou faction.");
        //PSendSysMessage("You must be alive to perform race or faction change.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->IsInCombat()) {
        //TODO translate
        PSendSysMessage("Impossible en combat.");
        //SendSysMessage("Cannot do this in combat.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->GetBattleground()) {
        //TODO translate
        PSendSysMessage("Impossible en champ de bataille ou en arène.");
        //PSendSysMessage("Impossible in battleground or arena.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->GetGroup()) {
        //TODO translate
        PSendSysMessage("Veuillez quitter votre groupe pour effectuer le changement.");
        //PSendSysMessage("Please leave your group to perform the change.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (m_session->GetPlayer()->GetInstanceId() != 0) {
        //TODO translate
        PSendSysMessage("Impossible en instance.");
        //PSendSysMessage("Impossible in instance.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (!force && m_session->GetPlayer()->GetLevel() < 10) {
        PSendSysMessage(LANG_FACTIONCHANGE_LEVEL_MIN);
        SetSentErrorMessage(true);
        return false;
    }

    result = CharacterDatabase.PQuery("SELECT guid, account, race, gender, playerBytes, playerBytes2 FROM characters WHERE name = '%s'", safeTargetName.c_str());
    
    if (!result) {
        //TODO translate
        PSendSysMessage("Personnage cible non trouvé.");
        //PSendSysMessage("Targeted character not found.");
        SetSentErrorMessage(true);
        return false;
    }
    
    fields = result->Fetch();
    
    Player* plr = m_session->GetPlayer();
    
    // My values
    uint32 m_guid = plr->GetGUID().GetCounter();
    uint32 m_account = m_session->GetAccountId();
    uint32 m_class = plr->GetClass();
    uint32 m_race = plr->GetRace();
//    uint8 m_gender = plr->GetGender();
    ObjectGuid m_fullGUID = plr->GetGUID();
    
    // Target values
    uint32 t_guid = fields[0].GetUInt32();
    uint32 t_account = fields[1].GetUInt32();
    uint32 t_race = fields[2].GetUInt32();
    uint8 t_gender = fields[3].GetUInt32();
    uint32 t_playerBytes = fields[4].GetUInt32();
    uint32 t_playerBytes2 = fields[5].GetUInt32();
    
    if (m_guid == t_guid) {
        //TODO translate
        PSendSysMessage("Vous avez essayé de lancer un changement sur vous-même. Merci d'aller lire le post explicatif sur le forum !");
        //PSendSysMessage("You tried to perform a change on yourself. Please read the dedicated post on the forums.");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (!force && m_account != t_account) {
        //TODO translate
        PSendSysMessage("Le personnage modèle doit être présent sur votre compte.");
        //PSendSysMessage("The model character must be on your account.");
        SetSentErrorMessage(true);
        return false;
    }
    
    uint32 dest_team = TEAM_ALLIANCE;
    // Search each faction is targeted
    switch (t_race)
    {
        case RACE_ORC:
        case RACE_TAUREN:
        case RACE_UNDEAD_PLAYER:
        case RACE_TROLL:
        case RACE_BLOODELF:
            dest_team = TEAM_HORDE;
            break;
        default:
            break;
    }

    PlayerInfo const* targetInfo = sObjectMgr->GetPlayerInfo(t_race, m_class);
    if (!targetInfo) {
        //TODO translate
        PSendSysMessage("La race du personnage cible est incompatible avec votre classe.");
        //PSendSysMessage("The targeted character's race is incompatible with your class.");
        SetSentErrorMessage(true);
        return false;
    }
    
    // Check if this transfer is currently allowed and get cost
    PlayerInfo const* myInfo = sObjectMgr->GetPlayerInfo(m_race, m_class);
    bool factionChange = (Player::TeamForRace(m_race) != Player::TeamForRace(t_race));
    uint32 cost = sWorld->getConfig(CONFIG_RACE_CHANGE_COST);
    if (factionChange) {
        if(dest_team == TEAM_HORDE)
        {
            if(!sWorld->getConfig(CONFIG_FACTION_CHANGE_A2H)) 
            {
                //TODO translate
                PSendSysMessage("Le changement de faction n'est actuellement pas autorisé dans le sens Alliance -> Horde.");
                //PSendSysMessage("Faction change from Alliance to Horde is deactivated.");
                SetSentErrorMessage(true);
                return false;
            }
            cost = sWorld->getConfig(CONFIG_FACTION_CHANGE_A2H_COST);
        } else if (dest_team == TEAM_ALLIANCE) 
        {
            if (!sWorld->getConfig(CONFIG_FACTION_CHANGE_H2A)) 
            {
                //TODO translate
                PSendSysMessage("Le changement de faction n'est actuellement pas autorisé dans le sens Horde -> Alliance.");
                //PSendSysMessage("Faction change from Horde to Alliance is deactivated.");
                SetSentErrorMessage(true);
                return false;
            }
            cost = sWorld->getConfig(CONFIG_FACTION_CHANGE_H2A_COST);
        }
    }
    
    // Check if enough credits
    if (cost && m_session->GetSecurity() <= SEC_PLAYER) {
        result = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

        if (!result) {
            PSendSysMessage(LANG_NO_CREDIT_EVER);
            SetSentErrorMessage(true);
            return false;
        }

        fields = result->Fetch();
        uint32 credits = fields[0].GetUInt32();

        if (credits < cost) {
            PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
            SetSentErrorMessage(true);
            return false;
        }
    }
    
    // Check guild and arena team, friends are removed after the SaveToDB() call
    // Guild
    if (factionChange) {
        Guild* guild = sObjectMgr->GetGuildById(plr->GetGuildId());
        if (guild) {
            //TODO translate
            PSendSysMessage("Vous êtes actuellement dans une guilde. Veuillez la quitter pour effectuer le changement de faction.");
            //PSendSysMessage("You must leave your guild to perform faction change.");
            SetSentErrorMessage(true);
            return false;
        }
    }
    
    // Arena teams
    if (factionChange) {
        result = CharacterDatabase.PQuery("SELECT arena_team_member.arenaTeamId FROM arena_team_member JOIN arena_team ON arena_team_member.arenaTeamId = arena_team.arenaTeamId WHERE guid = %u", plr->GetGUID().GetCounter());

        if (result) {
            //TODO translate
            PSendSysMessage("Vous êtes actuellement dans une ou plusieurs équipes d'arène. Veuillez les quitter pour effectuer le changement de faction.");
            //PSendSysMessage("You must leave your arena team(s) to perform faction change.");
            SetSentErrorMessage(true);
            return false;
        }
    }
    
    // Dump player by safety
    std::ostringstream oss;
    std::string fname = sConfigMgr->GetStringDefault("LogsDir", "");
    oss << fname;
    if (fname.length() > 0 && fname.at(fname.length()-1) != '/')
        oss << "/";
    oss << "chardump_factionchange/" << account_id << "_" << m_session->GetPlayer()->GetGUID().GetCounter() << "_" << m_session->GetPlayer()->GetName();
    PlayerDumpWriter().WriteDump(oss.str().c_str(), m_session->GetPlayer()->GetGUID().GetCounter());
    
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
        plr->SetHomebind(loc, area_id);
    
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
            plr->RemoveSpell(tspell,false);
    }
    // Add new race starting spells
    for (spell_itr = targetInfo->spell.begin(); spell_itr != targetInfo->spell.end(); ++spell_itr) {
        uint16 tspell = spell_itr->first;
        if (tspell) {
            if (!spell_itr->second)               // don't care about passive spells or loading case
                plr->AddSpell(tspell,spell_itr->second);
            else                                            // but send in normal spell in game learn case
                plr->LearnSpell(tspell, false);
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
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->factionchange_titles.begin(); it != sObjectMgr->factionchange_titles.end(); ++it) {
            CharTitlesEntry const* title_alliance = sCharTitlesStore.LookupEntry(it->first);
            CharTitlesEntry const* title_horde = sCharTitlesStore.LookupEntry(it->second);
            
            if (!title_alliance || !title_horde)
                continue;

            if (dest_team == TEAM_ALLIANCE) {
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
            Field* _fields = result->Fetch();
            
            uint32 from = _fields[0].GetUInt32();
            uint32 to = _fields[1].GetUInt32();
            
            if (!from)
                continue;
            
            if (!to)
                plr->DropFactionReputation(from);
            else
                plr->SwapFactionReputation(from, to);
        } while (result->NextRow());
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->factionchange_reput_generic.begin(); it != sObjectMgr->factionchange_reput_generic.end(); ++it) {
            uint32 faction_alliance = it->first;
            uint32 faction_horde = it->second;

            if (dest_team == TEAM_ALLIANCE) {
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
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->factionchange_spells.begin(); it != sObjectMgr->factionchange_spells.end(); ++it) {
            uint32 spell_alliance = it->first;
            uint32 spell_horde = it->second;

            if (dest_team == TEAM_ALLIANCE) {
                if (spell_alliance == 0) {
                    if (plr->HasSpell(spell_horde))
                        plr->RemoveSpell(spell_horde);
                } else {
                    if (plr->HasSpell(spell_horde)) {
                        plr->RemoveSpell(spell_horde);
                        plr->LearnSpell(spell_alliance, false);
                    }
                }
            }
            else {
                if (spell_horde == 0) {
                    if (plr->HasSpell(spell_alliance))
                        plr->RemoveSpell(spell_alliance);
                } else {
                    if (plr->HasSpell(spell_alliance)) {
                        plr->RemoveSpell(spell_alliance);
                        plr->LearnSpell(spell_horde, false);
                    }
                }
            }
        }
    }

    
    // Spells, priest specific
    if(plr->GetClass() == CLASS_PRIEST)
    {
        result = WorldDatabase.PQuery("SELECT spell1, spell2 FROM player_factionchange_spells_priest_specific WHERE race1 IN (0,%u) AND race2 IN (0,%u) ORDER BY race1,race2", m_race, t_race); //order by is here to handle non race specific spells first
        if (result) {
            do {
                Field* _fields = result->Fetch();
            
                uint32 from = _fields[0].GetUInt32();
                uint32 to = _fields[1].GetUInt32();

                if (plr->HasSpell(from))
                    plr->RemoveSpell(from);

                if (to != 0)
                    plr->LearnSpell(to, false);

            } while (result->NextRow());
        }
    }

    // Items
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->factionchange_items.begin(); it != sObjectMgr->factionchange_items.end(); ++it) {
            uint32 item_alliance = it->first;
            uint32 item_horde = it->second;

            if (dest_team == TEAM_ALLIANCE) {
                if (item_alliance == 0) {
                    uint32 count = plr->GetItemCount(item_horde, true);
                    if (count != 0)
                        plr->DestroyItemCount(item_horde, count, true, false, true);
                } else {
                    uint32 count = plr->GetItemCount(item_horde, true);
                    if (count != 0) {
                        plr->DestroyItemCount(item_horde, count, true, false, true);
                        ItemPosCountVec dest;
                        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item_alliance, count);
                        if (msg == EQUIP_ERR_OK)
                            plr->StoreNewItem(dest, item_alliance, count, true);
                        else {
                            if (Item* newItem = Item::CreateItem(item_alliance, count, plr)) {
                                newItem->SaveToDB(trans);

                                MailItemsInfo mi;
                                mi.AddItem(newItem->GetGUID().GetCounter(), newItem->GetEntry(), newItem);
                                std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                                WorldSession::SendMailTo(trans, plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUID().GetCounter(), plr->GetGUID().GetCounter(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
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
                        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item_horde, count);
                        if (msg == EQUIP_ERR_OK)
                            plr->StoreNewItem(dest, item_horde, count, true);
                        else {
                            if (Item* newItem = Item::CreateItem(item_horde, count, plr)) {
                                newItem->SaveToDB(trans);

                                MailItemsInfo mi;
                                mi.AddItem(newItem->GetGUID().GetCounter(), newItem->GetEntry(), newItem);
                                std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                                WorldSession::SendMailTo(trans, plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUID().GetCounter(), plr->GetGUID().GetCounter(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
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
            Field* _fields = result->Fetch();
            
            uint32 from = _fields[0].GetUInt32();
            uint32 to = _fields[1].GetUInt32();

            if (to == 0) {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0)
                    plr->DestroyItemCount(from, count, true, false, true);
            } else {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0) {
                    plr->DestroyItemCount(from, count, true, false, true);
                    ItemPosCountVec dest;
                    uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, to, count);
                    if (msg == EQUIP_ERR_OK)
                        plr->StoreNewItem( dest, to, count, true);
                    else {
                        if (Item* newItem = Item::CreateItem(to, count, plr)) {
                            SQLTransaction trans_ = CharacterDatabase.BeginTransaction();
                            newItem->SaveToDB(trans_);
                            CharacterDatabase.CommitTransaction(trans_);

                            MailItemsInfo mi;
                            mi.AddItem(newItem->GetGUID().GetCounter(), newItem->GetEntry(), newItem);
                            std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                            WorldSession::SendMailTo(plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUID().GetCounter(), plr->GetGUID().GetCounter(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
                        }
                    }
                }
            }
        } while (result->NextRow());
    }

    result = WorldDatabase.PQuery("SELECT item2, item1 FROM player_factionchange_items_race_specific WHERE race2 = %u AND race1 = %u", m_race, t_race);
    if (result) {
        do {
            Field* _fields = result->Fetch();
            
            uint32 from = _fields[0].GetUInt32();
            uint32 to = _fields[1].GetUInt32();

            if (to == 0) {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0)
                    plr->DestroyItemCount(from, count, true, false, true);
            } else {
                uint32 count = plr->GetItemCount(from, true);
                if (count != 0) {
                    plr->DestroyItemCount(from, count, true, false, true);
                    ItemPosCountVec dest;
                    uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, to, count);
                    if (msg == EQUIP_ERR_OK)
                        plr->StoreNewItem(dest, to, count, true);
                    else {
                        if (Item* newItem = Item::CreateItem(to, count, plr)) {
                            newItem->SaveToDB(trans);

                            MailItemsInfo mi;
                            mi.AddItem(newItem->GetGUID().GetCounter(), newItem->GetEntry(), newItem);
                            std::string subject = GetSession()->GetTrinityString(LANG_NOT_EQUIPPED_ITEM);
                            WorldSession::SendMailTo(trans, plr, MAIL_NORMAL, MAIL_STATIONERY_GM, plr->GetGUID().GetCounter(), plr->GetGUID().GetCounter(), subject, 0, &mi, 0, 0, MAIL_CHECK_MASK_NONE);
                        }
                    }
                }
            }
        } while (result->NextRow());
    }
    
    // Act like auctions are expired
    sAuctionMgr->RemoveAllAuctionsOf(trans, plr->GetGUID().GetCounter());
    plr->RemoveAllAuras();

    // Remove instance tag
    for (uint8 i = 0; i < MAX_DIFFICULTY; i++) {
        Player::BoundInstancesMap &binds = plr->GetBoundInstances(Difficulty(i));
        for (auto itr = binds.begin(); itr != binds.end(); ) {
            if (itr->first != plr->GetMapId())
                plr->UnbindInstance(itr, Difficulty(i));
            else
                ++itr;
        }
    }

    if (m_session->GetSecurity() <= SEC_PLAYER) {
        LoginDatabase.PExecute("UPDATE account_credits SET amount = amount - %u, last_update = %u, `from` = 'Boutique' WHERE id = %u", cost, time(nullptr), account_id);
      //  trans->PAppend("INSERT INTO character_purchases (guid, actions, time) VALUES (%u, '%s', %u)", plr->GetGUID(), "Changement de faction", time(NULL));
    }

    CharacterDatabase.CommitTransaction(trans);

    plr->SaveToDB();
    plr->m_kickatnextupdate = true;
    
    sCharacterCache->UpdateCharacterData(plr->GetGUID(), PLAYER_UPDATE_DATA_RACE | PLAYER_UPDATE_DATA_GENDER, plr->GetName(), 0, plr->GetGender(), plr->GetRace());

    //***********************************************************************//
    //* BEYOND THIS LINE, ONLY STUFF THAT WILL NOT BE SAVED WITH SaveToDB() *//
    //***********************************************************************//
    
    // Quests
    if (factionChange) {
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->factionchange_quests.begin(); it != sObjectMgr->factionchange_quests.end(); ++it) {
            uint32 quest_alliance = it->first;
            uint32 quest_horde = it->second;

            if (dest_team == TEAM_ALLIANCE) {
                if (quest_alliance == 0)
                    CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = %u AND quest = %u", plr->GetGUID().GetCounter(), quest_horde);
                else
                    CharacterDatabase.PExecute("UPDATE character_queststatus SET quest = %u WHERE guid = %u AND quest = %u", quest_alliance, plr->GetGUID().GetCounter(), quest_horde);
            }
            else {
                if (quest_horde == 0)
                    CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = %u AND quest = %u", plr->GetGUID().GetCounter(), quest_alliance);
                else
                    CharacterDatabase.PExecute("UPDATE character_queststatus SET quest = %u WHERE guid = %u AND quest = %u", quest_horde, plr->GetGUID().GetCounter(), quest_alliance);
            }
        }
    }

    // Friend list
    if (factionChange)
        CharacterDatabase.PExecute("DELETE FROM character_social WHERE guid = %u OR friend = %u", plr->GetGUID().GetCounter(), plr->GetGUID().GetCounter());

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
