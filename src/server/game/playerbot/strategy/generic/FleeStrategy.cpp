
#include "../../playerbot.h"
#include "FleeStrategy.h"

using namespace ai;

void FleeStrategy::InitTriggers(list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "panic",
        NextAction::array({ new NextAction("flee", ACTION_EMERGENCY + 9) })));

    triggers.push_back(new TriggerNode(
       "critical health",
       NextAction::array({ new NextAction("flee", ACTION_MOVE + 9) })));

   triggers.push_back(new TriggerNode(
       "low mana",
       NextAction::array({ new NextAction("flee", ACTION_MOVE + 9) })));
}

void FleeFromAddsStrategy::InitTriggers(list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "has nearest adds",
        NextAction::array({ new NextAction("runaway", 50.0f) })));
}
