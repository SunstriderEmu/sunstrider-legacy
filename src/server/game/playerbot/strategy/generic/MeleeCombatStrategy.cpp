
#include "../../playerbot.h"
#include "MeleeCombatStrategy.h"

using namespace ai;


void MeleeCombatStrategy::InitTriggers(list<std::shared_ptr<TriggerNode>> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "not facing target",
        NextAction::array({ std::make_shared<NextAction>("set facing", ACTION_NORMAL + 7) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of melee",
        NextAction::array({ std::make_shared<NextAction>("reach melee", ACTION_NORMAL + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for melee",
        NextAction::array({ std::make_shared<NextAction>("move out of enemy contact", ACTION_NORMAL + 8) })));
}
