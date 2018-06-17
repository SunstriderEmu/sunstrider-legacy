
#include "../playerbot.h"
#include "Strategy.h"
#include "NamedObjectContext.h"

using namespace ai;
using namespace std;


class ActionNodeFactoryInternal : public NamedObjectFactory<ActionNode>
{
public:
    ActionNodeFactoryInternal()
    {
        creators["melee"] = &melee;
        creators["healthstone"] = &healthstone;
        creators["be near"] = &follow_master_random;
        creators["attack anything"] = &attack_anything;
        creators["move random"] = &move_random;
        creators["move to loot"] = &move_to_loot;
        creators["food"] = &food;
        creators["drink"] = &drink;
        creators["mana potion"] = &mana_potion;
        creators["healing potion"] = &healing_potion;
        creators["flee"] = &flee;
    }

private:
    static ActionNode* melee(PlayerbotAI* ai)
    {
        return new ActionNode("melee",
            /*P*/ NextAction::array({ new NextAction("reach melee") }),
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* healthstone(PlayerbotAI* ai)
    {
        return new ActionNode("healthstone",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("healing potion") }),
            /*C*/ {});
    }
    static ActionNode* follow_master_random(PlayerbotAI* ai)
    {
        return new ActionNode("be near",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("follow") }),
            /*C*/{});
    }
    static ActionNode* attack_anything(PlayerbotAI* ai)
    {
        return new ActionNode("attack anything",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* move_random(PlayerbotAI* ai)
    {
        return new ActionNode("move random",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("stay line") }),
            /*C*/ {});
    }
    static ActionNode* move_to_loot(PlayerbotAI* ai)
    {
        return new ActionNode("move to loot",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* food(PlayerbotAI* ai)
    {
        return new ActionNode("food",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* drink(PlayerbotAI* ai)
    {
        return new ActionNode("drink",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
    static ActionNode* mana_potion(PlayerbotAI* ai)
    {
        return new ActionNode("mana potion",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("drink") }),
            /*C*/ {});
    }
    static ActionNode* healing_potion(PlayerbotAI* ai)
    {
        return new ActionNode("healing potion",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("food") }),
            /*C*/ {});
    }
    static ActionNode* flee(PlayerbotAI* ai)
    {
        return new ActionNode("flee",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {});
    }
};

Strategy::Strategy(PlayerbotAI* ai) : PlayerbotAIAware(ai)
{
    actionNodeFactories.Add(new ActionNodeFactoryInternal());
}

ActionNode* Strategy::GetAction(std::string name)
{
    return actionNodeFactories.GetObject(name, ai);
}

