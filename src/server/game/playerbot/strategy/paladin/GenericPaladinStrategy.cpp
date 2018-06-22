
#include "../../playerbot.h"
#include "GenericPaladinStrategy.h"
#include "GenericPaladinStrategyActionNodeFactory.h"

using namespace ai;


GenericPaladinStrategy::GenericPaladinStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericPaladinStrategyActionNodeFactory>());
}

void GenericPaladinStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("flash of light", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("flash of light on party", ACTION_MEDIUM_HEAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("divine protection", ACTION_CRITICAL_HEAL + 2), std::make_shared<NextAction>("holy light", ACTION_CRITICAL_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member low health",
        NextAction::array({ std::make_shared<NextAction>("holy light on party", ACTION_CRITICAL_HEAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hammer of justice interrupt",
        NextAction::array({ std::make_shared<NextAction>("hammer of justice", ACTION_INTERRUPT) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hammer of justice on enemy healer",
        NextAction::array({ std::make_shared<NextAction>("hammer of justice on enemy healer", ACTION_INTERRUPT) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("lay on hands", ACTION_EMERGENCY) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member critical health",
        NextAction::array({ std::make_shared<NextAction>("lay on hands on party", ACTION_EMERGENCY) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "target critical health",
        NextAction::array({ std::make_shared<NextAction>("hammer of wrath", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse cure disease",
        NextAction::array({ std::make_shared<NextAction>("cleanse disease", ACTION_DISPEL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse party member cure disease",
        NextAction::array({ std::make_shared<NextAction>("cleanse disease on party", ACTION_DISPEL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse cure poison",
        NextAction::array({ std::make_shared<NextAction>("cleanse poison", ACTION_DISPEL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse party member cure poison",
        NextAction::array({ std::make_shared<NextAction>("cleanse poison on party", ACTION_DISPEL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse cure magic",
        NextAction::array({ std::make_shared<NextAction>("cleanse magic", ACTION_DISPEL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse party member cure magic",
        NextAction::array({ std::make_shared<NextAction>("cleanse magic on party", ACTION_DISPEL + 1) })));
}
