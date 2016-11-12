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

#ifndef TRINITY_ROTATEMOVEMENTGENERATOR_H
#define TRINITY_ROTATEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class RotateMovementGenerator : public MovementGenerator
{
    public:
        explicit RotateMovementGenerator(uint32 time, RotateDirection direction) : m_duration(time), m_maxDuration(time), m_direction(direction) { }

        bool Initialize(Unit*) override;
        void Finalize(Unit*, bool) override;
        void Reset(Unit* owner) override { Initialize(owner); }
        bool Update(Unit*, uint32) override ;
        MovementGeneratorType GetMovementGeneratorType() override { return ROTATE_MOTION_TYPE; }

    private:
        uint32 m_duration, m_maxDuration;
        RotateDirection m_direction;
};

#endif //TRINITY_ROTATEMOVEMENTGENERATOR_H