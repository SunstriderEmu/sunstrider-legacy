
#include "../../playerbot.h"
#include "GenericWarriorStrategy.h"
#include "WarriorAiObjectContext.h"

using namespace ai;

class GenericWarriorStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarriorStrategyActionNodeFactory()
    {
        creators["hamstring"] = &hamstring;
        creators["heroic strike"] = &heroic_strike;
        creators["battle shout"] = &battle_shout;
    }
private:
    static std::shared_ptr<ActionNode> hamstring(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("hamstring",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("battle stance") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> heroic_strike(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("heroic strike",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> battle_shout(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("battle shout",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
};

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericWarriorStrategyActionNodeFactory>());
}

void GenericWarriorStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "battle shout",
        NextAction::array({ std::make_shared<NextAction>("battle shout", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "rend",
        NextAction::array({ std::make_shared<NextAction>("rend", ACTION_NORMAL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "bloodrage",
        NextAction::array({ std::make_shared<NextAction>("bloodrage", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shield bash",
        NextAction::array({ std::make_shared<NextAction>("shield bash", ACTION_INTERRUPT + 4) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shield bash on enemy healer",
        NextAction::array({ std::make_shared<NextAction>("shield bash on enemy healer", ACTION_INTERRUPT + 3) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("intimidating shout", ACTION_EMERGENCY) })));
}
