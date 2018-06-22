//
#include "../../playerbot.h"
#include "../Strategy.h"
#include "DeadStrategy.h"

using namespace ai;

void DeadStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "dead",
        NextAction::array({ std::make_shared<NextAction>("revive from corpse", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "resurrect request",
        NextAction::array({ std::make_shared<NextAction>("accept resurrect", relevance) })));
}

DeadStrategy::DeadStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
}
