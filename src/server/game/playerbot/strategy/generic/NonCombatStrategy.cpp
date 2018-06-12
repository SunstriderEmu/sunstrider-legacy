
#include "../../playerbot.h"
#include "NonCombatStrategy.h"

using namespace ai;

void NonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "timer",
        NextAction::array({ new NextAction("check mount state", 1.0f) })));
}


void LfgStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no possible targets",
        NextAction::array({ new NextAction("lfg join", 1.0f) })));

    triggers.push_back(new TriggerNode(
        "lfg proposal",
        NextAction::array({ new NextAction("lfg accept", 1.0f) })));

    triggers.push_back(new TriggerNode(
        "lfg proposal active",
        NextAction::array({ new NextAction("lfg accept", 1.0f) })));
}
