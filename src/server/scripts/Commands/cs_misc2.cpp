#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#include "WaypointManager.h"
#include "ChannelMgr.h"
#include "LogsDatabaseAccessor.h"
#include "GridMap.h"
#include "BattleGround.h"
#include "AccountMgr.h"
#include "CellImpl.h"
#include "MovementDefines.h"
#include "MMapFactory.h"
#include "ArenaTeamMgr.h"
#include "MapManager.h"
#include "Mail.h"
#include "ReputationMgr.h"

class misc2_commandscript : public CommandScript
{
public:
    misc2_commandscript() : CommandScript("misc2_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commandTable =
        {
            { "announce",       SEC_GAMEMASTER1,  true,  &HandleAnnounceCommand,            "" },
            { "appear",         SEC_GAMEMASTER1,  false, &HandleAppearCommand,              "" },
            { "arenarename",    SEC_GAMEMASTER2,  true,  &HandleRenameArenaTeamCommand,     "" },
            { "bg",             SEC_PLAYER,       false, &HandleBattlegroundCommand,        "" },
            { "blink",          SEC_GAMEMASTER1,  false, &HandleBlinkCommand,               "" },
            { "chanban",        SEC_GAMEMASTER1,  true,  &HandleChanBan,                    "" },
            { "chaninfoban",    SEC_GAMEMASTER1,  true,  &HandleChanInfoBan,                "" },
            { "chanunban",      SEC_GAMEMASTER1,  true,  &HandleChanUnban,                  "" },
            { "chardelete",     SEC_ADMINISTRATOR,true,  &HandleCharacterDeleteCommand,     "" },
            { "combatstop",     SEC_GAMEMASTER2,  false, &HandleCombatStopCommand,          "" },
            { "commands",       SEC_PLAYER,       true,  &HandleCommandsCommand,            "" },
            { "copystuff",      SEC_GAMEMASTER2,  false, &HandleCopyStuffCommand,           "" },
            { "dismount",       SEC_PLAYER,       false, &HandleDismountCommand,            "" },
            { "gmannounce",     SEC_GAMEMASTER1,  true,  &HandleGMAnnounceCommand,          "" },
            { "gmnotify",       SEC_GAMEMASTER1,  true,  &HandleGMNotifyCommand,            "" },
            { "gps",            SEC_GAMEMASTER1,  true,  &HandleGPSCommand,                 "" },
            { "gpss",           SEC_GAMEMASTER1,  false, &HandleGPSSCommand,                "" },
            { "help",           SEC_PLAYER,       true,  &HandleHelpCommand,                "" },
            { "heroday",        SEC_PLAYER,       true,  &HandleHerodayCommand,             "" },
            { "loadpath",       SEC_GAMEMASTER3,  false, &HandleReloadAllPaths,             "" },
            { "lockaccount",    SEC_PLAYER,       false, &HandleLockAccountCommand,         "" },
            { "summon",         SEC_GAMEMASTER1,  false, &HandleSummonCommand,              "" },
            { "notify",         SEC_GAMEMASTER1,  true,  &HandleNotifyCommand,              "" },
            { "password",       SEC_PLAYER,       false, &HandlePasswordCommand,            "" },
            { "pinfo",          SEC_GAMEMASTER2,  true,  &HandlePInfoCommand,               "" },
            { "recall",         SEC_GAMEMASTER1,  false, &HandleRecallCommand,              "" },
            { "rename",         SEC_GAMEMASTER2,  true,  &HandleCharacterRenameCommand,     "" },
            { "repairitems",    SEC_GAMEMASTER2,  false, &HandleRepairitemsCommand,         "" },
            { "save",           SEC_PLAYER,       false, &HandleSaveCommand,                "" },
            { "saveall",        SEC_GAMEMASTER1,  true,  &HandleSaveAllCommand,             "" },
            { "sendmail",       SEC_GAMEMASTER1,  true,  &HandleSendMailCommand,            "" },
            { "start",          SEC_PLAYER,       false, &HandleStartCommand,               "" },
            { "updatetitle",    SEC_PLAYER,       false, &HandleUpdatePvPTitleCommand,      "" },
            { "whispers",       SEC_GAMEMASTER1,  false, &HandleWhispersCommand,            "" },
        };
        return commandTable;
    }

