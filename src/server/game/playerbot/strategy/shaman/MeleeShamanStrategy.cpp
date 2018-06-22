
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "MeleeShamanStrategy.h"

using namespace ai;

class MeleeShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    MeleeShamanStrategyActionNodeFactory()
    {
        creators["stormstrike"] = &stormstrike;
        creators["lava lash"] = &lava_lash;
        creators["magma totem"] = &magma_totem;
    }
private:
    static std::shared_ptr<ActionNode> stormstrike(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("stormstrike",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("lava lash") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> lava_lash(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("lava lash",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("melee") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> magma_totem(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("magma totem",
            /*P*/ ActionList(),
            /*A*/ ActionList(),
            /*C*/ NextAction::array({ std::make_shared<NextAction>("fire nova") }));
    }
};

MeleeShamanStrategy::MeleeShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<MeleeShamanStrategyActionNodeFactory>());
}

ActionList MeleeShamanStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("stormstrike", 10.0f) });
}

void MeleeShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "shaman weapon",
        NextAction::array({ std::make_shared<NextAction>("windfury weapon", 22.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "searing totem",
        NextAction::array({ std::make_shared<NextAction>("searing totem", 22.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shock",
        NextAction::array({ std::make_shared<NextAction>("earth shock", 20.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "not facing target",
        NextAction::array({ std::make_shared<NextAction>("set facing", ACTION_NORMAL + 7) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy too close for melee",
        NextAction::array({ std::make_shared<NextAction>("move out of enemy contact", ACTION_NORMAL + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("strength of earth totem", ACTION_LIGHT_HEAL) })));
}

void MeleeAoeShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of melee",
        NextAction::array({ std::make_shared<NextAction>("reach melee", ACTION_NORMAL + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "magma totem",
        NextAction::array({ std::make_shared<NextAction>("magma totem", 26.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("fire nova", 25.0f) })));
}
