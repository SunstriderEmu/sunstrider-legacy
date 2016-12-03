#include "Monitor.h"
#ifdef USE_GPERFTOOLS
  #include <gperftools/profiler.h>
#endif
#include "World.h"
#include "BattleGroundMgr.h"


Monitor::Monitor() 
	: _worldTickCount(0), _generalInfoTimer(0)
{
	_worldTicksInfo.reserve(DAY * 20); //already prepare 1 day worth of 20 updates per seconds
}

void Monitor::Update(uint32 diff)
{
    if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
        return;

	UpdateGeneralInfos(diff);

    //dynamic visible distance todo
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

#ifdef USE_GPERFTOOLS
	//ProfilerStart("filename");
#endif //USE_GPERFTOOLS
}

void Monitor::MapUpdateEnd(Map const& map)
{
    if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
        return;

	if (map.GetMapType() == MAP_TYPE_MAP_INSTANCED)
		return; //ignore these, not true maps

	//this function can be called from several maps at the same time
	std::lock_guard<std::mutex> lock(_currentWorldTickLock);

	InstanceTicksInfo& updateInfoListForMap = _currentWorldTickInfo.updateInfos[map.GetId()];
	MapTicksInfo& mapsTicksInfo = updateInfoListForMap[map.GetInstanceId()];
	auto& mapTick = mapsTicksInfo.ticks[mapsTicksInfo.currentTick];
	DEBUG_ASSERT(mapTick.startTime != 0);
	mapTick.endTime = GetMSTime();

	//CONFIG_MONITOR_ALERT_THRESHOLD_DIFF
	//CONFIG_MONITOR_ALERT_THRESHOLD_COUNT
	//trigger profiling for next update. cannot occur more than once per minute ?

#ifdef USE_GPERFTOOLS
    //ProfilerStop();
#endif //USE_GPERFTOOLS
}

void Monitor::StartedWorldLoop()
{
	if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
		return;

	_worldTickCount++;
	_currentWorldTickInfo.worldTick = _worldTickCount;
	_currentWorldTickInfo.startTime = GetMSTime();
}

void Monitor::FinishedWorldLoop()
{
	if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
		return;

	_currentWorldTickInfo.endTime = GetMSTime();
	_currentWorldTickInfo.worldTick = _worldTickCount;
	_worldTicksInfo.push_back(std::move(_currentWorldTickInfo));
	_currentWorldTickInfo = {}; //reset
}

void Monitor::UpdateGeneralInfosIfExpired(uint32 diff)
{
	if (!sWorld->getConfig(CONFIG_MONITORING_ENABLED))
		return;

	uint32 generalInfosUpdateTimeout = sWorld->getConfig(CONFIG_MONITORING_GENERALINFOS_UPDATE);
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

	trans->PAppend("INSERT INTO mon_timediff (time, diff) VALUES (%u, %u)", (uint32)now, sWorld->GetFastTimeDiff());

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

	/* max creature guid */

	//sprintf(data, "%u", sObjectMgr->GetMaxCreatureGUID());

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
