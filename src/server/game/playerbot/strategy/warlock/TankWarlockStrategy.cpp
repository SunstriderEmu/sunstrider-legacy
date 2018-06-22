
#include "../../playerbot.h"
#include "WarlockMultipliers.h"
#include "TankWarlockStrategy.h"

using namespace ai;

class GenericWarlockStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockStrategyActionNodeFactory()
    {
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
    }
private:
    static std::shared_ptr<ActionNode> summon_voidwalker(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("summon voidwalker",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("drain soul") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> summon_felguard(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("summon felguard",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("summon voidwalker") }),
            /*C*/ ActionList());
    }
};

TankWarlockStrategy::TankWarlockStrategy(PlayerbotAI* ai) : GenericWarlockStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<GenericWarlockStrategyActionNodeFactory>());
}

ActionList TankWarlockStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("shoot", 10.0f) });
}

void TankWarlockStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericWarlockStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "no pet",
        NextAction::array({ std::make_shared<NextAction>("summon felguard", 50.0f) })));

}
