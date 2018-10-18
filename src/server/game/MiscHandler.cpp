#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "GossipDef.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "BigNumber.h"
#include "SHA1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "BattleGround.h"
#include "OutdoorPvP.h"
#include "SpellAuras.h"
#include "Pet.h"
#include "SocialMgr.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"
#include "WhoListStorage.h"
#include "GameTime.h"
#include "CinematicMgr.h"

void WorldSession::HandleRepopRequestOpcode( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_REPOP_REQUEST Message");

    if(GetPlayer()->IsAlive()||GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if(GetPlayer()->GetDeathState() == JUST_DIED)
    {
        TC_LOG_ERROR("network","HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated", GetPlayer()->GetName().c_str(), GetPlayer()->GetGUID().GetCounter());
        GetPlayer()->KillPlayer();
    }

    //this is spirit release confirm?
    GetPlayer()->RemovePet(nullptr,PET_SAVE_NOT_IN_SLOT, true);
    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->SetIsRepopPending(true);
}

void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket& recvData)
{
    //TC_LOG_DEBUG("network", "WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 gossipListId;
    uint32 menuId;
    ObjectGuid guid;
    std::string code = "";

    recvData >> guid >> menuId >> gossipListId;

    if (!_player->PlayerTalkClass->GetGossipMenu().GetItem(gossipListId))
    {
        recvData.rfinish();
        return;
    }

    if (_player->PlayerTalkClass->IsGossipOptionCoded(gossipListId))
        recvData >> code;

    // Prevent cheating on C++ scripted menus
    if (_player->PlayerTalkClass->GetGossipMenu().GetSenderGUID() != guid)
        return;

    Creature* unit = nullptr;
    GameObject* go = nullptr;
    if (guid.IsCreatureOrVehicle())
    {
        unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_GOSSIP);
        if (!unit)
        {
            TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - %u not found or you can't interact with him.", guid.GetCounter());
            return;
        }
    }
    else if (guid.IsGameObject())
    {
        go = _player->GetGameObjectIfCanInteractWith(guid);
        if (!go)
        {
            TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - %u not found.", guid.GetCounter());
            return;
        }
    }
    else
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - unsupported %u.", guid.GetCounter());
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if ((unit && unit->GetScriptId() != unit->LastUsedScriptID) || (go && go->GetScriptId() != go->LastUsedScriptID))
    {
        TC_LOG_DEBUG("network", "WORLD: HandleGossipSelectOptionOpcode - Script reloaded while in use, ignoring and set new scipt id");
        if (unit)
            unit->LastUsedScriptID = unit->GetScriptId();
        if (go)
            go->LastUsedScriptID = go->GetScriptId();
        _player->PlayerTalkClass->SendCloseGossip();
        return;
    }
    if (!code.empty())
    {
        if (unit)
        {
            if (!unit->AI()->GossipSelectCode(_player, menuId, gossipListId, code.c_str()))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
        {
            if (!go->AI()->GossipSelectCode(_player, menuId, gossipListId, code.c_str()))
                _player->OnGossipSelect(go, gossipListId, menuId);
        }
    }
    else
    {
        if (unit)
        {
            if (!unit->AI()->GossipSelect(_player, menuId, gossipListId))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
        {
            if (!go->AI()->GossipSelect(_player, menuId, gossipListId))
                _player->OnGossipSelect(go, gossipListId, menuId);
        }
    }
}

void WorldSession::HandleWhoOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_WHO Message");

    uint32 matchCount = 0;

    uint32 levelMin, levelMax, racemask, classmask, zonesCount, strCount;
    uint32 zoneids[10];                                     // 10 is client limit
    std::string packetPlayerName, packetGuildName;

    recvData >> levelMin;                                 // maximal player level, default 0
    recvData >> levelMax;                                 // minimal player level, default 100 (MAX_LEVEL)
    recvData >> packetPlayerName;                               // player name, case sensitive...

    recvData >> packetGuildName;                                // guild name, case sensitive...

    recvData >> racemask;                                  // race mask
    recvData >> classmask;                                 // class mask
    recvData >> zonesCount;                               // zones count, client limit=10 (2.0.10)

    if(zonesCount > 10)
        return;                                             // can't be received from real client or broken packet

    for(uint32 i = 0; i < zonesCount; i++)
    {
        uint32 temp;
        recvData >> temp;                                  // zone id, 0 if zone is unknown...
        zoneids[i] = temp;
    }

    recvData >> strCount;                                 // user entered strings count, client limit=4 (checked on 2.0.10)

    if(strCount > 4)
        return;                                             // can't be received from real client or broken packet

//    TC_LOG_DEBUG("network", "Minlvl %u, maxlvl %u, name %s, guild %s, racemask %u, classmask %u, zones %u, strings %u", levelMin, levelMax, packetPlayerName.c_str(), packetGuildName.c_str(), racemask, classmask, zonesCount, strCount);

    std::wstring str[4];                                    // 4 is client limit
    for(uint32 i = 0; i < strCount; i++)
    {
        std::string temp;
        recvData >> temp;                                  // user entered string, it used as universal search pattern(guild+player name)?

        if(!Utf8toWStr(temp,str[i]))
            continue;

        wstrToLower(str[i]);
    }

    std::wstring wplayer_name;
    std::wstring wguild_name;
    if(!(Utf8toWStr(packetPlayerName, wplayer_name) && Utf8toWStr(packetGuildName, wguild_name)))
        return;

    wstrToLower(wplayer_name);
    wstrToLower(wguild_name);

    // client send in case not set max level value 100 but mangos support 255 max level,
    // update it to show GMs with characters after 100 level
    if(levelMax >= MAX_LEVEL)
        levelMax = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    uint32 security = GetSecurity();
    bool allowTwoSideWhoList = sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    uint32 gmLevelInWhoList  = sWorld->getConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);
    uint32 displaycount = 0;

    WorldPacket data( SMSG_WHO, 500 );                       // guess size
    data << uint32(matchCount); //placeholder, will be overriden later
    data << uint32(displaycount);

    WhoListInfoVector const& whoList = sWhoListStorageMgr->GetWhoList();
    for (WhoListPlayerInfo const& target : whoList)
    {
        if (security == SEC_PLAYER)
        {
            // player can see member of other team only if CONFIG_ALLOW_TWO_SIDE_WHO_LIST
            if (target.GetTeam() != team && !allowTwoSideWhoList )
                continue;

            // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
            if ((target.GetSecurity() > gmLevelInWhoList))
                continue;
        }

        // check if target is globally visible for player
        if (_player->GetGUID() != target.GetGuid() && !target.IsVisible())
            if (AccountMgr::IsPlayerAccount(_player->GetSession()->GetSecurity()) || target.GetSecurity() > _player->GetSession()->GetSecurity())
                continue;

        /* Older code... better but I don't see how to implement it with WhoList
        if (!(target.IsVisibleGloballyFor(_player)))
            continue;
        */

        // check if target's level is in level range
        uint32 lvl = target.GetLevel();
        if (lvl < levelMin || lvl > levelMax)
            continue;

        // check if class matches classmask
        uint32 class_ = target.GetClass();
        if (!(classmask & (1 << class_)))
            continue;

        // check if race matches racemask
        uint32 race = target.GetRace();
        if (!(racemask & (1 << race)))
            continue;

        uint32 playerZoneId = target.GetZoneId();
        uint8 gender = target.GetGender();

        bool z_show = true;
        for(uint32 i = 0; i < zonesCount; i++)
        {
            if(zoneids[i] == playerZoneId)
            {
                z_show = true;
                break;
            }

            z_show = false;
        }
        if (!z_show)
            continue;

        std::string pname = target.GetPlayerName();
        std::wstring wpname;
        if(!Utf8toWStr(pname,wpname))
            continue;
        wstrToLower(wpname);

        if (!(wplayer_name.empty() || wpname.find(wplayer_name) != std::wstring::npos))
            continue;

        std::string gname = target.GetGuildName();
        std::wstring wgname;
        if(!Utf8toWStr(gname,wgname))
            continue;
        wstrToLower(wgname);

        if (!(wguild_name.empty() || wgname.find(wguild_name) != std::wstring::npos))
            continue;

        std::string aname;
        if(AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(playerZoneId))
            aname = areaEntry->area_name[GetSessionDbcLocale()];

        bool s_show = true;
        for(uint32 i = 0; i < strCount; i++)
        {
            if (!str[i].empty())
            {
                if (wgname.find(str[i]) != std::wstring::npos ||
                    wpname.find(str[i]) != std::wstring::npos ||
                    Utf8FitTo(aname, str[i]) )
                {
                    s_show = true;
                    break;
                }
                s_show = false;
            }
        }
        if (!s_show)
            continue;


        ++matchCount;
        if (matchCount >= 50) // 49 is maximum player count sent to client - apparently can be overriden but is said unstable
            continue; //continue counting, just do not insert

        data << pname;                                    // player name
        data << gname;                                    // guild name
        data << uint32(lvl);                              // player level
        data << uint32(class_);                           // player class
        data << uint32(race);                             // player race
        data << uint8(gender);                            // new 2.4.0
        data << uint32(playerZoneId);                     // player zone id

        ++displaycount;
    }

    data.put(0, displaycount);                             // insert right count, count of matches
    data.put(4, matchCount);                               // insert right count, count displayed

    SendPacket(&data);
}

