
#include "../../playerbot.h"
#include "TankAssistStrategy.h"

using namespace ai;


void TankAssistStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no attackers",
        NextAction::array({ new NextAction("tank assist", 50.0f) })));
}
