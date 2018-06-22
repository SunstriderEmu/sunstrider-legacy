//
#include "../../playerbot.h"
#include "DuelStrategy.h"

using namespace ai;

void DuelStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "duel requested",
        NextAction::array({ std::make_shared<NextAction>("accept duel", relevance) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no attackers",
        NextAction::array({ std::make_shared<NextAction>("attack duel opponent", 70.0f) })));
}



DuelStrategy::DuelStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
}
