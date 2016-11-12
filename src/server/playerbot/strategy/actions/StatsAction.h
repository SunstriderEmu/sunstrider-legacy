#pragma once

#include "../Action.h"

namespace ai
{
    class StatsAction : public Action {
    public:
        StatsAction(PlayerbotAI* ai) : Action(ai, "stats") {}
        virtual bool Execute(Event event);

    private:
        void ListBagSlots(std::ostringstream &out);
        void ListXP(std::ostringstream &out);
        void ListRepairCost(std::ostringstream &out);
        void ListGold(std::ostringstream &out);
        uint32 EstRepairAll();
        uint32 EstRepair(uint16 pos);

    };

}