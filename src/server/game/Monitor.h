#ifndef __MONITOR_H
#define __MONITOR_H

/*
Ideas:
- Allow to trigger profiling at next udpate, by command or automatically every X according to config
- Some form of packet monitoring?
- A command basically checking "WHY DO I LAG?" enabling various checks for one loop
*/

typedef uint64 WorldTick;

struct MapTickInfo
{
	uint32 startTime = 0;
	uint32 endTime = 0;

	uint32 diff() { return endTime - startTime; }
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
	WorldTick worldTick   = 0;
	MapUpdateInfos updateInfos;
	uint32 startTime      = 0;
	uint32 endTime        = 0;
	uint32 diff           = 0; //value calculated from startTime and endTime
};

class MonitorAutoReboot
{
public:
	const uint32 CHECK_INTERVAL = 15 * MINUTE * IN_MILLISECONDS;

	void Update(uint32 diff);

private:
	uint32 checkTimer = 0;
};

struct CheckTimer //this is used to get a default value on first unordered_map access for one map
{
	uint32 timer = 0;
};

class MonitorDynamicLoS
{
	const uint32 IDEAL_DIFF = 200;
	const uint32 CHECK_INTERVAL = 10 * MINUTE * IN_MILLISECONDS;

public:
	const uint32 SEARCH_COUNT = 4500; // 15 minutes at 20 updates per seconds
	void UpdateForMap(Map& map, uint32 diff);

private:
	std::mutex _mapCheckTimersLock;
	std::unordered_map<uint64 /*map pointer as id*/, CheckTimer> _mapCheckTimers;
};

class MonitorAlert
{
public:
	const uint32 CHECK_INTERVAL = 30 * MINUTE * IN_MILLISECONDS;

	void UpdateForWorld(uint32 diff);
private:
	//std::mutex _mapCheckTimersLock;
	//std::unordered_map<uint64 /*map pointer as id*/, CheckTimer> _mapCheckTimers;
	CheckTimer _worldCheckTimer;
};

//Smoothed value of lasts update times, updated every 5 minutes
struct SmoothedTimeDiff
{
	uint32 const UPDATE_SMOOTH_TD = 1 * MINUTE * IN_MILLISECONDS;

	uint32 Get() const { return lastAVG; }
	void Update(uint32 diff);
private:
	uint32 updateTimer = 0;
	uint32 lastAVG = 0;
	uint32 sum = 0;
	uint32 count = 0;
};

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

	// Returns average world diff for the last <searchCount> loops. Return 0 if not enough loops available atm.
	uint32 GetAverageWorldDiff(uint32 searchCount);
	// Returns average map diff for the last <searchCount> world loops. Return 0 if not enough loops available atm.
	uint32 GetAverageDiffForMap(Map const& map, uint32 searchCount);
	uint32 GetLastDiffForMap(Map const& map);

	// Flattened timediff upated every minute. This is a cached value.
	uint32 GetSmoothTimeDiff() const { return smoothTD.Get(); }
private:
	// -- MapUpdater & World functions
	void MapUpdateStart(Map const& map);
	void MapUpdateEnd(Map& map);
	void StartedWorldLoop();
	void FinishedWorldLoop();

	void Update(uint32 diff);
	// --


	Monitor();

	void UpdateGeneralInfosIfExpired(uint32 diff);
	void UpdateGeneralInfos(uint32 diff);

	WorldTick _worldTickCount;


	//info about the current world tick, this get reset at FinishedWorldLoop
	std::mutex _currentWorldTickLock;
	WorldTickInfo _currentWorldTickInfo;

	//info for all world loops since Monitor is running
	std::mutex _worldTicksInfoLock;
	std::vector<WorldTickInfo> _worldTicksInfo;

	//last map diffs. This is redundant with info in _worldTicksInfo but this allows for greater speed and to avoid locking it.
	std::unordered_map<uint64 /* map pointer*/, uint32 /* diff*/> _lastMapDiffs;
	std::mutex _lastMapDiffsLock;

	//time since last general info check
	uint32 _generalInfoTimer;

	MonitorAutoReboot _monitAutoReboot;
	MonitorDynamicLoS _monitDynamicLoS;
	MonitorAlert      _monitAlert;

	SmoothedTimeDiff smoothTD;
};

#define sMonitor Monitor::instance()

#endif // __MONITOR_H