void WorldSession::HandleLogoutRequestOpcode( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_LOGOUT_REQUEST Message, security - %u", GetSecurity());

    if (ObjectGuid lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);

    bool canLogoutInCombat = GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);

    bool instantLogout = canLogoutInCombat ||
        GetPlayer()->IsInFlight() || GetSecurity() >= sWorld->getConfig(CONFIG_INSTANT_LOGOUT);

    uint8 reason = 0; //reason but... seems at least on BC, they all show "You can't logout now."
    if (GetPlayer()->IsInCombat() && !canLogoutInCombat)
        reason = 1;
    else if (GetPlayer()->HasUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING | MOVEMENTFLAG_FALLING_FAR)) // is jumping or falling
        reason = 3;
    else if (GetPlayer()->duel || GetPlayer()->HasAura(9454)) // is dueling or frozen by GM via freeze command
        reason = 0xC;  //not right id, need to get the correct value


    WorldPacket data(SMSG_LOGOUT_RESPONSE, 4+1);
    data << uint32(reason);
    data << uint8(instantLogout);
    SendPacket(&data);

    if (reason)
    {
        LogoutRequest(0);
        return;
    }

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in mangosd.conf
    if (instantLogout)
    {
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if(GetPlayer()->CanFreeMove())
    {
        if (GetPlayer()->GetStandState() == UNIT_STAND_STATE_STAND)
           GetPlayer()->SetStandState(PLAYER_STATE_SIT);

        GetPlayer()->SetRooted(true);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    LogoutRequest(WorldGameTime::GetGameTime());
}

void WorldSession::HandlePlayerLogoutOpcode( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_PLAYER_LOGOUT Message");
}

void WorldSession::HandleLogoutCancelOpcode( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_LOGOUT_CANCEL Message");

    // Player have already logged out serverside, too late to cancel
    if (!GetPlayer())
        return;

    LogoutRequest(0);

    WorldPacket data( SMSG_LOGOUT_CANCEL_ACK, 0 );
    SendPacket( &data );

    // not remove flags if can't free move - its not set in Logout request code.
    if(GetPlayer()->CanFreeMove())
    {
        //!we can move again
        GetPlayer()->SetRooted(false);

        //! Stand Up
        GetPlayer()->SetStandState(PLAYER_STATE_NONE);

        //! DISABLE_ROTATE
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    //TC_LOG_DEBUG("network", "WORLD: Sent SMSG_LOGOUT_CANCEL_ACK Message");
}

void WorldSession::HandleTogglePvP( WorldPacket & recvData )
{
    // this opcode can be used in two ways: Either set explicit new status or toggle old status
    if(recvData.size() == 1)
    {
        bool newPvPStatus;
        recvData >> newPvPStatus;
        if(!newPvPStatus || !GetPlayer()->IsInDuelArea()) //can only be set active outside pvp zone
            GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, newPvPStatus);
    }
    else
    {
        if(!GetPlayer()->IsInDuelArea())
            GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
        else
            GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, false);
    }

    if(GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
    {
        if(!GetPlayer()->IsPvP() || GetPlayer()->pvpInfo.endTimer != 0)
            GetPlayer()->UpdatePvP(true, true);
    }
    else
    {
        if(!GetPlayer()->pvpInfo.IsHostile && GetPlayer()->IsPvP())
            GetPlayer()->pvpInfo.endTimer = GetPlayer()->GetMap()->GetGameTime();     // start toggle-off
    }
}

void WorldSession::HandleZoneUpdateOpcode( WorldPacket & recvData )
{
    uint32 newZone;
    recvData >> newZone;

    //TC_LOG_DEBUG("network","WORLD: Recvd ZONE_UPDATE: %u", newZone);

    // use server side data, but only after update the player position. See Player::UpdatePosition().
    GetPlayer()->SetNeedsZoneUpdate(true);
}

void WorldSession::HandleSetTargetOpcode( WorldPacket & recvData )
{
    // When this packet send?

    ObjectGuid guid ;
    recvData >> guid;

    _player->SetSelection(guid);
}

void WorldSession::HandleSetSelectionOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;

    _player->SetSelection(guid);

    Unit* unit = ObjectAccessor::GetUnit(*_player, guid);
    if (_player->HaveSpectators())
    {
        if (Battleground *bg = _player->GetBattleground())
        {
            if (unit && bg->isSpectator(unit->GetGUID()))
                return;
        }
        SpectatorAddonMsg msg;
        msg.SetPlayer(_player->GetName());
        msg.SetTarget(unit ? unit->GetName() : "0");
        _player->SendSpectatorAddonMsgToBG(msg);
    }
}

