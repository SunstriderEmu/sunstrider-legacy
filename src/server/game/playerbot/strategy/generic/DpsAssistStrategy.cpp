//
#include "../../playerbot.h"
#include "DpsAssistStrategy.h"

using namespace ai;

void DpsAssistStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no attackers",
        NextAction::array({ std::make_shared<NextAction>("dps assist", 50.0f) })));
}



