#pragma once

#include "../generic/NonCombatStrategy.h"

namespace ai
{
    class GenericPaladinNonCombatStrategy : public NonCombatStrategy
    {
    public:
        GenericPaladinNonCombatStrategy(PlayerbotAI* ai);
        virtual std::string getName() { return "nc"; }
    
    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
   };
}
