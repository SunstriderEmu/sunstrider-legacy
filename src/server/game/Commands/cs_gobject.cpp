#include "Chat.h"
#include "Language.h"
#include "GameEventMgr.h"
#include "Transport.h"

bool ChatHandler::HandleTargetObjectCommand(const char* args)
{
    Player* pl = m_session->GetPlayer();
    QueryResult result;
    GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr->GetActiveEventList();
    if(*args)
    {
        int32 id = atoi((char*)args);
        if(id)
            result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, orientation, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE map = '%i' AND id = '%u' ORDER BY order_ ASC LIMIT 1",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(),id);
        else
        {
            std::string name = args;
            WorldDatabase.EscapeString(name);
            result = WorldDatabase.PQuery(
                "SELECT guid, id, position_x, position_y, position_z, orientation, map, (POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ "
                "FROM gameobject,gameobject_template WHERE gameobject_template.entry = gameobject.id AND map = %i AND name " _LIKE_ " " _CONCAT3_ ("'%%'","'%s'","'%%'")" ORDER BY order_ ASC LIMIT 1",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(),name.c_str());
        }
    }
    else
    {
        std::ostringstream eventFilter;
        eventFilter << " AND (event IS NULL ";
        bool initString = true;

        for (unsigned short itr : activeEventsList)
        {
            if (initString)
            {
                eventFilter  <<  "OR event IN (" <<itr;
                initString =false;
            }
            else
                eventFilter << "," << itr;
        }

        if (!initString)
            eventFilter << "))";
        else
            eventFilter << ")";

        result = WorldDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, "
            "(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
            "LEFT OUTER JOIN game_event_gameobject on gameobject.guid=game_event_gameobject.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 1",
            m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ(), m_session->GetPlayer()->GetMapId(),eventFilter.str().c_str());
    }

    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
        return true;
    }

    Field *fields = result->Fetch();
    uint32 lowguid = fields[0].GetUInt32();
    uint32 id = fields[1].GetUInt32();
    float x = fields[2].GetFloat();
    float y = fields[3].GetFloat();
    float z = fields[4].GetFloat();
    float o = fields[5].GetFloat();
    int mapid = fields[6].GetUInt16();

    GameObjectTemplate const* goI = sObjectMgr->GetGameObjectTemplate(id);

    if (!goI)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        return false;
    }

    GameObject* target = ObjectAccessor::GetGameObject(*m_session->GetPlayer(),MAKE_NEW_GUID(lowguid,id,HighGuid::GameObject));

    PSendSysMessage(LANG_GAMEOBJECT_DETAIL, lowguid, goI->name.c_str(), lowguid, id, x, y, z, mapid, o);

    if(target)
    {
        int32 curRespawnDelay = target->GetRespawnTimeEx()-time(nullptr);
        if(curRespawnDelay < 0)
            curRespawnDelay = 0;

        std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay,true);
        std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(),true);

        PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(),curRespawnDelayStr.c_str());
    }
    return true;
}

