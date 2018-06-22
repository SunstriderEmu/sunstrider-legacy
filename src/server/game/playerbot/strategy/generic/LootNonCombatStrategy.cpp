
#include "../../playerbot.h"
#include "LootNonCombatStrategy.h"

using namespace ai;

void LootNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "loot available",
        NextAction::array({ std::make_shared<NextAction>("loot", 6.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "far from loot target",
        NextAction::array({ std::make_shared<NextAction>("move to loot", 7.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "can loot",
        NextAction::array({ std::make_shared<NextAction>("open loot", 8.0f) })));
}

void GatherStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no possible targets",
        NextAction::array({ std::make_shared<NextAction>("add gathering loot", 2.0f) })));
}


