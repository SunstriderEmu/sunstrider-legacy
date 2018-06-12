
#include "../../playerbot.h"
#include "MageMultipliers.h"
#include "FrostMageStrategy.h"

using namespace ai;


FrostMageStrategy::FrostMageStrategy(PlayerbotAI* ai) : GenericMageStrategy(ai)
{
}

NextAction** FrostMageStrategy::getDefaultActions()
{
    return NextAction::array({ new NextAction("frostbolt", 7.0f) });
}

void FrostMageStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "icy veins",
        NextAction::array({ new NextAction("icy veins", 50.0f) })));
}

void FrostMageAoeStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "high aoe",
        NextAction::array({ new NextAction("blizzard", 40.0f) })));
}
