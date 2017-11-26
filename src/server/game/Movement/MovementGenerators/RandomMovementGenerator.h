
#ifndef TRINITY_RANDOMMOTIONGENERATOR_H
#define TRINITY_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium< T, RandomMovementGenerator<T> >
{
    public:
        RandomMovementGenerator(float spawn_dist = 0.0f) : i_nextMoveTime(0), wander_distance(spawn_dist) { }

        void _setRandomLocation(T*);
        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, const uint32);
        bool GetResetPos(T*, float& x, float& y, float& z);
        MovementGeneratorType GetMovementGeneratorType() { return RANDOM_MOTION_TYPE; }
    private:
        TimeTrackerSmall i_nextMoveTime;

        float wander_distance;
};
#endif
