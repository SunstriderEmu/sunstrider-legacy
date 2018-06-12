
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "ShamanNonCombatStrategy.h"

using namespace ai;

void ShamanNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array({ new NextAction("ancestral spirit", 33.0f) })));

    triggers.push_back(new TriggerNode(
        "water breathing",
        NextAction::array({ new NextAction("water breathing", 12.0f) })));

    triggers.push_back(new TriggerNode(
        "water walking",
        NextAction::array({ new NextAction("water walking", 12.0f) })));

    triggers.push_back(new TriggerNode(
        "water breathing on party",
        NextAction::array({ new NextAction("water breathing on party", 11.0f) })));

    triggers.push_back(new TriggerNode(
        "water walking on party",
        NextAction::array({ new NextAction("water walking on party", 11.0f) })));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("healing wave", 70.0f) })));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array({ new NextAction("healing wave on party", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array({ new NextAction("chain heal", 27.0f) })));
}

void ShamanNonCombatStrategy::InitMultipliers(std::list<Multiplier*> &multipliers)
{
    NonCombatStrategy::InitMultipliers(multipliers);
}

