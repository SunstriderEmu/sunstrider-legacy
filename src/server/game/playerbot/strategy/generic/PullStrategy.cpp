
#include "../../playerbot.h"
#include "../PassiveMultiplier.h"
#include "PullStrategy.h"

using namespace ai;

class MagePullMultiplier : public PassiveMultiplier
{
public:
    MagePullMultiplier(PlayerbotAI* ai, std::string action) : PassiveMultiplier(ai)
    {
        this->action = action;
    }

public:
    virtual float GetValue(Action* action);

private:
    std::string action;
};

float MagePullMultiplier::GetValue(Action* _action) 
{
    if (!_action)
        return 1.0f;

    std::string _name = _action->getName();
    if (this->action == _name ||
        _name == "reach spell" ||
        _name == "change strategy")
        return 1.0f;

    return PassiveMultiplier::GetValue(_action);
}

NextAction** PullStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction(action, 105.0f), new NextAction("follow", 104.0f), new NextAction("end pull", 103.0f) });
}

void PullStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);
}

void PullStrategy::InitMultipliers(std::list<Multiplier*> &multipliers)
{
    multipliers.push_back(new MagePullMultiplier(ai, action));
    RangedCombatStrategy::InitMultipliers(multipliers);
}

