#include "ClassSpellsDamage.h"
#include <algorithm>

float CalcCatMinDmg(uint32 lvl)
{
    return std::min(lvl, 60u) * 0.8f - 4;
}

uint32 ClassSpellsDamage::Druid::GetCatMinDmg(uint32 level /*= 70*/)
{
    return uint32(CalcCatMinDmg(level));
}

uint32 ClassSpellsDamage::Druid::GetCatMaxDmg(uint32 level /*= 70*/)
{
    return uint32(CalcCatMinDmg(level) * 1.5f);
}