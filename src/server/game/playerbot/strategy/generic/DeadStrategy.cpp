//
#include "../../playerbot.h"
#include "../Strategy.h"
#include "DeadStrategy.h"

using namespace ai;

void DeadStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "dead",
        NextAction::array({ new NextAction("revive from corpse", relevance) })));

    triggers.push_back(new TriggerNode(
        "resurrect request",
        NextAction::array({ new NextAction("accept resurrect", relevance) })));
}

DeadStrategy::DeadStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
}
