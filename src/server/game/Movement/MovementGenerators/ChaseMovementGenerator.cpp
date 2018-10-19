
#include "ChaseMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "G3DPosition.hpp"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "PathGenerator.h"
#include "Unit.h"
#include "Util.h"

static bool IsMutualChase(Unit* owner, Unit* target)
{
    if (target->GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE)
        return false;

    if (ChaseMovementGenerator* movement = dynamic_cast<ChaseMovementGenerator*>(target->GetMotionMaster()->GetCurrentMovementGenerator()))
        return movement->GetTarget() == owner;

    return false;
}

static bool PositionOkay(Unit* owner, Unit* target, Optional<float> minDistance, Optional<float> maxDistance, Optional<ChaseAngle> angle)
{
    float const distSq = owner->GetExactDistSq(target);
    if (minDistance && distSq < square(*minDistance))
        return false;
    if (maxDistance && distSq > square(*maxDistance))
        return false;
    if (owner->GetTransGUID() != target->GetTransGUID())
        return false;
    return !angle || angle->IsAngleOkay(target->GetRelativeAngle(owner));
}

static void DoMovementInform(Unit* owner, Unit* target)
{
    if (owner->GetTypeId() != TYPEID_UNIT)
        return;

    if (CreatureAI* AI = owner->ToCreature()->AI())
        AI->MovementInform(CHASE_MOTION_TYPE, target->GetGUID().GetCounter());
}

ChaseMovementGenerator::ChaseMovementGenerator(Unit* target, Optional<ChaseRange> range, Optional<ChaseAngle> angle, bool run /*= true*/) :
    AbstractFollower(ASSERT_NOTNULL(target)), 
    MovementGenerator(MOTION_MODE_DEFAULT, MOTION_PRIORITY_NORMAL, UNIT_STATE_CHASE),
    _range(range), _angle(angle), _run(run)
{
}

ChaseMovementGenerator::~ChaseMovementGenerator() = default;

bool ChaseMovementGenerator::Initialize(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    _lastTargetPosition.reset();
    owner->SetWalk(!_run);
    _path = nullptr;
    return true;
}
#define MAX_SPREAD_ATTEMPTS 3

void ChaseMovementGenerator::DoSpreadIfNeeded(Unit* owner, Unit* target)
{
    // Move away from any NPC deep in our bounding box. There's no limit to the
    // angle moved; NPCs will eventually start spreading behind the target if
    // there's enough of them.
    Unit* spreadingTarget = nullptr;

    for (auto& attacker : target->GetAttackers())
    {
        if (attacker->GetTypeId() == TYPEID_UNIT && (attacker != owner) &&   // Get other creature attacker
            (owner->GetCombatReach() - 2.0f < attacker->GetCombatReach()) && // With smaller, equal or slighty bigger combat reach (why?)
            !attacker->isMoving() &&                                         // And not moving
                                                                             // And close to us
            (owner->GetDistanceSqr(attacker->GetPositionX(), attacker->GetPositionY(), attacker->GetPositionZ()) < std::min(std::max(owner->GetCombatReach(), attacker->GetCombatReach()), 0.25f)))
        {
            spreadingTarget = attacker;
            break;
        }
    }

    if (!spreadingTarget)
    {
        _canSpread = false;
        return;
    }

    float const myAngle = target->GetAbsoluteAngle(owner);
    float const hisAngle = target->GetAbsoluteAngle(spreadingTarget);
    float const newAngle = (hisAngle > myAngle) ? myAngle - frand(0.4f, 1.0f) : myAngle + frand(0.4f, 1.0f);
    //hitboxSum and maxRange match chase max range logic in Update
    float const hitboxSum = owner->GetCombatReach() + target->GetCombatReach();
    float maxRange = _range ? _range->MaxRange + hitboxSum : owner->GetMeleeRange(target); // melee range already includes hitboxes
    maxRange -= owner->GetCombatReach(); // Combat reach gets added again in GetNearPoint
    static float const minSpreadRange = 0.8f;
    if (maxRange < minSpreadRange)
    {
        _canSpread = false;
        return;
    }

    float const nearRange = frand(minSpreadRange, std::min(target->GetAttackers().size() > 5 ? 4.0f : 2.0f, maxRange));

    float x, y, z;
    target->GetNearPoint(owner, x, y, z, nearRange, newAngle);

    _spreadAttempts++;

    // Don't circle target infinitely if too many attackers.
    if (_spreadAttempts >= MAX_SPREAD_ATTEMPTS)
        _canSpread = false;

    Movement::MoveSplineInit init(owner);
    init.MoveTo(x, y, z);
    init.SetWalk(true);
    init.Launch();
}

