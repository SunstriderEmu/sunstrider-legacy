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

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(uint32 _path_id) : 
    path_id(_path_id), 
    path_type(WP_PATH_TYPE_LOOP),
    direction(WP_PATH_DIRECTION_NORMAL),
    i_nextMoveTime(0), 
    m_isArrivalDone(false)
{ 
}

void WaypointMovementGenerator<Creature>::LoadPath(Creature* creature)
{
    if (!path_id)
        path_id = creature->GetWaypointPathId();

    WaypointPath const* path = sWaypointMgr->GetPath(path_id);
    i_path = &(path->nodes);//i_path contain WaypointPathNodes, not WaypointPath*

    if (!i_path)
    {
        // No path id found for entry
        TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator::LoadPath: creature %s (Entry: %u GUID: %u DB GUID: %u) doesn't have waypoint path id: %u", creature->GetName().c_str(), creature->GetEntry(), creature->GetGUIDLow(), creature->GetDBTableGUIDLow(), path_id);
        return;
    }
    
    path_type = WaypointPathType(path->pathType);
    direction = WaypointPathDirection(path->pathDirection);

    if(path_type >= WP_PATH_TYPE_TOTAL)
    {
        TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load an invalid path type : %u (path id %u). Setting it to WP_PATH_TYPE_LOOP", path_type, path_id);
        path_type = WP_PATH_TYPE_LOOP;
    }

    if(direction >= WP_PATH_DIRECTION_TOTAL)
    {
        TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load an invalid path direction : %u (path id %u). Setting it to WP_PATH_DIRECTION_NORMAL", direction, path_id);
        direction = WP_PATH_DIRECTION_NORMAL;
    }

    //direction specific checks
    switch(direction)
    {
    //if direction is reversed, start at last point
    case WP_PATH_DIRECTION_REVERSE:
        i_currentNode = i_path->size()-1;
        break;
    case WP_PATH_DIRECTION_RANDOM:
        //randomize first node
        SetNextNode(); 
        //also check for type, no other types than loop make any sense with this direction
        if(path_type != WP_PATH_TYPE_LOOP)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load a path with random direction but not with type loop, this does not make any sense so let's set type to loop");
            path_type = WP_PATH_TYPE_LOOP;
        }
        break;
    default:
        break;
    }

    StartMoveNow(creature);
}

void WaypointMovementGenerator<Creature>::DoInitialize(Creature* creature)
{
    LoadPath(creature);
    creature->AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
}

void WaypointMovementGenerator<Creature>::DoFinalize(Creature* creature)
{
    creature->ClearUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    creature->SetWalk(false);
}

void WaypointMovementGenerator<Creature>::DoReset(Creature* creature)
{
    creature->AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    StartMoveNow(creature);
}

//Must be called at each point reached
void WaypointMovementGenerator<Creature>::OnArrived(Creature* creature)
{
    if (!i_path || i_path->empty())
        return;
    if (m_isArrivalDone)
        return;

    m_isArrivalDone = true;

    if (i_path->at(i_currentNode)->event_id && urand(0, 99) < i_path->at(i_currentNode)->event_chance)
    {
        TC_LOG_DEBUG("maps.script", "Creature movement start script %u at point %u for " UI64FMTD ".", i_path->at(i_currentNode)->event_id, i_currentNode, creature->GetGUID());
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        sWorld->ScriptsStart(sWaypointScripts, i_path->at(i_currentNode)->event_id, creature, NULL, false);
    }

    // Inform script
    MovementInform(creature);
    creature->UpdateWaypointID(i_currentNode);

    if (i_path->at(i_currentNode)->delay)
    {
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        Stop(i_path->at(i_currentNode)->delay);
    }
}

bool WaypointMovementGenerator<Creature>::IsLastNode(uint32 node)
{
    switch(direction)
    {
    case WP_PATH_DIRECTION_NORMAL:
        return i_currentNode == i_path->size()-1;
    case WP_PATH_DIRECTION_REVERSE:
        return i_currentNode == 0;
    case WP_PATH_DIRECTION_RANDOM:
        return false;
    default:
        TC_LOG_ERROR("misc","WaypointMovementGenerator::IsLastPoint() - Movement generator has non handled direction %u", direction);
        return false;
    }
}

