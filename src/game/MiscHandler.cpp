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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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
#include "ScriptCalls.h"
#include <zlib/zlib.h>
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "Battleground.h"
#include "OutdoorPvP.h"
#include "SpellAuras.h"
#include "Pet.h"
#include "SocialMgr.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"
#include "IRCMgr.h"

void WorldSession::HandleRepopRequestOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
    if(GetPlayer()->IsAlive()||GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if(GetPlayer()->GetDeathState() == JUST_DIED)
    {
        TC_LOG_ERROR("FIXME","HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated", GetPlayer()->GetName().c_str(), GetPlayer()->GetGUIDLow());
        GetPlayer()->KillPlayer();
    }

    //this is spirit release confirm?
    GetPlayer()->RemovePet(NULL,PET_SAVE_NOT_IN_SLOT, true);
    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleGossipSelectOptionOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,8+4+4);

    uint32 option;
    uint32 unk;
    uint64 guid;
    std::string code = "";

    recvData >> guid >> unk >> option;

    if(_player->PlayerTalkClass->GossipOptionCoded( option ))
    {
        // recheck
        CHECK_PACKET_SIZE(recvData,8+4+1);
        TC_LOG_DEBUG("FIXME","reading string");
        recvData >> code;
        TC_LOG_DEBUG("FIXME","string read: %s", code.c_str());
    }

    Creature *unit = NULL;
    GameObject *go = NULL;
    if(IS_CREATURE_GUID(guid))
    {
        unit = GetPlayer()->GetNPCIfCanInteractWith(guid);
        if (!unit)
        {
            TC_LOG_ERROR( "FIXME","WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
            return;
        }
    }
    else if(IS_GAMEOBJECT_GUID(guid))
    {
        go = ObjectAccessor::GetGameObject(*_player, guid);
        if (!go)
        {
            TC_LOG_ERROR( "FIXME","WORLD: HandleGossipSelectOptionOpcode - GameObject (GUID: %u) not found.", uint32(GUID_LOPART(guid)) );
            return;
        }
    }
    else
    {
        TC_LOG_ERROR( "FIXME","WORLD: HandleGossipSelectOptionOpcode - unsupported GUID type for highguid %u. lowpart %u.", uint32(GUID_HIPART(guid)), uint32(GUID_LOPART(guid)) );
        return;
    }

    // remove fake death
    if(GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if(!code.empty())
    {
        if(unit)
        {
            if(!sScriptMgr->GossipSelectWithCode( _player, unit, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option ), code.c_str()) )
                unit->OnGossipSelect( _player, option );
            
            unit->AI()->sGossipSelectCode(_player, _player->PlayerTalkClass->GossipOptionSender(option), _player->PlayerTalkClass->GossipOptionAction(option), code.c_str());
        }
        else {
            sScriptMgr->GOSelectWithCode( _player, go, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option ), code.c_str());
            go->AI()->GossipSelectCode(_player, _player->PlayerTalkClass->GossipOptionSender(option), _player->PlayerTalkClass->GossipOptionAction(option), code.c_str());
        }
    }
    else
    {
        if(unit)
        {
            if(!sScriptMgr->GossipSelect( _player, unit, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option )) )
                unit->OnGossipSelect( _player, option );
                
            unit->AI()->sGossipSelect(_player, _player->PlayerTalkClass->GossipOptionSender(option), _player->PlayerTalkClass->GossipOptionAction(option));
        }
        else {
            sScriptMgr->GOSelect( _player, go, _player->PlayerTalkClass->GossipOptionSender( option ), _player->PlayerTalkClass->GossipOptionAction( option ));
            go->AI()->GossipSelect(_player, _player->PlayerTalkClass->GossipOptionSender(option), _player->PlayerTalkClass->GossipOptionAction(option));
        }
    }
}

