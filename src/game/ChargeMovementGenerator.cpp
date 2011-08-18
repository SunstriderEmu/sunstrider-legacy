#include "ChargeMovementGenerator.h"
#include "DestinationHolderImp.h"
#include "SpellMgr.h"

template<class T, class U>
ChargeMovementGeneratorMedium<T, U>::ChargeMovementGeneratorMedium(Unit* target, const uint32 triggeredSpellId)
    : m_target(target), m_triggeredSpellId(triggeredSpellId), i_currentNode(0)
{
}

template<class T, class U>
bool ChargeMovementGeneratorMedium<T, U>::Update(T &owner, const uint32 &diff)
{
    if (!&owner)
        return false;

    // if the unit can't move, stop charge
    if (owner.hasUnitState(UNIT_STAT_NOT_MOVE))
        return false;

    // if there is no path, stop charge
    if (i_path->getPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE))
        return false;
        
    if (owner.IsWithinMeleeRange(m_target))
        return false;

    Traveller<T> traveller(owner);
    traveller.SetCustomSpeed(CHARGE_SPEED);

    i_destinationHolder.UpdateTraveller(traveller, diff, false);
    /*if (i_destinationHolder.UpdateTraveller(traveller, diff, false, false))
        if (!IsActive(owner))
            return true;*/

    PointPath pointPath = i_path->getFullPath();
    if (i_destinationHolder.HasArrived())
    {
        ++i_currentNode;
            
        /*PathNode &node = pointPath[i_currentNode];
        // prevent half-turn at last point
        if (!owner.HasInArc(3*M_PI/2, node.x, node.y))
            ++i_currentNode;*/

        // if we are at the last node, stop charge
        if (i_currentNode > pointPath.size())
            return false;

        MoveToNextNode(traveller);
    }

    return true;
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::MoveToNextNode(Traveller<T> &traveller)
{
    PointPath pointPath = i_path->getFullPath();
    PathNode &node = pointPath[i_currentNode];
    i_destinationHolder.SetDestination(traveller, node.x, node.y, node.z, false);
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::LoadPath(T &owner)
{
    // set the destination
    float x, y, z;
    m_target->GetContactPoint(&owner, x, y, z);

    // get the path to the destination
    i_path = new PathInfo(&owner, x, y, z);
    PointPath pointPath = i_path->getFullPath();

    // start movement
    Traveller<T> traveller(owner);
    traveller.SetCustomSpeed(CHARGE_SPEED);
    MoveToNextNode(traveller);

    // send path to client
    uint32 transitTime = uint32(pointPath.GetTotalLength() / (CHARGE_SPEED / IN_MILLISECONDS));
    owner.MonsterMoveByPath(pointPath, 1, pointPath.size(), transitTime);
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::Initialize(T &owner)
{
    owner.addUnitState(UNIT_STAT_CHARGE|UNIT_STAT_CHARGE_MOVE);

    LoadPath(owner);

    // TODO: disable player bar?
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::Finalize(T &owner)
{
    owner.clearUnitState(UNIT_STAT_CHARGE|UNIT_STAT_CHARGE_MOVE);

    PointPath pointPath = i_path->getFullPath();
    if (i_currentNode >= pointPath.size() && m_target)
    {
        // we are at the destination, turn to target and cast spell
        //owner.SetInFront(m_target);

        if (m_triggeredSpellId)
            owner.CastSpell(m_target, m_triggeredSpellId, true);
            
        // not all charge effects used in negative spells
        /*if (!IsPositiveSpell(m_triggeredSpellId) && owner.GetTypeId() == TYPEID_PLAYER)
            owner.Attack(m_target, true);*/
            
        //owner.SetInFront(m_target);
    }
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::Interrupt(T &owner)
{
    owner.clearUnitState(UNIT_STAT_CHARGE|UNIT_STAT_CHARGE_MOVE);
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::Reset(T &owner)
{
    Initialize(owner);
}

template ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::ChargeMovementGeneratorMedium(Unit*, const uint32);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Finalize(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Initialize(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Interrupt(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::LoadPath(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::MoveToNextNode(PlayerTraveller &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Reset(Player &);
template bool ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Update(Player &, const uint32 &);

template ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::ChargeMovementGeneratorMedium(Unit*, const uint32);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Finalize(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Initialize(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Interrupt(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::LoadPath(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::MoveToNextNode(CreatureTraveller &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Reset(Creature &);
template bool ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Update(Creature &, const uint32 &);
