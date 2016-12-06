#include "Monitor.h"
#ifdef USE_GPERFTOOLS
  #include <gperftools/profiler.h>
#endif
#include "World.h"
#include "BattleGroundMgr.h"
#include "Language.h"
#include "IRCMgr.h"

Monitor::Monitor()
	: _worldTickCount(0),
	_generalInfoTimer(0),
	_startProfilerAtNextWorldLoop(false),
	_profilerRunning(false)
{
	_worldTicksInfo.reserve(DAY * 20); //already prepare 1 day worth of 20 updates per seconds
}

void Monitor::Update(uint32 diff)
{
    if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
        return;

	UpdateGeneralInfosIfExpired(diff);

	smoothTD.Update(diff);
}

void SmoothedTimeDiff::Update(uint32 diff)
{
	updateTimer += diff;
	count += 1;
	sum += diff;

	if (updateTimer >= UPDATE_SMOOTH_TD)
	{
		lastAVG = uint32(sum / float(count));
		sum = 0;
		count = 0;
		updateTimer = 0;
	}
}

void Monitor::MapUpdateStart(Map const& map)
{
    if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
        return;
    
	if (map.GetMapType() == MAP_TYPE_MAP_INSTANCED)
		return; //ignore these, not true maps

	//this function can be called from several maps at the same time
	std::lock_guard<std::mutex> lock(_currentWorldTickLock);

	InstanceTicksInfo& updateInfoListForMap = _currentWorldTickInfo.updateInfos[map.GetId()];
	MapTicksInfo& mapsTicksInfo = updateInfoListForMap[map.GetInstanceId()];
	mapsTicksInfo.currentTick++;
	auto& mapTick = mapsTicksInfo.ticks[mapsTicksInfo.currentTick];
	DEBUG_ASSERT(mapTick.endTime == 0);
	mapTick.startTime = GetMSTime();
}

void Monitor::MapUpdateEnd(Map& map)
{
    if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
        return;

	if (map.GetMapType() == MAP_TYPE_MAP_INSTANCED)
		return; //ignore these, not true maps

	//this function can be called from several maps at the same time
	_currentWorldTickLock.lock();
	MapTicksInfo& mapsTicksInfo = _currentWorldTickInfo.updateInfos[map.GetId()][map.GetInstanceId()];
	auto& mapTick = mapsTicksInfo.ticks[mapsTicksInfo.currentTick];
	if (mapTick.startTime == 0)
		return; //shouldn't happen unless we changed CONFIG_MONITORING_ENABLED while running
	mapTick.endTime = GetMSTime();
	uint32 diff = mapTick.endTime - mapTick.startTime;
	_currentWorldTickLock.unlock();

	_monitDynamicLoS.UpdateForMap(map, diff);

	_lastMapDiffsLock.lock();
	_lastMapDiffs[uint64(&map)] = diff;
	_lastMapDiffsLock.unlock();
}

bool Monitor::ProfileNextUpdate()
{
#ifdef USE_GPERFTOOLS
	_startProfilerAtNextWorldLoop = true;
	return true;
#else
	return false;
#endif
}

void Monitor::StartedWorldLoop()
{
	if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
		return;

	_worldTickCount++;
	_currentWorldTickInfo.worldTick = _worldTickCount;
	_currentWorldTickInfo.startTime = GetMSTime();

#ifdef USE_GPERFTOOLS
	if (_startProfilerAtNextWorldLoop)
	{
		_startProfilerAtNextWorldLoop = false;
		_profilerRunning = true;
		std::string filename = std::string(std::to_string(std::time(NULL)) + "prof");
		ProfilerStart(filename.c_str());
	}
#endif //USE_GPERFTOOLS
}

void Monitor::FinishedWorldLoop()
{
	if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
		return;

	if (_currentWorldTickInfo.startTime == 0)
		return; //shouldn't happen unless we changed CONFIG_MONITORING_ENABLED while running

	_currentWorldTickInfo.endTime = GetMSTime();
	_currentWorldTickInfo.diff = _currentWorldTickInfo.endTime - _currentWorldTickInfo.startTime;
	_currentWorldTickInfo.worldTick = _worldTickCount;

	_monitAutoReboot.Update(_currentWorldTickInfo.diff);
	_monitAlert.UpdateForWorld(_currentWorldTickInfo.diff); 
	

	//Store current world tick and reset it
	_worldTicksInfo.push_back(std::move(_currentWorldTickInfo));
	_currentWorldTickInfo = {};

#ifdef USE_GPERFTOOLS
	if (_profilerRunning)
	{
		ProfilerStop();
		_profilerRunning = false;
		ChatHandler::SendGlobalGMSysMessage("Profiling has been stopped");
	}
	//+ some auto profiling trigger logic?
#endif //USE_GPERFTOOLS
}

