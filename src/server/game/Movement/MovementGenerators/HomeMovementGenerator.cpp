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

#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "WorldPacket.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"

template<class T>
MovementGeneratorType HomeMovementGenerator<T>::GetMovementGeneratorType() const
{
    return HOME_MOTION_TYPE;
}
template MovementGeneratorType HomeMovementGenerator<Creature>::GetMovementGeneratorType() const;

template<>
void HomeMovementGenerator<Creature>::DoFinalize(Creature* owner)
{
    owner->ClearUnitState(UNIT_STATE_EVADE);
    if (_arrived)
    {
        owner->SetWalk(true);
        owner->InitCreatureAddon(true);
        owner->AI()->JustReachedHome();
        owner->SetSpawnHealth();
    }
}

template<>
void HomeMovementGenerator<Creature>::SetTargetLocation(Creature* owner)
{
    if (owner->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        // if we are ROOT/STUNNED/DISTRACTED even after aura clear, finalize on next update - otherwise we would get stuck in evade
        _skipToHome = true;
        return;
    }

    Movement::MoveSplineInit init(owner);
    float x, y, z, o;
    // at apply we can select more nice return points base at current movegen
    if (owner->GetMotionMaster()->empty() || !owner->GetMotionMaster()->top()->GetResetPosition(owner, x, y, z))
    {
        owner->GetHomePosition(x, y, z, o);
        init.SetFacing(o);
    }
    init.MoveTo(x, y, z);
    init.SetWalk(false);
    init.Launch();

    _skipToHome = false;
    _arrived = false;

    owner->ClearUnitState(UNIT_STATE_ALL_ERASABLE & ~UNIT_STATE_EVADE);
}

template<class T>
void HomeMovementGenerator<T>::SetTargetLocation(T*) { }

template<>
bool HomeMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    owner->AddUnitState(UNIT_STATE_EVADE);
    SetTargetLocation(owner);
    return true;
}

template<>
void HomeMovementGenerator<Creature>::DoReset(Creature*) { }


template<class T>
bool HomeMovementGenerator<T>::DoUpdate(T*, uint32)
{
    return false;
}

template<>
bool HomeMovementGenerator<Creature>::DoUpdate(Creature* owner, const uint32 /*time_diff*/)
{
    _arrived = _skipToHome || owner->movespline->Finalized();
    return !_arrived;
}
