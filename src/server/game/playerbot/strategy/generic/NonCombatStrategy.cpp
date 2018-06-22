
#include "../../playerbot.h"
#include "NonCombatStrategy.h"

using namespace ai;

void NonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "timer",
        NextAction::array({ std::make_shared<NextAction>("check mount state", 1.0f) })));
}


void LfgStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no possible targets",
        NextAction::array({ std::make_shared<NextAction>("lfg join", 1.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "lfg proposal",
        NextAction::array({ std::make_shared<NextAction>("lfg accept", 1.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "lfg proposal active",
        NextAction::array({ std::make_shared<NextAction>("lfg accept", 1.0f) })));
}
