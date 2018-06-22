
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
    static std::shared_ptr<ActionNode> summon_voidwalker(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("summon voidwalker",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("drain soul") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> banish(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("banish",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("fear") }),
            /*C*/ ActionList());
    }
};

GenericWarlockStrategy::GenericWarlockStrategy(PlayerbotAI* ai) : RangedCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericWarlockStrategyActionNodeFactory>());
}

ActionList GenericWarlockStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("shoot", 10.0f) });
}

void GenericWarlockStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "curse of agony",
        NextAction::array({ std::make_shared<NextAction>("curse of agony", 11.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("drain life", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("life tap", ACTION_EMERGENCY + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "target critical health",
        NextAction::array({ std::make_shared<NextAction>("drain soul", 30.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "banish",
        NextAction::array({ std::make_shared<NextAction>("banish", 21.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "fear",
        NextAction::array({ std::make_shared<NextAction>("fear on cc", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "immolate",
        NextAction::array({ std::make_shared<NextAction>("immolate", 19.0f), std::make_shared<NextAction>("conflagrate", 19.0f) })));
}
