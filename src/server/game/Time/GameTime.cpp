
#include "GameTime.h"
#include "Timer.h"

namespace GameTime
{
    time_t const StartTime = time(nullptr);

    time_t GameTime = time(nullptr);
    uint32 GameMSTime = 0;

    std::chrono::system_clock::time_point GameTimeSystemPoint = std::chrono::system_clock::time_point::min();
    std::chrono::steady_clock::time_point GameTimeSteadyPoint = std::chrono::steady_clock::time_point::min();

    time_t GetStartTime()
    {
        return StartTime;
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

namespace WorldGameTime
{
    time_t GetGameTime()
    {
        return GameTime::GameTime;
    }

    uint32 GetGameTimeMS()
    {
        return GameTime::GameMSTime;
    }

    std::chrono::system_clock::time_point GetGameTimeSystemPoint()
    {
        return GameTime::GameTimeSystemPoint;
    }

    std::chrono::steady_clock::time_point GetGameTimeSteadyPoint()
    {
        return GameTime::GameTimeSteadyPoint;
    }
}