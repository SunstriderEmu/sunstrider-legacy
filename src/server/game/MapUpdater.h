/*
* Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MAP_UPDATER_H_INCLUDED
#define _MAP_UPDATER_H_INCLUDED

#include "Define.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include "ProducerConsumerQueue.h"

class MapUpdateRequest;
class Map;

/**
Two kinds of maps:
- Maps we update only once (continents, instances base maps)
- Maps we keep updating until the first type has finished (instances, battlegrounds)
*/
class MapUpdater
{
public:

    MapUpdater() : _cancelationToken(false), _enable_updates_loop(false), pending_once_maps(0), pending_loop_maps(0) {}
    ~MapUpdater() = default;;

    friend class MapUpdateRequest;

    void schedule_update(Map& map, uint32 diff);

    void waitUpdateOnces();
	//when enabled, instance update requests are re enqueued instead of consumed
	void enableUpdateLoop(bool enable);
	void waitUpdateLoops();

    void activate(size_t num_threads);

    void deactivate();

    bool activated();
private:

	void onceMapsFinished();
	void loopMapsFinished();

	ProducerConsumerQueue<MapUpdateRequest*> _loop_queue;

    std::vector<std::thread> _loop_maps_workerThreads; 
	std::vector<std::thread> _once_maps_workerThreads; //created/deleted at each loop, one for each continent
    std::atomic<bool> _cancelationToken;
	std::atomic<bool> _enable_updates_loop;

    std::mutex _lock;
	//notified when an update loop request is finished
    std::condition_variable _loops_finished_condition;
	//notified when an update once request is finished
	std::condition_variable _onces_finished_condition;
	std::atomic<uint32> pending_once_maps;
    std::atomic<uint32> pending_loop_maps;

	/* Loop workers keep running and processing _loop_queue, updating maps and requeuing them afterwards.
	When onceMapsFinished becomes true, the worker finish the current request and delete the request instead of requeuing it.
	*/
    void LoopWorkerThread(std::atomic<bool>* onceMapsFinished);
	//Single update, descrease pending_once_maps when done
	void OnceWorkerThread(MapUpdateRequest* request);
};

#endif //_MAP_UPDATER_H_INCLUDED
