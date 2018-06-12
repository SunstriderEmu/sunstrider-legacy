
#include "../../playerbot.h"
#include "GenericPaladinStrategy.h"
#include "GenericPaladinStrategyActionNodeFactory.h"

using namespace ai;


GenericPaladinStrategy::GenericPaladinStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array({ new NextAction("flash of light", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array({ new NextAction("flash of light on party", ACTION_MEDIUM_HEAL + 1) })));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array({ new NextAction("divine protection", ACTION_CRITICAL_HEAL + 2), new NextAction("holy light", ACTION_CRITICAL_HEAL + 2) })));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array({ new NextAction("holy light on party", ACTION_CRITICAL_HEAL + 1) })));

    triggers.push_back(new TriggerNode(
        "hammer of justice interrupt",
        NextAction::array({ new NextAction("hammer of justice", ACTION_INTERRUPT) })));

    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        NextAction::array({ new NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT) })));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("lay on hands", ACTION_EMERGENCY) })));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array({ new NextAction("lay on hands on party", ACTION_EMERGENCY) })));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array({ new NextAction("hammer of wrath", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "cleanse cure disease",
        NextAction::array({ new NextAction("cleanse disease", ACTION_DISPEL + 2) })));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure disease",
        NextAction::array({ new NextAction("cleanse disease on party", ACTION_DISPEL + 1) })));

    triggers.push_back(new TriggerNode(
        "cleanse cure poison",
        NextAction::array({ new NextAction("cleanse poison", ACTION_DISPEL + 2) })));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure poison",
        NextAction::array({ new NextAction("cleanse poison on party", ACTION_DISPEL + 1) })));

    triggers.push_back(new TriggerNode(
        "cleanse cure magic",
        NextAction::array({ new NextAction("cleanse magic", ACTION_DISPEL + 2) })));

    triggers.push_back(new TriggerNode(
        "cleanse party member cure magic",
        NextAction::array({ new NextAction("cleanse magic on party", ACTION_DISPEL + 1) })));
}