void WorldSession::HandleWhoOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,4+4+1+1+4+4+4+4);

    uint32 clientcount = 0;

    uint32 level_min, level_max, racemask, classmask, zones_count, str_count;
    uint32 zoneids[10];                                     // 10 is client limit
    std::string player_name, guild_name;

    recvData >> level_min;                                 // maximal player level, default 0
    recvData >> level_max;                                 // minimal player level, default 100 (MAX_LEVEL)
    recvData >> player_name;                               // player name, case sensitive...

    // recheck
    CHECK_PACKET_SIZE(recvData,4+4+(player_name.size()+1)+1+4+4+4+4);

    recvData >> guild_name;                                // guild name, case sensitive...

    // recheck
    CHECK_PACKET_SIZE(recvData,4+4+(player_name.size()+1)+(guild_name.size()+1)+4+4+4+4);

    recvData >> racemask;                                  // race mask
    recvData >> classmask;                                 // class mask
    recvData >> zones_count;                               // zones count, client limit=10 (2.0.10)

    if(zones_count > 10)
        return;                                             // can't be received from real client or broken packet

    // recheck
    CHECK_PACKET_SIZE(recvData,4+4+(player_name.size()+1)+(guild_name.size()+1)+4+4+4+(4*zones_count)+4);

    for(uint32 i = 0; i < zones_count; i++)
    {
        uint32 temp;
        recvData >> temp;                                  // zone id, 0 if zone is unknown...
        zoneids[i] = temp;
    }

    recvData >> str_count;                                 // user entered strings count, client limit=4 (checked on 2.0.10)

    if(str_count > 4)
        return;                                             // can't be received from real client or broken packet

    // recheck
    CHECK_PACKET_SIZE(recvData,4+4+(player_name.size()+1)+(guild_name.size()+1)+4+4+4+(4*zones_count)+4+(1*str_count));

    std::wstring str[4];                                    // 4 is client limit
    for(uint32 i = 0; i < str_count; i++)
    {
        // recheck (have one more byte)
        CHECK_PACKET_SIZE(recvData,recvData.rpos());

        std::string temp;
        recvData >> temp;                                  // user entered string, it used as universal search pattern(guild+player name)?

        if(!Utf8toWStr(temp,str[i]))
            continue;

        wstrToLower(str[i]);
    }

    std::wstring wplayer_name;
    std::wstring wguild_name;
    if(!(Utf8toWStr(player_name, wplayer_name) && Utf8toWStr(guild_name, wguild_name)))
        return;
    wstrToLower(wplayer_name);
    wstrToLower(wguild_name);

    // client send in case not set max level value 100 but mangos support 255 max level,
    // update it to show GMs with characters after 100 level
    if(level_max >= MAX_LEVEL)
        level_max = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    uint32 security = GetSecurity();
    bool allowTwoSideWhoList = sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    uint32 gmLevelInWhoList  = sWorld->getConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);

    WorldPacket data( SMSG_WHO, 50 );                       // guess size
    data << clientcount;                                    // clientcount place holder
    data << clientcount;                                    // clientcount place holder

    //TODO: Guard Player map
    HashMapHolder<Player>::MapType& m = sObjectAccessor->GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (security == SEC_PLAYER)
        {
            // player can see member of other team only if CONFIG_ALLOW_TWO_SIDE_WHO_LIST
            if (itr->second->GetTeam() != team && !allowTwoSideWhoList )
                continue;

            // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
            if ((itr->second->GetSession()->GetSecurity() > gmLevelInWhoList))
                continue;
        }

        // check if target is globally visible for player
        if (!(itr->second->IsVisibleGloballyFor(_player)))
            continue;

        // check if target's level is in level range
        uint32 lvl = itr->second->GetLevel();
        if (lvl < level_min || lvl > level_max)
            continue;

        // check if class matches classmask
        uint32 class_ = itr->second->GetClass();
        if (!(classmask & (1 << class_)))
            continue;

        // check if race matches racemask
        uint32 race = itr->second->GetRace();
        if (!(racemask & (1 << race)))
            continue;

        uint32 pzoneid = itr->second->GetZoneId();
        //do not show players in arenas
        if(pzoneid == 3698 || pzoneid == 3968 || pzoneid == 3702)
        {
            uint32 mapId = itr->second->GetBattlegroundEntryPointMap();
            Map * map = sMapMgr->FindMap(mapId);
            if(map) 
            {
                float x = itr->second->GetBattlegroundEntryPointX();
                float y = itr->second->GetBattlegroundEntryPointY();
                float z = itr->second->GetBattlegroundEntryPointZ();
                pzoneid = map->GetZoneId(x,y,z);
            }
        }
        
        uint8 gender = itr->second->GetGender();

        bool z_show = true;
        for(uint32 i = 0; i < zones_count; i++)
        {
            if(zoneids[i] == pzoneid)
            {
                z_show = true;
                break;
            }

            z_show = false;
        }
        if (!z_show)
            continue;

        std::string pname = itr->second->GetName();
        std::wstring wpname;
        if(!Utf8toWStr(pname,wpname))
            continue;
        wstrToLower(wpname);

        if (!(wplayer_name.empty() || wpname.find(wplayer_name) != std::wstring::npos))
            continue;

        std::string gname = sObjectMgr->GetGuildNameById(itr->second->GetGuildId());
        std::wstring wgname;
        if(!Utf8toWStr(gname,wgname))
            continue;
        wstrToLower(wgname);

        if (!(wguild_name.empty() || wgname.find(wguild_name) != std::wstring::npos))
            continue;

        std::string aname;
        if(AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(itr->second->GetZoneId()))
            aname = areaEntry->area_name[GetSessionDbcLocale()];

        bool s_show = true;
        for(uint32 i = 0; i < str_count; i++)
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

        data << pname;                                      // player name
        data << gname;                                      // guild name
        data << uint32(lvl);                              // player level
        data << uint32(class_);                           // player class
        data << uint32(race);                             // player race
        data << uint8(gender);                                   // new 2.4.0
        data << uint32(pzoneid);                          // player zone id

        // 49 is maximum player count sent to client - can be overridden
        // through config, but is unstable
        if ((++clientcount) == sWorld->getConfig(CONFIG_MAX_WHO))
            break;
    }

    // TODO: both clientcount shouldn't be the same, check trinity implementation (one is total results, the other is displayed result)
    data.put( 0,              clientcount );                //insert right count
    data.put( sizeof(uint32), clientcount );                //insert right count

    SendPacket(&data);
}

