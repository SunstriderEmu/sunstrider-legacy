
#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"

#include "ConfusedMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "RotateMovementGenerator.h"
#include "DistractMovementGenerator.h"
#include "StealthAlertMovementGenerator.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include <cassert>

inline MovementGenerator* GetIdleMovementGenerator()
{
    return sMovementGeneratorRegistry->GetRegistryItem(IDLE_MOTION_TYPE)->Create();
}

inline bool isStatic(MovementGenerator* movement)
{
    return (movement == GetIdleMovementGenerator());
}

void MotionMaster::Initialize()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator *movement = top();
        pop();
        if (movement)
            DirectDelete(movement, true);
    }

    while (!_expireList.empty())
    {
        delete _expireList.back();
        _expireList.pop_back();
    }

    InitDefault();
}

// set new default movement generator
void MotionMaster::InitDefault()
{
    Mutate(FactorySelector::SelectMovementGenerator(_owner), MOTION_SLOT_IDLE);
}

MotionMaster::~MotionMaster()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator *curr = top();
        pop();
        if (curr && !isStatic(curr))
            delete curr;    // Skip finalizing on delete, it might launch new movement
    }
}

void MotionMaster::UpdateMotion(uint32 diff)
{
    if (!_owner)
        return;

    ASSERT(!empty());

    _cleanFlag |= MMCF_UPDATE;
    if (!top()->Update(_owner, diff))
    {
        _cleanFlag &= ~MMCF_UPDATE;
        /* sunstrider: tweaked this, a movement generator in the idle telling us it's done can only mean two things atm :
        - Waypoint generator has an invalid path 
        - Waypoint generator has finished its path (not repeating case). (TC handle this case by having the generator stalling)
        In both case, we want to end the generator. In both case restarting the waypoint generator does not make sense.
        */
        if (_top != MOTION_SLOT_IDLE)
            MovementExpired();
        else
            MoveIdle();
    } else
        _cleanFlag &= ~MMCF_UPDATE;

    if (!_expireList.empty())
        ClearExpireList();
}

void MotionMaster::Clear(bool reset /* = true*/)
{
    if (_cleanFlag & MMCF_UPDATE)
    {
        if (reset)
            _cleanFlag |= MMCF_RESET;
        else
            _cleanFlag &= ~MMCF_RESET;
        DelayedClean();
    }
    else
        DirectClean(reset);
}

void MotionMaster::MovementExpired(bool reset /* = true*/)
{
    if (_cleanFlag & MMCF_UPDATE)
    {
        if (reset)
            _cleanFlag |= MMCF_RESET;
        else
            _cleanFlag &= ~MMCF_RESET;
        DelayedExpire(false);
    }
    else
        DirectExpire(reset, false);
}

void MotionMaster::ClearExpireList()
{
    for (auto itr : _expireList)
        DirectDelete(itr);

    _expireList.clear();

    if (empty())
        Initialize();
    else if (needInitTop())
        InitTop();
    else if (_cleanFlag & MMCF_RESET)
        top()->Reset(_owner);

    _cleanFlag &= ~MMCF_RESET;
}

void MotionMaster::DirectClean(bool reset)
{
    while (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        if (curr) DirectDelete(curr, true);
    }

    if (empty())
        return;

    if (needInitTop())
        InitTop();
    else if (reset)
        top()->Reset(_owner);
}

void MotionMaster::DelayedClean()
{
    while (size() > 1)
    {
        MovementGenerator *curr = top();
        pop();
        if (curr)
            DelayedDelete(curr);
    }
}

//delete top movement generator if any, and re init top
void MotionMaster::DirectExpire(bool reset, bool premature)
{
    if (size() > 1)
    {
        int previousTop = _top;
        MovementGenerator *curr = top();
        ASSERT(curr != nullptr);
        pop();
        DirectDelete(curr, premature);

        /* sunstrider: There is one case when whe shouldn't reset: if a new movement generator was created (in scripts) during the delete,
        it was just initialized and we don't want to reset it. We can detect that's the case if _top has not decreased.
        Any case this may be wrong?    */
        if (reset && _top >= previousTop)
            reset = false;
    }

    //set the new top
    while (!empty() && !top())
        --_top;

    if (empty())
        Initialize();
    else if (needInitTop())
        InitTop();
    else if (reset)
    {
        top()->Reset(_owner);
    }
}

