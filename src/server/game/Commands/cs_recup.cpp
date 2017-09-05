#include "Chat.h"
#include "Language.h"
#include "RecupMgr.h"

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
            PSendSysMessage(LANG_COMMAND_FACTION_NOREP_ERROR, factionEntry->name[GetSessionDbcLocale()], factionId);
            SetSentErrorMessage(true);
            return false;
        }

        player->SetFactionReputation(factionEntry, amount);
        PSendSysMessage(LANG_COMMAND_MODIFY_REP, factionEntry->name[GetSessionDbcLocale()], factionId, player->GetName().c_str(), player->GetReputation(factionId));
    }

    return true;
}

bool ChatHandler::HandleRecupCommand(const char* args)
{
    Player *player = m_session->GetPlayer();
    uint64 account_id = m_session->GetAccountId();
    uint32 pGUID = player->GetGUID();

    QueryResult query = CharacterDatabase.PQuery("SELECT classe,faction,metier1,metier1_level,metier2,metier2_level,reputs,phase,id,stuff,metier3,metier3_level,guid,stufflevel FROM recups WHERE account = %u AND active = 1 ORDER BY id DESC LIMIT 1", account_id);
    if (!query) {
        PSendSysMessage(LANG_NO_RECUP_AVAILABLE);
        SetSentErrorMessage(true);
        return false;
    }

    Field *fields = query->Fetch();
    uint32 classe, faction, metier1, metier1_level, metier2, metier2_level, phase, recupID, stuff, metier3, metier3_level;
    //uint32 recupguid;
    uint32 stufflevel;
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
    //recupguid = fields[12].GetUInt64();
    stufflevel = fields[13].GetUInt32();

    /* additionnal checks */

    if ((player->GetTeam() == ALLIANCE && faction != 1) || (player->GetTeam() == HORDE && faction != 2) || player->GetClass() != classe || phase < 1 || phase > 2) {
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

    /* Now free
    query = LoginDatabase.PQuery("SELECT amount FROM account_credits WHERE id = %u", account_id);

    if (!query) {
        PSendSysMessage(LANG_NO_CREDIT_EVER);
        SetSentErrorMessage(true);
        return false;
    }

    fields = query->Fetch();
    int credits_count = fields[0].GetUInt32();

    if (credits_count < 2) {
        PSendSysMessage(LANG_CREDIT_NOT_ENOUGH);
        SetSentErrorMessage(true);
        return false;
    }
    */
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

    query = WorldDatabase.PQuery("SELECT command FROM recups_data WHERE classe = %u AND (faction = %u OR faction = 0) AND stufflevel = %u AND phase = 2 AND (stuff = %u OR stuff = -1)", classe, faction, stufflevel, stuff);
    if (!query) {
        PSendSysMessage(LANG_RECUP_CORRUPT);
        SetSentErrorMessage(true);
        return false;
    }

    //LoginDatabase.PExecute("UPDATE account_credits SET amount = amount-2 WHERE id = %u", account_id);

    player->GiveLevel(70);
    player->SetUInt32Value(PLAYER_XP, 0);
    PSendSysMessage(LANG_YOU_CHANGE_LVL, player->GetName().c_str(), 70);

    if (!RecupMgr::HandleRecupParseCommand(player, command, 0, true, this)) {
        PSendSysMessage(LANG_RECUP_CMD_FAILED);
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE recups SET phase=2 WHERE id = %u", recupID);
    CharacterDatabase.PExecute("UPDATE recups SET guid=%lu WHERE id=%u", pGUID, recupID);
    //PSendSysMessage(LANG_RECUP_PHASE1_SUCCESS);

    /* first, add all stuff items (set, offset, weapons, etc) */

    do {
        fields = query->Fetch();
        std::string command_p2 = fields[0].GetString();

        if (!RecupMgr::HandleRecupParseCommand(player, command_p2, 0, false, this)) {
            PSendSysMessage(LANG_RECUP_CMD_FAILED);
            SetSentErrorMessage(true);
            return false;
        }
    } while (query->NextRow());

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

        if (!RecupMgr::HandleRecupParseCommand(player, command, metier1_level, false, this)) {
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

        if (!RecupMgr::HandleRecupParseCommand(player, command, metier2_level, false, this)) {
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

        if (!RecupMgr::HandleRecupParseCommand(player, command, metier3_level, false, this)) {
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

    if (!RecupMgr::HandleRecupParseCommand(player, command, 0, false, this)) {
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

    if (!RecupMgr::HandleRecupParseCommand(player, command, 0, false, this)) {
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
        PSendSysMessage(LANG_IS_TELEPORTED, player->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_TELEPORTING_TO, player->GetName().c_str(),"", "Shattrath");

    player->TeleportTo(530, -1838.160034, 5301.790039, -12.428000, 5.951700);

    /* at this point, recup is completed */

    CharacterDatabase.PExecute("UPDATE recups SET active=0 WHERE id = %u", recupID);
    PSendSysMessage(LANG_RECUP_PHASE2_SUCCESS);

    std::string subject = "Welcome!";
    uint32 itemTextId = sObjectMgr->CreateItemText("Bienvenue sur Sunstrider !\n\nVous, qui avez récemment récupéré un personnage sur le serveur, êtes peut-être à la recherche d'une guilde. Si c'est le cas, consultez la section Générale du forum du serveur (forums.sunstrider.fr) : un topic épinglé liste les différentes guildes intéressées par de nouveaux arrivants comme vous. Par ailleurs en cas de question, n'hésitez pas à nous contacter via ce même topic, ou par MP à un membre du staff.\n\nCordialement,\n\nL'équipe Sunstrider.");

    WorldSession::SendMailTo(player, MAIL_NORMAL, MAIL_STATIONERY_GM, 0, player->GetGUIDLow(), subject, itemTextId, nullptr, 0, 0, MAIL_CHECK_MASK_NONE);

    player->SaveToDB();
    return true;
}
