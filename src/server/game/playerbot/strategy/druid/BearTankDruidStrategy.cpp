////
#include "../../playerbot.h"
#include "DruidMultipliers.h"
#include "BearTankDruidStrategy.h"

using namespace ai;

class BearTankDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BearTankDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["feral charge - bear"] = &feral_charge_bear;
        creators["swipe (bear)"] = &swipe_bear;
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["bear form"] = &bear_form;
        creators["dire bear form"] = &dire_bear_form;
        creators["mangle (bear)"] = &mangle_bear;
        creators["maul"] = &maul;
        creators["bash"] = &bash;
        creators["swipe"] = &swipe;
        creators["lacerate"] = &lacerate;
        creators["demoralizing roar"] = &demoralizing_roar;
    }
private:
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode ("melee",
            /*P*/ NextAction::array({ new NextAction("feral charge - bear") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* feral_charge_bear(PlayerbotAI* ai)
    {
        return new ActionNode ("feral charge - bear",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("reach melee") }),
            /*C*/ {});
    }
    static ActionNode* swipe_bear(PlayerbotAI* ai)
    {
        return new ActionNode ("swipe (bear)",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* faerie_fire_feral(PlayerbotAI* ai)
    {
        return new ActionNode ("faerie fire (feral)",
            /*P*/ NextAction::array({ new NextAction("feral charge - bear") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* bear_form(PlayerbotAI* ai)
    {
        return new ActionNode ("bear form",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* dire_bear_form(PlayerbotAI* ai)
    {
        return new ActionNode ("dire bear form",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("bear form") }),
            /*C*/ {});
    }
    static ActionNode* mangle_bear(PlayerbotAI* ai)
    {
        return new ActionNode ("mangle (bear)",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("lacerate") }),
            /*C*/ {});
    }
    static ActionNode* maul(PlayerbotAI* ai)
    {
        return new ActionNode ("maul",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
    static ActionNode* bash(PlayerbotAI* ai)
    {
        return new ActionNode ("bash",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
    static ActionNode* swipe(PlayerbotAI* ai)
    {
        return new ActionNode ("swipe",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
    static ActionNode* lacerate(PlayerbotAI* ai)
    {
        return new ActionNode ("lacerate",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("maul") }),
            /*C*/ {});
    }
    static ActionNode* growl(PlayerbotAI* ai)
    {
        return new ActionNode ("growl",
            /*P*/ NextAction::array({ new NextAction("reach spell") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* demoralizing_roar(PlayerbotAI* ai)
    {
        return new ActionNode ("demoralizing roar",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
};

BearTankDruidStrategy::BearTankDruidStrategy(PlayerbotAI* ai) : FeralDruidStrategy(ai)
{
    actionNodeFactories.Add(new BearTankDruidStrategyActionNodeFactory());
}

ActionList BearTankDruidStrategy::getDefaultActions()
{
    return NextAction::array({
            new NextAction("lacerate", ACTION_NORMAL + 4),
            new NextAction("mangle (bear)", ACTION_NORMAL + 3),
            new NextAction("maul", ACTION_NORMAL + 2),
            new NextAction("faerie fire (feral)", ACTION_NORMAL + 1) });
}

void BearTankDruidStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "thorns",
        NextAction::array({ new NextAction("thorns", ACTION_HIGH + 9) })));

    triggers.push_back(new TriggerNode(
        "bear form",
        NextAction::array({ new NextAction("dire bear form", ACTION_HIGH + 8) })));

    triggers.push_back(new TriggerNode(
        "faerie fire (feral)",
        NextAction::array({ new NextAction("faerie fire (feral)", ACTION_HIGH + 7) })));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array({ new NextAction("growl", ACTION_HIGH + 8) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("demoralizing roar", ACTION_HIGH + 6), new NextAction("swipe (bear)", ACTION_HIGH + 6) })));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array({ new NextAction("swipe (bear)", ACTION_HIGH + 5) })));

    triggers.push_back(new TriggerNode(
        "bash",
        NextAction::array({ new NextAction("bash", ACTION_INTERRUPT + 2) })));

    triggers.push_back(new TriggerNode(
        "bash on enemy healer",
        NextAction::array({ new NextAction("bash on enemy healer", ACTION_INTERRUPT + 1) })));

}
