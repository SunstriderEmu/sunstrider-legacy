////
#include "../../playerbot.h"
#include "AttackEnemyPlayersStrategy.h"

using namespace ai;

void AttackEnemyPlayersStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy player is attacking",
        NextAction::array({ std::make_shared<NextAction>("attack enemy player", 61.0f) })));
}

