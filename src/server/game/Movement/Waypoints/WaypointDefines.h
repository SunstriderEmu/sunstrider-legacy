#ifndef TRINITY_WAYPOINTDEFINES_H
#define TRINITY_WAYPOINTDEFINES_H

#include "Define.h"
#include <vector>

enum WaypointMoveType : uint32
{
    WAYPOINT_MOVE_TYPE_WALK = 0,
    WAYPOINT_MOVE_TYPE_RUN = 1,
    WAYPOINT_MOVE_TYPE_LAND = 2,
    WAYPOINT_MOVE_TYPE_TAKEOFF = 3,

    //check for MOVEMENTFLAG_WALKING
    WAYPOINT_MOVE_TYPE_USE_UNIT_MOVEMENT_FLAG = 100,

    WAYPOINT_MOVE_TYPE_MAX
};

struct WaypointNode
{
    WaypointNode() : id(0), x(0.f), y(0.f), z(0.f), orientation(0.f), delay(0), eventId(0), moveType(WAYPOINT_MOVE_TYPE_RUN), eventChance(0) { }
    WaypointNode(uint32 _id, float _x, float _y, float _z, float _orientation = 0.f, uint32 _delay = 0)
    {
        id = _id;
        x = _x;
        y = _y;
        z = _z;
        orientation = _orientation;
        delay = _delay;
        eventId = 0;
        moveType = WAYPOINT_MOVE_TYPE_WALK;
        eventChance = 100;
    }


    uint32 id;
    float x;
    float y;
    float z;
    float orientation = 0.0f;
    uint32 delay = 0;
    uint32 eventId = 0;
    uint32 moveType = WAYPOINT_MOVE_TYPE_USE_UNIT_MOVEMENT_FLAG;
    uint8 eventChance = 0;
};

typedef std::vector<WaypointNode*> WaypointPathNodes;

struct WaypointPath
{
    WaypointPath() :
        id(0),
        pathType(0),
        pathDirection(0)
    {}
    WaypointPath(uint32 _id, std::vector<WaypointNode>&& _nodes) :
        pathType(0),
        pathDirection(0)
    {
        id = _id;
        nodes = _nodes;
    }

    uint16 pathType;
    uint8 pathDirection;
    std::vector<WaypointNode> nodes;
    uint32 id;
};

#endif
