
#ifndef TRINITY_MAPMANAGER_H
#define TRINITY_MAPMANAGER_H

#include "Define.h"
#include "Map.h"
#include "MapUpdater.h"
#include "MapInstanced.h"
#include "GridStates.h"

class WorldLocation;

class TC_GAME_API MapManager
{
    typedef std::unordered_map<uint32, Map*> MapMapType;
    typedef std::pair<std::unordered_map<uint32, Map*>::iterator, bool>  MapMapPair;

    public:
        static MapManager* instance()
        {
            static MapManager instance;
            return &instance;
        }

        //Find base map, create it if needed
        Map* CreateBaseMap(uint32 id, bool testing = false);
        //Return base map if it exists, null otherwise
        Map* FindBaseMap(uint32 id, bool testing = false) const;
        //Same as FindBaseMap, but exclude all instanceable maps
        Map* FindBaseNonInstanceMap(uint32 mapId) const;
        Map* CreateMap(uint32 id, Player* player, uint32 loginInstanceId = 0);
        Map* FindMap(uint32 mapid, uint32 instanceId);
        //enableMapObjects = spawn npcs and gobjects on the map
        std::pair<TestMap*, uint32 /*testMapInstanceId*/> CreateTestMap(uint32 mapid, Difficulty diff = REGULAR_DIFFICULTY, bool enableMapObjects = false);

        uint32 GetAreaId(uint32 mapid, float x, float y, float z) const;
        uint32 GetZoneId(uint32 mapid, float x, float y, float z) const;
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid, uint32 mapid, float x, float y, float z);

        void Initialize(void);
        void Update(time_t);

		void SetGridCleanUpDelay(uint32 t)
		{
			if (t < MIN_GRID_DELAY)
				i_gridCleanUpDelay = MIN_GRID_DELAY;
			else
				i_gridCleanUpDelay = t;
		}

        inline void SetMapUpdateInterval(uint32 t)
        {
            if( t > MIN_MAP_UPDATE_DELAY )
                t = MIN_MAP_UPDATE_DELAY;

            i_timer.SetInterval(t);
            i_timer.Reset();
        }

        //void LoadGrid(int mapid, float x, float y, const WorldObject* obj, bool no_unload = false);
        void UnloadAll();
        bool UnloadTestMap(uint32 mapId, uint32 instanceId);

        static bool ExistMapAndVMap(uint32 mapid, float x, float y);
        static bool IsValidMAP(uint32 mapid, bool startUp = false);

        static bool IsValidMapCoord(uint32 mapid, float x,float y)
        {
            return IsValidMAP(mapid) && Trinity::IsValidMapCoord(x,y);
        }

        static bool IsValidMapCoord(uint32 mapid, float x,float y,float z)
        {
            return IsValidMAP(mapid) && Trinity::IsValidMapCoord(x,y,z);
        }

        static bool IsValidMapCoord(uint32 mapid, float x,float y,float z,float o)
        {
            return IsValidMAP(mapid) && Trinity::IsValidMapCoord(x,y,z,o);
        }

        static bool IsValidMapCoord(WorldLocation const& loc);

        Map::EnterState PlayerCannotEnter(uint32 mapid, Player* player, bool loginCheck = false);
        void InitializeVisibilityDistanceInfo();

        /* statistics */
        uint32 GetNumInstances();
        uint32 GetNumPlayersInInstances();
        uint32 GetNumPlayersInMap(uint32 mapId);

        // Instance ID management
        void InitInstanceIds();
        uint32 GenerateInstanceId();
        void RegisterInstanceId(uint32 instanceId);
        void FreeInstanceId(uint32 instanceId);

        uint32 GetNextInstanceId() const { return _nextInstanceId; };
        void SetNextInstanceId(uint32 nextInstanceId) { _nextInstanceId = nextInstanceId; };

        MapUpdater * GetMapUpdater() { return &m_updater; }

        void MapCrashed(Map& map);

        template<typename Worker>
        void DoForAllMaps(Worker&& worker);

        template<typename Worker>
        void DoForAllMapsWithMapId(uint32 mapId, Worker&& worker);

		void IncreaseScheduledScriptsCount() { ++_scheduledScripts; }
		void DecreaseScheduledScriptCount() { --_scheduledScripts; }
		void DecreaseScheduledScriptCount(std::size_t count) { _scheduledScripts -= count; }
		bool IsScriptScheduled() const { return _scheduledScripts > 0; }

    private:
        typedef std::vector<bool> InstanceIds;

        MapManager();
        ~MapManager();

        MapManager(const MapManager &);
        MapManager& operator=(const MapManager &) = delete;
        
        MapMapType i_maps;
        IntervalTimer i_timer;
        std::mutex _mapsLock;
		uint32 i_gridCleanUpDelay;

        InstanceIds _instanceIds;
        uint32 _nextInstanceId;
        MapUpdater m_updater;

		// atomic op counter for active scripts amount
		std::atomic<std::size_t> _scheduledScripts;
};

template<typename Worker>
void MapManager::DoForAllMaps(Worker&& worker)
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    for (auto& mapPair : i_maps)
    {
        Map* map = mapPair.second;
        if (MapInstanced* mapInstanced = map->ToMapInstanced())
        {
            MapInstanced::InstancedMaps& instances = mapInstanced->GetInstancedMaps();
            for (auto& instancePair : instances)
                worker(instancePair.second);
        }
        else
            worker(map);
    }
}

template<typename Worker>
inline void MapManager::DoForAllMapsWithMapId(uint32 mapId, Worker&& worker)
{
    std::lock_guard<std::mutex> lock(_mapsLock);

    auto itr = i_maps.find(mapId);
    if (itr != i_maps.end())
    {
        Map* map = itr->second;
        if (MapInstanced* mapInstanced = map->ToMapInstanced())
        {
            MapInstanced::InstancedMaps& instances = mapInstanced->GetInstancedMaps();
            for (auto& p : instances)
                worker(p.second);
        }
        else
            worker(map);
    }
}

#define sMapMgr MapManager::instance()
#endif

