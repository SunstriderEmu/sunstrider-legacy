
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
    static std::shared_ptr<ActionNode> earthliving_weapon(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("earthliving weapon",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("flametongue weapon") }),
            /*C*/ ActionList());
    }
    static std::shared_ptr<ActionNode> mana_tide_totem(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("mana tide totem",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("mana potion") }),
            /*C*/ ActionList());
    }

};

HealShamanStrategy::HealShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<HealShamanStrategyActionNodeFactory>());
}

void HealShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "enemy out of spell",
        NextAction::array({ std::make_shared<NextAction>("reach spell", ACTION_NORMAL + 9) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "shaman weapon",
        NextAction::array({ std::make_shared<NextAction>("earthliving weapon", 22.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana",
        NextAction::array({ std::make_shared<NextAction>("mana tide totem", ACTION_EMERGENCY + 5) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse spirit poison",
        NextAction::array({ std::make_shared<NextAction>("cleanse spirit", 24.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse spirit curse",
        NextAction::array({ std::make_shared<NextAction>("cleanse spirit", 24.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "cleanse spirit disease",
        NextAction::array({ std::make_shared<NextAction>("cleanse spirit", 24.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member cleanse spirit poison",
        NextAction::array({ std::make_shared<NextAction>("cleanse spirit poison on party", 23.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member cleanse spirit curse",
        NextAction::array({ std::make_shared<NextAction>("cleanse spirit curse on party", 23.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "party member cleanse spirit disease",
        NextAction::array({ std::make_shared<NextAction>("cleanse spirit disease on party", 23.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("healing stream totem", ACTION_LIGHT_HEAL) })));
}
