#pragma once

#include "GenericShamanStrategy.h"

namespace ai
{
    class MeleeShamanStrategy : public GenericShamanStrategy
    {
    public:
        MeleeShamanStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual ActionList getDefaultActions();
        virtual std::string getName() { return "melee"; }
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
    };

    class MeleeAoeShamanStrategy : public CombatStrategy
    {
    public:
        MeleeAoeShamanStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "melee aoe"; }
    };
}
