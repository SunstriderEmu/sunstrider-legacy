
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "PriestNonCombatStrategy.h"
#include "PriestNonCombatStrategyActionNodeFactory.h"

using namespace ai;

PriestNonCombatStrategy::PriestNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new PriestNonCombatStrategyActionNodeFactory());
}

void PriestNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "power word: fortitude",
        NextAction::array({ new NextAction("power word: fortitude", 12.0f) })));

    triggers.push_back(new TriggerNode(
        "power word: fortitude on party",
        NextAction::array({ new NextAction("power word: fortitude on party", 11.0f) })));


    triggers.push_back(new TriggerNode(
        "divine spirit",
        NextAction::array({ new NextAction("divine spirit", 14.0f) })));

    triggers.push_back(new TriggerNode(
        "divine spirit on party",
        NextAction::array({ new NextAction("divine spirit on party", 13.0f) })));


    triggers.push_back(new TriggerNode(
        "inner fire",
        NextAction::array({ new NextAction("inner fire", 10.0f) })));


    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("power word: shield", 70.0f), new NextAction("greater heal", 70.0f) })));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array({ new NextAction("power word: shield on party", 60.0f), new NextAction("greater heal on party", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array({ new NextAction("flash heal", 21.0f) })));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array({ new NextAction("flash heal on party", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array({ new NextAction("circle of healing", 27.0f) })));

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array({ new NextAction("resurrection", 30.0f) })));


    triggers.push_back(new TriggerNode(
        "dispel magic",
        NextAction::array({ new NextAction("dispel magic", 41.0f) })));

    triggers.push_back(new TriggerNode(
        "dispel magic on party",
        NextAction::array({ new NextAction("dispel magic on party", 40.0f) })));


    triggers.push_back(new TriggerNode(
        "cure disease",
        NextAction::array({ new NextAction("abolish disease", 31.0f) })));

    triggers.push_back(new TriggerNode(
        "party member cure disease",
        NextAction::array({ new NextAction("abolish disease on party", 30.0f) })));
}
