#pragma once

#include "GenericWarriorStrategy.h"

namespace ai
{
    class DpsWarriorStrategy : public GenericWarriorStrategy
    {
    public:
        DpsWarriorStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "dps"; }
        virtual ActionList getDefaultActions();
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
    };

    class DpsWarrirorAoeStrategy : public CombatStrategy
    {
    public:
        DpsWarrirorAoeStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "aoe"; }
    };
}
