
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "HealPriestStrategy.h"

using namespace ai;

ActionList HealPriestStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("shoot", 10.0f) });
}

void HealPriestStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe heal",
        NextAction::array({ std::make_shared<NextAction>("circle of healing", 27.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "almost full health",
        NextAction::array({ std::make_shared<NextAction>("renew", 15.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member almost full health",
        NextAction::array({ std::make_shared<NextAction>("renew on party", 10.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for spell",
        NextAction::array({ std::make_shared<NextAction>("fade", 50.0f), std::make_shared<NextAction>("flee", 49.0f) })));
}
