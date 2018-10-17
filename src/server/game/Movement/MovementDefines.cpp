/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
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

#include "Common.h"
#include "MovementDefines.h"
#include "ObjectDefines.h"
#include "Position.h"
#include <algorithm>

// ---- ChaseRange ---- //

//sun: Reworked constructors to clarify things: MinRange is defined as target min range, MinTolerance as the threshold for repositioning
ChaseRange::ChaseRange(float range) : MinRange(0.0f), MinTolerance(0.0f - CONTACT_DISTANCE), MaxRange(range + CONTACT_DISTANCE), MaxTolerance(range + 2*CONTACT_DISTANCE) {}
ChaseRange::ChaseRange(float min, float max) : MinRange(min), MinTolerance(std::min(min - CONTACT_DISTANCE, (min + max) / 2)), MaxRange(max), MaxTolerance(std::max(max + CONTACT_DISTANCE, MinTolerance)) {}
ChaseRange::ChaseRange(float min, float tMin, float tMax, float max) : MinRange(min), MinTolerance(tMin), MaxRange(max), MaxTolerance(tMax) {}

// ---- ChaseAngle ---- //

ChaseAngle::ChaseAngle(float angle, float _tolerance/* = M_PI_4*/) : RelativeAngle(Position::NormalizeOrientation(angle)), Tolerance(_tolerance) { }

float ChaseAngle::UpperBound() const
{
    return Position::NormalizeOrientation(RelativeAngle + Tolerance);
}

float ChaseAngle::LowerBound() const
{
    return Position::NormalizeOrientation(RelativeAngle - Tolerance);
}

bool ChaseAngle::IsAngleOkay(float relativeAngle) const
{
    float const diff = std::abs(relativeAngle - RelativeAngle);

    return (std::min(diff, float(2 * M_PI) - diff) <= Tolerance);
}