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

bool ChatHandler::HandleHelpCommand(const char* args)
{
    char* cmd = strtok((char*)args, " ");
    if (!cmd)
    {
        ShowHelpForCommand(getCommandTable(), "help");
        ShowHelpForCommand(getCommandTable(), "");
    }
    else
    {
        if (!ShowHelpForCommand(getCommandTable(), cmd))
            SendSysMessage(LANG_NO_HELP_CMD);
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* args)
{
    ShowHelpForCommand(getCommandTable(), "");
    return true;
}


bool ChatHandler::HandleStartCommand(const char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    if (chr->IsInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->IsInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->HasAuraEffect(9454)) // Char is freezed by GM
    {
        SendSysMessage("Impossible when you are frozen.");
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->InBattleground())
    {
        if (chr->IsAlive())
            SendSysMessage("Unusable in battlegrounds when you are alive.");
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
        chr->Kill(chr);
    chr->RepopAtGraveyard();
    return true;
}


bool ChatHandler::HandleDismountCommand(const char* /*args*/)
{
    //If player is not mounted, so go out :)
    if (!m_session->GetPlayer()->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (m_session->GetPlayer()->IsInFlight())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->Dismount();
    m_session->GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);
    return true;
}


bool ChatHandler::HandleSaveCommand(const char* /*args*/)
{
    Player *player = m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if (m_session->GetSecurity())
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
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


bool ChatHandler::HandlePasswordCommand(const char* args)
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
        SendSysMessage(LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage(true);
        return false;
    }

    if (!sAccountMgr->CheckPassword(m_session->GetAccountId(), password_old))
    {
        SendSysMessage(LANG_COMMAND_WRONGOLDPASSWORD);
        SetSentErrorMessage(true);
        return false;
    }

    AccountOpResult result = sAccountMgr->ChangePassword(m_session->GetAccountId(), password_new);

    switch (result)
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
        LoginDatabase.PExecute("UPDATE account SET locked = '1' WHERE id = '%d'", m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
        return true;
    }

    if (argstr == "off")
    {
        LoginDatabase.PExecute("UPDATE account SET locked = '0' WHERE id = '%d'", m_session->GetAccountId());
        PSendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    return true;
}

bool ChatHandler::HandleHerodayCommand(const char* args)
{
    LocaleConstant loc_idx = GetSessionDbcLocale();
    if (loc_idx >= 0) {
        QuestLocale const* pQuest = sObjectMgr->GetQuestLocale(sWorld->GetCurrentQuestForPool(1));
        if (pQuest) {
            if (pQuest->Title.size() > loc_idx && !pQuest->Title[loc_idx].empty())
                //PSendSysMessage("La quête héroïque du jour est : \"%s\".", pQuest->Title[loc_idx].c_str());
                PSendSysMessage("Daily heroic quest: \"%s\".", pQuest->Title[loc_idx].c_str());
            else {
                if (Quest const* qtemplate = sObjectMgr->GetQuestTemplate(sWorld->GetCurrentQuestForPool(1)))
                    PSendSysMessage("Daily heroic quest: \"%s\".", qtemplate->GetTitle().c_str());
            }
        }
        else
            PSendSysMessage("Error while fetching daily heroic quest.");
    }
    else {
        if (Quest const* qtemplate = sObjectMgr->GetQuestTemplate(sWorld->GetCurrentQuestForPool(1)))
            PSendSysMessage("Daily heroic quest: \"%s\".", qtemplate->GetTitle().c_str());
    }

    return true;
}

bool ChatHandler::HandleReportLagCommand(const char* args)
{
    time_t now = time(nullptr);
    Player* player = GetSession()->GetPlayer();
    if (now - player->lastLagReport > 10) { // Spam prevention
        TC_LOG_INFO("misc", "[LAG] Player %s (GUID: %u - IP: %s) reported lag - Current timediff: %u",
            player->GetName().c_str(), player->GetGUIDLow(), GetSession()->GetRemoteAddress().c_str(), sWorld->GetUpdateTime());
        player->lastLagReport = now;
    }

    return true;
}

bool ChatHandler::HandleBattlegroundCommand(const char* args)
{
    Player* p = m_session->GetPlayer();
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

    m_session->_HandleBattlegroundJoin(bgTypeId, 0, asGroup);

    return true;
}

//Summon Player
bool ChatHandler::HandleNamegoCommand(const char* args)
{
    ARGS_CHECK

    std::string name = args;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *target = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
    if (target)
    {
        if (target->IsBeingTeleported() == true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, target->GetName().c_str());
            SetSentErrorMessage(true);
            return false;
        }

        Map* pMap = m_session->GetPlayer()->GetMap();

        if (pMap->IsBattlegroundOrArena())
        {
            // only allow if gm mode is on
            if (!target->IsGameMaster())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, target->GetName().c_str());
                SetSentErrorMessage(true);
                return false;
            }
            // if both players are in different bgs
            else if (target->GetBattlegroundId() && m_session->GetPlayer()->GetBattlegroundId() != target->GetBattlegroundId())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_FROM_BG, target->GetName().c_str());
                SetSentErrorMessage(true);
                return false;
            }
            // all's well, set bg id
            // when porting out from the bg, it will be reset to 0
            target->SetBattlegroundId(m_session->GetPlayer()->GetBattlegroundId(), m_session->GetPlayer()->GetBattlegroundTypeId());
            // remember current position as entry point for return at bg end teleportation
            if (!target->GetMap()->IsBattlegroundOrArena())
                target->SetBattlegroundEntryPoint();
        }
        else if (pMap->IsDungeon())
        {
            Map* cMap = target->GetMap();
            if (cMap->Instanceable() && cMap->GetInstanceId() != pMap->GetInstanceId())
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, target->GetName().c_str());
                SetSentErrorMessage(true);
                return false;
            }

            // we are in instance, and can summon only player in our group with us as lead
            if (!m_session->GetPlayer()->GetGroup() || !target->GetGroup() ||
                (target->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
                (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()))
                // the last check is a bit excessive, but let it be, just in case
            {
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, target->GetName().c_str());
                SetSentErrorMessage(true);
                return false;
            }
        }

        // stop flight if need
        if (target->IsInFlight())
        {
            target->GetMotionMaster()->MovementExpired();
            target->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            target->SaveRecallPosition();

        // before GM
        float x, y, z;
        m_session->GetPlayer()->GetClosePoint(x, y, z, target->GetCombatReach());
        if (target->TeleportTo(m_session->GetPlayer()->GetMapId(), x, y, z, target->GetOrientation()))
        {
            PSendSysMessage(LANG_SUMMONING, target->GetName().c_str(), "");
            if (needReportToTarget(target))
                ChatHandler(target).PSendSysMessage(LANG_SUMMONED_BY, GetName().c_str());
        }
        else {
            PSendSysMessage("Teleportation failed");
        }
    }
    else if (uint64 guid = sCharacterCache->GetCharacterGuidByName(name))
    {
        PSendSysMessage(LANG_SUMMONING, name.c_str(), GetTrinityString(LANG_OFFLINE));

        // in point where GM stay
        Player::SavePositionInDB(m_session->GetPlayer()->GetMapId(),
            m_session->GetPlayer()->GetPositionX(),
            m_session->GetPlayer()->GetPositionY(),
            m_session->GetPlayer()->GetPositionZ(),
            m_session->GetPlayer()->GetOrientation(),
            m_session->GetPlayer()->GetZoneId(),
            guid);
    }
    else
    {
        PSendSysMessage(LANG_NO_PLAYER, args);
        SetSentErrorMessage(true);
    }

    return true;
}

