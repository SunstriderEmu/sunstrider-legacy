
#include "../../playerbot.h"
#include "GuardStrategy.h"

using namespace ai;


NextAction** GuardStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("guard", 4.0f) });
}

void GuardStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
}

