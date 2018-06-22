////
#include "../../playerbot.h"
#include "AttackRtiStrategy.h"

using namespace ai;


void AttackRtiStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no attackers",
        NextAction::array({ std::make_shared<NextAction>("attack rti target", 60.0f) })));
}

