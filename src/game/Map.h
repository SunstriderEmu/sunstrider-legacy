/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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

#ifndef TRINITY_MAP_H
#define TRINITY_MAP_H

#include "Define.h"
#include "Policies/ThreadingModel.h"
#include "DBCStructure.h"
#include "GridDefines.h"
#include "Cell.h"
#include "Timer.h"
#include "SharedDefines.h"
#include "GameSystem/GridRefManager.h"
#include "MapRefManager.h"
#include "mersennetwister/MersenneTwister.h"
#include "DynamicTree.h"
#include "Models/GameObjectModel.h"

#include <bitset>
#include <list>
#include <mutex>

class Unit;
class WorldPacket;
class InstanceScript;
class Group;
class InstanceSave;
class WorldObject;
class CreatureGroup;
class Battleground;
class GridMap;
class Transport;
struct Position;
namespace Trinity { struct ObjectUpdater; }

struct ObjectMover
{
    ObjectMover() : x(0), y(0), z(0), ang(0) {}
    ObjectMover(float _x, float _y, float _z, float _ang) : x(_x), y(_y), z(_z), ang(_ang) {}

    float x, y, z, ang;
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct InstanceTemplate
{
    uint32 parent;    
    uint32 maxPlayers;
    uint32 reset_delay;
    uint32 access_id;
    float startLocX;
    float startLocY;
    float startLocZ;
    float startLocO;
    uint32 script_id;
};

struct InstanceTemplateAddon
{
    uint32 map;
    bool forceHeroicEnabled; //true to enable this entry
};

enum LevelRequirementVsMode
{
    LEVELREQUIREMENT_HEROIC = 70
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

typedef std::unordered_map<Creature*, ObjectMover> CreatureMoveList;
typedef std::unordered_map<GameObject*, ObjectMover> GameObjectMoveList;

typedef std::map<uint32/*leaderDBGUID*/, CreatureGroup*>        CreatureGroupHolderType;

class Map : public GridRefManager<NGridType>
{
    friend class MapReference;
    public:
        Map(uint32 id, time_t, uint32 InstanceId, uint8 SpawnMode);
        virtual ~Map();

        // currently unused for normal maps
        bool CanUnload(uint32 diff)
        {
            if(!m_unloadTimer) return false;
            if(m_unloadTimer <= diff) return true;
            m_unloadTimer -= diff;
            return false;
        }

        virtual bool Add(Player *);
        virtual void Remove(Player *, bool);
        virtual bool Add(Transport *);
        virtual void Remove(Transport *, bool);
        template<class T> void Add(T *);
        template<class T> void Remove(T *, bool);

        void VisitNearbyCellsOf(WorldObject* obj, TypeContainerVisitor<Trinity::ObjectUpdater, GridTypeMapContainer> &gridVisitor, TypeContainerVisitor<Trinity::ObjectUpdater, WorldTypeMapContainer> &worldVisitor);
        virtual void Update(const uint32&);

        void MessageBroadcast(Player*, WorldPacket *, bool to_self, bool to_possessor);
        void MessageBroadcast(WorldObject *, WorldPacket *, bool to_possessor);
        void MessageDistBroadcast(Player *, WorldPacket *, float dist, bool to_self, bool to_possessor, bool own_team_only = false);
        void MessageDistBroadcast(WorldObject *, WorldPacket *, float dist, bool to_possessor);

        float GetVisibilityDistance() const { return m_VisibleDistance; }
        //function for setting up visibility distance for maps on per-type/per-Id basis
        virtual void InitVisibilityDistance();

        void PlayerRelocation(Player* player, float x, float y, float z, float angle);
        void CreatureRelocation(Creature* creature, float x, float y, float z, float angle);
        void GameObjectRelocation(GameObject* gob, float x, float y, float z, float angle);
        void DynamicObjectRelocation(DynamicObject* dob, float x, float y, float z, float angle);

        template<class T, class CONTAINER> void Visit(const Cell &cell, TypeContainerVisitor<T, CONTAINER> &visitor);

        bool IsRemovalGrid(float x, float y) const
        {
            GridPair p = Trinity::ComputeGridPair(x, y);
            return( !getNGrid(p.x_coord, p.y_coord) || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL );
        }