void WorldSession::HandleLogoutRequestOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
    if (uint64 lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);

    //Can not logout if...
    if( GetPlayer()->IsInCombat() ||                        //...is in combat
        GetPlayer()->duel         ||                        //...is in Duel
        GetPlayer()->HasAura(9454,0)         ||             //...is frozen by GM via freeze command
        GetPlayer()->HasUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING))  //...is jumping ...is falling
    {
        WorldPacket data( SMSG_LOGOUT_RESPONSE, (2+4) ) ;
        data << (uint8)0xC;
        data << uint32(0);
        data << uint8(0);
        SendPacket( &data );
        LogoutRequest(0);
        return;
    }

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in mangosd.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || GetPlayer()->IsInFlight() ||
        GetSecurity() >= sWorld->getConfig(CONFIG_INSTANT_LOGOUT))
    {
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if(GetPlayer()->CanFreeMove())
    {
        GetPlayer()->SetStandState(PLAYER_STATE_SIT);

        WorldPacket data( SMSG_FORCE_MOVE_ROOT, (8+4) );    // guess size
        data.append(GetPlayer()->GetPackGUID());
        data << (uint32)2;
        SendPacket( &data );
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    WorldPacket data( SMSG_LOGOUT_RESPONSE, 5 );
    data << uint32(0);
    data << uint8(0);
    SendPacket( &data );
    LogoutRequest(time(NULL));
}

void WorldSession::HandlePlayerLogoutOpcode( WorldPacket & /*recvData*/ )
{
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_PLAYER_LOGOUT Message");
}

void WorldSession::HandleLogoutCancelOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
    TC_LOG_DEBUG("network", "WORLD: Recvd CMSG_LOGOUT_CANCEL Message");

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
        data.Initialize( SMSG_FORCE_MOVE_UNROOT, 8 );       // guess size
        data.append(GetPlayer()->GetPackGUID());
        data << uint32(0);
        SendPacket( &data );

        //! Stand Up
        GetPlayer()->SetStandState(PLAYER_STATE_NONE);

        //! DISABLE_ROTATE
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    TC_LOG_DEBUG("network", "WORLD: Sent SMSG_LOGOUT_CANCEL_ACK Message");
}

void WorldSession::HandleTogglePvP( WorldPacket & recvData )
{
    PROFILE;
    
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
        if(!GetPlayer()->pvpInfo.inHostileArea && GetPlayer()->IsPvP())
            GetPlayer()->pvpInfo.endTimer = time(NULL);     // start toggle-off
    }

    if(OutdoorPvP * pvp = _player->GetOutdoorPvP())
    {
        pvp->HandlePlayerActivityChanged(_player);
    }
}

