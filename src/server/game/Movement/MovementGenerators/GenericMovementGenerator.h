 
#ifndef TRINITY_GENERICMOVEMENTGENERATOR_H
#define TRINITY_GENERICMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "MoveSplineInit.h"
#include "Timer.h"

class Unit;

enum MovementGeneratorType : uint8;

class GenericMovementGenerator : public MovementGenerator
{
    public:
        explicit GenericMovementGenerator(Movement::MoveSplineInit&& splineInit, MovementGeneratorType type, uint32 id);

        bool Initialize(Unit*) override;
        void Finalize(Unit*, bool active, bool movementInform) override;
        void Reset(Unit*) override;
        bool Update(Unit*, uint32) override;
        void Deactivate(Unit*) override;
        MovementGeneratorType GetMovementGeneratorType() const override { return _type; }

    private:
        void MovementInform(Unit*);

        Movement::MoveSplineInit _splineInit;
        MovementGeneratorType _type;
        uint32 _pointId;
        TimeTrackerSmall _duration;
};

#endif