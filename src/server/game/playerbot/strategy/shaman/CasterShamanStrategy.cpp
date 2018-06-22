////
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "CasterShamanStrategy.h"

using namespace ai;

class CasterShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterShamanStrategyActionNodeFactory()
    {
        creators["magma totem"] = &magma_totem;
    }
private:
    static std::shared_ptr<ActionNode> magma_totem(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("magma totem",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("fire nova") }));
    }
};

CasterShamanStrategy::CasterShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<CasterShamanStrategyActionNodeFactory>());
}

ActionList CasterShamanStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("lightning bolt", 10.0f) });
}

void CasterShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shaman weapon",
        NextAction::array({ std::make_shared<NextAction>("flametongue weapon", 23.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "searing totem",
        NextAction::array({ std::make_shared<NextAction>("searing totem", 19.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shock",
        NextAction::array({ std::make_shared<NextAction>("earth shock", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "frost shock snare",
        NextAction::array({ std::make_shared<NextAction>("frost shock", 21.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("flametongue totem", ACTION_LIGHT_HEAL) })));
}

void CasterAoeShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    MeleeAoeShamanStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "light aoe",
        NextAction::array({ std::make_shared<NextAction>("chain lightning", 25.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("thunderstorm", 26.0f) })));
}