//spawn go
bool ChatHandler::HandleGameObjectAddCommand(const char* args)
{
    ARGS_CHECK

    char* pParam1 = strtok((char*)args, " ");
    if (!pParam1)
        return false;

    uint32 id = atoi((char*)pParam1);
    if(!id)
        return false;

    char* spawntimeSecs = strtok(nullptr, " ");

    const GameObjectTemplate *goI = sObjectMgr->GetGameObjectTemplate(id);

    if (!goI)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = m_session->GetPlayer();
    float x = float(chr->GetPositionX());
    float y = float(chr->GetPositionY());
    float z = float(chr->GetPositionZ());
    float o = float(chr->GetOrientation());
    Map *map = chr->GetMap();

    float rot2 = sin(o/2);
    float rot3 = cos(o/2);
    
    /* Gobjects on transports have been added on LK (or did not found correct packet structure) */
    if (Transport* trans = chr->GetTransport())
    {
#ifdef LICH_KING
        uint32 guid = sObjectMgr->GenerateGameObjectSpawnId();
        GameObjectData& data = sObjectMgr->NewGOData(guid);
        data.id = id;
        data.posX = chr->GetTransOffsetX();
        data.posY = chr->GetTransOffsetY();
        data.posZ = chr->GetTransOffsetZ();
        data.orientation = chr->GetTransOffsetO();
        data.rotation = G3D::Quat(0, 0, rot2, rot3);
        data.spawntimesecs = 30;
        data.animprogress = 0;
        data.go_state = 1;
        data.spawnMask = 1;
        data.ArtKit = 0;

        if(!trans->GetGOInfo())
        {
            SendSysMessage("Error : Cannot save gameobject on transport because trans->GetGOInfo() == NULL");
            return true;
        }
        if(GameObject* gob = trans->CreateGOPassenger(guid, &data))
        {
            gob->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode());
            map->Add(gob);
            sTransportMgr->CreatePassengerForTransportInDB(PASSENGER_GAMEOBJECT,guid,trans->GetEntry());
            sObjectMgr->AddGameobjectToGrid(guid, &data);
        } else {
            SendSysMessage("Error : Cannot create gameobject passenger.");
        }
        return true;
#else
        SendSysMessage("Error : Cannot create gameobject passenger.");
#endif
    }

    auto pGameObj = new GameObject;
    uint32 db_lowGUID = sObjectMgr->GenerateGameObjectSpawnId();
    //use AddGOData instead? Then how do we spawn them in instance?

    if(!pGameObj->Create(db_lowGUID, goI->entry, map, chr->GetPhaseMask(), Position(x, y, z, o), G3D::Quat(0, 0, rot2, rot3), 0, GO_STATE_READY, 0, db_lowGUID))
    {
        delete pGameObj;
        return false;
    }

    if( spawntimeSecs )
    {
        uint32 value = atoi((char*)spawntimeSecs);
        pGameObj->SetRespawnTime(value);
        //TC_LOG_DEBUG("command","*** spawntimeSecs: %d", value);
    }
     
    // fill the gameobject data and save to the db
    pGameObj->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));

    delete pGameObj;
    pGameObj = sObjectMgr->IsGameObjectStaticTransport(goI->entry) ? new StaticTransport() : new GameObject();

    // this will generate a new guid if the object is in an instance
    if(!pGameObj->LoadFromDB(db_lowGUID, map))
    {
        delete pGameObj;
        SendSysMessage("Failed to create object");
        return true;
    }

    map->AddToMap(pGameObj);

    sObjectMgr->AddGameobjectToGrid(db_lowGUID, sObjectMgr->GetGOData(db_lowGUID));

    PSendSysMessage(LANG_GAMEOBJECT_ADD, id, goI->name.c_str(), db_lowGUID, x, y, z);
    return true;
}


//delete object by selection or guid
bool ChatHandler::HandleDelObjectCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if(!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if(!lowguid)
        return false;

    GameObject* obj = nullptr;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithSpawnId(lowguid,go_data->id);

    if(!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 owner_guid = obj->GetOwnerGUID();
    if(owner_guid)
    {
        Unit* owner = ObjectAccessor::GetUnit(*m_session->GetPlayer(),owner_guid);
        if(!owner && !IS_PLAYER_GUID(owner_guid))
        {
            PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, GUID_LOPART(owner_guid), obj->GetGUIDLow());
            SetSentErrorMessage(true);
            return false;
        }

        if(owner)
            owner->RemoveGameObject(obj,false);
    }

    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    obj->DeleteFromDB();

    PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, obj->GetGUIDLow());

    return true;
}

