
#ifndef TRINITY_FOLLOWMOVEMENTGENERATOR_H
#define TRINITY_FOLLOWMOVEMENTGENERATOR_H

#include "AbstractFollower.h"
#include "MovementDefines.h"
#include "MovementGenerator.h"
#include "Position.h"

#define FOLLOW_RANGE_TOLERANCE 1.0f

class PathGenerator;
class Unit;

class FollowMovementGenerator : public MovementGenerator, public AbstractFollower
{
    public:
        explicit FollowMovementGenerator(Unit* target, float range, ChaseAngle angle);
        ~FollowMovementGenerator();

        bool Initialize(Unit* owner) override;
        void Reset(Unit* owner) override;
        bool Update(Unit* owner, uint32 diff) override;
        void Deactivate(Unit*) override;
        void Finalize(Unit*, bool, bool) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return FOLLOW_MOTION_TYPE; }

        void UnitSpeedChanged() override { _lastTargetPosition.reset(); }

    private:
        static constexpr uint32 CHECK_INTERVAL = 500;

        void UpdatePetSpeed(Unit* owner);

        float const _range;
        ChaseAngle const _angle;

        uint32 _checkTimer = CHECK_INTERVAL;
        std::unique_ptr<PathGenerator> _path;
        Optional<Position> _lastTargetPosition;
};

#endif
