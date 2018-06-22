#pragma once

#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class GenericRogueNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericRogueNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        virtual std::string getName() { return "nc"; }
    
    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
    };
}