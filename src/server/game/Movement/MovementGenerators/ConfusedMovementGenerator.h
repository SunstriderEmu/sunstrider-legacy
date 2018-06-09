
#ifndef TRINITY_CONFUSEDGENERATOR_H
#define TRINITY_CONFUSEDGENERATOR_H

#include "MovementGenerator.h"
#include "Timer.h"

class PathGenerator;

template<class T>
class ConfusedMovementGenerator : public MovementGeneratorMedium<T, ConfusedMovementGenerator<T>>
{
    public:
        explicit ConfusedMovementGenerator();

        bool DoInitialize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);
        void DoDeactivate(T*);
        void DoFinalize(T*, bool, bool);

        void UnitSpeedChanged() override { ConfusedMovementGenerator<T>::AddFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

        MovementGeneratorType GetMovementGeneratorType() const override;

    private:
        std::unique_ptr<PathGenerator> _path;
        TimeTracker _nextMoveTime;
        Position _reference;
};
#endif
