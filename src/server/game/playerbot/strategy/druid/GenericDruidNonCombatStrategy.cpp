
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "GenericDruidNonCombatStrategy.h"

using namespace ai;

class GenericDruidNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidNonCombatStrategyActionNodeFactory()
    {
        creators["mark of the wild"] = &mark_of_the_wild;
        creators["mark of the wild on party"] = &mark_of_the_wild_on_party;
        creators["innervate"] = &innervate;
    }
private:
    static std::shared_ptr<ActionNode> mark_of_the_wild(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mark of the wild",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mark_of_the_wild_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mark of the wild on party",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> innervate(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("innervate",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("drink") }),
            /*C*/ ActionList());
    }
};

GenericDruidNonCombatStrategy::GenericDruidNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericDruidNonCombatStrategyActionNodeFactory>());
}

void GenericDruidNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "mark of the wild",
        NextAction::array({ std::make_shared<NextAction>("mark of the wild", 12.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "mark of the wild on party",
        NextAction::array({ std::make_shared<NextAction>("mark of the wild on party", 11.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cure poison",
        NextAction::array({ std::make_shared<NextAction>("abolish poison", 21.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member cure poison",
        NextAction::array({ std::make_shared<NextAction>("abolish poison on party", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member dead",
        NextAction::array({ std::make_shared<NextAction>("revive", 22.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("innervate", ACTION_EMERGENCY + 5) })));
}
