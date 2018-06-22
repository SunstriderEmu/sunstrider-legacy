#include "CombatStrategy.h"
#include "../generic/CombatStrategy.h"
#pragma once

namespace ai
{
    class MeleeCombatStrategy : public CombatStrategy
    {
    public:
        MeleeCombatStrategy(PlayerbotAI* ai) : CombatStrategy(ai) {}
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual int GetType() { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
    };


}
