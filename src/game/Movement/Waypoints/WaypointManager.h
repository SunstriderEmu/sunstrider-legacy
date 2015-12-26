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

#ifndef TRINITY_WAYPOINTMANAGER_H
#define TRINITY_WAYPOINTMANAGER_H

#include <vector>
#include <unordered_map>

enum WaypointMoveType
{
    WAYPOINT_MOVE_TYPE_WALK     = 0,
    WAYPOINT_MOVE_TYPE_RUN      = 1,
    WAYPOINT_MOVE_TYPE_LAND     = 2,
    WAYPOINT_MOVE_TYPE_TAKEOFF  = 3,

    WAYPOINT_MOVE_TYPE_MAX
};

struct WaypointData
{
    uint32 id;
    float x, y, z, orientation;
    uint32 delay;
    uint32 event_id;
    uint32 move_type;
    uint8 event_chance;
};

typedef std::vector<WaypointData*> WaypointPathNodes;

struct WaypointPath : public std::vector<WaypointData*>
{
    WaypointPath() :
        pathType(0),
        pathDirection(0)
    {}

    uint16 pathType;
    uint8 pathDirection;
};

typedef std::unordered_map<uint32, WaypointPath> WaypointPathContainer;

class WaypointMgr
{
    public:
        static WaypointMgr* instance()
        {
            static WaypointMgr instance;
            return &instance;
        }

        // Attempts to reload a single path from database
        void ReloadPath(uint32 id);

        // Loads all paths from database, do not reload since the pointers in _waypointStore could still be in use
        void Load();

        // Returns the path from a given id
        WaypointPath const* GetPath(uint32 id) const
        {
            WaypointPathContainer::const_iterator itr = _waypointStore.find(id);
            if (itr != _waypointStore.end())
                return &itr->second;

            return nullptr;
        }

    private:
        // Only allow instantiation from ACE_Singleton
        WaypointMgr();
        ~WaypointMgr();

        WaypointPathContainer _waypointStore;
};

#define sWaypointMgr WaypointMgr::instance()

#endif
