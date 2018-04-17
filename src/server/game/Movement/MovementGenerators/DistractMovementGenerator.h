#ifndef TRINITY_DISTRACTMOVEMENTGENERATOR_H
#define TRINITY_DISTRACTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class TC_GAME_API DistractMovementGenerator : public MovementGenerator
{
    public:
        explicit DistractMovementGenerator(Unit const* owner, float targetOrientation, uint32 timer);

        bool Initialize(Unit*) override;
        void Finalize(Unit*, bool) override;
        void Reset(Unit* owner) override{ Initialize(owner); }
        bool Update(Unit*, uint32) override;
        MovementGeneratorType GetMovementGeneratorType() const override { return DISTRACT_MOTION_TYPE; }

    private:
        uint32 m_timer;
        float originalOrientation;
        float targetOrientation;
};

class TC_GAME_API AssistanceDistractMovementGenerator : public MovementGenerator
{
    public:
        AssistanceDistractMovementGenerator(uint32 timer);
        
        bool Initialize(Unit*) override;
        void Finalize(Unit*, bool) override;
        void Reset(Unit* owner) override{ Initialize(owner); }
        bool Update(Unit*, uint32) override;
        MovementGeneratorType GetMovementGeneratorType() const override { return ASSISTANCE_DISTRACT_MOTION_TYPE; }
    private:
        uint32 m_timer;
};

#endif //TRINITY_DISTRACTMOVEMENTGENERATOR_H