
#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"
#include "Transport.h"

template<class T, typename D>
void TargetedMovementGenerator<T, D>::SetOffset(float offset)
{
    this->_offset = offset;
}

template<class T, typename D>
void TargetedMovementGenerator<T, D>::SetTargetLocation(T* owner)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE))
        return;

    if (owner->GetTypeId() == TYPEID_UNIT && !i_target->isInAccessiblePlaceFor(owner->ToCreature()))
        return;

    float x, y, z;
    bool transportImplied = owner->GetTransport() || i_target->GetTransport();

    lastOwnerXYZ.Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ());
    lastTargetXYZ.Relocate(i_target->GetPositionX(), i_target->GetPositionY(), i_target->GetPositionZ());

    if (!_offset)
    {
        // to nearest contact position
        i_target->GetContactPoint(owner, x, y, z);
        if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
        {
            //kelno: for chase movement, check if target point is in LoS with target. If not, force moving on target instead so that we can still melee it.
            if (!i_target->IsWithinLOS(x, y, z))
                i_target->GetPosition(x, y, z);
        }
    }
    else
        // to at _offset distance from target and _angle from target facing
        i_target->GetClosePoint(x, y, z, owner->GetCombatReach(), _offset, _angle);

    if (!_path)
        _path = new PathGenerator(owner);
    else
        _path->UpdateOptions();

    bool isPlayerPet = owner->IsPet() && IS_PLAYER_GUID(owner->GetOwnerGUID());
#ifdef LICH_KING
    if (owner->GetMapId() == 631 && owner->GetTransport() && owner->GetTransport()->IsMotionTransport() && i_target->GetTransport() && i_target->GetTransport()->IsMotionTransport()) // for ICC, if both on a motion transport => don't use mmaps
        sameTransport = owner->GetTypeId() == TYPEID_UNIT && i_target->isInAccessiblePlaceFor(owner->ToCreature());
#endif
    bool forceDest = 
           (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->IsPet() && owner->HasUnitState(UNIT_STATE_FOLLOW)) // allow pets to use shortcut if no path found when following their master
        || (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->IsWorldBoss()) /*|| ((Creature*)owner)->IsDungeonBoss()*/ // force for all bosses, even not in instances
        || transportImplied // transports at dock aren't handled by mmaps, this should help
        ///|| (owner->FindMap() && owner->FindMap()->IsDungeon() && !isPlayerPet) // force in instances to prevent exploiting //disabled, evade mode when target is not accessible should help with this in a better way. Maybe disable for bosses too ?
        ;

    bool result = _path->CalculatePath(x, y, z, forceDest);
    if (!result || (_path->GetPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE)))
    {
        // Cant reach target, try again at next update
        _recalculateTravel = true;
        return;
    }

    D::_addUnitStateMove(owner);
    _targetReached = false;
    _recalculateTravel = false;
    _speedChanged = false;
    owner->AddUnitState(UNIT_STATE_CHASE);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(_path->GetPath());
    init.SetWalk(((D*)this)->EnableWalking());
    // Using the same condition for facing target as the one that is used for SetInFront on movement end
    // - applies to ChaseMovementGenerator mostly
    if (_angle == 0.f)
        init.SetFacing(i_target.getTarget());

    // use player orientation if player pet
    if (owner->HasUnitState(UNIT_STATE_FOLLOW) && isPlayerPet)
        if (Player* p = owner->GetMap()->GetPlayer(owner->GetOwnerGUID()))
            if(!p->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD)) //don't do it if player is currently going backwards, as this is visually ugly
                init.SetFacing(p->GetOrientation());

    init.Launch();
}

template<class T, typename D>
bool TargetedMovementGenerator<T, D>::IsWithinAllowedDist(T* owner, float x, float y, float z)
{
    float allowedDist = GetAllowedDist(owner);

    if (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->CanFly())
        return i_target->IsWithinDist3d(x, y, z, allowedDist);
    else
        return i_target->IsWithinDist2d(x, y, allowedDist);
}

