////
#include "../../playerbot.h"
#include "AttackEnemyPlayersStrategy.h"

using namespace ai;

void AttackEnemyPlayersStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy player is attacking",
        NextAction::array({ new NextAction("attack enemy player", 61.0f) })));
}

