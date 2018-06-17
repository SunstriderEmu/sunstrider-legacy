
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "HealShamanStrategy.h"

using namespace ai;

class HealShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealShamanStrategyActionNodeFactory()
    {
        creators["earthliving weapon"] = &earthliving_weapon;
        creators["mana tide totem"] = &mana_tide_totem;
    }
private:
    static ActionNode* earthliving_weapon(PlayerbotAI* ai)
    {
        return new ActionNode ("earthliving weapon",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("flametongue weapon") }),
            /*C*/ {});
    }
    static ActionNode* mana_tide_totem(PlayerbotAI* ai)
    {
        return new ActionNode ("mana tide totem",
            /*P*/ {},
            /*A*/ NextAction::array({ new NextAction("mana potion") }),
            /*C*/ {});
    }

};

HealShamanStrategy::HealShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(new HealShamanStrategyActionNodeFactory());
}

void HealShamanStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "enemy out of spell",
        NextAction::array({ new NextAction("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(new TriggerNode(
        "shaman weapon",
        NextAction::array({ new NextAction("earthliving weapon", 22.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array({ new NextAction("mana tide totem", ACTION_EMERGENCY + 5) })));

    triggers.push_back(new TriggerNode(
        "cleanse spirit poison",
        NextAction::array({ new NextAction("cleanse spirit", 24.0f) })));

    triggers.push_back(new TriggerNode(
        "cleanse spirit curse",
        NextAction::array({ new NextAction("cleanse spirit", 24.0f) })));

    triggers.push_back(new TriggerNode(
        "cleanse spirit disease",
        NextAction::array({ new NextAction("cleanse spirit", 24.0f) })));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit poison",
        NextAction::array({ new NextAction("cleanse spirit poison on party", 23.0f) })));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit curse",
        NextAction::array({ new NextAction("cleanse spirit curse on party", 23.0f) })));

    triggers.push_back(new TriggerNode(
        "party member cleanse spirit disease",
        NextAction::array({ new NextAction("cleanse spirit disease on party", 23.0f) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("healing stream totem", ACTION_LIGHT_HEAL) })));
}
