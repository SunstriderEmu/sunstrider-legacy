#ifndef TRINITY_TOTEMAI_H
#define TRINITY_TOTEMAI_H

#include "CreatureAI.h"
#include "Timer.h"

class Creature;
class Totem;

class TC_GAME_API TotemAI : public CreatureAI
{
    public:

        TotemAI(Creature *c);
        ~TotemAI() override;

        void MoveInLineOfSight(Unit *) override;
        void AttackStart(Unit *) override;
        void EnterEvadeMode(EvadeReason /* why */) override;

        void UpdateAI(const uint32) override;
        static int Permissible(const Creature *);

    private:
        Totem &i_totem;
        ObjectGuid i_victimGuid;
};

//simply kill self
class KillMagnetEvent : public BasicEvent
{
public:
    KillMagnetEvent(Unit& self) : _self(self) { }
    bool Execute(uint64 e_time, uint32 p_time) override
    {
        _self.SetDeathState(JUST_DIED);
        return true;
    }

protected:
    Unit& _self;
};

#endif

