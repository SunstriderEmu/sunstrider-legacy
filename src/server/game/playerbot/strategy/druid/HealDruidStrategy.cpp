
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "HealDruidStrategy.h"

using namespace ai;

class HealDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealDruidStrategyActionNodeFactory()
    {
    }
private:
};

HealDruidStrategy::HealDruidStrategy(PlayerbotAI* ai) : GenericDruidStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<HealDruidStrategyActionNodeFactory>());
}

void HealDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "tree form",
        NextAction::array({ std::make_shared<NextAction>("tree form", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("regrowth", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("regrowth on party", ACTION_MEDIUM_HEAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "almost full health",
        NextAction::array({ std::make_shared<NextAction>("rejuvenation", ACTION_LIGHT_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member almost full health",
        NextAction::array({ std::make_shared<NextAction>("rejuvenation on party", ACTION_LIGHT_HEAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe heal",
        NextAction::array({ std::make_shared<NextAction>("tranquility", ACTION_MEDIUM_HEAL + 3) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "entangling roots",
        NextAction::array({ std::make_shared<NextAction>("entangling roots on cc", ACTION_HIGH + 1) })));
}
