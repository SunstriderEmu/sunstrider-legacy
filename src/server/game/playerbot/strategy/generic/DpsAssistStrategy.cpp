//
#include "../../playerbot.h"
#include "DpsAssistStrategy.h"

using namespace ai;

void DpsAssistStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "not dps target active",
        NextAction::array({ std::make_shared<NextAction>("dps assist", 60.0f) })));
}