bool WaypointMovementGenerator<Creature>::SetNextNode()
{
    if(path_type == WP_PATH_TYPE_ONCE && IsLastNode(i_currentNode))
        return false;

    switch(direction)
    {
    case WP_PATH_DIRECTION_NORMAL:
        i_currentNode = (i_currentNode+1) % i_path->size();
        break;
    case WP_PATH_DIRECTION_REVERSE:
        i_currentNode = i_currentNode == 0 ? (i_path->size() - 1) : (i_currentNode-1);
        break;
    case WP_PATH_DIRECTION_RANDOM:
    {
        if(i_path->size() <= 1)
            return false; //prevent infinite loop

        //a bit ugly
        uint32 lastNode = i_currentNode;
        while(i_currentNode == lastNode)
            i_currentNode = urand(0, i_path->size()-1);

        break;
    }
    default:
        TC_LOG_ERROR("misc","WaypointMovementGenerator::SetNextNode() - Movement generator has non handled direction %u", direction);
        break;
    }

    return true;
}

bool WaypointMovementGenerator<Creature>::StartMove(Creature* creature)
{
    if (!i_path || i_path->empty())
        return false;

    if(!creature->CanFreeMove())
        return true;

    if (Stopped())
        return true;

    creature->UpdateMovementFlags(); //restore disable gravity if needed
    bool transportPath = creature->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && creature->GetTransGUID();

    if (m_isArrivalDone)
    {
        if (IsLastNode(i_currentNode))
        {
            if (path_type == WP_PATH_TYPE_ONCE) 
            { //end waypoint movement
                
                //set new home at current position
                float x = i_path->at(i_currentNode)->x;
                float y = i_path->at(i_currentNode)->y;
                float z = i_path->at(i_currentNode)->z;
                float o = creature->GetOrientation();

                if (!transportPath)
                    creature->SetHomePosition(x, y, z, o);
                else
                {
                    if (Transport* trans = creature->GetTransport())
                    {
                        o -= trans->GetOrientation();
                        creature->SetTransportHomePosition(x, y, z, o);
                        trans->CalculatePassengerPosition(x, y, z, &o);
                        creature->SetHomePosition(x, y, z, o);
                    }
                    else
                        transportPath = false;
                }

                creature->GetMotionMaster()->Initialize();
                return false; //cause movement expire
            } else if (path_type == WP_PATH_TYPE_ROUND_TRIP) 
            {
                //reverse direction
                direction = direction == WP_PATH_DIRECTION_NORMAL ? WP_PATH_DIRECTION_REVERSE : WP_PATH_DIRECTION_NORMAL;
            }
        }

        //else 
        SetNextNode();
    }
    
    /*
    //we need next node to compute nice and smooth splines. Deactivated for now, see the other commented block below
    uint32 i_nextNode = (i_currentNode+1) % i_path->size();
    WaypointData const* currentNode = i_path->at(i_currentNode);
    WaypointData const* nextNode = i_path->at(i_nextNode);
    */
    WaypointData const* currentNode = i_path->at(i_currentNode);

    m_isArrivalDone = false;

    creature->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Movement::Location formationDest(currentNode->x, currentNode->y, currentNode->z, 0.0f);
    Movement::MoveSplineInit init(creature);

    //! If creature is on transport, we assume waypoints set in DB are already transport offsets
    if (transportPath)
    {
        init.DisableTransportPathTransformations();
        if (TransportBase* trans = creature->GetTransport())
            trans->CalculatePassengerPosition(formationDest.x, formationDest.y, formationDest.z, &formationDest.orientation);
    }

    //! Do not use formationDest here, MoveTo requires transport offsets due to DisableTransportPathTransformations() call
    //! but formationDest contains global coordinates
    init.MoveTo(currentNode->x, currentNode->y, currentNode->z);

    /* Doesn't work properly for now. We want to be able to send multiple points to make nice catmull stuff blablah. The following code isn't visually nice when 
    interrupted + it doesn't warn scripts when waypoint is reached
    //convert path format (clean me later if working, maybe change structure of i_path in class)
    Movement::PointsArray controls;
    controls.reserve(i_path->size());
    for(auto itr : *i_path)
        controls.push_back(G3D::Vector3(itr->x,itr->y,itr->z));

    init.MovebyPath(controls,i_currentNode);
        
        //init.MoveTo(currentNode->x, currentNode->y, currentNode->z);
    if(path_type == WP_PATH_TYPE_LOOP)
        init.SetCyclic();
    
    init.MovebyPath(controls,i_currentNode); */
    
    //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
   if (currentNode->orientation && currentNode->delay)
        init.SetFacing(currentNode->orientation);
        
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
    }

    init.Launch();

    //Call for creature group update
    if (creature->GetFormation() && creature->GetFormation()->getLeader() == creature)
    {
        creature->SetWalk(!currentNode->move_type);
        creature->GetFormation()->LeaderMoveTo(formationDest.x, formationDest.y, formationDest.z);
    }

    return true;
}

bool WaypointMovementGenerator<Creature>::SetPathType(WaypointPathType type)
{
    if(type >= WP_PATH_TYPE_TOTAL)
        return false;

    path_type = type;
    return true;
}

