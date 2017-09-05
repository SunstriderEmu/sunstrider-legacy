
#ifndef __GAMETIME_H
#define __GAMETIME_H

#include "Define.h"

#include <chrono>

namespace GameTime
{
    // Server start time
    TC_GAME_API time_t GetStartTime();

    // Current server time (unix) in seconds
    TC_GAME_API time_t GetGameTime();

    // Milliseconds since server start
    TC_GAME_API uint32 GetGameTimeMS();

    /// Current chrono system_clock time point
    TC_GAME_API std::chrono::system_clock::time_point GetGameTimeSystemPoint();

    /// Current chrono steady_clock time point
    TC_GAME_API std::chrono::steady_clock::time_point GetGameTimeSteadyPoint();

    /// Uptime (in secs)
    TC_GAME_API uint32 GetUptime();

    void UpdateGameTimers();
}

#endif
