
#include "../../playerbot.h"
#include "StayStrategy.h"

using namespace ai;

ActionList StayStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("stay", 1.0f) });
}

