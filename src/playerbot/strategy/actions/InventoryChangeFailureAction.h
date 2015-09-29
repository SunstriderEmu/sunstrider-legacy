#pragma once

#include "../Action.h"

namespace ai
{
    class InventoryResultAction : public Action {
    public:
        InventoryResultAction(PlayerbotAI* ai) : Action(ai, "inventory change failure") {}
        virtual bool Execute(Event event);
    };
}