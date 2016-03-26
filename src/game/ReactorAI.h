
#ifndef TRINITY_REACTORAI_H
#define TRINITY_REACTORAI_H

#include "CreatureAI.h"

class Unit;

//Default AI for civilian creatures
class ReactorAI : public CreatureAI
{
    public:

        explicit ReactorAI(Creature* c) : CreatureAI(c) {}

        void MoveInLineOfSight(Unit*) {}
        void UpdateAI(uint32 diff);

        static int Permissible(const Creature*);
};
#endif