//turn selected object
bool ChatHandler::HandleTurnObjectCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if(!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if(!lowguid)
        return false;

    GameObject* obj = nullptr;
    Player *chr = m_session->GetPlayer();

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithSpawnId(lowguid,go_data->id);

    if(!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    char* po = strtok(nullptr, " ");
    float o;

    if (po)
    {
        o = (float)atof(po);
    }
    else
    {
        o = chr->GetOrientation();
    }

    float rot2 = sin(o/2);
    float rot3 = cos(o/2);

    chr->GetMap()->RemoveFromMap(obj, false);

    obj->Relocate(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), o);

    obj->SetFloatValue(GAMEOBJECT_FACING, o);
    obj->SetFloatValue(GAMEOBJECT_PARENTROTATION+2, rot2);
    obj->SetFloatValue(GAMEOBJECT_PARENTROTATION+3, rot3);

    obj->SetMap(chr->GetMap());
    chr->GetMap()->AddToMap(obj);

    obj->SaveToDB();
    obj->Refresh();

    PSendSysMessage(LANG_COMMAND_TURNOBJMESSAGE, obj->GetSpawnId(), o);

    return true;
}

//move selected object
bool ChatHandler::HandleMoveObjectCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if(!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if(!lowguid)
        return false;

    GameObject* obj = nullptr;
    Player* player = m_session->GetPlayer();

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithSpawnId(lowguid,go_data->id);

    if(!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    char* px = strtok(nullptr, " ");
    char* py = strtok(nullptr, " ");
    char* pz = strtok(nullptr, " ");

    if (!px)
    {
        Player *chr = m_session->GetPlayer();
        
        Map* map = chr->GetMap();
        map->RemoveFromMap(obj,false);

        obj->Relocate(chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), obj->GetOrientation());
        obj->SetFloatValue(GAMEOBJECT_POS_X, chr->GetPositionX());
        obj->SetFloatValue(GAMEOBJECT_POS_Y, chr->GetPositionY());
        obj->SetFloatValue(GAMEOBJECT_POS_Z, chr->GetPositionZ());

        obj->SetMap(map);
        map->AddToMap(obj, true);
    }
    else
    {
        if(!py || !pz)
            return false;

        float x = (float)atof(px);
        float y = (float)atof(py);
        float z = (float)atof(pz);

        if(!MapManager::IsValidMapCoord(obj->GetMapId(),x,y,z))
        {
            PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,obj->GetMapId());
            SetSentErrorMessage(true);
            return false;
        }

        Map* map = player->GetMap();
        map->RemoveFromMap(obj, false);

        obj->Relocate(x, y, z, obj->GetOrientation());
        obj->SetFloatValue(GAMEOBJECT_POS_X, x);
        obj->SetFloatValue(GAMEOBJECT_POS_Y, y);
        obj->SetFloatValue(GAMEOBJECT_POS_Z, z);

        obj->SetMap(map);
        map->AddToMap(obj, true);
    }

    obj->SaveToDB();
    obj->Refresh();

    PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, obj->GetGUIDLow());

    return true;
}

bool ChatHandler::HandleNearObjectCommand(const char* args)
{
    float distance = (!*args) ? 10 : atol(args);
    uint32 count = 0;

    Player* pl = m_session->GetPlayer();
    QueryResult result = WorldDatabase.PQuery("SELECT g.guid, id, position_x, position_y, position_z, map, geg.event, "
        "(POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ "
        "FROM gameobject g "
        "LEFT JOIN game_event_gameobject geg ON g.guid = geg.guid "
        "WHERE map='%u' AND (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) <= '%f' "
        "ORDER BY order_",
        pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),
        pl->GetMapId(),pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),distance*distance);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            float x = fields[2].GetFloat();
            float y = fields[3].GetFloat();
            float z = fields[4].GetFloat();
            int mapid = fields[5].GetUInt16();
            int linked_event = fields[6].GetInt16();

            GameObjectTemplate const * gInfo = sObjectMgr->GetGameObjectTemplate(entry);

            if(!gInfo)
                continue;

            PSendSysMessage(LANG_GO_LIST_CHAT, guid, entry, guid, gInfo->name.c_str(), x, y, z, mapid);
            if (linked_event)
                PSendSysMessage("Linked event: %i", linked_event);

            ++count;
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE,distance,count);
    return true;
}


