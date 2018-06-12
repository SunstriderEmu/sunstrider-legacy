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
    static ActionNode* magma_totem(PlayerbotAI* ai)
    {
        return new ActionNode ("magma totem",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NextAction::array({ new NextAction("fire nova") }));
    }
};

CasterShamanStrategy::CasterShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(new CasterShamanStrategyActionNodeFactory());
}

NextAction** CasterShamanStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("lightning bolt", 10.0f) });
}

void CasterShamanStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array({ new NextAction("flametongue weapon", 23.0f) })));

    triggers.push_back(new TriggerNode(
        "searing totem",
        NextAction::array({ new NextAction("searing totem", 19.0f) })));

    triggers.push_back(new TriggerNode(
        "shock",
        NextAction::array({ new NextAction("earth shock", 20.0f) })));

    triggers.push_back(new TriggerNode(
        "frost shock snare",
        NextAction::array({ new NextAction("frost shock", 21.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("flametongue totem", ACTION_LIGHT_HEAL) })));
}

void CasterAoeShamanStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    MeleeAoeShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array({ new NextAction("chain lightning", 25.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("thunderstorm", 26.0f) })));
}
