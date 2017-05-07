
#include "GameTime.h"
#include "Timer.h"

namespace GameTime
{
    time_t const StartTime = time(nullptr);

    time_t GameTime = 0;
    uint32 GameMSTime = 0;

    std::chrono::system_clock::time_point GameTimeSystemPoint = std::chrono::system_clock::time_point::min();
    std::chrono::steady_clock::time_point GameTimeSteadyPoint = std::chrono::steady_clock::time_point::min();

    time_t GetStartTime()
    {
        return StartTime;
    }

    time_t GetGameTime()
    {
        return GameTime;
    }

    uint32 GetGameTimeMS()
    {
        return GameMSTime;
    }

    std::chrono::system_clock::time_point GetGameTimeSystemPoint()
    {
        return GameTimeSystemPoint;
    }

    std::chrono::steady_clock::time_point GetGameTimeSteadyPoint()
    {
        return GameTimeSteadyPoint;
    }

    uint32 GetUptime()
    {
        return uint32(GameTime - StartTime);
    }

    void UpdateGameTimers()
    {
        GameTime = time(nullptr);
        GameMSTime = GetMSTime();
        GameTimeSystemPoint = std::chrono::system_clock::now();
        GameTimeSteadyPoint = std::chrono::steady_clock::now();
    }
}