void MotionMaster::DelayedExpire(bool premature)
{
    if (size() > 1)
    {
        MovementGenerator *curr = top();
        ASSERT(curr != nullptr);
        pop();
        DelayedDelete(curr, premature);
    }

    while (!empty() && !top())
        --_top;
}

void MotionMaster::MoveIdle()
{
    if(!empty())
        MovementExpired(false);
    //! Should be preceded by MovementExpired or Clear if there's an overlying movementgenerator active
    if (empty() || !isStatic(top()))
        Mutate(GetIdleMovementGenerator(), MOTION_SLOT_IDLE);
}

void MotionMaster::MoveRandom(float spawndist)
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    if (_owner->GetTypeId() == TYPEID_UNIT)
    {
        //  TC_LOG_TRACE("misc", "Creature (GUID: %u) start moving random", _owner->GetGUIDLow());
        Mutate(new RandomMovementGenerator<Creature>(spawndist), MOTION_SLOT_IDLE);
    }
}

void MotionMaster::MoveTargetedHome()
{
    Clear(false);

    if (_owner->GetTypeId() == TYPEID_UNIT && !_owner->ToCreature()->GetCharmerOrOwnerGUID())
    {
        // TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) targeted home", _owner->GetEntry(), _owner->GetGUIDLow());
        Mutate(new HomeMovementGenerator<Creature>(), MOTION_SLOT_ACTIVE);
    }
    else if (_owner->GetTypeId() == TYPEID_UNIT && _owner->ToCreature()->GetCharmerOrOwnerGUID() && !_owner->ToCreature()->IsTotem())
    {
        //  TC_LOG_TRACE("misc", "Pet or controlled creature (Entry: %u GUID: %u) targeting home", _owner->GetEntry(), _owner->GetGUIDLow());
        Unit* target = _owner->ToCreature()->GetCharmerOrOwner();
        if (target)
        {
            //  TC_LOG_TRACE("misc", "Following %s (GUID: %u)", target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature", target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : ((Creature*)target)->GetSpawnId());
            Mutate(new FollowMovementGenerator<Creature>(target, PET_FOLLOW_DIST, _owner->GetFollowAngle()), MOTION_SLOT_ACTIVE);
        }
    }
    else
    {
        TC_LOG_ERROR("misc", "Player (GUID: %u) attempt targeted home", _owner->GetGUIDLow());
    }
}

