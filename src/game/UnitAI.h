#ifndef TRINITY_UNITAI_H
#define TRINITY_UNITAI_H

#include "Define.h"
#include "Unit.h"
class Unit;
class Quest;
class Player;

//Selection method used by SelectSpellTarget
enum SelectAggroTarget : int
{
    SELECT_TARGET_RANDOM = 0,                               //Just selects a random target
    SELECT_TARGET_TOPAGGRO,                                 //Selects targes from top aggro to bottom
    SELECT_TARGET_BOTTOMAGGRO,                              //Selects targets from bottom aggro to top
    SELECT_TARGET_NEAREST,
    SELECT_TARGET_FARTHEST,
};

class UnitAI
{
    protected:
        Unit *me;
        //combat movement part not yet implemented. Creatures with m_combatDistance and target distance > 5.0f wont show melee weapons.
        float m_combatDistance;         
        bool m_allowCombatMovement;
        bool m_restoreCombatMovementOnOOM;
    public:
        UnitAI(Unit *u) : me(u), m_combatDistance(0.5f), m_allowCombatMovement(true), m_restoreCombatMovementOnOOM(false) {}
        virtual ~UnitAI() { }
        
        virtual void AttackStart(Unit *);
        void AttackStartCaster(Unit* victim, float dist);
        virtual void UpdateAI(const uint32 diff) { }

        float GetCombatDistance() { return m_combatDistance; };
        void SetCombatDistance(float dist);

        bool IsCombatMovementAllowed() { return m_allowCombatMovement; };
        void SetCombatMovementAllowed(bool allow);
        void SetRestoreCombatMovementOnOOM(bool set);
        bool GetRestoreCombatMovementOnOOM();

        virtual void InitializeAI() { Reset(); }

        virtual void Reset() {}

        // Called when unit is charmed
        virtual void OnCharmed(Unit* charmer, bool apply) = 0;
        virtual void OnPossess(Unit* charmer, bool apply) { }

        // Pass parameters between AI
        virtual void DoAction(const int32 param) {}
        virtual uint32 GetData(uint32 /*id = 0*/) const { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/, Unit* setter = nullptr) {}
        virtual void SetGUID(uint64 /*guid*/, int32 /*id*/ = 0) { }
        virtual uint64 GetGUID(int32 /*id*/ = 0) const { return 0; }

        virtual void AttackedBy(Unit* who) {}
        
        // Called at any Damage from any attacker (before damage apply)
        virtual void DamageTaken(Unit *done_by, uint32 & /*damage*/) {}
        
        // Called at any Damage to any victim (before damage apply)
        virtual void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) { }

        // Called when the creature receives heal
        virtual void HealReceived(Unit* /*done_by*/, uint32& /*addhealth*/) {}

        // Called when the unit heals
        virtual void HealDone(Unit* /*done_to*/, uint32& /*addhealth*/) { }

        //Do melee swing of current victim if in rnage and ready and not casting
        virtual void DoMeleeAttackIfReady();
        bool DoSpellAttackIfReady(uint32 spell);

        //Selects a unit from the creature's current aggro list
        bool checkTarget(Unit* target, bool playersOnly, float radius, bool noTank = false);

        //SelectTargetFromPlayerList -> me->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0.0f, 500.0f, true)
        // Select the targets satisfying the predicate.
        // predicate shall extend std::unary_function<Unit*, bool>
        template<class PREDICATE> Unit* SelectTarget(SelectAggroTarget targetType, uint32 position, PREDICATE const& predicate)
        {
            ThreatContainer::StorageType const& threatlist = me->getThreatManager().getThreatList();
            if (position >= threatlist.size())
                return NULL;

            std::list<Unit*> targetList;
            for (ThreatContainer::StorageType::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                if (predicate((*itr)->getTarget()))
                    targetList.push_back((*itr)->getTarget());

            if (position >= targetList.size())
                return NULL;

            if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
                targetList.sort(Trinity::ObjectDistanceOrderPred(me));

            switch (targetType)
            {
            case SELECT_TARGET_NEAREST:
            case SELECT_TARGET_TOPAGGRO:
            {
                std::list<Unit*>::iterator itr = targetList.begin();
                std::advance(itr, position);
                return *itr;
            }
            case SELECT_TARGET_FARTHEST:
            case SELECT_TARGET_BOTTOMAGGRO:
            {
                std::list<Unit*>::reverse_iterator ritr = targetList.rbegin();
                std::advance(ritr, position);
                return *ritr;
            }
            case SELECT_TARGET_RANDOM:
            {
                std::list<Unit*>::iterator itr = targetList.begin();
                std::advance(itr, urand(position, targetList.size() - 1));
                return *itr;
            }
            default:
                break;
            }

            return NULL;
        }

        Unit* SelectTarget(SelectAggroTarget target, uint32 position);
        Unit* SelectTarget(SelectAggroTarget target, uint32 position, float dist, bool playerOnly, bool noTank = false);
        Unit* SelectTarget(SelectAggroTarget target, uint32 position, float distNear, float distFar, bool playerOnly);
        Unit* SelectTarget(uint32 position, float distMin, float distMax, bool playerOnly, bool auraCheck, bool exceptPossesed, uint32 spellId, uint32 effIndex);
        void SelectUnitList(std::list<Unit*> &targetList, uint32 num, SelectAggroTarget target, float dist, bool playerOnly, uint32 notHavingAuraId = 0, uint8 effIndex = 0);

        virtual void sGossipHello(Player* player) {}
        virtual void sGossipSelect(Player* player, uint32 sender, uint32 action) {}
        virtual void sGossipSelectCode(Player* player, uint32 sender, uint32 action, const char* code) {}
        virtual void sQuestAccept(Player* player, Quest const* quest) {}
        virtual void sQuestSelect(Player* player, Quest const* quest) {}
        virtual void sQuestComplete(Player* player, Quest const* quest) {}
        virtual void sQuestReward(Player* player, Quest const* quest, uint32 opt) {}
};

#endif //TRINITY_UNITAI_H