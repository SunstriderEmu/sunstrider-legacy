//
#include "../../playerbot.h"
#include "FeralDruidStrategy.h"

using namespace ai;

class FeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FeralDruidStrategyActionNodeFactory()
    {
        creators["survival instincts"] = &survival_instincts;
        creators["thorns"] = &thorns;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
    }
private:
    static std::shared_ptr<ActionNode> survival_instincts(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("survival instincts",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("barskin") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> thorns(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("thorns",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> cure_poison(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("cure poison",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> cure_poison_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("cure poison on party",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> abolish_poison(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("abolish poison",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> abolish_poison_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("abolish poison on party",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
};

FeralDruidStrategy::FeralDruidStrategy(PlayerbotAI* ai) : GenericDruidStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<FeralDruidStrategyActionNodeFactory>());
    actionNodeFactories.Add(std::make_unique<ShapeshiftDruidStrategyActionNodeFactory>());
}

void FeralDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "not facing target",
        NextAction::array({ std::make_shared<NextAction>("set facing", ACTION_NORMAL + 7) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of melee",
        NextAction::array({ std::make_shared<NextAction>("reach melee", ACTION_NORMAL + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for melee",
        NextAction::array({ std::make_shared<NextAction>("move out of enemy contact", ACTION_NORMAL + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("survival instincts", ACTION_EMERGENCY + 1) })));
}

