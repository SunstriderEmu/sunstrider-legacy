
#ifndef TRINITY_ROTATEMOVEMENTGENERATOR_H
#define TRINITY_ROTATEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
enum RotateDirection : uint8;

class RotateMovementGenerator : public MovementGenerator
{
    public:
        explicit RotateMovementGenerator(uint32 id, uint32 time, RotateDirection direction);

        bool Initialize(Unit*) override;
        void Finalize(Unit*, bool, bool) override;
        void Deactivate(Unit*) override;
        void Reset(Unit*) override;
        bool Update(Unit*, uint32) override ;
        MovementGeneratorType GetMovementGeneratorType() const override;

    private:
        uint32 _id, _duration, _maxDuration;
        RotateDirection _direction;
};

#endif //TRINITY_ROTATEMOVEMENTGENERATOR_H