bool ChatHandler::HandleActivateObjectCommand(const char *args)
{
    ARGS_CHECK

    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if(!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if(!lowguid)
        return false;

    GameObject* obj = nullptr;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithSpawnId(lowguid,go_data->id);

    if(!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    uint32_t const autoCloseTime = obj->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;

    // Activate
    obj->SetLootState(GO_READY);
    obj->UseDoorOrButton(autoCloseTime);

    PSendSysMessage("Object activated!");

    return true;
}

// add go, temp only
bool ChatHandler::HandleTempGameObjectCommand(const char* args)
{
    ARGS_CHECK
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    Player *chr = m_session->GetPlayer();

    char* spawntime = strtok(nullptr, " ");
    uint32 spawntm = 0;

    if( spawntime )
        spawntm = atoi((char*)spawntime);

    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float ang = chr->GetOrientation();

    float rot2 = sin(ang/2);
    float rot3 = cos(ang/2);

    uint32 id = atoi(charID);

    chr->SummonGameObject(id,Position(x,y,z,ang), G3D::Quat(0,0,rot2,rot3), spawntm);

    return true;
}

/* .gobject linkgameevent #event #guid */
bool ChatHandler::HandleGobLinkGameEventCommand(const char* args)
{
    GameObjectData const* data = nullptr;
    char* cEvent = strtok((char*)args, " ");
    char* cGobGUID = strtok(nullptr, " ");
    int16 event = 0;
    uint32 gobGUID = 0;

    if(!cEvent || !cGobGUID) 
       return false;

    event = atoi(cEvent);
    gobGUID = atoi(cGobGUID);

    if(!event || !gobGUID)
    {
        //PSendSysMessage("Valeurs incorrectes.");
        PSendSysMessage("Incorrect values.");
        SetSentErrorMessage(true);
        return false;
    }

    data = sObjectMgr->GetGOData(gobGUID);
    if(!data)
    {
        //PSendSysMessage("Gobject (guid : %u) introuvable.",gobGUID);
        PSendSysMessage("Gobject (guid: %u) not found.",gobGUID);
        SetSentErrorMessage(true);
        return false;
    }

    int16 currentEventId = sGameEventMgr->GetGameObjectEvent(gobGUID);
    if(currentEventId)
    {
        //PSendSysMessage("Le gobject est déjà lié à l'event %i.",currentEventId);
        PSendSysMessage("Gobject already linked to the event %i.",currentEventId);
        SetSentErrorMessage(true);
        return false;
    }

    if(sGameEventMgr->AddGameObjectToEvent(gobGUID, event))
        //PSendSysMessage("Le gobject (guid : %u) a été lié à l'event %i.",gobGUID,event);
        PSendSysMessage("Gobject (guid: %u) is now linked to the event %i.",gobGUID,event);
    else
        //PSendSysMessage("Erreur : Le gobject (guid : %u) n'a pas pu être lié à l'event %d (event inexistant ?).",gobGUID,event);
        PSendSysMessage("Error: gobject (guid: %u) could not be linked to the event %d (event nonexistent?).",gobGUID,event);

    return true;
}

/*.gobject unlinkgameevent #guid*/
bool ChatHandler::HandleGobUnlinkGameEventCommand(const char* args)
{
    GameObjectData const* data = nullptr;
    char* cGobGUID = strtok((char*)args, " ");
    uint32 gobGUID = 0;

    if(!cGobGUID)
        return false;

    gobGUID = atoi(cGobGUID);

    data = sObjectMgr->GetGOData(gobGUID);
    if(!data)
    {
        //PSendSysMessage("Gobject avec le guid %u introuvable.",gobGUID);
        PSendSysMessage("Gobject with guid %u not found.",gobGUID);
        SetSentErrorMessage(true);
        return false;
    } 

    int16 currentEventId = sGameEventMgr->GetGameObjectEvent(gobGUID);
    if (!currentEventId)
    {
        //PSendSysMessage("Le gobject (guid : %u) n'est lié à aucun event.",gobGUID);
        PSendSysMessage("Gobject (guid: %u) is not linked to any event.",gobGUID);
    } else {
        if(sGameEventMgr->RemoveGameObjectFromEvent(gobGUID))
            //PSendSysMessage("Le gobject (guid : %u) n'est plus lié à l'event %i.",gobGUID,currentEventId);
            PSendSysMessage("Gobject (guid: %u) is not linked anymore to the event %i.",gobGUID,currentEventId);
        else
            //PSendSysMessage("Erreur lors de la suppression du gobject (guid : %u) de l'event %i.",gobGUID,currentEventId);
            PSendSysMessage("Error on removing gobject (guid: %u) from the event %i.",gobGUID,currentEventId);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

/* Syntax : .gobject getvalue #guid #index [uint32/uint64/float]*/
bool ChatHandler::HandleGobGetValueCommand(const char * args)
{
    ARGS_CHECK

    char* cGUID = strtok((char*)args, " ");
    char* cIndex = strtok(nullptr, " ");
    char* cType = strtok(nullptr, " ");

    if (!cGUID || !cIndex)
        return false;

    uint64 guid = atoi(cGUID);
    if(!guid) 
        return false;

     GameObject* target = nullptr;
    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid))
        target = GetObjectGlobalyWithGuidOrNearWithSpawnId(guid,go_data->id);

    if(!target)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 index = (uint32)atoi(cIndex);
    if(index >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, index, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint64 uValue;
    float fValue;
    uint8 type = 0;
    if(cType)
    {
        if( strcmp(cType, "float") == 0 )
            type = 2;
        else if ( strcmp(cType, "uint64") == 0 )
            type = 1;
        else if ( strcmp(cType, "uint32") == 0 )
            type = 0;
    }

    switch(type)
    {
    case 0: //uint32
        uValue = target->GetUInt32Value(index);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
        break;
    case 1: //uint64
        uValue = target->GetUInt64Value(index);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
    case 2: //float
        fValue = target->GetFloatValue(index);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), index, fValue);
        break;
    }

    return true;
}

/* Syntax : .gobject setvalue #guid #index #value [uint32/uint64/float]*/
bool ChatHandler::HandleGobSetValueCommand(const char* args)
{
    ARGS_CHECK

    char* cGUID = strtok((char*)args, " ");
    char* cIndex = strtok(nullptr, " ");
    char* cValue = strtok(nullptr, " ");
    char* cType = strtok(nullptr, " ");

    if (!cGUID || !cIndex || !cValue)
        return false;

    uint64 guid = atoi(cGUID);
    if(!guid) 
        return false;

     GameObject* target = nullptr;
    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr->GetGOData(guid))
        target = GetObjectGlobalyWithGuidOrNearWithSpawnId(guid, go_data->id);

    if(!target)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 index = (uint32)atoi(cIndex);
    if(index >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, index, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint64 uValue;
    float fValue;
    uint8 type = 0;
    if(cType)
    {
        if( strcmp(cType, "float") == 0 )
            type = 2;
        else if ( strcmp(cType, "uint64") == 0 )
            type = 1;
        else if ( strcmp(cType, "uint32") == 0 )
            type = 0;
    }

    switch(type)
    {
    case 0: //uint32
        uValue = (uint32)atoi(cValue);
        target->SetUInt32Value(index,uValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
        break;
    case 1: //uint64
        uValue = (uint64)atoi(cValue);
        target->SetUInt64Value(index,uValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), index, uValue);
    case 2: //float
        fValue = (float)atof(cValue);
        target->SetFloatValue(index,fValue);
        PSendSysMessage(LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), index, fValue);
        break;
    }

    //update visual
    if(Map* map = sMapMgr->FindMap(target->GetMapId(), target->GetInstanceId()))
    {
        map->RemoveFromMap(target,false);
        target->SetMap(map);
        map->AddToMap(target, true);
    }

    return true;
}