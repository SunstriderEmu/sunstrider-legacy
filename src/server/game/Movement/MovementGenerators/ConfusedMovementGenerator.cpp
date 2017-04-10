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

#include "Creature.h"
#include "MapManager.h"
#include "ConfusedMovementGenerator.h"
#include "PathGenerator.h"
#include "Management/VMapFactory.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"

template<class T>
bool ConfusedMovementGenerator<T>::DoInitialize(T* owner)
{
    if (!owner || !owner->IsAlive())
        return false;

    owner->AddUnitState(UNIT_STATE_CONFUSED);
    owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    owner->StopMoving();

    _nextMoveTime.Reset(0);
    _reference = owner->GetPosition();

    return true;
}

template<class T>
void ConfusedMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template<class T>
bool ConfusedMovementGenerator<T>::DoUpdate(T* owner, uint32 diff)
{
    if (!owner || !owner->IsAlive())
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        _interrupt = true;
        owner->StopMoving();
        return true;
    }
    else
        _interrupt = false;

    // waiting for next move
    _nextMoveTime.Update(diff);
    if (!_interrupt && _nextMoveTime.Passed() && owner->movespline->Finalized())
    {
        // start moving
        owner->AddUnitState(UNIT_STATE_CONFUSED_MOVE);

        float dest = 4.0f * (float)rand_norm() - 2.0f;

        Position destination;
        destination.Relocate(_reference);
        owner->MovePositionToFirstWalkableCollision(destination, dest, 0.0f);

        if (!_path)
            _path = new PathGenerator(owner);

        _path->SetPathLengthLimit(30.0f);
        bool result = _path->CalculatePath(destination.m_positionX, destination.m_positionY, destination.m_positionZ);
        if (!result || (_path->GetPathType() & PATHFIND_NOPATH))
        {
            _nextMoveTime.Reset(100);
            return true;
        }

        Movement::MoveSplineInit init(owner);
        init.MovebyPath(_path->GetPath());
        init.SetWalk(true);
        int32 traveltime = init.Launch();
        _nextMoveTime.Reset(traveltime + urand(800, 1500));
    }

    return true;
}

template<class T>
void ConfusedMovementGenerator<T>::DoFinalize(T*) { }

template<>
void ConfusedMovementGenerator<Player>::DoFinalize(Player* owner)
{
    owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    owner->ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
    owner->StopMoving();
}

template<>
void ConfusedMovementGenerator<Creature>::DoFinalize(Creature* owner)
{
    owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    owner->ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
    if (owner->GetVictim())
        owner->SetTarget(owner->EnsureVictim()->GetGUID());
}

template bool ConfusedMovementGenerator<Player>::DoInitialize(Player*);
template bool ConfusedMovementGenerator<Creature>::DoInitialize(Creature*);
template void ConfusedMovementGenerator<Player>::DoReset(Player*);
template void ConfusedMovementGenerator<Creature>::DoReset(Creature*);
template bool ConfusedMovementGenerator<Player>::DoUpdate(Player*, uint32 diff);
template bool ConfusedMovementGenerator<Creature>::DoUpdate(Creature*, uint32 diff);
