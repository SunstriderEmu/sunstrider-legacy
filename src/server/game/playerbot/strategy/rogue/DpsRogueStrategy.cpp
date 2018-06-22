//
#include "../../playerbot.h"
#include "RogueMultipliers.h"
#include "DpsRogueStrategy.h"

using namespace ai;

class DpsRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsRogueStrategyActionNodeFactory()
    {
        creators["riposte"] = &riposte;
        creators["mutilate"] = &mutilate;
        creators["sinister strike"] = &sinister_strike;
        creators["kick"] = &kick;
        creators["kidney shot"] = &kidney_shot;
        creators["rupture"] = &rupture;
        creators["backstab"] = &backstab;
    }
private:
    static std::shared_ptr<ActionNode> riposte(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("riposte",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mutilate") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mutilate(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mutilate",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("sinister strike") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> sinister_strike(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("sinister strike",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> kick(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("kick",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("kidney shot") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> kidney_shot(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("kidney shot",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> rupture(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rupture",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("eviscerate") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> backstab(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("backstab",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mutilate") }),
            /*C*/ ActionList());
    }
};

DpsRogueStrategy::DpsRogueStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<DpsRogueStrategyActionNodeFactory>());
}

ActionList DpsRogueStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("riposte", ACTION_NORMAL) });
}

void DpsRogueStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "combo points available",
        NextAction::array({ std::make_shared<NextAction>("rupture", ACTION_HIGH + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium threat",
        NextAction::array({ std::make_shared<NextAction>("vanish", ACTION_HIGH) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("evasion", ACTION_EMERGENCY), std::make_shared<NextAction>("feint", ACTION_EMERGENCY) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "kick",
        NextAction::array({ std::make_shared<NextAction>("kick", ACTION_INTERRUPT + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "kick on enemy healer",
        NextAction::array({ std::make_shared<NextAction>("kick on enemy healer", ACTION_INTERRUPT + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "behind target",
        NextAction::array({ std::make_shared<NextAction>("backstab", ACTION_NORMAL) })));
}
