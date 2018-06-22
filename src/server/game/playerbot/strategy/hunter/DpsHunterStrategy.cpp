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
    static std::shared_ptr<ActionNode> viper_sting(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("viper sting",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mana potion", 10.0f) }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> aimed_shot(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("aimed shot",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("chimera shot", 10.0f) }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> chimera_shot(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("chimera shot",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("arcane shot", 10.0f) }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> explosive_shot(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("explosive shot",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("aimed shot") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> concussive_shot(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("concussive shot",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("wyvern sting", 11.0f) }));
    }

};

DpsHunterStrategy::DpsHunterStrategy(PlayerbotAI* ai) : GenericHunterStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<DpsHunterStrategyActionNodeFactory>());
}

ActionList DpsHunterStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("explosive shot", 11.0f), std::make_shared<NextAction>("auto shot", 10.0f) });
}

void DpsHunterStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "black arrow",
        NextAction::array({ std::make_shared<NextAction>("black arrow", 51.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("viper sting", ACTION_EMERGENCY + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no pet",
        NextAction::array({ std::make_shared<NextAction>("call pet", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hunters pet low health",
        NextAction::array({ std::make_shared<NextAction>("mend pet", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hunter's mark",
        NextAction::array({ std::make_shared<NextAction>("hunter's mark", 52.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "freezing trap",
        NextAction::array({ std::make_shared<NextAction>("freezing trap", 83.0f) })));
}

void DpsAoeHunterStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("multi-shot", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "high aoe",
        NextAction::array({ std::make_shared<NextAction>("volley", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "serpent sting on attacker",
        NextAction::array({ std::make_shared<NextAction>("serpent sting on attacker", 49.0f) })));
}

void DpsHunterDebuffStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "no stings",
        NextAction::array({ std::make_shared<NextAction>("serpent sting", 50.0f) })));
}
