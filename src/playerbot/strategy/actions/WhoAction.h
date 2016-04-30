#pragma once

#include "../Action.h"
#include "InventoryAction.h"

namespace ai
{
    class WhoAction : public InventoryAction {
    public:
        WhoAction(PlayerbotAI* ai) : InventoryAction(ai, "who") {}

    public:
        virtual bool Execute(Event event);
        static map<uint32, std::string> skills;

    private:
        void InitSkills();
        std::string QueryTrade(std::string text);
        std::string QuerySkill(std::string text);
        std::string QuerySpec(std::string text);
    };

}
