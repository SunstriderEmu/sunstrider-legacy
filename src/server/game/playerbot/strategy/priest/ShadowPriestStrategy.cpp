
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "ShadowPriestStrategy.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

using namespace ai;

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* ai) : GenericPriestStrategy(ai)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

ActionList ShadowPriestStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("mind blast", 10.0f) });
}

void ShadowPriestStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(new TriggerNode(
        "shadowform",
        NextAction::array({ new NextAction("shadowform", 15.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("dispersion", ACTION_EMERGENCY + 5) })));

    triggers.push_back(new TriggerNode(
        "vampiric embrace",
        NextAction::array({ new NextAction("vampiric embrace", 16.0f) })));
}

void ShadowPriestAoeStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array({ new NextAction("shadow word: pain on attacker", 11.0f) })));
}

void ShadowPriestDebuffStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "devouring plague",
        NextAction::array({ new NextAction("devouring plague", 13.0f) })));

    triggers.push_back(new TriggerNode(
        "vampiric touch",
        NextAction::array({ new NextAction("vampiric touch", 11.0f) })));

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array({ new NextAction("shadow word: pain", 12.0f) })));
}
