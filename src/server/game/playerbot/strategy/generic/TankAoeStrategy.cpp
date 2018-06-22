
#include "../../playerbot.h"
#include "TankAoeStrategy.h"

using namespace ai;

void TankAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "tank aoe",
        NextAction::array({ std::make_shared<NextAction>("tank assist", 50.0f) })));
}
