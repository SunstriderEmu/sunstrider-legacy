//
#include "../../playerbot.h"
#include "CombatStrategy.h"

using namespace ai;

void CombatStrategy::InitTriggers(list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "invalid target",
        NextAction::array({ std::make_shared<NextAction>("drop target", ACTION_HIGH + 9) })));
}
