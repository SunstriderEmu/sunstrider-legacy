
#include "../../playerbot.h"
#include "UseFoodStrategy.h"

using namespace ai;

void UseFoodStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("food", 2.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("drink", 2.0f) })));
}
