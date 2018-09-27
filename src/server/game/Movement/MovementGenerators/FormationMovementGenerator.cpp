// rewritten for sunstrider
// - Main difference is that instead of going directly to dest, we also go to a start point with the correct offset. 
//   This ensures a trajectory parallel to the owner one
// 
// Todo: 
// - Launch a single movement with different speeds for position to start then start to dest.
//   This is possible but currently not handled by our movement system.
//   This would allow for even more precise positions.

#include "FormationMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MovementDefines.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

FormationMovementGenerator::FormationMovementGenerator(uint32 id, ObjectGuid leaderGUID, FormationMoveSegment moveSegment) :
    MovementGeneratorMedium(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    _movementId(id), 
    _leaderGUID(leaderGUID),
    _moveSegment(moveSegment),
    _movingToStart(false)
{
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
}

MovementGeneratorType FormationMovementGenerator::GetMovementGeneratorType() const
{
    return FORMATION_MOTION_TYPE;
}

Position FormationMovementGenerator::GetMemberDestination(Creature* member, uint32 followDist, Position leaderDest, uint8 depth) const
{
    //no valid position found after 3 iterations, return leader position instead
    if (depth > 3)
        return leaderDest;

    float pathAngle = _moveSegment.start.GetAbsoluteAngle(_moveSegment.dest);

    Position dest;
    dest.m_orientation = leaderDest.GetOrientation();
    dest.m_positionX = leaderDest.GetPositionX() + cos(_moveSegment.followAngle + pathAngle) * followDist;
    dest.m_positionY = leaderDest.GetPositionY() + sin(_moveSegment.followAngle + pathAngle) * followDist;
    float height;
    if (!member->IsFlying())
        height = member->GetMap()->GetHeight(dest.m_positionX, dest.m_positionY, leaderDest.GetPositionZ() + 5.0f, true, 10.0f, member->GetCollisionHeight(), true);
    else
        height = leaderDest.GetPositionZ();

    if (height != INVALID_HEIGHT)
        dest.m_positionZ = height;
    else //no valid height found, try closer
        return GetMemberDestination(member, followDist / 2.0f, leaderDest, ++depth);

    Trinity::NormalizeMapCoord(dest.m_positionX);
    Trinity::NormalizeMapCoord(dest.m_positionY);

    /* No ground pos update since we already do GetHeight call before
    if (!member->CanFly() && !member->IsFlying())
    member->UpdateGroundPositionZ(dest.m_positionX, dest.m_positionY, dest.m_positionZ);*/

    return dest;
}

float FormationMovementGenerator::GetDistanceFromLine(Position point, Position start, Position end)
{
    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::linestring<point_type> linestring_type;

    point_type p(point.GetPositionX(), point.GetPositionY());
    linestring_type line;
    line.push_back(point_type(start.GetPositionX(), start.GetPositionY()));
    line.push_back(point_type(end.GetPositionX(), end.GetPositionY()));

    return boost::geometry::distance(p, line);
}

void FormationMovementGenerator::MoveToStart(Creature* owner)
{
    _movingToStart = true;

    Position start = GetMemberDestination(owner, _moveSegment.followDist, _moveSegment.start);
    Position dest = GetMemberDestination(owner, _moveSegment.followDist, _moveSegment.dest);

    // Decide if we should skip the start point, skip if we're:
    if (   start.GetExactDist(owner) < 5.0f                                  // - Close from the start point
        && owner->GetExactDist2d(dest) < start.GetExactDist2d(dest)          // - Closer to start than from end
        && GetDistanceFromLine(owner->GetPosition(), start, dest) < 3.0f)    // - Close to the line between start and end
    {
        return; // This will trigger start to dest at next update
    }
       
    Movement::MoveSplineInit init(owner);
    init.MoveTo(start.GetPositionX(), start.GetPositionY(), start.GetPositionZ(), false, true);  //waypoint generator is not using mmaps... so neither should be
    if (start.GetOrientation())
        init.SetFacing(start.GetOrientation());

    bool walk = true;
    switch (_moveSegment.moveType)
    {
#ifdef LICH_KING
    case 2: // WAYPOINT_MOVE_TYPE_LAND
        init.SetAnimation(Movement::ToGround);
        break;
    case 3: // WAYPOINT_MOVE_TYPE_TAKEOFF
        init.SetAnimation(Movement::ToFly);
        break;
#endif
    case WAYPOINT_MOVE_TYPE_RUN:
        walk = false;
        break;
    case WAYPOINT_MOVE_TYPE_WALK:
        walk = true;
        break;
}
    init.SetWalk(walk);

    // Speed calc - Max 1 sec to get there
    UnitMoveType mtype = walk ? MOVE_WALK : MOVE_RUN;
    float const baseVelocity = owner->GetSpeed(mtype);
    float const distance = owner->GetExactDist(start);
    float const velocityFor2s = distance / 2.0f; //velocity to move this distance in 2 sec
    init.SetVelocity(std::max(baseVelocity, velocityFor2s));

    init.Launch();
    owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED | MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING);
}

