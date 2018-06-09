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
#include "MovementDefines.h"


template<class T>
ConfusedMovementGenerator<T>::ConfusedMovementGenerator() :
    MovementGeneratorMedium<T, ConfusedMovementGenerator<T>>(MOTION_MODE_DEFAULT, MOTION_PRIORITY_HIGHEST, UNIT_STATE_CONFUSED),
    _nextMoveTime(0)
{ }

template<class T>
MovementGeneratorType ConfusedMovementGenerator<T>::GetMovementGeneratorType() const
{
    return CONFUSED_MOTION_TYPE;
}

template<class T>
bool ConfusedMovementGenerator<T>::DoInitialize(T* owner)
{
    MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING);
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    if (!owner || !owner->IsAlive())
        return false;

    // TODO: UNIT_FIELD_FLAGS should not be handled by generators
    owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
    owner->StopMoving();

    _nextMoveTime.Reset(0);
    _reference = owner->GetPosition();
    _path = nullptr;

    return true;
}

template<class T>
void ConfusedMovementGenerator<T>::DoReset(T* owner)
{
    MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    DoInitialize(owner);
}

template<class T>
bool ConfusedMovementGenerator<T>::DoUpdate(T* owner, uint32 diff)
{
    if (!owner || !owner->IsAlive())
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);
        owner->StopMoving();
        _path = nullptr;
        return true;
    }
    else
        MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_INTERRUPTED);

    // waiting for next move
    _nextMoveTime.Update(diff);
    if ((MovementGenerator::HasFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING) && !owner->movespline->Finalized()) || (_nextMoveTime.Passed() && owner->movespline->Finalized()))
    {
        // start moving
        MovementGenerator::RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY);

        float dest = 4.0f * (float)rand_norm() - 2.0f;

        Position destination;
        destination.Relocate(_reference);
        owner->MovePositionToFirstWalkableCollision(destination, dest, 0.0f);

        if (!_path)
        {
            _path = std::make_unique<PathGenerator>(owner);
            _path->SetPathLengthLimit(10.0f);
            _path->ExcludeSteepSlopes();
        }

        bool result = _path->CalculatePath(destination.m_positionX, destination.m_positionY, destination.m_positionZ);
        if (!result || (_path->GetPathType() & PATHFIND_NOPATH))
        {
            _nextMoveTime.Reset(100);
            return true;
        }

        owner->AddUnitState(UNIT_STATE_CONFUSED_MOVE);

        Movement::MoveSplineInit init(owner);
        init.MovebyPath(_path->GetPath());
        init.SetWalk(true);
        int32 traveltime = init.Launch();
        _nextMoveTime.Reset(traveltime + urand(800, 1500));
    }

    return true;
}

template<class T>
void ConfusedMovementGenerator<T>::DoDeactivate(T* owner)
{
    MovementGenerator::AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    owner->ClearUnitState(UNIT_STATE_CONFUSED_MOVE);
}

template<class T>
void ConfusedMovementGenerator<T>::DoFinalize(T*, bool, bool) { }

template<>
void ConfusedMovementGenerator<Player>::DoFinalize(Player* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

     if (active)
    {
        owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
        owner->StopMoving();
    }
}

template<>
void ConfusedMovementGenerator<Creature>::DoFinalize(Creature* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);

    if (active)
    {
        owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
        owner->ClearUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_CONFUSED_MOVE);
        if (owner->GetVictim())
            owner->SetTarget(owner->EnsureVictim()->GetGUID());
    }
}

template ConfusedMovementGenerator<Player>::ConfusedMovementGenerator();
template ConfusedMovementGenerator<Creature>::ConfusedMovementGenerator();
template MovementGeneratorType ConfusedMovementGenerator<Player>::GetMovementGeneratorType() const;
template MovementGeneratorType ConfusedMovementGenerator<Creature>::GetMovementGeneratorType() const;
template bool ConfusedMovementGenerator<Player>::DoInitialize(Player*);
template bool ConfusedMovementGenerator<Creature>::DoInitialize(Creature*);
template void ConfusedMovementGenerator<Player>::DoReset(Player*);
template void ConfusedMovementGenerator<Creature>::DoReset(Creature*);
template bool ConfusedMovementGenerator<Player>::DoUpdate(Player*, uint32);
template bool ConfusedMovementGenerator<Creature>::DoUpdate(Creature*, uint32);
template void ConfusedMovementGenerator<Player>::DoDeactivate(Player*);
template void ConfusedMovementGenerator<Creature>::DoDeactivate(Creature*);
