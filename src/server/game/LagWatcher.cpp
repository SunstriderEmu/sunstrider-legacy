#include "LagWatcher.h"
#ifdef USE_GPERFTOOLS
  #include <gperftools/profiler.h>
#endif
#include "World.h"

struct MapWatchData
{
    struct
    {
        time_t start;
        time_t end;
    } lastUpdate;
};

std::unordered_map<uint32, MapWatchData> mapWatchData;

void LagWatcher::Update()
{
    if (!sWorld->getConfig(CONFIG_LAGWATCHER_ENABLE))
        return;

    //dynamic visible distance todo
}

void LagWatcher::MapUpdateStart(Map const& map)
{
    if (!sWorld->getConfig(CONFIG_LAGWATCHER_ENABLE))
        return;

    if (!sWorld->getConfig(CONFIG_LAGWATCHER_PROFILING_ENABLE))
        return;
    
    MapWatchData& data = mapWatchData[map.GetId()];
    data.lastUpdate.start = GetMSTime();

    //CONFIG_LAGWATCHER_PROFILING_RATE
    //CONFIG_LAGWATCHER_PROFILING_THRESHOLD
    //CONFIG_LAGWATCHER_PROFILING_THRESHOLD_DIFF
#ifdef USE_GPERFTOOLS
    //ProfilerStart("filename");
#endif //USE_GPERFTOOLS
}

void LagWatcher::MapUpdateEnd(Map const& map)
{
    if (!sWorld->getConfig(CONFIG_LAGWATCHER_ENABLE))
        return;

    if (!sWorld->getConfig(CONFIG_LAGWATCHER_PROFILING_ENABLE))
        return;

    MapWatchData& data = mapWatchData[map.GetId()];
    data.lastUpdate.end = GetMSTime();
    uint32 diff = GetMSTimeDiff(data.lastUpdate.start, data.lastUpdate.end);

#ifdef USE_GPERFTOOLS
    //ProfilerStop();
#endif //USE_GPERFTOOLS
}