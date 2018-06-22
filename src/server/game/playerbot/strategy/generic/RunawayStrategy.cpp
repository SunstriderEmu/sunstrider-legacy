
#include "../../playerbot.h"
#include "RunawayStrategy.h"

using namespace ai;


ActionList RunawayStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("runaway", 50.0f) });
}

void RunawayStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for spell",
        NextAction::array({ std::make_shared<NextAction>("runaway", 50.0f) })));
}
