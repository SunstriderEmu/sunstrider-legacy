
#include "../../playerbot.h"
#include "PriestMultipliers.h"
#include "HolyPriestStrategy.h"

namespace ai
{
    class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        HolyPriestStrategyActionNodeFactory()
        {
            creators["smite"] = &smite;
        }
    private:
        static std::shared_ptr<ActionNode> smite(PlayerbotAI* ai)
        {
            return std::make_shared<ActionNode> ("smite",
                /*P*/ ActionList(),
                /*A*/ NextAction::array({ std::make_shared<NextAction>("shoot") }),
                /*C*/ ActionList());
        }
    };
};

using namespace ai;

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* ai) : HealPriestStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<HolyPriestStrategyActionNodeFactory>());
}

ActionList HolyPriestStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("holy fire", 10.0f), std::make_shared<NextAction>("smite", 10.0f) });
}

void HolyPriestStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    HealPriestStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_NORMAL + 9) })));

}
