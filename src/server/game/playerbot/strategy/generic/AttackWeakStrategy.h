#include "../generic/NonCombatStrategy.h"
#pragma once

namespace ai
{
    class AttackWeakStrategy : public NonCombatStrategy
    {
    public:
        AttackWeakStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}
        virtual std::string getName() { return "attack weak"; }

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
    };

}
