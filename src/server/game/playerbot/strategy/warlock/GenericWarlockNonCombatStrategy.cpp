
#include "../../playerbot.h"
#include "WarlockMultipliers.h"
#include "GenericWarlockNonCombatStrategy.h"

using namespace ai;

class GenericWarlockNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockNonCombatStrategyActionNodeFactory()
    {
        creators["fel armor"] = &fel_armor;
        creators["demon armor"] = &demon_armor;
    }
private:
    static std::shared_ptr<ActionNode> fel_armor(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("fel armor",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("demon armor") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> demon_armor(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("demon armor",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("demon skin") }),
            /*C*/ ActionList());
    }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericWarlockNonCombatStrategyActionNodeFactory>());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "demon armor",
        NextAction::array({ std::make_shared<NextAction>("fel armor", 21.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no healthstone",
        NextAction::array({ std::make_shared<NextAction>("create healthstone", 15.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no firestone",
        NextAction::array({ std::make_shared<NextAction>("create firestone", 14.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no spellstone",
        NextAction::array({ std::make_shared<NextAction>("create spellstone", 13.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "spellstone",
        NextAction::array({ std::make_shared<NextAction>("spellstone", 13.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no pet",
        NextAction::array({ std::make_shared<NextAction>("summon imp", 10.0f) })));
}
