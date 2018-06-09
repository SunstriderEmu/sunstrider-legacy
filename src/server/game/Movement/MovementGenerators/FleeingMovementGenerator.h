
#ifndef TRINITY_FLEEINGMOVEMENTGENERATOR_H
#define TRINITY_FLEEINGMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "ObjectGuid.h"

template<class T>
class TC_GAME_API FleeingMovementGenerator : public MovementGeneratorMedium< T, FleeingMovementGenerator<T> >
{
    public:
        explicit FleeingMovementGenerator(ObjectGuid fleeTargetGUID);

        bool DoInitialize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);
        void DoDeactivate(T*);
        void DoFinalize(T*, bool, bool);

        MovementGeneratorType GetMovementGeneratorType() const override;
        void UnitSpeedChanged() override { FleeingMovementGenerator<T>::AddFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

    private:
        void SetTargetLocation(T*);
        void GetPoint(T*, Position& position);

        ObjectGuid _fleeTargetGUID;
        TimeTracker i_nextCheckTime;
        std::unique_ptr<PathGenerator> _path;
};

class TC_GAME_API TimedFleeingMovementGenerator : public FleeingMovementGenerator<Creature>
{
    public:
        TimedFleeingMovementGenerator(ObjectGuid fright, uint32 time) :
            FleeingMovementGenerator<Creature>(fright),
            _totalFleeTime(time) { }

        bool Update(Unit*, uint32) override;
        void Finalize(Unit*, bool, bool) override;

        MovementGeneratorType GetMovementGeneratorType() const override;

    private:
        TimeTracker _totalFleeTime;
};

#endif
