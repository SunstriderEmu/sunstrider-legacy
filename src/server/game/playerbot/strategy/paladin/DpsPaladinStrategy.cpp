//
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "DpsPaladinStrategy.h"

using namespace ai;

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
    }

private:
    static ActionNode* seal_of_vengeance(PlayerbotAI* ai)
    {
        return new ActionNode ("seal of vengeance",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("seal of command") }),
            /*C*/ {});
    }
    static ActionNode* seal_of_command(PlayerbotAI* ai)
    {
        return new ActionNode ("seal of command",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("seal of wisdom") }),
            /*C*/ {});
    }
    static ActionNode* blessing_of_might(PlayerbotAI* ai)
    {
        return new ActionNode ("blessing of might",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("blessing of kings") }),
            /*C*/ {});
    }
    static ActionNode* crusader_strike(PlayerbotAI* ai)
    {
        return new ActionNode ("crusader strike",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

ActionList DpsPaladinStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("crusader strike", ACTION_NORMAL + 1) });
}

void DpsPaladinStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);
    
    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array({ new NextAction("divine shield", ACTION_CRITICAL_HEAL + 2), new NextAction("holy light", ACTION_CRITICAL_HEAL + 2) })));

    triggers.push_back(new TriggerNode(
        "judgement of wisdom",
        NextAction::array({ new NextAction("judgement of wisdom", ACTION_NORMAL + 2) })));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array({ new NextAction("blessing of might", ACTION_HIGH + 8) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("divine storm", ACTION_HIGH + 1), new NextAction("consecration", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "art of war",
        NextAction::array({ new NextAction("exorcism", ACTION_HIGH + 2) })));
}