void MotionMaster::MoveConfused()
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        //  TC_LOG_TRACE("misc", "Player (GUID: %u) move confused", _owner->GetGUIDLow());
        Mutate(new ConfusedMovementGenerator<Player>(), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        //  TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) move confused",
        //    _owner->GetEntry(), _owner->GetGUIDLow());
        Mutate(new ConfusedMovementGenerator<Creature>(), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveChase(Unit* target, float dist, float angle)
{
    // ignore movement request if target not exist
    if (!target || target == _owner || _owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    //_owner->ClearUnitState(UNIT_STATE_FOLLOW);
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        /* TC_LOG_TRACE("misc", "Player (GUID: %u) chase to %s (GUID: %u)",
            _owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetSpawnId()); */
        Mutate(new ChaseMovementGenerator<Player>(target, dist, angle), MOTION_SLOT_ACTIVE);
    }
    else
    {
        /*  TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) chase to %s (GUID: %u)",
            _owner->GetEntry(), _owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetSpawnId());  */
        Mutate(new ChaseMovementGenerator<Creature>(target, dist, angle), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveFollow(Unit* target, float dist, float angle, MovementSlot slot)
{
    // ignore movement request if target not exist
    if (!target || target == _owner || _owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    //_owner->AddUnitState(UNIT_STATE_FOLLOW);
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        /*  TC_LOG_TRACE("misc", "Player (GUID: %u) follow to %s (GUID: %u)", _owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetSpawnId()); */
        Mutate(new FollowMovementGenerator<Player>(target, dist, angle), slot);
    }
    else
    {
        /* TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) follow to %s (GUID: %u)",
            _owner->GetEntry(), _owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetSpawnId()); */
        Mutate(new FollowMovementGenerator<Creature>(target, dist, angle), slot);
    }
}

void MotionMaster::MovePoint(uint32 id, float x, float y, float z, float o, bool generatePath, bool forceDestination)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        //  TC_LOG_TRACE("misc", "Player (GUID: %u) targeted point (Id: %u X: %f Y: %f Z: %f O: %f)", _owner->GetGUIDLow(), id, x, y, z, o);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, o, generatePath, 0.0f, forceDestination), MOTION_SLOT_ACTIVE);
    }
    else
    {
       /* TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) targeted point (ID: %u X: %f Y: %f Z: %f O: %f)",
            _owner->GetEntry(), _owner->GetGUIDLow(), id, x, y, z, o); */
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, o, generatePath, 0.0f, forceDestination), MOTION_SLOT_ACTIVE);
    }
}

/* Unused, from Sunwell core. Not sure what this actually does.
void MotionMaster::GenerateWaypointArray(Unit* me, Movement::PointsArray& from, uint32 startingWaypointId, Movement::PointsArray& points)
{
    if (from.empty())
        return;

    // Flying unit, just fill array
    if (me->m_movementInfo.HasMovementFlag((MovementFlags)(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY)))
    {
        // sunwell: first point in vector is unit real position
        points.clear();
        points.push_back(G3D::Vector3(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()));
        for (uint32 i = startingWaypointId; i < from.size(); i++)
            points.push_back(G3D::Vector3(from[i].x, from[i].y, from[i].z));
    }
    else
    {
        for (float size = 1.0f; size; size *= 0.5f)
        {
            std::vector<G3D::Vector3> pVector;
            // sunwell: first point in vector is unit real position
            pVector.push_back(G3D::Vector3(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()));
            uint32 length = (from.size() - startingWaypointId)*size;

            uint32 cnt = 0;
            for (uint32 i = startingWaypointId; i < from.size() && cnt <= length; i++, ++cnt)
                pVector.push_back(G3D::Vector3(from[i].x, from[i].y, from[i].z));

            if (pVector.size() > 2) // more than source + dest
            {
                G3D::Vector3 middle = (pVector[0] + pVector[pVector.size() - 1]) / 2.f;
                G3D::Vector3 offset;

                bool continueLoop = false;
                for (uint32 i = 1; i < pVector.size() - 1; ++i)
                {
                    offset = middle - pVector[i];
                    if (fabs(offset.x) >= 0xFF || fabs(offset.y) >= 0xFF || fabs(offset.z) >= 0x7F)
                    {
                        // offset is too big, split points
                        continueLoop = true;
                        break;
                    }
                }
                if (continueLoop)
                    continue;
            }
            // everything ok
            points = pVector;
            break;
        }
    }
}
*/

void MotionMaster::MoveCloserAndStop(uint32 id, Unit* target, float distance)
{
    float distanceToTravel = _owner->GetExactDist2d(target) - distance;
    if (distanceToTravel > 0.0f)
    {
        float angle = _owner->GetAngle(target);
        float destx = _owner->GetPositionX() + distanceToTravel * std::cos(angle);
        float desty = _owner->GetPositionY() + distanceToTravel * std::sin(angle);
        MovePoint(id, destx, desty, target->GetPositionZ());
    } else
    {
        // we are already close enough. We just need to turn toward the target without changing position.
        Movement::MoveSplineInit init(_owner);
        init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZMinusOffset());
        init.SetFacing(target);
        init.Launch();
        Mutate(new EffectMovementGenerator(id), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveLand(uint32 id, Position const& pos)
{
    float x, y, z;
    pos.GetPosition(x, y, z);

   // TC_LOG_TRACE("misc", "Creature (Entry: %u) landing point (ID: %u X: %f Y: %f Z: %f)", _owner->GetEntry(), id, x, y, z);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
    //TODOLK init.SetAnimation(Movement::ToGround);
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveTakeoff(uint32 id, Position const& pos)
{
    float x, y, z;
    pos.GetPosition(x, y, z);

    //TC_LOG_TRACE("misc", "Creature (Entry: %u) landing point (ID: %u X: %f Y: %f Z: %f)", _owner->GetEntry(), id, x, y, z);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
#ifdef LICH_KING
    init.SetAnimation(Movement::ToFly);
#endif
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ)
{
    //this function may make players fall below map
    if (_owner->GetTypeId() == TYPEID_PLAYER)
        return;

    //BC creatures can't be knocked back
#ifdef LICH_KING
    else
        return; //BC creatures can't be knocked back

    if (speedXY <= 0.1f)
        return;

    float x, y, z;
    float moveTimeHalf = speedZ / Movement::gravity;
    float dist = 2 * moveTimeHalf * speedXY;
    //float max_heigt = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);

    _owner->GetNearPoint(_owner, x, y, z, _owner->GetCombatReach(), dist, _owner->GetAngle(srcX, srcY) + M_PI);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z);
    //init.SetParabolic(max_height, 0);
    //init.SetOrientationFixed(true);
    init.SetVelocity(speedXY);
    init.Launch();
    Mutate(new EffectMovementGenerator(0), MOTION_SLOT_CONTROLLED);
#endif
}

void MotionMaster::MoveJumpTo(float angle, float speedXY, float speedZ)
{
    //this function may make players fall below map
    if (_owner->GetTypeId() == TYPEID_PLAYER)
        return;

    float x, y, z;

    float moveTimeHalf = speedZ / Movement::gravity;
    float dist = 2 * moveTimeHalf * speedXY;
    _owner->GetNearPoint2D(x, y, dist, _owner->GetOrientation() + angle);
    z = _owner->GetPositionZ() + _owner->GetMidsectionHeight();
    _owner->UpdateAllowedPositionZ(x, y, z);
    MoveJump(x, y, z, speedXY, speedZ);
}

void MotionMaster::MoveJump(float x, float y, float z, float speedXY, float speedZ, uint32 id)
{
    //TC_LOG_TRACE("misc", "Unit (GUID: %u) jump to point (X: %f Y: %f Z: %f)", _owner->GetGUIDLow(), x, y, z);
    if (speedXY <= 0.1f)
        return;

//    float moveTimeHalf = speedZ / Movement::gravity;
 //   float max_height = -Movement::computeFallElevation(moveTimeHalf, false, -speedZ);

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z, false);
//    init.SetParabolic(max_height, 0);
    init.SetVelocity(speedXY);
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveFall(uint32 id /*=0*/)
{
    // use larger distance for vmap height search than in most other cases
    float tz = _owner->GetMap()->GetHeight(_owner->GetPhaseMask(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (tz <= INVALID_HEIGHT)
    {
        TC_LOG_DEBUG("misc", "MotionMaster::MoveFall: unable retrive a proper height at map %u (x: %f, y: %f, z: %f).",
            _owner->GetMap()->GetId(), _owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ());
        return;
    }

    // Abort too if the ground is very near
    if (fabs(_owner->GetPositionZ() - tz) < 0.1f)
        return;

    _owner->AddUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING);
    _owner->m_movementInfo.SetFallTime(0);

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        _owner->ToPlayer()->SetFallInformation(0, _owner->GetPositionZ());
        return;
    }

    Movement::MoveSplineInit init(_owner);
    init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), tz, false);
    init.SetFall();
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}

void MotionMaster::MoveCharge(float x, float y, float z, float speed, uint32 id, bool generatePath)
{
    if (Impl[MOTION_SLOT_CONTROLLED] && Impl[MOTION_SLOT_CONTROLLED]->GetMovementGeneratorType() != DISTRACT_MOTION_TYPE)
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        //TC_LOG_TRACE("misc", "Player (GUID: %u) charge point (X: %f Y: %f Z: %f)", _owner->GetGUIDLow(), x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, generatePath, speed), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        /*TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) charge point (X: %f Y: %f Z: %f)",
            _owner->GetEntry(), _owner->GetGUIDLow(), x, y, z); */
        //stop stealth detect warning if any, so that it does not restore orientation afterwards
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, speed), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveCharge(PathGenerator const& path, float speed /*= SPEED_CHARGE*/, Unit* target /* = nullptr */)
{
    G3D::Vector3 dest = path.GetActualEndPosition();

    MoveCharge(dest.x, dest.y, dest.z, speed, EVENT_CHARGE_PREPATH);

    // Charge movement is not started when using EVENT_CHARGE_PREPATH
    Movement::MoveSplineInit init(_owner);
    init.MovebyPath(path.GetPath());
    init.SetVelocity(speed);
    if(target)
        init.SetFacing(target);
    init.Launch();
}

