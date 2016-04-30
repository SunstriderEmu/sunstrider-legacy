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
//Basic headers
#include "WaypointMovementGenerator.h"
//Extended headers
#include "ObjectMgr.h"
#include "World.h"
#include "Transport.h"
//Flightmaster grid preloading
#include "MapManager.h"
//Creature-specific headers
#include "Creature.h"
#include "CreatureAI.h"
#include "CreatureGroups.h"
//Player-specific
#include "Player.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"

#include "WaypointManager.h"

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(Movement::PointsArray& points) :
    WaypointMovementGenerator(uint32(0))
{
    CreateCustomPath(points);
    path_type = WP_PATH_TYPE_ONCE;
    direction = WP_PATH_DIRECTION_NORMAL;
}

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(uint32 _path_id) : 
    path_id(_path_id), 
    path_type(WP_PATH_TYPE_LOOP),
    direction(WP_PATH_DIRECTION_NORMAL),
    i_nextMoveTime(0), 
    i_recalculatePath(false),
    reachedFirstNode(false),
    customPath(nullptr)
{ 

}

WaypointMovementGenerator<Creature>::~WaypointMovementGenerator()
{
    if (customPath)
    {
        customPath->clear(); //clear WaypointData if any
        delete customPath;
    }
}

bool WaypointMovementGenerator<Creature>::CreateCustomPath(Movement::PointsArray& points)
{
    if (points.size() == 0)
        return false;

    WaypointPath* path = new WaypointPath();
    path->reserve(points.size());
    for (uint32 i = 0; i < points.size(); i++)
    {
        WaypointData* node = new WaypointData();
        node->id = i;
        node->x = points[i].x;
        node->y = points[i].y;
        node->z = points[i].z;
        //leave the other members as default
        path->push_back(node);
    }
    customPath = path;
    return true;
}

bool WaypointMovementGenerator<Creature>::LoadPath(Creature* creature)
{
    if (customPath)
        i_path = customPath;
    else
    {
        if (!path_id)
            path_id = creature->GetWaypointPathId();

        i_path = sWaypointMgr->GetPath(path_id);

        if (!i_path)
        {
            // No path id found for entry
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator::LoadPath: creature %s (Entry: %u GUID: %u DB GUID: %u) could not find path id: %u", creature->GetName().c_str(), creature->GetEntry(), creature->GetGUIDLow(), creature->GetDBTableGUIDLow(), path_id);
            return false;
        }

        path_type = WaypointPathType(i_path->pathType);
        direction = WaypointPathDirection(i_path->pathDirection);
    }
    
    //some data validation
    {
        if (path_type >= WP_PATH_TYPE_TOTAL)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load an invalid path type : %u (path id %u). Setting it to WP_PATH_TYPE_LOOP", path_type, path_id);
            path_type = WP_PATH_TYPE_LOOP;
        }

        if (direction >= WP_PATH_DIRECTION_TOTAL)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load an invalid path direction : %u (path id %u). Setting it to WP_PATH_DIRECTION_NORMAL", direction, path_id);
            direction = WP_PATH_DIRECTION_NORMAL;
        }

        if (direction == WP_PATH_DIRECTION_RANDOM && path_type != WP_PATH_TYPE_LOOP)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load a path with random direction but not with type loop, this does not make any sense so let's set type to loop");
            path_type = WP_PATH_TYPE_LOOP;
        }
    }

    //prepare pathIdsToPathIndexes
    pathIdsToPathIndexes.reserve(i_path->size());
    for (int i = 0; i < i_path->size(); i++)
        pathIdsToPathIndexes[(*i_path)[i]->id] = i;

    i_currentNode = GetFirstMemoryNode();

    return StartMoveNow(creature);
}

bool WaypointMovementGenerator<Creature>::DoInitialize(Creature* creature)
{
    if (!creature->IsAlive())
        return false;

    bool result = LoadPath(creature);
    if (result == false)
    {
        TC_LOG_ERROR("misc","WaypointMovementGenerator failed to init for creature %u (entry %u)", creature->GetDBTableGUIDLow(), creature->GetEntry());
        return false;
    }

    return true;
}

void WaypointMovementGenerator<Creature>::DoFinalize(Creature* creature)
{
    creature->ClearUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    creature->SetWalk(false);
}

void WaypointMovementGenerator<Creature>::DoReset(Creature* creature)
{
    bool result = StartMoveNow(creature);
    if (!result)
        DoInitialize(creature); //this may help with some cases where the generator keep resetting because there is no next points
}

