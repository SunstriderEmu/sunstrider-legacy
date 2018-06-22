////
#include "../../playerbot.h"
#include "AttackWeakStrategy.h"

using namespace ai;

void AttackWeakStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "not least hp target active",
        NextAction::array({ std::make_shared<NextAction>("attack least hp target", 60.0f) })));
}