void WorldSession::HandleZoneUpdateOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,4);

    uint32 newZone;
    recvData >> newZone;

    TC_LOG_DEBUG("FIXME","WORLD: Recvd ZONE_UPDATE: %u", newZone);

    GetPlayer()->UpdateZone(newZone);

    GetPlayer()->SendInitWorldStates(true,newZone);
}

void WorldSession::HandleSetTargetOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    // When this packet send?
    CHECK_PACKET_SIZE(recvData,8);

    uint64 guid ;
    recvData >> guid;

    _player->SetUInt32Value(UNIT_FIELD_TARGET,guid);

    // update reputation list if need
    Unit* unit = ObjectAccessor::GetUnit(*_player, guid );
    if(!unit)
        return;

    _player->SetFactionVisibleForFactionTemplateId(unit->GetFaction());
}

void WorldSession::HandleSetSelectionOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,8);

    uint64 guid;
    recvData >> guid;

    _player->SetSelection(guid);

    // update reputation list if need
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

    if (unit)
        _player->SetFactionVisibleForFactionTemplateId(unit->GetFaction());
}

void WorldSession::HandleStandStateChangeOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    if(!_player->m_mover->IsAlive())
        return;

    CHECK_PACKET_SIZE(recvData,1);

    uint8 animstate;
    recvData >> animstate;

    _player->m_mover->SetStandState(animstate);
}

void WorldSession::HandleContactListOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 4);
    
    uint32 unk;
    recvData >> unk; // TODO
    _player->GetSocial()->SendSocialList();
}

void WorldSession::HandleAddFriendOpcode(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 1+1);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ADD_FRIEND");

    std::string friendName = GetTrinityString(LANG_FRIEND_IGNORE_UNKNOWN);
    std::string friendNote;

    recvData >> friendName;

    // recheck
    CHECK_PACKET_SIZE(recvData, (friendName.size()+1)+1);

    recvData >> friendNote;

    if (!normalizePlayerName(friendName))
        return;

    TC_LOG_DEBUG("network", "WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName().c_str(), friendName.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_RACE_ACC_BY_NAME);

    stmt->setString(0, friendName);

    _addFriendCallback.SetParam(friendNote);
    _addFriendCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleAddFriendOpcodeCallBack(PreparedQueryResult result, std::string const& friendNote)
{
    if (!GetPlayer())
        return;

    uint64 friendGuid;
    uint64 friendAcctid;
    uint32 team;
    FriendsResult friendResult;
 
    friendResult = FRIEND_NOT_FOUND;
    friendGuid = 0;

    if(result)
    {
        friendGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        team = Player::TeamForRace((*result)[1].GetUInt8());
        friendAcctid = (*result)[2].GetUInt32();

        if ( GetSecurity() >= SEC_GAMEMASTER1 || sWorld->getConfig(CONFIG_ALLOW_GM_FRIEND) || sAccountMgr->GetSecurity(friendAcctid) < SEC_GAMEMASTER1)
            if(friendGuid)
            {
                if(friendGuid==GetPlayer()->GetGUID())
                    friendResult = FRIEND_SELF;
                else if(GetPlayer()->GetTeam() != team && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && GetSecurity() < SEC_GAMEMASTER1)
                    friendResult = FRIEND_ENEMY;
                else if(GetPlayer()->GetSocial()->HasFriend(GUID_LOPART(friendGuid)))
                    friendResult = FRIEND_ALREADY;
                else
                {
                    Player* pFriend = ObjectAccessor::FindPlayer(friendGuid);
                    if( pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyFor(GetPlayer()))
                    friendResult = FRIEND_ADDED_ONLINE;
                    else
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if(!GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(friendGuid), false))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        TC_LOG_DEBUG("network", "WORLD: %s's friend list is full.", GetPlayer()->GetName().c_str());
                    }
                }
                GetPlayer()->GetSocial()->SetFriendNote(GUID_LOPART(friendGuid), friendNote);
            }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), friendResult, GUID_LOPART(friendGuid), false);

    TC_LOG_DEBUG("network", "WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelFriendOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 friendGUID;
    recvData >> friendGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(friendGUID), false);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_REMOVED, GUID_LOPART(friendGUID), false);
}

