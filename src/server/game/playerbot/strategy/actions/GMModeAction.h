#pragma once

#include "../Action.h"

namespace ai
{
    class GMModeAction : public Action {
    public:
        GMModeAction(PlayerbotAI* ai) : Action(ai, "gm") {}
        virtual bool Execute(Event event);

    private:

    };

}