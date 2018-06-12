
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
    static ActionNode* blessing_of_sanctuary(PlayerbotAI* ai)
    {
        return new ActionNode ("blessing of sanctuary",
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("blessing of kings") }),
            /*C*/ NULL);
    }
};

TankPaladinStrategy::TankPaladinStrategy(PlayerbotAI* ai) : GenericPaladinStrategy(ai)
{
    actionNodeFactories.Add(new TankPaladinStrategyActionNodeFactory());
}

NextAction** TankPaladinStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("melee", ACTION_NORMAL) });
}

void TankPaladinStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "judgement of light",
        NextAction::array({ new NextAction("judgement of light", ACTION_NORMAL + 2) })));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array({ new NextAction("judgement of wisdom", ACTION_NORMAL + 3) })));

    triggers.push_back(new TriggerNode(
        "righteous fury",
        NextAction::array({ new NextAction("righteous fury", ACTION_HIGH + 8) })));

    triggers.push_back(new TriggerNode(
        "light aoe",
        NextAction::array({ new NextAction("hammer of the righteous", ACTION_HIGH + 6), new NextAction("avenger's shield", ACTION_HIGH + 6) })));

    triggers.push_back(new TriggerNode(
        "medium aoe",
        NextAction::array({ new NextAction("consecration", ACTION_HIGH + 6) })));

    triggers.push_back(new TriggerNode(
        "lose aggro",
        NextAction::array({ new NextAction("hand of reckoning", ACTION_HIGH + 7) })));

    triggers.push_back(new TriggerNode(
        "holy shield",
        NextAction::array({ new NextAction("holy shield", ACTION_HIGH + 7) })));

    triggers.push_back(new TriggerNode(
        "blessing",
        NextAction::array({ new NextAction("blessing of sanctuary", ACTION_HIGH + 9) })));
}
