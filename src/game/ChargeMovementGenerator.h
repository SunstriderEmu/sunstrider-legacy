#ifndef MANGOS_CHARGEMOVEMENTGENERATOR_H
#define MANGOS_CHARGEMOVEMENTGENERATOR_H

#include "WaypointMovementGenerator.h"
#include "PathFinder.h"

// TODO: figure out nice speed for charge
#define CHARGE_SPEED 25.0f

template<class T, class U>
class ChargeMovementGeneratorMedium : public MovementGeneratorMedium<T, U>, public PathMovementBase<T>
{
public:
    ChargeMovementGeneratorMedium(uint64 targetGUID, const uint32 triggeredSpellId, const uint32 triggeredSpellId2);

    void Initialize(T &u);
    void Interrupt(T &);
    void Finalize(T &);
    void Reset(T &u);
    bool Update(T &u, const uint32 &diff);

    void LoadPath(T &u);

    MovementGeneratorType GetMovementGeneratorType() const { return CHARGE_MOTION_TYPE; }

private:
    uint64 m_targetGUID;
    const uint32 m_triggeredSpellId, m_triggeredSpellId2;

    void MoveToNextNode(Traveller<T> &traveller);
    
    DestinationHolder< Traveller<T> > i_destinationHolder;
    PathInfo* i_path;
    uint32 i_currentNode;
    
    float finalX, finalY, finalZ;
    bool castedSpells;
};

template<class T>
class ChargeMovementGenerator : public ChargeMovementGeneratorMedium<T, ChargeMovementGenerator<T> >
{
public:
    ChargeMovementGenerator(uint64 targetGUID, const uint32 triggeredSpellId, const uint32 triggeredSpellId2)
        : ChargeMovementGeneratorMedium<T, ChargeMovementGenerator<T> >(targetGUID, triggeredSpellId, triggeredSpellId2) {}
        
    MovementGeneratorType GetMovementGeneratorType() { return CHARGE_MOTION_TYPE; }
};

#endif
