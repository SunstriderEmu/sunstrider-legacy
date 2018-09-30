
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
    ~MapUpdater();

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

    void onceMapFinished();
    void loopMapFinished();

	//this will ensure once_map_workerThreads match the pending_once_maps count
	void spawnMissingOnceUpdateThreads();

    ProducerConsumerQueue<MapUpdateRequest*> _loop_queue;
	ProducerConsumerQueue<MapUpdateRequest*> _once_queue;

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
	void OnceWorkerThread();
};

#endif //_MAP_UPDATER_H_INCLUDED