void WorldSession::HandleAddIgnoreOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,1);

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_ADD_IGNORE");

    std::string ignoreName = GetTrinityString(LANG_FRIEND_IGNORE_UNKNOWN);

    recvData >> ignoreName;

    if (!normalizePlayerName(ignoreName))
        return;

    TC_LOG_DEBUG("network", "WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName().c_str(), ignoreName.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_BY_NAME);

    stmt->setString(0, ignoreName);

    _addIgnoreCallback = CharacterDatabase.AsyncQuery(stmt);
}

void WorldSession::HandleAddIgnoreOpcodeCallBack(PreparedQueryResult result)
{
    if (!GetPlayer())
        return;

    uint64 IgnoreGuid;
    FriendsResult ignoreResult;

    ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    IgnoreGuid = 0;

    if (result)
    {
        IgnoreGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);

        if (IgnoreGuid)
        {
            if (IgnoreGuid == GetPlayer()->GetGUID())              //not add yourself
                ignoreResult = FRIEND_IGNORE_SELF;
            else if (GetPlayer()->GetSocial()->HasIgnore(GUID_LOPART(IgnoreGuid)))
                ignoreResult = FRIEND_IGNORE_ALREADY;
            else
            {
                ignoreResult = FRIEND_IGNORE_ADDED;

                // ignore list full
                if (!GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(IgnoreGuid), true))
                    ignoreResult = FRIEND_IGNORE_FULL;
            }
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), ignoreResult, GUID_LOPART(IgnoreGuid), false);

    TC_LOG_DEBUG("network", "WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelIgnoreOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 ignoreGUID;

    recvData >> ignoreGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(ignoreGUID), true);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_IGNORE_REMOVED, GUID_LOPART(ignoreGUID), false);
}

void WorldSession::HandleSetContactNotesOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8+1);
    uint64 guid;
    std::string note;
    recvData >> guid >> note;
    _player->GetSocial()->SetFriendNote(guid, note);
}

void WorldSession::HandleBugOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,4+4+1+4+1);

    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recvData >> suggestion >> contentlen >> content;

    //recheck
    CHECK_PACKET_SIZE(recvData,4+4+(content.size()+1)+4+1);

    recvData >> typelen >> type;

    CharacterDatabase.EscapeString(type);
    CharacterDatabase.EscapeString(content);
    CharacterDatabase.PExecute ("INSERT INTO bugreport (type,content) VALUES('%s', '%s')", type.c_str( ), content.c_str( ));
}

void WorldSession::HandleReclaimCorpseOpcode(WorldPacket &recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,8);

    TC_LOG_DEBUG("FIXME","WORLD: Received CMSG_RECLAIM_CORPSE");
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
    if(GetPlayer()->GetDeathTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType()==CORPSE_RESURRECTABLE_PVP) > time(NULL))
        return;

    float dist = corpse->GetDistance2d(GetPlayer());
    if (dist > CORPSE_RECLAIM_RADIUS)
        return;

    uint64 guid;
    recvData >> guid;

    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleground() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();

    GetPlayer()->SaveToDB();
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,8+1);

    TC_LOG_DEBUG("FIXME","WORLD: Received CMSG_RESURRECT_RESPONSE");

    if(GetPlayer()->IsAlive())
        return;

    uint64 guid;
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

    GetPlayer()->ResurectUsingRequestData();
    GetPlayer()->SaveToDB();
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,4);

    uint32 triggerId;

    recvData >> triggerId;
    
    if(GetPlayer()->IsGameMaster())
        SendAreaTriggerMessage("Entered areatrigger %u.", triggerId);

    if(GetPlayer()->IsInFlight())
        return;

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(triggerId);
    if(!atEntry)
    {
        TC_LOG_ERROR("FIXME","Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID:%u",GetPlayer()->GetName().c_str(),GetPlayer()->GetGUIDLow(), triggerId);
        return;
    }

    if (GetPlayer()->GetMapId()!=atEntry->mapid)
    {
        TC_LOG_ERROR("FIXME","Player '%s' (GUID: %u) too far (trigger map: %u player map: %u), ignore Area Trigger ID: %u", GetPlayer()->GetName().c_str(), atEntry->mapid, GetPlayer()->GetMapId(), GetPlayer()->GetGUIDLow(), triggerId);
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
            TC_LOG_ERROR("FIXME","Player '%s' (GUID: %u) too far (radius: %f distance: %f), ignore Area Trigger ID: %u",
                pl->GetName().c_str(), pl->GetGUIDLow(), atEntry->radius, dist, triggerId);
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

    if(sScriptMgr->AreaTrigger(GetPlayer(), atEntry))
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
        GetPlayer()->InnEnter(time(NULL), atEntry->mapid, atEntry->x, atEntry->y, atEntry->z);
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

    if(!GetPlayer()->Satisfy(sObjectMgr->GetAccessRequirement(at->access_id), at->target_mapId, true))
        return;

    GetPlayer()->TeleportTo(at->target_mapId,at->target_X,at->target_Y,at->target_Z,at->target_Orientation,TELE_TO_NOT_LEAVE_TRANSPORT);
}

