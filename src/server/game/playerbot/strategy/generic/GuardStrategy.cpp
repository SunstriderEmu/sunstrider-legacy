
#include "../../playerbot.h"
#include "GuardStrategy.h"

using namespace ai;


ActionList GuardStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("guard", 4.0f) });
}

void GuardStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
}

