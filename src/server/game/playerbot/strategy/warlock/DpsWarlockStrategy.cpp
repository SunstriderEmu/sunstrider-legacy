//
#include "../../playerbot.h"
#include "WarlockTriggers.h"
#include "WarlockMultipliers.h"
#include "DpsWarlockStrategy.h"
#include "WarlockActions.h"

using namespace ai;

class DpsWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsWarlockStrategyActionNodeFactory()
    {
        creators["shadow bolt"] = &shadow_bolt;
    }
private:
    static std::shared_ptr<ActionNode> shadow_bolt(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("shadow bolt",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("shoot") }),
            /*C*/ ActionList());
    }
};

DpsWarlockStrategy::DpsWarlockStrategy(PlayerbotAI* ai) : GenericWarlockStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<DpsWarlockStrategyActionNodeFactory>());
}


ActionList DpsWarlockStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("incinirate", 10.0f), std::make_shared<NextAction>("shadow bolt", 10.0f) });
}

void DpsWarlockStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "shadow trance",
        NextAction::array({ std::make_shared<NextAction>("shadow bolt", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "backlash",
        NextAction::array({ std::make_shared<NextAction>("shadow bolt", 20.0f) })));
}

void DpsAoeWarlockStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "high aoe",
        NextAction::array({ std::make_shared<NextAction>("rain of fire", 30.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("seed of corruption", 31.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "light aoe",
        NextAction::array({ std::make_shared<NextAction>("shadowfury", 29.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "corruption on attacker",
        NextAction::array({ std::make_shared<NextAction>("corruption on attacker", 28.0f) })));

}

void DpsWarlockDebuffStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "corruption",
        NextAction::array({ std::make_shared<NextAction>("corruption", 12.0f) })));
}
