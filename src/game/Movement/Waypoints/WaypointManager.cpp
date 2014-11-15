/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DatabaseEnv.h"
#include "GridDefines.h"
#include "WaypointManager.h"
#include "MapManager.h"
#include "Log.h"

WaypointMgr::WaypointMgr() { }

WaypointMgr::~WaypointMgr()
{
    ClearStore();
}

void WaypointMgr::ClearStore()
{
    for (WaypointPathContainer::iterator itr = _waypointStore.begin(); itr != _waypointStore.end(); ++itr)
    {
        auto nodes = itr->second.nodes;
        while(!nodes.empty())
        {
            delete nodes.back();
            nodes.pop_back();
        }

        itr->second.nodes.clear();
    }

    _waypointStore.clear();
}

void WaypointMgr::Load()
{
    uint32 oldMSTime = getMSTime();

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
        WaypointData* wp = new WaypointData();

        uint32 pathId = fields[0].GetUInt32();
        WaypointPath& path = _waypointStore[pathId];

        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[1].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->move_type = fields[6].GetUInt32();
        wp->delay = fields[7].GetUInt32();
        wp->event_id = fields[8].GetUInt32();
        wp->event_chance = fields[9].GetInt16();

        path.nodes.push_back(wp);
        ++count;
    } while (result->NextRow());

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_INFO);
    if(PreparedQueryResult result = WorldDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 pathId = fields[0].GetUInt32();
            WaypointPath& path = _waypointStore[pathId];

            path.pathType = fields[1].GetUInt16();
            path.pathDirection = fields[2].GetUInt8();

        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u waypoints in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void WaypointMgr::ReloadPath(uint32 id)
{
    if(_waypointStore.find(id) != _waypointStore.end())
    {
        //Don't remove the vector, there may be pointers to it elsewhere
        _waypointStore[id].nodes.clear(); 
        _waypointStore[id].pathType = 0;
        _waypointStore[id].pathDirection = 0;
    }
    
    QueryResult result = WorldDatabase.PQuery("SELECT point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM waypoint_data WHERE id = %u ORDER BY point",id);

    if (!result)
        return;

    WaypointPath& path = _waypointStore[id];

    do
    {
        Field* fields = result->Fetch();
        WaypointData* wp = new WaypointData();

        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        float o = fields[4].GetFloat();

        Trinity::NormalizeMapCoord(x);
        Trinity::NormalizeMapCoord(y);

        wp->id = fields[0].GetUInt32();
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->orientation = o;
        wp->move_type = fields[5].GetUInt32();
        wp->delay = fields[6].GetUInt32();
        wp->event_id = fields[7].GetUInt32();
        wp->event_chance = fields[8].GetUInt8();

        path.nodes.push_back(wp);
    }
    while (result->NextRow());

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_WAYPOINT_INFO_BY_ID);
    stmt->setUInt32(0, id);
    if(PreparedQueryResult result = WorldDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();

        path.pathType = fields[0].GetUInt16();
        path.pathDirection = fields[1].GetUInt8();
    }
}
