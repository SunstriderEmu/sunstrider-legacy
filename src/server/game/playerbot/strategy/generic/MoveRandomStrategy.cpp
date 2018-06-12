
#include "../../playerbot.h"
#include "MoveRandomStrategy.h"

using namespace ai;

void MoveRandomStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "random",
        NextAction::array({ new NextAction("move random", 1.0f) })));
}

