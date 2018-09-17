#include "Chat.h"
#include "Language.h"
#include "Transport.h"
#include "MapManager.h"

using namespace Trinity::ChatCommands;
class go_commandscript : public CommandScript
{
public:
    go_commandscript() : CommandScript("go_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> goCreatureCommandTable =
        {
            { "id",             SEC_GAMEMASTER2,     false, &HandleGoCreatureCIdCommand,       "" },
            { "",               SEC_GAMEMASTER2,     false, &HandleGoCreatureSpawnIdCommand,   "" }
        };

        static std::vector<ChatCommand> goCommandTable =
        {
            { "grid",           SEC_GAMEMASTER1,     false, &HandleGoGridCommand,              "" },
            { "creature",       SEC_GAMEMASTER2,     false, nullptr,                           "", goCreatureCommandTable },
            { "object",         SEC_GAMEMASTER2,     false, &HandleGoObjectCommand,            "" },
            { "ticket",         SEC_GAMEMASTER1,     false, &HandleGoTicketCommand,            "" },
            { "trigger",        SEC_GAMEMASTER2,     false, &HandleGoTriggerCommand,           "" },
            { "graveyard",      SEC_GAMEMASTER2,     false, &HandleGoGraveyardCommand,         "" },
            { "zonexy",         SEC_GAMEMASTER1,     false, &HandleGoZoneXYCommand,            "" },
            { "xy",             SEC_GAMEMASTER1,     false, &HandleGoXYCommand,                "" },
            { "xyz",            SEC_GAMEMASTER1,     false, &HandleGoXYZCommand,               "" },
            { "xyzo",           SEC_GAMEMASTER1,     false, &HandleGoXYZOCommand,              "" },
            { "at",             SEC_GAMEMASTER3,     false, &HandleGoATCommand,                "" },
            { "",               SEC_GAMEMASTER1,     false, &HandleGoXYZCommand,               "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "go",             SEC_GAMEMASTER1,     false, nullptr,                           "", goCommandTable },
        };
        return commandTable;
    }

    static bool HandleGoTicketCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            char *cstrticket_id = strtok((char*)args, " ");

        if (!cstrticket_id)
            return false;

        uint64 ticket_id = atoi(cstrticket_id);
        if (!ticket_id)
            return false;

        GM_Ticket *ticket = sObjectMgr->GetGMTicket(ticket_id);
        if (!ticket)
        {
            handler->SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
            return true;
        }

        float x, y, z;
        int mapid;

        x = ticket->pos_x;
        y = ticket->pos_y;
        z = ticket->pos_z;
        mapid = ticket->map;

        Player* _player = handler->GetSession()->GetPlayer();
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(mapid, x, y, z, 1, 0);
        return true;
    }

