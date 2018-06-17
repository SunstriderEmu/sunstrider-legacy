
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
    static ActionNode* stormstrike(PlayerbotAI* ai)
    {
        return new ActionNode ("stormstrike",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("lava lash") }),
            /*C*/ {});
    }
    static ActionNode* lava_lash(PlayerbotAI* ai)
    {
        return new ActionNode ("lava lash",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("melee") }),
            /*C*/ {});
    }
    static ActionNode* magma_totem(PlayerbotAI* ai)
    {
        return new ActionNode ("magma totem",
            /*P*/ {},
            /*A*/ {},
            /*C*/ NextAction::array({ new NextAction("fire nova") }));
    }
};

MeleeShamanStrategy::MeleeShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(new MeleeShamanStrategyActionNodeFactory());
}

ActionList MeleeShamanStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("stormstrike", 10.0f) });
}

void MeleeShamanStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array({ new NextAction("windfury weapon", 22.0f) })));

    triggers.push_back(new TriggerNode(
        "searing totem",
        NextAction::array({ new NextAction("searing totem", 22.0f) })));

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array({ new NextAction("earth shock", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "not facing target",
        NextAction::array({ new NextAction("set facing", ACTION_NORMAL + 7) })));

    triggers.push_back(new TriggerNode(
        "enemy too close for melee",
        NextAction::array({ new NextAction("move out of enemy contact", ACTION_NORMAL + 8) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("strength of earth totem", ACTION_LIGHT_HEAL) })));
}

void MeleeAoeShamanStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "enemy out of melee",
        NextAction::array({ new NextAction("reach melee", ACTION_NORMAL + 8) })));

    triggers.push_back(new TriggerNode(
        "magma totem",
        NextAction::array({ new NextAction("magma totem", 26.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("fire nova", 25.0f) })));
}
