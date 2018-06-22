//
#include "../../playerbot.h"
#include "EmoteStrategy.h"

using namespace ai;


void EmoteStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "random",
        NextAction::array({ std::make_shared<NextAction>("emote", 1.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "seldom",
        NextAction::array({ std::make_shared<NextAction>("suggest what to do", 1.0f) })));
}
