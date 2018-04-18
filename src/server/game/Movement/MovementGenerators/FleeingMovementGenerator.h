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

#ifndef TRINITY_FLEEINGMOVEMENTGENERATOR_H
#define TRINITY_FLEEINGMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class TC_GAME_API FleeingMovementGenerator : public MovementGeneratorMedium< T, FleeingMovementGenerator<T> >
{
    public:
        FleeingMovementGenerator(ObjectGuid fleeTargetGUID) : _path(nullptr), _fleeTargetGUID(fleeTargetGUID), i_nextCheckTime(0) { }
		~FleeingMovementGenerator()
		{
			delete _path;
		}

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);

        MovementGeneratorType GetMovementGeneratorType() const override { return FLEEING_MOTION_TYPE; }

    private:
        void SetTargetLocation(T*);
        void GetPoint(T*, Position &position);

        ObjectGuid _fleeTargetGUID;
        TimeTracker i_nextCheckTime;
        PathGenerator* _path;
        bool _interrupt;
};

class TC_GAME_API TimedFleeingMovementGenerator : public FleeingMovementGenerator<Creature>
{
    public:
        TimedFleeingMovementGenerator(ObjectGuid fright, uint32 time) :
            FleeingMovementGenerator<Creature>(fright),
            _totalFleeTime(time) { }

        MovementGeneratorType GetMovementGeneratorType() const override { return TIMED_FLEEING_MOTION_TYPE; }
        bool Update(Unit*, uint32) override;
        void Finalize(Unit*, bool) override;

    private:
        TimeTracker _totalFleeTime;
};

#endif
