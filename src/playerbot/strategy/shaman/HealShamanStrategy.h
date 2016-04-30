#pragma once

#include "GenericShamanStrategy.h"

namespace ai
{
    class HealShamanStrategy : public GenericShamanStrategy
    {
    public:
        HealShamanStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::list<TriggerNode*> &triggers);
        virtual std::string getName() { return "heal"; }
		virtual int GetType() { return STRATEGY_TYPE_HEAL; }
    };
}
