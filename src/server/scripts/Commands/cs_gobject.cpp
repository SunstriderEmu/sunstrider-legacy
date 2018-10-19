#include "Chat.h"
#include "Language.h"
#include "GameEventMgr.h"
#include "Transport.h"
#include "PoolMgr.h"
#include "MapManager.h"

// definitions are over in cs_npc.cpp
bool HandleNpcSpawnGroup(ChatHandler* handler, char const* args);
bool HandleNpcDespawnGroup(ChatHandler* handler, char const* args);

class gobject_commandscript : public CommandScript
{
public:
    gobject_commandscript() : CommandScript("gobject_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> gobjectCommandTable =
        {
            { "add",            SEC_GAMEMASTER3,     false, &HandleGameObjectAddCommand,       "" },
            { "delete",         SEC_GAMEMASTER3,     false, &HandleGameObjectDeleteCommand,    "" },
            { "target",         SEC_GAMEMASTER2,     false, &HandleTargetObjectCommand,        "" },
            { "turn",           SEC_GAMEMASTER3,     false, &HandleTurnObjectCommand,          "" },
            { "move",           SEC_GAMEMASTER3,     false, &HandleMoveObjectCommand,          "" },
            { "near",           SEC_GAMEMASTER3,     false, &HandleNearObjectCommand,          "" },
            { "activate",       SEC_GAMEMASTER2,     false, &HandleActivateObjectCommand,      "" },
            { "addtemp",        SEC_GAMEMASTER3,     false, &HandleTempGameObjectCommand,      "" },
            { "linkgameevent",  SEC_ADMINISTRATOR,   false, &HandleGobLinkGameEventCommand,    "" },
            { "unlinkgameevent",SEC_ADMINISTRATOR,   false, &HandleGobUnlinkGameEventCommand,  "" },
            { "getvalue",       SEC_GAMEMASTER3,     false, &HandleGobGetValueCommand,         "" },
            { "setvalue",       SEC_GAMEMASTER3,     false, &HandleGobSetValueCommand,         "" },
            { "spawngroup",     SEC_ADMINISTRATOR,   false, &HandleNpcSpawnGroup,              "" },
            { "despawngroup",   SEC_ADMINISTRATOR,   false, &HandleNpcDespawnGroup,            "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "gobject",        SEC_GAMEMASTER2,  false, nullptr,                              "", gobjectCommandTable },
        };
        return commandTable;
    }

    static void SendInfoAbout(uint32 spawnId, uint32 entry, WorldLocation loc, ChatHandler* chatHandler)
    {
        Player* pl = chatHandler->GetSession()->GetPlayer();
        GameObjectTemplate const* gInfo = sObjectMgr->GetGameObjectTemplate(entry);
        if (!gInfo || !pl)
            return;

        //orientation now shown here
        chatHandler->PSendSysMessage(LANG_GO_LIST_CHAT, spawnId, entry, spawnId, gInfo->name.c_str(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetMapId());
        GameObject* target = pl->GetMap()->GetGameObjectBySpawnId(spawnId);
        if (target)
        {
            int32 curRespawnDelay = target->GetRespawnTimeEx() - time(nullptr);
            if (curRespawnDelay < 0)
                curRespawnDelay = 0;

            std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay, true);
            std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

            chatHandler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
        }

        SpawnData const* data = sObjectMgr->GetSpawnData(SPAWN_TYPE_GAMEOBJECT, spawnId);
        if (data && data->spawnGroupData)
            chatHandler->PSendSysMessage("Member of spawn group %u", data->spawnGroupData->groupId);

        if (uint32 poolid = sPoolMgr->IsPartOfAPool<GameObject>(spawnId))
            chatHandler->PSendSysMessage("Part of pool %u", poolid);
    }

#define _CONCAT3_(A, B, C) "CONCAT( " A ", " B ", " C " )"

    static bool HandleTargetObjectCommand(ChatHandler* handler, char const* args)
    {
        Player* pl = handler->GetSession()->GetPlayer();
        QueryResult result;
        GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr->GetActiveEventList();
        if (*args)
        {
            int32 id = atoi((char*)args);
            if (id)
                result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, orientation, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE map = '%i' AND id = '%u' ORDER BY order_ ASC LIMIT 1",
                    pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(), id);
            else
            {
                std::string name = args;
                WorldDatabase.EscapeString(name);
                result = WorldDatabase.PQuery(
                    "SELECT guid, id, position_x, position_y, position_z, orientation, map, (POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ "
                    "FROM gameobject,gameobject_template WHERE gameobject_template.entry = gameobject.id AND map = %i AND name LIKE " _CONCAT3_("'%%'", "'%s'", "'%%'")" ORDER BY order_ ASC LIMIT 1",
                    pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(), name.c_str());
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
                    eventFilter << "OR event IN (" << itr;
                    initString = false;
                }
                else
                    eventFilter << "," << itr;
            }

            if (!initString)
                eventFilter << "))";
            else
                eventFilter << ")";

            result = WorldDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, geg.event "
                "(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
                "LEFT OUTER JOIN game_event_gameobject geg ON gameobject.guid=geg.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 1",
                handler->GetSession()->GetPlayer()->GetPositionX(), handler->GetSession()->GetPlayer()->GetPositionY(), handler->GetSession()->GetPlayer()->GetPositionZ(), handler->GetSession()->GetPlayer()->GetMapId(), eventFilter.str().c_str());
        }

        if (!result)
        {
            handler->SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
            return true;
        }

        Field *fields = result->Fetch();
        uint32 spawnId = fields[0].GetUInt32();
        uint32 entry = fields[1].GetUInt32();
        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();
        int mapid = fields[6].GetUInt16();
        int linked_event = fields[7].GetInt16();

        SendInfoAbout(spawnId, entry, WorldLocation(mapid, x, y, z, o), handler);
        if (linked_event)
            handler->PSendSysMessage("Linked game_event: %i", linked_event);
        return true;
    }

    //spawn go
    static bool HandleGameObjectAddCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* pParam1 = strtok((char*)args, " ");
        if (!pParam1)
            return false;

        uint32 id = atoi((char*)pParam1);
        if (!id)
            return false;

        char* spawntimeSecs = strtok(nullptr, " ");

        const GameObjectTemplate *goI = sObjectMgr->GetGameObjectTemplate(id);

        if (!goI)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* chr = handler->GetSession()->GetPlayer();
        float x = float(chr->GetPositionX());
        float y = float(chr->GetPositionY());
        float z = float(chr->GetPositionZ());
        float o = float(chr->GetOrientation());
        Map *map = chr->GetMap();

        float rot2 = sin(o / 2);
        float rot3 = cos(o / 2);

        /* Gobjects on transports have been added on LK (or did not found correct packet structure) */
        if (Transport* trans = chr->GetTransport())
        {
#ifdef LICH_KING
            uint32 guid = sObjectMgr->GenerateGameObjectSpawnId();
            GameObjectData& data = sObjectMgr->NewOrExistGameObjectData(guid);
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

            if (!trans->GetGOInfo())
            {
                handler->SendSysMessage("Error : Cannot save gameobject on transport because trans->GetGOInfo() == NULL");
                return true;
            }
            if (GameObject* gob = trans->CreateGOPassenger(guid, &data))
            {
                gob->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode());
                map->Add(gob);
                sTransportMgr->CreatePassengerForTransportInDB(PASSENGER_GAMEOBJECT, guid, trans->GetEntry());
                sObjectMgr->AddGameobjectToGrid(guid, &data);
            }
            else {
                handler->SendSysMessage("Error : Cannot create gameobject passenger.");
            }
            return true;
