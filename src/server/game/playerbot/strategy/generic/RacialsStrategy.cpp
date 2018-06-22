
#include "../../playerbot.h"
#include "RacialsStrategy.h"

using namespace ai;


class RacialsStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    RacialsStrategyActionNodeFactory()
    {
        creators["lifeblood"] = &lifeblood;
    }
private:
    static std::shared_ptr<ActionNode> lifeblood(PlayerbotAI* ai)
    {
        return std::make_shared<ActionNode> ("lifeblood",  
            /*P*/ ActionList(),
            /*A*/ NextAction::array({ std::make_shared<NextAction>("gift of the naaru") }), 
            /*C*/ ActionList());
    }
};

void RacialsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "low health", 
        NextAction::array({ std::make_shared<NextAction>("lifeblood", 71.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "low mana", 
        NextAction::array({ std::make_shared<NextAction>("arcane torrent", ACTION_EMERGENCY + 6) })));
}

RacialsStrategy::RacialsStrategy(PlayerbotAI* ai) : Strategy(ai)
{
    actionNodeFactories.Add(std::make_unique<RacialsStrategyActionNodeFactory>());
}
