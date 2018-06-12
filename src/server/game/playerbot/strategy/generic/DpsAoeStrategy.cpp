//
#include "../../playerbot.h"
#include "DpsAoeStrategy.h"

using namespace ai;

void DpsAoeStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "timer",
        NextAction::array({ new NextAction("dps assist", 50.0f) })));
}
