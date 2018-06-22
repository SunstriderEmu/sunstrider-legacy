
#include "../../playerbot.h"
#include "ThreatStrategy.h"
#include "../../PlayerbotAIConfig.h"
#include "../actions/GenericSpellActions.h"

using namespace ai;

float ThreatMultiplier::GetValue(Action* action)
{
    if (action == NULL || action->getThreatType() == ACTION_THREAT_NONE)
        return 1.0f;

    if (action->getThreatType() == ACTION_THREAT_AOE)
    {
        uint8 threat = AI_VALUE2(uint8, "threat", "aoe");
        if (threat >= 90)
            return 0.0f;
    }

    uint8 threat = AI_VALUE2(uint8, "threat", "current target");

    if (threat >= 90)
        return 0.0f;

    return 1.0f;
}

void ThreatStrategy::InitMultipliers(std::list<std::shared_ptr<Multiplier>> &multipliers)
{
    multipliers.push_back(std::make_shared<ThreatMultiplier>(ai));
}