void FormationMovementGenerator::MoveToDest(Creature* owner)
{
    _movingToStart = false;

    Position dest = GetMemberDestination(owner, _moveSegment.followDist, _moveSegment.dest);

    Movement::MoveSplineInit init(owner);
    init.MoveTo(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false, true); //waypoint generator is not using mmaps... so neither should be
    if (dest.GetOrientation())
        init.SetFacing(dest.GetOrientation());

    bool walk = true;
    switch (_moveSegment.moveType)
    {
#ifdef LICH_KING
    case 2: // WAYPOINT_MOVE_TYPE_LAND
        init.SetAnimation(Movement::ToGround);
        break;
    case 3: // WAYPOINT_MOVE_TYPE_TAKEOFF
        init.SetAnimation(Movement::ToFly);
        break;
#endif
    case WAYPOINT_MOVE_TYPE_RUN:
        walk = false;
        break;
    case WAYPOINT_MOVE_TYPE_WALK:
        walk = true;
        break;
    }
    init.SetWalk(walk);

    if (Creature* leader = owner->GetMap()->GetCreature(_leaderGUID))
    {
        // change members speed basing on distance
        // -> What we aim here is arriving at the same time
        UnitMoveType mtype = walk ? MOVE_WALK : MOVE_RUN;
        float leaderSegmentLength = _moveSegment.start.GetExactDist(_moveSegment.dest);
        float memberSegmentLength = owner->GetExactDist(dest);
        float rate = memberSegmentLength / leaderSegmentLength;
        init.SetVelocity(leader->GetSpeed(mtype) * rate);
    }

    init.Launch();
    owner->AddUnitState(UNIT_STATE_ROAMING_MOVE);
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED | MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING);

    owner->SetHomePosition(POSITION_GET_X_Y_Z(&dest), 0.0f);
}

bool FormationMovementGenerator::DoInitialize(Creature* owner)
{
    _previousHome = owner->GetHomePosition();
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        return true;
    }

    MoveToStart(owner);
    return true;
}

void FormationMovementGenerator::DoReset(Creature* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    DoInitialize(owner);
}

bool FormationMovementGenerator::DoUpdate(Creature* owner, uint32 /*diff*/)
{
    if (!owner)
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        return true;
    }

    if (   (_movingToStart && owner->movespline->Finalized())
        || (HasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED) && owner->movespline->Finalized())
        || (HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()))
    {
        MoveToDest(owner);
    }

    if (owner->movespline->Finalized())
    {
        RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);
        AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        return false;
    }
    return true;
}

void FormationMovementGenerator::DoDeactivate(Creature* owner)
{
    owner->SetHomePosition(_previousHome);
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

void FormationMovementGenerator::DoFinalize(Creature* owner, bool active, bool movementInform)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->ClearUnitState(UNIT_STATE_ROAMING_MOVE);

    if (movementInform && HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED))
        MovementInform(owner);
}

void FormationMovementGenerator::MovementInform(Creature* owner)
{
    if (owner->AI())
        owner->AI()->MovementInform(FORMATION_MOTION_TYPE, _movementId);
}
