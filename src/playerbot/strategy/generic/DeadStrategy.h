#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class DeadStrategy : public PassTroughStrategy
    {
    public:
        DeadStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::list<TriggerNode*> &triggers);
        virtual std::string getName() { return "dead"; }
    };
}
