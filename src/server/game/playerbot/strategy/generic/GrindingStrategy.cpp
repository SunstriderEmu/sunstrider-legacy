
#include "../../playerbot.h"
#include "GrindingStrategy.h"

using namespace ai;


ActionList GrindingStrategy::getDefaultActions()
{
    return {};
}

void GrindingStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no target",
        NextAction::array( { std::make_shared<NextAction>("attack anything", 5.0f) })));
}

