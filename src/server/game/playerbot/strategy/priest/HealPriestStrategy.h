#pragma once

#include "GenericPriestStrategy.h"

namespace ai
{
    class HealPriestStrategy : public GenericPriestStrategy
    {
    public:
        HealPriestStrategy(PlayerbotAI* ai) : GenericPriestStrategy(ai) {}

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual ActionList getDefaultActions();
        virtual std::string getName() { return "heal"; }
        virtual int GetType() { return STRATEGY_TYPE_HEAL; }
    };
}
