#ifndef TRINITY_UNITAI_H
#define TRINITY_UNITAI_H

#include "Define.h"
#include "Unit.h"
class Unit;
class Quest;
class Player;

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
        virtual void UpdateAI(const uint32 diff) { }

        float GetCombatDistance() { return m_combatDistance; };
        void SetCombatDistance(float dist);

        bool IsCombatMovementAllowed() { return m_allowCombatMovement; };
        void SetCombatMovementAllowed(bool allow);
        void SetRestoreCombatMovementOnOOM(bool set);
        bool GetRestoreCombatMovementOnOOM();

        virtual void InitializeAI() { Reset(); }

        virtual void Reset() {};

        // Called when unit is charmed
        virtual void OnCharmed(Unit* charmer, bool apply) = 0;
        virtual void OnPossess(Unit* charmer, bool apply) = 0;

        // Pass parameters between AI
        virtual void DoAction(const int32 param) {}
        virtual uint32 GetData(uint32 /*id = 0*/) const { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/) {}
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
        
        virtual void sOnGossipHello(Player* player) {}
        virtual void sGossipSelect(Player* player, uint32 sender, uint32 action) {}
        virtual void sOnGossipSelectCode(Player* player, uint32 sender, uint32 action, const char* code) {}
        virtual void sQuestAccept(Player* player, Quest const* quest) {}
        virtual void sQuestSelect(Player* player, Quest const* quest) {}
        virtual void sQuestComplete(Player* player, Quest const* quest) {}
        virtual void sQuestReward(Player* player, Quest const* quest, uint32 opt) {}
};

#endif //TRINITY_UNITAI_H