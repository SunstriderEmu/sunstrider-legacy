
#include "../../playerbot.h"
#include "ShamanMultipliers.h"
#include "TotemsShamanStrategy.h"

using namespace ai;

TotemsShamanStrategy::TotemsShamanStrategy(PlayerbotAI* ai) : GenericShamanStrategy(ai)
{
}

void TotemsShamanStrategy::InitTriggers(std::list<std::shared_ptr<TriggerNode>> &triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    triggers.push_back(std::make_shared<TriggerNode>(
        "windfury totem",
        NextAction::array({ std::make_shared<NextAction>("windfury totem", 16.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "mana spring totem",
        NextAction::array({ std::make_shared<NextAction>("mana spring totem", 19.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "strength of earth totem",
        NextAction::array({ std::make_shared<NextAction>("strength of earth totem", 18.0f) })));

    triggers.push_back(std::make_shared<TriggerNode>(
        "flametongue totem",
        NextAction::array({ std::make_shared<NextAction>("flametongue totem", 17.0f) })));
}
