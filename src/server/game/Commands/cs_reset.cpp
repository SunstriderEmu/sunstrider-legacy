#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"

static bool HandleResetStatsOrLevelHelper(Player* player)
{
    PlayerInfo const *info = sObjectMgr->GetPlayerInfo(player->GetRace(), player->GetClass());
    if (!info) return false;

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->GetClass());
    if (!cEntry)
    {
        TC_LOG_ERROR("command", "Class %u not found in DBC (Wrong DBC files?)", player->GetClass());
        return false;
    }

    uint8 powertype = cEntry->PowerType;

    uint32 unitfield;
    if (powertype == POWER_RAGE)
        unitfield = 0x1100EE00;
    else if (powertype == POWER_ENERGY)
        unitfield = 0x00000000;
    else if (powertype == POWER_MANA)
        unitfield = 0x0000EE00;
    else
    {
        TC_LOG_ERROR("command", "Invalid default powertype %u for player (class %u)", powertype, player->GetClass());
        return false;
    }

    // reset m_form if no aura
    if (!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        player->m_form = FORM_NONE;

    player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_PLAYER_BOUNDING_RADIUS);
    player->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_PLAYER_COMBAT_REACH);

    player->SetFactionForRace(player->GetRace());

    player->SetUInt32Value(UNIT_FIELD_BYTES_0, ((player->GetRace()) | (player->GetClass() << 8) | (player->GetGender() << 16) | (powertype << 24)));

    // reset only if player not in some form;
    if (player->m_form == FORM_NONE)
    {
        switch (player->GetGender())
        {
        case GENDER_FEMALE:
            player->SetDisplayId(info->displayId_f);
            player->SetNativeDisplayId(info->displayId_f);
            break;
        case GENDER_MALE:
            player->SetDisplayId(info->displayId_m);
            player->SetNativeDisplayId(info->displayId_m);
            break;
        default:
            break;
        }
    }

    // set UNIT_FIELD_BYTES_1 to init state but preserve m_form value
    player->SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield);
    player->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_1_UNK, UNIT_BYTE2_FLAG_UNK3 | UNIT_BYTE2_FLAG_UNK5);
    player->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM, player->m_form);

    player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

    //-1 is default value
    player->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    //player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );
    return true;
}

bool ChatHandler::HandleResetHonorCommand (const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = nullptr;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
        player = sObjectMgr->GetPlayer(guid);
    }
    else
        player = GetSelectedPlayerOrSelf();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    player->SetUInt32Value(PLAYER_FIELD_KILLS, 0);
    player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
    player->SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, 0);
    player->SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
    player->SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);

    return true;
}

bool ChatHandler::HandleResetLevelCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = nullptr;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
        player = sObjectMgr->GetPlayer(guid);
    }
    else
        player = GetSelectedPlayerOrSelf();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(!HandleResetStatsOrLevelHelper(player))
        return false;

    player->SetLevel(1);
    player->InitStatsForLevel(true);
    player->InitTaxiNodesForLevel();
    player->InitTalentForLevel();
    player->SetUInt32Value(PLAYER_XP,0);

    // reset level to summoned pet
    Pet* pet = player->GetPet();
    if(pet && pet->getPetType()==SUMMON_PET)
        pet->InitStatsForLevel(1);

    return true;
}

bool ChatHandler::HandleResetStatsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = nullptr;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
        player = sObjectMgr->GetPlayer(guid);
    }
    else
        player = GetSelectedPlayerOrSelf();

    if(!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(!HandleResetStatsOrLevelHelper(player))
        return false;

    player->InitStatsForLevel(true);
    player->InitTaxiNodesForLevel();
    player->InitTalentForLevel();

    return true;
}

bool ChatHandler::HandleResetSpellsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = nullptr;
    uint64 playerGUID = 0;
    if (pName)
    {
        std::string name = pName;

        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
        if(!player)
            playerGUID = sCharacterCache->GetCharacterGuidByName(name.c_str());
    }
    else
        player = GetSelectedPlayerOrSelf();

    if(!player && !playerGUID)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(player)
    {
        player->resetSpells();

        ChatHandler(player).SendSysMessage(LANG_RESET_SPELLS);

        if(m_session->GetPlayer()!=player)
            PSendSysMessage(LANG_RESET_SPELLS_ONLINE,player->GetName().c_str());
    }
    else
    {
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'",uint32(AT_LOGIN_RESET_SPELLS), GUID_LOPART(playerGUID));
        PSendSysMessage(LANG_RESET_SPELLS_OFFLINE,pName);
    }

    return true;
}

bool ChatHandler::HandleResetTalentsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = nullptr;
    uint64 playerGUID = 0;
    if (pName)
    {
        std::string name = pName;
        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
        if(!player)
            playerGUID = sCharacterCache->GetCharacterGuidByName(name.c_str());
    }
    else
        player = GetSelectedPlayerOrSelf();

    if(!player && !playerGUID)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(player)
    {
        player->ResetTalents(true);

        ChatHandler(player).SendSysMessage(LANG_RESET_TALENTS);

        if(m_session->GetPlayer()!=player)
            PSendSysMessage(LANG_RESET_TALENTS_ONLINE,player->GetName().c_str());
    }
    else
    {
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'",uint32(AT_LOGIN_RESET_TALENTS), GUID_LOPART(playerGUID) );
        PSendSysMessage(LANG_RESET_TALENTS_OFFLINE,pName);
    }

    return true;
}

bool ChatHandler::HandleResetAllCommand(const char * args)
{
    ARGS_CHECK

    std::string casename = args;

    AtLoginFlags atLogin;

    // Command specially created as single command to prevent using short case names
    if(casename=="spells")
    {
        atLogin = AT_LOGIN_RESET_SPELLS;
        sWorld->SendWorldText(LANG_RESETALL_SPELLS);
    }
    else if(casename=="talents")
    {
        atLogin = AT_LOGIN_RESET_TALENTS;
        sWorld->SendWorldText(LANG_RESETALL_TALENTS);
    }
    else
    {
        PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE,args);
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'",atLogin,atLogin);
    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& plist = ObjectAccessor::GetPlayers();
    for(const auto & itr : plist)
        itr.second->SetAtLoginFlag(atLogin);

    return true;
}
