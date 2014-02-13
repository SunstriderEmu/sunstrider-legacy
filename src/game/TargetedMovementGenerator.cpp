/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "MapManager.h"
#include "DestinationHolderImp.h"
#include "World.h"

#define SMALL_ALPHA 0.05f

#include <cmath>

/*
struct StackCleaner
{
    Creature &i_creature;
    StackCleaner(Creature &creature) : i_creature(creature) {}
    void Done(void) { i_creature.StopMoving(); }
    ~StackCleaner()
    {
        i_creature->Clear();
    }
};
*/

template<class T>
void
TargetedMovementGenerator<T>::_setTargetLocation(T &owner)
{
    if( !i_target.isValid() || !&owner )
        return;

    if( owner.HasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DISTRACTED) )
        return;

    Traveller<T> traveller(owner);
    float travSpeed = traveller.Speed();
    if(!travSpeed)
        return;

    // prevent redundant micro-movement for pets, other followers.
    //if(i_offset && i_target->IsWithinDistInMap(&owner,2*i_offset))
    //    return;

    float x, y, z;
    if(i_onPoint)
    {
        i_target->GetPosition(x,y,z);
    }
    else if(!i_offset && i_target->IsWithinMeleeRange(&owner,6.0f)) //prevent changing target point at every update before we're close enough
    {
        // to nearest random contact position
        i_target->GetRandomContactPoint( &owner, x, y, z, 0, MELEE_RANGE - 0.5f );
    }
    else
    {
        // to at i_offset distance from target and i_angle from target facing
        i_target->GetClosePoint(x,y,z,owner.GetObjectSize(),i_offset,i_angle);
    }

    /*
        We MUST not check the distance difference and avoid setting the new location for smaller distances.
        By that we risk having far too many GetContactPoint() calls freezing the whole system.
        In TargetedMovementGenerator<T>::Update() we check the distance to the target and at
        some range we calculate a new position. The calculation takes some processor cycles due to vmaps.
        If the distance to the target it too large to ignore,
        but the distance to the new contact point is short enough to be ignored,
        we will calculate a new contact point each update loop, but will never move to it.
        The system will freeze.
        ralf

        //We don't update Mob Movement, if the difference between New destination and last destination is < BothObjectSize
        float  bothObjectSize = i_target->GetObjectSize() + owner.GetObjectSize() + CONTACT_DISTANCE;
        if( i_destinationHolder.HasDestination() && i_destinationHolder.GetDestinationDiff(x,y,z) < bothObjectSize )
            return;
    */
    bool forceDest = false;
    // allow pets following their master to cheat while generating paths
    if(owner.GetTypeId() == TYPEID_UNIT && ((Creature*)&owner)->IsPet()
        && owner.HasUnitState(UNIT_STAT_FOLLOW))
        forceDest = true;

    bool newPathCalculated = true;
    if(!i_path)
        i_path = new PathInfo(&owner, x, y, z, false, forceDest);
    else
        newPathCalculated = i_path->Update(x, y, z, false, forceDest);

    //timer.stop();
    //timer.elapsed_microseconds(elapsed);
    //sLog.outDebug("Path found in %llu microseconds", elapsed);

    // nothing we can do here, use direct old school path instead
    if (i_path->getPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE)) {
        //sLog.outError("PATHFIND_NOPATH1 for creature %u (%s)", /*(owner.GetTypeId() == TYPEID_PLAYER) ? */owner.GetGUIDLow(), owner.GetName()/* : owner.GetDBTableGUIDLow()*/);
        //i_forceStraight = true;
        //i_path = new PathInfo(&owner, x, y, z, true, true);
        if (owner.IsPet()) {
            i_path->BuildShortcut();
            owner.addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
            i_path = new PathInfo(&owner, x, y, z, true, true);
        }
    }

    PointPath pointPath = i_path->getFullPath();

    if (i_destinationHolder.HasArrived() && m_pathPointsSent)
        --m_pathPointsSent;
    
    i_path->getNextPosition(x, y, z);
    i_destinationHolder.SetDestination(traveller, x, y, z, false);

    // send the path if:
    //    we have brand new path
    //    we have visited almost all of the previously sent points
    //    movespeed has changed
    //    the owner is stopped (caused by some movement effects)
    if (newPathCalculated || m_pathPointsSent < 2 || i_recalculateTravel || owner.IsStopped())
    {
        // send 10 nodes, or send all nodes if there are less than 10 left
        m_pathPointsSent = std::min<uint32>(10, pointPath.size() - 1);
        uint32 endIndex = m_pathPointsSent + 1;

        // dist to next node + world-unit length of the path
        x -= owner.GetPositionX();
        y -= owner.GetPositionY();
        z -= owner.GetPositionZ();
        float dist = sqrt(x*x + y*y + z*z) + pointPath.GetTotalLength(1, endIndex);

        // calculate travel time, set spline, then send path
        uint32 traveltime = uint32(dist / (travSpeed*0.001f));
        owner.SendMonsterMoveByPath(pointPath, 1, endIndex, traveltime); // TODOMMAPS
    }
    owner.addUnitState(UNIT_STAT_CHASE);
}

template<class T>
void
TargetedMovementGenerator<T>::Initialize(T &owner)
{
    if(!&owner)
        return;
    owner.RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
        
    //i_forceStraight = false;

    _setTargetLocation(owner);
}

template<class T>
void
TargetedMovementGenerator<T>::Finalize(T &owner)
{
    owner.clearUnitState(UNIT_STAT_CHASE);
    owner.clearUnitState(UNIT_STAT_IGNORE_PATHFINDING);
    //i_forceStraight = false;
}

