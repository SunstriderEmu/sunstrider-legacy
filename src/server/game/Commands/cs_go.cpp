#include "Chat.h"
#include "Language.h"
#include "Transport.h"

bool ChatHandler::HandleGoTicketCommand(const char * args)
{
    ARGS_CHECK

    char *cstrticket_id = strtok((char*)args, " ");

    if(!cstrticket_id)
        return false;

    uint64 ticket_id = atoi(cstrticket_id);
    if(!ticket_id)
        return false;

    GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticket_id);
    if(!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    float x, y, z;
    int mapid;

    x = ticket->pos_x;
    y = ticket->pos_y;
    z = ticket->pos_z;
    mapid = ticket->map;

    Player* _player = m_session->GetPlayer();
    if(_player->IsInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->CleanupAfterTaxiFlight();
    }
     else
        _player->SaveRecallPosition();

    _player->TeleportTo(mapid, x, y, z, 1, 0);
    return true;
}

bool ChatHandler::HandleGoObjectCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();
    
    char* cId = strtok((char*)args, " ");
    if (!cId)
        return false;
        
    float x, y, z, ort;
    uint32 mapid;

    if (strcmp(cId, "id") == 0) {
        char* cEntry = strtok(nullptr, "");
        if (!cEntry)
            return false;

        uint32 entry = atoi(cEntry);
        if (!entry)
            return false;
            
        QueryResult result = WorldDatabase.PQuery("SELECT position_x, position_y, position_z, orientation, map FROM gameobject WHERE id = %u LIMIT 1", entry);
        if (!result) {
            SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
            return true;
        }
        
        Field* fields = result->Fetch();
        x = fields[0].GetFloat();
        y = fields[1].GetFloat();
        z = fields[2].GetFloat();
        ort = fields[3].GetFloat();
        mapid = fields[4].GetUInt32();
    }
    else {
        uint32 guid = atoi(cId);
        if (!guid)
            return false;

        if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(guid))
        {
            go_data->spawnPoint.GetPosition(x, y, z, ort);
            mapid = go_data->spawnPoint.GetMapId();
        }
        else
        {
            SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }

    if(!MapManager::IsValidMapCoord(mapid,x,y,z,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
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

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}


//teleport to grid
bool ChatHandler::HandleGoGridCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(nullptr, " ");
    char* pmapid = strtok(nullptr, " ");

    if (!px || !py)
        return false;

    float grid_x = (float)atof(px);
    float grid_y = (float)atof(py);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else mapid = _player->GetMapId();

    // center of grid
    float x = (grid_x-CENTER_GRID_ID+0.5f)*SIZE_OF_GRIDS;
    float y = (grid_y-CENTER_GRID_ID+0.5f)*SIZE_OF_GRIDS;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
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

    Map const *map = sMapMgr->CreateBaseMap(mapid);
    float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport at coordinates
bool ChatHandler::HandleGoXYCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(nullptr, " ");
    char* pmapid = strtok(nullptr, " ");

    if (!px || !py)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else mapid = _player->GetMapId();

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
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

    Map const *map = sMapMgr->CreateBaseMap(mapid);
    float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport at coordinates, including Z
bool ChatHandler::HandleGoXYZCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(nullptr, " ");
    char* pz = strtok(nullptr, " ");
    char* pmapid = strtok(nullptr, " ");

    if (!px || !py || !pz)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else
        mapid = _player->GetMapId();

    if(!MapManager::IsValidMapCoord(mapid,x,y,z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
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

    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

bool ChatHandler::HandleGoXYZOCommand(const char* args)
{
    ARGS_CHECK

        Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(nullptr, " ");
    char* pz = strtok(nullptr, " ");
    char* po = strtok(nullptr, " ");
    char* pmapid = strtok(nullptr, " ");

    if (!px || !py || !pz || !po)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);
    float o = (float)atof(po);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else
        mapid = _player->GetMapId();

    if (!MapManager::IsValidMapCoord(mapid, x, y, z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
        SetSentErrorMessage(true);
        return false;
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

    _player->TeleportTo(mapid, x, y, z, o);

    return true;
}

//teleport at coordinates
bool ChatHandler::HandleGoZoneXYCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(nullptr, " ");
    char* tail = strtok(nullptr,"");

    char* cAreaId = extractKeyFromLink(tail,"Harea");       // string or [name] Shift-click form |color|Harea:area_id|h[name]|h|r

    if (!px || !py)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    uint32 areaid = cAreaId ? (uint32)atoi(cAreaId) : _player->GetAreaId();

    AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaid);

    if( x<0 || x>100 || y<0 || y>100 || !areaEntry )
    {
        PSendSysMessage(LANG_INVALID_ZONE_COORD,x,y,areaid);
        SetSentErrorMessage(true);
        return false;
    }

    // update to parent zone if exist (client map show only zones without parents)
    areaEntry = areaEntry->zone ? sAreaTableStore.LookupEntry(areaEntry->zone) : areaEntry;

    Map const *map = sMapMgr->CreateBaseMap(areaEntry->mapid);

    if(map->Instanceable())
    {
        PSendSysMessage(LANG_INVALID_ZONE_MAP,areaEntry->ID,areaEntry->area_name[GetSessionDbcLocale()],map->GetId(),map->GetMapName());
        SetSentErrorMessage(true);
        return false;
    }

    Zone2MapCoordinates(x,y, areaEntry->zone);

    if(!MapManager::IsValidMapCoord(areaEntry->mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y, areaEntry->mapid);
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

    float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
    _player->TeleportTo(areaEntry->mapid, x, y, z, _player->GetOrientation());

    return true;
}

bool ChatHandler::HandleGoTriggerCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    char *atId = strtok((char*)args, " ");
    if (!atId)
        return false;

    int32 i_atId = atoi(atId);

    if(!i_atId)
        return false;

    AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(i_atId);
    if (!at)
    {
        PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND,i_atId);
        SetSentErrorMessage(true);
        return false;
    }

    if(!MapManager::IsValidMapCoord(at->mapid,at->x,at->y,at->z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,at->x,at->y,at->mapid);
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

    _player->TeleportTo(at->mapid, at->x, at->y, at->z, _player->GetOrientation());
    return true;
}

bool ChatHandler::HandleGoGraveyardCommand(const char* args)
{
    

    Player* _player = m_session->GetPlayer();

    if (!*args)
        return false;

    char *gyId = strtok((char*)args, " ");
    if (!gyId)
        return false;

    int32 i_gyId = atoi(gyId);

    if(!i_gyId)
        return false;

    WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(i_gyId);
    if (!gy)
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST,i_gyId);
        SetSentErrorMessage(true);
        return false;
    }

    if(!MapManager::IsValidMapCoord(gy->map_id,gy->x,gy->y,gy->z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,gy->x,gy->y,gy->map_id);
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

    _player->TeleportTo(gy->map_id, gy->x, gy->y, gy->z, _player->GetOrientation());
    return true;
}

/** \brief Teleport the GM to the specified creature
 *
 * .gocreature <GUID>      --> TP using creature.guid
 * .gocreature azuregos    --> TP player to the mob with this name
 *                             Warning: If there is more than one mob with this name
 *                                      you will be teleported to the first one that is found.
 * .gocreature id 6109     --> TP player to the mob, that has this creature_template.entry
 *                             Warning: If there is more than one mob with this "id"
 *                                      you will be teleported to the first one that is found.
 */
//teleport to creature
bool ChatHandler::HandleGoCreatureCommand(const char* args)
{
    ARGS_CHECK

    Player* _player = m_session->GetPlayer();

    // "id" or number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
    char* pParam1 = extractKeyFromLink((char*)args,"Hcreature");
    if (!pParam1)
        return false;

    std::ostringstream whereClause;

    // User wants to teleport to the NPC's template entry
    if( strcmp(pParam1, "id") == 0 )
    {
        //TC_LOG_ERROR("command","DEBUG: ID found");

        // Get the "creature_template.entry"
        // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
        char* tail = strtok(nullptr,"");
        if(!tail)
            return false;
        char* cId = extractKeyFromLink(tail,"Hcreature_entry");
        if(!cId)
            return false;

        int32 tEntry = atoi(cId);
        //TC_LOG_ERROR("DEBUG: ID value: %d", tEntry);
        if(!tEntry)
            return false;

        whereClause << "WHERE id = '" << tEntry << "'";
    }
    else
    {
        //TC_LOG_ERROR("command","DEBUG: ID *not found*");

        int32 guid = atoi(pParam1);

        // Number is invalid - maybe the user specified the mob's name
        if(!guid)
        {
            std::string name = pParam1;
            WorldDatabase.EscapeString(name);
            whereClause << ", creature_template WHERE creature.id = creature_template.entry AND creature_template.name LIKE '" << name << "'";
        }
        else
        {
            whereClause <<  "WHERE guid = '" << guid << "'";
            QueryResult result = WorldDatabase.PQuery("SELECT id FROM creature WHERE guid = %u LIMIT 1", guid);
            if (result) {
                Field *fields = result->Fetch();
                uint32 creatureentry = fields[0].GetUInt32();

                ObjectGuid fullguid = ObjectGuid(HighGuid::Unit, creatureentry, uint32(guid));
                if (Unit *cre = ObjectAccessor::GetUnit((*_player), fullguid)) {
                    PSendSysMessage("Creature found, you are now teleported on its current location!");

                    // stop flight if need
                    if(_player->IsInFlight())
                    {
                        _player->GetMotionMaster()->MovementExpired();
                        _player->CleanupAfterTaxiFlight();
                    }
                    // save only in non-flight case
                    else
                        _player->SaveRecallPosition();

                    _player->TeleportTo(cre->GetMapId(), cre->GetPositionX(), cre->GetPositionY(), cre->GetPositionZ(), cre->GetOrientation());
                    if(Transport* transport = cre->GetTransport())
                        transport->AddPassenger(_player);
                    return true;
                }
            }
        }
    }
    //TC_LOG_ERROR("DEBUG: %s", whereClause.c_str());

    QueryResult result = WorldDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map FROM creature %s", whereClause.str().c_str() );
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }
    if( result->GetRowCount() > 1 )
    {
        SendSysMessage(LANG_COMMAND_GOCREATMULTIPLE);
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();

    if(!MapManager::IsValidMapCoord(mapid,x,y,z,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
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

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleGoATCommand(const char* args)
{
    ARGS_CHECK
        
    Player* plr = m_session->GetPlayer();
    if (!plr)
        return false;
        
    char* atIdChar = strtok((char*)args, " ");
    int atId = atoi(atIdChar);
    if (!atId)
        return false;
        
    AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(atId);
    if (!at)
        return false;
        
    // Teleport player on at coords
    plr->TeleportTo(at->mapid, at->x, at->y, at->z, plr->GetOrientation());
    return true;
}