template<class T, typename D>
float TargetedMovementGenerator<T, D>::GetAllowedDist(T* owner)
{
    //More distance let have better performance, less distance let have more sensitive reaction at target move.
    float combatReach = owner->GetCombatReach();
    if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
        combatReach /= 2; //Divide CombatReach by 2.0f, chasing creature relocation should be triggered long before being at max range

    return combatReach + sWorld->GetRate(RATE_TARGET_POS_RECALCULATION_RANGE) + _offset;
}

template<class T, typename D>
bool TargetedMovementGenerator<T, D>::DoUpdate(T* owner, uint32 time_diff)
{
    if (!i_target.isValid() || !i_target->IsInWorld() || i_target->GetMap() != owner->GetMap())
        return false;

    if (!owner || !owner->IsAlive())
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || HasLostTarget(owner))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    // prevent movement while casting spells with cast time or channel time
    if (owner->IsMovementPreventedByCasting() )
    {
        if (!owner->IsStopped())
            owner->StopMoving();
        return true;
    }

    if (owner->GetTypeId() == TYPEID_UNIT)
    {
        if (_path && _path->GetPathType() & (PATHFIND_INCOMPLETE | PATHFIND_NOPATH))
        {
            owner->ToCreature()->SetCannotReachTarget(true);
        }
        else {
            owner->ToCreature()->SetCannotReachTarget(false);
        }
    }

    _timer.Update(time_diff);
    if (!_recalculateTravel && _timer.Passed())
    {
        _timer.Reset(100);
        G3D::Vector3 dest = owner->movespline->FinalDestination();

        if (owner->movespline->onTransport)
            if (TransportBase* transport = owner->GetTransport())
                transport->CalculatePassengerPosition(dest.x, dest.y, dest.z);
        
        _recalculateTravel = !IsWithinAllowedDist(owner, dest.x, dest.y, dest.z);
        // then, if the target is in range, check also Line of Sight. Consider target has moved if out of sight.
        if (!_recalculateTravel)
            _recalculateTravel = !i_target->IsWithinLOSInMap(owner, VMAP::ModelIgnoreFlags::M2);
    }

    bool someoneMoved = (owner->GetExactDistSq(&lastOwnerXYZ) >= 0.1f*0.1f) || (i_target->GetExactDistSq(&lastTargetXYZ) >= 0.1f*0.1f);
    if (_speedChanged || (_recalculateTravel && someoneMoved))
        SetTargetLocation(owner);

    if (owner->movespline->Finalized())
    {
        static_cast<D*>(this)->MovementInform(owner);
        if (_angle == 0.f && !owner->HasInArc(0.01f, i_target.getTarget()))
            owner->SetInFront(i_target.getTarget());

        if (!_targetReached)
        {
            _targetReached = true;
            static_cast<D*>(this)->_reachTarget(owner);
        }
    }

    return true;
}

//-----------------------------------------------//
template<class T>
void ChaseMovementGenerator<T>::_reachTarget(T* owner)
{
    if (owner->IsWithinMeleeRange(this->i_target.getTarget()))
        owner->Attack(this->i_target.getTarget(), true);
    if (owner->GetTypeId() == TYPEID_UNIT)
        owner->ToCreature()->SetCannotReachTarget(false);
}

template<>
bool ChaseMovementGenerator<Player>::DoInitialize(Player* owner)
{
    owner->AddUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    SetTargetLocation(owner);
    return true;
}

template<>
bool ChaseMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    _restoreWalking = owner->IsWalking();
    owner->SetWalk(false);
    owner->AddUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    SetTargetLocation(owner);
    return true;
}

template<class T>
void ChaseMovementGenerator<T>::DoFinalize(T* owner)
{
    owner->ClearUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    if(Creature* creature = owner->ToCreature())
        creature->SetCannotReachTarget(false);
    if (_restoreWalking)
        owner->SetWalk(true);
}

template<class T>
void ChaseMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template<class T>
void ChaseMovementGenerator<T>::MovementInform(T* /*unit*/) { }

template<>
void ChaseMovementGenerator<Creature>::MovementInform(Creature* unit)
{
    // Pass back the GUIDLow of the target. If it is pet's owner then PetAI will handle
    if (unit->AI())
        unit->AI()->MovementInform(CHASE_MOTION_TYPE, i_target.getTarget()->GetGUIDLow());
}

