
#include "../playerbot.h"
#include "AiObjectContext.h"
#include "Action.h"

using namespace ai;

ActionList NextAction::clone(ActionList actions)
{
    ActionList res;
    res.reserve(actions.size());
    for (auto itr : actions)
        res.push_back(std::make_shared<NextAction>(*itr));

    return res;
}

ActionList NextAction::merge(ActionList left, ActionList right)
{
    int leftSize = left.size();
    int rightSize = right.size();

    ActionList res;
    res.reserve(leftSize + rightSize);
    for (auto itr : left)
        res.push_back(itr);
    for (auto itr : right)
        res.push_back(itr);

    return res;
}

ActionList NextAction::array(std::initializer_list<std::shared_ptr<NextAction>> args)
{
    ActionList res;
    res.reserve(args.size());
    for (auto elem : args)
        res.push_back(elem);

    return res;
}

void NextAction::destroy(ActionList& actions)
{
    actions.clear();
}

std::shared_ptr<Value<Unit*>> Action::GetTargetValue()
{
    return context->GetValue<Unit*>(GetTargetName());
}

Unit* Action::GetTarget()
{
    return GetTargetValue()->Get();
}