//Teleport to Player
bool ChatHandler::HandleGonameCommand(const char* args)
{
    ARGS_CHECK
    Player* _player = m_session->GetPlayer();

    std::string name = args;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *target = ObjectAccessor::FindConnectedPlayerByName(name.c_str());
    if (target)
    {
        Map* cMap = target->GetMap();
        if (!cMap)
        {
            SendSysMessage("Target is not in any map");
            return true;
        }
        if (cMap->IsBattlegroundOrArena())
        {
            // only allow if gm mode is on
            if (!_player->IsGameMaster())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, target->GetName().c_str());
                SetSentErrorMessage(true);
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
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY, target->GetName().c_str());
                    SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                // we are not in group, let's verify our GM mode
                if (!_player->IsGameMaster())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM, target->GetName().c_str());
                    SetSentErrorMessage(true);
                    return false;
                }
            }

            // if the player or the player's group is bound to another instance
            // the player will not be bound to another one
            InstancePlayerBind* pBind = _player->GetBoundInstance(target->GetMapId(), target->GetDifficulty());
            if (!pBind)
            {
                Group* group = _player->GetGroup();
                // if no bind exists, create a solo bind
                InstanceGroupBind* gBind = group ? group->GetBoundInstance(target->GetDifficulty(), target->GetMapId()) : nullptr;
                if (!gBind)
                    if (InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(target->GetInstanceId()))
                        _player->BindToInstance(save, !save->CanReset());
            }

            _player->SetDifficulty(target->GetDifficulty(), true, false);
        }

        // stop flight if need
        if (_player->IsInFlight())
        {
            _player->GetMotionMaster()->MovementExpired();
            _player->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        // to point to see at target with same orientation
        float x, y, z;
        target->GetContactPoint(m_session->GetPlayer(), x, y, z);

        if (_player->TeleportTo(target->GetMapId(), x, y, z, _player->GetAngle(target), TELE_TO_GM_MODE))
        {
            _player->SetPhaseMask(target->GetPhaseMask(), true);
            PSendSysMessage(LANG_APPEARING_AT, target->GetName().c_str());
            if (_player->IsVisibleGloballyFor(target))
                ChatHandler(target).PSendSysMessage(LANG_APPEARING_TO, _player->GetName().c_str());
        }
        else {
            PSendSysMessage("Teleportation failed");
        }

        return true;
    }

    if (uint64 guid = sCharacterCache->GetCharacterGuidByName(name))
    {
        PSendSysMessage(LANG_APPEARING_AT, name.c_str());

        // to point where player stay (if loaded)
        float x, y, z, o;
        uint32 map;
        bool in_flight;
        if (Player::LoadPositionFromDB(map, x, y, z, o, in_flight, guid))
        {
            // stop flight if need
            if (_player->IsInFlight())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->CleanupAfterTaxiFlight();
            }
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            _player->TeleportTo(map, x, y, z, _player->GetOrientation());
            return true;
        }
    }

    PSendSysMessage(LANG_NO_PLAYER, args);
    SetSentErrorMessage(true);
    return false;
}

