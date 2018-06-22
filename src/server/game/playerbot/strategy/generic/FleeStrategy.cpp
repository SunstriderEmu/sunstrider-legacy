
#include "../../playerbot.h"
#include "FleeStrategy.h"

using namespace ai;

void FleeStrategy::InitTriggers(list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "panic",
        NextAction::array({ std::make_shared<NextAction>("flee", ACTION_EMERGENCY + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
       "critical health",
       NextAction::array({ std::make_shared<NextAction>("flee", ACTION_MOVE + 9) })));

   triggers.push_back(std::make_shared<TriggerNode>(
       "low mana",
       NextAction::array({ std::make_shared<NextAction>("flee", ACTION_MOVE + 9) })));
}

void FleeFromAddsStrategy::InitTriggers(list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "has nearest adds",
        NextAction::array({ std::make_shared<NextAction>("runaway", 50.0f) })));
}
