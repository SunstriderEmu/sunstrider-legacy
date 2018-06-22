#pragma once

#include "../Strategy.h"
#include "../generic/MeleeCombatStrategy.h"

namespace ai
{
    class AiObjectContext;

    class GenericWarriorStrategy : public MeleeCombatStrategy
    {
    public:
        GenericWarriorStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers);
        virtual std::string getName() { return "warrior"; }
    };
}