void WorldSession::HandleUpdateAccountData(WorldPacket &/*recvData*/)
{
    // TODO
}

void WorldSession::HandleRequestAccountData(WorldPacket& /*recvData*/)
{
    // TODO
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,1+2+1+1);

    uint8 button, misc, type;
    uint16 action;
    recvData >> button >> action >> misc >> type;

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
            TC_LOG_ERROR( "FIXME", "MISC: Unknown action button type %u for action %u into button %u", type, action, button );
    }
}

void WorldSession::HandleCompleteCinematic( WorldPacket & /*recvData*/ )
{
    // TODO
}

void WorldSession::HandleNextCinematicCamera( WorldPacket & /*recvData*/ )
{
    // TODO
}

void WorldSession::HandleMoveTimeSkippedOpcode( WorldPacket & /*recvData*/ )
{
    // TODO

    /*
        CHECK_PACKET_SIZE(recvData,8+4);
        uint64 guid;
        uint32 time_skipped;
        recvData >> guid;
        recvData >> time_skipped;
        TC_LOG_DEBUG("FIXME", "WORLD: CMSG_MOVE_TIME_SKIPPED" );

        /// TODO
        must be need use in Trinity
        We substract server Lags to move time ( AntiLags )
        for exmaple
        GetPlayer()->ModifyLastMoveTime( -int32(time_skipped) );
    */
}

void WorldSession::HandleFeatherFallAck(WorldPacket & recvData)
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_MOVE_FEATHER_FALL_ACK");

    // not used
    recvData.rfinish();
}

void WorldSession::HandleMoveUnRootAck(WorldPacket&/* recvData*/)
{
    PROFILE;
    
    /*
        CHECK_PACKET_SIZE(recvData,8+8+4+4+4+4+4);

        TC_LOG_DEBUG("FIXME", "WORLD: CMSG_FORCE_MOVE_UNROOT_ACK" );
        recvData.hexlike();
        uint64 guid;
        uint64 unknown1;
        uint32 unknown2;
        float PositionX;
        float PositionY;
        float PositionZ;
        float Orientation;

        recvData >> guid;
        recvData >> unknown1;
        recvData >> unknown2;
        recvData >> PositionX;
        recvData >> PositionY;
        recvData >> PositionZ;
        recvData >> Orientation;

        // TODO for later may be we can use for anticheat
        TC_LOG_DEBUG("FIXME","Guid " UI64FMTD,guid);
        TC_LOG_DEBUG("FIXME","unknown1 " UI64FMTD,unknown1);
        TC_LOG_DEBUG("FIXME","unknown2 %u",unknown2);
        TC_LOG_DEBUG("FIXME","X %f",PositionX);
        TC_LOG_DEBUG("FIXME","Y %f",PositionY);
        TC_LOG_DEBUG("FIXME","Z %f",PositionZ);
        TC_LOG_DEBUG("FIXME","O %f",Orientation);
    */
}

void WorldSession::HandleMoveRootAck(WorldPacket&/* recvData*/)
{
    PROFILE;
    
    /*
        CHECK_PACKET_SIZE(recvData,8+8+4+4+4+4+4);

        TC_LOG_DEBUG("FIXME", "WORLD: CMSG_FORCE_MOVE_ROOT_ACK" );
        recvData.hexlike();
        uint64 guid;
        uint64 unknown1;
        uint32 unknown2;
        float PositionX;
        float PositionY;
        float PositionZ;
        float Orientation;

        recvData >> guid;
        recvData >> unknown1;
        recvData >> unknown2;
        recvData >> PositionX;
        recvData >> PositionY;
        recvData >> PositionZ;
        recvData >> Orientation;

        // for later may be we can use for anticheat
        TC_LOG_DEBUG("FIXME","Guid " UI64FMTD,guid);
        TC_LOG_DEBUG("FIXME","unknown1 " UI64FMTD,unknown1);
        TC_LOG_DEBUG("FIXME","unknown1 %u",unknown2);
        TC_LOG_DEBUG("FIXME","X %f",PositionX);
        TC_LOG_DEBUG("FIXME","Y %f",PositionY);
        TC_LOG_DEBUG("FIXME","Z %f",PositionZ);
        TC_LOG_DEBUG("FIXME","O %f",Orientation);
    */
}

