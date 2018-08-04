//
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "DpsPaladinStrategy.h"

using namespace ai;

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
    }

private:
    static std::shared_ptr<ActionNode> seal_of_vengeance(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("seal of vengeance",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("seal of command") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> seal_of_command(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("seal of command",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("seal of wisdom") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> blessing_of_might(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("blessing of might",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("blessing of kings") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> crusader_strike(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("crusader strike",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<DpsPaladinStrategyActionNodeFactory>());
}

ActionList DpsPaladinStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("crusader strike", ACTION_NORMAL + 1) });
}

void DpsPaladinStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);
    
    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("divine shield", ACTION_CRITICAL_HEAL + 2), std::make_shared<NextAction>("holy light", ACTION_CRITICAL_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "judgement of wisdom",
        NextAction::array({ std::make_shared<NextAction>("judgement of wisdom", ACTION_NORMAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("divine storm", ACTION_HIGH + 1), std::make_shared<NextAction>("consecration", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "art of war",
        NextAction::array({ std::make_shared<NextAction>("exorcism", ACTION_HIGH + 2) })));
}
