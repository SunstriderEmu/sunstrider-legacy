#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "MapManager.h"

class tele_commandscript : public CommandScript
{
public:
    tele_commandscript() : CommandScript("tele_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> teleCommandTable =
        {
            { "add",            SEC_GAMEMASTER3,  false, &HandleTeleAddCommand,             "" },
            { "del",            SEC_GAMEMASTER3,  true,  &HandleTeleDelCommand,             "" },
            { "name",           SEC_GAMEMASTER1,  true,  &HandleTeleNameCommand,            "" },
            { "group",          SEC_GAMEMASTER1,  false, &HandleTeleGroupCommand,           "" },
            { "",               SEC_GAMEMASTER1,  false, &HandleTeleCommand,                "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "tele",           SEC_GAMEMASTER1,  true,  nullptr,                                        "", teleCommandTable },
            { "groupgo",        SEC_GAMEMASTER1,  false, &HandleGroupgoCommand,                          "" },
        };
        return commandTable;
    }

    static bool HandleTeleAddCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player *player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        std::string name = args;

        if (sObjectMgr->GetGameTele(name))
        {
            handler->SendSysMessage(LANG_COMMAND_TP_ALREADYEXIST);
            handler->SetSentErrorMessage(true);
            return false;
        }

        GameTele tele;
        tele.position_x = player->GetPositionX();
        tele.position_y = player->GetPositionY();
        tele.position_z = player->GetPositionZ();
        tele.orientation = player->GetOrientation();
        tele.mapId = player->GetMapId();
        tele.name = name;

        if (sObjectMgr->AddGameTele(tele))
        {
            handler->SendSysMessage(LANG_COMMAND_TP_ADDED);
        }
        else
        {
            handler->SendSysMessage(LANG_COMMAND_TP_ADDEDERR);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleTeleDelCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string name = args;

        if (!sObjectMgr->DeleteGameTele(name))
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->SendSysMessage(LANG_COMMAND_TP_DELETED);
        return true;
    }

    // teleport player to given game_tele.entry
    static bool HandleTeleNameCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* pName = strtok((char*)args, " ");

        if (!pName)
            return false;

        std::string name = pName;

        if (!normalizePlayerName(name))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* tail = strtok(nullptr, "");
        if (!tail)
            return false;

        // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
        GameTele const* tele = handler->extractGameTeleFromLink(tail);
        if (!tele)
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
        if (!me || me->IsBattlegroundOrArena())
        {
            handler->SendSysMessage(LANG_CANNOT_TELE_TO_BG);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player *chr = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
        if (chr)
        {

            if (chr->IsBeingTeleported() == true)
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName().c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            handler->PSendSysMessage(LANG_TELEPORTING_TO, chr->GetName().c_str(), "", tele->name.c_str());
            if (handler->needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_TELEPORTED_TO_BY, handler->GetName().c_str());

            // stop flight if need
            if (chr->IsInFlight())
                chr->FinishTaxiFlight();
            else
                chr->SaveRecallPosition();  // save only in non-flight case

            chr->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
        }
        else if (ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(name.c_str()))
        {
            handler->PSendSysMessage(LANG_TELEPORTING_TO, name.c_str(), handler->GetTrinityString(LANG_OFFLINE), tele->name.c_str());
            Player::SavePositionInDB(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation, sMapMgr->GetZoneId(tele->mapId, tele->position_x, tele->position_y, tele->position_z), guid);
        }
        else
            handler->PSendSysMessage(LANG_NO_PLAYER, name.c_str());

        return true;
    }

    //Teleport group to given game_tele.entry
    static bool HandleTeleGroupCommand(ChatHandler* handler, GameTele const* tele)
    {
        if (!tele)
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;	           
        }

        Player *player = handler->GetSelectedPlayerOrSelf();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
        if (!me || me->IsBattlegroundOrArena())
        {
            handler->SendSysMessage(LANG_CANNOT_TELE_TO_BG);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Group *grp = player->GetGroup();
        if (!grp)
        {
            handler->PSendSysMessage(LANG_NOT_IN_GROUP, player->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (GroupReference *itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player *pl = itr->GetSource();

            if (!pl || !pl->GetSession())
                continue;

            if (pl->IsBeingTeleported())
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName().c_str());
                continue;
            }

            handler->PSendSysMessage(LANG_TELEPORTING_TO, pl->GetName().c_str(), "", tele->name.c_str());
            if (handler->needReportToTarget(pl))
                ChatHandler(pl).PSendSysMessage(LANG_TELEPORTED_TO_BY, handler->GetName().c_str());

            // stop flight if need
            if (pl->IsInFlight())
                pl->FinishTaxiFlight();
            // save only in non-flight case
            else
                pl->SaveRecallPosition();

            pl->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
        }

        return true;
    }

    //Summon group of player
    static bool HandleGroupgoCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string name = args;

        if (!normalizePlayerName(name))
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player *player = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
        if (!player)
        {
            handler->PSendSysMessage(LANG_NO_PLAYER, args);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Group *grp = player->GetGroup();

        if (!grp)
        {
            handler->PSendSysMessage(LANG_NOT_IN_GROUP, player->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        Map* gmMap = sMapMgr->CreateMap(handler->GetSession()->GetPlayer()->GetMapId(), handler->GetSession()->GetPlayer());
        bool to_instance = gmMap->Instanceable();

        // we are in instance, and can summon only player in our group with us as lead
        if (to_instance && (
            !handler->GetSession()->GetPlayer()->GetGroup() || (grp->GetLeaderGUID() != handler->GetSession()->GetPlayer()->GetGUID()) ||
            (handler->GetSession()->GetPlayer()->GetGroup()->GetLeaderGUID() != handler->GetSession()->GetPlayer()->GetGUID())))
            // the last check is a bit excessive, but let it be, just in case
        {
            handler->SendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (GroupReference *itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player *pl = itr->GetSource();

            if (!pl || pl == handler->GetSession()->GetPlayer() || !pl->GetSession())
                continue;

            if (pl->IsBeingTeleported() == true)
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName().c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            if (to_instance)
            {
                Map* plMap = sMapMgr->CreateMap(pl->GetMapId(), pl);

                if (plMap->Instanceable() && plMap->GetInstanceId() != gmMap->GetInstanceId())
                {
                    // cannot summon from instance to instance
                    handler->PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, pl->GetName().c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }

            handler->PSendSysMessage(LANG_SUMMONING, pl->GetName().c_str(), "");
            if (handler->needReportToTarget(pl))
                ChatHandler(pl).PSendSysMessage(LANG_SUMMONED_BY, handler->GetName().c_str());

            // stop flight if need
            if (pl->IsInFlight())
                pl->FinishTaxiFlight();
            // save only in non-flight case
            else
                pl->SaveRecallPosition();

            // before GM
            float x, y, z;
            handler->GetSession()->GetPlayer()->GetClosePoint(x, y, z, pl->GetCombatReach());
            pl->TeleportTo(handler->GetSession()->GetPlayer()->GetMapId(), x, y, z, pl->GetOrientation());
        }

        return true;
    }

    static bool HandleTeleCommand(ChatHandler* handler, GameTele const* tele)
    {
        if (!tele)
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* _player = handler->GetSession()->GetPlayer();
        if (_player->IsBeingTeleported())
        {
            handler->PSendSysMessage(LANG_IS_TELEPORTED, _player->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        MapEntry const * map = sMapStore.LookupEntry(tele->mapId);
        if (!map || (map->IsBattlegroundOrArena() && (_player->GetMapId() != tele->mapId || !_player->IsGameMaster())))
        {
            handler->SendSysMessage(LANG_CANNOT_TELE_TO_BG);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
        return true;
    }
};

void AddSC_tele_commandscript()
{
    new tele_commandscript();
}
