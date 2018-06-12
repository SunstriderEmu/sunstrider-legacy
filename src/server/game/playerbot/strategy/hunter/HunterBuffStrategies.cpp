
#include "../../playerbot.h"
#include "HunterMultipliers.h"
#include "HunterBuffStrategies.h"

using namespace ai;

void HunterBuffDpsStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the hawk", 
        NextAction::array({ new NextAction("aspect of the hawk", 90.0f) })));
}

void HunterNatureResistanceStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the wild", 
        NextAction::array({ new NextAction("aspect of the wild", 90.0f) })));
}


void HunterBuffSpeedStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the pack",
        NextAction::array({ new NextAction("aspect of the pack", 10.0f) })));
}

void HunterBuffManaStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "aspect of the viper",
        NextAction::array({ new NextAction("aspect of the viper", 10.0f) })));
}
