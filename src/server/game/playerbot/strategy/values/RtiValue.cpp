
#include "../../playerbot.h"
#include "RtiValue.h"

using namespace ai;

RtiValue::RtiValue(PlayerbotAI* ai)
    : ManualSetValue<std::string>(ai, "skull")
{
}