// Teleport player to last position
bool ChatHandler::HandleRecallCommand(const char* args)
{
    Player* chr = nullptr;

    if (!*args)
    {
        chr = GetSelectedPlayer();
        if (!chr)
            chr = m_session->GetPlayer();
    }
    else
    {
        std::string name = args;

        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        chr = ObjectAccessor::FindConnectedPlayerByName(name.c_str());

        if (!chr)
        {
            PSendSysMessage(LANG_NO_PLAYER, args);
            SetSentErrorMessage(true);
            return false;
        }
    }

    if (chr->IsBeingTeleported())
    {
        PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if (chr->IsInFlight())
    {
        chr->GetMotionMaster()->MovementExpired();
        chr->CleanupAfterTaxiFlight();
    }

    chr->TeleportTo(chr->m_recallMap, chr->m_recallX, chr->m_recallY, chr->m_recallZ, chr->m_recallO);
    return true;
}

// global announce
bool ChatHandler::HandleAnnounceCommand(const char* args)
{
    ARGS_CHECK

        sWorld->SendWorldText(LANG_SYSTEMMESSAGE, args);
    return true;
}

// announce to logged in GMs
bool ChatHandler::HandleGMAnnounceCommand(const char* args)
{
    ARGS_CHECK

        sWorld->SendGMText(LANG_GM_BROADCAST, args);
    return true;
}

//notification player at the screen
bool ChatHandler::HandleNotifyCommand(const char* args)
{
    ARGS_CHECK

        std::string str = GetTrinityString(LANG_GLOBAL_NOTIFY);
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
    data << str;
    sWorld->SendGlobalMessage(&data);

    return true;
}

//notification GM at the screen
bool ChatHandler::HandleGMNotifyCommand(const char* args)
{
    ARGS_CHECK

        std::string str = GetTrinityString(LANG_GM_NOTIFY);
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
    data << str;
    sWorld->SendGlobalGMMessage(&data);

    return true;
}


bool ChatHandler::HandleGPSCommand(const char* args)
{
    WorldObject *obj = nullptr;
    if (*args)
    {
        std::string name = args;
        if (normalizePlayerName(name))
            obj = ObjectAccessor::FindConnectedPlayerByName(name.c_str());

        if (!obj)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        obj = GetSelectedUnit();

        if (!obj)
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            SetSentErrorMessage(true);
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
    float floor_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());

    GridCoord p = Trinity::ComputeGridCoord(obj->GetPositionX(), obj->GetPositionY());

    int gx = 63 - p.x_coord;
    int gy = 63 - p.y_coord;

    uint32 have_map = GridMap::ExistMap(obj->GetMapId(), gx, gy) ? 1 : 0;
    uint32 have_vmap = GridMap::ExistVMap(obj->GetMapId(), gx, gy) ? 1 : 0;

    if (have_vmap)
    {
        if (map->IsOutdoors(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ()))
            PSendSysMessage("You are outdoors");
        else
            PSendSysMessage("You are indoor");

        uint32 mogpFlags;
        int32 adtId, rootId, groupId;
        WMOAreaTableEntry const* wmoEntry = nullptr;

        if (map->GetAreaInfo(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), mogpFlags, adtId, rootId, groupId))
        {
            if ((wmoEntry = GetWMOAreaTableEntryByTripple(rootId, adtId, groupId)))
                PSendSysMessage(LANG_GPS_WMO_DATA, wmoEntry->Id, wmoEntry->Flags, mogpFlags);
        }
    }
    else PSendSysMessage("no VMAP available for area info");

    PSendSysMessage(LANG_MAP_POSITION,
        obj->GetMapId(), (mapEntry ? mapEntry->name[GetSessionDbcLocale()] : "<unknown>"),
        zone_id, (zoneEntry ? zoneEntry->area_name[GetSessionDbcLocale()] : "<unknown>"),
        area_id, (areaEntry ? areaEntry->area_name[GetSessionDbcLocale()] : "<unknown>"),
        obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
        cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetInstanceId(),
        zone_x, zone_y, ground_z, floor_z, have_map, have_vmap);

    //more correct format for script, you just have to copy/paste !
    PSendSysMessage(LANG_GPS_FOR_SCRIPT, obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());

    //TC_LOG_DEBUG("command","%f, %f, %f, %f", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());

    return true;
}

