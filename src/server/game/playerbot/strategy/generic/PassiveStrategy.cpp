
#include "../../playerbot.h"
#include "PassiveStrategy.h"
#include "../PassiveMultiplier.h"

using namespace ai;


void PassiveStrategy::InitMultipliers(std::list<std::shared_ptr<Multiplier>> &multipliers)
{
    multipliers.push_back(std::make_shared<PassiveMultiplier>(ai));
}

