
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "TotemsShamanStrategy.h"

using namespace ai;

TotemsShamanStrategy::TotemsShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
}

void TotemsShamanStrategy::InitTriggers(std::list<TriggerNode*> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "windfury totem",
        NextAction::array({ new NextAction("windfury totem", 16.0f) })));

    triggers.push_back(new TriggerNode(
        "mana spring totem",
        NextAction::array({ new NextAction("mana spring totem", 19.0f) })));

    triggers.push_back(new TriggerNode(
        "strength of earth totem",
        NextAction::array({ new NextAction("strength of earth totem", 18.0f) })));

    triggers.push_back(new TriggerNode(
        "flametongue totem",
        NextAction::array({ new NextAction("flametongue totem", 17.0f) })));
}
