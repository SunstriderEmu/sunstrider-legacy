#pragma once

#include "../Strategy.h"
#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class ShamanNonCombatStrategy : public NonCombatStrategy
    {
    public:
        ShamanNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual void InitMultipliers(std::list<std::shared_ptr<Multiplier>> &multipliers);
        virtual std::string getName() { return "nc"; }

    };
}