void WorldSession::HandleStandStateChangeOpcode(WorldPacket & recvData)
{
    //sun: affect moved unit and not player
    Unit* moved = GetAllowedActiveMover();
    if(!moved || !moved->IsAlive())
        return;

    uint8 animstate;
    recvData >> animstate;

    if (moved->GetStandState() != animstate)
        moved->SetStandState(animstate);
}

void WorldSession::HandleBugOpcode( WorldPacket & recvData )
{
    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recvData >> suggestion >> contentlen >> content;
    recvData >> typelen >> type;

    CharacterDatabase.EscapeString(type);
    CharacterDatabase.EscapeString(content);
    CharacterDatabase.PExecute ("INSERT INTO bugreport (type,content) VALUES('%s', '%s')", type.c_str( ), content.c_str( ));
}

void WorldSession::HandleReclaimCorpseOpcode(WorldPacket &recvData)
{
    //TC_LOG_DEBUG("network","WORLD: Received CMSG_RECLAIM_CORPSE");
    if (GetPlayer()->IsAlive())
        return;

    // do not allow corpse reclaim in arena
    if (GetPlayer()->InArena())
        return;

    // body not released yet
    if(!GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    Corpse *corpse = GetPlayer()->GetCorpse();

    if (!corpse )
        return;

    // prevent resurrect before 30-sec delay after body release not finished
    if(GetPlayer()->GetDeathTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType()==CORPSE_RESURRECTABLE_PVP) > GetPlayer()->GetMap()->GetGameTime())
        return;

    float dist = corpse->GetDistance2d(GetPlayer());
    if (dist > CORPSE_RECLAIM_RADIUS)
        return;

    ObjectGuid guid;
    recvData >> guid;

    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleground() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();

    GetPlayer()->SaveToDB();
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recvData)
{
    //TC_LOG_DEBUG("network","WORLD: Received CMSG_RESURRECT_RESPONSE");

    if(GetPlayer()->IsAlive())
        return;

    ObjectGuid guid;
    uint8 status;
    recvData >> guid;
    recvData >> status;

    if(status == 0)
    {
        GetPlayer()->clearResurrectRequestData();           // reject
        return;
    }

    if(!GetPlayer()->isRessurectRequestedBy(guid))
        return;

    GetPlayer()->RessurectUsingRequestData();
    GetPlayer()->SaveToDB();
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recvData)
{
    uint32 triggerId;
    recvData >> triggerId;

    //TC_LOG_DEBUG("network", "CMSG_AREATRIGGER. Trigger ID: %u", triggerId);

    if(GetPlayer()->IsGameMaster())
        SendAreaTriggerMessage("Entered areatrigger %u.", triggerId);

    if(GetPlayer()->IsInFlight())
        return;

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(triggerId);
    if(!atEntry)
    {
        TC_LOG_ERROR("network","Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID:%u",GetPlayer()->GetName().c_str(),GetPlayer()->GetGUID().GetCounter(), triggerId);
        return;
    }

    if (GetPlayer()->GetMapId()!=atEntry->mapid)
    {
        TC_LOG_ERROR("network","Player '%s' (GUID: %u) too far (trigger map: %u player map: %u), ignore Area Trigger ID: %u", GetPlayer()->GetName().c_str(), atEntry->mapid, GetPlayer()->GetMapId(), GetPlayer()->GetGUID().GetCounter(), triggerId);
        return;
    }

    // delta is safe radius
    const float delta = 5.0f;
    // check if player in the range of areatrigger
    Player* pl = GetPlayer();

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist = pl->GetDistance(atEntry->x,atEntry->y,atEntry->z);
        if(dist > atEntry->radius + delta)
        {
            TC_LOG_ERROR("network","Player '%s' (GUID: %u) too far (radius: %f distance: %f), ignore Area Trigger ID: %u",
                pl->GetName().c_str(), pl->GetGUID().GetCounter(), atEntry->radius, dist, triggerId);
            return;
        }
    }
    else if (atEntry->id != 4853)
    {
        // we have only extent
        float dx = pl->GetPositionX() - atEntry->x;
        float dy = pl->GetPositionY() - atEntry->y;
        float dz = pl->GetPositionZ() - atEntry->z;
        double es = sin(atEntry->box_orientation);
        double ec = cos(atEntry->box_orientation);
        // calc rotated vector based on extent axis
        double rotateDx = dx*ec - dy*es;
        double rotateDy = dx*es + dy*ec;

        if ((fabs(rotateDx) > atEntry->box_x / 2 + delta) ||
                (fabs(rotateDy) > atEntry->box_y / 2 + delta) ||
                (fabs(dz) > atEntry->box_z / 2 + delta)) {
            return;
        }
    }

    if(sScriptMgr->OnAreaTrigger(GetPlayer(), atEntry))
        return;

    uint32 quest_id = sObjectMgr->GetQuestForAreaTrigger( triggerId );
    if( quest_id && GetPlayer()->IsAlive() && GetPlayer()->IsActiveQuest(quest_id) )
    {
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(quest_id);
        if( pQuest )
        {
            if(GetPlayer()->GetQuestStatus(quest_id) == QUEST_STATUS_INCOMPLETE)
                GetPlayer()->AreaExploredOrEventHappens( quest_id );
        }
    }

    if(sObjectMgr->IsTavernAreaTrigger(triggerId))
    {
        // set resting flag we are in the inn
        GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        GetPlayer()->InnEnter(GetPlayer()->GetMap()->GetGameTime(), atEntry->mapid, atEntry->x, atEntry->y, atEntry->z);
        GetPlayer()->SetRestType(REST_TYPE_IN_TAVERN);

        if(sWorld->IsFFAPvPRealm())
            GetPlayer()->RemoveFlag(PLAYER_FLAGS,PLAYER_FLAGS_FFA_PVP);

        return;
    }

    if(GetPlayer()->InBattleground())
    {
        Battleground* bg = GetPlayer()->GetBattleground();
        if(bg)
            if(bg->GetStatus() == STATUS_IN_PROGRESS)
                bg->HandleAreaTrigger(GetPlayer(), triggerId);

        return;
    }

    if(OutdoorPvP * pvp = GetPlayer()->GetOutdoorPvP())
    {
        if(pvp->HandleAreaTrigger(_player, triggerId))
            return;
    }

    // NULL if all values default (non teleport trigger)
    AreaTrigger const* at = sObjectMgr->GetAreaTrigger(triggerId);
    if(!at)
        return;

    bool teleported = false;
    if (pl->GetMapId() != at->target_mapId)
    {
        Map::EnterState denyReason = sMapMgr->PlayerCannotEnter(at->target_mapId, pl, false);
        if (denyReason != Map::CAN_ENTER)
        {
            bool reviveAtTrigger = false; // should we revive the player if he is trying to enter the correct instance?
            switch (denyReason)
            {
            case Map::CANNOT_ENTER_NO_ENTRY:
                TC_LOG_DEBUG("maps", "MAP: Player '%s' attempted to enter map with id %d which has no entry", pl->GetName().c_str(), at->target_mapId);
                break;
            case Map::CANNOT_ENTER_UNINSTANCED_DUNGEON:
                TC_LOG_DEBUG("maps", "MAP: Player '%s' attempted to enter dungeon map %d but no instance template was found", pl->GetName().c_str(), at->target_mapId);
                break;
            case Map::CANNOT_ENTER_DIFFICULTY_UNAVAILABLE:
                TC_LOG_DEBUG("maps", "MAP: Player '%s' attempted to enter instance map %d but the requested difficulty was not found", pl->GetName().c_str(), at->target_mapId);
                if (MapEntry const* entry = sMapStore.LookupEntry(at->target_mapId))
#ifdef LICH_KING
                    pl->SendTransferAborted(entry->MapID, TRANSFER_ABORT_DIFFICULTY, player->GetDifficulty(entry->IsRaid()));
#else
                    pl->SendTransferAborted(entry->MapID, TRANSFER_ABORT_DIFFICULTY2);
#endif
                break;
            case Map::CANNOT_ENTER_NOT_IN_RAID:
            {
                WorldPacket data(SMSG_RAID_GROUP_ONLY, 4 + 4);
                data << uint32(0);
#ifdef LICH_KING
                data << uint32(2); // You must be in a raid group to enter this instance.
#else
                data << uint32(1); // You must be in a raid group to enter this instance.
#endif
                pl->GetSession()->SendPacket(&data);
                TC_LOG_DEBUG("maps", "MAP: Player '%s' must be in a raid group to enter instance map %d", pl->GetName().c_str(), at->target_mapId);
                reviveAtTrigger = true;
                break;
            }
            case Map::CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE:
            {
                /* TC
                WorldPacket data(SMSG_CORPSE_NOT_IN_INSTANCE);
                pl->GetSession()->SendPacket(&data);
                */
                ChatHandler(pl->GetSession()).PSendSysMessage("Your corpse is in a different instance.");
                TC_LOG_DEBUG("maps", "MAP: Player '%s' does not have a corpse in instance map %d and cannot enter", pl->GetName().c_str(), at->target_mapId);
                break;
            }
            case Map::CANNOT_ENTER_INSTANCE_BIND_MISMATCH:
                if (MapEntry const* entry = sMapStore.LookupEntry(at->target_mapId))
                {
                    char const* mapName = entry->name[pl->GetSession()->GetSessionDbcLocale()];
                    TC_LOG_DEBUG("maps", "MAP: Player '%s' cannot enter instance map '%s' because their permanent bind is incompatible with their group's", pl->GetName().c_str(), mapName);
                    // is there a special opcode for this?
                    // @todo figure out how to get player localized difficulty string (e.g. "10 player", "Heroic" etc)
                    //TC ChatHandler(pl->GetSession()).PSendSysMessage(player->GetSession()->GetTrinityString(LANG_INSTANCE_BIND_MISMATCH), mapName);
                    ChatHandler(pl->GetSession()).PSendSysMessage("You are already locked to %s.", mapName);

                }
                reviveAtTrigger = true;
                break;
            case Map::CANNOT_ENTER_TOO_MANY_INSTANCES:
                pl->SendTransferAborted(at->target_mapId, TRANSFER_ABORT_TOO_MANY_INSTANCES);
                TC_LOG_DEBUG("maps", "MAP: Player '%s' cannot enter instance map %d because he has exceeded the maximum number of instances per hour.", pl->GetName().c_str(), at->target_mapId);
                reviveAtTrigger = true;
                break;
            case Map::CANNOT_ENTER_MAX_PLAYERS:
                pl->SendTransferAborted(at->target_mapId, TRANSFER_ABORT_MAX_PLAYERS);
                reviveAtTrigger = true;
                break;
            case Map::CANNOT_ENTER_ZONE_IN_COMBAT:
                pl->SendTransferAborted(at->target_mapId, TRANSFER_ABORT_ZONE_IN_COMBAT);
                reviveAtTrigger = true;
                break;
            default:
                break;
            }

            if (reviveAtTrigger) // check if the player is touching the areatrigger leading to the map his corpse is on
                if (!pl->IsAlive() && pl->HasCorpse())
                    if (pl->GetCorpseLocation().GetMapId() == at->target_mapId)
                    {
                        pl->ResurrectPlayer(0.5f);
                        pl->SpawnCorpseBones();
                    }

            return;
        }

        if (Group* group = pl->GetGroup())
            if (group->isLFGGroup() && pl->GetMap()->IsDungeon())
                teleported = pl->TeleportToBGEntryPoint();
    }

    if (!teleported)
        GetPlayer()->TeleportTo(at->target_mapId,at->target_X,at->target_Y,at->target_Z,at->target_Orientation,TELE_TO_NOT_LEAVE_TRANSPORT);
}