void Monitor::UpdateGeneralInfosIfExpired(uint32 diff)
{
	uint32 generalInfosUpdateTimeout = IN_MILLISECONDS * sWorld->getConfig(CONFIG_MONITORING_GENERALINFOS_UPDATE);
	if (!generalInfosUpdateTimeout)
		return;

	if (_generalInfoTimer > generalInfosUpdateTimeout)
	{
		UpdateGeneralInfos(diff);
		_generalInfoTimer = 0;
	}
	_generalInfoTimer += diff;
}


void Monitor::UpdateGeneralInfos(uint32 diff)
{
	time_t now = time(nullptr);

	SQLTransaction trans = LogsDatabase.BeginTransaction();

	/* players */
	trans->PAppend("INSERT INTO mon_players (time, active, queued) VALUES (%u, %u, %u)", (uint32)now, sWorld->GetActiveSessionCount(), sWorld->GetQueuedSessionCount());

	/* time diff */
	uint32 smoothTD = GetSmoothTimeDiff();
	if(smoothTD) //it may be not available yet
		trans->PAppend("INSERT INTO mon_timediff (time, diff) VALUES (%u, %u)", (uint32)now, GetSmoothTimeDiff());

	/* maps */
	std::string maps = "eastern kalimdor outland karazhan hyjal ssc blacktemple tempestkeep zulaman warsong arathi eye alterac arenas sunwell";
	std::stringstream cnts;
	int arena_cnt = 0;
	arena_cnt += sMapMgr->GetNumPlayersInMap(562); /* nagrand */
	arena_cnt += sMapMgr->GetNumPlayersInMap(559); /* blade's edge */
	arena_cnt += sMapMgr->GetNumPlayersInMap(572); /* lordaeron */

	int mapIds[14] = { 0, 1, 530, 532, 534, 548, 564, 550, 568, 489, 529, 566, 30, 580 };
	for (int & mapId : mapIds)
		trans->PAppend("INSERT INTO mon_maps (time, map, players) VALUES (%u, %u, %u)", (uint32)now, mapId, sMapMgr->GetNumPlayersInMap(mapId));
	// arenas
	trans->PAppend("INSERT INTO mon_maps (time, map, players) VALUES (%u, 559, %u)", (uint32)now, arena_cnt); // Nagrand!

	/* battleground queue time */
	std::string bgs = "alterac warsong arathi eye 2v2 3v3 5v5";
	std::stringstream bgs_wait;

	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_AV].GetAvgTime() << " ";
	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_WS].GetAvgTime() << " ";
	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_AB].GetAvgTime() << " ";
	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_EY].GetAvgTime() << " ";
	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_2v2].GetAvgTime() << " ";
	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_3v3].GetAvgTime() << " ";
	bgs_wait << sBattlegroundMgr->m_BattlegroundQueues[BATTLEGROUND_QUEUE_5v5].GetAvgTime();

	/* races && classes */
	std::string races = "human orc dwarf nightelf undead tauren gnome troll bloodelf draenei";
	std::stringstream ssraces;

	std::string classes = "warrior paladin hunter rogue priest shaman mage warlock druid";
	std::stringstream ssclasses;

	uint32 racesCount[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint32 classesCount[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	auto lock = HashMapHolder<Player>::GetLock();
	lock->lock();
	HashMapHolder<Player>::MapType const & m = ObjectAccessor::GetPlayers();
	for (auto & itr : m) {
		racesCount[itr.second->GetRace()]++;
		classesCount[itr.second->GetClass()]++;
	}
	lock->unlock();

	ssraces << racesCount[1] << " " << racesCount[2] << " " << racesCount[3] << " ";
	ssraces << racesCount[4] << " " << racesCount[5] << " " << racesCount[6] << " ";
	ssraces << racesCount[7] << " " << racesCount[8] << " " << racesCount[10] << " ";
	ssraces << racesCount[11];

	ssclasses << classesCount[1] << " " << classesCount[2] << " " << classesCount[3] << " ";
	ssclasses << classesCount[4] << " " << classesCount[5] << " " << classesCount[7] << " ";
	ssclasses << classesCount[8] << " " << classesCount[9] << " " << classesCount[11] << " ";

	for (int i = 1; i < 12; i++) {
		if (i != 9)
			trans->PAppend("INSERT INTO mon_races (time, race, players) VALUES (%u, %u, %u)", (uint32)now, i, racesCount[i]);
	}

	for (int i = 1; i < 12; i++) {
		if (i != 6 && i != 10)
			trans->PAppend("INSERT INTO mon_classes (time, `class`, players) VALUES (%u, %u, %u)", (uint32)now, i, classesCount[i]);
	}

	LogsDatabase.CommitTransaction(trans);
}

uint32 Monitor::GetAverageWorldDiff(uint32 searchCount)
{
	if (!searchCount)
		return 0;

	std::lock_guard<std::mutex> lock(_worldTicksInfoLock);

	if (_worldTicksInfo.size() < searchCount)
		return 0; //not enough data yet

	uint32 sum = 0;
	for (uint32 i = _worldTicksInfo.size() - searchCount; i != _worldTicksInfo.size(); i++)
		sum += _worldTicksInfo[i].diff;

	uint32 avgTD = uint32(sum / float(searchCount));
	
	return avgTD;
}

uint32 Monitor::GetAverageDiffForMap(Map const& map, uint32 searchCount)
{
	if (!searchCount)
		return 0;

	std::lock_guard<std::mutex> lock(_worldTicksInfoLock);

	if (_worldTicksInfo.size() < searchCount)
		return 0; //not enough data yet

	uint32 sum = 0;
	uint32 count = 0;
	for (uint32 i = _worldTicksInfo.size() - searchCount; i != _worldTicksInfo.size(); i++)
	{
		auto ticks = _worldTicksInfo[i].updateInfos[map.GetId()][map.GetInstanceId()].ticks;
		for (auto itr : ticks)
		{
			sum += itr.second.diff();
			count++;
		}
	}

	if (!count)
		return 0;

	uint32 avg = uint32(sum / float(count));

	return avg;
}

uint32 Monitor::GetLastDiffForMap(Map const& map)
{
	std::lock_guard<std::mutex> lock(_lastMapDiffsLock);
	auto itr = _lastMapDiffs.find(uint64(&map));
	if (itr == _lastMapDiffs.end())
		return 0;

	return itr->second;
}

void MonitorAutoReboot::Update(uint32 diff)
{
	uint32 searchCount = sWorld->getConfig(CONFIG_MONITORING_LAG_AUTO_REBOOT_COUNT);
	uint32 thresholdDiff = sWorld->getConfig(CONFIG_MONITORING_ABNORMAL_WORLD_UPDATE_DIFF);
	if (!searchCount || !thresholdDiff)
		return;

	checkTimer += diff;

	//not yet time to check
	if (checkTimer < CHECK_INTERVAL)
		return;

	checkTimer = 0;

	uint32 avgDiff = sMonitor->GetAverageWorldDiff(searchCount);
	if (!avgDiff) {
		return; //not enough data atm
	}

	if (avgDiff >= thresholdDiff && !sWorld->IsShuttingDown())
	{
		// Trigger restart
		sWorld->ShutdownServ(15 * MINUTE, SHUTDOWN_MASK_RESTART, "Auto-restart triggered due to abnormal server load.");
	}
}

void MonitorDynamicLoS::UpdateForMap(Map& map, uint32 diff)
{
	//is it time to check?
	auto& timer = _mapCheckTimers[uint64(&map)].timer;
	timer += diff;

	if (timer < CHECK_INTERVAL)
		return;

	timer = 0;

	uint32 abnormalDiff = sWorld->getConfig(CONFIG_MONITORING_ABNORMAL_MAP_UPDATE_DIFF);
	if (!abnormalDiff)
		return;

	//do we have enough data?
	uint32 avgTD = sMonitor->GetAverageDiffForMap(map, SEARCH_COUNT);
	if (!avgTD)
		return; //not enough data atm

	//is it laggy?
	float baseVisibilityRange = map.GetDefaultVisibilityDistance();
	float currentVisibilityRange = map.GetVisibilityRange();
	if (avgTD >= abnormalDiff)
	{
		// Map is laggy, update visibility distance
		float minDist = float(sWorld->getConfig(CONFIG_MONITORING_DYNAMIC_LOS_MINDIST));

		/* New visibility distance calculation. This is a first draft, don't hesitate to tweak it.
		Example with: IDEAL_DIFF = 200; avgTD = 420; currentVisib = 120 :
		dynamicDist = 80
		*/
		float dynamicDist = std::max(minDist, (IDEAL_DIFF / float(avgTD)) * 1.4f * currentVisibilityRange);

		map.SetVisibilityDistance(dynamicDist);
	} else if (avgTD <= IDEAL_DIFF)
	{
		//td is normal, restore visibility range if needed
		if (currentVisibilityRange != baseVisibilityRange)
			map.InitVisibilityDistance();
	}
}

void MonitorAlert::UpdateForWorld(uint32 diff)
{
	uint32 searchCount = sWorld->getConfig(CONFIG_MONITORING_ALERT_THRESHOLD_COUNT);
	uint32 abnormalDiff = sWorld->getConfig(CONFIG_MONITORING_ABNORMAL_WORLD_UPDATE_DIFF);
	if (!searchCount || !abnormalDiff)
		return; //not enabled in config

	_worldCheckTimer.timer += diff;
	if (_worldCheckTimer.timer < CHECK_INTERVAL)
		return;

	_worldCheckTimer.timer = 0;

	uint32 avgTD = sMonitor->GetAverageWorldDiff(searchCount);
	if (!avgTD)
		return; //not enough data atm

	if (avgTD < abnormalDiff)
		return; //all okay

	std::string msg = "/!\\ World updates have been slow for the last " + std::to_string(searchCount) + " updates with an average of " + std::to_string(avgTD);
	ChatHandler::SendGlobalGMSysMessage(msg.c_str());
}
