
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "ShamanNonCombatStrategy.h"

using namespace ai;

void ShamanNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member dead",
        NextAction::array({ std::make_shared<NextAction>("ancestral spirit", 33.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "water breathing",
        NextAction::array({ std::make_shared<NextAction>("water breathing", 12.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "water walking",
        NextAction::array({ std::make_shared<NextAction>("water walking", 12.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "water breathing on party",
        NextAction::array({ std::make_shared<NextAction>("water breathing on party", 11.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "water walking on party",
        NextAction::array({ std::make_shared<NextAction>("water walking on party", 11.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("healing wave", 70.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member critical health",
        NextAction::array({ std::make_shared<NextAction>("healing wave on party", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe heal",
        NextAction::array({ std::make_shared<NextAction>("chain heal", 27.0f) })));
}

void ShamanNonCombatStrategy::InitMultipliers(std::list<std::shared_ptr<Multiplier>> &multipliers)
{
    NonCombatStrategy::InitMultipliers(multipliers);
}

