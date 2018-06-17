
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
    static ActionNode* frostbolt(PlayerbotAI* ai)
    {
        return new ActionNode ("frostbolt",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("shoot") }),
            /*C*/ {});
    }
    static ActionNode* fire_blast(PlayerbotAI* ai)
    {
        return new ActionNode ("fire blast",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("scorch") }),
            /*C*/ {});
    }
    static ActionNode* scorch(PlayerbotAI* ai)
    {
        return new ActionNode ("scorch",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("shoot") }),
            /*C*/ {});
    }
    static ActionNode* frost_nova(PlayerbotAI* ai)
    {
        return new ActionNode ("frost nova",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("flee") }),
            /*C*/ NextAction::array({ new NextAction("flee") }));
    }
    static ActionNode* icy_veins(PlayerbotAI* ai)
    {
        return new ActionNode ("icy veins",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* combustion(PlayerbotAI* ai)
    {
        return new ActionNode ("combustion",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* evocation(PlayerbotAI* ai)
    {
        return new ActionNode ("evocation",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("mana potion") }),
            /*C*/ {});
    }
    static ActionNode* dragons_breath(PlayerbotAI* ai)
    {
        return new ActionNode ("dragon's breath",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("blast wave") }),
            /*C*/ NextAction::array({ new NextAction("flamestrike", 71.0f) }));
    }
    static ActionNode* blast_wave(PlayerbotAI* ai)
    {
        return new ActionNode ("blast wave",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("frost nova") }),
            /*C*/ NextAction::array({ new NextAction("flamestrike", 71.0f) }));
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* ai) : RangedCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array({ new NextAction("remove curse", 41.0f) })));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array({ new NextAction("remove curse on party", 40.0f) })));

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array({ new NextAction("frost nova", 50.0f) })));

    triggers.push_back(new TriggerNode(
        "counterspell",
        NextAction::array({ new NextAction("counterspell", 40.0f) })));

    triggers.push_back(new TriggerNode(
        "counterspell on enemy healer",
        NextAction::array({ new NextAction("counterspell on enemy healer", 40.0f) })));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("ice block", 80.0f) })));

    triggers.push_back(new TriggerNode(
        "polymorph",
        NextAction::array({ new NextAction("polymorph", 30.0f) })));

    triggers.push_back(new TriggerNode(
        "spellsteal",
        NextAction::array({ new NextAction("spellsteal", 40.0f) })));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array({ new NextAction("invisibility", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("evocation", ACTION_EMERGENCY + 5) })));
}
