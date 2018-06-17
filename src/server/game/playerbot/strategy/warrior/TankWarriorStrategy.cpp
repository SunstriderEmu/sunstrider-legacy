
#include "../../playerbot.h"
#include "WarriorMultipliers.h"
#include "TankWarriorStrategy.h"

using namespace ai;

class TankWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankWarriorStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["shield wall"] = &shield_wall;
        creators["rend"] = &rend;
        creators["revenge"] = &revenge;
        creators["devastate"] = &devastate;
        creators["shockwave"] = &shockwave;
        creators["taunt"] = &taunt;
    }
private:
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode ("melee",
            /*P*/ NextAction::array({ new NextAction("defensive stance"), new NextAction("reach melee") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* shield_wall(PlayerbotAI* ai)
    {
        return new ActionNode ("shield wall",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("shield block") }),
            /*C*/ {});
    }
    static ActionNode* rend(PlayerbotAI* ai)
    {
        return new ActionNode ("rend",
            /*P*/ NextAction::array({ new NextAction("defensive stance") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* revenge(PlayerbotAI* ai)
    {
        return new ActionNode ("revenge",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
    static ActionNode* devastate(PlayerbotAI* ai)
    {
        return new ActionNode ("devastate",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("sunder armor") }),
            /*C*/ {});
    }
    static ActionNode* shockwave(PlayerbotAI* ai)
    {
        return new ActionNode ("shockwave",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("cleave") }),
            /*C*/ {});
    }
    static ActionNode* taunt(PlayerbotAI* ai)
    {
        return new ActionNode ("taunt",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("mocking blow") }),
            /*C*/ {});
    }
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* ai) : GenericWarriorStrategy(ai)
{
    actionNodeFactories.Add(new TankWarriorStrategyActionNodeFactory());
}

ActionList TankWarriorStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("devastate", ACTION_NORMAL + 1), new NextAction("revenge", ACTION_NORMAL + 1) });
}

void TankWarriorStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "medium rage available",
        NextAction::array({ new NextAction("shield slam", ACTION_NORMAL + 2), new NextAction("heroic strike", ACTION_NORMAL + 2) })));

    triggers.push_back(new TriggerNode(
        "disarm",
        NextAction::array({ new NextAction("disarm", ACTION_NORMAL) })));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array({ new NextAction("taunt", ACTION_HIGH + 9) })));

    triggers.push_back(new TriggerNode(
        "medium health",
        NextAction::array({ new NextAction("shield wall", ACTION_MEDIUM_HEAL) })));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("last stand", ACTION_EMERGENCY + 3) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("shockwave", ACTION_HIGH + 2) })));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array({ new NextAction("thunder clap", ACTION_HIGH + 2), new NextAction("demoralizing shout", ACTION_HIGH + 2),  new NextAction("cleave", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "high aoe",
        NextAction::array({ new NextAction("challenging shout", ACTION_HIGH + 3) })));

    triggers.push_back(new TriggerNode(
        "concussion blow",
        NextAction::array({ new NextAction("concussion blow", ACTION_INTERRUPT) })));

    triggers.push_back(new TriggerNode(
        "sword and board",
        NextAction::array({ new NextAction("shield slam", ACTION_HIGH + 3) })));
}
