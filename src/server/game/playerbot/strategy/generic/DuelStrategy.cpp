//
#include "../../playerbot.h"
#include "DuelStrategy.h"

using namespace ai;

void DuelStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "duel requested",
        NextAction::array({ new NextAction("accept duel", relevance) })));

    triggers.push_back(new TriggerNode(
        "no attackers",
        NextAction::array({ new NextAction("attack duel opponent", 70.0f) })));
}



DuelStrategy::DuelStrategy(PlayerbotAI* ai) : PassTroughStrategy(ai)
{
}