void WorldSession::HandleSetActionBarToggles(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,1);

    uint8 ActionBar;

    recvData >> ActionBar;

    if(!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if(ActionBar!=0)
            TC_LOG_ERROR("FIXME","WorldSession::HandleSetActionBar in not logged state with value: %u, ignored",uint32(ActionBar));
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, 2, ActionBar);
}

void WorldSession::HandlePlayedTime(WorldPacket& /*recvData*/)
{
    PROFILE;
    
    uint32 TotalTimePlayed = GetPlayer()->GetTotalPlayedTime();
    uint32 LevelPlayedTime = GetPlayer()->GetLevelPlayedTime();

    WorldPacket data(SMSG_PLAYED_TIME, 8);
    data << TotalTimePlayed;
    data << LevelPlayedTime;
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;
    recvData >> guid;

    _player->SetSelection(guid);

    Player *plr = sObjectMgr->GetPlayer(guid);
    if(!plr)                                                // wrong player
        return;

    uint32 talent_points = 0x3D;
    uint32 guid_size = plr->GetPackGUID().wpos();
    WorldPacket data(SMSG_INSPECT_TALENT, guid_size+4+talent_points);
    data.append(plr->GetPackGUID());
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
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;
    recvData >> guid;

    Player *player = sObjectMgr->GetPlayer(guid);

    if(!player)
    {
        TC_LOG_ERROR("FIXME","InspectHonorStats: WTF, player not found...");
        return;
    }

    WorldPacket data(MSG_INSPECT_HONOR_STATS, 8+1+4*4);
    data << uint64(player->GetGUID());
    data << uint8(player->GetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_KILLS));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
    SendPacket(&data);
}

void WorldSession::HandleWorldTeleportOpcode(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,4+4+4+4+4+4);

    // write in client console: worldport 469 452 6454 2536 180 or /console worldport 469 452 6454 2536 180
    // Received opcode CMSG_WORLD_TELEPORT
    // Time is ***, map=469, x=452.000000, y=6454.000000, z=2536.000000, orient=3.141593

    //TC_LOG_DEBUG("FIXME","Received opcode CMSG_WORLD_TELEPORT");

    if(GetPlayer()->IsInFlight())
        return;

    uint32 time;
    uint32 mapid;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recvData >> time;                                      // time in m.sec.
    recvData >> mapid;
    recvData >> PositionX;
    recvData >> PositionY;
    recvData >> PositionZ;
    recvData >> Orientation;                               // o (3.141593 = 180 degrees)

    if (GetSecurity() >= SEC_GAMEMASTER3)
        GetPlayer()->TeleportTo(mapid,PositionX,PositionY,PositionZ,Orientation);
    else
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 1);

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

    Player *plr = sObjectMgr->GetPlayer(charname.c_str());

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
    _player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleComplainOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 1+8);

    uint8 spam_type;                                        // 0 - mail, 1 - chat
    uint64 spammer_guid;
    uint32 unk1, unk2, unk3, unk4 = 0;
    std::string description = "";
    recvData >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recvData >> spammer_guid;                              // player guid
    switch(spam_type)
    {
        case 0:
            CHECK_PACKET_SIZE(recvData, recvData.rpos()+4+4+4);
            recvData >> unk1;                              // const 0
            recvData >> unk2;                              // probably mail id
            recvData >> unk3;                              // const 0
            break;
        case 1:
            CHECK_PACKET_SIZE(recvData, recvData.rpos()+4+4+4+4+1);
            recvData >> unk1;                              // probably language
            recvData >> unk2;                              // message type?
            recvData >> unk3;                              // probably channel id
            recvData >> unk4;                              // unk random value
            recvData >> description;                       // spam description string (messagetype, channel name, player name, message)
            break;
    }

    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_COMPLAIN_RESULT, 1);
    data << uint8(0);
    SendPacket(&data);
    
    if (spam_type == 1) {
        if (Player* spammer = sObjectMgr->GetPlayer(spammer_guid))
            spammer->addSpamReport(_player->GetGUID(), description.c_str());
    }
}

