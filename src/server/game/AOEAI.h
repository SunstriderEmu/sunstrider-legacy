
#ifndef TRINITY_AOEAI_H
#define TRINITY_AOEAI_H

#include "CreatureAI.h"

class Creature;

struct AOEAI : public CreatureAI
{
    public:
        explicit AOEAI(Creature *c);
        void AttackStart(Unit *who);
        void UpdateAI(const uint32 diff);

        static int Permissible(const Creature *);
};

#endif
