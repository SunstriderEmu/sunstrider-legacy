
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

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(float fake) : 
    MovementGeneratorMedium(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    direction(WP_PATH_DIRECTION_NORMAL),
    path_type(WP_PATH_TYPE_UNSPECIFIED),
    _nextMoveTime(0),
    _recalculateTravel(false),
    _reachedFirstNode(false),
    _useSmoothSpline(false),
    erasePathAtEnd(false),
    _done(false),
    _splineId(0),
    customPath(nullptr),
    _pathId(0)
{
}

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(Movement::PointsArray& points, Optional<bool> repeating /*= {}*/, bool smoothSpline) :
    WaypointMovementGenerator(uint32(0), repeating)
{
    CreateCustomPath(points);
    erasePathAtEnd = true;
    path_type = WP_PATH_TYPE_ONCE;
    direction = WP_PATH_DIRECTION_NORMAL;
    _useSmoothSpline = smoothSpline;
}

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(uint32 _path_id, Optional<bool> repeating /*= {}*/, bool smoothSpline) :
    WaypointMovementGenerator(0.0f)
{
    _pathId = _path_id;
    _useSmoothSpline = smoothSpline;
    if (repeating)
        path_type = repeating.get() ? WP_PATH_TYPE_LOOP : WP_PATH_TYPE_ONCE;
}

WaypointMovementGenerator<Creature>::WaypointMovementGenerator(WaypointPath& path, Optional<bool> repeating /*= {}*/, bool smoothSpline) :
    WaypointMovementGenerator(0.0f)
{
    _pathId = 0;
    customPath = &path;
    _useSmoothSpline = smoothSpline;
    if(repeating)
        path_type = repeating.get() ? WP_PATH_TYPE_LOOP : WP_PATH_TYPE_ONCE;
}

MovementGeneratorType WaypointMovementGenerator<Creature>::GetMovementGeneratorType() const
{
    return WAYPOINT_MOTION_TYPE;
}

WaypointMovementGenerator<Creature>::~WaypointMovementGenerator()
{
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
    {
        _path = customPath; 
        ASSERT(path_type != WP_PATH_TYPE_UNSPECIFIED);
    }
    else
    {
        if (!_pathId)
            _pathId = creature->GetWaypointPath();

        _path = sWaypointMgr->GetPath(_pathId);
        if (!_path)
        {
            // No path id found for entry
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator::LoadPath: creature %s (Entry: %u GUID: %u DB GUID: %u) could not find path id: %u", creature->GetName().c_str(), creature->GetEntry(), creature->GetGUID().GetCounter(), creature->GetSpawnId(), _pathId);
            return false;
        }

        if(path_type == WP_PATH_TYPE_UNSPECIFIED)
            path_type = WaypointPathType(_path->pathType);
        direction = WaypointPathDirection(_path->pathDirection);
    }
    
    //some data validation, should be moved elsewhere
    {
        if (path_type >= WP_PATH_TYPE_TOTAL)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load an invalid path type : %u (path id %u). Setting it to WP_PATH_TYPE_LOOP", path_type, _pathId);
            path_type = WP_PATH_TYPE_LOOP;
        }

        if (direction >= WP_PATH_DIRECTION_TOTAL)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load an invalid path direction : %u (path id %u). Setting it to WP_PATH_DIRECTION_NORMAL", direction, _pathId);
            direction = WP_PATH_DIRECTION_NORMAL;
        }

        if (direction == WP_PATH_DIRECTION_RANDOM && path_type != WP_PATH_TYPE_LOOP)
        {
            TC_LOG_ERROR("sql.sql", "WaypointMovementGenerator tried to load a path with random direction but not with type loop, this does not make any sense so let's set type to loop");
            path_type = WP_PATH_TYPE_LOOP;
        }
    }

    _currentNode = GetFirstMemoryNode();
    _done = false;
    _nextMoveTime.Reset(1000); //movement will start after 1s

    // inform AI
    if(!_path->nodes.empty())
        if (CreatureAI* AI = creature->AI())
            AI->WaypointPathStarted(_path->nodes[_currentNode].id, _path->id);

    return true;
}

