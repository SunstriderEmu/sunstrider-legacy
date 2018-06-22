
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "GenericMageNonCombatStrategy.h"

using namespace ai;

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["ice armor"] = &ice_armor;
    }
private:
    static std::shared_ptr<ActionNode> molten_armor(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("molten armor",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mage armor") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mage_armor(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mage armor",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("ice armor") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> ice_armor(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("ice armor",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("frost armor") }),
            /*C*/ ActionList());
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericMageNonCombatStrategyActionNodeFactory>());
}

void GenericMageNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "arcane intellect",
        NextAction::array({ std::make_shared<NextAction>("arcane intellect", 21.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "arcane intellect on party",
        NextAction::array({ std::make_shared<NextAction>("arcane intellect on party", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no drink",
        NextAction::array({ std::make_shared<NextAction>("conjure water", 16.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no food",
        NextAction::array({ std::make_shared<NextAction>("conjure food", 15.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "remove curse",
        NextAction::array({ std::make_shared<NextAction>("remove curse", 41.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "remove curse on party",
        NextAction::array({ std::make_shared<NextAction>("remove curse on party", 40.0f) })));
}

void MageBuffManaStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "mage armor",
        NextAction::array({ std::make_shared<NextAction>("mage armor", 19.0f) })));
}

void MageBuffDpsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "mage armor",
        NextAction::array({ std::make_shared<NextAction>("molten armor", 19.0f) })));
}
