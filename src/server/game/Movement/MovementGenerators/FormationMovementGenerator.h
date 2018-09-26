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

#ifndef TRINITY_FORMATIONMOVEMENTGENERATOR_H
#define TRINITY_FORMATIONMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "Position.h"

class Creature;

class FormationMovementGenerator : public MovementGeneratorMedium<Creature, FormationMovementGenerator>
{
    public:
        explicit FormationMovementGenerator(uint32 id, ObjectGuid leaderGUID, FormationMoveSegment moveSegment);

        MovementGeneratorType GetMovementGeneratorType() const override;

        bool DoInitialize(Creature*);
        void DoReset(Creature*);
        bool DoUpdate(Creature*, uint32);
        void DoDeactivate(Creature*);
        void DoFinalize(Creature*, bool, bool);

        ObjectGuid GetLeaderGuid() const { return _leaderGUID; }

        void UnitSpeedChanged() override { AddFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

    private:
        void MovementInform(Creature*);

        void MoveToStart(Creature*);
        void MoveToDest(Creature*);
        Position GetMemberDestination(Creature* member, uint32 followDist, Position leaderDest, uint8 depth = 0) const;

        float GetDistanceFromLine(Position point, Position start, Position end);

        uint32 _movementId;
        FormationMoveSegment _moveSegment;
        ObjectGuid _leaderGUID;
        bool _movingToStart;

        Position _previousHome;
};

#endif // TRINITY_FORMATIONMOVEMENTGENERATOR_H
