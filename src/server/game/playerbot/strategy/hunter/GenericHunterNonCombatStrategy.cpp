
#include "../../playerbot.h"
#include "HunterMultipliers.h"
#include "GenericHunterNonCombatStrategy.h"

using namespace ai;

class GenericHunterNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterNonCombatStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
    }
private:
    static ActionNode* rapid_fire(PlayerbotAI* ai)
    {
        return new ActionNode ("rapid fire",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("readiness") }),
            /*C*/ NULL);
    }
    static ActionNode* aspect_of_the_pack(PlayerbotAI* ai)
    {
        return new ActionNode ("aspect of the pack",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("aspect of the cheetah") }),
            /*C*/ NULL);
    }
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericHunterNonCombatStrategyActionNodeFactory());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "trueshot aura",
        NextAction::array({ new NextAction("trueshot aura", 2.0f) })));

    triggers.push_back(new TriggerNode(
        "no pet",
        NextAction::array({ new NextAction("call pet", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "hunters pet dead",
        NextAction::array({ new NextAction("revive pet", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "hunters pet low health",
        NextAction::array({ new NextAction("mend pet", 60.0f) })));
}
