/*
Written by Xinef
 */

#ifndef TRINITY_ESCORTMOVEMENTGENERATOR_H
#define TRINITY_ESCORTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class EscortMovementGenerator : public MovementGeneratorMedium< T, EscortMovementGenerator<T> >
{
    public:
        EscortMovementGenerator(Movement::PointsArray* _path = NULL) : i_recalculateSpeed(false)
        {
            if (_path)
                m_precomputedPath = *_path;
        }

        void DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);

        void unitSpeedChanged() { i_recalculateSpeed = true; }

        MovementGeneratorType GetMovementGeneratorType() override { return ESCORT_MOTION_TYPE; }

		uint32 GetSplineId() const override { return _splineId; }
        //Update pos to position we're actually going to. Return false if we're not actually going somewhere
        bool GetCurrentDesinationPoint(T* unit, Position& pos)
        {
            uint32 splineIdx = unit->movespline->_currentSplineIdx();
            if (m_precomputedPath.size() > splineIdx)
            {
                pos.Relocate(m_precomputedPath[splineIdx].x, m_precomputedPath[splineIdx].y, m_precomputedPath[splineIdx].z);
                return true;
            }
            return false;
        }

    private:
        bool i_recalculateSpeed;
        Movement::PointsArray m_precomputedPath;

		uint32 _splineId;
};

#endif

