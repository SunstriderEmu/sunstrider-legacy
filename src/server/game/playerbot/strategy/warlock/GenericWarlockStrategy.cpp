
#include "../../playerbot.h"
#include "WarlockMultipliers.h"
#include "GenericWarlockStrategy.h"

using namespace ai;

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["banish"] = &banish;
    }
private:
    static ActionNode* summon_voidwalker(PlayerbotAI* ai)
    {
        return new ActionNode ("summon voidwalker",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("drain soul") }),
            /*C*/ {});
    }
    static ActionNode* banish(PlayerbotAI* ai)
    {
        return new ActionNode ("banish",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("fear") }),
            /*C*/ {});
    }
};

GenericWarlockStrategy::GenericWarlockStrategy(PlayerbotAI* ai) : RangedCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericWarlockStrategyActionNodeFactory());
}

ActionList GenericWarlockStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("shoot", 10.0f) });
}

void GenericWarlockStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "curse of agony",
        NextAction::array({ new NextAction("curse of agony", 11.0f) })));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array({ new NextAction("drain life", 40.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("life tap", ACTION_EMERGENCY + 5) })));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array({ new NextAction("drain soul", 30.0f) })));

    triggers.push_back(new TriggerNode(
        "banish",
        NextAction::array({ new NextAction("banish", 21.0f) })));

    triggers.push_back(new TriggerNode(
        "fear",
        NextAction::array({ new NextAction("fear on cc", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "immolate",
        NextAction::array({ new NextAction("immolate", 19.0f), new NextAction("conflagrate", 19.0f) })));
}
