//
#include "../../playerbot.h"
#include "CombatStrategy.h"

using namespace ai;

void CombatStrategy::InitTriggers(list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "invalid target",
        NextAction::array({ new NextAction("drop target", ACTION_HIGH + 9) })));
}
