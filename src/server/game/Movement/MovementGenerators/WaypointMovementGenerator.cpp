
#include "WaypointMovementGenerator.h"
#include "MovementDefines.h"
#include "Errors.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Transport.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "CreatureGroups.h"
#include "Player.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "WaypointManager.h"

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(Movement::PointsArray& points, bool repeating, bool smoothSpline) :
    WaypointMovementGenerator(uint32(0), repeating)
{
    CreateCustomPath(points);
    path_type = WP_PATH_TYPE_ONCE;
    direction = WP_PATH_DIRECTION_NORMAL;
    erasePathAtEnd = true;
}

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(uint32 _path_id, bool repeating, bool smoothSpline) :
    MovementGeneratorMedium(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    path_id(_path_id), 
    path_type(repeating ? WP_PATH_TYPE_LOOP : WP_PATH_TYPE_ONCE),
    direction(WP_PATH_DIRECTION_NORMAL),
    _nextMoveTime(0), 
    _recalculateTravel(false),
    reachedFirstNode(false),
    customPath(nullptr), 
    erasePathAtEnd(false),
    m_useSmoothSpline(smoothSpline) ,
    _done(false)
{ }

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(WaypointPath& path, bool repeating, bool smoothSpline) :
    MovementGeneratorMedium(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    path_id(0),
    path_type(repeating ? WP_PATH_TYPE_LOOP : WP_PATH_TYPE_ONCE),
    direction(WP_PATH_DIRECTION_NORMAL),
    _nextMoveTime(0),
    _recalculateTravel(false),
    reachedFirstNode(false),
    customPath(&path),
    erasePathAtEnd(false),
    m_useSmoothSpline(smoothSpline)
{
}

MovementGeneratorType WaypointMovementGenerator<Creature>::GetMovementGeneratorType() const
{
    return WAYPOINT_MOTION_TYPE;
}

#ifdef TRINITY_DEBUG
std::atomic<bool> onArrivedProcessing(false);
#endif

WaypointMovementGenerator<Creature>::~WaypointMovementGenerator()
{
#ifdef TRINITY_DEBUG
    ASSERT(!onArrivedProcessing);
#endif

    if (erasePathAtEnd && customPath)
        delete customPath;
}

bool WaypointMovementGenerator<Creature>::CreateCustomPath(Movement::PointsArray& points)
{
    if (points.size() == 0)
        return false;

    WaypointPath* path = new WaypointPath();
    path->nodes.reserve(points.size());
    for (uint32 i = 0; i < points.size(); i++)
    {
        WaypointNode node = WaypointNode();
        node.id = i;
        node.x = points[i].x;
        node.y = points[i].y;
        node.z = points[i].z;
        //leave the other members as default
        path->nodes.push_back(std::move(node));
    }
    customPath = path;
    return true;
}

bool WaypointMovementGenerator<Creature>::LoadPath(Creature* creature)
{
    if (customPath)
        _path = customPath;
    else
    {
        if (!path_id)
            path_id = creature->GetWaypointPath();

        _path = sWaypointMgr->GetPath(path_id);
        if (!_path)
        {
            // No path id found for entry
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator::LoadPath: creature %s (Entry: %u GUID: %u DB GUID: %u) could not find path id: %u", creature->GetName().c_str(), creature->GetEntry(), creature->GetGUID().GetCounter(), creature->GetSpawnId(), path_id);
            return false;
        }

        path_type = WaypointPathType(_path->pathType);
        direction = WaypointPathDirection(_path->pathDirection);

    }
    
    //some data validation, should be moved elsewhere
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
    pathIdsToPathIndexes.reserve(_path->nodes.size());
    for (int i = 0; i < _path->nodes.size(); i++)
        pathIdsToPathIndexes[_path->nodes[i].id] = i;

    _currentNode = GetFirstMemoryNode();
    _done = false;
    _nextMoveTime.Reset(1000); //movement will start after 1s

    // inform AI
    if (creature->IsAIEnabled)
        creature->AI()->WaypointPathStarted(_path->nodes[_currentNode].id, _path->id);

    return true;
}

bool WaypointMovementGenerator<Creature>::DoInitialize(Creature* creature)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING);

    if (!creature->IsAlive())
        return false;

    bool result = LoadPath(creature);
    if (result == false)
    {
        TC_LOG_ERROR("misc","WaypointMovementGenerator failed to init for creature %u (entry %u)", creature->GetSpawnId(), creature->GetEntry());
        return false;
    }

    TC_LOG_TRACE("misc", "Creature %u  WaypointMovementGenerator<Creature>::DoInitialize", creature->GetEntry());
    return true;
}

