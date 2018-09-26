// rewritten for sunstrider
// ...
// Todo: 
// - Handle the special 180° logic 
// - Redo speed corrections

#include "FormationMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MovementDefines.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"

FormationMovementGenerator::FormationMovementGenerator(uint32 id, ObjectGuid leaderGUID) :
    MovementGeneratorMedium(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_ROAMING),
    _movementId(id), 
    _leaderGUID(leaderGUID)
{
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
}

MovementGeneratorType FormationMovementGenerator::GetMovementGeneratorType() const
{
    return FORMATION_MOTION_TYPE;
}

Position FormationMovementGenerator::GetMemberDestination(Creature* member, uint32 followDist, Position leaderDest, uint8 depth) const
{
    if (!_moveSegment)
        return member->GetPosition();

    //no valid position found after 3 iterations, return leader position instead
    if (depth > 3)
        return leaderDest;

    float pathAngle = _moveSegment->start.GetAbsoluteAngle(_moveSegment->dest);

    Position dest;
    dest.m_orientation = leaderDest.GetOrientation();
    dest.m_positionX = leaderDest.GetPositionX() + cos(_moveSegment->followAngle + pathAngle) * followDist;
    dest.m_positionY = leaderDest.GetPositionY() + sin(_moveSegment->followAngle + pathAngle) * followDist;
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

void FormationMovementGenerator::StartMove(Creature* owner)
{
    if (!_moveSegment)
        return;

    Position start = GetMemberDestination(owner, _moveSegment->followDist, _moveSegment->start);
    Position dest  = GetMemberDestination(owner, _moveSegment->followDist, _moveSegment->dest);

    Movement::MoveSplineInit init(owner);
    Movement::PointsArray points;
    points.reserve(3);
    points.emplace_back(); //dummy point 0, will be replaced in MoveByPath
    if (start.GetExactDist(owner) > 3.0f) //skip first point if its already real close. Might need tweaking
        points.emplace_back(start.GetPositionX(), start.GetPositionY(), start.GetPositionZ());
    points.emplace_back(dest.GetPositionX(),  dest.GetPositionY(),  dest.GetPositionZ());
    init.MovebyPath(points);
    if (dest.GetOrientation())
        init.SetFacing(dest.GetOrientation());

    bool walk = true;
    switch (_moveSegment->moveType)
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
        UnitMoveType mtype = walk ? MOVE_WALK : MOVE_RUN;
        float leaderSegmentLength = leader->GetExactDist(_moveSegment->dest);
        float memberSegmentLength = owner->GetExactDist(start) + owner->GetExactDist(dest);
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

    StartMove(owner);
    
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

    if (!_moveSegment)
        return true; //Just wait for new destination

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        return true;
    }

    if (   (HasFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED) && owner->movespline->Finalized())
        || (HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()))
    {
        StartMove(owner);
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

void FormationMovementGenerator::NewLeaderDestination(FormationMoveSegment moveSegment)
{
    _moveSegment = moveSegment;
    AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED); //trigger new movement if one is already ongoing
}
