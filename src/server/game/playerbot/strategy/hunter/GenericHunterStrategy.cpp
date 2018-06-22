
#include "../../playerbot.h"
#include "GenericHunterStrategy.h"
#include "HunterAiObjectContext.h"

using namespace ai;

class GenericHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterStrategyActionNodeFactory()
    {
        creators["rapid fire"] = &rapid_fire;
        creators["boost"] = &rapid_fire;
        creators["aspect of the pack"] = &aspect_of_the_pack;
        creators["feign death"] = &feign_death;
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
    static std::shared_ptr<ActionNode> feign_death(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("feign death",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("flee") }),
            /*C*/ ActionList());
    }
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* ai) : RangedCombatStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericHunterStrategyActionNodeFactory>());
}

void GenericHunterStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for spell",
        NextAction::array({ std::make_shared<NextAction>("wing clip", 50.0f), std::make_shared<NextAction>("flee",49.0f), std::make_shared<NextAction>("concussive shot", 48.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium threat",
        NextAction::array({ std::make_shared<NextAction>("feign death", 52.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "hunters pet low health",
        NextAction::array({ std::make_shared<NextAction>("mend pet", 60.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "rapid fire",
        NextAction::array({ std::make_shared<NextAction>("rapid fire", 55.0f) })));
}
