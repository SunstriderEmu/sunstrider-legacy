
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
    static ActionNode* lifeblood(PlayerbotAI* ai)
    {
        return new ActionNode ("lifeblood",  
            /*P*/ NULL,
            /*A*/ NextAction::array({ new NextAction("gift of the naaru") }), 
            /*C*/ NULL);
    }
};

void RacialsStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "low health", 
        NextAction::array({ new NextAction("lifeblood", 71.0f) })));

    triggers.push_back(new TriggerNode(
        "low mana", 
        NextAction::array({ new NextAction("arcane torrent", ACTION_EMERGENCY + 6) })));
}

RacialsStrategy::RacialsStrategy(PlayerbotAI* ai) : Strategy(ai)
{
    actionNodeFactories.Add(new RacialsStrategyActionNodeFactory());
}
