
#include "../../playerbot.h"
#include "FollowMasterStrategy.h"

using namespace ai;

ActionList FollowMasterStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("follow", 1.0f) });
}

void FollowMasterStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "out of react range",
        NextAction::array({ std::make_shared<NextAction>("tell out of react range", 10.0f) })));
}
