
#ifndef TRINITY_PETAI_H
#define TRINITY_PETAI_H

#include "CreatureAI.h"
#include "Timer.h"

class Creature;
class Spell;

typedef std::vector<std::pair<Unit*, Spell*>> TargetSpellList;

class TC_GAME_API PetAI : public CreatureAI
{
    public:

        PetAI(Creature *c);

        void EnterEvadeMode(EvadeReason /* why */) override;
        void JustDied(Unit* /*who*/) override { _stopAttack(); }

        void UpdateAI(const uint32) override;
        static int Permissible(const Creature *);

		void MovementInform(uint32 moveType, uint32 data) override;
		void KilledUnit(Unit* /*victim*/) override;
        void AttackStart(Unit* target) override; // only start attacking if not attacking something else already
        void _AttackStart(Unit* target); // always start attacking if possible
		void OwnerAttackedBy(Unit* attacker) override;
		void OwnerAttacked(Unit* target) override;
        void DamageTaken(Unit* attacker, uint32& /*damage*/) override { AttackStart(attacker); }
		void ReceiveEmote(Player* player, uint32 textEmote) override;

		void ForceAttackBreakable(Unit const* target);
		void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/) override;

        void MoveInLineOfSight(Unit* /*who*/) override { } // CreatureAI interferes with returning pets
        void MoveInLineOfSight_Safe(Unit* /*who*/) { } // CreatureAI interferes with returning pets

    private:
        bool _needToStop(void) const;
        void _stopAttack(void);

        void UpdateAllies();

        Creature &i_pet;
        TimeTracker i_tracker;
        std::set<ObjectGuid> m_AllySet;
        uint32 m_updateAlliesTimer;

		TargetSpellList m_targetSpellStore;

		Unit* SelectNextTarget(bool allowAutoSelect) const;
		void HandleReturnMovement();
		void DoAttack(Unit* target, bool chase);
		bool CanAttack(Unit* target);
		void ClearCharmInfoFlags();

		Unit const* _forceAttackBreakable; //force attack target even though it has breakable aura
};
#endif

