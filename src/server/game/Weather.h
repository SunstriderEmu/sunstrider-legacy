
/// \addtogroup world
/// @{
/// \file

#ifndef __WEATHER_H
#define __WEATHER_H

#include "SharedDefines.h"
#include "Timer.h"

class Player;

enum WeatherState : int
{
    WEATHER_STATE_FINE              = 0,
    WEATHER_STATE_LIGHT_RAIN        = 3,
    WEATHER_STATE_MEDIUM_RAIN       = 4,
    WEATHER_STATE_HEAVY_RAIN        = 5,
    WEATHER_STATE_LIGHT_SNOW        = 6,
    WEATHER_STATE_MEDIUM_SNOW       = 7,
    WEATHER_STATE_HEAVY_SNOW        = 8,
    WEATHER_STATE_LIGHT_SANDSTORM   = 22,
    WEATHER_STATE_MEDIUM_SANDSTORM  = 41,
    WEATHER_STATE_HEAVY_SANDSTORM   = 42,
    WEATHER_STATE_THUNDERS          = 86,
    WEATHER_STATE_BLACKRAIN         = 90
};

struct WeatherZoneChances;

/// Weather for one zone
class TC_GAME_API Weather
{
    public:
        Weather(uint32 zone, WeatherZoneChances const* weatherChances);
        ~Weather() = default;;
        bool ReGenerate();
        bool UpdateWeather();
        void SendWeatherUpdateToPlayer(Player *player);
        static void SendFineWeatherUpdateToPlayer(Player *player);
        void SetWeather(WeatherType type, float grade);
        /// For which zone is this weather?
        uint32 GetZone() { return m_zone; };
        bool Update(time_t diff);
    private:
        WeatherState GetWeatherState() const;
        uint32 m_zone;
        WeatherType m_type;
        float m_grade;
        IntervalTimer m_timer;
        WeatherZoneChances const* m_weatherChances;
};
#endif

