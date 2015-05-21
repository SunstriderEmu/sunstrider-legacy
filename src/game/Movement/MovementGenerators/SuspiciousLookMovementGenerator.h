#ifndef TRINITY_SUSPICIOUSLOOKMOVEMENTGENERATOR_H
#define TRINITY_SUSPICIOUSLOOKMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

/** Look towards the target for given time, and restore orientation afterwards. 
    Does not restore orientation on premature ending. */
class SuspiciousLookMovementGenerator : public MovementGenerator
{
    public:
        explicit SuspiciousLookMovementGenerator(Unit const* owner, Unit const* target, uint32 timer);

        void Initialize(Unit*) override;
        void Finalize(Unit*, bool) override;
        void Reset(Unit* owner) override{ Initialize(owner); }
        bool Update(Unit*, uint32) override;
        MovementGeneratorType GetMovementGeneratorType() override { return STEALTH_WARN_MOTION_TYPE; }

    private:
        uint32 m_timer;
        float originalOrientation;
        float targetOrientation;
};

#endif //TRINITY_SUSPICIOUSLOOKMOVEMENTGENERATOR_H