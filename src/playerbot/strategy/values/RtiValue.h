#pragma once
#include "../Value.h"

namespace ai
{
    class RtiValue : public ManualSetValue<std::string>
	{
	public:
        RtiValue(PlayerbotAI* ai);
    };
}
