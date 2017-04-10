
#ifndef TRINITY_CONFUSEDGENERATOR_H
#define TRINITY_CONFUSEDGENERATOR_H

#include "MovementGenerator.h"
#include "Timer.h"

template<class T>
class ConfusedMovementGenerator : public MovementGeneratorMedium< T, ConfusedMovementGenerator<T> >
{
    public:
        explicit ConfusedMovementGenerator() : _path(nullptr), _nextMoveTime(0), _interrupt(false){ }
        ~ConfusedMovementGenerator()
        {
            delete _path;
        }

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);

        MovementGeneratorType GetMovementGeneratorType() { return CONFUSED_MOTION_TYPE; }
    private:
        PathGenerator* _path;
        bool _interrupt;
        TimeTracker _nextMoveTime;
        Position _reference;
};
#endif