void MotionMaster::MoveSeekAssistance(float x, float y, float z)
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        TC_LOG_ERROR("misc", "Player (GUID: %u) attempt to seek assistance", _owner->GetGUIDLow());
    }
    else
    {
        /*TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) seek assistance (X: %f Y: %f Z: %f)",
            _owner->GetEntry(), _owner->GetGUIDLow(), x, y, z); */
        _owner->AttackStop();
        _owner->CastStop();
        _owner->ToCreature()->SetReactState(REACT_PASSIVE);
        Mutate(new AssistanceMovementGenerator(x, y, z), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveSeekAssistanceDistract(uint32 time)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        //TC_LOG_TRACE("misc", "Player (GUID: %u) attempt to call distract after assistance", _owner->GetGUIDLow());
    }
    else
    {
        /*TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) is distracted after assistance call (Time: %u)",
            _owner->GetEntry(), _owner->GetGUIDLow(), time); */
        Mutate(new AssistanceDistractMovementGenerator(time), MOTION_SLOT_ACTIVE);
    }
}

void MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    if (_owner->HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        /*TC_LOG_TRACE("misc", "Player (GUID: %u) flee from %s (GUID: %u)", _owner->GetGUIDLow(),
            enemy->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            enemy->GetTypeId() == TYPEID_PLAYER ? enemy->GetGUIDLow() : enemy->ToCreature()->GetSpawnId()); */
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        /*TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) flee from %s (GUID: %u)%s",
            _owner->GetEntry(), _owner->GetGUIDLow(),
            enemy->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            enemy->GetTypeId() == TYPEID_PLAYER ? enemy->GetGUIDLow() : enemy->ToCreature()->GetSpawnId(),
            time ? " for a limited time" : ""); */
        if (time)
            Mutate(new TimedFleeingMovementGenerator(enemy->GetGUID(), time), MOTION_SLOT_CONTROLLED);
        else
            Mutate(new FleeingMovementGenerator<Creature>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveTaxiFlight(uint32 path, uint32 pathnode)
{
    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (path < sTaxiPathNodesByPath.size())
        {
            //TC_LOG_TRACE("misc", "%s taxi to (Path %u node %u)", _owner->GetName().c_str(), path, pathnode);
            FlightPathMovementGenerator* mgen = new FlightPathMovementGenerator(pathnode);
            mgen->LoadPath(_owner->ToPlayer());
            Mutate(mgen, MOTION_SLOT_CONTROLLED);
        }
        else
        {
            TC_LOG_ERROR("misc", "%s attempt taxi to (not existed Path %u node %u)",
            _owner->GetName().c_str(), path, pathnode);
        }
    }
    else
    {
        TC_LOG_ERROR("misc", "Creature (Entry: %u GUID: %u) attempt taxi to (Path %u node %u)",
            _owner->GetEntry(), _owner->GetGUIDLow(), path, pathnode);
    }
}

void MotionMaster::MoveStealthAlert(Unit const* target, uint32 timer)
{
    if (_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL))
        return;

    /*TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: " UI64FMTD ") almost detected target " UI64FMTD " (timer: %u)",
        _owner->GetEntry(), _owner->GetGUID(), target->GetGUID(), timer); */

    Mutate(new StealthAlertMovementGenerator(_owner, target, timer), MOTION_SLOT_ACTIVE);
}

