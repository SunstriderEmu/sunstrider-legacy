
#include "../../playerbot.h"
#include "RangedCombatStrategy.h"

using namespace ai;


void RangedCombatStrategy::InitTriggers(list<TriggerNode*> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_NORMAL + 9) })));
}
