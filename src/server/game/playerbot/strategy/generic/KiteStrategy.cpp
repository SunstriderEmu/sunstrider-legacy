
#include "../../playerbot.h"
#include "KiteStrategy.h"

using namespace ai;

KiteStrategy::KiteStrategy(PlayerbotAI* ai) : Strategy(ai)
{
}

void KiteStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "has aggro",
        NextAction::array({ new NextAction("runaway", 51.0f) })));
}
