
#include "../../playerbot.h"
#include "StayStrategy.h"

using namespace ai;

ActionList StayStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("stay", 1.0f) });
}

