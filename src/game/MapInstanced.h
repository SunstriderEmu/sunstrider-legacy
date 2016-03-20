/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TRINITY_MAP_INSTANCED_H
#define TRINITY_MAP_INSTANCED_H

#include "Map.h"
#include "InstanceSaveMgr.h"

class MapInstanced : public Map
{
    friend class MapManager;
    public:
        typedef std::unordered_map< uint32, Map* > InstancedMaps;

        MapInstanced(uint32 id, time_t expiry);
        ~MapInstanced() {}

        // functions overwrite Map versions
        void Update(const uint32&);
        void DelayedUpdate(const uint32 diff);
        void MoveAllCreaturesInMoveList();
        void RemoveAllObjectsInRemoveList();
        bool RemoveBones(uint64 guid, float x, float y);
        void UnloadAll();
        bool CanEnter(Player* player);

        Map* GetInstance(const WorldObject* obj);
        Map* FindInstanceMap(uint32 InstanceId);
        //return true of instance was destroyed
        bool DestroyInstance(uint32 InstanceId);
        bool DestroyInstance(InstancedMaps::iterator &itr);

        void AddGridMapReference(const GridPair &p)
        {
            ++GridMapReference[p.x_coord][p.y_coord];
            SetUnloadReferenceLock(GridPair(63-p.x_coord, 63-p.y_coord), true);
        }

        void RemoveGridMapReference(const GridPair &p)
        {
            --GridMapReference[p.x_coord][p.y_coord];
            if (!GridMapReference[p.x_coord][p.y_coord])
                SetUnloadReferenceLock(GridPair(63-p.x_coord, 63-p.y_coord), false);
        }

        InstancedMaps &GetInstancedMaps() { return m_InstancedMaps; }
        virtual void InitVisibilityDistance();

        void MapCrashed(Map* map);
    private:
       
        std::list<Map*> crashedMaps; //those map have crashed, remove them as soon as possible. Contains either BattlegroundMap or InstanceMap


        InstanceMap* CreateInstance(uint32 InstanceId, InstanceSave *save, Difficulty difficulty);
        BattlegroundMap* CreateBattleground(uint32 InstanceId, Battleground* bg);

        InstancedMaps m_InstancedMaps;

        uint16 GridMapReference[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
};
#endif