bool WaypointMovementGenerator<Creature>::SetDirection(WaypointPathDirection dir)
{
    if(dir >= WP_PATH_DIRECTION_TOTAL)
        return false;

    direction = dir;
    return true;
}

bool WaypointMovementGenerator<Creature>::DoUpdate(Creature* creature, uint32 diff)
{
    // Waypoint movement can be switched on/off
    // This is quite handy for escort quests and other stuff
    if (creature->HasUnitState(UNIT_STATE_NOT_MOVE))
    {
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }
    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
        return false;

    if (Stopped())
    {
        if (CanMove(diff))
            return StartMove(creature);
    }
    else
    {
        // Set home position at place on waypoint movement.
        if (!creature->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) || !creature->GetTransGUID())
            creature->SetHomePosition(creature->GetPosition());

        if (creature->IsStopped())
            Stop(STOP_TIME_FOR_PLAYER);
        else if (creature->movespline->Finalized())
        {
            OnArrived(creature);
            return StartMove(creature);
        }
    }
     return true;
 }

void WaypointMovementGenerator<Creature>::MovementInform(Creature* creature)
{
    if (creature->AI())
        creature->AI()->MovementInform(WAYPOINT_MOTION_TYPE, i_currentNode);
}

bool WaypointMovementGenerator<Creature>::GetResetPos(Creature*, float& x, float& y, float& z)
{
    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
        return false;

    const WaypointData* node = i_path->at(i_currentNode);
    x = node->x; y = node->y; z = node->z;
    return true;
}

//----------------------------------------------------//

uint32 FlightPathMovementGenerator::GetPathAtMapEnd() const
{
    if (i_currentNode >= i_path->size())
        return i_path->size();

    uint32 curMapId = (*i_path)[i_currentNode].MapID;
    for (uint32 i = i_currentNode; i < i_path->size(); ++i)
    {
        if ((*i_path)[i].MapID != curMapId)
            return i;
    }

    return i_path->size();
}

void FlightPathMovementGenerator::DoInitialize(Player* player)
{
    Reset(player);
    InitEndGridInfo();
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
        G3D::Vector3 vertice((*i_path)[i].x, (*i_path)[i].y, (*i_path)[i].z);
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
            DoEventIfAny(player, (*i_path)[i_currentNode], departureEvent);
            if (pointId == i_currentNode)
                break;
            if (i_currentNode == _preloadTargetNode)
                PreloadEndGrid();
            i_currentNode += (uint32)departureEvent;
            departureEvent = !departureEvent;
        }
        while (true);
    }

    return i_currentNode < (i_path->size()-1);
}

void FlightPathMovementGenerator::SetCurrentNodeAfterTeleport()
{
    if (i_path->empty())
        return;

    uint32 map0 = (*i_path)[0].MapID;
    for (size_t i = 1; i < i_path->size(); ++i)
    {
        if ((*i_path)[i].MapID != map0)
        {
            i_currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::DoEventIfAny(Player* player, TaxiPathNodeEntry const& node, bool departure)
{
    if (uint32 eventid = departure ? node.departureEventID : node.arrivalEventID)
    {
        TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node.NodeIndex, node.PathID, player->GetName().c_str());
       // TODOMOV player->GetMap()->ScriptsStart(sEventScripts, eventid, player, player);
    }
}

bool FlightPathMovementGenerator::GetResetPos(Player*, float& x, float& y, float& z)
{
    const TaxiPathNodeEntry& node = (*i_path)[i_currentNode];
    x = node.x; y = node.y; z = node.z;
    return true;
}

void FlightPathMovementGenerator::InitEndGridInfo()
{
    /*! Storage to preload flightmaster grid at end of flight. For multi-stop flights, this will
       be reinitialized for each flightmaster at the end of each spline (or stop) in the flight. */
    uint32 nodeCount = (*i_path).size();        //! Number of nodes in path.
    _endMapId = (*i_path)[nodeCount - 1].MapID; //! MapId of last node
    _preloadTargetNode = nodeCount - 3;
    _endGridX = (*i_path)[nodeCount - 1].x;
    _endGridY = (*i_path)[nodeCount - 1].y;
}

void FlightPathMovementGenerator::PreloadEndGrid()
{
    // used to preload the final grid where the flightmaster is
    Map* endMap = sMapMgr->FindBaseNonInstanceMap(_endMapId);

    // Load the grid
    if (endMap)
    {
        TC_LOG_INFO("misc", "Preloading rid (%f, %f) for map %u at node index %u/%u", _endGridX, _endGridY, _endMapId, _preloadTargetNode, (uint32)(i_path->size()-1));
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