
#include "../../playerbot.h"
#include "TellTargetStrategy.h"

using namespace ai;


void TellTargetStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "target changed",
        NextAction::array({ std::make_shared<NextAction>("tell target", 51.0f) })));
}