void WorldSession::HandleUpdateAccountData(WorldPacket &/*recvData*/)
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_UPDATE_ACCOUNT_DATA");
    // TODO
    /* TC CODE

    uint32 type, timestamp, decompressedSize;
    recvData >> type >> timestamp >> decompressedSize;

    TC_LOG_DEBUG("network", "UAD: type %u, time %u, decompressedSize %u", type, timestamp, decompressedSize);

    if (type > NUM_ACCOUNT_DATA_TYPES)
    return;

    if (decompressedSize == 0)                               // erase
    {
    SetAccountData(AccountDataType(type), 0, "");

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4+4);
    data << uint32(type);
    data << uint32(0);
    SendPacket(&data);

    return;
    }

    if (decompressedSize > 0xFFFF)
    {
    recvData.rfinish();                   // unnneded warning spam in this case
    TC_LOG_ERROR("network", "UAD: Account data packet too big, size %u", decompressedSize);
    return;
    }

    ByteBuffer dest;
    dest.resize(decompressedSize);

    uLongf realSize = decompressedSize;
    if (uncompress(dest.contents(), &realSize, recvData.contents() + recvData.rpos(), recvData.size() - recvData.rpos()) != Z_OK)
    {
    recvData.rfinish();                   // unnneded warning spam in this case
    TC_LOG_ERROR("network", "UAD: Failed to decompress account data");
    return;
    }

    recvData.rfinish();                       // uncompress read (recvData.size() - recvData.rpos())

    std::string adata;
    dest >> adata;

    SetAccountData(AccountDataType(type), timestamp, adata);

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4+4);
    data << uint32(type);
    data << uint32(0);
    SendPacket(&data);

    */
}

