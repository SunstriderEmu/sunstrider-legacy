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

#include "IdleMovementGenerator.h"
#include "CreatureAI.h"
#include "Creature.h"

IdleMovementGenerator si_idleMovement;

// StopMoving is needed to make unit stop if its last movement generator expires
// But it should not be sent otherwise there are many redundent packets
void IdleMovementGenerator::Initialize(Unit* owner)
{
    Reset(owner);
}

void IdleMovementGenerator::Reset(Unit* owner)
{
    if (!owner->IsStopped())
        owner->StopMoving();
}

void RotateMovementGenerator::Initialize(Unit* owner)
{
    if (!owner->IsStopped())
        owner->StopMoving();

    if (owner->GetVictim())
        owner->SetInFront(owner->GetVictim());

    owner->AddUnitState(UNIT_STATE_ROTATING);

    owner->AttackStop();
}

bool RotateMovementGenerator::Update(Unit* owner, uint32 diff)
{
    float angle = owner->GetOrientation();
    if (m_direction == ROTATE_DIRECTION_LEFT)
    {
        angle += (float)diff * static_cast<float>(M_PI * 2) / m_maxDuration;
        while (angle >= static_cast<float>(M_PI * 2)) angle -= static_cast<float>(M_PI * 2);
    }
    else
    {
        angle -= (float)diff * static_cast<float>(M_PI * 2) / m_maxDuration;
        while (angle < 0) angle += static_cast<float>(M_PI * 2);
    }

    owner->SetFacingTo(angle);

    if (m_duration > diff)
        m_duration -= diff;
    else
        return false;

    return true;
}

void RotateMovementGenerator::Finalize(Unit* unit, bool /* premature */)
{
    unit->ClearUnitState(UNIT_STATE_ROTATING);
    if (unit->GetTypeId() == TYPEID_UNIT)
      unit->ToCreature()->AI()->MovementInform(ROTATE_MOTION_TYPE, 0);
}

DistractMovementGenerator::DistractMovementGenerator(Unit const* owner, float targetOrientation, uint32 timer) : 
    m_timer(timer), 
    originalOrientation(owner->GetOrientation()), 
    targetOrientation(targetOrientation) 
{ }

void DistractMovementGenerator::Initialize(Unit* owner)
{
    owner->AddUnitState(UNIT_STATE_DISTRACTED);
    owner->SetOrientation(targetOrientation);
}

void DistractMovementGenerator::Finalize(Unit* owner, bool premature)
{
    if(!premature)
        owner->SetOrientation(originalOrientation);

    owner->ClearUnitState(UNIT_STATE_DISTRACTED);
}

bool DistractMovementGenerator::Update(Unit* /*owner*/, uint32 time_diff)
{
    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

//SuspiciousLookMovementGenerator

SuspiciousLookMovementGenerator::SuspiciousLookMovementGenerator(Unit const* owner, Unit const* target, uint32 timer) : 
    m_timer(timer), 
    originalOrientation(owner->GetOrientation()), 
    targetOrientation(owner->GetAngle(target)) 
{ }

void SuspiciousLookMovementGenerator::Initialize(Unit* owner)
{
    owner->SetOrientation(targetOrientation);
}

void SuspiciousLookMovementGenerator::Finalize(Unit* owner, bool premature) 
{
    if(!premature)
        owner->SetOrientation(originalOrientation); // "Hmm, must have been the wind"
}

bool SuspiciousLookMovementGenerator::Update(Unit* owner, uint32 time_diff)
{
    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

// AssistanceDistractMovementGenerator

AssistanceDistractMovementGenerator::AssistanceDistractMovementGenerator(uint32 timer) :
    m_timer(timer)
{ }

void AssistanceDistractMovementGenerator::Initialize(Unit* owner)
{
    owner->AddUnitState(UNIT_STATE_DISTRACTED);
}

bool AssistanceDistractMovementGenerator::Update(Unit* /*owner*/, uint32 time_diff)
{
    if (time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

void AssistanceDistractMovementGenerator::Finalize(Unit* unit, bool /* premature */)
{
    unit->ClearUnitState(UNIT_STATE_DISTRACTED);
    unit->ToCreature()->SetReactState(REACT_AGGRESSIVE);
}
