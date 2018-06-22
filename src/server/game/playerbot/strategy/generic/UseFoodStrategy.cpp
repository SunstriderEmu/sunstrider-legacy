
#include "../../playerbot.h"
#include "UseFoodStrategy.h"

using namespace ai;

void UseFoodStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("food", 2.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("drink", 2.0f) })));
}
