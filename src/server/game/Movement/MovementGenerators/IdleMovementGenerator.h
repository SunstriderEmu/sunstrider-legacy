
#ifndef TRINITY_IDLEMOVEMENTGENERATOR_H
#define TRINITY_IDLEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class TC_GAME_API IdleMovementGenerator : public MovementGenerator
{
    public:
        explicit IdleMovementGenerator();

        bool Initialize(Unit*) override;
        void Reset(Unit*) override;
        bool Update(Unit*, uint32) override { return true; }
        void Deactivate(Unit*) override;
        void Finalize(Unit*, bool, bool) override;

        MovementGeneratorType GetMovementGeneratorType() const override;
};

#endif //TRINITY_IDLEMOVEMENTGENERATOR_H