
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
    static ActionNode* rapid_fire(PlayerbotAI* ai)
    {
        return new ActionNode ("rapid fire",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("readiness") }),
            /*C*/ {});
    }
    static ActionNode* aspect_of_the_pack(PlayerbotAI* ai)
    {
        return new ActionNode ("aspect of the pack",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("aspect of the cheetah") }),
            /*C*/ {});
    }
    static ActionNode* feign_death(PlayerbotAI* ai)
    {
        return new ActionNode ("feign death",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("flee") }),
            /*C*/ {});
    }
};

GenericHunterStrategy::GenericHunterStrategy(PlayerbotAI* ai) : RangedCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericHunterStrategyActionNodeFactory());
}

void GenericHunterStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy too close for spell",
        NextAction::array({ new NextAction("wing clip", 50.0f), new NextAction("flee",49.0f), new NextAction("concussive shot", 48.0f) })));

    triggers.push_back(new TriggerNode(
        "medium threat",
        NextAction::array({ new NextAction("feign death", 52.0f) })));

    triggers.push_back(new TriggerNode(
        "hunters pet low health",
        NextAction::array({ new NextAction("mend pet", 60.0f) })));

    triggers.push_back(new TriggerNode(
        "rapid fire",
        NextAction::array({ new NextAction("rapid fire", 55.0f) })));
}
