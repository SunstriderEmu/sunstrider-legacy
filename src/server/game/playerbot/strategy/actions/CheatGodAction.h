#pragma once

#include "../Action.h"

namespace ai
{
    class CheatGodAction : public Action {
    public:
        CheatGodAction(PlayerbotAI* ai) : Action(ai, "cheatgod") {}
        virtual bool Execute(Event event);

    private:

    };

}