//Must be called at each point reached. MovementInform is done in here.
void WaypointMovementGenerator<Creature>::OnArrived(Creature* creature, uint32 arrivedNodeIndex)
{
    if (!i_path || i_path->size() <= arrivedNodeIndex)
        return;

    WaypointData* arrivedNode = (*i_path)[arrivedNodeIndex];
    
    if (arrivedNode->event_id && urand(0, 99) < arrivedNode->event_chance)
    {
        TC_LOG_DEBUG("maps.script", "Creature movement start script %u at point %u for %u", arrivedNode->event_id, arrivedNode->id, creature->GetGUIDLow());
        sWorld->ScriptsStart(sWaypointScripts, arrivedNode->event_id, creature, NULL, false);
    }

    float x = arrivedNode->x;
    float y = arrivedNode->y;
    float z = arrivedNode->z;
    float o = creature->GetOrientation();

    // Set home position
    bool transportPath = creature->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && creature->GetTransGUID();
    if (!transportPath)
        creature->SetHomePosition(x, y, z, o);
    else
    {
        if (Transport* trans = (creature->GetTransport() ? creature->GetTransport()->ToMotionTransport() : NULL))
        {
            float x, y, z, o;
            creature->GetPosition(x, y, z, o);
            o -= trans->GetOrientation();
            creature->SetTransportHomePosition(x, y, z, o);
            trans->CalculatePassengerPosition(x, y, z, &o);
            creature->SetHomePosition(x, y, z, o);
        }
        else
            transportPath = false;
        // else if (vehicle) - this should never happen, vehicle offsets are const
    }

    // Inform script
    MovementInform(creature, arrivedNode->id);
    creature->UpdateWaypointID(arrivedNode->id);

    if (arrivedNode->delay)
    {
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        Pause(arrivedNode->delay);
        return;
    }

    //continue movement if needed/possible
    if (creature->movespline->Finalized())
    {
        if(GetNextMemoryNode(arrivedNodeIndex, i_currentNode, true))
            StartSplinePath(creature);
    }
}

bool WaypointMovementGenerator<Creature>::IsLastMemoryNode(uint32 node)
{
    switch(direction)
    {
    case WP_PATH_DIRECTION_NORMAL:
        return node == i_path->size()-1;
    case WP_PATH_DIRECTION_REVERSE:
        return node == 0;
    case WP_PATH_DIRECTION_RANDOM:
        return false;
    default:
        TC_LOG_ERROR("misc","WaypointMovementGenerator::IsLastPoint() - Movement generator has non handled direction %u", direction);
        return true;
    }
}

uint32 WaypointMovementGenerator<Creature>::GetFirstMemoryNode()
{
    switch (direction)
    {
    case WP_PATH_DIRECTION_NORMAL:
        return 0;
    case WP_PATH_DIRECTION_REVERSE:
    case WP_PATH_DIRECTION_RANDOM:
    {
        uint32 node = 0;
        //let GetNextMemoryNode handle it
        GetNextMemoryNode(node, node, true);
        return node;
        break;
    }
    default:
        return 0;
    }
}

bool WaypointMovementGenerator<Creature>::GetNextMemoryNode(uint32 fromNode, uint32& nextNode, bool allowReverseDirection)
{
    if (IsLastMemoryNode(fromNode))
    {
        if (path_type == WP_PATH_TYPE_ONCE)
            return false;
        if(allowReverseDirection && path_type == WP_PATH_TYPE_ROUND_TRIP)
            direction = direction == WP_PATH_DIRECTION_NORMAL ? WP_PATH_DIRECTION_REVERSE : WP_PATH_DIRECTION_NORMAL;
    }

    switch (direction)
    {
    case WP_PATH_DIRECTION_NORMAL:
        nextNode = (fromNode + 1) % i_path->size();
        break;
    case WP_PATH_DIRECTION_REVERSE:
        nextNode = fromNode == 0 ? (i_path->size() - 1) : (fromNode - 1);
        break;
    case WP_PATH_DIRECTION_RANDOM:
    {
        if (i_path->size() <= 1)
            return false; //prevent infinite loop

        //get a random node, a bit ugly
        uint32 lastNode = fromNode;
        nextNode = fromNode;
        while (nextNode == lastNode)
            nextNode = urand(0, i_path->size() - 1);

        break;
    }
    default:
        //should never happen
        TC_LOG_FATAL("misc", "WaypointMovementGenerator::GetNextNode() - Movement generator has non handled direction %u", direction);
        return false;
    }

    return true;
}

