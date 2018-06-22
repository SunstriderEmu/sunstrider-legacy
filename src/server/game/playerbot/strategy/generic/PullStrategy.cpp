
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

ActionList PullStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>(action, 105.0f), std::make_shared<NextAction>("follow", 104.0f), std::make_shared<NextAction>("end pull", 103.0f) });
}

void PullStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);
}

void PullStrategy::InitMultipliers(std::list<std::shared_ptr<Multiplier>> &multipliers)
{
    multipliers.push_back(std::make_shared<MagePullMultiplier>(ai, action));
    RangedCombatStrategy::InitMultipliers(multipliers);
}

