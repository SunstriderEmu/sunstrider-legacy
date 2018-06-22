
#include "../../playerbot.h"
#include "MoveRandomStrategy.h"

using namespace ai;

void MoveRandomStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "random",
        NextAction::array({ std::make_shared<NextAction>("move random", 1.0f) })));
}