void WaypointMovementGenerator<Creature>::Pause(int32 time)
{ 
    i_nextMoveTime.Reset(time); 
    i_recalculatePath = false; //no need to recalc if we stop anyway
}

bool WaypointMovementGenerator<Creature>::IsPaused()
{
    return i_nextMoveTime.GetExpiry() > 0;
}

bool WaypointMovementGenerator<Creature>::UpdatePause(int32 diff)
{
    //no pause currently going on
    if (i_nextMoveTime.GetExpiry() <= 0)
        return false;

    i_nextMoveTime.Update(diff);
    return i_nextMoveTime.Passed();
}

/* Update i_currentNode from current spline progression
There may be more splines than actual path nodes so we're doing some matching in there
*/
void WaypointMovementGenerator<Creature>::SplineFinished(Creature* creature, uint32 splineId)
{
    if (!i_path || i_path->size() <= i_currentNode)
        return;

    auto itr = splineToPathIds.find(splineId);
    if (itr == splineToPathIds.end()) //spline not matched to any path id, nothing to do
    {
      //  TC_LOG_TRACE("misc", "WaypointMovementGenerator: SplineFinished %u did not match any known path id, nothing to do", splineId);
        return;
    }

    WaypointData* currentNode = (*i_path)[i_currentNode];
    uint32 pathNodeDBId = itr->second; //id of db node we just reached

    //warn we reached a new db node if needed
    if (!reachedFirstNode || currentNode->id != pathNodeDBId)
    {
        reachedFirstNode = true;

        //update next node
        //find i_path index corresponding to this path node id
        auto itr = pathIdsToPathIndexes.find(pathNodeDBId);
        if (itr != pathIdsToPathIndexes.end())
            i_currentNode = itr->second;
        else
        {
            TC_LOG_FATAL("misc", "WaypointMovementGenerator::SplineFinished could not find pathNodeId %u in i_path %u", pathNodeDBId, path_id);
            i_currentNode = 0;
        }

        TC_LOG_TRACE("misc", "Reached node %u (path %u) (spline id %u) (path index : %u)", pathNodeDBId, path_id, splineId, i_currentNode);

        //this may start a new path if we reached the end of it
        OnArrived(creature, i_currentNode);
    }
}

bool WaypointMovementGenerator<Creature>::GeneratePathToNextPoint(Position const& from, Creature* creature, WaypointData* nextNode, uint32& splineId)
{
    //generate mmaps path to next point
    PathGenerator path(creature);
    path.SetSourcePosition(from);
    bool result = path.CalculatePath(nextNode->x, nextNode->y, nextNode->z, true);
    if (!result || (path.GetPathType() & PATHFIND_NOPATH))
        return false; //should never happen

    auto points = path.GetPath();
    ASSERT(!points.empty());
    //Calculate path first point is current position, skip it
    uint32 skip = 1;
    m_precomputedPath.reserve(m_precomputedPath.size() + points.size() - skip);
    for (uint32 i = 0 + skip; i < points.size(); i++)
    {
        m_precomputedPath.emplace_back(points[i].x, points[i].y, points[i].z);
        splineId++;
    }

    //register id of the last point for movement inform
    splineToPathIds[splineId] = nextNode->id;

    TC_LOG_TRACE("misc", "[path %u] Inserted node (db %u) at (%f,%f,%f) (splineId %u)", path_id, nextNode->id, nextNode->x, nextNode->y, nextNode->z, splineId);

    return true;
}