void WorldSession::HandleRequestAccountData(WorldPacket& /*recvData*/)
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");
    // TODO

    /* TC CODE

    uint32 type;
    recvData >> type;

    TC_LOG_DEBUG("network", "RAD: type %u", type);

    if (type >= NUM_ACCOUNT_DATA_TYPES)
    return;

    AccountData* adata = GetAccountData(AccountDataType(type));

    uint32 size = adata->Data.size();

    uLongf destSize = compressBound(size);

    ByteBuffer dest;
    dest.resize(destSize);

    if (size && compress(dest.contents(), &destSize, (uint8 const*)adata->Data.c_str(), size) != Z_OK)
    {
    TC_LOG_DEBUG("network", "RAD: Failed to compress account data");
    return;
    }

    dest.resize(destSize);

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA, 8+4+4+4+destSize);
    data << uint64(_player ? _player->GetGUID() : ObjectGuid::Empty);
    data << uint32(type);                                   // type (0-7)
    data << uint32(adata->Time);                            // unix time
    data << uint32(size);                                   // decompressed length
    data.append(dest);                                      // compressed data
    SendPacket(&data);
    */
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recvData)
{
    uint8 button, misc, type;
    uint16 action;
    recvData >> button >> action >> misc >> type;

    TC_LOG_DEBUG("network", "CMSG_SET_ACTION_BUTTON Button: %u", button);

    if (action == 0)
        GetPlayer()->removeActionButton(button);
    else
{
        if (type == ACTION_BUTTON_MACRO || type == ACTION_BUTTON_CMACRO) {
            GetPlayer()->addActionButton(button, action, type, misc);
        }
        else if(type==ACTION_BUTTON_SPELL)
        {
            GetPlayer()->addActionButton(button,action,type,misc);
        }
        else if(type==ACTION_BUTTON_ITEM)
        {
            GetPlayer()->addActionButton(button,action,type,misc);
        }
        else
            TC_LOG_ERROR( "network", "MISC: Unknown action button type %u for action %u into button %u", type, action, button );
    }
}

