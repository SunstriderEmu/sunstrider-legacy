
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
        static ActionNode* smite(PlayerbotAI* ai)
        {
            return new ActionNode ("smite",
                /*P*/ {},
                /*A*/ NextAction::array({ new NextAction("shoot") }),
                /*C*/ {});
        }
    };
};

using namespace ai;

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* ai) : HealPriestStrategy(ai)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

ActionList HolyPriestStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("holy fire", 10.0f), new NextAction("smite", 10.0f) });
}

void HolyPriestStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    HealPriestStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_NORMAL + 9) })));

}
