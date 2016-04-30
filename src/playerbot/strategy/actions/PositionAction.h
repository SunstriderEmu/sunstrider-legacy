#pragma once

#include "../Action.h"
#include "MovementActions.h"

namespace ai
{
    class PositionAction : public Action
    {
    public:
        PositionAction(PlayerbotAI* ai) : Action(ai, "position")
        {}

        virtual bool Execute(Event event);

    };

    class MoveToPositionAction : public MovementAction
    {
    public:
        MoveToPositionAction(PlayerbotAI* ai, std::string qualifier) : MovementAction(ai, "move to position"), qualifier(qualifier)
        {}

        virtual bool Execute(Event event);

    protected:
        std::string qualifier;
    };

    class GuardAction : public MoveToPositionAction
    {
    public:
        GuardAction(PlayerbotAI* ai) : MoveToPositionAction(ai, "guard")
        {}
    };

}