bool WaypointMovementGenerator<Creature>::DoInitialize(Creature* creature)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED | MOVEMENTGENERATOR_FLAG_TRANSITORY);

    if (!creature->IsAlive())
        return false;

    bool result = LoadPath(creature);
    if (result == false)
    {
        TC_LOG_ERROR("misc","WaypointMovementGenerator failed to init for creature %u (entry %u)", creature->GetSpawnId(), creature->GetEntry());
        return false;
    }

    _originalHome = creature->GetHomePosition();
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
void WaypointMovementGenerator<Creature>::OnArrived(Creature* creature)
{
    ASSERT(!_done);
    if (!_path || _path->nodes.size() <= _currentNode)
        return;

    if (creature->movespline->GetId() != _splineId)
        return;  //This movement could be a stop movement for pause

    WaypointNode const& arrivedNode = _path->nodes.at(_currentNode);
    
    if (arrivedNode.eventId && urand(0, 99) < arrivedNode.eventChance)
    {
        TC_LOG_DEBUG("maps.script", "Creature movement start script %u at point %u for %u", arrivedNode.eventId, arrivedNode.id, creature->GetGUID().GetCounter());
        //creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
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
        if (Transport* trans = (creature->GetTransport() ? creature->GetTransport()->ToMotionTransport() : nullptr))
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
    creature->UpdateCurrentWaypointInfo(arrivedNode.id, _path->id);
    MovementInform(creature, arrivedNode.id);

    //update _currentNode to next node
    bool hasNextPoint = GetNextMemoryNode(_currentNode, _currentNode, true);

    _done = !hasNextPoint && creature->movespline->Finalized();
    if (_done)
    {
        //waypoints ended
        creature->SetHomePosition(_originalHome);
        creature->UpdateCurrentWaypointInfo(0, 0);
        if (CreatureAI* ai = creature->AI())
            ai->WaypointPathEnded(arrivedNode.id, _path->id);
        return;
    }

    WaypointNode const& nextNode = _path->nodes.at(_currentNode);
    StartFormationMove(creature, _currentNode, nextNode.moveType);

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
    if (!_path->nodes.size()) //it may have been emptied by a script (atm SmartScript does that in SmartAI::EndPath)
        return false;

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

void WaypointMovementGenerator<Creature>::StartFormationMove(Creature* creature, uint32 node, uint32 moveType)
{
    WaypointNode const &waypoint = _path->nodes[_currentNode];
    Position dest(waypoint.x, waypoint.y, waypoint.z, (waypoint.orientation && waypoint.delay) ? waypoint.orientation : 0.0f);

    bool const transportPath = creature->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && !creature->GetTransGUID().IsEmpty();
    //! If creature is on transport, we assume waypoints set in DB are already transport offsets
    if (transportPath)
    {
        if (TransportBase* trans = creature->GetDirectTransport())
        {
            float orientation = dest.GetOrientation();
            trans->CalculatePassengerPosition(dest.m_positionX, dest.m_positionY, dest.m_positionZ, &orientation);
            dest.SetOrientation(orientation);
        }
    }    
    
    creature->SignalFormationMovement(dest, 0, moveType, dest.GetOrientation() != 0.0f);
}

bool WaypointMovementGenerator<Creature>::StartMove(Creature* creature)
{
    if (!_path || _path->nodes.empty() || HasFlag(MOVEMENTGENERATOR_FLAG_FINALIZED))
        return false;

    if (_path->nodes.size() <= _currentNode)
    {
        //should never happen
        TC_LOG_FATAL("misc","WaypointMovementGenerator::StartSplinePath - _currentNode was out of _path bounds");
        return false;
    }

    if (!creature->CanFreeMove() || !creature->IsAlive() || (creature->IsFormationLeader() && !creature->IsFormationLeaderMoveAllowed())) // if cannot move OR cannot move because of formation
    {
        _nextMoveTime.Reset(1000); // delay 1s
        return true; //do not do anything
    }

    WaypointNode const& currentNode = _path->nodes.at(_currentNode);
    //final orientation for spline movement. 0.0f mean no final orientation.
    float finalOrientation = 0.0f;
    
    _precomputedPath.clear();
    _precomputedPath.reserve(_path->nodes.size()+1);
    //MoveSplineInit::Launch() always replace the first point... so lets make a fake one for it to erase later
    _precomputedPath.push_back(G3D::Vector3(0.0f, 0.0f, 0.0f)); 
    static uint32 const firstRealPoint = 1;

    //nextNodeId is an index of _path
    uint32 nextMemoryNodeId = _currentNode;

    bool canCyclic = false;

    switch (path_type)
    {
        case WP_PATH_TYPE_LOOP:
        case WP_PATH_TYPE_ONCE:
        case WP_PATH_TYPE_ROUND_TRIP:
        {
            if (direction != WP_PATH_DIRECTION_RANDOM) //special handling for random
            {
                //insert first node
                WaypointNode const* nextNode = &(_path->nodes.at(nextMemoryNodeId));

                _precomputedPath.emplace_back(nextNode->x, nextNode->y, nextNode->z);

                //stop path if node has delay
                if (nextNode->delay)
                    break;

                //get move type of this first node
                WaypointMoveType lastMoveType = WaypointMoveType(currentNode.moveType);

                while (_useSmoothSpline && GetNextMemoryNode(nextMemoryNodeId, nextMemoryNodeId, false))
                {
                    nextNode = &(_path->nodes.at(nextMemoryNodeId));

                    //stop path at move type change (so we can handle walk/run/take off changes)
                    if (nextNode->moveType != lastMoveType)
                        break;

                    _precomputedPath.emplace_back(nextNode->x, nextNode->y, nextNode->z);

                    //stop if this is the last node in path
                    if (IsLastMemoryNode(nextMemoryNodeId))
                    {
                        // allow cyclic path only if this is a complete path (from 0 to last)
                        if(path_type == WP_PATH_TYPE_LOOP && _currentNode == 0)  
                            canCyclic = true;
                        break;
                    }

                    //stop path if node has delay
                    if (nextNode->delay)
                        break;
                }

                // if last node has orientation, set it to spline
                // ! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
                if (nextNode->orientation)
                    finalOrientation = nextNode->orientation;
            }
            else if (direction == WP_PATH_DIRECTION_RANDOM)
            { // WP_PATH_DIRECTION_RANDOM
                //random paths have no end so lets set our spline path limit at 10 nodes
                while (GetNextMemoryNode(nextMemoryNodeId, nextMemoryNodeId, true) && _precomputedPath.size() < 10)
                {
                    WaypointNode const& nextNode = _path->nodes.at(nextMemoryNodeId);

                    _precomputedPath.emplace_back(nextNode.x, nextNode.y, nextNode.z);

                    //stop path if node has delay
                    if (nextNode.delay)
                        break;
                }
            }
        } break;

        default:
            break;
    }

    if (_precomputedPath.size() <= 1)
        return false;

    Movement::MoveSplineInit init(creature);
    creature->UpdateMovementFlags(); //restore disable gravity if needed

    //Set move type. Path is ended at move type change in the filling of _precomputedPath up there so this is valid for the whole spline array. 
    //This is true but for WP_PATH_DIRECTION_RANDOM where move_type of the first point only will be used because whatever
    switch (currentNode.moveType)
    {
#ifdef LICH_KING
    case WAYPOINT_MOVE_TYPE_LAND:
        init.SetAnimation(Movement::ToGround);
        break;
    case WAYPOINT_MOVE_TYPE_TAKEOFF:
        init.SetAnimation(Movement::ToFly);
        break;
#endif
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

    // no sense making a path cyclic if there is any point with delay or with random direction
    if (path_type == WP_PATH_TYPE_LOOP && canCyclic && direction != WP_PATH_DIRECTION_RANDOM)
    {
        // Cyclic system is partially based on work here: https://github.com/TrinityCore/TrinityCore/issues/22448
        // for cyclic path, the first point gets removed after the first loop (if splineflags.enter_cycle is set)
        // A dummy point is already added previously in this function so the work is already done.
        init.SetCyclic();
    }

    if(finalOrientation)
        init.SetFacing(finalOrientation);

    init.MovebyPath(_precomputedPath, 0, creature->GetTransport());

    init.Launch();
    _splineId = creature->movespline->GetId();

    _reachedFirstNode = false;
    _recalculateTravel = false;
    creature->AddUnitState(UNIT_STATE_ROAMING_MOVE);
    RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED | MOVEMENTGENERATOR_FLAG_TIMED_PAUSED);

    //Call for creature group update
    StartFormationMove(creature, _currentNode, currentNode.moveType);

    // inform AI
    if (CreatureAI* AI = creature->AI())
    {
        WaypointNode const& dbCurrentNode = _path->nodes.at(_currentNode);
        AI->WaypointStarted(dbCurrentNode.id, _path->id);
        TC_LOG_TRACE("misc", "Creature %u started waypoint movement at node %u (path %u)", creature->GetEntry(), dbCurrentNode.id, _path->id);
    }

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

    if (!owner->CanFreeMove() || owner->HasUnitState(UNIT_STATE_LOST_CONTROL) || owner->IsMovementPreventedByCasting())
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
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
        if (owner->movespline->Finalized())
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

void WaypointMovementGenerator<Creature>::MovementInform(Creature* owner, uint32 DBNodeId)
{
    if (CreatureAI* AI = owner->AI())
    {
        AI->MovementInform(WAYPOINT_MOTION_TYPE, DBNodeId);
        if(_path)
            AI->WaypointReached(DBNodeId, _path->id);
    }
}

bool WaypointMovementGenerator<Creature>::GetResetPosition(Unit*, float& x, float& y, float& z) 
{
    // prevent a crash at empty waypoint path
    if (!_path || _path->nodes.empty())
        return false;

    ASSERT(_currentNode < _path->nodes.size(), "WaypointMovementGenerator::GetResetPosition: tried to reference a node id (%u) which is not included in path (%u)", _currentNode, _path->id);
    WaypointNode const &waypoint = _path->nodes.at(_currentNode);

    x = waypoint.x; y = waypoint.y; z = waypoint.z;
    return true;
}

bool WaypointMovementGenerator<Creature>::GetCurrentDestinationPoint(Creature* owner, Position& pos) const
{
    WaypointNode const &waypoint = _path->nodes[_currentNode];
    pos.Relocate(waypoint.x, waypoint.y, waypoint.z, (waypoint.orientation && waypoint.delay) ? waypoint.orientation : 0.0f);

    bool const transportPath = owner->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && !owner->GetTransGUID().IsEmpty();
    //! If creature is on transport, we assume waypoints set in DB are already transport offsets
    if (transportPath)
    {
        if (TransportBase* trans = owner->GetDirectTransport())
        {
            float orientation = pos.GetOrientation();
            trans->CalculatePassengerPosition(pos.m_positionX, pos.m_positionY, pos.m_positionZ, &orientation);
            pos.SetOrientation(orientation);
        }
    }
    return true;
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