void WorldSession::HandleCompleteCinematic( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_COMPLETE_CINEMATIC");
    //SO WHAT? Add a hook in map script if needed
    GetPlayer()->GetCinematicMgr()->EndCinematic();
}

void WorldSession::HandleNextCinematicCamera( WorldPacket & /*recvData*/ )
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_NEXT_CINEMATIC_CAMERA");
    GetPlayer()->GetCinematicMgr()->BeginCinematic();
}

void WorldSession::HandleSetActionBarToggles(WorldPacket& recvData)
{
    uint8 actionBar;
    recvData >> actionBar;

    if(!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if(actionBar != 0)
            TC_LOG_ERROR("network","WorldSession::HandleSetActionBar in not logged state with value: %u, ignored",uint32(actionBar));
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_ACTION_BAR_TOGGLES, actionBar);
}

void WorldSession::HandlePlayedTime(WorldPacket& recvData)
{
#ifdef LICH_KING
    uint8 unk1;
    recvData >> unk1;                                      // 0 or 1 expected
#endif

    uint32 TotalTimePlayed = GetPlayer()->GetTotalPlayedTime();
    uint32 LevelPlayedTime = GetPlayer()->GetLevelPlayedTime();

    //LK OK
    WorldPacket data(SMSG_PLAYED_TIME, 8);
    data << TotalTimePlayed;
    data << LevelPlayedTime;
#ifdef LICH_KING
    data << uint8(unk1);                                    // 0 - will not show in chat frame (not so unknown then?)
#endif
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    recvData >> guid;

    _player->SetSelection(guid);

    Player* plr = ObjectAccessor::GetPlayer(*_player, guid);
    if(!plr)                                                // wrong player
        return;

    if (!GetPlayer()->IsWithinDistInMap(plr, INSPECT_DISTANCE, false))
        return;

    if (GetPlayer()->IsValidAttackTarget(plr))
        return;

    uint32 talent_points = 0x3D; //bc talent count
    uint32 guid_size = plr->GetPackGUID().size();
    WorldPacket data(SMSG_INSPECT_TALENT, guid_size+4+talent_points);
    data << plr->GetPackGUID();
    data << uint32(talent_points);

    // fill by 0 talents array
    for(uint32 i = 0; i < talent_points; ++i)
        data << uint8(0);

    if(sWorld->getConfig(CONFIG_TALENTS_INSPECTING) || _player->IsGameMaster())
    {
        // find class talent tabs (all players have 3 talent tabs)
        uint32 const* talentTabIds = GetTalentTabPages(plr->GetClass());

        uint32 talentTabPos = 0;                            // pos of first talent rank in tab including all prev tabs
        for(uint32 i = 0; i < 3; ++i)
        {
            uint32 talentTabId = talentTabIds[i];

            // fill by real data
            for(uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
            {
                TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);
                if(!talentInfo)
                    continue;

                // skip another tab talents
                if(talentInfo->TalentTab != talentTabId)
                    continue;

                // find talent rank
                uint32 curtalent_maxrank = 0;
                for(uint32 k = 5; k > 0; --k)
                {
                    if(talentInfo->RankID[k-1] && plr->HasSpell(talentInfo->RankID[k-1]))
                    {
                        curtalent_maxrank = k;
                        break;
                    }
                }

                // not learned talent
                if(!curtalent_maxrank)
                    continue;

                // 1 rank talent bit index
                uint32 curtalent_index = talentTabPos + GetTalentInspectBitPosInTab(talentId);

                uint32 curtalent_rank_index = curtalent_index+curtalent_maxrank-1;

                // slot/offset in 7-bit bytes
                uint32 curtalent_rank_slot7   = curtalent_rank_index / 7;
                uint32 curtalent_rank_offset7 = curtalent_rank_index % 7;

                // rank pos with skipped 8 bit
                uint32 curtalent_rank_index2 = curtalent_rank_slot7 * 8 + curtalent_rank_offset7;

                // slot/offset in 8-bit bytes with skipped high bit
                uint32 curtalent_rank_slot = curtalent_rank_index2 / 8;
                uint32 curtalent_rank_offset =  curtalent_rank_index2 % 8;

                // apply mask
                uint32 val = data.read<uint8>(guid_size + 4 + curtalent_rank_slot);
                val |= (1 << curtalent_rank_offset);
                data.put<uint8>(guid_size + 4 + curtalent_rank_slot, val & 0xFF);
            }

            talentTabPos += GetTalentTabInspectBitSize(talentTabId);
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;
    recvData >> guid;

    Player *player = ObjectAccessor::GetPlayer(*_player, guid);

    if(!player)
    {
        TC_LOG_ERROR("network","InspectHonorStats: player not found...");
        return;
    }

    if (!GetPlayer()->IsWithinDistInMap(player, INSPECT_DISTANCE, false))
        return;

    if (GetPlayer()->IsValidAttackTarget(player))
        return;

    WorldPacket data(MSG_INSPECT_HONOR_STATS, 8+1+4*4);
    data << uint64(player->GetGUID());
    data << uint8(player->GetHonorPoints()); //mangos has GetHighestPvPRankIndex here
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_KILLS));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
    SendPacket(&data);
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recvData)
{
    // TC_LOG_DEBUG("network", "Received opcode CMSG_WHOIS");

    std::string charname;
    recvData >> charname;

    if (GetSecurity() < SEC_GAMEMASTER3)
    {
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if(charname.empty() || !normalizePlayerName (charname))
    {
        SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player *plr = ObjectAccessor::FindConnectedPlayerByName(charname.c_str());

    if(!plr)
    {
        SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, charname.c_str());
        return;
    }

    uint32 accid = plr->GetSession()->GetAccountId();

    QueryResult result = LoginDatabase.PQuery("SELECT username,email,last_ip FROM account WHERE id=%u", accid);
    if(!result)
    {
        SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, charname.c_str());
        return;
    }

    Field *fields = result->Fetch();
    std::string acc = fields[0].GetString();
    if(acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetString();
    if(email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetString();
    if(lastip.empty())
        lastip = "Unknown";

    std::string msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

    WorldPacket data(SMSG_WHOIS, msg.size()+1);
    data << msg;
    _player->SendDirectMessage(&data);

    TC_LOG_DEBUG("network", "Received whois command from player %s for character %s",
        GetPlayer()->GetName().c_str(), charname.c_str());
}

void WorldSession::HandleComplainOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "WORLD: CMSG_COMPLAIN");

    uint8 ComplaintType;                                        // 0 - mail, 1 - chat
    ObjectGuid spammer_guid;
    uint32 unk1 = 0, unk2 = 0, unk3 = 0, unk4 = 0;
    std::string description = "";
    recvData >> ComplaintType;                             // unk 0x01 const, may be spam type (mail/chat)
    recvData >> spammer_guid;                              // player guid
    switch (ComplaintType)
    {
        case 0:

            recvData >> unk1;                              // const 0
            recvData >> unk2;                              // probably mail id
            recvData >> unk3;                              // const 0
            break;
        case 1:

            recvData >> unk1;                              // probably language
            recvData >> unk2;                              // message type?
            recvData >> unk3;                              // probably channel id
            recvData >> unk4;                              // unk random value
            recvData >> description;                       // spam description string (messagetype, channel name, player name, message)
            break;
    }

    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Trigger "Complaint Registred" message at client
    WorldPacket data(SMSG_COMPLAIN_RESULT, 1);
    data << uint8(0);
    SendPacket(&data);

    if (ComplaintType == 1) {
        if (Player* spammer = ObjectAccessor::FindPlayer(spammer_guid))
            spammer->addSpamReport(_player->GetGUID(), description.c_str());
    }

    TC_LOG_DEBUG("network", "REPORT SPAM: type %u, %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s",
        ComplaintType, spammer_guid.GetCounter(), unk1, unk2, unk3, unk4, description.c_str());
}

