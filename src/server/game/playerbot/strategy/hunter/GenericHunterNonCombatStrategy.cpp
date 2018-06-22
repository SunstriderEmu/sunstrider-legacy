
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
    static std::shared_ptr<ActionNode> rapid_fire(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("rapid fire",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("readiness") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> aspect_of_the_pack(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("aspect of the pack",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("aspect of the cheetah") }),
            /*C*/ ActionList());
    }
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericHunterNonCombatStrategyActionNodeFactory>());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "trueshot aura",
        NextAction::array({ std::make_shared<NextAction>("trueshot aura", 2.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "no pet",
        NextAction::array({ std::make_shared<NextAction>("call pet", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hunters pet dead",
        NextAction::array({ std::make_shared<NextAction>("revive pet", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hunters pet low health",
        NextAction::array({ std::make_shared<NextAction>("mend pet", 60.0f) })));
}