template<class T>
bool ChaseMovementGenerator<T>::HasLostTarget(T* owner) const
{
    return owner->GetVictim() != TargetedMovementGeneratorBase::GetTarget();
}

//-----------------------------------------------//
template<>
bool FollowMovementGenerator<Creature>::EnableWalking() const
{
    return i_target.isValid() && i_target->IsWalking();
}

template<>
bool FollowMovementGenerator<Player>::EnableWalking() const
{
    return false;
}

template<>
void FollowMovementGenerator<Player>::_updateSpeed(Player* /*owner*/)
{
    // nothing to do for Player
}

template<>
void FollowMovementGenerator<Creature>::_updateSpeed(Creature* owner)
{
    // pet only sync speed with owner
    /// Make sure we are not in the process of a map change (IsInWorld)
    if (!owner->IsPet() || !owner->IsInWorld() || !i_target.isValid() || i_target->GetGUID() != owner->GetOwnerGUID())
        return;

    owner->UpdateSpeed(MOVE_RUN);
    owner->UpdateSpeed(MOVE_WALK);
    owner->UpdateSpeed(MOVE_SWIM);
}

template<>
bool FollowMovementGenerator<Player>::DoInitialize(Player* owner)
{
    owner->AddUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
    _updateSpeed(owner);
    SetTargetLocation(owner);
    return true;
}

template<>
bool FollowMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    owner->AddUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
    _updateSpeed(owner);
    SetTargetLocation(owner);
    return true;
}

template<class T>
void FollowMovementGenerator<T>::DoFinalize(T* owner)
{
    owner->ClearUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
    _updateSpeed(owner);
}

template<class T>
void FollowMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template<class T>
void FollowMovementGenerator<T>::MovementInform(T* /*unit*/) { }

template<>
void FollowMovementGenerator<Creature>::MovementInform(Creature* unit)
{
    // Pass back the GUIDLow of the target. If it is pet's owner then PetAI will handle
    if (unit->AI())
        unit->AI()->MovementInform(FOLLOW_MOTION_TYPE, i_target.getTarget()->GetGUIDLow());
}

//-----------------------------------------------//
template void TargetedMovementGenerator<Player, ChaseMovementGenerator<Player> >::SetTargetLocation(Player*);
template void TargetedMovementGenerator<Player, FollowMovementGenerator<Player> >::SetTargetLocation(Player*);
template void TargetedMovementGenerator<Creature, ChaseMovementGenerator<Creature> >::SetTargetLocation(Creature*);
template void TargetedMovementGenerator<Creature, FollowMovementGenerator<Creature> >::SetTargetLocation(Creature*);
template bool TargetedMovementGenerator<Player, ChaseMovementGenerator<Player> >::DoUpdate(Player*, uint32);
template bool TargetedMovementGenerator<Player, FollowMovementGenerator<Player> >::DoUpdate(Player*, uint32);
template bool TargetedMovementGenerator<Creature, ChaseMovementGenerator<Creature> >::DoUpdate(Creature*, uint32);
template bool TargetedMovementGenerator<Creature, FollowMovementGenerator<Creature> >::DoUpdate(Creature*, uint32);

template void ChaseMovementGenerator<Player>::_reachTarget(Player*);
template void ChaseMovementGenerator<Creature>::_reachTarget(Creature*);
template void ChaseMovementGenerator<Player>::DoFinalize(Player*);
template void ChaseMovementGenerator<Creature>::DoFinalize(Creature*);
template void ChaseMovementGenerator<Player>::DoReset(Player*);
template void ChaseMovementGenerator<Creature>::DoReset(Creature*);
template void ChaseMovementGenerator<Player>::MovementInform(Player*);
template bool ChaseMovementGenerator<Player>::HasLostTarget(Player*) const;
template bool ChaseMovementGenerator<Creature>::HasLostTarget(Creature*) const;

template void FollowMovementGenerator<Player>::DoFinalize(Player*);
template void FollowMovementGenerator<Creature>::DoFinalize(Creature*);
template void FollowMovementGenerator<Player>::DoReset(Player*);
template void FollowMovementGenerator<Creature>::DoReset(Creature*);
template void FollowMovementGenerator<Player>::MovementInform(Player*);