void WaypointMovementGenerator<Creature>::DoFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        // TODO: Research if this modification is needed, which most likely isnt
        owner->SetWalk(false);
    }
}

void WaypointMovementGenerator<Creature>::DoReset(Creature* creature)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    if (!HasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED) && _nextMoveTime.Passed())
        _nextMoveTime.Reset(1); // Needed so that Update does not behave as if node was reached
}

//Must be called at each point reached. MovementInform is done in here.
void WaypointMovementGenerator<Creature>::OnArrived(Creature* creature, uint32 arrivedNodeIndex)
{
    ASSERT(!_done);
    if (!_path || _path->nodes.size() <= arrivedNodeIndex)
        return;

    WaypointNode const& arrivedNode = _path->nodes.at(arrivedNodeIndex);
    
    if (arrivedNode.eventId && urand(0, 99) < arrivedNode.eventChance)
    {
        TC_LOG_DEBUG("maps.script", "Creature movement start script %u at point %u for %u", arrivedNode.eventId, arrivedNode.id, creature->GetGUID().GetCounter());
        creature->GetMap()->ScriptsStart(sWaypointScripts, arrivedNode.eventId, creature, NULL, false);
    }

    float x = arrivedNode.x;
    float y = arrivedNode.y;
    float z = arrivedNode.z;
    float o = creature->GetOrientation();

    // Set home position
    bool transportPath = creature->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && creature->GetTransGUID();
    if (!transportPath)
        creature->SetHomePosition(x, y, z, o);
    else
    {
        if (Transport* trans = (creature->GetTransport() ? creature->GetTransport()->ToMotionTransport() : NULL))
        {
            float tx, ty, tz, to;
            creature->GetPosition(tx, ty, tz, to);
            o -= trans->GetOrientation();
            creature->SetTransportHomePosition(tx, ty, tz, to);
            trans->CalculatePassengerPosition(tx, ty, tz, &to);
            creature->SetHomePosition(tx, ty, tz, to);
        }
        else
            transportPath = false;
        // else if (vehicle) - this should never happen, vehicle offsets are const
    }

    // Inform script
    MovementInform(creature, arrivedNode.id);

    //update _currentNode to next node
    bool hasNextPoint = GetNextMemoryNode(_currentNode, _currentNode, true);
    creature->UpdateCurrentWaypointInfo(arrivedNode.id, _path->id);

    _done = !hasNextPoint && creature->movespline->Finalized();
    if (_done)
    {
        //waypoints ended
        creature->UpdateCurrentWaypointInfo(0, 0);
        if (creature->IsAIEnabled)
            creature->AI()->WaypointPathEnded(arrivedNode.id, _path->id);
        return;
    }

    if (arrivedNode.delay)
    {
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        Pause(arrivedNode.delay);
        return;
    }
}

