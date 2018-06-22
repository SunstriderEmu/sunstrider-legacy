
#include "../../playerbot.h"
#include "HunterMultipliers.h"
#include "HunterBuffStrategies.h"

using namespace ai;

void HunterBuffDpsStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "aspect of the hawk", 
        NextAction::array({ std::make_shared<NextAction>("aspect of the hawk", 90.0f) })));
}

void HunterNatureResistanceStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "aspect of the wild", 
        NextAction::array({ std::make_shared<NextAction>("aspect of the wild", 90.0f) })));
}


void HunterBuffSpeedStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "aspect of the pack",
        NextAction::array({ std::make_shared<NextAction>("aspect of the pack", 10.0f) })));
}

void HunterBuffManaStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    triggers.push_back(std::make_shared<TriggerNode>(
        "aspect of the viper",
        NextAction::array({ std::make_shared<NextAction>("aspect of the viper", 10.0f) })));
}
