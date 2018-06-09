#ifndef TRINITY_DISTRACTMOVEMENTGENERATOR_H
#define TRINITY_DISTRACTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class TC_GAME_API DistractMovementGenerator : public MovementGenerator
{
    public:
        explicit DistractMovementGenerator(Unit const* owner, float targetOrientation, uint32 timer);

        bool Initialize(Unit*) override;
        void Finalize(Unit*, bool, bool) override;
        void Deactivate(Unit*) override;
        void Reset(Unit*) override;
        bool Update(Unit*, uint32) override;
        MovementGeneratorType GetMovementGeneratorType() const override;

    private:
        uint32 _timer;
        float originalOrientation;
        float targetOrientation;
};

class TC_GAME_API AssistanceDistractMovementGenerator : public DistractMovementGenerator
{
    public:
        AssistanceDistractMovementGenerator(Unit const* owner, float orientation, uint32 timer);
        
        void Finalize(Unit*, bool, bool) override;

        MovementGeneratorType GetMovementGeneratorType() const override;
};

#endif //TRINITY_DISTRACTMOVEMENTGENERATOR_H