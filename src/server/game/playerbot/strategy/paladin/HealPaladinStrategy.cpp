#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "HealPaladinStrategy.h"

using namespace ai;

HealPaladinStrategy::HealPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
}

ActionList HealPaladinStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("melee", ACTION_NORMAL ) });
}

void HealPaladinStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("flash of light", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("flash of light on party", ACTION_MEDIUM_HEAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "blessing",
        NextAction::array({ std::make_shared<NextAction>("blessing of sanctuary", ACTION_HIGH + 9) })));
}
