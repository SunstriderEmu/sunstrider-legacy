
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
    static std::shared_ptr<ActionNode> melee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("melee",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("defensive stance"), std::make_shared<NextAction>("reach melee") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> shield_wall(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("shield wall",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("shield block") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> rend(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rend",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("defensive stance") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> revenge(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("revenge",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> devastate(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("devastate",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("sunder armor") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> shockwave(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("shockwave",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("cleave") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> taunt(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("taunt",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mocking blow") }),
            /*C*/ ActionList());
    }
};

TankWarriorStrategy::TankWarriorStrategy(PlayerbotAI* ai) : GenericWarriorStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<TankWarriorStrategyActionNodeFactory>());
}

ActionList TankWarriorStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("devastate", ACTION_NORMAL + 1), std::make_shared<NextAction>("revenge", ACTION_NORMAL + 1) });
}

void TankWarriorStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericWarriorStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium rage available",
        NextAction::array({ std::make_shared<NextAction>("shield slam", ACTION_NORMAL + 2), std::make_shared<NextAction>("heroic strike", ACTION_NORMAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "disarm",
        NextAction::array({ std::make_shared<NextAction>("disarm", ACTION_NORMAL) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "lose aggro",
        NextAction::array({ std::make_shared<NextAction>("taunt", ACTION_HIGH + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium health",
        NextAction::array({ std::make_shared<NextAction>("shield wall", ACTION_MEDIUM_HEAL) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("last stand", ACTION_EMERGENCY + 3) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("shockwave", ACTION_HIGH + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "light aoe",
        NextAction::array({ std::make_shared<NextAction>("thunder clap", ACTION_HIGH + 2), std::make_shared<NextAction>("demoralizing shout", ACTION_HIGH + 2),  std::make_shared<NextAction>("cleave", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "high aoe",
        NextAction::array({ std::make_shared<NextAction>("challenging shout", ACTION_HIGH + 3) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "concussion blow",
        NextAction::array({ std::make_shared<NextAction>("concussion blow", ACTION_INTERRUPT) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "sword and board",
        NextAction::array({ std::make_shared<NextAction>("shield slam", ACTION_HIGH + 3) })));
}
