
#include "../../playerbot.h"
#include "UsePotionsStrategy.h"

using namespace ai;

void UsePotionsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("healing potion", ACTION_MEDIUM_HEAL) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("mana potion", ACTION_EMERGENCY) })));
}
