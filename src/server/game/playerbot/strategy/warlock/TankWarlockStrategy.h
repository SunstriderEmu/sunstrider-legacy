#pragma once

#include "GenericWarlockStrategy.h"

namespace ai
{
    class TankWarlockStrategy : public GenericWarlockStrategy
    {
    public:
        TankWarlockStrategy(PlayerbotAI* ai);
        virtual std::string getName() { return "tank"; }

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual ActionList getDefaultActions();
    };
}
