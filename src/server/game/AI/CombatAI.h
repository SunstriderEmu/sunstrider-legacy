

#ifndef TRINITY_COMBATAI_H
#define TRINITY_COMBATAI_H

#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "ConditionMgr.h"

class Creature;

//Default AI for non civilian creatures
class TC_GAME_API AggressorAI : public CreatureAI
{
    public:
        explicit AggressorAI(Creature* c) : CreatureAI(c) {}

        void UpdateAI(const uint32);
        static int Permissible(const Creature*);
};

typedef std::vector<uint32> SpellVct;

#endif
