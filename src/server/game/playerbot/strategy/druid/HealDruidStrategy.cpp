
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
    actionNodeFactories.Add(new HealDruidStrategyActionNodeFactory());
}

void HealDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(new TriggerNode(
        "tree form",
        NextAction::array({ new NextAction("tree form", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array({ new NextAction("regrowth", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array({ new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1) })));

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array({ new NextAction("rejuvenation", ACTION_LIGHT_HEAL + 2) })));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array({ new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 1) })));

    triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array({ new NextAction("tranquility", ACTION_MEDIUM_HEAL + 3) })));

    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array({ new NextAction("entangling roots on cc", ACTION_HIGH + 1) })));
}
