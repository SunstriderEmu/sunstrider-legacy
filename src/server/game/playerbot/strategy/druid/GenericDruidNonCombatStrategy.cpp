
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
    static ActionNode* mark_of_the_wild(PlayerbotAI* ai)
    {
        return new ActionNode ("mark of the wild",
            /*P*/ NextAction::array({ new NextAction("caster form") }),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* mark_of_the_wild_on_party(PlayerbotAI* ai)
    {
        return new ActionNode ("mark of the wild on party",
            /*P*/ NextAction::array({ new NextAction("caster form") }),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* innervate(PlayerbotAI* ai)
    {
        return new ActionNode ("innervate",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("drink") }),
            /*C*/ NULL);
    }
};

GenericDruidNonCombatStrategy::GenericDruidNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "mark of the wild",
        NextAction::array({ new NextAction("mark of the wild", 12.0f) })));

    triggers.push_back(new TriggerNode(
        "mark of the wild on party",
        NextAction::array({ new NextAction("mark of the wild on party", 11.0f) })));

    triggers.push_back(new TriggerNode(
        "cure poison",
        NextAction::array({ new NextAction("abolish poison", 21.0f) })));

    triggers.push_back(new TriggerNode(
        "party member cure poison",
        NextAction::array({ new NextAction("abolish poison on party", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "party member dead",
        NextAction::array({ new NextAction("revive", 22.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("innervate", ACTION_EMERGENCY + 5) })));
}
