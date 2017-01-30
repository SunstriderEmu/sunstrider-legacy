
#ifndef TRINITY_PETAI_H
#define TRINITY_PETAI_H

#include "CreatureAI.h"
#include "Timer.h"

class Creature;
class Spell;

class TC_GAME_API PetAI : public CreatureAI
{
    public:

        PetAI(Creature *c);

        void EnterEvadeMode(EvadeReason /* why */) override;
        void JustDied(Unit* who) override { _stopAttack(); }

        void UpdateAI(const uint32) override;
        static int Permissible(const Creature *);
        void ResetMovement();
    protected:
        void Minipet_DistanceCheck(uint32 diff);
    private:
        bool _needToStop(void) const;
        void _stopAttack(void);

        void UpdateAllies();

        Creature &i_pet;
        TimeTracker i_tracker;
        std::set<uint64> m_AllySet;
        uint32 m_updateAlliesTimer;

        typedef std::pair<Unit*, Spell*> TargetSpellPair;
        std::vector<TargetSpellPair> m_targetSpellStore;

        uint32 distanceCheckTimer; //minipet only
};
#endif

