
#include "../playerbot.h"
#include "AiObjectContext.h"
#include "Action.h"

using namespace ai;

ActionList&& NextAction::clone(ActionList actions)
{
    ActionList res;
    res.reserve(actions.size());
    for (auto itr : actions)
        res.push_back(new NextAction(*itr));

    return std::move(res);
}

ActionList NextAction::merge(ActionList left, ActionList right)
{
    int leftSize = left.size();
    int rightSize = right.size();

    ActionList res;
    res.reserve(leftSize + rightSize);
    for (auto itr : left)
        res.push_back(new NextAction(*itr));
    for (auto itr : right)
        res.push_back(new NextAction(*itr));

    NextAction::destroy(left);
    NextAction::destroy(right);

    return res;
}

ActionList NextAction::array(std::initializer_list<NextAction*> args)
{
    ActionList res;
    res.reserve(args.size());
    for (auto elem : args)
        res.push_back(elem);

    return res;
}

void NextAction::destroy(ActionList& actions)
{
    for (auto& itr : actions)
        delete itr;

    actions.clear();
}

Value<Unit*>* Action::GetTargetValue()
{
    return context->GetValue<Unit*>(GetTargetName());
}

Unit* Action::GetTarget()
{
    return GetTargetValue()->Get();
}
