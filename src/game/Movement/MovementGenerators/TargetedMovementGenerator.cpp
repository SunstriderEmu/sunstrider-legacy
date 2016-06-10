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
void TargetedMovementGeneratorMedium<T, D>::SetOffset(float offset)
{
    this->i_offset = offset;
}

template<class T, typename D>
void TargetedMovementGeneratorMedium<T, D>::_setTargetLocation(T* owner)
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

    if (!i_offset)
    {
        // to nearest contact position
		i_target->GetContactPoint(owner, x, y, z);

        //kelno: for chase movement, check if target point is in LoS with target. If not, force moving on target instead so that we can still melee it.
        if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE && !i_target->IsWithinLOS(x, y, z))
            i_target->GetPosition(x, y, z);
    }
    else
        // to at i_offset distance from target and i_angle from target facing
		i_target->GetClosePoint(x, y, z, owner->GetObjectSize(), i_offset, i_angle);

    if (!i_path)
        i_path = new PathGenerator(owner);
    else
        i_path->UpdateOptions();

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

    bool result = i_path->CalculatePath(x, y, z, forceDest);
    if (!result || (i_path->GetPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE)))
    {
        // Cant reach target, try again at next update
		i_recalculatePath = true;
        return;
    }

    D::_addUnitStateMove(owner);
    i_targetReached = false;
	i_recalculatePath = false;
	i_speedChanged = false;
    owner->AddUnitState(UNIT_STATE_CHASE);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(i_path->GetPath());
    init.SetWalk(((D*)this)->EnableWalking());
    // Using the same condition for facing target as the one that is used for SetInFront on movement end
    // - applies to ChaseMovementGenerator mostly
    if (i_angle == 0.f)
        init.SetFacing(i_target.getTarget());

    // use player orientation if player pet
    if (owner->HasUnitState(UNIT_STATE_FOLLOW) && isPlayerPet)
        if (Player* p = owner->GetMap()->GetPlayer(owner->GetOwnerGUID()))
			if(!p->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD)) //don't do it if player is currently going backwards, as this is visually ugly
				init.SetFacing(p->GetOrientation());


    init.Launch();
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::IsWithinAllowedDist(T* owner, float x, float y, float z)
{
    float allowedDist = GetAllowedDist(owner);

    if (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->CanFly())
        return i_target->IsWithinDist3d(x, y, z, allowedDist);
    else
        return i_target->IsWithinDist2d(x, y, allowedDist);
}

template<class T, typename D>
float TargetedMovementGeneratorMedium<T, D>::GetAllowedDist(T* owner)
{
	//More distance let have better performance, less distance let have more sensitive reaction at target move.
	float combatReach = owner->GetCombatReach();
	if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
		combatReach /= 2; //Divide CombatReach by 2.0f, chasing creature relocation should be triggered long before being at max range

	return combatReach + sWorld->GetRate(RATE_TARGET_POS_RECALCULATION_RANGE) + i_offset;
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::DoUpdate(T* owner, uint32 time_diff)
{
    if (!i_target.isValid() || !i_target->IsInWorld() || i_target->GetMap() != owner->GetMap())
        return false;

    if (!owner || !owner->IsAlive())
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    if (owner->GetTypeId() == TYPEID_UNIT)
    {
        if (i_path && i_path->GetPathType() & (PATHFIND_INCOMPLETE | PATHFIND_NOPATH))
        {
            owner->ToCreature()->SetCannotReachTarget(true);
        }
        else {
            owner->ToCreature()->SetCannotReachTarget(false);
        }
    }

    // prevent movement while casting spells with cast time or channel time
    if (owner->HasUnitState(UNIT_STATE_CASTING))
    {
        if (!owner->IsStopped())
            owner->StopMoving();
        return true;
    }

    // prevent crash after creature killed pet
    if (static_cast<D*>(this)->_lostTarget(owner))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    i_recheckDistance.Update(time_diff);
    if (!i_recalculatePath && i_recheckDistance.Passed())
    {
        i_recheckDistance.Reset(100);
        G3D::Vector3 dest = owner->movespline->FinalDestination();

        if (owner->movespline->onTransport)
            if (TransportBase* transport = owner->GetTransport())
                transport->CalculatePassengerPosition(dest.x, dest.y, dest.z);
        
		i_recalculatePath = !IsWithinAllowedDist(owner, dest.x, dest.y, dest.z);
        // then, if the target is in range, check also Line of Sight. Consider target has moved if out of sight.
        if (!i_recalculatePath)
			i_recalculatePath = !i_target->IsWithinLOSInMap(owner);
    }

	bool someoneMoved = (owner->GetExactDistSq(&lastOwnerXYZ) >= 0.1f*0.1f) || (i_target->GetExactDistSq(&lastTargetXYZ) >= 0.1f*0.1f);
	if (i_speedChanged || (i_recalculatePath && someoneMoved))
        _setTargetLocation(owner);

    if (owner->movespline->Finalized())
    {
        static_cast<D*>(this)->MovementInform(owner);
        if (i_angle == 0.f && !owner->HasInArc(0.01f, i_target.getTarget()))
            owner->SetInFront(i_target.getTarget());

        if (!i_targetReached)
        {
            i_targetReached = true;
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
    _setTargetLocation(owner);
    return true;
}

template<>
bool ChaseMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    owner->SetWalk(false);
    owner->AddUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    _setTargetLocation(owner);
    return true;
}

template<class T>
void ChaseMovementGenerator<T>::DoFinalize(T* owner)
{
    owner->ClearUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    if(Creature* creature = owner->ToCreature())
        creature->SetCannotReachTarget(false);
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
    _setTargetLocation(owner);
    return true;
}

template<>
bool FollowMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    owner->AddUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
    _updateSpeed(owner);
    _setTargetLocation(owner);
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
template void TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::_setTargetLocation(Player*);
template void TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::_setTargetLocation(Player*);
template void TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::_setTargetLocation(Creature*);
template void TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::_setTargetLocation(Creature*);
template bool TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::DoUpdate(Player*, uint32);
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::DoUpdate(Player*, uint32);
template bool TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::DoUpdate(Creature*, uint32);
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::DoUpdate(Creature*, uint32);

template void ChaseMovementGenerator<Player>::_reachTarget(Player*);
template void ChaseMovementGenerator<Creature>::_reachTarget(Creature*);
template void ChaseMovementGenerator<Player>::DoFinalize(Player*);
template void ChaseMovementGenerator<Creature>::DoFinalize(Creature*);
template void ChaseMovementGenerator<Player>::DoReset(Player*);
template void ChaseMovementGenerator<Creature>::DoReset(Creature*);
template void ChaseMovementGenerator<Player>::MovementInform(Player*);

template void FollowMovementGenerator<Player>::DoFinalize(Player*);
template void FollowMovementGenerator<Creature>::DoFinalize(Creature*);
template void FollowMovementGenerator<Player>::DoReset(Player*);
template void FollowMovementGenerator<Creature>::DoReset(Creature*);
template void FollowMovementGenerator<Player>::MovementInform(Player*);
