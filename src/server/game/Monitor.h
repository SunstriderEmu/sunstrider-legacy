#ifndef __Monitor_H
#define __Monitor_H

typedef uint64 WorldTick;

struct MapTickInfo
{
	uint32 startTime = 0;
	uint32 endTime = 0;
};

struct MapTicksInfo
{
	std::unordered_map<uint32 /*tick*/, MapTickInfo> ticks;
	uint32 currentTick = 0;
};

typedef std::unordered_map<uint32 /*instanceId*/, MapTicksInfo> InstanceTicksInfo;
typedef std::unordered_map<uint32 /*mapId*/, InstanceTicksInfo> MapUpdateInfos;

struct WorldTickInfo
{
	WorldTick worldTick = 0;
	MapUpdateInfos updateInfos;
	uint32 startTime = 0;
	uint32 endTime = 0;
};

/* 

*/
class Monitor
{
	friend class MapUpdater;
	friend class World;
	friend class MapUpdateRequest;

public:
    static Monitor* instance()
    {
        static Monitor instance;
        return &instance;
    }

	//uint32 GetAverageDiffForMap(uint32 mapId, uint32 searchUpdatesCount = 50);

private:
	// -- MapUpdater & World functions
	void MapUpdateStart(Map const& map);
	void MapUpdateEnd(Map const& map);
	void StartedWorldLoop();
	void FinishedWorldLoop();

	void Update(uint32 diff);
	// --


	Monitor();

	void UpdateGeneralInfosIfExpired(uint32 diff);
	void UpdateGeneralInfos(uint32 diff);

	WorldTick _worldTickCount;

	std::mutex _currentWorldTickLock;
	WorldTickInfo _currentWorldTickInfo;
	std::vector<WorldTickInfo> _worldTicksInfo;

	uint32 _generalInfoTimer;
};

#define sMonitor Monitor::instance()

#endif // __MONITOR_H

