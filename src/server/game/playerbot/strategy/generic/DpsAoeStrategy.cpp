//
#include "../../playerbot.h"
#include "DpsAoeStrategy.h"

using namespace ai;

void DpsAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "timer",
        NextAction::array({ std::make_shared<NextAction>("dps assist", 50.0f) })));
}
