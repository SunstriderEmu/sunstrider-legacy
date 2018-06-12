//
#include "../../playerbot.h"
#include "EmoteStrategy.h"

using namespace ai;


void EmoteStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array({ new NextAction("emote", 1.0f) })));

    triggers.push_back(new TriggerNode(
        "seldom",
        NextAction::array({ new NextAction("suggest what to do", 1.0f) })));
}
