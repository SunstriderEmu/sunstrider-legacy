
#ifndef TRINITY_HOMEMOVEMENTGENERATOR_H
#define TRINITY_HOMEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template <class T>
class HomeMovementGenerator : public MovementGeneratorMedium< T, HomeMovementGenerator<T> >
{
    public:
        explicit HomeMovementGenerator() : _arrived(false), _skipToHome(false) { }
        ~HomeMovementGenerator();

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, const uint32);
        MovementGeneratorType GetMovementGeneratorType() { return HOME_MOTION_TYPE; }

    private:
        void SetTargetLocation(T*);

        bool _arrived;
        bool _skipToHome;
};
#endif
