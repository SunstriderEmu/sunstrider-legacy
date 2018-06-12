//
#include "../../playerbot.h"
#include "DpsAssistStrategy.h"

using namespace ai;

void DpsAssistStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no attackers",
        NextAction::array({ new NextAction("dps assist", 50.0f) })));
}



