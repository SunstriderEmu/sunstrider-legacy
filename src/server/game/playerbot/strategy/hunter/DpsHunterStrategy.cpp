//
#include "../../playerbot.h"

#include "HunterMultipliers.h"
#include "DpsHunterStrategy.h"

using namespace ai;

class DpsHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsHunterStrategyActionNodeFactory()
    {
        creators["aimed shot"] = &aimed_shot;
        creators["chimera shot"] = &chimera_shot;
        creators["explosive shot"] = &explosive_shot;
        creators["concussive shot"] = &concussive_shot;
        creators["viper sting"] = &viper_sting;
    }
private:
    static ActionNode* viper_sting(PlayerbotAI* ai)
    {
        return new ActionNode ("viper sting",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("mana potion", 10.0f) }),
            /*C*/ {});
    }
    static ActionNode* aimed_shot(PlayerbotAI* ai)
    {
        return new ActionNode ("aimed shot",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("chimera shot", 10.0f) }),
            /*C*/ {});
    }
    static ActionNode* chimera_shot(PlayerbotAI* ai)
    {
        return new ActionNode ("chimera shot",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("arcane shot", 10.0f) }),
            /*C*/ {});
    }
    static ActionNode* explosive_shot(PlayerbotAI* ai)
    {
        return new ActionNode ("explosive shot",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("aimed shot") }),
            /*C*/ {});
    }
    static ActionNode* concussive_shot(PlayerbotAI* ai)
    {
        return new ActionNode ("concussive shot",
            /*P*/ {},
            /*A*/ {},
            /*C*/ NextAction::array({ new NextAction("wyvern sting", 11.0f) }));
    }

};

DpsHunterStrategy::DpsHunterStrategy(PlayerbotAI* ai) : GenericHunterStrategy(ai)
{
    actionNodeFactories.Add(new DpsHunterStrategyActionNodeFactory());
}

ActionList DpsHunterStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("explosive shot", 11.0f), new NextAction("auto shot", 10.0f) });
}

void DpsHunterStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "black arrow",
        NextAction::array({ new NextAction("black arrow", 51.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("viper sting", ACTION_EMERGENCY + 5) })));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array({ new NextAction("call pet", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "hunters pet low health",
        NextAction::array({ new NextAction("mend pet", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "hunter's mark",
        NextAction::array({ new NextAction("hunter's mark", 52.0f) })));

    triggers.push_back(new TriggerNode(
        "freezing trap",
        NextAction::array({ new NextAction("freezing trap", 83.0f) })));
}

void DpsAoeHunterStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("multi-shot", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "high aoe",
        NextAction::array({ new NextAction("volley", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "serpent sting on attacker",
        NextAction::array({ new NextAction("serpent sting on attacker", 49.0f) })));
}

void DpsHunterDebuffStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "no stings",
        NextAction::array({ new NextAction("serpent sting", 50.0f) })));
}
