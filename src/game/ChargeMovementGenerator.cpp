#include "ChargeMovementGenerator.h"
#include "DestinationHolderImp.h"

template<class T, class U>
ChargeMovementGeneratorMedium<T, U>::ChargeMovementGeneratorMedium(uint64 targetGUID, const uint32 triggeredSpellId, const uint32 triggeredSpellId2)
    : m_targetGUID(targetGUID), m_triggeredSpellId(triggeredSpellId), m_triggeredSpellId2(triggeredSpellId2), i_currentNode(0), castedSpells(false)
{
}

template<class T, class U>
bool ChargeMovementGeneratorMedium<T, U>::Update(T &owner, const uint32 &diff)
{
    if (!&owner)
        return false;

    // if the unit can't move, stop charge
    if (owner.HasUnitState(UNIT_STAT_NOT_MOVE))
        return false;

    // if there is no path, stop charge
    if (i_path->getPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE))
        return false;

    Traveller<T> traveller(owner);
    traveller.SetCustomSpeed(CHARGE_SPEED);

    i_destinationHolder.UpdateTraveller(traveller, diff, false);
    /*if (i_destinationHolder.UpdateTraveller(traveller, diff, false, false))
        if (!IsActive(owner))
            return true;*/
    if (Unit* m_target = Unit::GetUnit(owner, m_targetGUID)) {
        if (!castedSpells && owner.IsWithinMeleeRange(m_target, 4 * MELEE_RANGE)) {
            if (m_triggeredSpellId)
                owner.CastSpell(m_target, m_triggeredSpellId, true);
            if (m_triggeredSpellId2)
                owner.CastSpell(m_target, m_triggeredSpellId2, true);
                
            castedSpells = true;
        }
    }   

    PointPath pointPath = i_path->getFullPath();
    if (i_destinationHolder.HasArrived())
    {
        ++i_currentNode;

        // if we are at the last node, stop charge
        if (i_currentNode >= pointPath.size())
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
    if (traveller.GetTraveller().GetTypeId() == TYPEID_PLAYER)
        traveller.GetTraveller().ToPlayer()->SetPosition(node.x, node.y, node.z, traveller.GetTraveller().GetOrientation());
}

template<class T, class U>
void ChargeMovementGeneratorMedium<T, U>::LoadPath(T &owner)
{
    // set the destination
    float x, y, z;
    if (Unit* m_target = Unit::GetUnit(owner, m_targetGUID))
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
    PathNode &finalNode = pointPath[pointPath.size() - 1];
    finalX = finalNode.x;
    finalY = finalNode.y;
    finalZ = finalNode.z;
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
    Unit* m_target = Unit::GetUnit(owner, m_targetGUID);
    if (i_currentNode >= pointPath.size() && m_target)
    {
        // we are at the destination, turn to target and cast spell
        owner.SetInFront(m_target);
        // Prevent turning back after reaching target
        if (owner.GetTypeId() == TYPEID_PLAYER)
            owner.ToPlayer()->SetPosition(finalX, finalY, finalZ, owner.GetOrientation());
            
        if (!castedSpells) {
            if (Unit* m_target = Unit::GetUnit(owner, m_targetGUID)) {
                if (m_triggeredSpellId)
                    owner.CastSpell(m_target, m_triggeredSpellId, true);
                if (m_triggeredSpellId2)
                    owner.CastSpell(m_target, m_triggeredSpellId2, true);
                    
                castedSpells = true;
            }
        }
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

template ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::ChargeMovementGeneratorMedium(uint64, const uint32, const uint32);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Finalize(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Initialize(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Interrupt(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::LoadPath(Player &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::MoveToNextNode(PlayerTraveller &);
template void ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Reset(Player &);
template bool ChargeMovementGeneratorMedium<Player, ChargeMovementGenerator<Player> >::Update(Player &, const uint32 &);

template ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::ChargeMovementGeneratorMedium(uint64, const uint32, const uint32);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Finalize(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Initialize(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Interrupt(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::LoadPath(Creature &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::MoveToNextNode(CreatureTraveller &);
template void ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Reset(Creature &);
template bool ChargeMovementGeneratorMedium<Creature, ChargeMovementGenerator<Creature> >::Update(Creature &, const uint32 &);