bool WaypointMovementGenerator<Creature>::IsLastMemoryNode(uint32 node)
{
    switch(direction)
    {
    case WP_PATH_DIRECTION_NORMAL:
        return node == _path->nodes.size()-1;
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
    }
    default: //Invalid direction
        return 0;
    }
}
bool WaypointMovementGenerator<Creature>::HasNextMemoryNode(uint32 fromNode, bool allowReverseDirection)
{
    uint32 nextNode = 0;
    return GetNextMemoryNode(fromNode, nextNode, allowReverseDirection);
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
        nextNode = (fromNode + 1) % _path->nodes.size();
        break;
    case WP_PATH_DIRECTION_REVERSE:
        nextNode = fromNode == 0 ? (_path->nodes.size() - 1) : (fromNode - 1);
        break;
    case WP_PATH_DIRECTION_RANDOM:
    {
        if (_path->nodes.size() <= 1)
            return false; //prevent infinite loop

        //get a random node, a bit ugly
        uint32 lastNode = fromNode;
        nextNode = fromNode;
        while (nextNode == lastNode)
            nextNode = urand(0, _path->nodes.size() - 1);

        break;
    }
    default:
        //Should never happen. Wrong db value?
        TC_LOG_FATAL("misc", "WaypointMovementGenerator::GetNextNode() - Movement generator has non handled direction %u (pathID %u)", direction, _path->id);
        return false;
    }

    return true;
}

void WaypointMovementGenerator<Creature>::Pause(int32 time)
{ 
    TC_LOG_TRACE("misc", "Paused waypoint movement (path %u)", _path->id);
    _nextMoveTime.Reset(time); 
    _recalculateTravel = false; //no need to recalc if we stop anyway
}

bool WaypointMovementGenerator<Creature>::IsPaused()
{
    return _nextMoveTime.GetExpiry() > 0;
}

bool WaypointMovementGenerator<Creature>::UpdatePause(int32 diff)
{
    //no pause currently going on
    if (_nextMoveTime.GetExpiry() <= 0)
        return false;

    _nextMoveTime.Update(diff);
    return _nextMoveTime.Passed();
}

/* Update _currentNode from current spline progression
There may be more splines than actual path nodes so we're doing some matching in there
*/
void WaypointMovementGenerator<Creature>::SplineFinished(Creature* creature, uint32 splineId)
{
    if (!_path || _currentNode >= _path->nodes.size())
        return;

    auto splinePathItr = splineToPathIds.find(splineId);
    if (splinePathItr == splineToPathIds.end()) //spline not matched to any path id, nothing to do
    {
      //  TC_LOG_TRACE("misc", "WaypointMovementGenerator: SplineFinished %u did not match any known path id, nothing to do", splineId);
        return;
    }

   
    WaypointNode const& currentNode = _path->nodes.at(_currentNode);
    uint32 splineNodeDBId = splinePathItr->second; //id of db node we just reached

    //warn we reached a new db node if needed
    if (!reachedFirstNode || currentNode.id != splineNodeDBId)
    {
        reachedFirstNode = true;

        //update next node
        //find _path index corresponding to this path node id
        auto itr = pathIdsToPathIndexes.find(splineNodeDBId);
        if (itr != pathIdsToPathIndexes.end())
            _currentNode = itr->second;
        else
        {
            TC_LOG_FATAL("misc", "WaypointMovementGenerator::SplineFinished could not find pathNodeId %u in _path %u", splineNodeDBId, path_id);
            _currentNode = 0;
        }

        TC_LOG_TRACE("misc", "Reached node %u (path %u) (spline id %u) (path index : %u)", splineNodeDBId, path_id, splineId, _currentNode);

        //this may start a new path if we reached the end of it
        OnArrived(creature, _currentNode);
    }
}

bool WaypointMovementGenerator<Creature>::GeneratePathToNextPoint(Position const& from, Creature* creature, WaypointNode const& nextNode, uint32& splineId)
{
    //generate mmaps path to next point
    PathGenerator path(creature);
    path.SetSourcePosition(from);
    bool result = path.CalculatePath(nextNode.x, nextNode.y, nextNode.z, true);
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
    splineToPathIds[splineId] = nextNode.id;

    //TC_LOG_TRACE("misc", "[path %u] Inserted node (db %u) at (%f,%f,%f) (splineId %u)", path_id, nextNode.id, nextNode.x, nextNode.y, nextNode.z, splineId);
    return true;
}

