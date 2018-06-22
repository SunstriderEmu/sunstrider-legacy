
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "GenericMageStrategy.h"

using namespace ai;

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["fire blast"] = &fire_blast;
        creators["scorch"] = &scorch;
        creators["frost nova"] = &frost_nova;
        creators["icy veins"] = &icy_veins;
        creators["combustion"] = &combustion;
        creators["evocation"] = &evocation;
        creators["dragon's breath"] = &dragons_breath;
        creators["blast wave"] = &blast_wave;
    }
private:
    static std::shared_ptr<ActionNode> frostbolt(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("frostbolt",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("shoot") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> fire_blast(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("fire blast",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("scorch") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> scorch(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("scorch",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("shoot") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> frost_nova(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("frost nova",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("flee") }),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("flee") }));
    }
    static std::shared_ptr<ActionNode> icy_veins(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("icy veins",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> combustion(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("combustion",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> evocation(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("evocation",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mana potion") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> dragons_breath(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("dragon's breath",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("blast wave") }),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("flamestrike", 71.0f) }));
    }
    static std::shared_ptr<ActionNode> blast_wave(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("blast wave",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("frost nova") }),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("flamestrike", 71.0f) }));
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* ai) : RangedCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericMageStrategyActionNodeFactory>());
}

void GenericMageStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "remove curse",
        NextAction::array({ std::make_shared<NextAction>("remove curse", 41.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "remove curse on party",
        NextAction::array({ std::make_shared<NextAction>("remove curse on party", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for spell",
        NextAction::array({ std::make_shared<NextAction>("frost nova", 50.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "counterspell",
        NextAction::array({ std::make_shared<NextAction>("counterspell", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "counterspell on enemy healer",
        NextAction::array({ std::make_shared<NextAction>("counterspell on enemy healer", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("ice block", 80.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "polymorph",
        NextAction::array({ std::make_shared<NextAction>("polymorph", 30.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "spellsteal",
        NextAction::array({ std::make_shared<NextAction>("spellsteal", 40.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium threat",
        NextAction::array({ std::make_shared<NextAction>("invisibility", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("evocation", ACTION_EMERGENCY + 5) })));
}
