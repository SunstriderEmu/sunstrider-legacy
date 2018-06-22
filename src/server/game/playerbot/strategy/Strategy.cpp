
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
    static std::shared_ptr<ActionNode> melee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("melee",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("reach melee") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> healthstone(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("healthstone",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("healing potion") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> follow_master_random(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("be near",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("follow") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> attack_anything(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("attack anything",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> move_random(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("move random",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("stay line") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> move_to_loot(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("move to loot",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> food(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("food",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> drink(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("drink",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mana_potion(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("mana potion",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("drink") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> healing_potion(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("healing potion",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("food") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> flee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode>("flee",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
};

Strategy::Strategy(PlayerbotAI* ai) : PlayerbotAIAware(ai)
{
    actionNodeFactories.Add(std::make_unique<ActionNodeFactoryInternal>());
}

std::shared_ptr<ActionNode> Strategy::GetAction(std::string name)
{
    return actionNodeFactories.GetObject(name, ai);
}

