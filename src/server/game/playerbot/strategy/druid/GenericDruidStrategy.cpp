
#include "../../playerbot.h"
#include "GenericDruidStrategy.h"
#include "DruidAiObjectContext.h"

using namespace ai;

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["caster form"] = &caster_form;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["rebirth"] = &rebirth;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["innervate"] = &innervate;
    }

private:
    static std::shared_ptr<ActionNode> melee(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("melee",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("reach melee") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> caster_form(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("caster form",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> cure_poison(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("cure poison",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> cure_poison_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("cure poison on party",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> abolish_poison(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("abolish poison",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> abolish_poison_on_party(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("abolish poison on party",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> rebirth(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rebirth",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> entangling_roots_on_cc(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("entangling roots on cc",
            /*P*/ NextAction::array({ std::make_shared<NextAction>("caster form") }),
            /*A*/ ActionList(),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> innervate(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("innervate",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mana potion") }),
            /*C*/ ActionList());
    }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* ai) : CombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericDruidStrategyActionNodeFactory>());
}

void GenericDruidStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "low health",
        NextAction::array({ std::make_shared<NextAction>("regrowth", ACTION_MEDIUM_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member low health",
        NextAction::array({ std::make_shared<NextAction>("regrowth on party", ACTION_MEDIUM_HEAL + 1) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "critical health",
        NextAction::array({ std::make_shared<NextAction>("regrowth", ACTION_CRITICAL_HEAL + 2), std::make_shared<NextAction>("healing touch", ACTION_CRITICAL_HEAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member critical health",
        NextAction::array({ std::make_shared<NextAction>("regrowth on party", ACTION_CRITICAL_HEAL + 1), std::make_shared<NextAction>("healing touch on party", ACTION_CRITICAL_HEAL + 1) })));


    triggers.push_back(std::make_shared<TriggerNode>(
        "cure poison",
        NextAction::array({ std::make_shared<NextAction>("abolish poison", ACTION_DISPEL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member cure poison",
        NextAction::array({ std::make_shared<NextAction>("abolish poison on party", ACTION_DISPEL + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member dead",
        NextAction::array({ std::make_shared<NextAction>("rebirth", ACTION_HIGH + 1) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("innervate", ACTION_EMERGENCY + 5) })));
}
