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
    static std::shared_ptr<ActionNode> faerie_fire(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("faerie fire",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> hibernate(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("hibernate",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("entangling roots") }),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("flee", 49.0f) }));
    }
    static std::shared_ptr<ActionNode> entangling_roots(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("entangling roots",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("flee", 49.0f) }));
    }
    static std::shared_ptr<ActionNode> entangling_roots_on_cc(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("entangling roots on cc",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> wrath(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("wrath",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> starfall(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("starfall",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("hurricane") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> insect_swarm(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("insect swarm",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> moonfire(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("moonfire",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> starfire(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("starfire",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> natures_grasp(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("nature's grasp",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("moonkin form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
};

CasterDruidStrategy::CasterDruidStrategy(PlayerbotAI* ai) : GenericDruidStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<CasterDruidStrategyActionNodeFactory>());
    actionNodeFactories.Add(std::make_unique<ShapeshiftDruidStrategyActionNodeFactory>());
}

ActionList CasterDruidStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("starfire", ACTION_NORMAL + 2), std::make_shared<NextAction>("wrath", ACTION_NORMAL + 1) });
}

void CasterDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_MOVE) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("regrowth", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member medium health",
        NextAction::array({ std::make_shared<NextAction>("regrowth on party", ACTION_MEDIUM_HEAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "almost full health",
        NextAction::array({ std::make_shared<NextAction>("rejuvenation", ACTION_LIGHT_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member almost full health",
        NextAction::array({ std::make_shared<NextAction>("rejuvenation on party", ACTION_LIGHT_HEAL + 1) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "insect swarm",
        NextAction::array({ std::make_shared<NextAction>("insect swarm", ACTION_NORMAL + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "moonfire",
        NextAction::array({ std::make_shared<NextAction>("moonfire", ACTION_NORMAL + 4) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "eclipse (solar)",
        NextAction::array({ std::make_shared<NextAction>("wrath", ACTION_NORMAL + 6) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "eclipse (lunar)",
        NextAction::array({ std::make_shared<NextAction>("starfire", ACTION_NORMAL + 6) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "moonfire",
        NextAction::array({ std::make_shared<NextAction>("moonfire", ACTION_NORMAL + 4) })));



    triggers.push_back(std::make_shared<TriggerNode>(
        "nature's grasp",
        NextAction::array({ std::make_shared<NextAction>("nature's grasp", ACTION_EMERGENCY) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "entangling roots",
        NextAction::array({ std::make_shared<NextAction>("entangling roots on cc", ACTION_HIGH + 2) })));
}

void CasterDruidAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "high aoe",
        NextAction::array({ std::make_shared<NextAction>("starfall", ACTION_HIGH + 1) })));
}

void CasterDruidDebuffStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "faerie fire",
        NextAction::array({ std::make_shared<NextAction>("faerie fire", ACTION_HIGH) })));
}
