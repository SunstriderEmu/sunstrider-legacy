
#ifndef TRINITY_GUARDAI_H
#define TRINITY_GUARDAI_H

#include "ScriptedCreature.h"
#include "Timer.h"

class Creature;

class TC_GAME_API GuardAI : public CreatureAI
{
    enum GuardState
    {
        STATE_NORMAL = 1,
        STATE_LOOK_AT_VICTIM = 2
    };

    public:

        GuardAI(Creature *c);

        void MoveInLineOfSight(Unit *) override;
        void EnterEvadeMode(EvadeReason /* why */) override;
        void JustDied(Unit *) override;

        void UpdateAI(const uint32) override;
        static int Permissible(const Creature *);

		bool CanSeeAlways(WorldObject const* obj) override;

    private:
        Creature &i_creature;
        ObjectGuid i_victimGuid;
        GuardState i_state;
        TimeTracker i_tracker;
};

//old guardAI ? Both are still in use
struct TC_GAME_API guardAI : public ScriptedAI
{
    guardAI(Creature *c) : ScriptedAI(c) {}

    uint32 GlobalCooldown;                                  //This variable acts like the global cooldown that players have (1.5 seconds)
    uint32 BuffTimer;                                       //This variable keeps track of buffs

    void Reset();

    void JustEngagedWith(Unit *who);

    void JustDied(Unit *Killer);

    void UpdateAI(const uint32 diff);
};

#endif

