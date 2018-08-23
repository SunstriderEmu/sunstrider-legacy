#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"

class reset_commandscript : public CommandScript
{
public:
    reset_commandscript() : CommandScript("reset_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> resetCommandTable =
        {
            { "honor",          SEC_GAMEMASTER3,  false, &HandleResetHonorCommand,          "" },
            { "level",          SEC_GAMEMASTER3,  false, &HandleResetLevelCommand,          "" },
            { "spells",         SEC_GAMEMASTER3,  false, &HandleResetSpellsCommand,         "" },
            { "stats",          SEC_GAMEMASTER3,  false, &HandleResetStatsCommand,          "" },
            { "talents",        SEC_GAMEMASTER3,  false, &HandleResetTalentsCommand,        "" },
            { "all",            SEC_GAMEMASTER3,  false, &HandleResetAllCommand,            "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "reset",          SEC_GAMEMASTER3,  false, nullptr,                           "", resetCommandTable },
        };
        return commandTable;
    }

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

        player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_PLAYER_BOUNDING_RADIUS);
        player->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_PLAYER_COMBAT_REACH);

        player->SetFactionForRace(player->GetRace());

        player->SetUInt32Value(UNIT_FIELD_BYTES_0, ((player->GetRace()) | (player->GetClass() << 8) | (player->GetGender() << 16) | (powertype << 24)));

        // reset only if player not in some form;
        if (player->GetShapeshiftForm() == FORM_NONE)
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
        ShapeshiftForm form = player->GetShapeshiftForm();
        player->SetShapeshiftForm(form);

        player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        //-1 is default value
        player->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

        //player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );
        return true;
    }

    static bool HandleResetHonorCommand(ChatHandler* handler, char const* args)
    {
        char* pName = strtok((char*)args, "");
        Player *player = nullptr;
        if (pName)
        {
            std::string name = pName;
            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
            player = ObjectAccessor::FindPlayer(guid);
        }
        else
            player = handler->GetSelectedPlayerOrSelf();

        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            return true;
        }

        player->SetUInt32Value(PLAYER_FIELD_KILLS, 0);
        player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
        player->SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, 0);
        player->SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
        player->SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);

        return true;
    }

    static bool HandleResetLevelCommand(ChatHandler* handler, char const* args)
    {
        char* pName = strtok((char*)args, "");
        Player *player = nullptr;
        if (pName)
        {
            std::string name = pName;
            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
            player = ObjectAccessor::FindPlayer(guid);
        }
        else
            player = handler->GetSelectedPlayerOrSelf();

        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!HandleResetStatsOrLevelHelper(player))
            return false;

        player->SetLevel(1);
        player->InitStatsForLevel(true);
        player->InitTaxiNodesForLevel();
        player->InitTalentForLevel();
        player->SetUInt32Value(PLAYER_XP, 0);

        // reset level to summoned pet
        Pet* pet = player->GetPet();
        if (pet && pet->getPetType() == SUMMON_PET)
            pet->InitStatsForLevel(1);

        return true;
    }

    static bool HandleResetStatsCommand(ChatHandler* handler, char const* args)
    {
        char* pName = strtok((char*)args, "");
        Player *player = nullptr;
        if (pName)
        {
            std::string name = pName;
            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(name.c_str());
            player = ObjectAccessor::FindPlayer(guid);
        }
        else
            player = handler->GetSelectedPlayerOrSelf();

        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!HandleResetStatsOrLevelHelper(player))
            return false;

        player->InitStatsForLevel(true);
        player->InitTaxiNodesForLevel();
        player->InitTalentForLevel();

        return true;
    }

    static bool HandleResetSpellsCommand(ChatHandler* handler, char const* args)
    {
        char* pName = strtok((char*)args, "");
        Player *player = nullptr;
        ObjectGuid playerGUID = ObjectGuid::Empty;
        if (pName)
        {
            std::string name = pName;

            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            player = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
            if (!player)
                playerGUID = sCharacterCache->GetCharacterGuidByName(name.c_str());
        }
        else
            player = handler->GetSelectedPlayerOrSelf();

        if (!player && !playerGUID)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player)
        {
            player->resetSpells();

            ChatHandler(player).SendSysMessage(LANG_RESET_SPELLS);

            if (handler->GetSession()->GetPlayer() != player)
                handler->PSendSysMessage(LANG_RESET_SPELLS_ONLINE, player->GetName().c_str());
        }
        else
        {
            CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", uint32(AT_LOGIN_RESET_SPELLS), playerGUID.GetCounter());
            handler->PSendSysMessage(LANG_RESET_SPELLS_OFFLINE, pName);
        }

        return true;
    }

    static bool HandleResetTalentsCommand(ChatHandler* handler, char const* args)
    {
        char* pName = strtok((char*)args, "");
        Player *player = nullptr;
        ObjectGuid playerGUID = ObjectGuid::Empty;
        if (pName)
        {
            std::string name = pName;
            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            player = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
            if (!player)
                playerGUID = sCharacterCache->GetCharacterGuidByName(name.c_str());
        }
        else
            player = handler->GetSelectedPlayerOrSelf();

        if (!player && !playerGUID)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player)
        {
            player->ResetTalents(true);

            ChatHandler(player).SendSysMessage(LANG_RESET_TALENTS);

            if (handler->GetSession()->GetPlayer() != player)
                handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, player->GetName().c_str());
        }
        else
        {
            CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", uint32(AT_LOGIN_RESET_TALENTS), playerGUID.GetCounter());
            handler->PSendSysMessage(LANG_RESET_TALENTS_OFFLINE, pName);
        }

        return true;
    }

    static bool HandleResetAllCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string casename = args;

        AtLoginFlags atLogin;

        // Command specially created as single command to prevent using short case names
        if (casename == "spells")
        {
            atLogin = AT_LOGIN_RESET_SPELLS;
            sWorld->SendWorldText(LANG_RESETALL_SPELLS);
        }
        else if (casename == "talents")
        {
            atLogin = AT_LOGIN_RESET_TALENTS;
            sWorld->SendWorldText(LANG_RESETALL_TALENTS);
        }
        else
        {
            handler->PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE, args);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'", atLogin, atLogin);
        boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
        HashMapHolder<Player>::MapType const& plist = ObjectAccessor::GetPlayers();
        for (const auto & itr : plist)
            itr.second->SetAtLoginFlag(atLogin);

        return true;
    }
};

void AddSC_reset_commandscript()
{
    new reset_commandscript();
}
