
#include <mutex>
#include <condition_variable>

#include "MapUpdater.h"
#include "Map.h"
#include "Monitor.h"
#include "World.h"
#include "MapManager.h"

#define MINIMUM_MAP_UPDATE_INTERVAL 30

class MapUpdateRequest
{
    private:

        Map& m_map;
        MapUpdater& m_updater;
        uint32 m_diff;
        uint32 m_loopCount;

    public:

        MapUpdateRequest(Map& m, MapUpdater& u, uint32 d) :
            m_map(m), 
            m_updater(u), 
            m_diff(d), 
            m_loopCount(0)
        {
        }

        Map const* getMap() { return &m_map; }

        void call()
        {
            sMonitor->MapUpdateStart(m_map);
            m_map.DoUpdate(m_diff, MINIMUM_MAP_UPDATE_INTERVAL);
            sMonitor->MapUpdateEnd(m_map);
            m_loopCount++;
        }
};

void MapUpdater::activate(size_t num_threads)
{
    //spawn instances & battlegrounds threads
    for (size_t i = 0; i < num_threads; ++i)
        _loop_maps_workerThreads.push_back(std::thread(&MapUpdater::LoopWorkerThread, this, &_enable_updates_loop));

    //continents threads are spawned later when request are received
}

void MapUpdater::deactivate()
{
    _cancelationToken = true;

    _loop_queue.Cancel();
    _once_queue.Cancel();

    waitUpdateOnces();
    waitUpdateLoops();

    for (auto& thread : _once_maps_workerThreads)
        thread.join();

    for (auto& thread : _loop_maps_workerThreads)
        thread.join();
}

void MapUpdater::waitUpdateOnces()
{
    std::unique_lock<std::mutex> lock(_lock);

    while (pending_once_maps > 0)
        _onces_finished_condition.wait(lock);

    lock.unlock();
}

void MapUpdater::enableUpdateLoop(bool enable)
{
    _enable_updates_loop = enable;
}

void MapUpdater::waitUpdateLoops()
{
    std::unique_lock<std::mutex> lock(_lock);

    while (pending_loop_maps > 0)
        _loops_finished_condition.wait(lock);

    lock.unlock();
}

void MapUpdater::spawnMissingOnceUpdateThreads()
{
    for (uint32 i = _once_maps_workerThreads.size(); i < pending_once_maps; i++)
        _once_maps_workerThreads.push_back(std::thread(&MapUpdater::OnceWorkerThread, this));
}

void MapUpdater::schedule_update(Map& map, uint32 diff)
{
    std::lock_guard<std::mutex> lock(_lock);

    MapUpdateRequest* request = new MapUpdateRequest(map, *this, diff);
    // MapInstanced re schedule the instances it contains by itself, so we want to call it only once
    // Also currently test maps needs to be updated once per world update
    if((map.Instanceable() && map.GetMapType() != MAP_TYPE_MAP_INSTANCED) || map.GetMapType() == MAP_TYPE_TEST_MAP) 
    { 
        pending_loop_maps++;
        _loop_queue.Push(request);
    } 
    else 
    {
        pending_once_maps++;
        _once_queue.Push(request);
    }

    spawnMissingOnceUpdateThreads();
}

bool MapUpdater::activated()
{
    return _loop_maps_workerThreads.size() > 0;
}

void MapUpdater::LoopWorkerThread(std::atomic<bool>* enable_instance_updates_loop)
{
    while (1)
    {
        MapUpdateRequest* request = nullptr;

        _loop_queue.WaitAndPop(request);

        if (_cancelationToken) 
        {
            if(request)
                loopMapFinished();
            return;
        }

        ASSERT(request);
        request->call();

        //repush at end of queue, or delete if loop has been disabled by MapManager
        if(!(*enable_instance_updates_loop))
        {
            delete request;
            loopMapFinished();
        } else {
            _loop_queue.Push(request);
        }
    }
}

void MapUpdater::OnceWorkerThread()
{
    while (1)
    {
        MapUpdateRequest* request = nullptr;

        _once_queue.WaitAndPop(request);

        if (_cancelationToken) 
        {
            if(request)
                onceMapFinished();
            return;
        }

        ASSERT(request);
        request->call();

        delete request;
        onceMapFinished();
    }
}

void MapUpdater::onceMapFinished()
{
    std::lock_guard<std::mutex> lock(_lock);
    ASSERT(pending_once_maps > 0);
    --pending_once_maps;
    if(pending_once_maps == 0)
        _onces_finished_condition.notify_all();
}

void MapUpdater::loopMapFinished()
{
    std::lock_guard<std::mutex> lock(_lock);
    ASSERT(pending_loop_maps > 0);
    --pending_loop_maps;
    if(pending_loop_maps == 0)
        _loops_finished_condition.notify_all();
}