bool ChatHandler::HandleGPSSCommand(const char* args)
{
    WorldObject *obj = nullptr;
    if (*args)
    {
        std::string name = args;
        if (normalizePlayerName(name))
            obj = ObjectAccessor::FindConnectedPlayerByName(name.c_str());

        if (!obj)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        obj = GetSelectedUnit();

        if (!obj)
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }
    }
    PSendSysMessage("%f %f %f %f", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
    return true;
}

//Send mail by command
bool ChatHandler::HandleSendMailCommand(const char* args)
{
    ARGS_CHECK

        // format: name "subject text" "mail text"

        char* pName = strtok((char*)args, " ");
    if (!pName)
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

    // pName, msgSubject, msgText isn't NUL after prev. check
    std::string name = pName;
    std::string subject = msgSubject;
    std::string text = msgText;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = sCharacterCache->GetCharacterGuidByName(name);
    if (!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // from console show not existed sender
    uint32 sender_guidlo = m_session ? m_session->GetPlayer()->GetGUIDLow() : 0;

    MailMessageType messagetype = MAIL_NORMAL;
    uint32 stationery = MAIL_STATIONERY_GM;
    uint32 itemTextId = !text.empty() ? sObjectMgr->CreateItemText(text) : 0;

    Player *receiver = sObjectMgr->GetPlayer(receiver_guid);

    WorldSession::SendMailTo(receiver, messagetype, stationery, sender_guidlo, GUID_LOPART(receiver_guid), subject, itemTextId, nullptr, 0, 0, MAIL_CHECK_MASK_NONE);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

bool ChatHandler::HandleBlinkCommand(const char* args)
{
    uint32 distance = 0;

    if (args)
        distance = (uint32)atoi(args);

    if (!distance)
        distance = 15;

    Player* player = m_session->GetPlayer();
    if (!player)
        return true;

    float currentX, currentY, currentZ, currentO;
    player->GetPosition(currentX, currentY, currentZ);
    currentO = player->GetOrientation();

    float newX = currentX + cos(currentO) * distance;
    float newY = currentY + sin(currentO) * distance;
    float newZ = currentZ;

    player->TeleportTo(m_session->GetPlayer()->GetMapId(), newX, newY, newZ, currentO);

    return true;
}

//Save all players in the world
bool ChatHandler::HandleSaveAllCommand(const char* /*args*/)
{
    ObjectAccessor::SaveAllPlayers();
    SendSysMessage(LANG_PLAYERS_SAVED);
    return true;
}

//show info of player
bool ChatHandler::HandlePInfoCommand(const char* args)
{
    Player* target = nullptr;
    uint64 targetGUID = 0;

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
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
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
        target = GetSelectedPlayer();
    }

    if (!target && !targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
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
        name = target->GetName();                           // re-read for case GetSelectedPlayer() target
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
        stmt->setUInt32(0, GUID_LOPART(targetGUID));
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (!result)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Field *fields = result->Fetch();
        total_player_time = fields[0].GetUInt32();
        level = fields[1].GetUInt8();
        money = fields[2].GetUInt32();
        accId = fields[3].GetUInt32();
    }

    std::string username = GetTrinityString(LANG_ERROR);
    std::string last_ip = GetTrinityString(LANG_ERROR);
    uint8 security = 0;
    std::string last_login = GetTrinityString(LANG_ERROR);
    std::string current_mail = GetTrinityString(LANG_ERROR);
    std::string reg_mail = GetTrinityString(LANG_ERROR);

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

        if (!m_session || m_session->GetSecurity() >= security)
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

    PSendSysMessage(LANG_PINFO_ACCOUNT, (target ? "" : GetTrinityString(LANG_OFFLINE)), name.c_str(), GUID_LOPART(targetGUID), username.c_str(), accId, security, last_ip.c_str(), last_login.c_str(), latency);

    std::string timeStr = secsToTimeString(total_player_time, true, true);
    uint32 gold = money / GOLD;
    uint32 silv = (money % GOLD) / SILVER;
    uint32 copp = (money % GOLD) % SILVER;
    PSendSysMessage(LANG_PINFO_LEVEL, timeStr.c_str(), level, gold, silv, copp);

    PSendSysMessage("Current mail: %s", current_mail.c_str());

    if (py && strncmp(py, "rep", 3) == 0)
    {
        if (!target)
        {
            // rep option not implemented for offline case
            SendSysMessage(LANG_PINFO_NO_REP);
            SetSentErrorMessage(true);
            return false;
        }

        char const* FactionName;
        for (FactionStateList::const_iterator itr = target->m_factions.begin(); itr != target->m_factions.end(); ++itr)
        {
            FactionEntry const *factionEntry = sFactionStore.LookupEntry(itr->second.ID);
            if (factionEntry)
                FactionName = factionEntry->name[GetSessionDbcLocale()];
            else
                FactionName = "#Not found#";
            ReputationRank rank = target->GetReputationRank(factionEntry);
            std::string rankName = GetTrinityString(ReputationRankStrIndex[rank]);
            std::ostringstream ss;
            ss << itr->second.ID << ": |cffffffff|Hfaction:" << itr->second.ID << "|h[" << FactionName << "]|h|r " << rankName << "|h|r (" << target->GetReputation(factionEntry) << ")";

            if (itr->second.Flags & FACTION_FLAG_VISIBLE)
                ss << GetTrinityString(LANG_FACTION_VISIBLE);
            if (itr->second.Flags & FACTION_FLAG_AT_WAR)
                ss << GetTrinityString(LANG_FACTION_ATWAR);
            if (itr->second.Flags & FACTION_FLAG_PEACE_FORCED)
                ss << GetTrinityString(LANG_FACTION_PEACE_FORCED);
            if (itr->second.Flags & FACTION_FLAG_HIDDEN)
                ss << GetTrinityString(LANG_FACTION_HIDDEN);
            if (itr->second.Flags & FACTION_FLAG_INVISIBLE_FORCED)
                ss << GetTrinityString(LANG_FACTION_INVISIBLE_FORCED);
            if (itr->second.Flags & FACTION_FLAG_INACTIVE)
                ss << GetTrinityString(LANG_FACTION_INACTIVE);

            SendSysMessage(ss.str().c_str());
        }
    }
    return true;
}

bool ChatHandler::HandleReloadAllPaths(const char* args)
{
    ARGS_CHECK

    uint32 id = atoi(args);

    if (!id)
        return false;

    PSendSysMessage("%s%s|r|cff00ffff%u|r", "|cff00ff00", "Loading Path: ", id);
    sWaypointMgr->ReloadPath(id);
    return true;
}


//rename characters
bool ChatHandler::HandleRenameCommand(const char* args)
{
    Player* target = nullptr;
    uint64 targetGUID = 0;
    std::string oldname;

    char* px = strtok((char*)args, " ");

    if (px)
    {
        oldname = px;

        if (!normalizePlayerName(oldname))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        target = ObjectAccessor::FindConnectedPlayerByName(oldname.c_str());

        if (!target)
            targetGUID = sCharacterCache->GetCharacterGuidByName(oldname);
    }

    if (!target && !targetGUID)
    {
        target = GetSelectedPlayer();
    }

    if (!target && !targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (target)
    {
        PSendSysMessage(LANG_RENAME_PLAYER, target->GetName().c_str());
        target->SetAtLoginFlag(AT_LOGIN_RENAME);
    }
    else
    {
        PSendSysMessage(LANG_RENAME_PLAYER_GUID, oldname.c_str(), GUID_LOPART(targetGUID));
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '1' WHERE guid = '%u'", GUID_LOPART(targetGUID));
    }

    return true;
}

/* Syntax : .arenarename <playername> <type> <newname> */
bool ChatHandler::HandleRenameArenaTeamCommand(const char* args)
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
        SendSysMessage("Invalid team type (must be 2, 3 or 5).");
        return true;
    }

    uint64 targetGUID = 0;
    std::string stringName = playerName;

    targetGUID = sCharacterCache->GetCharacterGuidByName(stringName);
    if (!targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return true;
    }

    CharacterCacheEntry const* playerData = sCharacterCache->GetCharacterCacheByGuid(GUID_LOPART(targetGUID));
    if (!playerData)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return true;
    }

    uint32 arenateamid = playerData->arenaTeamId[dataIndex];
    if (!arenateamid)
    {
        PSendSysMessage("Team not found. Also double check your team type.");
        return true;
    }

    CharacterDatabase.PQuery("UPDATE arena_team SET name = '%s' WHERE arenateamid = '%u'", newName, arenateamid);
    // + Update within memory ?

    PSendSysMessage("Team (id %u) name changed to \"%s\"", arenateamid, newName);

    ArenaTeam* team = sObjectMgr->GetArenaTeamById(arenateamid);
    if (team)
    {
        team->SetName(newName);
    }
    else {
        PSendSysMessage("Could not change team name in current memory, the change will be effective only at next server start");
    }

    return true;
}

