#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"

bool ChatHandler::HandleAddTeleCommand(const char * args)
{
    ARGS_CHECK

    Player *player=m_session->GetPlayer();
    if (!player)
        return false;

    std::string name = args;

    if(sObjectMgr->GetGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TP_ALREADYEXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameTele tele;
    tele.position_x  = player->GetPositionX();
    tele.position_y  = player->GetPositionY();
    tele.position_z  = player->GetPositionZ();
    tele.orientation = player->GetOrientation();
    tele.mapId       = player->GetMapId();
    tele.name        = name;

    if(sObjectMgr->AddGameTele(tele))
    {
        SendSysMessage(LANG_COMMAND_TP_ADDED);
    }
    else
    {
        SendSysMessage(LANG_COMMAND_TP_ADDEDERR);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleDelTeleCommand(const char * args)
{
    ARGS_CHECK

    std::string name = args;

    if(!sObjectMgr->DeleteGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    SendSysMessage(LANG_COMMAND_TP_DELETED);
    return true;
}

// teleport player to given game_tele.entry
bool ChatHandler::HandleNameTeleCommand(const char * args)
{
    ARGS_CHECK

    char* pName = strtok((char*)args, " ");

    if(!pName)
        return false;

    std::string name = pName;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    char* tail = strtok(nullptr, "");
    if(!tail)
        return false;

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = extractGameTeleFromLink(tail);
    if(!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
    if(!me || me->IsBattlegroundOrArena())
    {
        SendSysMessage(LANG_CANNOT_TELE_TO_BG);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
    if (chr)
    {

        if(chr->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName().c_str());
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, chr->GetName().c_str(),"", tele->name.c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_TELEPORTED_TO_BY, GetName().c_str());

        // stop flight if need
        if(chr->IsInFlight())
        {
            chr->GetMotionMaster()->MovementExpired();
            chr->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            chr->SaveRecallPosition();

        chr->TeleportTo(tele->mapId,tele->position_x,tele->position_y,tele->position_z,tele->orientation);
    }
    else if (uint64 guid = sCharacterCache->GetCharacterGuidByName(name.c_str()))
    {
        PSendSysMessage(LANG_TELEPORTING_TO, name.c_str(), GetTrinityString(LANG_OFFLINE), tele->name.c_str());
        Player::SavePositionInDB(tele->mapId,tele->position_x,tele->position_y,tele->position_z,tele->orientation,sMapMgr->GetZoneId(tele->mapId,tele->position_x,tele->position_y,tele->position_z),guid);
    }
    else
        PSendSysMessage(LANG_NO_PLAYER, name.c_str());

    return true;
}

//Teleport group to given game_tele.entry
bool ChatHandler::HandleGroupTeleCommand(const char * args)
{
    ARGS_CHECK

    Player *player = GetSelectedPlayerOrSelf();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = extractGameTeleFromLink((char*)args);
    if(!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
    if(!me || me->IsBattlegroundOrArena())
    {
        SendSysMessage(LANG_CANNOT_TELE_TO_BG);
        SetSentErrorMessage(true);
        return false;
    }
    Group *grp = player->GetGroup();
    if(!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP,player->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    for(GroupReference *itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player *pl = itr->GetSource();

        if(!pl || !pl->GetSession() )
            continue;

        if(pl->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName().c_str());
            continue;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, pl->GetName().c_str(),"", tele->name.c_str());
        if (needReportToTarget(pl))
            ChatHandler(pl).PSendSysMessage(LANG_TELEPORTED_TO_BY, GetName().c_str());

        // stop flight if need
        if(pl->IsInFlight())
        {
            pl->GetMotionMaster()->MovementExpired();
            pl->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            pl->SaveRecallPosition();

        pl->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
    }

    return true;
}

//Summon group of player
bool ChatHandler::HandleGroupgoCommand(const char* args)
{
    ARGS_CHECK

    std::string name = args;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *player = sObjectAccessor->FindConnectedPlayerByName(name.c_str());
    if (!player)
    {
        PSendSysMessage(LANG_NO_PLAYER, args);
        SetSentErrorMessage(true);
        return false;
    }

    Group *grp = player->GetGroup();

    if(!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP,player->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    Map* gmMap = sMapMgr->CreateMap(m_session->GetPlayer()->GetMapId(),m_session->GetPlayer());
    bool to_instance =  gmMap->Instanceable();

    // we are in instance, and can summon only player in our group with us as lead
    if ( to_instance && (
        !m_session->GetPlayer()->GetGroup() || (grp->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
        (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ) )
        // the last check is a bit excessive, but let it be, just in case
    {
        SendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
        SetSentErrorMessage(true);
        return false;
    }

    for(GroupReference *itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player *pl = itr->GetSource();

        if(!pl || pl==m_session->GetPlayer() || !pl->GetSession() )
            continue;

        if(pl->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName().c_str());
            SetSentErrorMessage(true);
            return false;
        }

        if (to_instance)
        {
            Map* plMap = sMapMgr->CreateMap(pl->GetMapId(),pl);

            if ( plMap->Instanceable() && plMap->GetInstanceId() != gmMap->GetInstanceId() )
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,pl->GetName().c_str());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, pl->GetName().c_str(),"");
        if (needReportToTarget(pl))
            ChatHandler(pl).PSendSysMessage(LANG_SUMMONED_BY, GetName().c_str());

        // stop flight if need
        if(pl->IsInFlight())
        {
            pl->GetMotionMaster()->MovementExpired();
            pl->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            pl->SaveRecallPosition();

        // before GM
        float x,y,z;
        m_session->GetPlayer()->GetClosePoint(x,y,z,pl->GetObjectSize());
        pl->TeleportTo(m_session->GetPlayer()->GetMapId(),x,y,z,pl->GetOrientation());
    }

    return true;
}

bool ChatHandler::HandleTeleCommand(const char * args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    if (_player->IsBeingTeleported())
    {
        PSendSysMessage(LANG_IS_TELEPORTED, _player->GetName().c_str());
        SetSentErrorMessage(true);
        return false;
    }

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = extractGameTeleFromLink((char*)args);

    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    MapEntry const * map = sMapStore.LookupEntry(tele->mapId);
    if (!map || (map->IsBattlegroundOrArena() && (_player->GetMapId() != tele->mapId || !_player->IsGameMaster())))
    {
        SendSysMessage(LANG_CANNOT_TELE_TO_BG);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(_player->IsInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->CleanupAfterTaxiFlight();
    }
    // save only in non-flight case
    else
        _player->SaveRecallPosition();

    _player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
    return true;
}
