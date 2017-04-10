
#ifndef TRINITY_MOVEMENTGENERATOR_H
#define TRINITY_MOVEMENTGENERATOR_H

#include "Define.h"
#include "Dynamic/FactoryHolder.h"
#include "MotionMaster.h"

class Unit;

class MovementGenerator
{
    public:
        virtual ~MovementGenerator();

        virtual bool Initialize(Unit*) = 0;
        //@premature: generator was stopped before expiration
        virtual void Finalize(Unit*, bool premature) = 0;

        virtual void Reset(Unit*) = 0;

        virtual bool Update(Unit*, uint32 time_diff) = 0;

        virtual MovementGeneratorType GetMovementGeneratorType() = 0;

        virtual uint32 GetSplineId() const { return 0; }  // Xinef: Escort system

        virtual void UnitSpeedChanged() { }

        // used by Evade code for select point to evade with expected restart default movement
        virtual bool GetResetPosition(Unit*, float& /*x*/, float& /*y*/, float& /*z*/) { return false; }
};

template<class T, class D>
class MovementGeneratorMedium : public MovementGenerator
{
    public:
        bool Initialize(Unit* u) override
        {
            //u->AssertIsType<T>();
            return (static_cast<D*>(this))->DoInitialize(static_cast<T*>(u));
        }

        void Finalize(Unit* u, bool premature = false) override
        {
            //u->AssertIsType<T>();
            (static_cast<D*>(this))->DoFinalize(static_cast<T*>(u));
        }

        void Reset(Unit* u) override
        {
            //u->AssertIsType<T>();
            (static_cast<D*>(this))->DoReset(static_cast<T*>(u));
        }

        bool Update(Unit* u, uint32 time_diff) override
        {
            //u->AssertIsType<T>();
            return (static_cast<D*>(this))->DoUpdate(static_cast<T*>(u), time_diff);
        }
};

struct SelectableMovement : public FactoryHolder<MovementGenerator, MovementGeneratorType>
{
    SelectableMovement(MovementGeneratorType movementGeneratorType) : FactoryHolder<MovementGenerator, MovementGeneratorType>(movementGeneratorType) { }
};

template<class REAL_MOVEMENT>
struct MovementGeneratorFactory : public SelectableMovement
{
    MovementGeneratorFactory(MovementGeneratorType movementGeneratorType) : SelectableMovement(movementGeneratorType) { }

    MovementGenerator* Create(void *) const override;
};

typedef FactoryHolder<MovementGenerator, MovementGeneratorType> MovementGeneratorCreator;
typedef FactoryHolder<MovementGenerator, MovementGeneratorType>::FactoryHolderRegistry MovementGeneratorRegistry;

#endif