bool WaypointMovementGenerator<Creature>::StartSplinePath(Creature* creature, bool nextNode /*= false*/)
{
    //make sure we don't trigger OnArrived from last path at this point
    _splineId = 0;

    if (!i_path || i_path->empty())
        return false;

    if (!creature->CanFreeMove() || !creature->IsAlive())
        return false;

    if (nextNode)
    {
        bool result = GetNextMemoryNode(i_currentNode, i_currentNode, true);
        if (!result)
            return false;
    }

    if (i_path->size() <= i_currentNode)
    {
        //should never happen
        TC_LOG_FATAL("misc","WaypointMovementGenerator::StartSplinePath - i_currentNode was out of i_path bounds");
        return false;
    }

    WaypointData* currentNode = (*i_path)[i_currentNode];
    //final orientation for spline movement. 0.0f mean no final orientation.
    float finalOrientation = 0.0f;
    
    m_precomputedPath.clear();
    m_precomputedPath.reserve(i_path->size());
    //insert dummy first position as this will be replaced by MoveSplineInit
    m_precomputedPath.push_back(G3D::Vector3(0.0f, 0.0f, 0.0f));

    TC_LOG_TRACE("misc", "Creating new spline path for path %u", path_id);

    //we keep track of the index of the spline we insert to match them to path id later
    uint32 splineId = 0;
    //nextNodeId is an index of i_path
    uint32 nextMemoryNodeId = i_currentNode;

    switch (path_type)
    {
        case WP_PATH_TYPE_LOOP:
        case WP_PATH_TYPE_ONCE:
        case WP_PATH_TYPE_ROUND_TRIP:
        {
            if (direction != WP_PATH_DIRECTION_RANDOM) //special handling for random
            {
                uint32 lastMemoryNodeId = nextMemoryNodeId;
                //insert first node
                WaypointData* nextNode = (*i_path)[nextMemoryNodeId];

                GeneratePathToNextPoint(creature, creature, nextNode, splineId);

                //stop path if node has delay
                if (nextNode->delay)
                    break;
                //get move type of this first node
                WaypointMoveType lastMoveType = WaypointMoveType(currentNode->move_type);

                WaypointData* lastNode = nextNode;

                //next nodes
                while (GetNextMemoryNode(nextMemoryNodeId, nextMemoryNodeId, false))
                {
                    nextNode = (*i_path)[nextMemoryNodeId];

                    //stop path at move type change (so we can handle walk/run/take off changes
                    if (nextNode->move_type != lastMoveType)
                        break;

                    GeneratePathToNextPoint(Position(lastNode->x, lastNode->y, lastNode->z), creature, nextNode, splineId);

                    //stop if this is the last node in path
                    if (IsLastMemoryNode(nextMemoryNodeId))
                    {
                        lastMemoryNodeId = nextMemoryNodeId;
                        break;
                    }

                    //stop path if node has delay
                    if (nextNode->delay)
                        break;

                    lastNode = nextNode;
                }

                //if last node has orientation, set it to spline
                //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
                if (lastNode->orientation)
                    finalOrientation = lastNode->orientation;
            }
            else
            { // WP_PATH_DIRECTION_RANDOM
                //random paths have no end so lets set our spline path limit at 10 nodes
                uint32 count = 0;
                Position lastPosition = creature->GetPosition();
                while (GetNextMemoryNode(nextMemoryNodeId, nextMemoryNodeId, true) && count < 10)
                {
                    WaypointData* nextNode = (*i_path)[nextMemoryNodeId];

                    GeneratePathToNextPoint(lastPosition, creature, nextNode, splineId);
                    lastPosition = Position(nextNode->x, nextNode->y, nextNode->z);
                    count++;

                    //stop path if node has delay
                    if (nextNode->delay)
                        break;
                }
            }
        } break;

        default:
            break;
    }

    if (m_precomputedPath.size() <= 1)
        return false;

    Movement::MoveSplineInit init(creature);
    creature->UpdateMovementFlags(); //restore disable gravity if needed

    //Set move type. Path is ended at move type change in the filling of m_precomputedPath up there so this is valid for the whole spline array. 
    //This is true but for WP_PATH_DIRECTION_RANDOM where move_type of the first point only will be used because whatever
    switch (currentNode->move_type)
    {
#ifdef LICH_KING
    case WAYPOINT_MOVE_TYPE_LAND:
        init.SetAnimation(Movement::ToGround);
        break;
    case WAYPOINT_MOVE_TYPE_TAKEOFF:
        init.SetAnimation(Movement::ToFly);
#endif
        break;
    case WAYPOINT_MOVE_TYPE_RUN:
        init.SetWalk(false);
        break;
    case WAYPOINT_MOVE_TYPE_WALK:
        init.SetWalk(true);
        break;
    case WAYPOINT_MOVE_TYPE_USE_UNIT_MOVEMENT_FLAG:
        init.SetWalk(creature->HasUnitMovementFlag(MOVEMENTFLAG_WALKING));
        break;
    }

    /* Strange behavior with this, not sure how to use it.
    Not much time right now but if you want to try if you want to enable it : From what I see, MoveSplineInit inserts a first point into the spline that fucks this up, this needs to be changed first (else the creature position at this time will be included in the loop)
    if (path_type == WP_PATH_TYPE_LOOP)
        init.SetCyclic();
    */

    if(finalOrientation)
        init.SetFacing(finalOrientation);

    init.MovebyPath(m_precomputedPath);

    init.Launch();
    _splineId = creature->movespline->GetId(); //used by SplineHandler class to do movement inform's
    reachedFirstNode = false;
    i_recalculatePath = false;
    creature->AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);

    //Call for creature group update
    if (creature->GetFormation() && creature->GetFormation()->getLeader() == creature)
    {
        creature->SetWalk(currentNode->move_type == WAYPOINT_MOVE_TYPE_WALK);
        creature->GetFormation()->LeaderMoveTo(m_precomputedPath[1].x, m_precomputedPath[1].y, m_precomputedPath[1].z, currentNode->move_type == WAYPOINT_MOVE_TYPE_RUN);
    }

    /* Not handled for now
    bool transportPath = creature->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && creature->GetTransGUID();
    //! If creature is on transport, we assume waypoints set in DB are already transport offsets
    
    if (transportPath)
    {
        init.DisableTransportPathTransformations();
        if (TransportBase* trans = creature->GetTransport())
            trans->CalculatePassengerPosition(formationDest.x, formationDest.y, formationDest.z, &formationDest.orientation);
    }
    */
    return true;
}

