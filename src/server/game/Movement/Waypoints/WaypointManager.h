
#ifndef TRINITY_WAYPOINTMANAGER_H
#define TRINITY_WAYPOINTMANAGER_H

#include <unordered_map>
#include "WaypointDefines.h"

typedef std::unordered_map<uint32, WaypointPath> WaypointPathContainer;

class TC_GAME_API WaypointMgr
{
    public:
        static WaypointMgr* instance()
        {
            static WaypointMgr instance;
            return &instance;
        }

        // Attempts to reload a single path from database
        void ReloadPath(uint32 id);

        // Loads all paths from database, do not reload since the pointers in _waypointStore could still be in use
        void Load();

        // Returns the path from a given id
        WaypointPath const* GetPath(uint32 id) const
        {
            auto itr = _waypointStore.find(id);
            if (itr != _waypointStore.end())
                return &itr->second;

            return nullptr;
        }

    private:
        // Only allow instantiation from ACE_Singleton
        WaypointMgr();
        ~WaypointMgr();

        WaypointPathContainer _waypointStore;
};

#define sWaypointMgr WaypointMgr::instance()

#endif