    //Enable\Disable accept whispers (for GM)
    static bool HandleWhispersCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING, handler->GetSession()->GetPlayer()->IsAcceptWhispers() ? handler->GetTrinityString(LANG_ON) : handler->GetTrinityString(LANG_OFF));
            return true;
        }

        std::string argstr = (char*)args;
        // whisper on
        if (argstr == "on")
        {
            handler->GetSession()->GetPlayer()->SetAcceptWhispers(true);
            handler->SendSysMessage(LANG_COMMAND_WHISPERON);
            return true;
        }

        // whisper off
        if (argstr == "off")
        {
            handler->GetSession()->GetPlayer()->SetAcceptWhispers(false);
            handler->SendSysMessage(LANG_COMMAND_WHISPEROFF);
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleHelpCommand(ChatHandler* handler, char const* args)
    {
        char* cmd = strtok((char*)args, " ");
        if (!cmd)
        {
            handler->ShowHelpForCommand(handler->getCommandTable(), "help");
            handler->ShowHelpForCommand(handler->getCommandTable(), "");
        }
        else
        {
            if (!handler->ShowHelpForCommand(handler->getCommandTable(), cmd))
                handler->SendSysMessage(LANG_NO_HELP_CMD);
        }

        return true;
    }

    static bool HandleCommandsCommand(ChatHandler* handler, char const* args)
    {
        handler->ShowHelpForCommand(handler->getCommandTable(), "");
        return true;
    }


    static bool HandleStartCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *chr = handler->GetSession()->GetPlayer();

        if (chr->IsInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (chr->IsInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (chr->HasAura(9454)) // Char is freezed by GM
        {
            handler->SendSysMessage("Impossible when you are frozen.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (chr->InBattleground())
        {
            if (chr->IsAlive())
                handler->SendSysMessage("Unusable in battlegrounds when you are alive.");
            else {
                Battleground* bg = chr->GetBattleground();
                if (bg) {
                    WorldSafeLocsEntry const* closestGrave = bg->GetClosestGraveYard(chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), chr->GetTeam());
                    if (closestGrave)
                        chr->TeleportTo(bg->GetMapId(), closestGrave->x, closestGrave->y, closestGrave->z, chr->GetOrientation());
                }
            }

            return true;
        }

        // cast spell Stuck
        //chr->CastSpell(chr,7355, TRIGGERED_NONE);
        if (chr->IsAlive())
            chr->KillSelf();
        chr->RepopAtGraveyard();
        return true;
    }


    static bool HandleDismountCommand(ChatHandler* handler, char const* /*args*/)
    {
        //If player is not mounted, so go out :)
        if (!handler->GetSession()->GetPlayer()->IsMounted())
        {
            handler->SendSysMessage(LANG_CHAR_NON_MOUNTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (handler->GetSession()->GetPlayer()->IsInFlight())
        {
            handler->SendSysMessage(LANG_YOU_IN_FLIGHT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->GetSession()->GetPlayer()->Dismount();
        handler->GetSession()->GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);
        return true;
    }


    static bool HandleSaveCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *player = handler->GetSession()->GetPlayer();

        // save GM account without delay and output message (testing, etc)
        if (handler->GetSession()->GetSecurity())
        {
            player->SaveToDB();
            handler->SendSysMessage(LANG_PLAYER_SAVED);
            return true;
        }

        // save if player was saved less than 20s ago or if no save interval is set
        uint32 save_interval = sWorld->getConfig(CONFIG_INTERVAL_SAVE);
        if (save_interval == 0
            || (save_interval > 20 * SECOND*IN_MILLISECONDS && player->GetSaveTimer() <= (save_interval - 20 * SECOND*IN_MILLISECONDS)))
            player->SaveToDB();

        //no output to prevent cheat

        return true;
    }


    static bool HandlePasswordCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char *old_pass = strtok((char*)args, " ");
        char *new_pass = strtok(nullptr, " ");
        char *new_pass_c = strtok(nullptr, " ");

        if (!old_pass || !new_pass || !new_pass_c)
            return false;

        std::string password_old = old_pass;
        std::string password_new = new_pass;
        std::string password_new_c = new_pass_c;

        if (strcmp(new_pass, new_pass_c) != 0)
        {
            handler->SendSysMessage(LANG_NEW_PASSWORDS_NOT_MATCH);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sAccountMgr->CheckPassword(handler->GetSession()->GetAccountId(), password_old))
        {
            handler->SendSysMessage(LANG_COMMAND_WRONGOLDPASSWORD);
            handler->SetSentErrorMessage(true);
            return false;
        }

        AccountOpResult result = sAccountMgr->ChangePassword(handler->GetSession()->GetAccountId(), password_new);

        switch (result)
        {
        case AOR_OK:
            handler->SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_PASS_TOO_LONG:
            handler->SendSysMessage(LANG_PASSWORD_TOO_LONG);
            handler->SetSentErrorMessage(true);
            return false;
        case AOR_NAME_NOT_EXIST:                            // not possible case, don't want get account name for output
        default:
            handler->SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleLockAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
        {
            handler->SendSysMessage(LANG_USE_BOL);
            return true;
        }

        std::string argstr = (char*)args;
        if (argstr == "on")
        {
            LoginDatabase.PExecute("UPDATE account SET locked = '1' WHERE id = '%d'", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
            return true;
        }

        if (argstr == "off")
        {
            LoginDatabase.PExecute("UPDATE account SET locked = '0' WHERE id = '%d'", handler->GetSession()->GetAccountId());
            handler->PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        return true;
    }

    static bool HandleHerodayCommand(ChatHandler* handler, char const* args)
    {
        LocaleConstant loc_idx = handler->GetSessionDbcLocale();
        if (loc_idx >= 0) {
            QuestLocale const* pQuest = sObjectMgr->GetQuestLocale(sWorld->GetCurrentQuestForPool(1));
            if (pQuest) {
                if (pQuest->Title.size() > loc_idx && !pQuest->Title[loc_idx].empty())
                    //PSendSysMessage("La quête héroïque du jour est : \"%s\".", pQuest->Title[loc_idx].c_str());
                    handler->PSendSysMessage("Daily heroic quest: \"%s\".", pQuest->Title[loc_idx].c_str());
                else {
                    if (Quest const* qtemplate = sObjectMgr->GetQuestTemplate(sWorld->GetCurrentQuestForPool(1)))
                        handler->PSendSysMessage("Daily heroic quest: \"%s\".", qtemplate->GetTitle().c_str());
                }
            }
            else
                handler->PSendSysMessage("Error while fetching daily heroic quest.");
        }
        else {
            if (Quest const* qtemplate = sObjectMgr->GetQuestTemplate(sWorld->GetCurrentQuestForPool(1)))
                handler->PSendSysMessage("Daily heroic quest: \"%s\".", qtemplate->GetTitle().c_str());
        }

        return true;
    }

    static bool HandleBattlegroundCommand(ChatHandler* handler, char const* args)
    {
        Player* p = handler->GetSession()->GetPlayer();
        if (!p) return true;

        ARGS_CHECK

        if (p->InBattleground() || p->GetMap()->Instanceable())
            return true;

        char* cBGType = strtok((char*)args, " ");
        char* cAsGroup = strtok(nullptr, " ");

        BattlegroundTypeId bgTypeId = BATTLEGROUND_TYPE_NONE;
        if (strcmp(cBGType, "warsong") == 0)
            bgTypeId = BATTLEGROUND_WS;
        else if (strcmp(cBGType, "eye") == 0)
            bgTypeId = BATTLEGROUND_EY;
        else if (strcmp(cBGType, "arathi") == 0)
            bgTypeId = BATTLEGROUND_AB;
        else if (strcmp(cBGType, "alterac") == 0)
            bgTypeId = BATTLEGROUND_AV;

        if (bgTypeId == BATTLEGROUND_TYPE_NONE) //no valid bg type provided
            return false;

        bool asGroup = false;
        if (cAsGroup && strcmp(cAsGroup, "group") == 0)
            asGroup = true;

        handler->GetSession()->_HandleBattlegroundJoin(bgTypeId, 0, asGroup);

        return true;
    }

    //Summon Player
    static bool HandleSummonCommand(ChatHandler* handler, char const* args)
    {
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        if (target == _player || targetGuid == _player->GetGUID())
        {
            handler->SendSysMessage("cannot teleport self"); //handler->PSendSysMessage(LANG_CANT_TELEPORT_SELF);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            uint32 options = 0;
            if (target->IsBeingTeleported() == true)
            {
                handler->PSendSysMessage(LANG_IS_TELEPORTED, target->GetName().c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            Map* pMap = handler->GetSession()->GetPlayer()->GetMap();

            if (pMap->IsBattlegroundOrArena())
            {
                // only allow if gm mode is on
                if (!target->IsGameMaster())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, target->GetName().c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // if both players are in different bgs
                else if (target->GetBattlegroundId() && _player->GetBattlegroundId() != target->GetBattlegroundId())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_FROM_BG, target->GetName().c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                target->SetBattlegroundId(_player->GetBattlegroundId(), _player->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!target->GetMap()->IsBattlegroundOrArena())
                    target->SetBattlegroundEntryPoint();
            }
            else if (pMap->IsDungeon())
            {
                Map* cMap = target->GetMap();
                if (pMap && cMap->Instanceable() && cMap->GetInstanceId() != pMap->GetInstanceId())
                {
                    if (pMap->GetId() == cMap->GetId())
                        options |= TELE_TO_FORCE_RELOAD; //always force reload if same dungeon, this fixes teleporting to the wrong instance.

                    // cannot summon from instance to instance
                    /*handler->PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, target->GetName().c_str());
                    handler->SetSentErrorMessage(true);
                    return false;*/
                }

                // we are in instance, and can summon only player in our group with us as lead
                /*if (!_player->GetGroup() || !target->GetGroup() ||
                    (target->GetGroup()->GetLeaderGUID() != _player->GetGUID()) ||
                    (_player->GetGroup()->GetLeaderGUID() != _player->GetGUID()))
                    // the last check is a bit excessive, but let it be, just in case
                {
                    handler->PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, target->GetName().c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }*/

                if (InstanceSave* save = sInstanceSaveMgr->GetInstanceSave(_player->GetInstanceId()))
                    target->BindToInstance(save, false);

                target->SetDifficulty(_player->GetDifficulty(), false, false);
            }

            // stop flight if need
            if (target->IsInFlight())
                target->FinishTaxiFlight();
            // save only in non-flight case
            else
                target->SaveRecallPosition();

            // before GM
            float x, y, z;
            _player->GetClosePoint(x, y, z, target->GetCombatReach());
            if (target->TeleportTo(_player->GetMapId(), x, y, z, target->GetOrientation(), options))
            {
                target->SetPhaseMask(_player->GetPhaseMask(), true);
                handler->PSendSysMessage(LANG_SUMMONING, target->GetName().c_str(), "");
                if (handler->needReportToTarget(target))
                    ChatHandler(target).PSendSysMessage(LANG_SUMMONED_BY, handler->GetName().c_str());
            }
            else {
                handler->PSendSysMessage("Teleport failed");
            }
        }
        else 
        {
            handler->PSendSysMessage(LANG_SUMMONING, targetName.c_str(), handler->GetTrinityString(LANG_OFFLINE));

            // in point where GM stay
            Player::SavePositionInDB(handler->GetSession()->GetPlayer()->GetMapId(),
                handler->GetSession()->GetPlayer()->GetPositionX(),
                handler->GetSession()->GetPlayer()->GetPositionY(),
                handler->GetSession()->GetPlayer()->GetPositionZ(),
                handler->GetSession()->GetPlayer()->GetOrientation(),
                handler->GetSession()->GetPlayer()->GetZoneId(),
                targetGuid);
        }

        return true;
    }

    //Teleport to Player
    static bool HandleAppearCommand(ChatHandler* handler, char const* args)
    {

        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        Player* _player = handler->GetSession()->GetPlayer();
        if (target == _player || targetGuid == _player->GetGUID())
        {
            //handler->SendSysMessage(LANG_CANT_TELEPORT_SELF);
            handler->SendSysMessage("Can't teleport self");
            handler->SetSentErrorMessage(true);
            return false;
        }
        
        if (target)
        {
            uint32 options = TELE_TO_GM_MODE;
            Map* cMap = target->GetMap();
            if (!cMap)
            {
                handler->SendSysMessage("Target is not in any map");
                return true;
            }
            if (cMap->IsBattlegroundOrArena())
            {
                // only allow if gm mode is on
                if (!_player->IsGameMaster())
                {
                    handler->PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, target->GetName().c_str());
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                // if both players are in different bgs
                else if (_player->GetBattlegroundId() && _player->GetBattlegroundId() != target->GetBattlegroundId())
                    _player->LeaveBattleground(false); // Note: should be changed so _player gets no Deserter debuff

                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                _player->SetBattlegroundId(target->GetBattlegroundId(), target->GetBattlegroundTypeId());
                // remember current position as entry point for return at bg end teleportation
                if (!_player->GetMap()->IsBattlegroundOrArena())
                    _player->SetBattlegroundEntryPoint();

            }
            else if (cMap->IsDungeon())
            {
                sMapMgr->CreateMap(_player->GetMapId(), _player);

                // we have to go to instance, and can go to player only if:
                //   1) we are in his group (either as leader or as member)
                //   2) we are not bound to any group and have GM mode on
                if (_player->GetGroup())
                {
                    // we are in group, we can go only if we are in the player group
                    if (_player->GetGroup() != target->GetGroup())
                    {
                        handler->PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY, target->GetName().c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }
                else
                {
                    // we are not in group, let's verify our GM mode
                    if (!_player->IsGameMaster())
                    {
                        handler->PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM, target->GetName().c_str());
                        handler->SetSentErrorMessage(true);
                        return false;
                    }
                }

                Map* pMap = _player->GetMap();
                if (pMap && cMap->Instanceable() && pMap->GetId() == cMap->GetId() && cMap->GetInstanceId() != pMap->GetInstanceId())
                    options |= TELE_TO_FORCE_RELOAD; //always force reload if same dungeon, this fixes teleporting to the wrong instance.

                // if the player or the player's group is bound to another instance
                // the player will not be bound to another one
                /*sun: always bind appearing player to target instance, we want to appear next to target, not in our own instance.
                InstancePlayerBind* pBind = _player->GetBoundInstance(target->GetMapId(), target->GetDifficulty());
                if (!pBind)
                {
                    Group* group = _player->GetGroup();
                    // if no bind exists, create a solo bind
                    InstanceGroupBind* gBind = group ? group->GetBoundInstance(target->GetDifficulty(), target->GetMapId()) : nullptr;
                    if (!gBind)
                    */
                        if (InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(target->GetInstanceId()))
                            _player->BindToInstance(save, false);
                        /*
                }*/

                _player->SetDifficulty(target->GetDifficulty(), false, false);
            }

            // stop flight if need
            if (_player->IsInFlight())
                _player->FinishTaxiFlight();
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            // to point to see at target with same orientation
            float x, y, z;
            target->GetContactPoint(handler->GetSession()->GetPlayer(), x, y, z);

            if (_player->TeleportTo(target->GetMapId(), x, y, z, _player->GetAbsoluteAngle(target), options))
            {
                _player->SetPhaseMask(target->GetPhaseMask(), true);
                handler->PSendSysMessage(LANG_APPEARING_AT, target->GetName().c_str());
                if (_player->IsVisibleGloballyFor(target))
                    ChatHandler(target).PSendSysMessage(LANG_APPEARING_TO, _player->GetName().c_str());
            }
            else {
                handler->PSendSysMessage("Teleportation failed");
            }

            return true;
        }
        else
        {
            // check offline security
            if (handler->HasLowerSecurity(nullptr, targetGuid))
                return false;

            handler->PSendSysMessage(LANG_APPEARING_AT, targetName.c_str());

            // to point where player stay (if loaded)
            float x, y, z, o;
            uint32 map;
            bool in_flight;
            if (!Player::LoadPositionFromDB(map, x, y, z, o, in_flight, targetGuid))
            {
                handler->PSendSysMessage("Failed to load target player position");
                return false;
            }
            // stop flight if need
            if (_player->IsInFlight())
                _player->FinishTaxiFlight();
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            _player->TeleportTo(map, x, y, z, _player->GetOrientation());
            return true;
        }
    }

    // Teleport player to last position
    static bool HandleRecallCommand(ChatHandler* handler, char const* args)
    {
        Player* chr = nullptr;

        if (!*args)
        {
            chr = handler->GetSelectedPlayer();
            if (!chr)
                chr = handler->GetSession()->GetPlayer();
        }
        else
        {
            std::string name = args;

            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            chr = ObjectAccessor::FindConnectedPlayerByName(name.c_str());

            if (!chr)
            {
                handler->PSendSysMessage(LANG_NO_PLAYER, args);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (chr->IsBeingTeleported())
        {
            handler->PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        chr->FinishTaxiFlight();

        chr->Recall();
        return true;
    }

    // global announce
    static bool HandleAnnounceCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            sWorld->SendWorldText(LANG_SYSTEMMESSAGE, args);
        return true;
    }

    // announce to logged in GMs
    static bool HandleGMAnnounceCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            sWorld->SendGMText(LANG_GM_BROADCAST, args);
        return true;
    }

    //notification player at the screen
    static bool HandleNotifyCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string str = handler->GetTrinityString(LANG_GLOBAL_NOTIFY);
        str += args;

        WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
        data << str;
        sWorld->SendGlobalMessage(&data);

        return true;
    }

    //notification GM at the screen
    static bool HandleGMNotifyCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string str = handler->GetTrinityString(LANG_GM_NOTIFY);
        str += args;

        WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
        data << str;
        sWorld->SendGlobalGMMessage(&data);

        return true;
    }


    static bool HandleGPSCommand(ChatHandler* handler, char const* args)
    {
        WorldObject *obj = nullptr;
        if (*args)
        {
            std::string name = args;
            if (normalizePlayerName(name))
                obj = ObjectAccessor::FindConnectedPlayerByName(name.c_str());

            if (!obj)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            obj = handler->GetSelectedUnit();

            if (!obj)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        CellCoord cell_val = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
        Cell cell(cell_val);

        uint32 zone_id = obj->GetZoneId();
        uint32 area_id = obj->GetAreaId();

        MapEntry const* mapEntry = sMapStore.LookupEntry(obj->GetMapId());
        AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(zone_id);
        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(area_id);

        float zone_x = obj->GetPositionX();
        float zone_y = obj->GetPositionY();

        Map2ZoneCoordinates(zone_x, zone_y, zone_id);

        Map const *map = obj->GetMap();
        float ground_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), MAX_HEIGHT);
        float floor_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ() + obj->GetCollisionHeight());

        GridCoord gridCoord = Trinity::ComputeGridCoord(obj->GetPositionX(), obj->GetPositionY());

        int gridX = (MAX_NUMBER_OF_GRIDS - 1) - gridCoord.x_coord;
        int gridY = (MAX_NUMBER_OF_GRIDS - 1) - gridCoord.y_coord;

        uint32 haveMap = GridMap::ExistMap(obj->GetMapId(), gridX, gridY) ? 1 : 0;
        uint32 haveVMap = GridMap::ExistVMap(obj->GetMapId(), gridX, gridY) ? 1 : 0;
        uint32 haveMMap = (/*DisableMgr::IsPathfindingEnabled(mapId) &&*/ MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(handler->GetSession()->GetPlayer()->GetMapId())) ? 1 : 0;

        if (haveVMap)
        {
            if (obj->IsOutdoors())
                handler->PSendSysMessage("You are outdoors");
            else
                handler->PSendSysMessage("You are indoor");

            /*
            uint32 mogpFlags;
            int32 adtId, rootId, groupId;
            WMOAreaTableEntry const* wmoEntry = nullptr;

            if (map->GetAreaInfo(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), mogpFlags, adtId, rootId, groupId))
            {
                if ((wmoEntry = GetWMOAreaTableEntryByTripple(rootId, adtId, groupId)))
                    handler->PSendSysMessage(LANG_GPS_WMO_DATA, wmoEntry->Id, wmoEntry->Flags, mogpFlags);
            }*/
        }
            else handler->PSendSysMessage("no VMAP available for area info");

        handler->PSendSysMessage(LANG_MAP_POSITION,
            obj->GetMapId(), (mapEntry ? mapEntry->name[handler->GetSessionDbcLocale()] : "<unknown>"),
            zone_id, (zoneEntry ? zoneEntry->area_name[handler->GetSessionDbcLocale()] : "<unknown>"),
            area_id, (areaEntry ? areaEntry->area_name[handler->GetSessionDbcLocale()] : "<unknown>"),
            obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
            cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetInstanceId(),
            zone_x, zone_y, ground_z, floor_z, haveMap, haveVMap, haveMMap);

        LiquidData liquidStatus;
        ZLiquidStatus status = map->GetLiquidStatus(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), MAP_ALL_LIQUIDS, &liquidStatus, obj->GetCollisionHeight());
        if (status)
            handler->PSendSysMessage(LANG_LIQUID_STATUS, liquidStatus.level, liquidStatus.depth_level, liquidStatus.entry, liquidStatus.type_flags, status);

        //more correct format for script, you just have to copy/paste !
        handler->PSendSysMessage(LANG_GPS_FOR_SCRIPT, obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());

        //TC_LOG_DEBUG("command","%f, %f, %f, %f", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());

        return true;
    }

    static bool HandleGPSSCommand(ChatHandler* handler, char const* args)
    {
        WorldObject *obj = nullptr;
        if (*args)
        {
            std::string name = args;
            if (normalizePlayerName(name))
                obj = ObjectAccessor::FindConnectedPlayerByName(name.c_str());

            if (!obj)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            obj = handler->GetSelectedUnit();

            if (!obj)
            {
                handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        handler->PSendSysMessage("%f %f %f %f", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
        return true;
    }

    //Send mail by command
    static bool HandleSendMailCommand(ChatHandler* handler, char const* args)
    {
        // format: name "subject text" "mail text"
        Player* target;
        ObjectGuid targetGuid;
        std::string targetName;
        if (!handler->extractPlayerTarget((char*)args, &target, &targetGuid, &targetName))
            return false;

        char* tail1 = strtok(nullptr, "");
        if (!tail1)
            return false;

        char* msgSubject;
        if (*tail1 == '"')
            msgSubject = strtok(tail1 + 1, "\"");
        else
        {
            char* space = strtok(tail1, "\"");
            if (!space)
                return false;
            msgSubject = strtok(nullptr, "\"");
        }

        if (!msgSubject)
            return false;

        char* tail2 = strtok(nullptr, "");
        if (!tail2)
            return false;

        char* msgText;
        if (*tail2 == '"')
            msgText = strtok(tail2 + 1, "\"");
        else
        {
            char* space = strtok(tail2, "\"");
            if (!space)
                return false;
            msgText = strtok(nullptr, "\"");
        }

        if (!msgText)
            return false;

        // msgSubject, msgText isn't NUL after prev. check
        std::string subject = msgSubject;
        std::string text = msgText;


        // from console, use non-existing sender
        MailSender sender(MAIL_NORMAL, handler->GetSession() ? handler->GetSession()->GetPlayer()->GetGUID().GetCounter() : 0, MAIL_STATIONERY_GM);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        MailDraft(subject, text)
            .SendMailTo(trans, MailReceiver(target, targetGuid.GetCounter()), sender, MAIL_CHECK_MASK_COPIED);

        CharacterDatabase.CommitTransaction(trans);

        handler->PSendSysMessage(LANG_MAIL_SENT, targetName.c_str());
        return true;
    }

    static bool HandleBlinkCommand(ChatHandler* handler, char const* args)
    {
        uint32 distance = 0;

        if (args)
            distance = (uint32)atoi(args);

        if (!distance)
            distance = 15;

        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return true;

        float currentX, currentY, currentZ, currentO;
        player->GetPosition(currentX, currentY, currentZ);
        currentO = player->GetOrientation();

        float newX = currentX + cos(currentO) * distance;
        float newY = currentY + sin(currentO) * distance;
        float newZ = currentZ;

        player->TeleportTo(handler->GetSession()->GetPlayer()->GetMapId(), newX, newY, newZ, currentO);

        return true;
    }

    //Save all players in the world
    static bool HandleSaveAllCommand(ChatHandler* handler, char const* /*args*/)
    {
        ObjectAccessor::SaveAllPlayers();
        handler->SendSysMessage(LANG_PLAYERS_SAVED);
        return true;
    }

    //show info of player
    static bool HandlePInfoCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;
        ObjectGuid targetGUID = ObjectGuid::Empty;

        PreparedStatement* stmt = nullptr;

        char* px = strtok((char*)args, " ");
        char* py = nullptr;

        std::string name;

        if (px)
        {
            name = px;

            if (name.empty())
                return false;

            if (!normalizePlayerName(name))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            target = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
            if (target)
                py = strtok(nullptr, " ");
            else
            {
                targetGUID = sCharacterCache->GetCharacterGuidByName(name);
                if (targetGUID)
                    py = strtok(nullptr, " ");
                else
                    py = px;
            }
        }

        if (!target && !targetGUID)
        {
            target = handler->GetSelectedPlayer();
        }

        if (!target && !targetGUID)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accId = 0;
        uint32 money = 0;
        uint32 total_player_time = 0;
        uint32 level = 0;
        uint32 latency = 0;

        // get additional information from Player object
        if (target)
        {
            targetGUID = target->GetGUID();
            name = target->GetName();                           // re-read for case handler->GetSelectedPlayer() target
            accId = target->GetSession()->GetAccountId();
            money = target->GetMoney();
            total_player_time = target->GetTotalPlayedTime();
            level = target->GetLevel();
            latency = target->GetSession()->GetLatency();
        }
        // get additional information from DB
        else
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_PINFO);
            stmt->setUInt32(0, targetGUID.GetCounter());
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (!result)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            Field *fields = result->Fetch();
            total_player_time = fields[0].GetUInt32();
            level = fields[1].GetUInt8();
            money = fields[2].GetUInt32();
            accId = fields[3].GetUInt32();
        }

        std::string username = handler->GetTrinityString(LANG_ERROR);
        std::string last_ip = handler->GetTrinityString(LANG_ERROR);
        uint8 security = 0;
        std::string last_login = handler->GetTrinityString(LANG_ERROR);
        std::string current_mail = handler->GetTrinityString(LANG_ERROR);
        std::string reg_mail = handler->GetTrinityString(LANG_ERROR);

        // Query the prepared statement for login data
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_PINFO);
        stmt->setInt32(0, int32(realm.Id.Realm));
        stmt->setUInt32(1, accId);
        PreparedQueryResult result = LoginDatabase.Query(stmt);

        if (result)
        {
            Field* fields = result->Fetch();
            username = fields[0].GetString();
            security = fields[1].GetUInt8();

            if (!handler->GetSession() || handler->GetSession()->GetSecurity() >= security)
            {
                current_mail = fields[2].GetString();
                reg_mail = fields[3].GetString();
                last_ip = fields[4].GetString();
                last_login = fields[5].GetString();
            }
            else
            {
                current_mail = "-";
                reg_mail = "-";
                last_ip = "-";
                last_login = "-";
            }
        }

        handler->PSendSysMessage(LANG_PINFO_ACCOUNT, (target ? "" : handler->GetTrinityString(LANG_OFFLINE)), name.c_str(), targetGUID.GetCounter(), username.c_str(), accId, security, last_ip.c_str(), last_login.c_str(), latency);

        std::string timeStr = secsToTimeString(total_player_time, true, true);
        uint32 gold = money / GOLD;
        uint32 silv = (money % GOLD) / SILVER;
        uint32 copp = (money % GOLD) % SILVER;
        handler->PSendSysMessage(LANG_PINFO_LEVEL, timeStr.c_str(), level, gold, silv, copp);

        handler->PSendSysMessage("Current mail: %s", current_mail.c_str());
        return true;
    }

    static bool HandleReloadAllPaths(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        uint32 id = atoi(args);

        if (!id)
            return false;

        handler->PSendSysMessage("%s%s|r|cff00ffff%u|r", "|cff00ff00", "Loading Path: ", id);
        sWaypointMgr->ReloadPath(id);
        return true;
    }


    //rename characters
    static bool HandleCharacterRenameCommand(ChatHandler* handler, char const* args)
    {
        Player* target = nullptr;
        ObjectGuid targetGUID = ObjectGuid::Empty;
        std::string oldname;

        char* px = strtok((char*)args, " ");

        if (px)
        {
            oldname = px;

            if (!normalizePlayerName(oldname))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            target = ObjectAccessor::FindConnectedPlayerByName(oldname.c_str());
            if (target)
            {
                // check online security
                if (handler->HasLowerSecurity(target, ObjectGuid::Empty))
                    return false;
            }
            else {
                targetGUID = sCharacterCache->GetCharacterGuidByName(oldname);
                // check offline security
                if (handler->HasLowerSecurity(nullptr, targetGUID))
                    return false;
            }
        }

        if (!target && !targetGUID)
        {
            target = handler->GetSelectedPlayer();
        }

        if (!target && !targetGUID)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            handler->PSendSysMessage(LANG_RENAME_PLAYER, target->GetName().c_str());
            target->SetAtLoginFlag(AT_LOGIN_RENAME);
        }
        else
        {
            handler->PSendSysMessage(LANG_RENAME_PLAYER_GUID, oldname.c_str(), targetGUID.GetCounter());
            CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '1' WHERE guid = '%u'", targetGUID.GetCounter());
        }

        return true;
    }

    /* Syntax : .arenarename <playername> <type> <newname> */
    static bool HandleRenameArenaTeamCommand(ChatHandler* handler, char const* args)
    {
        char* playerName = strtok((char*)args, " ");
        char* cType = strtok(nullptr, " ");
        char* newName = strtok(nullptr, "");
        if (!playerName || !cType || !newName)
            return false;

        uint8 type = atoi(cType);

        uint8 dataIndex = 0;
        switch (type)
        {
        case 2: dataIndex = 0; break;
        case 3: dataIndex = 1; break;
        case 5: dataIndex = 2; break;
        default:
            handler->SendSysMessage("Invalid team type (must be 2, 3 or 5).");
            return true;
        }

        ObjectGuid targetGUID = ObjectGuid::Empty;
        std::string stringName = playerName;

        targetGUID = sCharacterCache->GetCharacterGuidByName(stringName);
        if (!targetGUID)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return true;
        }

        CharacterCacheEntry const* playerData = sCharacterCache->GetCharacterCacheByGuid(targetGUID.GetCounter());
        if (!playerData)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return true;
        }

        uint32 arenateamid = playerData->arenaTeamId[dataIndex];
        if (!arenateamid)
        {
            handler->PSendSysMessage("Team not found. Also double check your team type.");
            return true;
        }

        CharacterDatabase.PQuery("UPDATE arena_team SET name = '%s' WHERE arenateamid = '%u'", newName, arenateamid);
        // + Update within memory ?

        handler->PSendSysMessage("Team (id %u) name changed to \"%s\"", arenateamid, newName);

        ArenaTeam* team = sArenaTeamMgr->GetArenaTeamById(arenateamid);
        if (team)
        {
            team->SetName(newName);
        }
        else {
            handler->PSendSysMessage("Could not change team name in current memory, the change will be effective only at next server start");
        }

        return true;
    }

    static bool HandleCombatStopCommand(ChatHandler* handler, char const* args)
    {
        Player *player;

        if (*args)
        {
            std::string playername = args;

            if (!normalizePlayerName(playername))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }

            player = ObjectAccessor::FindConnectedPlayerByName(playername.c_str());

            if (!player)
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            player = handler->GetSelectedPlayerOrSelf();
        }

        player->CombatStop();
        return true;
    }

    static bool HandleRepairitemsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player *target = handler->GetSelectedPlayerOrSelf();

        if (!target)
        {
            handler->PSendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Repair items
        target->DurabilityRepairAll(false, 0, false);

        handler->PSendSysMessage(LANG_YOU_REPAIR_ITEMS, target->GetName().c_str());
        if (handler->needReportToTarget(target))
            ChatHandler(target).PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, handler->GetName().c_str());
        return true;
    }

    static bool HandleChanBan(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string channelNamestr = "world";

        char* charname = strtok((char*)args, " ");
        if (!charname)
            return false;

        std::string charNamestr = charname;

        if (!normalizePlayerName(charNamestr)) {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(charNamestr.c_str());
        if (!accountid)
        {
            handler->PSendSysMessage("No account found for player name: %s.", charNamestr.c_str());
            return true;
        }

        char* duration = strtok(nullptr, " ");
        if (!duration || !atoi(duration))
            return false;

        char* reason = strtok(nullptr, "");
        std::string reasonstr;
        if (!reason)
        {
            handler->SendSysMessage("You must specify a reason.");
            return false;
        }

        reasonstr = reason;

        LogsDatabase.EscapeString(reasonstr);

        uint32 durationSecs = TimeStringToSecs(duration);

        CharacterDatabase.PExecute("INSERT INTO channel_ban VALUES (%u, %lu, \"%s\", \"%s\")", accountid, time(nullptr) + durationSecs, channelNamestr.c_str(), reasonstr.c_str());
        LogsDatabaseAccessor::Sanction(handler->GetSession(), accountid, 0, SANCTION_CHAN_BAN, durationSecs, reasonstr);

        handler->PSendSysMessage("You banned %s from World channed with the reason: %s.", charNamestr.c_str(), reasonstr.c_str());

        Player *player = ObjectAccessor::FindConnectedPlayerByName(charNamestr.c_str());
        if (!player)
            return true;

        if (ChannelMgr* cMgr = channelMgr(player->GetTeam())) {
            if (Channel *chn = cMgr->GetChannel(channelNamestr.c_str(), player)) {
                chn->Kick(handler->GetSession() ? handler->GetSession()->GetPlayer()->GetGUID() : ObjectGuid::Empty, player->GetName());
                chn->AddNewGMBan(accountid, time(nullptr) + durationSecs);
                //TODO translate
                ChatHandler(player).PSendSysMessage("You have been banned from World channel with this reason: %s", reasonstr.c_str());
                //ChatHandler(player).PSendSysMessage("Vous avez été banni du channel world avec la raison suivante : %s", reasonstr.c_str());
            }
        }

        return true;
    }

    static bool HandleChanUnban(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string channelNamestr = "world";

        char* charname = strtok((char*)args, "");
        if (!charname)
            return false;

        std::string charNamestr = charname;

        if (!normalizePlayerName(charNamestr)) {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(charNamestr.c_str());
        if (!accountid)
        {
            handler->PSendSysMessage("No account found for player %s.", charNamestr.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        CharacterDatabase.PExecute("UPDATE channel_ban SET expire = %lu WHERE accountid = %u AND expire > %lu", time(nullptr), accountid, time(nullptr));

        if (handler->GetSession())
        {
            if (ChannelMgr* cMgr = channelMgr(handler->GetSession()->GetPlayer()->GetTeam())) {
                if (Channel *chn = cMgr->GetChannel(channelNamestr.c_str(), handler->GetSession()->GetPlayer()))
                    chn->RemoveGMBan(accountid);
            }
        }

        LogsDatabaseAccessor::RemoveSanction(handler->GetSession(), accountid, 0, "", SANCTION_CHAN_BAN);

        handler->PSendSysMessage("Player %s is unbanned.", charNamestr.c_str());
        if (Player *player = ObjectAccessor::FindConnectedPlayerByName(charNamestr.c_str()))
        {
            //TODO translate
            ChatHandler(player).PSendSysMessage("You are now unbanned from the World channel.");
            //ChatHandler(player).PSendSysMessage("Vous êtes maintenant débanni du channel world.");
        }

        return true;
    }

    static bool HandleChanInfoBan(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            std::string channelNamestr = "world";

        char* charname = strtok((char*)args, "");
        if (!charname)
            return false;

        std::string charNamestr = charname;

        if (!normalizePlayerName(charNamestr)) {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(charNamestr.c_str());
        if (!accountid)
        {
            handler->PSendSysMessage("No account found for player %s", charNamestr.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        QueryResult result = CharacterDatabase.PQuery("SELECT reason, FROM_UNIXTIME(expire), expire FROM channel_ban WHERE accountid = %u AND channel = '%s'", accountid, channelNamestr.c_str());
        if (result) {
            do {
                Field *fields = result->Fetch();
                std::string reason = fields[0].GetString();
                std::string expiredate = fields[1].GetString();
                time_t expiretimestamp = time_t(fields[2].GetUInt32());

                handler->PSendSysMessage("Reason: \"%s\" - Expires in: %s %s", reason.c_str(), expiredate.c_str(), (expiretimestamp > time(nullptr)) ? "(actif)" : "");
            } while (result->NextRow());
        }
        else {
            handler->PSendSysMessage("No ban on this player.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool HandleUpdatePvPTitleCommand(ChatHandler* handler, char const* args)
    {
        if (Player * player = handler->GetSession()->GetPlayer()) {
            player->UpdateKnownPvPTitles();
            return true;
        }
        return false;
    }

    static bool HandleCharacterDeleteCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char *character_name_str = strtok((char*)args, " ");
        if (!character_name_str)
            return false;

        std::string character_name = character_name_str;
        if (!normalizePlayerName(character_name))
            return false;

        ObjectGuid character_guid;
        uint32 account_id;

        Player *player = ObjectAccessor::FindConnectedPlayerByName(character_name.c_str());
        if (player)
        {
            character_guid = player->GetGUID();
            account_id = player->GetSession()->GetAccountId();
            player->GetSession()->KickPlayer();
        }
        else
        {
            character_guid = sCharacterCache->GetCharacterGuidByName(character_name);
            if (!character_guid)
            {
                handler->PSendSysMessage(LANG_NO_PLAYER, character_name.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            account_id = sCharacterCache->GetCharacterAccountIdByGuid(character_guid);
        }

        std::string account_name;
        sAccountMgr->GetName(account_id, account_name);

        Player::DeleteFromDB(character_guid, account_id, true);
        handler->PSendSysMessage(LANG_CHARACTER_DELETED, character_name.c_str(), character_guid.GetCounter(), account_name.c_str(), account_id);
        return true;
    }


    //Visually copy stuff from player given to target player (fade off at disconnect like a normal morph)
    static bool HandleCopyStuffCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

        std::string fromPlayerName = args;
        Player* fromPlayer = nullptr;
        Player* toPlayer = handler->GetSelectedPlayerOrSelf();

        if (normalizePlayerName(fromPlayerName))
            fromPlayer = ObjectAccessor::FindConnectedPlayerByName(fromPlayerName.c_str());

        if (!fromPlayer || !toPlayer)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return true;
        }

        //4 lasts EQUIPMENT_SLOT = weapons (16-17-18?) + ammunition (19?)
        for (uint8 slot = 0; slot < (EQUIPMENT_SLOT_END - 4); slot++)
        {
            uint32 visualbase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
            toPlayer->SetUInt32Value(visualbase, fromPlayer->GetUInt32Value(visualbase));
        }

        //copy helm/cloak settings
        if (fromPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
            toPlayer->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
        else
            toPlayer->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);

        if (fromPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
            toPlayer->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
        else
            toPlayer->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);

        return true;
    }

};

void AddSC_misc2_commandscript()
{
    new misc2_commandscript();
}