bool WaypointMovementGenerator<Creature>::SetPathType(WaypointPathType type)
{
    if(type >= WP_PATH_TYPE_TOTAL)
        return false;

    path_type = type;
    //trigger creation of a new spline
    i_recalculatePath = true;
    return true;
}

bool WaypointMovementGenerator<Creature>::SetDirection(WaypointPathDirection dir)
{
    if(dir >= WP_PATH_DIRECTION_TOTAL)
        return false;

    direction = dir;
    //trigger creation a new spline
    i_recalculatePath = true;
    return true;
}

bool WaypointMovementGenerator<Creature>::DoUpdate(Creature* creature, uint32 diff)
{
    // Waypoint movement can be switched on/off
    // This is quite handy for escort quests and other stuff */
    if (!creature->CanFreeMove())
    {
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE); //will be reset at next move
        return true;
    }

    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
        return false;

    //End pause if needed. Keep this before last node checking so that we may have a pause at path end and we don't want to destroy the generator until it's done
    if (IsPaused())
    {
        if (UpdatePause(diff))
        {
            if(GetNextMemoryNode(i_currentNode, i_currentNode, true))
                return StartSplinePath(creature, false);
            else
            {
                //OForce reset of this generator. 
                //This may be needed because the movement genreator may not get reset upon expire
                //(if it's the last creature movement generator) and we may end up in a loop
                //where the creature keeps restarting movement towards the last point
                DoInitialize(creature);
                return false;
            }
        }
        else //pause not finisshed, nothing to do
            return true;
    }
    else 
    { //!IsPaused()
        bool arrived = creature->movespline->Finalized();

        if (i_recalculatePath)
            return StartSplinePath(creature);

        if (arrived)
            bool result = StartSplinePath(creature, true);
    }

     return true;
 }

void WaypointMovementGenerator<Creature>::MovementInform(Creature* creature, uint32 DBNodeId)
{
    if (creature->AI())
        creature->AI()->MovementInform(WAYPOINT_MOTION_TYPE, DBNodeId);
}

bool WaypointMovementGenerator<Creature>::GetResetPos(Creature*, float& x, float& y, float& z) const
{
    // prevent a crash at empty waypoint path
    if (!i_path || i_path->empty())
        return false;

    const WaypointData* node = (*i_path)[i_currentNode];
    x = node->x; y = node->y; z = node->z;
    return true;
}

bool WaypointMovementGenerator<Creature>::GetCurrentDestinationPoint(Creature* creature, Position& pos) const
{
    uint32 splineIdx = creature->movespline->_currentSplineIdx();
    if (m_precomputedPath.size() > splineIdx)
    {
        pos.Relocate(m_precomputedPath[splineIdx].x, m_precomputedPath[splineIdx].y, m_precomputedPath[splineIdx].z);
        return true;
    }
    return false;
}

//----------------------------------------------------//