void WorldSession::HandleRealmSplitOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "CMSG_REALM_SPLIT");

    uint32 Decision;
    std::string split_date = "01/01/01";
    recvData >> Decision;

    WorldPacket data(SMSG_REALM_SPLIT, 4+4+split_date.size()+1);
    data << Decision;
    data << uint32(0x00000000);                             // realm split state
    // split states:
    // 0x0 realm normal
    // 0x1 realm split
    // 0x2 realm split pending
    data << split_date;
    SendPacket(&data);
}

void WorldSession::HandleFarSightOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "WORLD: CMSG_FAR_SIGHT");

    bool apply;
    recvData >> apply;

    if (apply)
    {
        TC_LOG_DEBUG("network", "Added FarSight %s to player %u", ObjectGuid(_player->GetGuidValue(PLAYER_FARSIGHT)).ToString().c_str(), ObjectGuid(_player->GetGUID()).GetCounter());
        if (WorldObject* target = _player->GetViewpoint())
            _player->SetSeer(target);
        else
            TC_LOG_DEBUG("network", "Player %s (%s) requests non-existing seer %s", _player->GetName().c_str(), ObjectGuid(_player->GetGUID()).ToString().c_str(), ObjectGuid(_player->GetGuidValue(PLAYER_FARSIGHT)).ToString().c_str());
    }
    else
    {
        TC_LOG_DEBUG("network", "Player %u set vision to self", ObjectGuid(_player->GetGUID()).GetCounter());
        _player->SetSeer(_player);
    }

    GetPlayer()->UpdateVisibilityForPlayer();
}

