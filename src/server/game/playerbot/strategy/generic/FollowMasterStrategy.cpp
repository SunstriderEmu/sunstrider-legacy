
#include "../../playerbot.h"
#include "FollowMasterStrategy.h"

using namespace ai;

ActionList FollowMasterStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("follow", 1.0f) });
}

void FollowMasterStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "out of react range",
        NextAction::array({ new NextAction("tell out of react range", 10.0f) })));
}
