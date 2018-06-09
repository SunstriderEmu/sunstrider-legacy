
#ifndef TRINITY_HOMEMOVEMENTGENERATOR_H
#define TRINITY_HOMEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template <class T>
class HomeMovementGenerator : public MovementGeneratorMedium< T, HomeMovementGenerator<T> >
{
    public:
        explicit HomeMovementGenerator();

        bool DoInitialize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, const uint32);
        void DoDeactivate(T*);
        void DoFinalize(T*, bool, bool);

        MovementGeneratorType GetMovementGeneratorType() const override;

    private:
        void SetTargetLocation(T*);
};
#endif
