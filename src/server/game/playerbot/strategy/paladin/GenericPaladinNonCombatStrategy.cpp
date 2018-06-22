
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "GenericPaladinNonCombatStrategy.h"
#include "GenericPaladinStrategyActionNodeFactory.h"

using namespace ai;

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericPaladinStrategyActionNodeFactory>());
}

void GenericPaladinNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "blessing of kings on party",
        NextAction::array({ std::make_shared<NextAction>("blessing of kings on party", 11.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member dead",
        NextAction::array({ std::make_shared<NextAction>("redemption", 30.0f) })));

     triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("flash of light", 25.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("flash of light on party", 26.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("holy light", 50.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member low health",
        NextAction::array({ std::make_shared<NextAction>("holy light on party", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse cure disease",
        NextAction::array({ std::make_shared<NextAction>("cleanse disease", 41.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse party member cure disease",
        NextAction::array({ std::make_shared<NextAction>("cleanse disease on party", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse cure poison",
        NextAction::array({ std::make_shared<NextAction>("cleanse poison", 41.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse party member cure poison",
        NextAction::array({ std::make_shared<NextAction>("cleanse poison on party", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse cure magic",
        NextAction::array({ std::make_shared<NextAction>("cleanse magic", 41.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse party member cure magic",
        NextAction::array({ std::make_shared<NextAction>("cleanse magic on party", 40.0f) })));
}
