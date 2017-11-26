
#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "WaypointManager.h"
#include "MapManager.h"
#include "Log.h"
#include "WaypointDefines.h"

WaypointMgr::WaypointMgr() { }

WaypointMgr::~WaypointMgr()
{
}

void WaypointMgr::Load()
{
    uint32 oldMSTime = GetMSTime();

    //                                                0    1         2           3          4            5           6        7      8           9
    QueryResult result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM waypoint_data wd ORDER BY wd.id, wd.point");

    if (!result)
    {
        TC_LOG_ERROR("server.loading", ">> Loaded 0 waypoints. DB table `waypoint_data` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 pathId = fields[0].GetUInt32();
        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        WaypointNode waypoint;
        waypoint.id = fields[1].GetUInt32();
        waypoint.x = x;
        waypoint.y = y;
        waypoint.z = z;
        waypoint.orientation = o;
        waypoint.moveType = fields[6].GetUInt32();

        if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
        {
            TC_LOG_ERROR("sql.sql", "Waypoint %u in waypoint_data has invalid move_type, ignoring", waypoint.id);
            continue;
        }

        waypoint.delay = fields[7].GetUInt32();
        waypoint.eventId = fields[8].GetUInt32();
        waypoint.eventChance = fields[9].GetInt8();

        WaypointPath& path = _waypointStore[pathId];
        path.id = pathId;
        path.nodes.push_back(std::move(waypoint));
        ++count;
    } while (result->NextRow());

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_INFO);
    if(PreparedQueryResult result = WorldDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 pathId = fields[0].GetUInt32();
            auto itr = _waypointStore.find(pathId);
            if (itr == _waypointStore.end())
            {
                TC_LOG_ERROR("sql.sql", "Found waypoint_info data for empty path %u", pathId);
                continue;
            }
            WaypointPath& path = _waypointStore[pathId];
            path.pathType = fields[1].GetUInt16();
            path.pathDirection = fields[2].GetUInt8();

        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u waypoints in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WaypointMgr::ReloadPath(uint32 id)
{
    auto itr = _waypointStore.find(id);
    if (itr != _waypointStore.end())
        _waypointStore.erase(itr);

    QueryResult result = WorldDatabase.PQuery("SELECT point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM waypoint_data WHERE id = %u ORDER BY point",id);

    if (!result)
        return;

    std::vector<WaypointNode> values;
    do
    {
        Field* fields = result->Fetch();

        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        float o = fields[4].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        WaypointNode waypoint;
        waypoint.id = fields[0].GetUInt32();
        waypoint.x = x;
        waypoint.y = y;
        waypoint.z = z;
        waypoint.orientation = o;
        waypoint.moveType = fields[5].GetUInt32();

        if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
        {
            TC_LOG_ERROR("sql.sql", "Waypoint %u in waypoint_data has invalid move_type, ignoring", waypoint.id);
            continue;
        }

        waypoint.delay = fields[6].GetUInt32();
        waypoint.eventId = fields[7].GetUInt32();
        waypoint.eventChance = fields[8].GetUInt8();
    }
    while (result->NextRow());

    WaypointPath path(id, std::move(values));

    //sunstrider: additional path info
    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_INFO_BY_ID);
    stmt->setUInt32(0, id);
    if(PreparedQueryResult result = WorldDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();

        path.pathType = fields[0].GetUInt16();
        path.pathDirection = fields[1].GetUInt8();
    }

    _waypointStore[id] = std::move(path);
}