void MotionMaster::MoveDistract(float x, float y, uint32 timer)
{
    //return if already a generator in MOTION_SLOT_CONTROLLED
    if (Impl[MOTION_SLOT_CONTROLLED])
        return;

    if (_owner->GetTypeId() == TYPEID_PLAYER)
    {
        //TC_LOG_TRACE("misc", "Player (GUID: %u) is distracted (timer: %u)", _owner->GetGUIDLow(), timer);
    }
    else
    {
        /*TC_LOG_TRACE("misc", "Creature (Entry: %u GUID: %u) is distracted (timer: %u)",
            _owner->GetEntry(), _owner->GetGUIDLow(), timer); */
    }

    float angle = _owner->GetAngle(x,y);
    Mutate(new DistractMovementGenerator(_owner, angle, timer), MOTION_SLOT_CONTROLLED);
}

bool MotionMaster::Mutate(MovementGenerator *m, MovementSlot slot)
{
    //remove last generator in this slot if any
    if (MovementGenerator *curr = Impl[slot])
    {
        bool delayed = (_top == slot && (_cleanFlag & MMCF_UPDATE));

        Impl[slot] = nullptr; // in case a new one is generated in this slot during directdelete
        //kelno: clear slot AND decrease top immediately to avoid crashes when referencing null top in DirectDelete
        while (!empty() && !top())
            --_top;

        if (delayed)
            DelayedDelete(curr, true);
        else
            DirectDelete(curr, true);
    }

    //set top to new top if needed
    if (_top < slot)
        _top = slot;

    Impl[slot] = m;
    ASSERT(Impl[slot] != nullptr);
    //if generator is new top, init it immediately, else mark as need init
    if (_top > slot)
        _needInit[slot] = true;
    else
    {
        _needInit[slot] = false;
        return m->Initialize(_owner);
    }
    return true;
}

