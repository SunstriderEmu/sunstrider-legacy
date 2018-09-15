
#include "FollowMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "Optional.h"
#include "PathGenerator.h"
#include "Pet.h"
#include "Player.h"
#include "Unit.h"
#include "Util.h"
#include "Transport.h"

static void DoMovementInform(Unit* owner, Unit* target)
{
    if (owner->GetTypeId() != TYPEID_UNIT)
        return;

    if (CreatureAI* AI = owner->ToCreature()->AI())
        AI->MovementInform(FOLLOW_MOTION_TYPE, target->GetGUID().GetCounter());
}

static bool PositionOkay(Unit* owner, Unit* target, float range, Optional<ChaseAngle> angle = {})
{
    if (owner->GetExactDistSq(target) > square(owner->GetCombatReach() + target->GetCombatReach() + range))
        return false;

    return !angle || angle->IsAngleOkay(target->GetRelativeAngle(owner));
}

FollowMovementGenerator::FollowMovementGenerator(Unit* target, float range, ChaseAngle angle) : 
    AbstractFollower(ASSERT_NOTNULL(target)), 
    MovementGenerator(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_FOLLOW),
    _range(range), _angle(angle) 
{}
FollowMovementGenerator::~FollowMovementGenerator() = default;

bool FollowMovementGenerator::Initialize(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    owner->StopMoving();
    UpdatePetSpeed(owner);
    _path = nullptr;
    _lastTargetPosition.reset();
    return true;
}

void FollowMovementGenerator::Reset(Unit* owner) 
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    Initialize(owner); 
}

bool FollowMovementGenerator::Update(Unit* owner, uint32 diff)
{
    // owner might be dead or gone
    if (!owner || !owner->IsAlive())
        return false;

    // our target might have gone away
    Unit* const target = GetTarget();
    if (!target)
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        owner->StopMoving();
        _lastTargetPosition.reset();
        return true;
    }

    if (owner->HasUnitState(UNIT_STATE_FOLLOW_MOVE))
    {
        if (_checkTimer > diff)
            _checkTimer -= diff;
        else
        {
            _checkTimer = CHECK_INTERVAL;
            if (PositionOkay(owner, target, _range, _angle))
            {
                _path = nullptr;
                owner->StopMoving();
                DoMovementInform(owner, target);
                return true;
            }
        }
    }

    if (owner->HasUnitState(UNIT_STATE_FOLLOW_MOVE) && owner->movespline->Finalized())
    {
        _path = nullptr;
        owner->ClearUnitState(UNIT_STATE_FOLLOW_MOVE);
        DoMovementInform(owner, target);
    }

    if (!_lastTargetPosition || _lastTargetPosition->GetExactDistSq(target->GetPosition()) > 0.0f)
    {
        _lastTargetPosition = target->GetPosition();
        if (owner->HasUnitState(UNIT_STATE_FOLLOW_MOVE) || !PositionOkay(owner, target, _range + FOLLOW_RANGE_TOLERANCE))
        {
            if (!_path)
                _path = std::make_unique<PathGenerator>(owner);

            Transport* targetTransport = target->GetTransport();
            // Creature will always use target mmaps
            _path->SetTransport(targetTransport);

            float x, y, z;

            // select angle
            float tAngle;
            float const curAngle = target->GetRelativeAngle(owner);
            if (_angle.IsAngleOkay(curAngle))
                tAngle = curAngle;
            else
            {
                float const diffUpper = Position::NormalizeOrientation(curAngle - _angle.UpperBound());
                float const diffLower = Position::NormalizeOrientation(_angle.LowerBound() - curAngle);
                if (diffUpper < diffLower)
                    tAngle = _angle.UpperBound();
                else
                    tAngle = _angle.LowerBound();
            }

            target->GetNearPoint(owner, x, y, z, _range, target->ToAbsoluteAngle(tAngle));

            if (owner->IsHovering())
                owner->UpdateAllowedPositionZ(x, y, z);

            // pets are allowed to "cheat" on pathfinding when following their master
            bool allowShortcut = false;
            if (ObjectGuid ownerGUID = owner->GetOwnerGUID())
                if (target->GetGUID() == ownerGUID)
                    allowShortcut = true;

            bool success = _path->CalculatePath(x, y, z, allowShortcut);
            if (!success || (_path->GetPathType() & PATHFIND_NOPATH))
            {
                owner->StopMoving();
                return true;
            }

            owner->AddUnitState(UNIT_STATE_FOLLOW_MOVE);

            Movement::MoveSplineInit init(owner);
            init.MovebyPath(_path->GetPath(), 0, targetTransport);
            init.SetWalk(target->IsWalking());
            if (!target->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD) && !targetTransport) //sun: don't do it if target is currently going backwards, as this is visually ugly + don't do it on transport for now, we'd need to translate orientation 
                init.SetFacing(target->GetOrientation());

            init.Launch();
        }
    }
    return true;
}

void FollowMovementGenerator::Deactivate(Unit* owner)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_FOLLOW_MOVE);
}

void FollowMovementGenerator::Finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
    {
        owner->ClearUnitState(UNIT_STATE_FOLLOW_MOVE);
        UpdatePetSpeed(owner);
    }
}

void FollowMovementGenerator::UpdatePetSpeed(Unit* owner)
{
    if (Pet* oPet = owner->ToPet())
        if (!GetTarget() || GetTarget()->GetGUID() == owner->GetOwnerGUID())
        {
            oPet->UpdateSpeed(MOVE_RUN);
            oPet->UpdateSpeed(MOVE_WALK);
            oPet->UpdateSpeed(MOVE_SWIM);
        }
}
