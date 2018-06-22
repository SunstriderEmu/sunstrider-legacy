
#include "../../playerbot.h"
#include "TankAssistStrategy.h"

using namespace ai;


void TankAssistStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no attackers",
        NextAction::array({ std::make_shared<NextAction>("tank assist", 50.0f) })));
}