bool ChatHandler::HandleCombatStopCommand(const char* args)
{
    Player *player;

    if (*args)
    {
        std::string playername = args;

        if (!normalizePlayerName(playername))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = ObjectAccessor::FindConnectedPlayerByName(playername.c_str());

        if (!player)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        player = GetSelectedPlayerOrSelf();
    }

    player->CombatStop();
    player->GetHostileRefManager().deleteReferences();
    return true;
}

bool ChatHandler::HandleRepairitemsCommand(const char* /*args*/)
{
    Player *target = GetSelectedPlayerOrSelf();

    if (!target)
    {
        PSendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // Repair items
    target->DurabilityRepairAll(false, 0, false);

    PSendSysMessage(LANG_YOU_REPAIR_ITEMS, target->GetName().c_str());
    if (needReportToTarget(target))
        ChatHandler(target).PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, GetName().c_str());
    return true;
}

bool ChatHandler::HandleChanBan(const char* args)
{
    ARGS_CHECK

        std::string channelNamestr = "world";

    char* charname = strtok((char*)args, " ");
    if (!charname)
        return false;

    std::string charNamestr = charname;

    if (!normalizePlayerName(charNamestr)) {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(charNamestr.c_str());
    if (!accountid)
    {
        PSendSysMessage("No account found for player name: %s.", charNamestr.c_str());
        return true;
    }

    char* duration = strtok(nullptr, " ");
    if (!duration || !atoi(duration))
        return false;

    char* reason = strtok(nullptr, "");
    std::string reasonstr;
    if (!reason)
    {
        SendSysMessage("You must specify a reason.");
        return false;
    }

    reasonstr = reason;

    LogsDatabase.EscapeString(reasonstr);

    uint32 durationSecs = TimeStringToSecs(duration);

    CharacterDatabase.PExecute("INSERT INTO channel_ban VALUES (%u, %lu, \"%s\", \"%s\")", accountid, time(nullptr) + durationSecs, channelNamestr.c_str(), reasonstr.c_str());
    LogsDatabaseAccessor::Sanction(m_session, accountid, 0, SANCTION_CHAN_BAN, durationSecs, reasonstr);

    PSendSysMessage("You banned %s from World channed with the reason: %s.", charNamestr.c_str(), reasonstr.c_str());

    Player *player = ObjectAccessor::FindConnectedPlayerByName(charNamestr.c_str());
    if (!player)
        return true;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam())) {
        if (Channel *chn = cMgr->GetChannel(channelNamestr.c_str(), player)) {
            chn->Kick(m_session ? m_session->GetPlayer()->GetGUID() : 0, player->GetName());
            chn->AddNewGMBan(accountid, time(nullptr) + durationSecs);
            //TODO translate
            ChatHandler(player).PSendSysMessage("You have been banned from World channel with this reason: %s", reasonstr.c_str());
            //ChatHandler(player).PSendSysMessage("Vous avez été banni du channel world avec la raison suivante : %s", reasonstr.c_str());
        }
    }

    return true;
}

