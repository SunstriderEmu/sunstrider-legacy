
#ifndef TRINITY_CONFUSEDGENERATOR_H
#define TRINITY_CONFUSEDGENERATOR_H

#include "MovementGenerator.h"
#include "Timer.h"

class PathGenerator;

template<class T>
class ConfusedMovementGenerator : public MovementGeneratorMedium< T, ConfusedMovementGenerator<T> >
{
    public:
        explicit ConfusedMovementGenerator() :_nextMoveTime(0), _interrupt(false) { }
        ~ConfusedMovementGenerator();

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);

        MovementGeneratorType GetMovementGeneratorType() const override;
        void UnitSpeedChanged() override { } //TODO

    private:
        std::unique_ptr<PathGenerator> _path;
        bool _interrupt;
        TimeTracker _nextMoveTime;
        Position _reference;
};
#endif
