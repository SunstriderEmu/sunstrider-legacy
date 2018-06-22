
#include "../../playerbot.h"
#include "KiteStrategy.h"

using namespace ai;

KiteStrategy::KiteStrategy(PlayerbotAI* ai) : Strategy(ai)
{
}

void KiteStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "has aggro",
        NextAction::array({ std::make_shared<NextAction>("runaway", 51.0f) })));
}
