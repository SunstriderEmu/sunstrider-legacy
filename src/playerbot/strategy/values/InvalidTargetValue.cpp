
#include "../../playerbot.h"
#include "InvalidTargetValue.h"
#include "../../PlayerbotAIConfig.h"

using namespace ai;

bool InvalidTargetValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (qualifier == "current target")
    {
        return !target ||
                target->GetMapId() != bot->GetMapId() ||
                target->IsDead() ||
                target->IsPolymorphed() ||
                target->IsCharmed() ||
                target->IsFeared() ||
                target->HasUnitState(UNIT_STATE_ISOLATED) ||
                target->IsFriendlyTo(bot) ||
                !bot->IsWithinDistInMap(target, sPlayerbotAIConfig.sightDistance) ||
                !bot->IsWithinLOSInMap(target);
    }

    return !target;
}
