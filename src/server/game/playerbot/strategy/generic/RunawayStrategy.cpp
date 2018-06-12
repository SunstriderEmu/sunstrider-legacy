
#include "../../playerbot.h"
#include "RunawayStrategy.h"

using namespace ai;


NextAction** RunawayStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("runaway", 50.0f) });
}

void RunawayStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array({ new NextAction("runaway", 50.0f) })));
}