uint32 FlightPathMovementGenerator::GetPathAtMapEnd() const
{
    if (i_currentNode >= i_path.size())
        return i_path.size();

    uint32 curMapId = i_path[i_currentNode]->MapID;
    for (uint32 i = i_currentNode; i < i_path.size(); ++i)
        if (i_path[i]->MapID != curMapId)
            return i;

    return i_path.size();
}

#define SKIP_SPLINE_POINT_DISTANCE_SQ (40.0f * 40.0f)

bool IsNodeIncludedInShortenedPath(TaxiPathNodeEntry const* p1, TaxiPathNodeEntry const* p2)
{
    return p1->MapID != p2->MapID || std::pow(p1->LocX - p2->LocX, 2) + std::pow(p1->LocY - p2->LocY, 2) > SKIP_SPLINE_POINT_DISTANCE_SQ;
}

void FlightPathMovementGenerator::LoadPath(Player* player)
{
    _pointsForPathSwitch.clear();
    std::deque<uint32> const& taxi = player->m_taxi.GetPath();
    for (uint32 src = 0, dst = 1; dst < taxi.size(); src = dst++)
    {
        uint32 path, cost;
        sObjectMgr->GetTaxiPath(taxi[src], taxi[dst], path, cost);
        if (path > sTaxiPathNodesByPath.size())
            return;

        TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[path];
        if (!nodes.empty())
        {
            TaxiPathNodeEntry const* start = nodes[0];
            TaxiPathNodeEntry const* end = nodes[nodes.size() - 1];
            bool passedPreviousSegmentProximityCheck = false;
            for (uint32 i = 0; i < nodes.size(); ++i)
            {
                if (passedPreviousSegmentProximityCheck || !src || i_path.empty() || IsNodeIncludedInShortenedPath(i_path[i_path.size() - 1], nodes[i]))
                {
                    if ((!src || (IsNodeIncludedInShortenedPath(start, nodes[i]) && i >= 2)) &&
                        (dst == taxi.size() - 1 || (IsNodeIncludedInShortenedPath(end, nodes[i]) && i < nodes.size() - 1)))
                    {
                        passedPreviousSegmentProximityCheck = true;
                        i_path.push_back(nodes[i]);
                    }
                }
                else
                {
                    i_path.pop_back();
                    --_pointsForPathSwitch.back().PathIndex;
                }
            }
        }

        _pointsForPathSwitch.push_back({ uint32(i_path.size() - 1), int32(cost) });
    }
}

bool FlightPathMovementGenerator::DoInitialize(Player* player)
{
    Reset(player);
    InitEndGridInfo();
    return true;
}

void FlightPathMovementGenerator::DoFinalize(Player* player)
{
    // remove flag to prevent send object build movement packets for flight state and crash (movement generator already not at top of stack)
    player->ClearUnitState(UNIT_STATE_IN_FLIGHT);

    player->Dismount();
    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_TAXI_FLIGHT);

    if (player->m_taxi.empty())
    {
        player->GetHostileRefManager().setOnlineOfflineState(true);
        // update z position to ground and orientation for landing point
        // this prevent cheating with landing  point at lags
        // when client side flight end early in comparison server side
        player->StopMoving();
    }

    player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_TAXI_BENCHMARK);
}

#define PLAYER_FLIGHT_SPEED 32.0f

void FlightPathMovementGenerator::DoReset(Player* player)
{
    player->GetHostileRefManager().setOnlineOfflineState(false);
    player->AddUnitState(UNIT_STATE_IN_FLIGHT);
    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_TAXI_FLIGHT);

    Movement::MoveSplineInit init(player);
    uint32 end = GetPathAtMapEnd();
    for (uint32 i = GetCurrentNode(); i != end; ++i)
    {
        G3D::Vector3 vertice(i_path[i]->LocX, i_path[i]->LocY, i_path[i]->LocZ);
        init.Path().push_back(vertice);
    }
    init.SetFirstPointId(GetCurrentNode());
    init.SetFly();
    init.SetVelocity(PLAYER_FLIGHT_SPEED);
    init.Launch();
}

