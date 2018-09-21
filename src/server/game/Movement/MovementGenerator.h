
#ifndef TRINITY_MOVEMENTGENERATOR_H
#define TRINITY_MOVEMENTGENERATOR_H

#include "Define.h"
#include "SelectableAI.h"
#include "FactoryHolder.h"
#include "ObjectRegistry.h"

class Creature;
class Unit;
enum MovementGeneratorType : uint8;

enum MovementGeneratorFlags : uint16
{
    MOVEMENTGENERATOR_FLAG_NONE                   = 0x000,
    MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING = 0x001,
    MOVEMENTGENERATOR_FLAG_INITIALIZED            = 0x002,
    MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING   = 0x004,
    MOVEMENTGENERATOR_FLAG_INTERRUPTED            = 0x008,
    MOVEMENTGENERATOR_FLAG_PAUSED                 = 0x010,
    MOVEMENTGENERATOR_FLAG_TIMED_PAUSED           = 0x020,
    MOVEMENTGENERATOR_FLAG_DEACTIVATED            = 0x040,
    MOVEMENTGENERATOR_FLAG_INFORM_ENABLED         = 0x080,
    MOVEMENTGENERATOR_FLAG_FINALIZED              = 0x100,

    MOVEMENTGENERATOR_FLAG_TRANSITORY = MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING | MOVEMENTGENERATOR_FLAG_INTERRUPTED
};

class MovementGenerator
{
    public:
        MovementGenerator(uint8 mode, uint8 priority, uint32 baseUnitState) :
            Mode(mode),
            Priority(priority),
            BaseUnitState(baseUnitState)
        {}

        virtual ~MovementGenerator();

        // on top first update
        virtual bool Initialize(Unit*) = 0;
        //@premature: generator was stopped before expiration
        /*virtual void Finalize(Unit*, bool premature) = 0;*/ //can be replaced by Deactivate?
        // on movement delete
        virtual void Finalize(Unit*, bool, bool) = 0;
        // on top reassign
        virtual void Reset(Unit*) = 0;
        // on current top if another movement replaces
        virtual void Deactivate(Unit*) = 0;

        virtual bool Update(Unit*, uint32 time_diff) = 0;

        virtual MovementGeneratorType GetMovementGeneratorType() const = 0;

        virtual void UnitSpeedChanged() { }
        // timer in ms
        virtual void Pause(uint32/* timer = 0*/) { } 
        // timer in ms
        virtual void Resume(uint32/* overrideTimer = 0*/) { }

        // used by Evade code for select point to evade with expected restart default movement
        virtual bool GetResetPosition(Unit*, float&/* x*/, float&/* y*/, float&/* z*/) { return false; } 

        void AddFlag(uint16 const flag) { Flags |= flag; }
        bool HasFlag(uint16 const flag) const { return (Flags & flag) != 0; }
        void RemoveFlag(uint16 const flag) { Flags &= ~flag; }

        uint8 Mode;
        uint8 Priority;
        uint16 Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
        uint32 BaseUnitState;
};

template<class T, class D>
class MovementGeneratorMedium : public MovementGenerator
{
    public:
        MovementGeneratorMedium(uint8 mode, uint8 priority, uint32 baseUnitState) :
            MovementGenerator(mode, priority, baseUnitState)
        {}

        bool Initialize(Unit* owner) override
        {
            return (static_cast<D*>(this))->DoInitialize(static_cast<T*>(owner));
        }

        void Reset(Unit* owner) override
        {
            (static_cast<D*>(this))->DoReset(static_cast<T*>(owner));
        }

        bool Update(Unit* owner, uint32 time_diff) override
        {
            return (static_cast<D*>(this))->DoUpdate(static_cast<T*>(owner), time_diff);
        }

        void Deactivate(Unit* owner) override
        {
            (static_cast<D*>(this))->DoDeactivate(static_cast<T*>(owner));
        }

        //@active: Generator is currently active
        void Finalize(Unit* owner, bool active, bool movementInform) override
        {
            (static_cast<D*>(this))->DoFinalize(static_cast<T*>(owner), active, movementInform);
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
