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
    static std::shared_ptr<ActionNode> overpower(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("overpower",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> melee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("melee",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("charge") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> charge(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("charge",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("battle stance") }),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("reach melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> bloodthirst(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("bloodthirst",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("battle stance") }),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("heroic strike") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> rend(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rend",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("battle stance") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mocking_blow(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mocking blow",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("battle stance") }),
            /*A*/ NextAction::array({ }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> death_wish(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("death wish",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("berserker rage") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> execute(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("execute",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("battle stance") }),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("heroic strike") }),
            /*C*/ ActionList());
    }
};

DpsWarriorStrategy::DpsWarriorStrategy(PlayerbotAI* ai) : GenericWarriorStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<DpsWarriorStrategyActionNodeFactory>());
}

ActionList DpsWarriorStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("bloodthirst", ACTION_NORMAL + 1) });
}

void DpsWarriorStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of melee",
        NextAction::array({ std::make_shared<NextAction>("charge", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "target critical health",
        NextAction::array({ std::make_shared<NextAction>("execute", ACTION_HIGH + 4) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hamstring",
        NextAction::array({ std::make_shared<NextAction>("hamstring", ACTION_INTERRUPT) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "victory rush",
        NextAction::array({ std::make_shared<NextAction>("victory rush", ACTION_HIGH + 3) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "death wish",
        NextAction::array({ std::make_shared<NextAction>("death wish", ACTION_HIGH + 2) })));
}


void DpsWarrirorAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "rend on attacker",
        NextAction::array({ std::make_shared<NextAction>("rend on attacker", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "light aoe",
        NextAction::array({ std::make_shared<NextAction>("thunder clap", ACTION_HIGH + 2), std::make_shared<NextAction>("demoralizing shout", ACTION_HIGH + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("cleave", ACTION_HIGH + 3) })));
}