void MotionMaster::MovePath(uint32 path_id, bool repeatable, bool smoothSpline)
{
    if (!path_id)
        return;
    //We set waypoint movement as new default movement generator
    Mutate(new WaypointMovementGenerator<Creature>(path_id, repeatable, smoothSpline), MOTION_SLOT_IDLE);

   /* TC_LOG_DEBUG("misc", "%s (GUID: %u) start moving over path(Id: %u)",
        _owner->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature",
        _owner->GetGUIDLow(), path_id); */
}

void MotionMaster::MovePath(WaypointPath& path, bool repeatable, bool smoothSpline)
{
    //We set waypoint movement as new default movement generator. If non repeating case, the generator will trigger a MoveIdle itself at the end
    Mutate(new WaypointMovementGenerator<Creature>(path, repeatable, smoothSpline), MOTION_SLOT_IDLE);

    TC_LOG_DEBUG("misc", "%s (GUID: %u) start moving over path(repeatable: %s)", _owner->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature", _owner->GetGUIDLow(), repeatable ? "YES" : "NO");
}

void MotionMaster::MoveRotate(uint32 time, RotateDirection direction)
{
    if (!time)
        return;

    Mutate(new RotateMovementGenerator(time, direction), MOTION_SLOT_ACTIVE);
}

void MotionMaster::PropagateSpeedChange()
{
    for (int i = 0; i <= _top; ++i)
    {
        if (Impl[i])
            Impl[i]->UnitSpeedChanged();
    }
}

void MotionMaster::ReinitializeMovement()
{
    for (int i = 0; i <= _top; ++i)
    {
        if (Impl[i])
            Impl[i]->Reset(_owner);
    }
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
   if (empty())
       return IDLE_MOTION_TYPE;

   return top()->GetMovementGeneratorType();
}

MovementGeneratorType MotionMaster::GetMotionSlotType(int slot) const
{
    if (!Impl[slot])
        return NULL_MOTION_TYPE;
    else
        return Impl[slot]->GetMovementGeneratorType();
}

// sunwell: Escort system
uint32 MotionMaster::GetCurrentSplineId() const
{
    if (empty())
        return 0;

    return top()->GetSplineId();
}

void MotionMaster::InitTop()
{
    top()->Initialize(_owner);
    _needInit[_top] = false;
}

void MotionMaster::DirectDelete(_Ty curr, bool premature)
{
    if (isStatic(curr))
        return;
    curr->Finalize(_owner, premature);
    delete curr;
}

void MotionMaster::DelayedDelete(_Ty curr, bool premature)
{
    TC_LOG_FATAL("misc", "Unit (Entry %u) is trying to delete its updating MG (Type %u)!", _owner->GetEntry(), curr->GetMovementGeneratorType());
    if (isStatic(curr))
        return;

    _expireList.push_back(curr);
}

bool MotionMaster::GetDestination(float &x, float &y, float &z)
{
    if (_owner->movespline->Finalized())
        return false;

    G3D::Vector3 const& dest = _owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}