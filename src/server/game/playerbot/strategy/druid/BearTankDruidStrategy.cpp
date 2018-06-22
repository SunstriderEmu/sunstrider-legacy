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
    static std::shared_ptr<ActionNode> melee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("melee",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("feral charge - bear") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> feral_charge_bear(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("feral charge - bear",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("reach melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> swipe_bear(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("swipe (bear)",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> faerie_fire_feral(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("faerie fire (feral)",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("feral charge - bear") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> bear_form(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("bear form",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> dire_bear_form(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("dire bear form",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("bear form") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mangle_bear(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mangle (bear)",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("lacerate") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> maul(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("maul",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> bash(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("bash",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> swipe(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("swipe",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> lacerate(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("lacerate",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("maul") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> growl(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("growl",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("reach spell") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> demoralizing_roar(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("demoralizing roar",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
};

BearTankDruidStrategy::BearTankDruidStrategy(PlayerbotAI* ai) : FeralDruidStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<BearTankDruidStrategyActionNodeFactory>());
}

ActionList BearTankDruidStrategy::getDefaultActions()
{
    return NextAction::array({
            std::make_shared<NextAction>("lacerate", ACTION_NORMAL + 4),
            std::make_shared<NextAction>("mangle (bear)", ACTION_NORMAL + 3),
            std::make_shared<NextAction>("maul", ACTION_NORMAL + 2),
            std::make_shared<NextAction>("faerie fire (feral)", ACTION_NORMAL + 1) });
}

void BearTankDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "thorns",
        NextAction::array({ std::make_shared<NextAction>("thorns", ACTION_HIGH + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "bear form",
        NextAction::array({ std::make_shared<NextAction>("dire bear form", ACTION_HIGH + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "faerie fire (feral)",
        NextAction::array({ std::make_shared<NextAction>("faerie fire (feral)", ACTION_HIGH + 7) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "lose aggro",
        NextAction::array({ std::make_shared<NextAction>("growl", ACTION_HIGH + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("demoralizing roar", ACTION_HIGH + 6), std::make_shared<NextAction>("swipe (bear)", ACTION_HIGH + 6) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "light aoe",
        NextAction::array({ std::make_shared<NextAction>("swipe (bear)", ACTION_HIGH + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "bash",
        NextAction::array({ std::make_shared<NextAction>("bash", ACTION_INTERRUPT + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "bash on enemy healer",
        NextAction::array({ std::make_shared<NextAction>("bash on enemy healer", ACTION_INTERRUPT + 1) })));

}