#else
            handler->SendSysMessage("Error : Cannot create gameobject passenger.");
#endif
        }

        auto pGameObj = new GameObject;
        ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();
        //use AddGameObjectData instead? Then how do we spawn them in instance?

        if (!pGameObj->Create(guidLow, goI->entry, map, chr->GetPhaseMask(), { x, y, z, o }, G3D::Quat(0, 0, rot2, rot3), 0, GO_STATE_READY))
        {
            delete pGameObj;
            return false;
        }

        if (spawntimeSecs)
        {
            uint32 value = atoi((char*)spawntimeSecs);
            pGameObj->SetRespawnTime(value);
            //TC_LOG_DEBUG("command","*** spawntimeSecs: %d", value);
        }

        // fill the gameobject data and save to the db
        pGameObj->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()));
        guidLow = pGameObj->GetSpawnId();

        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete pGameObj;

        pGameObj = sObjectMgr->CreateGameObject(goI->entry);
        // this will generate a new guid if the object is in an instance
        if (!pGameObj->LoadFromDB(guidLow, map, true))
        {
            delete pGameObj;
            handler->SendSysMessage("Failed to create object");
            return true;
        }

        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));

        handler->PSendSysMessage(LANG_GAMEOBJECT_ADD, id, goI->name.c_str(), guidLow, x, y, z);
        return true;
    }

    //delete object by selection or guid
    static bool HandleGameObjectDeleteCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!cId)
            return false;

        ObjectGuid::LowType guidLow = atoi(cId);
        if (!guidLow)
            return false;

        Player const* const player = handler->GetSession()->GetPlayer();
        // force respawn to make sure we find something
        player->GetMap()->ForceRespawn(SPAWN_TYPE_GAMEOBJECT, guidLow);
        GameObject* object = handler->GetObjectFromPlayerMapByDbGuid(guidLow);
        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid ownerGuid = object->GetOwnerGUID();
        if (ownerGuid)
        {
            Unit* owner = ObjectAccessor::GetUnit(*handler->GetSession()->GetPlayer(), ownerGuid);
            if (!owner || !ownerGuid.IsPlayer())
            {
                handler->PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, ownerGuid.GetCounter(), object->GetSpawnId());
                handler->SetSentErrorMessage(true);
                return false;
            }

            owner->RemoveGameObject(object, false);
        }

        object->SetRespawnTime(0);                                 // not save respawn time
        object->Delete();
        object->DeleteFromDB();

        handler->PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, object->GetSpawnId());

        return true;
    }

    //turn selected object
    static bool HandleTurnObjectCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!cId)
            return false;

        ObjectGuid::LowType lowguid = atoi(cId);
        if (!lowguid)
            return false;

        GameObject* obj = nullptr;
        Player *chr = handler->GetSession()->GetPlayer();

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(lowguid))
            obj = handler->GetObjectFromPlayerMapByDbGuid(lowguid);

        if (!obj)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
            handler->SetSentErrorMessage(true);
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


        chr->GetMap()->RemoveFromMap(obj, false);

        obj->Relocate(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), o);

        obj->SetFloatValue(GAMEOBJECT_FACING, o);
        float rot2 = sin(o / 2.f);
        float rot3 = cos(o / 2.f);
        obj->SetFloatValue(GAMEOBJECT_PARENTROTATION + 2, rot2);
        obj->SetFloatValue(GAMEOBJECT_PARENTROTATION + 3, rot3);

        obj->SetMap(chr->GetMap());
        chr->GetMap()->AddToMap(obj);

        obj->SaveToDB();
        obj->Refresh();

        handler->PSendSysMessage(LANG_COMMAND_TURNOBJMESSAGE, obj->GetSpawnId(), o);

        return true;
    }

    //move selected object
    static bool HandleMoveObjectCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!cId)
            return false;

        ObjectGuid::LowType lowguid = atoi(cId);
        if (!lowguid)
            return false;

        GameObject* obj = nullptr;
        Player* player = handler->GetSession()->GetPlayer();

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(lowguid))
            obj = handler->GetObjectFromPlayerMapByDbGuid(lowguid);

        if (!obj)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* px = strtok(nullptr, " ");
        char* py = strtok(nullptr, " ");
        char* pz = strtok(nullptr, " ");

        if (!px)
        {
            Player *chr = handler->GetSession()->GetPlayer();

            Map* map = chr->GetMap();
            map->RemoveFromMap(obj, false);

            obj->Relocate(chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), obj->GetOrientation());
            obj->SetFloatValue(GAMEOBJECT_POS_X, chr->GetPositionX());
            obj->SetFloatValue(GAMEOBJECT_POS_Y, chr->GetPositionY());
            obj->SetFloatValue(GAMEOBJECT_POS_Z, chr->GetPositionZ());

            obj->SetMap(map);
            map->AddToMap(obj, true);
        }
        else
        {
            if (!py || !pz)
                return false;

            float x = (float)atof(px);
            float y = (float)atof(py);
            float z = (float)atof(pz);

            if (!MapManager::IsValidMapCoord(obj->GetMapId(), x, y, z))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, obj->GetMapId());
                handler->SetSentErrorMessage(true);
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

        handler->PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, obj->GetSpawnId());

        return true;
    }

    static bool HandleNearObjectCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10 : atol(args);
        uint32 count = 0;

        Player* pl = handler->GetSession()->GetPlayer();
        QueryResult result = WorldDatabase.PQuery("SELECT g.guid, id, position_x, position_y, position_z, orientation, map, geg.event, "
            "(POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ "
            "FROM gameobject g "
            "LEFT JOIN game_event_gameobject geg ON g.guid = geg.guid "
            "WHERE map='%u' AND (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) <= '%f' "
            "ORDER BY order_",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),
            pl->GetMapId(), pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), distance*distance);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 spawnId = fields[0].GetUInt32();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                float o = fields[5].GetFloat();
                int mapid = fields[6].GetUInt16();
                int linked_event = fields[7].GetInt16();

                SendInfoAbout(spawnId, entry, WorldLocation(mapid, x, y, z, o), handler);
                if (linked_event)
                    handler->PSendSysMessage("Linked game_event: %i", linked_event);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE, distance, count);
        return true;
    }


    static bool HandleActivateObjectCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* cId = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!cId)
            return false;

        ObjectGuid::LowType lowguid = atoi(cId);
        if (!lowguid)
            return false;

        GameObject* obj = nullptr;

        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(lowguid))
            obj = handler->GetObjectFromPlayerMapByDbGuid(lowguid);

        if (!obj)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32_t const autoCloseTime = obj->GetGOInfo()->GetAutoCloseTime() ? 10000u : 0u;

        // Activate
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(autoCloseTime);

        handler->PSendSysMessage("Object activated!");

        return true;
    }

    // add go, temp only
    static bool HandleTempGameObjectCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK
            char* charID = strtok((char*)args, " ");
        if (!charID)
            return false;

        Player *chr = handler->GetSession()->GetPlayer();

        char* spawntime = strtok(nullptr, " ");
        uint32 spawntm = 0;

        if (spawntime)
            spawntm = atoi((char*)spawntime);

        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float ang = chr->GetOrientation();

        float rot2 = sin(ang / 2);
        float rot3 = cos(ang / 2);

        uint32 id = atoi(charID);

        chr->SummonGameObject(id, Position(x, y, z, ang), G3D::Quat(0, 0, rot2, rot3), spawntm);

        return true;
    }

    /* .gobject linkgameevent #event #guid */
    static bool HandleGobLinkGameEventCommand(ChatHandler* handler, char const* args)
    {
        GameObjectData const* data = nullptr;
        char* cEvent = strtok((char*)args, " ");
        char* cGobGUID = strtok(nullptr, " ");
        int16 event = 0;
        ObjectGuid::LowType gobGUID = 0;

        if (!cEvent || !cGobGUID)
            return false;

        event = atoi(cEvent);
        gobGUID = atoi(cGobGUID);

        if (!event || !gobGUID)
        {
            //PSendSysMessage("Valeurs incorrectes.");
            handler->PSendSysMessage("Incorrect values.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        data = sObjectMgr->GetGameObjectData(gobGUID);
        if (!data)
        {
            //PSendSysMessage("Gobject (guid : %u) introuvable.",gobGUID);
            handler->PSendSysMessage("Gobject (guid: %u) not found.", gobGUID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid fullGUID = ObjectGuid(HighGuid::GameObject, data->id, gobGUID);

        int16 currentEventId = sGameEventMgr->GetGameObjectEvent(fullGUID);
        if (currentEventId)
        {
            //PSendSysMessage("Le gobject est déjà lié à l'event %i.",currentEventId);
            handler->PSendSysMessage("Gobject already linked to the event %i.", currentEventId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (sGameEventMgr->AddGameObjectToEvent(fullGUID, event))
            //PSendSysMessage("Le gobject (guid : %u) a été lié à l'event %i.",gobGUID,event);
            handler->PSendSysMessage("Gobject (guid: %u) is now linked to the event %i.", gobGUID, event);
        else
            //PSendSysMessage("Erreur : Le gobject (guid : %u) n'a pas pu être lié à l'event %d (event inexistant ?).",gobGUID,event);
            handler->PSendSysMessage("Error: gobject (guid: %u) could not be linked to the event %d (event nonexistent?).", gobGUID, event);

        return true;
    }

    /*.gobject unlinkgameevent #guid*/
    static bool HandleGobUnlinkGameEventCommand(ChatHandler* handler, char const* args)
    {
        GameObjectData const* data = nullptr;
        char* cGobGUID = strtok((char*)args, " ");
        ObjectGuid::LowType gobGUID = 0;

        if (!cGobGUID)
            return false;

        gobGUID = atoi(cGobGUID);

        data = sObjectMgr->GetGameObjectData(gobGUID);
        if (!data)
        {
            //PSendSysMessage("Gobject avec le guid %u introuvable.",gobGUID);
            handler->PSendSysMessage("Gobject with guid %u not found.", gobGUID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid fullGUID = ObjectGuid(HighGuid::GameObject, data->id, gobGUID);
        int16 currentEventId = sGameEventMgr->GetGameObjectEvent(fullGUID);
        if (!currentEventId)
        {
            //PSendSysMessage("Le gobject (guid : %u) n'est lié à aucun event.",gobGUID);
            handler->PSendSysMessage("Gobject (guid: %u) is not linked to any event.", gobGUID);
        }
        else {
            if (sGameEventMgr->RemoveGameObjectFromEvent(fullGUID))
                //PSendSysMessage("Le gobject (guid : %u) n'est plus lié à l'event %i.",gobGUID,currentEventId);
                handler->PSendSysMessage("Gobject (guid: %u) is not linked anymore to the event %i.", gobGUID, currentEventId);
            else
                //PSendSysMessage("Erreur lors de la suppression du gobject (guid : %u) de l'event %i.",gobGUID,currentEventId);
                handler->PSendSysMessage("Error on removing gobject (guid: %u) from the event %i.", gobGUID, currentEventId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    /* Syntax : .gobject getvalue #guid #index [uint32/uint64/float]*/
    static bool HandleGobGetValueCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* cGUID = strtok((char*)args, " ");
        char* cIndex = strtok(nullptr, " ");
        char* cType = strtok(nullptr, " ");

        if (!cGUID || !cIndex)
            return false;

        ObjectGuid::LowType guid = atoi(cGUID);
        if (!guid)
            return false;

        GameObject* target = nullptr;
        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(guid))
            target = handler->GetObjectFromPlayerMapByDbGuid(guid);

        if (!target)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 index = (uint32)atoi(cIndex);
        if (index >= target->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, index, uint32(guid), target->GetValuesCount());
            return false;
        }
        uint64 uValue;
        float fValue;
        uint8 type = 0;
        if (cType)
        {
            if (strcmp(cType, "float") == 0)
                type = 2;
            else if (strcmp(cType, "uint64") == 0)
                type = 1;
            else if (strcmp(cType, "uint32") == 0)
                type = 0;
        }

        switch (type)
        {
        case 0: //uint32
            uValue = target->GetUInt32Value(index);
            handler->PSendSysMessage(LANG_GET_UINT_FIELD, guid, index, uValue);
            break;
        case 1: //uint64
            uValue = target->GetUInt64Value(index);
            handler->PSendSysMessage(LANG_GET_UINT_FIELD, guid, index, uValue);
            break;
        case 2: //float
            fValue = target->GetFloatValue(index);
            handler->PSendSysMessage(LANG_GET_FLOAT_FIELD, guid, index, fValue);
            break;
        }

        return true;
    }

    /* Syntax : .gobject setvalue #guid #index #value [uint32/uint64/float]*/
    static bool HandleGobSetValueCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char* cGUID = strtok((char*)args, " ");
        char* cIndex = strtok(nullptr, " ");
        char* cValue = strtok(nullptr, " ");
        char* cType = strtok(nullptr, " ");

        if (!cGUID || !cIndex || !cValue)
            return false;

        ObjectGuid::LowType guid = atoi(cGUID);
        if (!guid)
            return false;

        GameObject* target = nullptr;
        // by DB guid
        if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(guid))
            target = handler->GetObjectFromPlayerMapByDbGuid(guid);

        if (!target)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 index = (uint32)atoi(cIndex);
        if (index >= target->GetValuesCount())
        {
            handler->PSendSysMessage(LANG_TOO_BIG_INDEX, index, guid, target->GetValuesCount());
            return false;
        }
        uint64 uValue;
        float fValue;
        uint8 type = 0;
        if (cType)
        {
            if (strcmp(cType, "float") == 0)
                type = 2;
            else if (strcmp(cType, "uint64") == 0)
                type = 1;
            else if (strcmp(cType, "uint32") == 0)
                type = 0;
        }

        switch (type)
        {
        case 0: //uint32
            uValue = (uint32)atoi(cValue);
            target->SetUInt32Value(index, uValue);
            handler->PSendSysMessage(LANG_SET_UINT_FIELD, guid, index, uValue);
            break;
        case 1: //uint64
            uValue = (uint64)atoi(cValue);
            target->SetUInt64Value(index, uValue);
            handler->PSendSysMessage(LANG_SET_UINT_FIELD, guid, index, uValue);
            break;
        case 2: //float
            fValue = (float)atof(cValue);
            target->SetFloatValue(index, fValue);
            handler->PSendSysMessage(LANG_SET_FLOAT_FIELD, guid, index, fValue);
            break;
        }

        //update visual
        if (Map* map = sMapMgr->FindMap(target->GetMapId(), target->GetInstanceId()))
        {
            map->RemoveFromMap(target, false);
            target->SetMap(map);
            map->AddToMap(target, true);
        }

        return true;
    }
};

void AddSC_gobject_commandscript()
{
    new gobject_commandscript();
}
