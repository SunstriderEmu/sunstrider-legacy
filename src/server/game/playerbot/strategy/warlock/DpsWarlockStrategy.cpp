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
    static ActionNode* shadow_bolt(PlayerbotAI* ai)
    {
        return new ActionNode ("shadow bolt",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("shoot") }),
            /*C*/ NULL);
    }
};

DpsWarlockStrategy::DpsWarlockStrategy(PlayerbotAI* ai) : GenericWarlockStrategy(ai)
{
    actionNodeFactories.Add(new DpsWarlockStrategyActionNodeFactory());
}


NextAction** DpsWarlockStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("incinirate", 10.0f), new NextAction("shadow bolt", 10.0f) });
}

void DpsWarlockStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shadow trance",
        NextAction::array({ new NextAction("shadow bolt", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "backlash",
        NextAction::array({ new NextAction("shadow bolt", 20.0f) })));
}

void DpsAoeWarlockStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "high aoe",
        NextAction::array({ new NextAction("rain of fire", 30.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("seed of corruption", 31.0f) })));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array({ new NextAction("shadowfury", 29.0f) })));

    triggers.push_back(new TriggerNode(
        "corruption on attacker",
        NextAction::array({ new NextAction("corruption on attacker", 28.0f) })));

}

void DpsWarlockDebuffStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "corruption",
        NextAction::array({ new NextAction("corruption", 12.0f) })));
}