void WorldSession::HandleRealmSplitOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 4);

    uint32 unk;
    std::string split_date = "01/01/01";
    recvData >> unk;

    WorldPacket data(SMSG_REALM_SPLIT, 4+4+split_date.size()+1);
    data << unk;
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
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 1);

    uint8 apply;
    recvData >> apply;

    CellPair pair;

    switch(apply)
    {
        case 0:
            _player->SetFarsightVision(false);
            break;
        case 1:
            _player->SetFarsightVision(true);
            //set the target to notify, so it updates visibility for shared visions (see PlayerRelocationNotifier and CreatureRelocationNotifier)
            if(WorldObject* farSightTarget = _player->GetFarsightTarget())
                if(farSightTarget->isType(TYPEMASK_UNIT))
                    farSightTarget->ToUnit()->SetToNotify(); 
            
            break;
        default:
            TC_LOG_ERROR("FIXME","Unhandled mode in CMSG_FAR_SIGHT: %u", apply);
            return;
    }
    GetPlayer()->SetToNotify();
}

void WorldSession::HandleSetTitleOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 4);

    int32 title;
    recvData >> title;

    // -1 at none
    if(title > 0 && title < 128)
    {
       if(!GetPlayer()->HasTitle(title))
            return;
    }
    else
        title = 0;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
}

void WorldSession::HandleAllowMoveAckOpcod( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 4+4);

    uint32 counter, time_;
    recvData >> counter >> time_;

    // time_ seems always more than getMSTime()
    uint32 diff = GetMSTimeDiff(getMSTime(),time_);
}

void WorldSession::HandleResetInstancesOpcode( WorldPacket & /*recvData*/ )
{
    Group *pGroup = _player->GetGroup();
    if(pGroup)
    {
        if(pGroup->IsLeader(_player->GetGUID()))
            pGroup->ResetInstances(INSTANCE_RESET_ALL, _player);
    }
    else
        _player->ResetInstances(INSTANCE_RESET_ALL);
}

void WorldSession::HandleSetDungeonDifficultyOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 4);

    uint32 mode;
    recvData >> mode;

    if(mode == _player->GetDifficulty())
        return;

    if(mode > DIFFICULTY_HEROIC)
    {
        TC_LOG_ERROR("FIXME","WorldSession::HandleSetDungeonDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    // cannot reset while in an instance
    Map *map = _player->GetMap();
    if(map && map->IsDungeon())
    {
        TC_LOG_ERROR("FIXME","WorldSession::HandleSetDungeonDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
        return;
    }

    if(_player->GetLevel() < LEVELREQUIREMENT_HEROIC)
        return;
    Group *pGroup = _player->GetGroup();
    if(pGroup)
    {
        if(pGroup->IsLeader(_player->GetGUID()))
        {
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            pGroup->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, _player);
            pGroup->SetDifficulty(mode);
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY);
        _player->SetDifficulty(mode);
    }
}

void WorldSession::HandleCancelMountAuraOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
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

void WorldSession::HandleMoveSetCanFlyAckOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8+4+4);

    uint64 guid = 0;                                            // guid - unused
    recvData >> guid;

    recvData.read_skip<uint32>();                          // unk

    uint32 flags;
    recvData >> flags;

    _player->m_mover->m_movementInfo.flags = flags;
}

void WorldSession::HandleRequestPetInfoOpcode( WorldPacket & /*recvData */)
{
    /*
        TC_LOG_DEBUG("FIXME","WORLD: CMSG_REQUEST_PET_INFO");
        recvData.hexlike();
    */
}

void WorldSession::HandleSetTaxiBenchmarkOpcode( WorldPacket & recvData )
{
    TC_LOG_DEBUG("network", "WORLD: CMSG_SET_TAXI_BENCHMARK_MODE");

    uint8 mode;
    recvData >> mode;

    mode ? _player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK) : _player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK);

    TC_LOG_DEBUG("network", "Client used \"/timetest %d\" command", mode);
}

