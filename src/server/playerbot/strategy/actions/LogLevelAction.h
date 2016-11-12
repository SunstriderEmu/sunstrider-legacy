#pragma once

#include "../Action.h"

namespace ai
{
    class LogLevelAction : public Action {
    public:
        LogLevelAction(PlayerbotAI* ai) : Action(ai, "log") {}
        virtual bool Execute(Event event);

    public:
        static std::string logLevel2string(LogLevel level);
        static LogLevel string2logLevel(std::string level);
    };

}