void WorldSession::HandleSetTitleOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "CMSG_SET_TITLE");

    int32 title;
    recvData >> title;

    // -1 at none
    if(title > 0 && title < MAX_TITLE_INDEX)
    {
       if(!GetPlayer()->HasTitle(title))
            return;
    }
    else
        title = 0;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
}

// CMSG_TIME_SYNC_RESP
void WorldSession::HandleTimeSyncResp(WorldPacket & recvData)
{
    uint32 counter, clientTimestamp;
    recvData >> counter >> clientTimestamp;

    // time_ seems always more than GetMSTime()
    // uint32 diff = GetMSTimeDiff(GetMSTime(),time_);

    if (counter != m_timeSyncCounter - 1)
    {
        TC_LOG_DEBUG("network", "Wrong time sync counter from player %s (cheater?)", _player->GetName().c_str());
        return;
    }

    //Implement part of http://www.mine-control.com/zack/timesync/timesync.html
    //May be improved by implementing the rest, not so complicated :) We're currently too sensible to TCP retransmissions
    // time it took for the request to travel to the client, for the client to process it and reply and for response to travel back to the server.
    uint32 roundTripDuration = GetMSTimeDiff(m_timeSyncServer, GetMSTime());

    // We want to estimate delay between our request and the client response. The client timestamp is the time he actually received it
    uint32 lagDelay = roundTripDuration / 2; // we assume that the request processing time is 0
    /*
    clockDelta = serverTime - clientTime
    where
    serverTime: time that was displayed on the clock of the SERVER at the moment when the client processed the SMSG_TIME_SYNC_REQUEST packet.
    clientTime: time that was displayed on the clock of the CLIENT at the moment when the client processed the SMSG_TIME_SYNC_REQUEST packet.
    Once clockDelta has been computed, we can compute the time on server clock of an event when we know the time of the event on the client clock,
    using this relation:
    serverTime = clockDelta + clientTime
    Or in english: delta is the time we need to add to client time to get the server time
    */
    m_timeSyncClockDelta = int64(m_timeSyncServer) + lagDelay - int64(clientTimestamp);
}

void WorldSession::HandleResetInstancesOpcode( WorldPacket & /*recvData*/ )
{
    Group *pGroup = _player->GetGroup();
    if(pGroup)
    {
        if(pGroup->IsLeader(_player->GetGUID()))
            pGroup->ResetInstances(INSTANCE_RESET_ALL, false, _player);
    }
    else
        _player->ResetInstances(INSTANCE_RESET_ALL, false);
}

void WorldSession::HandleSetDungeonDifficultyOpcode( WorldPacket & recvData )
{
    uint32 mode;
    recvData >> mode;

    if(mode == _player->GetDifficulty())
        return;

    if(mode >= MAX_DIFFICULTY)
    {
        TC_LOG_ERROR("network","WorldSession::HandleSetDungeonDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUID().GetCounter(), mode);
        return;
    }

    // cannot reset while in an instance
    Map* map = _player->FindMap();
    if(map && map->IsDungeon())
    {
        TC_LOG_ERROR("network","WorldSession::HandleSetDungeonDifficultyOpcode: player %d tried to reset the instance while inside a dungeon!", _player->GetGUID().GetCounter());
        return;
    }

    if(_player->GetLevel() < LEVELREQUIREMENT_HEROIC)
        return;

    Group *group = _player->GetGroup();
    if(group)
    {
        if(group->IsLeader(_player->GetGUID()))
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* groupGuy = itr->GetSource();
                if (!groupGuy)
                    continue;

                //sun: reworked condition
                if (!groupGuy->GetMap()) 
                    return;

                if (groupGuy->GetMap()->IsNonRaidDungeon())
                {
                    TC_LOG_DEBUG("network", "WorldSession::HandleSetDungeonDifficultyOpcode: player %d tried to reset the instance while group member (Name: %s, GUID: %u) is inside!",
                        _player->GetGUID().GetCounter(), groupGuy->GetName().c_str(), groupGuy->GetGUID().GetCounter());
                    return;
                }
            }

            // the difficulty is set even if the instances can't be reset
            group->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false, _player);
            group->SetDifficulty(Difficulty(mode));
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false);
        _player->SetDifficulty(Difficulty(mode), false, false); //client changes it without needing to be told
    }
}

void WorldSession::HandleCancelMountAuraOpcode( WorldPacket & /*recvData*/ )
{
    //If player is not mounted, so go out :)
    if (!_player->IsMounted())                              // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_CHAR_NON_MOUNTED);
        return;
    }

    if(_player->IsInFlight())                               // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_YOU_IN_FLIGHT);
        return;
    }

    _player->Dismount();
    _player->RemoveAurasByType(SPELL_AURA_MOUNTED);
}

void WorldSession::HandleRequestPetInfoOpcode( WorldPacket & /*recvData */)
{
    if (_player->GetPet())
        _player->PetSpellInitialize();
    else if (_player->GetCharmed())
        _player->CharmSpellInitialize();
}

void WorldSession::HandleSetTaxiBenchmarkOpcode( WorldPacket & recvData )
{
    //TC_LOG_DEBUG("network", "WORLD: CMSG_SET_TAXI_BENCHMARK_MODE");

    uint8 mode;
    recvData >> mode;

    mode ? _player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK) : _player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK);

    //TC_LOG_DEBUG("network", "Client used \"/timetest %d\" command", mode);
}
