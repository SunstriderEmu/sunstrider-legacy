
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "PriestNonCombatStrategy.h"
#include "PriestNonCombatStrategyActionNodeFactory.h"

using namespace ai;

PriestNonCombatStrategy::PriestNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<PriestNonCombatStrategyActionNodeFactory>());
}

void PriestNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "power word: fortitude",
        NextAction::array({ std::make_shared<NextAction>("power word: fortitude", 12.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "power word: fortitude on party",
        NextAction::array({ std::make_shared<NextAction>("power word: fortitude on party", 11.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "divine spirit",
        NextAction::array({ std::make_shared<NextAction>("divine spirit", 14.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "divine spirit on party",
        NextAction::array({ std::make_shared<NextAction>("divine spirit on party", 13.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "inner fire",
        NextAction::array({ std::make_shared<NextAction>("inner fire", 10.0f) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("power word: shield", 70.0f), std::make_shared<NextAction>("greater heal", 70.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member critical health",
        NextAction::array({ std::make_shared<NextAction>("power word: shield on party", 60.0f), std::make_shared<NextAction>("greater heal on party", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("flash heal", 21.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member low health",
        NextAction::array({ std::make_shared<NextAction>("flash heal on party", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe heal",
        NextAction::array({ std::make_shared<NextAction>("circle of healing", 27.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member dead",
        NextAction::array({ std::make_shared<NextAction>("resurrection", 30.0f) })));


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
}