template<class T>
void
TargetedMovementGenerator<T>::Reset(T &owner)
{
    Initialize(owner);
}

template<class T>
bool

TargetedMovementGenerator<T>::Update(T &owner, const uint32 & time_diff)
{
    if(!i_target.isValid())

        return false;

    if( !&owner || !owner.IsAlive())
        return true;

    if( owner.HasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_FLEEING | UNIT_STAT_DISTRACTED) )
        return true;

    Traveller<T> traveller(owner);

    // prevent movement while casting spells with cast time or channel time / or if speed reduced to 0
    if ( owner.IsNonMeleeSpellCasted(false, false, true) || !traveller.Speed())
    {
        if (!owner.IsStopped())
            owner.StopMoving();
        return true;
    }

    // prevent crash after creature killed pet
    if (!owner.HasUnitState(UNIT_STAT_FOLLOW) && owner.GetVictim() != i_target.getTarget())
        return true;
        
    if (i_path && ((i_path->getPathType() & PATHFIND_NOPATH) || (i_path->getPathType() & PATHFIND_INCOMPLETE))) {
        //sLog.outError("PATHFIND_NOPATH2 for creature %u (%s)", /*(owner.GetTypeId() == TYPEID_PLAYER) ? */owner.GetGUIDLow(), owner.GetName()/* : owner.GetDBTableGUIDLow()*/);
        float x, y, z;
        i_target->GetRandomContactPoint( &owner, x, y, z, 0, MELEE_RANGE - 0.5f );
        //i_forceStraight = true;
        //i_path = new PathInfo(&owner, x, y, z, true, true);
        if (owner.IsPet()) {
            owner.addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
            i_path->BuildShortcut();
            i_path = new PathInfo(&owner, x, y, z, true, true);
        }

        //return true;
    }

    if( !i_destinationHolder.HasDestination() )
        _setTargetLocation(owner);

    if (i_destinationHolder.UpdateTraveller(traveller, time_diff, i_recalculateTravel || owner.IsStopped()))
    {
        // put targeted movement generators on a higher priority
        if (owner.GetObjectSize())
            i_destinationHolder.ResetUpdate(100);

        //More distance let have better performance, less distance let have more sensitive reaction at target move.
        float dist = (owner.GetCombatReach() + i_target.getTarget()->GetCombatReach())/2.0f + sWorld.getRate(RATE_TARGET_POS_RECALCULATION_RANGE);
        
        float x,y,z;
        i_target->GetPosition(x, y, z);
        PathNode next_point(x, y, z);

       bool targetMoved = false, needNewDest = false;
        if (i_path)
        {
            PathNode end_point = i_path->getEndPosition();
            next_point = i_path->getNextPosition();

            needNewDest = i_destinationHolder.HasArrived() && !inRange(next_point, i_path->getActualEndPosition(), dist, dist);

            // GetClosePoint() will always return a point on the ground, so we need to
            // handle the difference in elevation when the creature is flying
            if (owner.GetTypeId() == TYPEID_UNIT && ((Creature*)&owner)->canFly())
                targetMoved = i_target->GetDistanceSqr(end_point.x, end_point.y, end_point.z) >= dist*dist;
            else
                targetMoved = i_target->GetDistance2d(end_point.x, end_point.y) >= dist;
        }

        // try to counter precision differences
        //if( i_destinationHolder.GetDistance2dFromDestSq(*i_target.getTarget()) >= dist * dist)
        if ((!i_path || targetMoved || needNewDest || i_recalculateTravel || owner.IsStopped()) && !i_target->IsWithinMeleeRange(&owner))
        {
            // (re)calculate path
            _setTargetLocation(owner);

            // Set new Angle For Map::
            owner.SetOrientation(owner.GetAngle(i_target.getTarget()));
        }
        // Update the Angle of the target only for Map::, no need to send packet for player
        else if (!i_angle && !owner.HasInArc(0.01f, i_target.getTarget()))
            owner.SetOrientation(owner.GetAngle(i_target.getTarget()));

        if(( owner.IsStopped() && !i_destinationHolder.HasArrived() ) || i_recalculateTravel )
        {
            i_recalculateTravel = false;
            //Angle update will take place into owner.StopMoving()
            owner.SetOrientation(owner.GetAngle(i_target.getTarget()));

            owner.StopMoving();
            if(owner.IsWithinMeleeRange(i_target.getTarget()) && !owner.HasUnitState(UNIT_STAT_FOLLOW))
                owner.Attack(i_target.getTarget(),true);
        }
    }
    return true;
}

template<class T>
Unit*
TargetedMovementGenerator<T>::GetTarget() const
{
    return i_target.getTarget();
}

template void TargetedMovementGenerator<Player>::_setTargetLocation(Player &);
template void TargetedMovementGenerator<Creature>::_setTargetLocation(Creature &);
template void TargetedMovementGenerator<Player>::Initialize(Player &);
template void TargetedMovementGenerator<Creature>::Initialize(Creature &);
template void TargetedMovementGenerator<Player>::Finalize(Player &);
template void TargetedMovementGenerator<Creature>::Finalize(Creature &);
template void TargetedMovementGenerator<Player>::Reset(Player &);
template void TargetedMovementGenerator<Creature>::Reset(Creature &);
template bool TargetedMovementGenerator<Player>::Update(Player &, const uint32 &);
template bool TargetedMovementGenerator<Creature>::Update(Creature &, const uint32 &);
template Unit* TargetedMovementGenerator<Player>::GetTarget() const;
template Unit* TargetedMovementGenerator<Creature>::GetTarget() const;

