
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
    static ActionNode* fel_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("fel armor",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("demon armor") }),
            /*C*/ NULL);
    }
    static ActionNode* demon_armor(PlayerbotAI* ai)
    {
        return new ActionNode ("demon armor",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("demon skin") }),
            /*C*/ NULL);
    }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "demon armor",
        NextAction::array({ new NextAction("fel armor", 21.0f) })));

    triggers.push_back(new TriggerNode(
        "no healthstone",
        NextAction::array({ new NextAction("create healthstone", 15.0f) })));

    triggers.push_back(new TriggerNode(
        "no firestone",
        NextAction::array({ new NextAction("create firestone", 14.0f) })));

    triggers.push_back(new TriggerNode(
        "no spellstone",
        NextAction::array({ new NextAction("create spellstone", 13.0f) })));

    triggers.push_back(new TriggerNode(
        "spellstone",
        NextAction::array({ new NextAction("spellstone", 13.0f) })));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array({ new NextAction("summon imp", 10.0f) })));
}
