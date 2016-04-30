#pragma once

#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class GenericDruidNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericDruidNonCombatStrategy(PlayerbotAI* ai);
        virtual std::string getName() { return "nc"; }

    public:
        virtual void InitTriggers(std::list<TriggerNode*> &triggers);
    };
}
