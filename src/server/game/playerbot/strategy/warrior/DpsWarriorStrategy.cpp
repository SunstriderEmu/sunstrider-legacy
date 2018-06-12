//
#include "../../playerbot.h"
#include "WarriorMultipliers.h"
#include "DpsWarriorStrategy.h"

using namespace ai;

class DpsWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsWarriorStrategyActionNodeFactory()
    {
        creators["overpower"] = &overpower;
        creators["melee"] = &melee;
        creators["charge"] = &charge;
        creators["bloodthirst"] = &bloodthirst;
        creators["rend"] = &rend;
        creators["mocking blow"] = &mocking_blow;
        creators["death wish"] = &death_wish;
        creators["execute"] = &execute;
    }
private:
    static ActionNode* overpower(PlayerbotAI* ai)
    {
        return new ActionNode ("overpower",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ NULL);
    }
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode ("melee",
            /*P*/ NextAction::array({ new NextAction("charge") }),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* charge(PlayerbotAI* ai)
    {
        return new ActionNode ("charge",
            /*P*/ NextAction::array({ new NextAction("battle stance") }),
            /*A*/ NextAction::array({ new NextAction("reach melee") }),
            /*C*/ NULL);
    }
    static ActionNode* bloodthirst(PlayerbotAI* ai)
    {
        return new ActionNode ("bloodthirst",
            /*P*/ NextAction::array({ new NextAction("battle stance") }),
            /*A*/ NextAction::array({ new NextAction("heroic strike") }),
            /*C*/ NULL);
    }
    static ActionNode* rend(PlayerbotAI* ai)
    {
        return new ActionNode ("rend",
            /*P*/ NextAction::array({ new NextAction("battle stance") }),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* mocking_blow(PlayerbotAI* ai)
    {
        return new ActionNode ("mocking blow",
            /*P*/ NextAction::array({ new NextAction("battle stance") }),
            /*A*/ NextAction::array({ }),
            /*C*/ NULL);
    }
    static ActionNode* death_wish(PlayerbotAI* ai)
    {
        return new ActionNode ("death wish",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("berserker rage") }),
            /*C*/ NULL);
    }
    static ActionNode* execute(PlayerbotAI* ai)
    {
        return new ActionNode ("execute",
            /*P*/ NextAction::array({ new NextAction("battle stance") }),
            /*A*/ NextAction::array({ new NextAction("heroic strike") }),
            /*C*/ NULL);
    }
};

DpsWarriorStrategy::DpsWarriorStrategy(PlayerbotAI* ai) : GenericWarriorStrategy(ai)
{
    actionNodeFactories.Add(new DpsWarriorStrategyActionNodeFactory());
}

NextAction** DpsWarriorStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("bloodthirst", ACTION_NORMAL + 1) });
}

void DpsWarriorStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array({ new NextAction("charge", ACTION_NORMAL + 9) })));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array({ new NextAction("execute", ACTION_HIGH + 4) })));

    triggers.push_back(new TriggerNode(
        "hamstring",
        NextAction::array({ new NextAction("hamstring", ACTION_INTERRUPT) })));

    triggers.push_back(new TriggerNode(
        "victory rush",
        NextAction::array({ new NextAction("victory rush", ACTION_HIGH + 3) })));

    triggers.push_back(new TriggerNode(
        "death wish",
        NextAction::array({ new NextAction("death wish", ACTION_HIGH + 2) })));
}


void DpsWarrirorAoeStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "rend on attacker",
        NextAction::array({ new NextAction("rend on attacker", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array({ new NextAction("thunder clap", ACTION_HIGH + 2), new NextAction("demoralizing shout", ACTION_HIGH + 2) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("cleave", ACTION_HIGH + 3) })));
}
