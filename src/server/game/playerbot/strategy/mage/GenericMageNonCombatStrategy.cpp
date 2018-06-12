
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
    static ActionNode* molten_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("molten armor",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("mage armor") }),
            /*C*/ NULL);
    }
    static ActionNode* mage_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("mage armor",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("ice armor") }),
            /*C*/ NULL);
    }
    static ActionNode* ice_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("ice armor",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("frost armor") }),
            /*C*/ NULL);
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "arcane intellect",
        NextAction::array({ new NextAction("arcane intellect", 21.0f) })));

    triggers.push_back(new TriggerNode(
        "arcane intellect on party",
        NextAction::array({ new NextAction("arcane intellect on party", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "no drink",
        NextAction::array({ new NextAction("conjure water", 16.0f) })));

    triggers.push_back(new TriggerNode(
        "no food",
        NextAction::array({ new NextAction("conjure food", 15.0f) })));

    triggers.push_back(new TriggerNode(
        "remove curse",
        NextAction::array({ new NextAction("remove curse", 41.0f) })));

    triggers.push_back(new TriggerNode(
        "remove curse on party",
        NextAction::array({ new NextAction("remove curse on party", 40.0f) })));
}

void MageBuffManaStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "mage armor",
        NextAction::array({ new NextAction("mage armor", 19.0f) })));
}

void MageBuffDpsStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "mage armor",
        NextAction::array({ new NextAction("molten armor", 19.0f) })));
}
