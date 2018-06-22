
#include "../../playerbot.h"
#include "PaladinMultipliers.h"
#include "TankPaladinStrategy.h"

using namespace ai;

class TankPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    TankPaladinStrategyActionNodeFactory()
    {
        creators["blessing of sanctuary"] = &blessing_of_sanctuary;
    }
private:
    static std::shared_ptr<ActionNode> blessing_of_sanctuary(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("blessing of sanctuary",
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("blessing of kings") }),
            /*C*/ ActionList());
    }
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
    actionNodeFactories.Add(std::make_unique<TankPaladinStrategyActionNodeFactory>());
}

ActionList TankPaladinStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("melee", ACTION_NORMAL) });
}

void TankPaladinStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "judgement of light",
        NextAction::array({ std::make_shared<NextAction>("judgement of light", ACTION_NORMAL + 2) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium mana",
        NextAction::array({ std::make_shared<NextAction>("judgement of wisdom", ACTION_NORMAL + 3) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "righteous fury",
        NextAction::array({ std::make_shared<NextAction>("righteous fury", ACTION_HIGH + 8) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "light aoe",
        NextAction::array({ std::make_shared<NextAction>("hammer of the righteous", ACTION_HIGH + 6), std::make_shared<NextAction>("avenger's shield", ACTION_HIGH + 6) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "medium aoe",
        NextAction::array({ std::make_shared<NextAction>("consecration", ACTION_HIGH + 6) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "lose aggro",
        NextAction::array({ std::make_shared<NextAction>("hand of reckoning", ACTION_HIGH + 7) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "holy shield",
        NextAction::array({ std::make_shared<NextAction>("holy shield", ACTION_HIGH + 7) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "blessing",
        NextAction::array({ std::make_shared<NextAction>("blessing of sanctuary", ACTION_HIGH + 9) })));
}