    static bool HandleGoObjectCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

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
                handler->SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
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
                handler->SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        if (!MapManager::IsValidMapCoord(mapid, x, y, z, ort))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(mapid, x, y, z, ort);
        return true;
    }


    //teleport to grid
    static bool HandleGoGridCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

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
        float x = (grid_x - CENTER_GRID_ID + 0.5f)*SIZE_OF_GRIDS;
        float y = (grid_y - CENTER_GRID_ID + 0.5f)*SIZE_OF_GRIDS;

        if (!MapManager::IsValidMapCoord(mapid, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        Map const *map = sMapMgr->CreateBaseMap(mapid);
        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
        _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

        return true;
    }

    //teleport at coordinates
    static bool HandleGoXYCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

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

        if (!MapManager::IsValidMapCoord(mapid, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        Map const *map = sMapMgr->CreateBaseMap(mapid);
        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

        _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

        return true;
    }

    //teleport at coordinates, including Z
    static bool HandleGoXYZCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

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

        if (!MapManager::IsValidMapCoord(mapid, x, y, z))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

        return true;
    }

    static bool HandleGoXYZOCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

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
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(mapid, x, y, z, o);

        return true;
    }

    //teleport at coordinates
    static bool HandleGoZoneXYCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

        char* px = strtok((char*)args, " ");
        char* py = strtok(nullptr, " ");
        char* tail = strtok(nullptr, "");

        char* cAreaId = handler->extractKeyFromLink(tail, "Harea");       // string or [name] Shift-click form |color|Harea:area_id|h[name]|h|r

        if (!px || !py)
            return false;

        float x = (float)atof(px);
        float y = (float)atof(py);
        uint32 areaid = cAreaId ? (uint32)atoi(cAreaId) : _player->GetAreaId();

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaid);

        if (x < 0 || x>100 || y < 0 || y>100 || !areaEntry)
        {
            handler->PSendSysMessage(LANG_INVALID_ZONE_COORD, x, y, areaid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // update to parent zone if exist (client map show only zones without parents)
        areaEntry = areaEntry->zone ? sAreaTableStore.LookupEntry(areaEntry->zone) : areaEntry;

        Map const *map = sMapMgr->CreateBaseMap(areaEntry->mapid);

        if (map->Instanceable())
        {
            handler->PSendSysMessage(LANG_INVALID_ZONE_MAP, areaEntry->ID, areaEntry->area_name[handler->GetSessionDbcLocale()], map->GetId(), map->GetMapName());
            handler->SetSentErrorMessage(true);
            return false;
        }

        Zone2MapCoordinates(x, y, areaEntry->zone);

        if (!MapManager::IsValidMapCoord(areaEntry->mapid, x, y))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, areaEntry->mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
        _player->TeleportTo(areaEntry->mapid, x, y, z, _player->GetOrientation());

        return true;
    }

    static bool HandleGoTriggerCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* _player = handler->GetSession()->GetPlayer();

        char *atId = strtok((char*)args, " ");
        if (!atId)
            return false;

        int32 i_atId = atoi(atId);

        if (!i_atId)
            return false;

        AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(i_atId);
        if (!at)
        {
            handler->PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND, i_atId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!MapManager::IsValidMapCoord(at->mapid, at->x, at->y, at->z))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, at->x, at->y, at->mapid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(at->mapid, at->x, at->y, at->z, _player->GetOrientation());
        return true;
    }

    static bool HandleGoGraveyardCommand(ChatHandler* handler, char const* args)
    {
        Player* _player = handler->GetSession()->GetPlayer();

        if (!*args)
            return false;

        char *gyId = strtok((char*)args, " ");
        if (!gyId)
            return false;

        int32 i_gyId = atoi(gyId);

        if (!i_gyId)
            return false;

        WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(i_gyId);
        if (!gy)
        {
            handler->PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, i_gyId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!MapManager::IsValidMapCoord(gy->map_id, gy->x, gy->y, gy->z))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, gy->x, gy->y, gy->map_id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (_player->IsInFlight())
            _player->FinishTaxiFlight();
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        _player->TeleportTo(gy->map_id, gy->x, gy->y, gy->z, _player->GetOrientation());
        return true;
    }

    static bool DoTeleport(ChatHandler* handler, WorldLocation loc)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!MapManager::IsValidMapCoord(loc) || sObjectMgr->IsTransportMap(loc.GetMapId()))
        {
            handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (player->IsInFlight())
            player->FinishTaxiFlight();
        else
            player->SaveRecallPosition(); // save only in non-flight case

        player->TeleportTo(loc);
        return true;
    }

    static bool HandleGoCreatureSpawnIdCommand(ChatHandler* handler, Variant<Hyperlink<creature>, ObjectGuid::LowType> spawnId)
    {
        CreatureData const* spawnpoint = sObjectMgr->GetCreatureData(spawnId);
        if (!spawnpoint)
        {
            handler->SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return DoTeleport(handler, spawnpoint->spawnPoint);
    }

    static bool HandleGoCreatureCIdCommand(ChatHandler* handler, Variant<Hyperlink<creature_entry>, uint32> cId)
    {
        CreatureData const* spawnpoint = nullptr;
        for (auto const& pair : sObjectMgr->GetAllCreatureData())
        {
            if (!std::any_of(pair.second.ids.begin(), pair.second.ids.end(), [&](auto e) { return e.id = *cId; }))
                continue;

            if (!spawnpoint)
                spawnpoint = &pair.second;
            else
            {
                handler->SendSysMessage(LANG_COMMAND_GOCREATMULTIPLE);
                break;
            }
        }

        if (!spawnpoint)
        {
            handler->SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return DoTeleport(handler, spawnpoint->spawnPoint);
    }

    static bool HandleGoATCommand(ChatHandler* handler, char const* args)
    {
        ARGS_CHECK

            Player* plr = handler->GetSession()->GetPlayer();
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
};

void AddSC_go_commandscript()
{
    new go_commandscript();
}
