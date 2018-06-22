
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "FrostMageStrategy.h"

using namespace ai;


FrostMageStrategy::FrostMageStrategy(PlayerbotAI* ai) : GenericMageStrategy(ai)
{
}

ActionList FrostMageStrategy::getDefaultActions()
{
    return NextAction::array({ std::make_shared<NextAction>("frostbolt", 7.0f) });
}

void FrostMageStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "icy veins",
        NextAction::array({ std::make_shared<NextAction>("icy veins", 50.0f) })));
}

void FrostMageAoeStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "high aoe",
        NextAction::array({ std::make_shared<NextAction>("blizzard", 40.0f) })));
}
