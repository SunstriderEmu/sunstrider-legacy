
#include "../../playerbot.h"
#include "LootNonCombatStrategy.h"

using namespace ai;

void LootNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "loot available",
        NextAction::array({ new NextAction("loot", 6.0f) })));

    triggers.push_back(new TriggerNode(
        "far from loot target",
        NextAction::array({ new NextAction("move to loot", 7.0f) })));

    triggers.push_back(new TriggerNode(
        "can loot",
        NextAction::array({ new NextAction("open loot", 8.0f) })));
}

void GatherStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no possible targets",
        NextAction::array({ new NextAction("add gathering loot", 2.0f) })));
}


