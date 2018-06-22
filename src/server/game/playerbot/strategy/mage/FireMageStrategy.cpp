
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "FireMageStrategy.h"

using namespace ai;

ActionList FireMageStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("scorch", 7.0f), std::make_shared<NextAction>("fireball", 6.0f), std::make_shared<NextAction>("fire blast", 5.0f) });
}

void FireMageStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "pyroblast",
        NextAction::array({ std::make_shared<NextAction>("pyroblast", 10.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hot streak",
        NextAction::array({ std::make_shared<NextAction>("pyroblast", 25.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "combustion",
        NextAction::array({ std::make_shared<NextAction>("combustion", 50.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for spell",
        NextAction::array({ std::make_shared<NextAction>("dragon's breath", 70.0f) })));
}

void FireMageAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("flamestrike", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "living bomb",
        NextAction::array({ std::make_shared<NextAction>("living bomb", 25.0f) })));
}

