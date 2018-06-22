////
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "ArcaneMageStrategy.h"

using namespace ai;

class ArcaneMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArcaneMageStrategyActionNodeFactory()
    {
        creators["arcane blast"] = &arcane_blast;
        creators["arcane barrage"] = &arcane_barrage;
        creators["arcane missiles"] = &arcane_missiles;
    }
private:
    static std::shared_ptr<ActionNode> arcane_blast(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("arcane blast",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("arcane missiles") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> arcane_barrage(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("arcane barrage",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("arcane missiles") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> arcane_missiles(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("arcane missiles",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("shoot") }),
            /*C*/ ActionList());
    }
};

ArcaneMageStrategy::ArcaneMageStrategy(PlayerbotAI* ai) : GenericMageStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<ArcaneMageStrategyActionNodeFactory>());
}

ActionList ArcaneMageStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("arcane barrage", 10.0f) });
}

void ArcaneMageStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "arcane blast",
        NextAction::array({ std::make_shared<NextAction>("arcane blast", 15.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "missile barrage",
        NextAction::array({ std::make_shared<NextAction>("arcane missiles", 15.0f) })));

}

