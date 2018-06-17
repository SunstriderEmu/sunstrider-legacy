////
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "CasterDruidStrategy.h"
#include "FeralDruidStrategy.h"

using namespace ai;

class CasterDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterDruidStrategyActionNodeFactory()
    {
        creators["faerie fire"] = &faerie_fire;
        creators["hibernate"] = &hibernate;
        creators["entangling roots"] = &entangling_roots;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["wrath"] = &wrath;
        creators["starfall"] = &starfall;
        creators["insect swarm"] = &insect_swarm;
        creators["moonfire"] = &moonfire;
        creators["starfire"] = &starfire;
        creators["nature's grasp"] = &natures_grasp;
    }
private:
    static ActionNode* faerie_fire(PlayerbotAI* ai)
    {
        return new ActionNode ("faerie fire",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* hibernate(PlayerbotAI* ai)
    {
        return new ActionNode ("hibernate",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ NextAction::array({ new NextAction("entangling roots") }),
            /*C*/ NextAction::array({ new NextAction("flee", 49.0f) }));
    }
    static ActionNode* entangling_roots(PlayerbotAI* ai)
    {
        return new ActionNode ("entangling roots",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ NextAction::array({ new NextAction("flee", 49.0f) }));
    }
    static ActionNode* entangling_roots_on_cc(PlayerbotAI* ai)
    {
        return new ActionNode ("entangling roots on cc",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* wrath(PlayerbotAI* ai)
    {
        return new ActionNode ("wrath",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* starfall(PlayerbotAI* ai)
    {
        return new ActionNode ("starfall",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ NextAction::array({ new NextAction("hurricane") }),
            /*C*/ {});
    }
    static ActionNode* insect_swarm(PlayerbotAI* ai)
    {
        return new ActionNode ("insect swarm",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* moonfire(PlayerbotAI* ai)
    {
        return new ActionNode ("moonfire",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* starfire(PlayerbotAI* ai)
    {
        return new ActionNode ("starfire",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* natures_grasp(PlayerbotAI* ai)
    {
        return new ActionNode ("nature's grasp",
            /*P*/ NextAction::array({ new NextAction("moonkin form") }),
            /*A*/ {},
            /*C*/ {});
    }
};

CasterDruidStrategy::CasterDruidStrategy(PlayerbotAI* ai) : GenericDruidStrategy(ai)
{
    actionNodeFactories.Add(new CasterDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

ActionList CasterDruidStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("starfire", ACTION_NORMAL + 2), new NextAction("wrath", ACTION_NORMAL + 1) });
}

void CasterDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_MOVE) })));

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
        "insect swarm",
        NextAction::array({ new NextAction("insect swarm", ACTION_NORMAL + 5) })));

    triggers.push_back(new TriggerNode(
        "moonfire",
        NextAction::array({ new NextAction("moonfire", ACTION_NORMAL + 4) })));

    triggers.push_back(new TriggerNode(
        "eclipse (solar)",
        NextAction::array({ new NextAction("wrath", ACTION_NORMAL + 6) })));

    triggers.push_back(new TriggerNode(
        "eclipse (lunar)",
        NextAction::array({ new NextAction("starfire", ACTION_NORMAL + 6) })));

    triggers.push_back(new TriggerNode(
        "moonfire",
        NextAction::array({ new NextAction("moonfire", ACTION_NORMAL + 4) })));



    triggers.push_back(new TriggerNode(
        "nature's grasp",
        NextAction::array({ new NextAction("nature's grasp", ACTION_EMERGENCY) })));

    triggers.push_back(new TriggerNode(
        "entangling roots",
        NextAction::array({ new NextAction("entangling roots on cc", ACTION_HIGH + 2) })));
}

void CasterDruidAoeStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "high aoe",
        NextAction::array({ new NextAction("starfall", ACTION_HIGH + 1) })));
}

void CasterDruidDebuffStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "faerie fire",
        NextAction::array({ new NextAction("faerie fire", ACTION_HIGH) })));
}