void ChaseMovementGenerator::Reset(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    Initialize(owner);
}

bool ChaseMovementGenerator::Update(Unit* owner, uint32 diff)
{
    // owner might be dead or gone (can we even get nullptr here?)
    if (!owner || !owner->IsAlive())
        return false;

    Creature* cOwner = owner->ToCreature();

    // our target might have gone away
    Unit* const target = GetTarget();
    if (!target)
        return false;

    // the owner might've selected a different target (feels like we shouldn't check this here...)
    if (owner->GetVictim() != target)
        return false;

    // the owner might be unable to move (rooted or casting), pause movement
    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        owner->StopMoving();
        _lastTargetPosition.reset();
        if (cOwner)
            cOwner->SetCannotReachTarget(false);
        return true;
    }

    bool const mutualChase     = IsMutualChase(owner, target);
    float const hitboxSum      = owner->GetCombatReach() + target->GetCombatReach();
    //exact dist min range
    float const minTolerance   = _range ? _range->MinTolerance + hitboxSum : owner->GetCombatReach();
    float const minTarget      = (_range ? _range->MinRange : 0.0f) + hitboxSum;
    //exact dist max range
    float const maxTolerance   = _range ? _range->MaxTolerance + hitboxSum : owner->GetMeleeRange(target); // melee range already includes hitboxes
    float const maxTarget      = _range ? _range->MaxRange + hitboxSum : CONTACT_DISTANCE + hitboxSum;
    Optional<ChaseAngle> angle = mutualChase ? Optional<ChaseAngle>() : _angle;

    // if we're already moving, periodically check if we're already in the expected range...
    if (owner->HasUnitState(UNIT_STATE_CHASE_MOVE))
    {
        if (_rangeCheckTimer > diff)
            _rangeCheckTimer -= diff;
        else
        {
            _rangeCheckTimer = RANGE_CHECK_INTERVAL; 
            if (PositionOkay(owner, target, _movingTowards ? Optional<float>() : minTarget, _movingTowards ? maxTarget : Optional<float>(), angle))
            {
                _path = nullptr;
                owner->StopMoving();
                owner->SetInFront(target);
                DoMovementInform(owner, target);
                return true;
            }
        }
    }

    // if the target moved, we have to consider whether to adjust
    bool positionChanged = false;
    if (mutualChase != _mutualChase)
        positionChanged = true;
    else if (!_lastTargetPosition)
        positionChanged = true;
    else if (_lastTargetPosition->GetPositionX() != target->GetPositionX()
          || _lastTargetPosition->GetPositionY() != target->GetPositionY()
          || _lastTargetPosition->GetPositionZ() != target->GetPositionZ()) // we can't just used == operator because we want to ignore orientation
        positionChanged = true;

    if (positionChanged)
    {
        _lastTargetPosition = target->GetPosition();
        _mutualChase = mutualChase;
        if (owner->HasUnitState(UNIT_STATE_CHASE_MOVE) || !PositionOkay(owner, target, minTolerance, maxTolerance, angle))
        {
            // can we get to the target?
            if (cOwner && !target->isInAccessiblePlaceFor(cOwner))
            {
                cOwner->SetCannotReachTarget(true);
                cOwner->StopMoving();
                _path = nullptr;
                return true;
            }

            // figure out which way we want to move
            bool const moveToward = !owner->IsInDist(target, maxTolerance);

            //sun: always create a new Generator, creature fly/walk/swim may have changed
            _path = std::make_unique<PathGenerator>(owner);
      
            Transport* targetTransport = target->GetTransport();
            _path->SetTransport(targetTransport);

            float x, y, z;
            bool shortenPath;
            // if we want to move toward the target and there's no fixed angle...
            if (moveToward && !angle.is_initialized())
            {
                // ...we'll pathfind to the center, then shorten the path
                target->GetPosition(x, y, z);
                shortenPath = true;
            }
            else
            {
                // otherwise, we fall back to nearpoint finding
                //sun: note that LoS in checked in GetNearPoint, it'll try to get a closer position if target is not in los
                //sun: cancel combat reach added in GetNearPoint
                target->GetNearPoint(owner, x, y, z, (moveToward ? maxTarget : minTarget) - target->GetCombatReach(), angle.is_initialized() ? target->ToAbsoluteAngle(angle->RelativeAngle) : target->GetAbsoluteAngle(owner));
                shortenPath = false;
            }

            if (owner->IsHovering())
                owner->UpdateAllowedPositionZ(x, y, z);

            // sun: force dest for all bosses
            bool forceDest = cOwner && (cOwner->IsWorldBoss() || cOwner->IsDungeonBoss()); 

            bool success = _path->CalculatePath(x, y, z, forceDest);
            if (!success || (_path->GetPathType() & PATHFIND_NOPATH))
            {
                if (cOwner)
                    cOwner->SetCannotReachTarget(true);

                owner->StopMoving();
                return true;
            }

            if (shortenPath)
                _path->ShortenPathUntilDist(PositionToVector3(target), maxTarget);

            if (cOwner)
                cOwner->SetCannotReachTarget(false);

            owner->AddUnitState(UNIT_STATE_CHASE_MOVE);

            Movement::MoveSplineInit init(owner);
            init.MovebyPath(_path->GetPath(), 0, targetTransport);
            init.SetWalk(!_run);
            init.SetFacing(target);

            init.Launch();

            _movingTowards = moveToward;
        }
    }

    if (owner->movespline->Finalized())
    {
        // We just finished a chase move, set in front and notify
        if (owner->HasUnitState(UNIT_STATE_CHASE_MOVE))
        {
            _path = nullptr;
            if (cOwner)
                cOwner->SetCannotReachTarget(false);

            owner->ClearUnitState(UNIT_STATE_CHASE_MOVE);
            owner->SetInFront(target);

            DoMovementInform(owner, target);

            _spreadAttempts = 0;
            _canSpread = true;
            _spreadTimer.Reset(1000);
        }
        // Handle spreading
        else if(cOwner 
            && !cOwner->IsPet() 
            && maxTolerance < 5.0f
            && !cOwner->IsDungeonBoss() 
            && !cOwner->IsWorldBoss() 
            && !target->isMoving() 
            && _canSpread)
        {
            //!(cOwner->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_CHASE_GEN_NO_BACKING) 
            _spreadTimer.Update(diff);
            if (_spreadTimer.Passed())
            {
                _spreadTimer.Reset(urand(1500, 2500));
                DoSpreadIfNeeded(owner, target);
            }
        }
    }

    // and then, finally, we're done for the tick
    return true;
}

void ChaseMovementGenerator::Deactivate(Unit* owner)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_CHASE_MOVE);
    if (Creature* cOwner = owner->ToCreature())
        cOwner->SetCannotReachTarget(false);
}

void ChaseMovementGenerator::Finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->ClearUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
        if (Creature* cOwner = owner->ToCreature())
            cOwner->SetCannotReachTarget(false);
    }
}