bool ChatHandler::HandleChanUnban(const char* args)
{
    ARGS_CHECK

        std::string channelNamestr = "world";

    char* charname = strtok((char*)args, "");
    if (!charname)
        return false;

    std::string charNamestr = charname;

    if (!normalizePlayerName(charNamestr)) {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(charNamestr.c_str());
    if (!accountid)
    {
        PSendSysMessage("No account found for player %s.", charNamestr.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE channel_ban SET expire = %lu WHERE accountid = %u AND expire > %lu", time(nullptr), accountid, time(nullptr));

    if (m_session)
    {
        if (ChannelMgr* cMgr = channelMgr(m_session->GetPlayer()->GetTeam())) {
            if (Channel *chn = cMgr->GetChannel(channelNamestr.c_str(), m_session->GetPlayer()))
                chn->RemoveGMBan(accountid);
        }
    }

    LogsDatabaseAccessor::RemoveSanction(m_session, accountid, 0, "", SANCTION_CHAN_BAN);

    PSendSysMessage("Player %s is unbanned.", charNamestr.c_str());
    if (Player *player = ObjectAccessor::FindConnectedPlayerByName(charNamestr.c_str()))
    {
        //TODO translate
        ChatHandler(player).PSendSysMessage("You are now unbanned from the World channel.");
        //ChatHandler(player).PSendSysMessage("Vous êtes maintenant débanni du channel world.");
    }

    return true;
}

bool ChatHandler::HandleChanInfoBan(const char* args)
{
    ARGS_CHECK

        std::string channelNamestr = "world";

    char* charname = strtok((char*)args, "");
    if (!charname)
        return false;

    std::string charNamestr = charname;

    if (!normalizePlayerName(charNamestr)) {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sCharacterCache->GetCharacterAccountIdByName(charNamestr.c_str());
    if (!accountid)
    {
        PSendSysMessage("No account found for player %s", charNamestr.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT reason, FROM_UNIXTIME(expire), expire FROM channel_ban WHERE accountid = %u AND channel = '%s'", accountid, channelNamestr.c_str());
    if (result) {
        do {
            Field *fields = result->Fetch();
            std::string reason = fields[0].GetString();
            std::string expiredate = fields[1].GetString();
            time_t expiretimestamp = time_t(fields[2].GetUInt32());

            PSendSysMessage("Reason: \"%s\" - Expires in: %s %s", reason.c_str(), expiredate.c_str(), (expiretimestamp > time(nullptr)) ? "(actif)" : "");
        } while (result->NextRow());
    }
    else {
        PSendSysMessage("No ban on this player.");
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleUpdatePvPTitleCommand(const char *args)
{
    if (Player * player = GetSession()->GetPlayer()) {
        player->UpdateKnownPvPTitles();
        return true;
    }
    return false;
}

bool ChatHandler::HandleCharacterDeleteCommand(const char* args)
{
    ARGS_CHECK

        char *character_name_str = strtok((char*)args, " ");
    if (!character_name_str)
        return false;

    std::string character_name = character_name_str;
    if (!normalizePlayerName(character_name))
        return false;

    uint64 character_guid;
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
            PSendSysMessage(LANG_NO_PLAYER, character_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        account_id = sCharacterCache->GetCharacterAccountIdByGuid(character_guid);
    }

    std::string account_name;
    sAccountMgr->GetName(account_id, account_name);

    Player::DeleteFromDB(character_guid, account_id, true);
    PSendSysMessage(LANG_CHARACTER_DELETED, character_name.c_str(), GUID_LOPART(character_guid), account_name.c_str(), account_id);
    return true;
}


//Visually copy stuff from player given to target player (fade off at disconnect like a normal morph)
bool ChatHandler::HandleCopyStuffCommand(char const * args)
{
    ARGS_CHECK

    std::string fromPlayerName = args;
    Player* fromPlayer = nullptr;
    Player* toPlayer = GetSelectedPlayerOrSelf();

    if (normalizePlayerName(fromPlayerName))
        fromPlayer = ObjectAccessor::FindConnectedPlayerByName(fromPlayerName.c_str());

    if (!fromPlayer || !toPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
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