        bool GetUnloadLock(const GridPair &p) const { return getNGrid(p.x_coord, p.y_coord)->getUnloadLock(); }
        void SetUnloadLock(const GridPair &p, bool on) { getNGrid(p.x_coord, p.y_coord)->setUnloadExplicitLock(on); }
        void LoadGrid(float x, float y);
        bool UnloadGrid(const uint32 &x, const uint32 &y, bool pForce);
        virtual void UnloadAll();

        bool IsGridLoadedAt(float x, float y) const;

        void ResetGridExpiry(NGridType &grid, float factor = 1) const
        {
            grid.ResetTimeTracker((time_t)((float)i_gridExpiry*factor));
        }

        time_t GetGridExpiry(void) const { return i_gridExpiry; }
        uint32 GetId(void) const { return i_id; }

        void LoadMapAndVMap(uint32 mapid, uint32 instanceid, int x, int y);

        static void InitStateMachine();
        static void DeleteStateMachine();

        // some calls like isInWater should not use vmaps due to processor power
        // can return INVALID_HEIGHT if under z+2 z coord not found height
        float _GetHeight(float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const;
        /* Returns closest height for given position, search in map height.
        @checkVMap search in vmap height as well. If both map and vmap heights were found, the closest one will be returned
        Returns INVALID_HEIGHT if no height found at position or if height is further than maxSearchDist
        */
        float GetHeight(float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const;
        /* Get map level (checking vmaps) or liquid level at given point */
        float GetWaterOrGroundLevel(float x, float y, float z, float* ground = NULL, bool swim = false) const;
        //Returns INVALID_HEIGHT if nothing found
        float GetHeight(PhaseMask phasemask, float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const;
        void RemoveGameObjectModel(const GameObjectModel& model) { _dynamicTree.remove(model); }
        void InsertGameObjectModel(const GameObjectModel& model) { _dynamicTree.insert(model); }
        bool ContainsGameObjectModel(const GameObjectModel& model) const { return _dynamicTree.contains(model);}

        bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, PhaseMask phasemask = (PhaseMask)0) const;
        void Balance() { _dynamicTree.balance(); }
        //get dynamic collision (gameobjects only ?)
        bool getObjectHitPos(PhaseMask phasemask, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float &ry, float& rz, float modifyDist);

        ZLiquidStatus getLiquidStatus(float x, float y, float z, BaseLiquidTypeMask reqBaseLiquidTypeMask, LiquidData *data = 0) const;

        uint16 GetAreaFlag(float x, float y, float z, bool *isOutdoors=0) const;
        bool GetAreaInfo(float x, float y, float z, uint32 &mogpflags, int32 &adtId, int32 &rootId, int32 &groupId) const;

        bool IsOutdoors(float x, float y, float z) const;

        uint8 GetTerrainType(float x, float y) const;
        float GetWaterLevel(float x, float y) const;
        bool IsInWater(float x, float y, float z, LiquidData *data = 0) const;
        bool IsUnderWater(float x, float y, float z) const;

        static uint32 GetAreaId(uint16 areaflag,uint32 map_id);
        static uint32 GetZoneId(uint16 areaflag,uint32 map_id);

        uint32 GetAreaId(float x, float y, float z) const
        {
            return GetAreaId(GetAreaFlag(x,y,z),i_id);
        }

        uint32 GetZoneId(float x, float y, float z) const
        {
            return GetZoneId(GetAreaFlag(x,y,z),i_id);
        }

        virtual void MoveAllCreaturesInMoveList();
        virtual void MoveAllGameObjectsInMoveList();
        virtual void RemoveAllObjectsInRemoveList();
        virtual void RemoveAllPlayers();

        bool CreatureRespawnRelocation(Creature *c, bool diffGridOnly);        // used only in MoveAllCreaturesInMoveList and ObjectGridUnloader
        bool GameObjectRespawnRelocation(GameObject* go, bool diffGridOnly);

        // assert print helper
        bool CheckGridIntegrity(Creature* c, bool moved) const;
        bool CheckGridIntegrity(GameObject* c, bool moved) const;

        uint32 GetInstanceId() const { return i_InstanceId; }
        uint8 GetSpawnMode() const { return (i_spawnMode); }
        virtual bool CanEnter(Player* /*player*/) { return true; }
        const char* GetMapName() const;

        bool Instanceable() const { return i_mapEntry && i_mapEntry->Instanceable(); }
        // NOTE: this duplicate of Instanceable(), but Instanceable() can be changed when BG also will be instanceable
        bool IsDungeon() const { return i_mapEntry && i_mapEntry->IsDungeon(); }
        bool IsRaid() const { return i_mapEntry && i_mapEntry->IsRaid(); }
        bool IsCommon() const { return i_mapEntry && i_mapEntry->IsCommon(); }
        bool IsHeroic() const { return i_spawnMode == DIFFICULTY_HEROIC; }
        bool IsBattleground() const { return i_mapEntry && i_mapEntry->IsBattleground(); }
        bool IsBattleArena() const { return i_mapEntry && i_mapEntry->IsBattleArena(); }
        bool IsBattlegroundOrArena() const { return i_mapEntry && i_mapEntry->IsBattlegroundOrArena(); }
   
        void AddObjectToRemoveList(WorldObject *obj);
        void AddObjectToSwitchList(WorldObject *obj, bool on);

        virtual bool RemoveBones(uint64 guid, float x, float y);

        void UpdateObjectVisibility(WorldObject* obj, Cell cell, CellCoord cellpair);

        void resetMarkedCells() { marked_cells.reset(); }
        bool isCellMarked(uint32 pCellId) { return marked_cells.test(pCellId); }
        void markCell(uint32 pCellId) { marked_cells.set(pCellId); }
        Player* GetPlayer(uint64 guid);
        Creature* GetCreature(uint64 guid);
        GameObject* GetGameObject(uint64 guid);
        Transport* GetTransport(uint64 guid);
        DynamicObject* GetDynamicObject(uint64 guid);   

        bool HavePlayers() const { return !m_mapRefManager.isEmpty(); }
        uint32 GetPlayersCountExceptGMs() const;
        bool ActiveObjectsNearGrid(uint32 x, uint32 y) const;

        void AddUnitToNotify(Unit* unit);
        void RelocationNotify();

        void SendToPlayers(WorldPacket* data) const;

        typedef MapRefManager PlayerList;
        PlayerList const& GetPlayers() const { return m_mapRefManager; }

        // must called with AddToWorld
        template<class T>
        void AddToForceActive(T* obj) { AddToForceActiveHelper(obj); }

        void AddToForceActive(Creature* obj);

        // must called with RemoveFromWorld
        template<class T>
        void RemoveFromForceActive(T* obj) { RemoveFromForceActiveHelper(obj); }

        void RemoveFromForceActive(Creature* obj);

        template<class T> void SwitchGridContainers(T* obj, bool active);
        template<class NOTIFIER> void VisitAll(const float &x, const float &y, float radius, NOTIFIER &notifier);
        template<class NOTIFIER> void VisitWorld(const float &x, const float &y, float radius, NOTIFIER &notifier);
        template<class NOTIFIER> void VisitGrid(const float &x, const float &y, float radius, NOTIFIER &notifier);
        CreatureGroupHolderType CreatureGroupHolder;
        MTRand mtRand;

        int32 irand(int32 min, int32 max)
        {
          return int32 (mtRand.randInt(max - min)) + min;
        }

        uint32 urand(uint32 min, uint32 max)
        {
          return mtRand.randInt(max - min) + min;
        }

        int32 rand32()
        {
          return mtRand.randInt();
        }

        double rand_norm(void)
        {
          return mtRand.randExc();
        }

        double rand_chance(void)
        {
          return mtRand.randExc(100.0);
        }
        
        void AddCreatureToPool(Creature*, uint32);
        void RemoveCreatureFromPool(Creature*, uint32);
        std::list<Creature*> GetAllCreaturesFromPool(uint32);

        static bool SupportsHeroicMode(const MapEntry* mapEntry);

        // Objects that must update even in inactive grids without activating them
        typedef std::set<Transport*> TransportsContainer;
        TransportsContainer _transports;
        TransportsContainer::iterator _transportsUpdateIter;

    private:

        void LoadVMap(int pX, int pY);
        void LoadMap(uint32 mapid, uint32 instanceid, int x,int y);

        GridMap *GetGrid(float x, float y);

        void SetTimer(uint32 t) { i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t; }
        //uint64 CalculateGridMask(const uint32 &y) const;

        void SendInitSelf( Player * player );

        void SendInitTransports( Player * player );
        void SendRemoveTransports( Player * player );

        bool CreatureCellRelocation(Creature* creature, Cell new_cell);
        bool GameObjectCellRelocation(GameObject* gob, Cell new_cell);

        void AddCreatureToMoveList(Creature* c, float x, float y, float z, float ang);
        void AddGameObjectToMoveList(GameObject* go, float x, float y, float z, float ang);
        CreatureMoveList i_creaturesToMove;
        GameObjectMoveList i_gameObjectsToMove;

        bool loaded(const GridPair &) const;
        void EnsureGridLoaded(const Cell&, Player* player = NULL);
        void EnsureGridCreated(const GridPair &);
        void EnsureGridCreated_i(const GridPair &);

        void buildNGridLinkage(NGridType* pNGridType) { pNGridType->link(this); }

        template<class T> void AddType(T *obj);
        template<class T> void RemoveType(T *obj, bool);

        NGridType* getNGrid(uint32 x, uint32 y) const
        {
            assert(x < MAX_NUMBER_OF_GRIDS);
            assert(y < MAX_NUMBER_OF_GRIDS);
            return i_grids[x][y];
        }

        bool isGridObjectDataLoaded(uint32 x, uint32 y) const { return getNGrid(x,y)->isGridObjectDataLoaded(); }
        void setGridObjectDataLoaded(bool pLoaded, uint32 x, uint32 y) { getNGrid(x,y)->setGridObjectDataLoaded(pLoaded); }

        void setNGrid(NGridType* grid, uint32 x, uint32 y);

        void UpdateActiveCells(const float &x, const float &y, const uint32 &t_diff);
    protected:
        void SetUnloadReferenceLock(const GridPair &p, bool on) { getNGrid(p.x_coord, p.y_coord)->setUnloadReferenceLock(on); }

        std::mutex _mapLock;
        std::mutex _gridLock;

        MapEntry const* i_mapEntry;
        uint8 i_spawnMode;
        uint32 i_id;
        uint32 i_InstanceId;
        uint32 m_unloadTimer;
        float m_VisibleDistance;
        DynamicMapTree _dynamicTree;

        MapRefManager m_mapRefManager;
        MapRefManager::iterator m_mapRefIter;

        /** The objects in m_activeForcedNonPlayers are always kept active and makes everything around them also active, just like players
        */
        typedef std::set<WorldObject*> ActiveForcedNonPlayers;
        ActiveForcedNonPlayers m_activeForcedNonPlayers;
        ActiveForcedNonPlayers::iterator m_activeForcedNonPlayersIter;

    private:
        NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        GridMap *GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cells;

        time_t i_gridExpiry;

        bool i_lock;
        std::vector<uint64> i_unitsToNotifyBacklog;
        std::vector<Unit*> i_unitsToNotify;
        std::set<WorldObject *> i_objectsToRemove;
        std::map<WorldObject*, bool> i_objectsToSwitch;

        // Type specific code for add/remove to/from grid
        template<class T>
            void AddToGrid(T*, NGridType *, Cell const&);

        template<class T>
            void AddNotifier(T*);

        template<class T>
            void RemoveFromGrid(T*, NGridType *, Cell const&);

        template<class T>
            void DeleteFromWorld(T*);

        template<class T>
        void AddToForceActiveHelper(T* obj)
        {
            m_activeForcedNonPlayers.insert(obj);
        }

        template<class T>
        void RemoveFromForceActiveHelper(T* obj)
        {
            // Map::Update for active object in proccess
            if(m_activeForcedNonPlayersIter != m_activeForcedNonPlayers.end())
            {
                ActiveForcedNonPlayers::iterator itr = m_activeForcedNonPlayers.find(obj);
                if(itr == m_activeForcedNonPlayers.end())
                    return;
                if(itr==m_activeForcedNonPlayersIter)
                    ++m_activeForcedNonPlayersIter;
                m_activeForcedNonPlayers.erase(itr);
            }
            else
                m_activeForcedNonPlayers.erase(obj);
        }

        typedef std::map<uint32, std::set<uint64> > CreaturePoolMember;
        CreaturePoolMember m_cpmembers;
};

enum InstanceResetMethod
{
    INSTANCE_RESET_ALL,
    INSTANCE_RESET_CHANGE_DIFFICULTY,
    INSTANCE_RESET_GLOBAL,
    INSTANCE_RESET_GROUP_DISBAND,
    INSTANCE_RESET_GROUP_JOIN,
    INSTANCE_RESET_RESPAWN_DELAY
};

class InstanceMap : public Map
{
    public:
        InstanceMap(uint32 id, time_t, uint32 InstanceId, uint8 SpawnMode);
        ~InstanceMap();
        bool Add(Player *);
        void Remove(Player *, bool);
        void Update(const uint32&);
        void CreateInstanceData(bool load);
        bool Reset(uint8 method);
        uint32 GetScriptId() { return i_script_id; }
        InstanceScript* GetInstanceScript() { return i_data; }
        void PermBindAllPlayers(Player *player);
        time_t GetResetTime();
        void UnloadAll();
        bool CanEnter(Player* player);
        void SendResetWarnings(uint32 timeLeft) const;
        void SetResetSchedule(bool on);

        virtual void InitVisibilityDistance();
    private:
        bool m_resetAfterUnload;
        bool m_unloadWhenEmpty;
        InstanceScript* i_data;
        uint32 i_script_id;
};

class BattlegroundMap : public Map
{
    public:
        BattlegroundMap(uint32 id, time_t, uint32 InstanceId);
        ~BattlegroundMap();

        bool Add(Player *) override;
        void Remove(Player *, bool) override;
        bool CanEnter(Player* player) override;
        void SetUnload();
        //void UnloadAll();
        void RemoveAllPlayers() override;

        virtual void InitVisibilityDistance() override;
        Battleground* GetBG() { return m_bg; }
        void SetBG(Battleground* bg) { m_bg = bg; }
    private:
        Battleground* m_bg;
};

/*inline
uint64
Map::CalculateGridMask(const uint32 &y) const
{
    uint64 mask = 1;
    mask <<= y;
    return mask;
}
*/

template<class T, class CONTAINER>
inline void
Map::Visit(const Cell &cell, TypeContainerVisitor<T, CONTAINER> &visitor)
{
    const uint32 x = cell.GridX();
    const uint32 y = cell.GridY();
    const uint32 cell_x = cell.CellX();
    const uint32 cell_y = cell.CellY();

    if( !cell.NoCreate() || loaded(GridPair(x,y)) )
    {
        EnsureGridLoaded(cell);
        //LOCK_TYPE guard(i_info[x][y]->i_lock);
        getNGrid(x, y)->Visit(cell_x, cell_y, visitor);
    }
}

template<class NOTIFIER>
inline void
Map::VisitAll(const float &x, const float &y, float radius, NOTIFIER &notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    TypeContainerVisitor<NOTIFIER, WorldTypeMapContainer> world_object_notifier(notifier);
    cell.Visit(p, world_object_notifier, *this, radius, x, y);
    TypeContainerVisitor<NOTIFIER, GridTypeMapContainer >  grid_object_notifier(notifier);
    cell.Visit(p, grid_object_notifier, *this, radius, x, y);
}

template<class NOTIFIER>
inline void
Map::VisitWorld(const float &x, const float &y, float radius, NOTIFIER &notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    TypeContainerVisitor<NOTIFIER, WorldTypeMapContainer> world_object_notifier(notifier);
    cell.Visit(p, world_object_notifier, *this, radius, x, y);
}

template<class NOTIFIER>
inline void
Map::VisitGrid(const float &x, const float &y, float radius, NOTIFIER &notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    TypeContainerVisitor<NOTIFIER, GridTypeMapContainer >  grid_object_notifier(notifier);
    cell.Visit(p, grid_object_notifier, *this, radius, x, y);
}
#endif

