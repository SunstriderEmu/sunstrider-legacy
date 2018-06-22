
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "ShadowPriestStrategy.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

using namespace ai;

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* ai) : GenericPriestStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<ShadowPriestStrategyActionNodeFactory>());
}

ActionList ShadowPriestStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("mind blast", 10.0f) });
}

void ShadowPriestStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shadowform",
        NextAction::array({ std::make_shared<NextAction>("shadowform", 15.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("dispersion", ACTION_EMERGENCY + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "vampiric embrace",
        NextAction::array({ std::make_shared<NextAction>("vampiric embrace", 16.0f) })));
}

void ShadowPriestAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "shadow word: pain on attacker",
        NextAction::array({ std::make_shared<NextAction>("shadow word: pain on attacker", 11.0f) })));
}

void ShadowPriestDebuffStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "devouring plague",
        NextAction::array({ std::make_shared<NextAction>("devouring plague", 13.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "vampiric touch",
        NextAction::array({ std::make_shared<NextAction>("vampiric touch", 11.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shadow word: pain",
        NextAction::array({ std::make_shared<NextAction>("shadow word: pain", 12.0f) })));
}
