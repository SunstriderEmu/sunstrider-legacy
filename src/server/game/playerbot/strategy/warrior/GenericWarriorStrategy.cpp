
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
    static ActionNode* hamstring(PlayerbotAI* ai)
    {
        return new ActionNode ("hamstring",
            /*P*/ NextAction::array({ new NextAction("battle stance") }),
            /*A*/ NULL,
            /*C*/ NULL);
    }
    static ActionNode* heroic_strike(PlayerbotAI* ai)
    {
        return new ActionNode ("heroic strike",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ NULL);
    }
    static ActionNode* battle_shout(PlayerbotAI* ai)
    {
        return new ActionNode ("battle shout",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ NULL);
    }
};

GenericWarriorStrategy::GenericWarriorStrategy(PlayerbotAI* ai) : MeleeCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericWarriorStrategyActionNodeFactory());
}

void GenericWarriorStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "battle shout",
        NextAction::array({ new NextAction("battle shout", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "rend",
        NextAction::array({ new NextAction("rend", ACTION_NORMAL + 1) })));

    triggers.push_back(new TriggerNode(
        "bloodrage",
        NextAction::array({ new NextAction("bloodrage", ACTION_HIGH + 1) })));

    triggers.push_back(new TriggerNode(
        "shield bash",
        NextAction::array({ new NextAction("shield bash", ACTION_INTERRUPT + 4) })));

    triggers.push_back(new TriggerNode(
        "shield bash on enemy healer",
        NextAction::array({ new NextAction("shield bash on enemy healer", ACTION_INTERRUPT + 3) })));

    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array({ new NextAction("intimidating shout", ACTION_EMERGENCY) })));
}
