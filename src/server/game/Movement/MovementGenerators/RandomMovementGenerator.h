
#ifndef TRINITY_RANDOMMOTIONGENERATOR_H
#define TRINITY_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium< T, RandomMovementGenerator<T> >
{
    public:
        RandomMovementGenerator(float spawn_dist = 0.0f);

        void SetRandomLocation(T*);
        bool DoInitialize(T*);
        void DoFinalize(T*, bool, bool);
        void DoReset(T*);
        void DoDeactivate(T*);
        bool DoUpdate(T*, const uint32);
        MovementGeneratorType GetMovementGeneratorType() const override;
    private:
        TimeTrackerSmall _timer;
        Position _reference;
        std::unique_ptr<PathGenerator> _path;

        float _wanderDistance;
};
#endif
