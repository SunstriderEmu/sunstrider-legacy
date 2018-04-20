
#ifndef TRINITY_MOVEMENTGENERATOR_H
#define TRINITY_MOVEMENTGENERATOR_H

#include "Define.h"
#include "SelectableAI.h"
#include "FactoryHolder.h"
#include "ObjectRegistry.h"

class Creature;
class Unit;
enum MovementGeneratorType : uint8;

class MovementGenerator
{
    public:
        virtual ~MovementGenerator();

        virtual bool Initialize(Unit*) = 0;
        //@premature: generator was stopped before expiration
        virtual void Finalize(Unit*, bool premature) = 0;

        virtual void Reset(Unit*) = 0;

        virtual bool Update(Unit*, uint32 time_diff) = 0;

        virtual MovementGeneratorType GetMovementGeneratorType() const = 0;

        virtual uint32 GetSplineId() const { return 0; }  // sunwell: Escort system

        virtual void UnitSpeedChanged() { }
        virtual void Pause(uint32/* timer = 0*/) { } // timer in ms
        virtual void Resume(uint32/* overrideTimer = 0*/) { } // timer in ms

        virtual bool GetResetPosition(Unit*, float&/* x*/, float&/* y*/, float&/* z*/) { return false; } // used by Evade code for select point to evade with expected restart default movement
};

template<class T, class D>
class MovementGeneratorMedium : public MovementGenerator
{
    public:
        bool Initialize(Unit* owner) override
        {
            return (static_cast<D*>(this))->DoInitialize(static_cast<T*>(owner));
        }

        void Finalize(Unit* owner, bool premature = false) override
        {
            (static_cast<D*>(this))->DoFinalize(static_cast<T*>(owner));
        }

        void Reset(Unit* owner) override
        {
            (static_cast<D*>(this))->DoReset(static_cast<T*>(owner));
        }

        bool Update(Unit* owner, uint32 time_diff) override
        {
            return (static_cast<D*>(this))->DoUpdate(static_cast<T*>(owner), time_diff);
        }
};

typedef FactoryHolder<MovementGenerator, Unit, MovementGeneratorType> MovementGeneratorCreator;

template<class Movement>
struct MovementGeneratorFactory : public MovementGeneratorCreator
{
    MovementGeneratorFactory(MovementGeneratorType movementGeneratorType) : MovementGeneratorCreator(movementGeneratorType) { }

    MovementGenerator* Create(Unit* /*object*/) const override
    {
        return new Movement();
    }
};

struct IdleMovementFactory : public MovementGeneratorCreator
{
    IdleMovementFactory();

    MovementGenerator* Create(Unit* object) const override;
};

struct RandomMovementFactory : public MovementGeneratorCreator
{
    RandomMovementFactory();

    MovementGenerator* Create(Unit* object) const override;
};

struct WaypointMovementFactory : public MovementGeneratorCreator
{
    WaypointMovementFactory();
	 
    MovementGenerator* Create(Unit* object) const override;
};

typedef MovementGeneratorCreator::FactoryHolderRegistry MovementGeneratorRegistry;

#define sMovementGeneratorRegistry MovementGeneratorRegistry::instance()

#endif
