
#include "../../playerbot.h"
#include "MeleeCombatStrategy.h"

using namespace ai;


void MeleeCombatStrategy::InitTriggers(list<TriggerNode*> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "not facing target",
        NextAction::array({ new NextAction("set facing", ACTION_NORMAL + 7) })));

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array({ new NextAction("reach melee", ACTION_NORMAL + 8) })));

    triggers.push_back(new TriggerNode(
        "enemy too close for melee",
        NextAction::array({ new NextAction("move out of enemy contact", ACTION_NORMAL + 8) })));
}
