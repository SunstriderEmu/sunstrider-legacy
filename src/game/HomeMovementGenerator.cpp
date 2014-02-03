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

#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "Traveller.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "DestinationHolderImp.h"
#include "WorldPacket.h"
#include "CreatureAINew.h"

void
HomeMovementGenerator<Creature>::Initialize(Creature & owner)
{
    owner.RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
    _setTargetLocation(owner);
}

void
HomeMovementGenerator<Creature>::Finalize(Creature & owner)
{
    if (owner.GetTypeId() == TYPEID_UNIT) {
        owner.InitCreatureAddon(true);
        if (owner.IsAIEnabled) {
            owner.AI()->JustReachedHome();
            if (owner.getAI())
                owner.getAI()->onReachedHome();
        }
    }
}

void
HomeMovementGenerator<Creature>::Reset(Creature &)
{
}

void
HomeMovementGenerator<Creature>::_setTargetLocation(Creature & owner)
{
    if( !&owner )
        return;

    if( owner.HasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DISTRACTED) )
        return;

    float x, y, z;
    owner.GetHomePosition(x, y, z, ori);

    CreatureTraveller traveller(owner);
    i_destinationHolder.SetDestination(traveller, x, y, z, false);

    PathInfo path(&owner, x, y, z);
    PointPath pointPath = path.getFullPath();

    float speed = traveller.Speed() * 0.001f; // in ms
    uint32 traveltime = uint32(pointPath.GetTotalLength() / speed);
    modifyTravelTime(traveltime);

    owner.SendMonsterMoveByPath(pointPath, 1, pointPath.size(), ((SplineFlags)owner.GetUnitMovementFlags()), traveltime);    // TODOMMAPS
    owner.clearUnitState(UNIT_STAT_ALL_STATE);
}

bool
HomeMovementGenerator<Creature>::Update(Creature &owner, const uint32& time_diff)
{
    CreatureTraveller traveller( owner);
    
    i_destinationHolder.UpdateTraveller(traveller, time_diff, false);
    /*if (i_destinationHolder.UpdateTraveller(traveller, time_diff, false))
    {
        if (!owner.IsActive(owner))                               // force stop processing (movement can move out active zone with cleanup movegens list)
            return true;                                    // not expire now, but already lost
    }*/

    if (time_diff > i_travel_time)
    {
        owner.AddUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);

        // restore orientation of not moving creature at returning to home
        if(owner.GetDefaultMovementType()==IDLE_MOTION_TYPE)
        {
            owner.SetOrientation(ori);
            WorldPacket packet;
            owner.BuildHeartBeatMsg(&packet);
            owner.SendMessageToSet(&packet, false);
        }
        return false;
    }

    i_travel_time -= time_diff;

    return true;
}