bool FlightPathMovementGenerator::DoUpdate(Player* player, uint32 /*diff*/)
{
    uint32 pointId = (uint32)player->movespline->currentPathIdx();
    if (pointId > i_currentNode)
    {
        bool departureEvent = true;
        do
        {
            DoEventIfAny(player, i_path[i_currentNode], departureEvent);
            while (!_pointsForPathSwitch.empty() && _pointsForPathSwitch.front().PathIndex <= i_currentNode)
            {
                _pointsForPathSwitch.pop_front();
                player->m_taxi.NextTaxiDestination();
                if (!_pointsForPathSwitch.empty())
                {
#ifdef LICH_KING
                    player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING, _pointsForPathSwitch.front().Cost);
#endif
                    player->ModifyMoney(-_pointsForPathSwitch.front().Cost);
                }
            }

            if (pointId == i_currentNode)
                break;

            if (i_currentNode == _preloadTargetNode)
                PreloadEndGrid();
            i_currentNode += (uint32)departureEvent;
            departureEvent = !departureEvent;
        } while (true);
    }

    return i_currentNode < (i_path.size() - 1);
}

void FlightPathMovementGenerator::SetCurrentNodeAfterTeleport()
{
    if (i_currentNode >= i_path.size())
        return;

    uint32 map0 = i_path[i_currentNode]->MapID;
    for (size_t i = i_currentNode + 1; i < i_path.size(); ++i)
    {
        if (i_path[i]->MapID != map0)
        {
            i_currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::DoEventIfAny(Player* player, TaxiPathNodeEntry const* node, bool departure)
{
    if (uint32 eventid = departure ? node->departureEventID : node->arrivalEventID)
    {
        // TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node->NodeIndex, node->PathID, player->GetName().c_str());
        //TC player->GetMap()->ScriptsStart(sEventScripts, eventid, player, player);
        sWorld->ScriptsStart(sEventScripts, eventid, player, player, true);
    }
}

bool FlightPathMovementGenerator::GetResetPos(Player*, float& x, float& y, float& z) const
{
    TaxiPathNodeEntry const* node = i_path[i_currentNode];
    x = node->LocX;
    y = node->LocY;
    z = node->LocZ;
    return true;
}

void FlightPathMovementGenerator::InitEndGridInfo()
{
    /*! Storage to preload flightmaster grid at end of flight. For multi-stop flights, this will
    be reinitialized for each flightmaster at the end of each spline (or stop) in the flight. */
    uint32 nodeCount = i_path.size();        //! Number of nodes in path.
    if (nodeCount == 0)
    {
        TC_LOG_ERROR("misc", "FlightPathMovementGenerator::InitEndGridInfo(): FATAL: Flight path has 0 nodes!");
        ABORT();
        return;
    }

    _endMapId = i_path[nodeCount - 1]->MapID; //! MapId of last node
    _preloadTargetNode = nodeCount - 3;
    _endGridX = i_path[nodeCount - 1]->LocX;
    _endGridY = i_path[nodeCount - 1]->LocY;
}

void FlightPathMovementGenerator::PreloadEndGrid()
{
    // used to preload the final grid where the flightmaster is
    Map* endMap = sMapMgr->FindBaseNonInstanceMap(_endMapId);

    // Load the grid
    if (endMap)
    {
        TC_LOG_DEBUG("misc", "Preloading rid (%f, %f) for map %u at node index %u/%u", _endGridX, _endGridY, _endMapId, _preloadTargetNode, (uint32)(i_path.size() - 1));
        endMap->LoadGrid(_endGridX, _endGridY);
    }
    else
        TC_LOG_INFO("misc", "Unable to determine map to preload flightmaster grid");
}

std::string GetWaypointPathDirectionName(WaypointPathDirection dir)
{
    std::string pathDirStr;
    switch(dir)
    {
    case WP_PATH_DIRECTION_NORMAL: pathDirStr = "WP_PATH_DIRECTION_NORMAL"; break;
    case WP_PATH_DIRECTION_REVERSE: pathDirStr = "WP_PATH_DIRECTION_REVERSE"; break;
    case WP_PATH_DIRECTION_RANDOM: pathDirStr = "WP_PATH_DIRECTION_RANDOM"; break;
    default: pathDirStr = "Error"; break;
    }
    return pathDirStr;
}

std::string GetWaypointPathTypeName(WaypointPathType type)
{
    std::string pathTypeStr;
    switch(type)
    {
    case WP_PATH_TYPE_LOOP: pathTypeStr = "WP_PATH_TYPE_LOOP"; break;
    case WP_PATH_TYPE_ONCE: pathTypeStr = "WP_PATH_TYPE_ONCE"; break;
    case WP_PATH_TYPE_ROUND_TRIP: pathTypeStr = "WP_PATH_TYPE_ROUND_TRIP"; break;
    default: pathTypeStr = "Error"; break;
    }
    return pathTypeStr;
}