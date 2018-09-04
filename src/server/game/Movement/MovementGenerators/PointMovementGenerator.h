
#ifndef TRINITY_POINTMOVEMENTGENERATOR_H
#define TRINITY_POINTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class PointMovementGenerator : public MovementGeneratorMedium<T, PointMovementGenerator<T>>
{
    public:
        //_o = 0 means no orientation used. Use values like 0.00001 for orientation 0.
        explicit PointMovementGenerator(uint32 _id, float _x, float _y, float _z, bool _generatePath, float _speed = 0.0f, Optional<float> finalOrient = {}, bool forceDestination = false);

        bool DoInitialize(T*); 
        void DoReset(T*);
        bool DoUpdate(T*, uint32);
        void DoDeactivate(T*);
        void DoFinalize(T*, bool, bool);

        void MovementInform(T*);

        void UnitSpeedChanged() override { PointMovementGenerator<T>::AddFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

        MovementGeneratorType GetMovementGeneratorType() const override;

        void GetDestination(float& x, float& y, float& z) const { x = _destination.GetPositionX(); y = _destination.GetPositionY(); z = _destination.GetPositionZ(); }

    private:
        uint32 _movementId;
        Position _destination;
        float _speed;

        bool _generatePath;
        bool _forceDestination;

        //! if set then unit will turn to specified _orient in provided _pos
        Optional<float> _finalOrient;

        void LaunchMove(T*);
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        explicit AssistanceMovementGenerator(uint32 id, float _x, float _y, float _z) :
            PointMovementGenerator<Creature>(id, _x, _y, _z, true, 0.0f) { }

        void Finalize(Unit*, bool, bool) override;

        MovementGeneratorType GetMovementGeneratorType() const override;
};

#endif