bool WaypointMovementGenerator<Creature>::StartMove(Creature* creature)
{
    //make sure we don't trigger OnArrived from last path at this point
    _splineId = 0;

    if (!_path || _path->nodes.empty())
        return false;

    if (_path->nodes.size() <= _currentNode)
    {
        //should never happen
        TC_LOG_FATAL("misc","WaypointMovementGenerator::StartSplinePath - _currentNode was out of _path bounds");
        return false;
    }

    if (!creature->CanFreeMove() || !creature->IsAlive())
        return true; //do not do anything

    WaypointNode const& currentNode = _path->nodes.at(_currentNode);
    //final orientation for spline movement. 0.0f mean no final orientation.
    float finalOrientation = 0.0f;
    
    splineToPathIds.clear();
    m_precomputedPath.clear();
    m_precomputedPath.reserve(_path->nodes.size());
    //insert dummy first position as this will be replaced by MoveSplineInit
    m_precomputedPath.push_back(G3D::Vector3(0.0f, 0.0f, 0.0f));

    //TC_LOG_TRACE("misc", "Creating new spline path for path %u", path_id);

    //we keep track of the index of the spline we insert to match them to path id later
    uint32 splineId = 0;
    //nextNodeId is an index of _path
    uint32 nextMemoryNodeId = _currentNode;

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
                WaypointNode const* next_node = &(_path->nodes.at(nextMemoryNodeId));

                GeneratePathToNextPoint(creature, creature, *next_node, splineId);

                //stop path if node has delay
                if (next_node->delay)
                    break;
                //get move type of this first node
                WaypointMoveType lastMoveType = WaypointMoveType(currentNode.moveType);

                WaypointNode const* lastNode = next_node;

                //prepare next nodes if m_useSmoothSpline is enabled
                while (m_useSmoothSpline && GetNextMemoryNode(nextMemoryNodeId, nextMemoryNodeId, false))
                {
                    next_node = &(_path->nodes.at(nextMemoryNodeId));

                    //stop path at move type change (so we can handle walk/run/take off changes)
                    if (next_node->moveType != lastMoveType)
                        break;

                    GeneratePathToNextPoint(Position(lastNode->x, lastNode->y, lastNode->z), creature, *next_node, splineId);

                    //stop if this is the last node in path
                    if (IsLastMemoryNode(nextMemoryNodeId))
                    {
                        lastMemoryNodeId = nextMemoryNodeId;
                        break;
                    }

                    //stop path if node has delay
                    if (next_node->delay)
                        break;

                    lastNode = next_node;
                }

                //if last node has orientation, set it to spline
                //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
                if (lastNode->orientation)
                    finalOrientation = lastNode->orientation;
            }
            else if (direction == WP_PATH_DIRECTION_RANDOM)
            { // WP_PATH_DIRECTION_RANDOM
                //random paths have no end so lets set our spline path limit at 10 nodes
                uint32 count = 0;
                Position lastPosition = creature->GetPosition();
                while (GetNextMemoryNode(nextMemoryNodeId, nextMemoryNodeId, true) && count < 10)
                {
                    WaypointNode const& nxtNode = _path->nodes.at(nextMemoryNodeId);

                    GeneratePathToNextPoint(lastPosition, creature, nxtNode, splineId);
                    lastPosition = Position(nxtNode.x, nxtNode.y, nxtNode.z);
                    count++;

                    //stop path if node has delay
                    if (nxtNode.delay)
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
    switch (currentNode.moveType)
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

    /* kelno: Strange behavior with this, not sure how to use it.
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
    _recalculateTravel = false;
    creature->AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);

    //Call for creature group update
    if (creature->GetFormation() && creature->GetFormation()->getLeader() == creature)
    {
        creature->SetWalk(currentNode.moveType == WAYPOINT_MOVE_TYPE_WALK);
        creature->GetFormation()->LeaderMoveTo(m_precomputedPath[1].x, m_precomputedPath[1].y, m_precomputedPath[1].z, currentNode.moveType == WAYPOINT_MOVE_TYPE_RUN);
    }

    // inform AI
    if (creature->IsAIEnabled)
    {
        WaypointNode const& dbCurrentNode = _path->nodes.at(_currentNode);
        creature->AI()->WaypointStarted(dbCurrentNode.id, _path->id);
        TC_LOG_TRACE("misc", "Creature %u started waypoint movement at node %u (path %u)", creature->GetEntry(), dbCurrentNode.id, _path->id);
    }

    RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

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
    _recalculateTravel = true;
    return true;
}

bool WaypointMovementGenerator<Creature>::SetDirection(WaypointPathDirection dir)
{
    if(dir >= WP_PATH_DIRECTION_TOTAL)
        return false;

    direction = dir;
    //trigger creation a new spline
    _recalculateTravel = true;
    return true;
}

bool WaypointMovementGenerator<Creature>::DoUpdate(Creature* owner, uint32 diff)
{
    if (!owner || !owner->IsAlive())
        return true;

    // prevent a crash at empty waypoint path.
    if (!_path || _path->nodes.empty())
        return false;

    if (_done || HasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED | MOVEMENTGENERATOR_FLAG_PAUSED))
        return true;

    if (!owner->CanFreeMove() || owner->IsMovementPreventedByCasting())
    {
        owner->StopMoving();
        return true;
    }

    //End pause if needed. Keep this before last node checking so that we may have a pause at path end and we don't want to destroy the generator until it's done
    if (IsPaused())
    {
        if (UpdatePause(diff))
            return StartMove(owner); //restart movement on _currentNode (was updated just before pause in OnArrived)

        else //pause not finished, nothing to do
            return true;
    }
    else 
    { //!IsPaused()
        bool arrived = owner->movespline->Finalized();

        if (arrived) 
            return StartMove(owner);
        else 
        {
            // Set home position at place on waypoint movement.
            if (!owner->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) || owner->GetTransGUID())
                owner->SetHomePosition(owner->GetPosition());

            if (HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) || _recalculateTravel)
                return StartMove(owner); //relaunch spline to current dest
        }
    }

     return true;
 }

void WaypointMovementGenerator<Creature>::DoDeactivate(Creature* owner)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

void WaypointMovementGenerator<Creature>::MovementInform(Creature* creature, uint32 DBNodeId)
{
#ifdef TRINITY_DEBUG
    onArrivedProcessing = true;
#endif

    if (creature->IsAIEnabled)
    {
        creature->AI()->MovementInform(WAYPOINT_MOTION_TYPE, DBNodeId);
        if(_path)
            creature->AI()->WaypointReached(DBNodeId, _path->id);
    }
#ifdef TRINITY_DEBUG
    onArrivedProcessing = false;
#endif
}

bool WaypointMovementGenerator<Creature>::GetResetPosition(Unit*, float& x, float& y, float& z) 
{
    // prevent a crash at empty waypoint path
    if (!_path || _path->nodes.empty())
        return false;

    ASSERT(_currentNode < _path->nodes.size(), "WaypointMovementGenerator::GetResetPosition: tried to reference a node id (%u) which is not included in path (%u)", _currentNode, _path->id);
    WaypointNode const &waypoint = _path->nodes.at(_currentNode);

    WaypointNode const& node = _path->nodes.at(_currentNode);
    x = node.x; y = node.y; z = node.z;
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

void WaypointMovementGenerator<Creature>::Pause(uint32 timer/* = 0*/)
{
    if (timer)
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
        _nextMoveTime.Reset(timer);
        RemoveFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
    }
    else
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
        _nextMoveTime.Reset(1); // Needed so that Update does not behave as if node was reached
        RemoveFlag(MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);
    }
}

void WaypointMovementGenerator<Creature>::Resume(uint32 overrideTimer/* = 0*/)
{
    if (overrideTimer)
        _nextMoveTime.Reset(overrideTimer);

    if (_nextMoveTime.Passed())
        _nextMoveTime.Reset(1); // Needed so that Update does not behave as if node was reached

    RemoveFlag(MOVEMENTGENERATOR_FLAG_PAUSED);
}

//----------------------------------------------------//

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
