
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "FireMageStrategy.h"

using namespace ai;

ActionList FireMageStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("scorch", 7.0f), new NextAction("fireball", 6.0f), new NextAction("fire blast", 5.0f) });
}

void FireMageStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "pyroblast",
        NextAction::array({ new NextAction("pyroblast", 10.0f) })));

    triggers.push_back(new TriggerNode(
        "hot streak",
        NextAction::array({ new NextAction("pyroblast", 25.0f) })));

    triggers.push_back(new TriggerNode(
        "combustion",
        NextAction::array({ new NextAction("combustion", 50.0f) })));

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array({ new NextAction("dragon's breath", 70.0f) })));
}

void FireMageAoeStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("flamestrike", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "living bomb",
        NextAction::array({ new NextAction("living bomb", 25.0f) })));
}

