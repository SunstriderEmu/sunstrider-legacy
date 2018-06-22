
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "HealPriestStrategy.h"
#include "GenericPriestStrategyActionNodeFactory.h"

using namespace ai;

GenericPriestStrategy::GenericPriestStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericPriestStrategyActionNodeFactory>());
}

void GenericPriestStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("flash heal", 25.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("flash heal on party", 20.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("power word: shield", 70.0f), std::make_shared<NextAction>("flash heal", 70.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member critical health",
        NextAction::array({ std::make_shared<NextAction>("power word: shield on party", 60.0f), std::make_shared<NextAction>("flash heal on party", 60.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("power word: shield", 60.0f), std::make_shared<NextAction>("greater heal", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member low health",
        NextAction::array({ std::make_shared<NextAction>("power word: shield on party", 50.0f), std::make_shared<NextAction>("greater heal on party", 50.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "dispel magic",
        NextAction::array({ std::make_shared<NextAction>("dispel magic", 41.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "dispel magic on party",
        NextAction::array({ std::make_shared<NextAction>("dispel magic on party", 40.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "cure disease",
        NextAction::array({ std::make_shared<NextAction>("abolish disease", 31.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member cure disease",
        NextAction::array({ std::make_shared<NextAction>("abolish disease on party", 30.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "medium threat",
        NextAction::array({ std::make_shared<NextAction>("psychic scream", 50.0f) })));

}
