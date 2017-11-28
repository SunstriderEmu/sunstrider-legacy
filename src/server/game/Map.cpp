
#include "MapManager.h"
#include "Player.h"
#include "GridNotifiers.h"
#include "WorldSession.h"
#include "Log.h"
#include "CellImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "GridNotifiersImpl.h"
#include "Transport.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Group.h"
#include "MapRefManager.h"
#include "ScriptMgr.h"
#include "Util.h"
#include "Chat.h"
#include "Language.h"
#include "Weather.h"
#include "MapInstanced.h"
#include "InstanceSaveMgr.h"
#include "Management/VMapFactory.h"
#include "Management/MMapManager.h"
#include "Management/MMapFactory.h"
#include "Management/IVMapManager.h"
#include "DynamicTree.h"
#include "BattleGround.h"
#include "GridMap.h"
#include "ObjectGridLoader.h"
#include "Pet.h"
#include "GridStates.h"
#include "Totem.h"
#include "Transport.h"
#include "ScriptMgr.h"
#ifdef TESTS
#include "TestCase.h"
#endif

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50
#define MAX_CREATURE_ATTACK_RADIUS  (45.0f * sWorld->GetRate(RATE_CREATURE_AGGRO))

extern u_map_magic MapMagic;
extern u_map_magic MapVersionMagic;
extern u_map_magic MapAreaMagic;
extern u_map_magic MapHeightMagic;
extern u_map_magic MapLiquidMagic; 

ZoneDynamicInfo::ZoneDynamicInfo() : 
    MusicId(0), 
    WeatherId(WEATHER_STATE_FINE), 
    WeatherGrade(0.0f),
    OverrideLightId(0), 
    LightFadeInTime(0) 
{ }

GridState* si_GridStates[MAX_GRID_STATE];

Map::~Map()
{
    sScriptMgr->OnDestroyMap(this);

    UnloadAll();

    if (!m_scriptSchedule.empty())
        sMapMgr->DecreaseScheduledScriptCount(m_scriptSchedule.size());

    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(GetId(), i_InstanceId);
}

void Map::LoadVMap(int x,int y)
{
    if (!VMAP::VMapFactory::createOrGetVMapManager()->isMapLoadingEnabled())
        return;
                                                            // x and y are swapped !!
    int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld->GetDataPath()+ "vmaps").c_str(),  GetId(), x,y);
    switch(vmapLoadResult)
    {
        case VMAP::VMAP_LOAD_RESULT_OK:
            TC_LOG_DEBUG("maps","VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), x,y, x,y);
            break;
        case VMAP::VMAP_LOAD_RESULT_ERROR:
            TC_LOG_DEBUG("maps","Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), x,y, x,y);
            break;
        case VMAP::VMAP_LOAD_RESULT_IGNORED:
            TC_LOG_DEBUG("maps","Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", GetMapName(), GetId(), x,y, x,y);
            break;
    }
}

void Map::LoadMap(uint32 mapid, uint32 instanceid, int x,int y)
{
    if( instanceid != 0)
    {
        if(GridMaps[x][y])
            return;

        // load gridmap for base map
        if (!m_parentMap->GridMaps[x][y])
            m_parentMap->EnsureGridCreated(GridCoord(63-x,63-y));

//+++        if (!baseMap->GridMaps[x][y])  don't check for GridMaps[gx][gy], we need the management for vmaps
//            return;

        ASSERT(m_parentMap != this);
        ((MapInstanced*)(m_parentMap))->AddGridMapReference(GridCoord(x,y));
        GridMaps[x][y] = m_parentMap->GridMaps[x][y];
        return;
    }

    //map already load, delete it before reloading (Is it necessary? Do we really need the ability the reload maps during runtime?)
    if(GridMaps[x][y])
    {
        sScriptMgr->OnUnloadGridMap(this, GridMaps[x][y], x, y);

        TC_LOG_DEBUG("maps","Unloading already loaded map %u before reloading.",mapid);
        delete (GridMaps[x][y]);
        GridMaps[x][y]=nullptr;
    }

    // map file name
    char *tmp=nullptr;
    // Pihhan: dataPath length + "maps/" + 3+2+2+ ".map" length may be > 32 !
    int len = sWorld->GetDataPath().length()+strlen("maps/%03u%02u%02u.map")+1;
    tmp = new char[len];
    snprintf(tmp, len, (char *)(sWorld->GetDataPath()+"maps/%03u%02u%02u.map").c_str(),mapid,x,y);
    TC_LOG_DEBUG("maps","Loading map %s",tmp);
    // loading data
    GridMaps[x][y] = new GridMap();
    if (!GridMaps[x][y]->loadData(tmp))
    {
        TC_LOG_ERROR("maps","ERROR loading map file: \n %s\n", tmp);
    }
    delete [] tmp;

    sScriptMgr->OnLoadGridMap(this, GridMaps[x][y], x, y);
}

void Map::LoadMapAndVMap(uint32 mapid, uint32 instanceid, int x,int y)
{
    LoadMap(mapid,instanceid,x,y);
    if(VMAP::VMapFactory::createOrGetVMapManager()->isMapLoadingEnabled() && instanceid == 0) {
        LoadVMap(x, y);                                     // Only load the data for the base map
        // load navmesh
        MMAP::MMapFactory::createOrGetMMapManager()->loadMap((sWorld->GetDataPath() + "mmaps").c_str(),mapid, x, y);
    }
}


void Map::InitStateMachine()
{
    si_GridStates[GRID_STATE_INVALID] = new InvalidState;
    si_GridStates[GRID_STATE_ACTIVE] = new ActiveState;
    si_GridStates[GRID_STATE_IDLE] = new IdleState;
    //si_GridStates[GRID_STATE_REMOVAL] = new RemovalState;
}

void Map::DeleteStateMachine()
{
    delete si_GridStates[GRID_STATE_INVALID];
    delete si_GridStates[GRID_STATE_ACTIVE];
    delete si_GridStates[GRID_STATE_IDLE];
    //delete si_GridStates[GRID_STATE_REMOVAL];
}

Map::Map(MapType type, uint32 id, time_t expiry, uint32 InstanceId, uint8 SpawnMode, Map* _parent)
   : i_mapEntry(sMapStore.LookupEntry(id)), i_spawnMode(SpawnMode),
   _creatureToMoveLock(false), _gameObjectsToMoveLock(false), _dynamicObjectsToMoveLock(false),
   i_id(id), i_InstanceId(InstanceId), m_unloadTimer(0), _lastMapUpdate(0),
   m_VisibleDistance(DEFAULT_VISIBILITY_DISTANCE), m_VisibilityNotifyPeriod(DEFAULT_VISIBILITY_NOTIFY_PERIOD),
   m_activeForcedNonPlayersIter(m_activeForcedNonPlayers.end()), 
   _transportsUpdateIter(_transports.end()),
   _defaultLight(GetDefaultMapLight(id)),
   i_mapType(type), i_gridExpiry(expiry),
   i_scriptLock(false), m_disableMapObjects(false)
{
    m_parentMap = (_parent ? _parent : this);
    for(uint32 idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
    {
        for(uint32 j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
        {
            //z code
            GridMaps[idx][j] =nullptr;
            setNGrid(nullptr, idx, j);
        }
    }

    Map::InitVisibilityDistance();

    sScriptMgr->OnCreateMap(this);
}

float Map::GetDefaultVisibilityDistance() const
{
    return World::GetMaxVisibleDistanceOnContinents();
}

float Map::GetVisibilityNotifierPeriod() const
{
    return World::GetVisibilityNotifyPeriodOnContinents();
}

void Map::InitVisibilityDistance()
{
    //init visibility for continents
    m_VisibleDistance = GetDefaultVisibilityDistance();
    m_VisibilityNotifyPeriod = GetVisibilityNotifierPeriod();
}

void Map::SetVisibilityDistance(float dist)
{
    ASSERT(dist != 0);
    m_VisibleDistance = dist;
}

// Template specialization of utility methods
template<class T>
void Map::AddToGrid(T* obj, Cell const& cell)
{
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    if (obj->IsWorldObject())
        grid->GetGridType(cell.CellX(), cell.CellY()).template AddWorldObject<T>(obj);
    else
        grid->GetGridType(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj);
}

template<>
void Map::AddToGrid(Corpse *obj, Cell const& cell)
{
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());

    // Corpses are a special object type - they can be added to grid via a call to AddToMap
    // or loaded through ObjectGridLoader.
    // Both corpses loaded from database and these freshly generated by Player::CreateCoprse are added to _corpsesByCell
    // ObjectGridLoader loads all corpses from _corpsesByCell even if they were already added to grid before it was loaded
    // so we need to explicitly check it here (Map::AddToGrid is only called from Player::BuildPlayerRepop, not from ObjectGridLoader)
    // to avoid failing an assertion in GridObject::AddToGrid

    // add to world object registry in grid
    if (grid->isGridObjectDataLoaded())
    {
        if (obj->IsWorldObject())
            grid->GetGridType(cell.CellX(), cell.CellY()).AddWorldObject(obj);
        else
            grid->GetGridType(cell.CellX(), cell.CellY()).AddGridObject(obj);
    }
}

template<>
void Map::AddToGrid(Creature* obj, Cell const& cell)
{
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    if (obj->IsWorldObject())
        grid->GetGridType(cell.CellX(), cell.CellY()).AddWorldObject(obj);
    else
        grid->GetGridType(cell.CellX(), cell.CellY()).AddGridObject(obj);

    obj->SetCurrentCell(cell);
}

template<>
void Map::AddToGrid(DynamicObject* obj, Cell const& cell)
{
    NGridType* grid = getNGrid(cell.GridX(), cell.GridY());
    grid->GetGridType(cell.CellX(), cell.CellY()).AddGridObject(obj);

    obj->SetCurrentCell(cell);
}

template<class T>
void Map::SwitchGridContainers(T* /*obj*/, bool /*on*/) { }

template<>
void Map::SwitchGridContainers(Creature* obj, bool on)
{
    ASSERT(!obj->IsPermanentWorldObject());
    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    if (!p.IsCoordValid())
    {
        TC_LOG_ERROR("maps", "Map::SwitchGridContainers: Object " UI64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if( !IsGridLoaded(GridCoord(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    /*
    if (sLog->ShouldLog("maps", LOG_LEVEL_DEBUG))
    {
    // Extract bitfield values
    uint32 const grid_x = cell.data.Part.grid_x;
    uint32 const grid_y = cell.data.Part.grid_y;

    TC_LOG_DEBUG("maps", "Switch object %s from grid[%u, %u] %u", obj->GetGUID().ToString().c_str(), grid_x, grid_y, on);
    }
    */

    NGridType *ngrid = getNGrid(cell.GridX(), cell.GridY());
    assert( ngrid != nullptr);

    GridType &grid = ngrid->GetGridType(cell.CellX(), cell.CellY());

    obj->RemoveFromGrid(); //This step is not really necessary but we want to do ASSERT in remove/add

    if(on)
    {
        grid.AddWorldObject(obj);
        AddWorldObject(obj);
    }
    else
    {
        grid.AddGridObject(obj);
        RemoveWorldObject(obj);
    }

    obj->m_isTempWorldObject = on;
}

template<>
void Map::SwitchGridContainers(GameObject* obj, bool on)
{
    ASSERT(!obj->IsPermanentWorldObject());
    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    if (!p.IsCoordValid())
    {
        TC_LOG_ERROR("maps", "Map::SwitchGridContainers: Object %s has invalid coordinates X:%f Y:%f grid cell [%u:%u]", ObjectGuid(obj->GetGUID()).ToString().c_str(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (!IsGridLoaded(GridCoord(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    /*
    if (sLog->ShouldLog("maps", LOG_LEVEL_DEBUG))
    {
        // Extract bitfield values
        uint32 const grid_x = cell.data.Part.grid_x;
        uint32 const grid_y = cell.data.Part.grid_y;

        TC_LOG_DEBUG("maps", "Switch object %s from grid[%u, %u] %u", obj->GetGUID().ToString().c_str(), grid_x, grid_y, on);
    }
    */

    NGridType *ngrid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(ngrid != NULL);

    GridType &grid = ngrid->GetGridType(cell.CellX(), cell.CellY());

    obj->RemoveFromGrid(); //This step is not really necessary but we want to do ASSERT in remove/add

    if (on)
    {
        grid.AddWorldObject(obj);
        AddWorldObject(obj);
    }
    else
    {
        grid.AddGridObject(obj);
        RemoveWorldObject(obj);
    }
}

template<class T>
void Map::DeleteFromWorld(T* obj)
{
    // Note: In case resurrectable corpse and pet its removed from global lists in own destructor
    delete obj;
}

template<>
void Map::DeleteFromWorld(Player* player)
{
    ObjectAccessor::RemoveObject(player);
    RemoveUpdateObject(player); /// @todo I do not know why we need this, it should be removed in ~Object anyway
    delete player;
}

template<>
void Map::DeleteFromWorld(MotionTransport* transport)
{
    ObjectAccessor::RemoveObject(transport);
    delete transport;
}

void Map::EnsureGridCreated(const GridCoord &p)
{
    std::lock_guard<std::mutex> lock(_gridLock);
    EnsureGridCreated_i(p);
}

//Create NGrid so the object can be added to it
//But object data is not loaded here
void Map::EnsureGridCreated_i(const GridCoord &p)
{
    if (!getNGrid(p.x_coord, p.y_coord))
    {
        if (!getNGrid(p.x_coord, p.y_coord))
        {
            setNGrid(new NGridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord, p.x_coord, p.y_coord, i_gridExpiry),
                p.x_coord, p.y_coord);

            // build a linkage between this map and NGridType
            buildNGridLinkage(getNGrid(p.x_coord, p.y_coord));
            getNGrid(p.x_coord, p.y_coord)->SetGridState(GRID_STATE_IDLE);

            //z coord
            int gx = (MAX_NUMBER_OF_GRIDS - 1) - p.x_coord;
            int gy = (MAX_NUMBER_OF_GRIDS - 1) - p.y_coord;

            if (!GridMaps[gx][gy])
                Map::LoadMapAndVMap(i_id, i_InstanceId, gx, gy);
        }
    }
}

//Load NGrid and make it active
void Map::EnsureGridLoadedForActiveObject(const Cell &cell, WorldObject* object)
{
    EnsureGridLoaded(cell);
    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(grid != NULL);

    // refresh grid state & timer
    if (grid->GetGridState() != GRID_STATE_ACTIVE)
    {
        TC_LOG_DEBUG("maps", "Active object %s triggers loading of grid [%u, %u] on map %u", ObjectGuid(object->GetGUID()).ToString().c_str(), cell.GridX(), cell.GridY(), GetId());
        ResetGridExpiry(*grid, 0.1f);
        grid->SetGridState(GRID_STATE_ACTIVE);
    }
}

void Map::EnsureGridLoaded(const Cell& cell)
{
    EnsureGridCreated(GridCoord(cell.GridX(), cell.GridY()));
    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());

    assert(grid != nullptr);
    if (!isGridObjectDataLoaded(cell.GridX(), cell.GridY()))
    {
        //TC_LOG_DEBUG("maps", "Loading grid[%u, %u] for map %u instance %u", cell.GridX(), cell.GridY(), GetId(), i_InstanceId);
        TC_LOG_DEBUG("maps", "Active object nearby triggers of loading grid [%u,%u] on map %u", cell.GridX(), cell.GridY(), i_id);

        setGridObjectDataLoaded(true, cell.GridX(), cell.GridY());

        if (!m_disableMapObjects)
        {
            ObjectGridLoader loader(*grid, this, cell);
            loader.LoadN();
        }

        Balance();
    }
}

void Map::LoadGrid(float x, float y)
{
    EnsureGridLoaded(Cell(x,y));
}

bool Map::AddPlayerToMap(Player *player)
{
    // update player state for other player and visa-versa
    CellCoord cellCoord = Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY());
    if (!cellCoord.IsCoordValid())
    {
        TC_LOG_ERROR("maps", "Map::Add: Player (GUID: %u) has invalid coordinates X:%f Y:%f grid cell [%u:%u]", ObjectGuid(player->GetGUID()).GetCounter(), player->GetPositionX(), player->GetPositionY(), cellCoord.x_coord, cellCoord.y_coord);
        return false;
    }

    Cell cell(cellCoord);
    EnsureGridLoadedForActiveObject(cell, player);
    AddToGrid(player, cell);

    // Check if we are adding to correct map
    ASSERT(player->GetMap() == this);
    player->SetMap(this);
    player->AddToWorld();

    SendInitSelf(player);
    SendInitTransports(player);
    SendZoneDynamicInfo(player);

    player->m_clientGUIDs.clear();
    player->UpdateObjectVisibility(false);

    if (player->IsAlive())
        ConvertCorpseToBones(player->GetGUID());

    sScriptMgr->OnPlayerEnterMap(this, player);
    return true;
}

bool Map::CanUnload(uint32 diff)
{
    if (!m_unloadTimer) 
        return false;

    if (m_unloadTimer <= diff) 
        return true;

    m_unloadTimer -= diff;
    return false;
}

template<class T>
void Map::InitializeObject(T* /*obj*/) { }

template<>
void Map::InitializeObject(Creature* obj)
{
    obj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
}

template<>
void Map::InitializeObject(GameObject* obj)
{
    obj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
}

template<class T>
bool Map::AddToMap(T *obj, bool checkTransport)
{
    static_assert(!std::is_same<Player, T>::value, "Players must use AddPlayerToMap function");

    /// @todo Needs clean up. An object should not be added to map twice.
    if (obj->IsInWorld())
    {
        DEBUG_ASSERT(false); //sunstrider addition
        ASSERT(obj->IsInGrid());
        obj->UpdateObjectVisibility(true);
        return true;
    }

    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());

    assert(obj);

    if (!p.IsCoordValid())
    {
        TC_LOG_ERROR("maps","Map::Add: Object " UI64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return false;
    }

    Cell cell(p);
    if(obj->isActiveObject())
        EnsureGridLoadedForActiveObject(cell, obj);
    else
        EnsureGridCreated(GridCoord(cell.GridX(), cell.GridY()));

    AddToGrid(obj, cell);
    //TC_LOG_DEBUG("maps", "Object %s enters grid[%u, %u]", obj->GetGUID().ToString().c_str(), cell.GridX(), cell.GridY());

    //Must already be set before AddToMap. Usually during obj->Create.
    //obj->SetMap(this);
    obj->AddToWorld();

    InitializeObject(obj);

    if (checkTransport)
        if (!(obj->GetTypeId() == TYPEID_GAMEOBJECT && obj->ToGameObject()->IsTransport())) // dont add transport to transport ;d
            if (Transport* transport = GetTransportForPos(obj->GetPhaseMask(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj))
                transport->AddPassenger(obj, true);

    if(obj->isActiveObject())
        AddToForceActive(obj);

    //TC_LOG_DEBUG("maps","Object %u enters grid[%u,%u]", GUID_LOPART(obj->GetGUID()), cell.GridX(), cell.GridY());

    obj->UpdateObjectVisibilityOnCreate();
    return true;
}

template<>
bool Map::AddToMap(MotionTransport* obj, bool /* checkTransport */)
{
    assert(obj);
    if (obj->IsInWorld())
        DEBUG_ASSERT(false);

    CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
    if (!p.IsCoordValid())
    {
        TC_LOG_ERROR("maps", "Map::Add: Object " UI64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return false;
    }

    Cell cell(p);
    if (obj->isActiveObject())
        EnsureGridLoaded(cell);

    obj->AddToWorld();

    if (obj->isActiveObject())
        AddToForceActive(obj);

    //DO NOT ADD TO GRID. Else transport will be removed with grid unload. Transports are being kept updated even in unloaded grid.
    _transports.insert(obj);

    // Broadcast creation to players
    if (!GetPlayers().isEmpty())
    {
        for (const auto & itr : GetPlayers())
        {
            if (itr.GetSource()->GetTransport() != obj)
            {
                UpdateData data;
                obj->BuildCreateUpdateBlockForPlayer(&data, itr.GetSource());
                WorldPacket* packetBC = nullptr;
                WorldPacket* packetLK = nullptr;
                WorldSession::SendUpdateDataPacketForBuild(data, packetBC, packetLK, itr.GetSource()->GetSession(), true);
                delete packetBC;
                delete packetLK;
            }
        }
    }

    return true;
}

bool Map::IsGridLoaded(const GridCoord &p) const
{
    return ( getNGrid(p.x_coord, p.y_coord) && isGridObjectDataLoaded(p.x_coord, p.y_coord));
}

struct ResetNotifier
{
    template<class T>inline void resetNotify(GridRefManager<T> &m)
    {
        for (typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
            iter->GetSource()->ResetAllNotifies();
    }
    template<class T> void Visit(GridRefManager<T> &) { }
    void Visit(CreatureMapType &m) { resetNotify<Creature>(m); }
    void Visit(PlayerMapType &m) { resetNotify<Player>(m); }
};

void Map::ProcessRelocationNotifies(const uint32 diff)
{
    for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end(); ++i)
    {
        NGridType *grid = i->GetSource();

        if (grid->GetGridState() != GRID_STATE_ACTIVE)
            continue;

        grid->getGridInfoRef()->getRelocationTimer().TUpdate(diff);
        if (!grid->getGridInfoRef()->getRelocationTimer().TPassed())
            continue;

        uint32 gx = grid->getX(), gy = grid->getY();

        CellCoord cell_min(gx*MAX_NUMBER_OF_CELLS, gy*MAX_NUMBER_OF_CELLS);
        CellCoord cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord + MAX_NUMBER_OF_CELLS);

        for (uint32 x = cell_min.x_coord; x < cell_max.x_coord; ++x)
        {
            for (uint32 y = cell_min.y_coord; y < cell_max.y_coord; ++y)
            {
                uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
                if (!isCellMarked(cell_id))
                    continue;

                CellCoord pair(x, y);
                Cell cell(pair);
                cell.SetNoCreate();

                Trinity::DelayedUnitRelocation cell_relocation(cell, pair, *this, MAX_VISIBILITY_DISTANCE);
                TypeContainerVisitor<Trinity::DelayedUnitRelocation, GridTypeMapContainer  > grid_object_relocation(cell_relocation);
                TypeContainerVisitor<Trinity::DelayedUnitRelocation, WorldTypeMapContainer > world_object_relocation(cell_relocation);
                Visit(cell, grid_object_relocation);
                Visit(cell, world_object_relocation);
            }
        }
    }

    ResetNotifier reset;
    TypeContainerVisitor<ResetNotifier, GridTypeMapContainer >  grid_notifier(reset);
    TypeContainerVisitor<ResetNotifier, WorldTypeMapContainer > world_notifier(reset);
    for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end(); ++i)
    {
        NGridType *grid = i->GetSource();

        if (grid->GetGridState() != GRID_STATE_ACTIVE)
            continue;

        if (!grid->getGridInfoRef()->getRelocationTimer().TPassed())
            continue;

        grid->getGridInfoRef()->getRelocationTimer().TReset(diff, m_VisibilityNotifyPeriod);

        uint32 gx = grid->getX(), gy = grid->getY();

        CellCoord cell_min(gx*MAX_NUMBER_OF_CELLS, gy*MAX_NUMBER_OF_CELLS);
        CellCoord cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord + MAX_NUMBER_OF_CELLS);

        for (uint32 x = cell_min.x_coord; x < cell_max.x_coord; ++x)
        {
            for (uint32 y = cell_min.y_coord; y < cell_max.y_coord; ++y)
            {
                uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
                if (!isCellMarked(cell_id))
                    continue;

                CellCoord pair(x, y);
                Cell cell(pair);
                cell.SetNoCreate();
                Visit(cell, grid_notifier);
                Visit(cell, world_notifier);
            }
        }
    }
}

void Map::VisitNearbyCellsOf(WorldObject* obj, TypeContainerVisitor<Trinity::ObjectUpdater, GridTypeMapContainer> &gridVisitor, TypeContainerVisitor<Trinity::ObjectUpdater, WorldTypeMapContainer> &worldVisitor)
{
    // Check for valid position
    if (!obj->IsPositionValid())
        return;

    // Update mobs/objects in ALL visible cells around object!
    CellArea area = Cell::CalculateCellArea(obj->GetPositionX(), obj->GetPositionY(), obj->GetGridActivationRange());

    for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
    {
        for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
        {
            // marked cells are those that have been visited
            // don't visit the same cell twice
            uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
            if (isCellMarked(cell_id))
                continue;

            markCell(cell_id);
            CellCoord pair(x, y);
            Cell cell(pair);
            cell.SetNoCreate();
            Visit(cell, gridVisitor);
            Visit(cell, worldVisitor);
        }
    }
}

void Map::DoUpdate(uint32 maxDiff, uint32 minimumTimeSinceLastUpdate)
{
    uint32 now = GetMSTime();
    uint32 diff = GetMSTimeDiff(_lastMapUpdate, now);
    //freeze thread if last update was less than 10ms ago
    if(diff < minimumTimeSinceLastUpdate) {
        std::this_thread::sleep_for(std::chrono::milliseconds(minimumTimeSinceLastUpdate - diff));
        diff = minimumTimeSinceLastUpdate;
    }
    if (diff > maxDiff)
        diff = maxDiff;
    _lastMapUpdate = now;
    Update(diff);
}


void Map::Update(const uint32 &t_diff)
{
    _dynamicTree.update(t_diff);
    /// update worldsessions for existing players
    for(m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* plr = m_mapRefIter->GetSource();
        if(plr && plr->IsInWorld())
        {
            //plr->Update(t_diff);
            WorldSession * pSession = plr->GetSession();
            MapSessionFilter updater(pSession);

            pSession->Update(t_diff, updater);
        }
    }

    resetMarkedCells();

    Trinity::ObjectUpdater updater(t_diff);
    // for creature
    TypeContainerVisitor<Trinity::ObjectUpdater, GridTypeMapContainer  > grid_object_update(updater);
    // for pets
    TypeContainerVisitor<Trinity::ObjectUpdater, WorldTypeMapContainer > world_object_update(updater);

    // the player iterator is stored in the map object
    // to make sure calls to Map::Remove don't invalidate it
    for(m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* plr = m_mapRefIter->GetSource();

        if (!plr || !plr->IsInWorld())
            continue;

        // update players at tick
        plr->Update(t_diff);

        // If player is using far sight or mind vision, visit that object too
        if (WorldObject* viewPoint = plr->GetViewpoint())
            VisitNearbyCellsOf(viewPoint, grid_object_update, world_object_update);

        VisitNearbyCellsOf(plr, grid_object_update, world_object_update);
    }

    //must be done before creatures update
    for (auto itr : CreatureGroupHolder)
        itr.second->Update(t_diff);

    // non-player active objects, increasing iterator in the loop in case of object removal
    for (m_activeForcedNonPlayersIter = m_activeForcedNonPlayers.begin(); m_activeForcedNonPlayersIter != m_activeForcedNonPlayers.end();)
    {
        WorldObject* obj = *m_activeForcedNonPlayersIter;
        ++m_activeForcedNonPlayersIter;

        if (!obj || !obj->IsInWorld())
            continue;

        VisitNearbyCellsOf(obj, grid_object_update, world_object_update);
    }

    //update our transports
    for (_transportsUpdateIter = _transports.begin(); _transportsUpdateIter != _transports.end();)
    {
        WorldObject* obj = *_transportsUpdateIter;
        ++_transportsUpdateIter;

        if (!obj->IsInWorld())
            continue;

        obj->Update(t_diff);
    }

    SendObjectUpdates();

    ///- Process necessary scripts
    if (!m_scriptSchedule.empty())
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }

    MoveAllCreaturesInMoveList();
    MoveAllGameObjectsInMoveList();

    if (!m_mapRefManager.isEmpty() || !m_activeForcedNonPlayers.empty())
        ProcessRelocationNotifies(t_diff);

    sScriptMgr->OnMapUpdate(this, t_diff);
}

void Map::RemovePlayerFromMap(Player *player, bool remove)
{
    // Before leaving map, update zone/area for stats
    player->UpdateZone(MAP_INVALID_ZONE, 0);
    sScriptMgr->OnPlayerLeaveMap(this, player);

    player->GetHostileRefManager().deleteReferences(); // multithreading crashfix

    ASSERT(player);

    bool const inWorld = player->IsInWorld();
    player->RemoveFromWorld();

    SendRemoveTransports(player);

    if (!inWorld) // if was in world, RemoveFromWorld() called DestroyForNearbyPlayers()
        player->DestroyForNearbyPlayers(); // previous player->UpdateObjectVisibility(true)

    if (player->IsInGrid())
        player->RemoveFromGrid();
    else
        ASSERT(remove); //maybe deleted in logoutplayer when player is not in a map

    if (remove)
        DeleteFromWorld(player);
}

template<class T>
void Map::RemoveFromMap(T *obj, bool remove)
{
    bool const inWorld = obj->IsInWorld() && obj->GetTypeId() >= TYPEID_UNIT && obj->GetTypeId() <= TYPEID_GAMEOBJECT;
    obj->RemoveFromWorld();

    if(obj->isActiveObject())
        RemoveFromForceActive(obj);

    if (!inWorld) // if was in world, RemoveFromWorld() called DestroyForNearbyPlayers()
        obj->DestroyForNearbyPlayers(); // previous obj->UpdateObjectVisibility(true)

    obj->RemoveFromGrid();

    obj->ResetMap();

    if(remove)
    {
        // if option set then object already saved at this moment
        if(!sWorld->getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();

        DeleteFromWorld(obj);
    }
}

template<>
void Map::RemoveFromMap(MotionTransport* obj, bool remove)
{
    obj->RemoveFromWorld();
    if (obj->isActiveObject())
        RemoveFromForceActive(obj);

    //remove for players in map
    Map::PlayerList const& players = GetPlayers();
    if (!players.isEmpty())
    {
        UpdateData data;
        obj->BuildOutOfRangeUpdateBlock(&data);
        WorldPacket* packetBC = nullptr;
        WorldPacket* packetLK = nullptr;
        for (const auto & player : players)
            if (player.GetSource()->GetTransport() != obj)
                WorldSession::SendUpdateDataPacketForBuild(data, packetBC, packetLK, player.GetSource()->GetSession());

        delete packetBC;
        delete packetLK;
    }

    if (_transportsUpdateIter != _transports.end())
    {
        auto itr = _transports.find(obj);
        if (itr == _transports.end())
            //did not find transport in list ? Should not happen, logic error somewhere
            ASSERT(false);
        if (itr == _transportsUpdateIter)
            ++_transportsUpdateIter;

        _transports.erase(itr);
    }
    else
        _transports.erase(obj);

    obj->ResetMap();

    if (remove)
    {
        // if option set then object already saved at this moment
        if (!sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();

        DeleteFromWorld(obj);
    }
}

void Map::PlayerRelocation(Player *player, float x, float y, float z, float orientation)
{
    assert(player);

    CellCoord old_val = Trinity::ComputeCellCoord(player->GetPositionX(), player->GetPositionY());
    CellCoord new_val = Trinity::ComputeCellCoord(x, y);

    Cell old_cell(old_val);
    Cell new_cell(new_val);

    player->Relocate(x, y, z, orientation);
#ifdef LICH_KING
    if (player->IsVehicle())
        player->GetVehicleKit()->RelocatePassengers();
#endif

    if( old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell))
    {
        //TC_LOG_DEBUG("maps","Player %s relocation grid[%u,%u]cell[%u,%u]->grid[%u,%u]cell[%u,%u]", player->GetName(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        // update player position for group at taxi flight
        if(player->GetGroup() && player->IsInFlight())
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);

        player->RemoveFromGrid();
        if( old_cell.DiffGrid(new_cell))
            EnsureGridLoadedForActiveObject(new_cell, player);

        AddToGrid(player, new_cell);
    }

    player->UpdatePositionData();
    player->UpdateObjectVisibility(false);
}

void Map::CreatureRelocation(Creature *creature, float x, float y, float z, float ang)
{
    assert(CheckGridIntegrity(creature,false));

    Cell old_cell = creature->GetCurrentCell();

    CellCoord new_val = Trinity::ComputeCellCoord(x, y);
    Cell new_cell(new_val);

    // delay creature move for grid/cell to grid/cell moves
    if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        TC_LOG_DEBUG("debug.creature","Creature (GUID: %u Entry: %u) added to moving list from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", creature->GetGUIDLow(), creature->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        AddCreatureToMoveList(creature,x,y,z,ang);
        // in diffcell/diffgrid case notifiers called at finishing move creature in Map::MoveAllCreaturesInMoveList
    }
    else
    {
        creature->Relocate(x, y, z, ang);
#ifdef LICH_KING
        if (creature->IsVehicle())
            creature->GetVehicleKit()->RelocatePassengers();
#endif
        creature->UpdateObjectVisibility(false);
        creature->UpdatePositionData();
        RemoveCreatureFromMoveList(creature);
    }
    assert(CheckGridIntegrity(creature,true));
}

void Map::GameObjectRelocation(GameObject* go, float x, float y, float z, float ang)
{
    assert(CheckGridIntegrity(go,false));

    Cell old_cell = go->GetCurrentCell();

    CellCoord new_val = Trinity::ComputeCellCoord(x, y);
    Cell new_cell(new_val);

    // delay creature move for grid/cell to grid/cell moves
    if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
        TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) added to moving list from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
        AddGameObjectToMoveList(go, x, y, z, ang);
        // in diffcell/diffgrid case notifiers called at finishing move creature in Map::MoveAllCreaturesInMoveList
    }
    else
    {
        go->Relocate(x, y, z, ang);
        go->UpdateModelPosition();
        go->UpdatePositionData();
        go->UpdateObjectVisibility(false);
        RemoveGameObjectFromMoveList(go);
    }
    assert(CheckGridIntegrity(go,true));
}

void Map::DynamicObjectRelocation(DynamicObject* dynObj, float x, float y, float z, float orientation)
{
    Cell integrity_check(dynObj->GetPositionX(), dynObj->GetPositionY());
    Cell old_cell = dynObj->GetCurrentCell();

    ASSERT(integrity_check == old_cell);
    CellCoord new_val = Trinity::ComputeCellCoord(x, y);
    Cell new_cell(new_val);

    if (!getNGrid(new_cell.GridX(), new_cell.GridY()))
        return;

    // delay creature move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
    {
#ifdef TRINITY_DEBUG
        TC_LOG_DEBUG("maps", "GameObject (GUID: %u) added to moving list from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", ObjectGuid(dynObj->GetGUID()).GetCounter(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif
        AddDynamicObjectToMoveList(dynObj, x, y, z, orientation);
        // in diffcell/diffgrid case notifiers called at finishing move dynObj in Map::MoveAllGameObjectsInMoveList
    }
    else
    {
        dynObj->Relocate(x, y, z, orientation);
        dynObj->UpdatePositionData();
        dynObj->UpdateObjectVisibility(false);
        RemoveDynamicObjectFromMoveList(dynObj);
    }

    old_cell = dynObj->GetCurrentCell();
    integrity_check = Cell(dynObj->GetPositionX(), dynObj->GetPositionY());
    ASSERT(integrity_check == old_cell);
}

void Map::AddCreatureToMoveList(Creature *c, float x, float y, float z, float ang)
{
    if (_creatureToMoveLock) //can this happen?
        return;

    if (c->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _creaturesToMove.push_back(c);
    c->SetNewCellPosition(x, y, z, ang);
}

void Map::RemoveCreatureFromMoveList(Creature* c)
{
    if (_creatureToMoveLock) //can this happen?
        return;

    if (c->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        c->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
}

void Map::AddGameObjectToMoveList(GameObject* go, float x, float y, float z, float ang)
{
    if (_gameObjectsToMoveLock) //can this happen?
        return;

    if (go->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _gameObjectsToMove.push_back(go);
    go->SetNewCellPosition(x, y, z, ang);
}

void Map::RemoveGameObjectFromMoveList(GameObject* go)
{
    if (_gameObjectsToMoveLock) //can this happen?
        return;

    if (go->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        go->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
}

void Map::AddDynamicObjectToMoveList(DynamicObject* dynObj, float x, float y, float z, float ang)
{
    if (_dynamicObjectsToMoveLock) //can this happen?
        return;

    if (dynObj->_moveState == MAP_OBJECT_CELL_MOVE_NONE)
        _dynamicObjectsToMove.push_back(dynObj);
    dynObj->SetNewCellPosition(x, y, z, ang);
}

void Map::RemoveDynamicObjectFromMoveList(DynamicObject* dynObj)
{
    if (_dynamicObjectsToMoveLock) //can this happen?
        return;

    if (dynObj->_moveState == MAP_OBJECT_CELL_MOVE_ACTIVE)
        dynObj->_moveState = MAP_OBJECT_CELL_MOVE_INACTIVE;
}

void Map::MoveAllCreaturesInMoveList()
{
    _creatureToMoveLock = true;
    for (auto c : _creaturesToMove)
    {
        if (c->FindMap() != this) //pet is teleported to another map
            continue;

        if (c->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            c->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        c->_moveState = MAP_OBJECT_CELL_MOVE_NONE;

        if (!c->IsInWorld())
            continue;

        // calculate cells
        CellCoord new_val = Trinity::ComputeCellCoord(c->_newPosition.m_positionX, c->_newPosition.m_positionY);
        Cell new_cell(new_val);

        // do move or do move to respawn or remove creature if previous all fail
        if (CreatureCellRelocation(c, new_cell))
        {
            // update pos
            c->Relocate(c->_newPosition);
#ifdef LICH_KING
        if (c->IsVehicle())
            c->GetVehicleKit()->RelocatePassengers();
#endif
            c->UpdatePositionData();
            c->UpdateObjectVisibility(false);
        }
        else
        {
            // if creature can't be move in new cell/grid (not loaded) move it to repawn cell/grid
            // creature coordinates will be updated and notifiers send
            if(!CreatureRespawnRelocation(c,false))
            {
                // ... or unload (if respawn grid also not loaded)
                TC_LOG_DEBUG("debug.creature","Creature (GUID: %u Entry: %u) can't be move to unloaded respawn grid.",c->GetGUIDLow(),c->GetEntry());
                /// @todo pets will disappear if this is outside CreatureRespawnRelocation
                //need to check why pet is frequently relocated to an unloaded cell
                if (c->IsPet())
                    ((Pet*)c)->Remove(PET_SAVE_NOT_IN_SLOT, true);
                else 
                    AddObjectToRemoveList(c);
            }
        }
    }
    _creaturesToMove.clear();
    _creatureToMoveLock = false;
}

void Map::MoveAllGameObjectsInMoveList()
{
    _gameObjectsToMoveLock = true;
    for(auto go : _gameObjectsToMove)
    {
        if (go->FindMap() != this) //transport is teleported to another map
            continue;

        if (go->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            go->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        go->_moveState = MAP_OBJECT_CELL_MOVE_NONE;

        if (!go->IsInWorld())
            continue;

        CellCoord new_val = Trinity::ComputeCellCoord(go->_newPosition.m_positionX, go->_newPosition.m_positionY);
        Cell new_cell(new_val);

        // do move or do move to respawn or remove creature if previous all fail
        if (GameObjectCellRelocation(go, new_cell))
        {
            // update pos
            go->Relocate(go->_newPosition);
            go->UpdateModelPosition();
            go->UpdatePositionData();
            go->UpdateObjectVisibility(false);
        }
        else
        {
            // if creature can't be move in new cell/grid (not loaded) move it to repawn cell/grid
            // creature coordinates will be updated and notifiers send
            if(!GameObjectRespawnRelocation(go,false))
            {
                // ... or unload (if respawn grid also not loaded)
                TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) can't be move to unloaded respawn grid.",go->GetGUIDLow(),go->GetEntry());

                AddObjectToRemoveList(go);
            }
        }
    }
    _gameObjectsToMove.clear();
    _gameObjectsToMoveLock = false;
}

void Map::MoveAllDynamicObjectsInMoveList()
{
    _dynamicObjectsToMoveLock = true;
    for(auto dynObj : _dynamicObjectsToMove)
    {
        if (dynObj->FindMap() != this) //transport is teleported to another map
            continue;

        if (dynObj->_moveState != MAP_OBJECT_CELL_MOVE_ACTIVE)
        {
            dynObj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;
            continue;
        }

        dynObj->_moveState = MAP_OBJECT_CELL_MOVE_NONE;

        if (!dynObj->IsInWorld())
            continue;

        CellCoord new_val = Trinity::ComputeCellCoord(dynObj->_newPosition.m_positionX, dynObj->_newPosition.m_positionY);
        Cell new_cell(new_val);

        // do move or do move to respawn or remove creature if previous all fail
        if (DynamicObjectCellRelocation(dynObj, new_cell))
        {
            // update pos
            dynObj->Relocate(dynObj->_newPosition);
            dynObj->UpdatePositionData();
            dynObj->UpdateObjectVisibility(false);
        }
        else
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG("maps", "DynamicObject (GUID: %u) cannot be moved to unloaded grid.", ObjectGuid(dynObj->GetGUID()).GetCounter());
#endif
        }
    }

    _dynamicObjectsToMove.clear();
    _dynamicObjectsToMoveLock = false;
}

bool Map::CreatureCellRelocation(Creature *c, Cell new_cell)
{
    Cell const& old_cell = c->GetCurrentCell();
    if(!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if(old_cell.DiffCell(new_cell))
        {
            #ifdef TRINITY_DEBUG
                TC_LOG_DEBUG("debug.grid","Creature (GUID: %u Entry: %u) moved in grid[%u,%u] from cell[%u,%u] to cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
            #endif

            c->RemoveFromGrid();
            AddToGrid(c, new_cell);
        }
        else
        {
            #ifdef TRINITY_DEBUG
                TC_LOG_DEBUG("debug.grid","Creature (GUID: %u Entry: %u) move in same grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
            #endif    
        }

        return true;
    }

    // in diff. grids but active creature
    if(c->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, c);

        TC_LOG_DEBUG("debug.creature","Active creature (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        c->RemoveFromGrid();
        AddToGrid(c, new_cell);

        return true;
    }

    // in diff. loaded grid normal creature
    if(IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
        TC_LOG_DEBUG("debug.creature","Creature (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        c->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(c, new_cell);

        return true;
    }

    // fail to move: normal creature attempt move to unloaded grid
    TC_LOG_DEBUG("debug.creature","Creature (GUID: %u Entry: %u) attempt move from grid[%u,%u]cell[%u,%u] to unloaded grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
    return false;
}

bool Map::GameObjectCellRelocation(GameObject* go, Cell new_cell)
{
    Cell const& old_cell = go->GetCurrentCell();
    if(!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if(old_cell.DiffCell(new_cell))
        {
            TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) moved in grid[%u,%u] from cell[%u,%u] to cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());

            go->RemoveFromGrid();
            AddToGrid(go, new_cell);
        }
        else
        {
            TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) move in same grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
        }

        return true;
    }

    // in diff. grids but active creature
    if(go->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, go);

        TC_LOG_DEBUG("debug.creature","Active gameobject (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        go->RemoveFromGrid();
        AddToGrid(go, new_cell);

        return true;
    }

    // in diff. loaded grid normal creature
    if(IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
        TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) moved from grid[%u,%u]cell[%u,%u] to grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

        go->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(go, new_cell);

        return true;
    }

    // fail to move: normal creature attempt move to unloaded grid
    TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) attempt move from grid[%u,%u]cell[%u,%u] to unloaded grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());

    return false;
}

bool Map::DynamicObjectCellRelocation(DynamicObject* go, Cell new_cell)
{
    Cell const& old_cell = go->GetCurrentCell();
    if (!old_cell.DiffGrid(new_cell))                       // in same grid
    {
        // if in same cell then none do
        if (old_cell.DiffCell(new_cell))
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG("maps", "DynamicObject (GUID: %u) moved in grid[%u, %u] from cell[%u, %u] to cell[%u, %u].", ObjectGuid(go->GetGUID()).GetCounter(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.CellX(), new_cell.CellY());
#endif

            go->RemoveFromGrid();
            AddToGrid(go, new_cell);
        }
        else
        {
#ifdef TRINITY_DEBUG
            TC_LOG_DEBUG("maps", "DynamicObject (GUID: %u) moved in same grid[%u, %u]cell[%u, %u].", ObjectGuid(go->GetGUID()).GetCounter(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY());
#endif
        }

        return true;
    }

    // in diff. grids but active GameObject
    if (go->isActiveObject())
    {
        EnsureGridLoadedForActiveObject(new_cell, go);

#ifdef TRINITY_DEBUG
        TC_LOG_DEBUG("maps", "Active DynamicObject (GUID: %u) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", ObjectGuid(go->GetGUID()).GetCounter(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif

        go->RemoveFromGrid();
        AddToGrid(go, new_cell);

        return true;
    }

    // in diff. loaded grid normal GameObject
    if (IsGridLoaded(GridCoord(new_cell.GridX(), new_cell.GridY())))
    {
#ifdef TRINITY_DEBUG
        TC_LOG_DEBUG("maps", "DynamicObject (GUID: %u) moved from grid[%u, %u]cell[%u, %u] to grid[%u, %u]cell[%u, %u].", ObjectGuid(go->GetGUID()).GetCounter(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif

        go->RemoveFromGrid();
        EnsureGridCreated(GridCoord(new_cell.GridX(), new_cell.GridY()));
        AddToGrid(go, new_cell);

        return true;
    }

    // fail to move: normal GameObject attempt move to unloaded grid
#ifdef TRINITY_DEBUG
    TC_LOG_DEBUG("maps", "DynamicObject (GUID: %u) attempted to move from grid[%u, %u]cell[%u, %u] to unloaded grid[%u, %u]cell[%u, %u].", ObjectGuid(go->GetGUID()).GetCounter(), old_cell.GridX(), old_cell.GridY(), old_cell.CellX(), old_cell.CellY(), new_cell.GridX(), new_cell.GridY(), new_cell.CellX(), new_cell.CellY());
#endif
    return false;
}

bool Map::CreatureRespawnRelocation(Creature *c, bool diffGridOnly)
{
    float resp_x, resp_y, resp_z, resp_o;
    c->GetRespawnPosition(resp_x, resp_y, resp_z, &resp_o);

    CellCoord resp_val = Trinity::ComputeCellCoord(resp_x, resp_y);
    Cell resp_cell(resp_val);

    const Cell current_cell = c->GetCurrentCell();
    //creature will be unloaded with grid
    if (diffGridOnly && !current_cell.DiffGrid(resp_cell))
        return true;

    c->CombatStop();
    c->GetMotionMaster()->Clear();

    TC_LOG_DEBUG("debug.creature","Creature (GUID: %u Entry: %u) will moved from grid[%u,%u]cell[%u,%u] to respawn grid[%u,%u]cell[%u,%u].", c->GetGUIDLow(), c->GetEntry(), current_cell.GridX(), current_cell.GridY(), current_cell.CellX(),current_cell.CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());

    // teleport it to respawn point (like normal respawn if player see)
    if(CreatureCellRelocation(c,resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, resp_o);
        c->GetMotionMaster()->Initialize();                 // prevent possible problems with default move generators
        //CreatureRelocationNotify(c,resp_cell,resp_cell.GetCellCoord());
        c->UpdatePositionData();
        c->UpdateObjectVisibility(false);
        return true;
    }
    else
        return false;
}

bool Map::GameObjectRespawnRelocation(GameObject* go, bool diffGridOnly)
{
    float resp_x, resp_y, resp_z, resp_o;
    go->GetRespawnPosition(resp_x, resp_y, resp_z, &resp_o);

    CellCoord resp_val = Trinity::ComputeCellCoord(resp_x, resp_y);
    Cell resp_cell(resp_val);

    const Cell current_cell = go->GetCurrentCell();
    //gameobjects will be unloaded with grid
    if (diffGridOnly && !current_cell.DiffGrid(resp_cell))
        return true;

    TC_LOG_DEBUG("debug.creature","GameObject (GUID: %u Entry: %u) will moved from grid[%u,%u]cell[%u,%u] to respawn grid[%u,%u]cell[%u,%u].", go->GetGUIDLow(), go->GetEntry(), current_cell.GridX(), current_cell.GridY(), current_cell.CellX(),current_cell.CellY(), resp_cell.GridX(), resp_cell.GridY(), resp_cell.CellX(), resp_cell.CellY());

    // teleport it to respawn point (like normal respawn if player see)
    if(GameObjectCellRelocation(go,resp_cell))
    {
        go->Relocate(resp_x, resp_y, resp_z, resp_o);
        go->UpdatePositionData();
        go->UpdateObjectVisibility(false);
        return true;
    }

    return false;
}

bool Map::UnloadGrid(NGridType& ngrid, bool unloadAll)
{
    const uint32 x = ngrid.getX();
    const uint32 y = ngrid.getY();

    {
        if(!unloadAll)
        {  
            //pets, possessed creatures (must be active), transport passengers
            if (ngrid.GetWorldObjectCountInNGrid<Creature>())
                return false;

             if(ActiveObjectsNearGrid(ngrid))
                return false;
        }

        if(!unloadAll)
        {
            // Finish creature moves, remove and delete all creatures with delayed remove before moving to respawn grids
            // Must know real mob position before move
            MoveAllCreaturesInMoveList();
            MoveAllGameObjectsInMoveList();

            // move creatures to respawn grids if this is diff.grid or to remove list
            ObjectGridEvacuator worker;
            TypeContainerVisitor<ObjectGridEvacuator, GridTypeMapContainer> visitor(worker);
            ngrid.VisitAllGrids(visitor);

            // Finish creature moves, remove and delete all creatures with delayed remove before unload
            MoveAllCreaturesInMoveList();
            MoveAllGameObjectsInMoveList();
        }

        {
            ObjectGridCleaner worker;
            TypeContainerVisitor<ObjectGridCleaner, GridTypeMapContainer> visitor(worker);
            ngrid.VisitAllGrids(visitor);
        }

        RemoveAllObjectsInRemoveList();

        {
            ObjectGridUnloader worker;
            TypeContainerVisitor<ObjectGridUnloader, GridTypeMapContainer> visitor(worker);
            ngrid.VisitAllGrids(visitor);
        }

        ASSERT(i_objectsToRemove.empty());

        delete &ngrid;
        setNGrid(nullptr, x, y);
    }
    int gx = (MAX_NUMBER_OF_GRIDS - 1) - x;
    int gy = (MAX_NUMBER_OF_GRIDS - 1) - y;

    // delete grid map, but don't delete if it is from parent map (and thus only reference)
    //+++if (GridMaps[gx][gy]) don't check for GridMaps[gx][gy], we might have to unload vmaps
    {
        if (i_InstanceId == 0)
        {
            if (GridMaps[gx][gy])
            {
                GridMaps[gx][gy]->unloadData();
                delete GridMaps[gx][gy];
            }
            VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(GetId(), gx, gy);
            MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(GetId(), gx, gy);
        }
        else
            ((MapInstanced*)m_parentMap)->RemoveGridMapReference(GridCoord(gx, gy)); 

        GridMaps[gx][gy] = nullptr;
    }
    TC_LOG_DEBUG("maps","Unloading grid[%u,%u] for map %u finished", x,y, i_id);
    return true;
}

bool Map::getObjectHitPos(uint32 phasemask, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist)
{
    G3D::Vector3 startPos = G3D::Vector3(x1, y1, z1);
    G3D::Vector3 dstPos = G3D::Vector3(x2, y2, z2);
    
    G3D::Vector3 resultPos;
    bool result = _dynamicTree.getObjectHitPos(phasemask, startPos, dstPos, resultPos, modifyDist);
    
    rx = resultPos.x;
    ry = resultPos.y;
    rz = resultPos.z;
    return result;
}

float Map::GetWaterOrGroundLevel(float x, float y, float z, float* ground /*= NULL*/, bool /*swim = false*/) const
{
    if (const_cast<Map*>(this)->GetGrid(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = GetHeight(x, y, z, true, 50.0f);
        if (ground)
            *ground = ground_z;

        LiquidData liquid_status;

        ZLiquidStatus res = GetLiquidStatus(x, y, ground_z, MAP_ALL_LIQUIDS, &liquid_status);
        switch (res)
        {
        case LIQUID_MAP_ABOVE_WATER:
            return std::max<float>(liquid_status.level, ground_z);
        case LIQUID_MAP_NO_WATER:
            return ground_z;
        default:
            return liquid_status.level;
        }
    }

    return INVALID_HEIGHT;
}

bool Map::GetLiquidLevelBelow(float x, float y, float z, float& liquidLevel, float maxSearchDist) const
{
    return false; //TODO
}

float Map::GetHeight(uint32 phasemask, float x, float y, float z, bool vmap/*=true*/, float maxSearchDist/*=DEFAULT_HEIGHT_SEARCH*/, bool walkableOnly /*= false*/) const
{
    return std::max<float>(GetHeight(x, y, z, vmap, maxSearchDist, walkableOnly), _dynamicTree.getHeight(x, y, z, maxSearchDist, phasemask)); //walkableOnly not implemented in dynamicTree
}

Transport* Map::GetTransportForPos(uint32 phase, float x, float y, float z, WorldObject* worldobject)
{
    G3D::Vector3 v(x, y, z + 2.0f);
    G3D::Ray r(v, G3D::Vector3(0, 0, -1));
    for (auto _transport : _transports)
        if (_transport->IsInWorld() && _transport->GetExactDistSq(x, y, z) < 75.0f*75.0f && _transport->m_model)
        {
            float dist = 30.0f;
            bool hit = _transport->m_model->intersectRay(r, dist, true, phase, VMAP::ModelIgnoreFlags::Nothing);
            if (hit)
                return _transport;
        }

    if (worldobject)
        if (GameObject* staticTrans = worldobject->FindNearestGameObjectOfType(GAMEOBJECT_TYPE_TRANSPORT, 75.0f))
            if (staticTrans->m_model)
            {
                float dist = 10.0f;
                bool hit = staticTrans->m_model->intersectRay(r, dist, true, phase, VMAP::ModelIgnoreFlags::Nothing);
                if (hit)
                    if (GetHeight(/*phase,*/ x, y, z, true, 30.0f) < (v.z - dist + 1.0f))
                        return staticTrans->ToTransport();
            }

    return nullptr;
}

float Map::GetHeight(float x, float y, float z, bool checkVMap, float maxSearchDist/*=DEFAULT_HEIGHT_SEARCH*/, bool walkableOnly /* = false */) const
{
    // find raw .map surface under Z coordinates
    float mapHeight = INVALID_HEIGHT;
    float mapHeightDist = 0.0f;
    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
    {
        float gridHeight = gmap->getHeight(x, y, walkableOnly);
        // if valid map height found, check for max search distance
        if (gridHeight > INVALID_HEIGHT)
        {
            mapHeightDist = fabs(gridHeight - z);
            if(mapHeightDist < maxSearchDist)
                mapHeight = gridHeight;
        }
    }

    // if no vmap check to do, we're done
    if(!checkVMap)
        return mapHeight;

    // else get vmap height
    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if (vmgr->isHeightCalcEnabled())
        vmapHeight = vmgr->getHeight(GetId(), x, y, z + 2.0f, maxSearchDist);   // look from a bit higher pos to find the floor
    
    // no valid vmap height found, we're done, return map height
    if(vmapHeight == VMAP_INVALID_HEIGHT_VALUE)
        return mapHeight; //may be INVALID_HEIGHT
    
    // at this point vmapHeight is valid, and mapHeight may still not be
    // if no valid mapHeight found, returns the found vmapHeight
    if(mapHeight == INVALID_HEIGHT)
        return vmapHeight;

    // we have both map height and vmap height at valid distance, lets choose the closer
    return mapHeightDist < std::fabs(vmapHeight - z) ? mapHeight : vmapHeight;
}

float Map::_GetHeight(float x, float y, float z, bool pUseVmaps, float maxSearchDist) const
{
    // find raw .map surface under Z coordinates
    float mapHeight;
    if (GridMap *gmap = const_cast<Map*>(this)->GetGrid(x, y))
    {
        float _mapheight = gmap->getHeight(x,y);

        // look from a bit higher pos to find the floor, ignore under surface case
        if (z + 2.0f > _mapheight)
            mapHeight = _mapheight;
        else
            mapHeight = VMAP_INVALID_HEIGHT_VALUE;
    }
    else
        mapHeight = VMAP_INVALID_HEIGHT_VALUE;

    float vmapHeight;
    if (pUseVmaps)
    {
        VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
        if (vmgr->isHeightCalcEnabled())
        {
            // look from a bit higher pos to find the floor
            vmapHeight = vmgr->getHeight(GetId(), x, y, z + 2.0f, maxSearchDist);
        }
        else
            vmapHeight = VMAP_INVALID_HEIGHT_VALUE;
    }
    else
        vmapHeight = VMAP_INVALID_HEIGHT_VALUE;

    // mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
    // vmapheight set for any under Z value or <= INVALID_HEIGHT

    if (vmapHeight > INVALID_HEIGHT)
    {
        if (mapHeight > INVALID_HEIGHT)
        {
            // we have mapheight and vmapheight and must select more appropriate

            // we are already under the surface or vmap height above map heigt
            // or if the distance of the vmap height is less the land height distance
            if (z < mapHeight || vmapHeight > mapHeight || fabs(mapHeight-z) > fabs(vmapHeight-z))
                return vmapHeight;
            else
                return mapHeight;                           // better use .map surface height

        }
        else
            return vmapHeight;                              // we have only vmapHeight (if have)
    }
    else
    {
        if (!pUseVmaps)
            return mapHeight;                               // explicitly use map data (if have)
        else if (mapHeight > INVALID_HEIGHT && (z < mapHeight + 2 || z == MAX_HEIGHT))
            return mapHeight;                               // explicitly use map data if original z < mapHeight but map found (z+2 > mapHeight)
        else
            return VMAP_INVALID_HEIGHT_VALUE;               // we not have any height
    }
}

float Map::GetMinHeight(float x, float y) const
{
    if (GridMap const* grid = const_cast<Map*>(this)->GetGrid(x, y))
        return grid->getMinHeight(x, y);
    
    return -500.0f;
}

inline bool IsOutdoorWMO(uint32 mogpFlags, int32 adtId, int32 rootId, int32 groupId, WMOAreaTableEntry const* wmoEntry, AreaTableEntry const* atEntry, uint32 mapId)
{
    // If this flag is set we are outdoors and can mount up
    if (mogpFlags & 0x8000)
        return true;
    
    if (mogpFlags == 0x809 || mogpFlags == 0x849) // From observations, these zones are actually outdoor
        return true;
    
    // If flag 0x800 is set and we are in non-flyable areas we cannot mount up even if we are physically outdoors
    if (mapId != 530 && (mogpFlags & 0x800))
        return false;
    
    // If this flag is set we are physically outdoors, mounting up is allowed if previous check failed
    return mogpFlags & 0x8;
}

bool Map::IsOutdoors(float x, float y, float z) const
{
    uint32 mogpFlags;
    int32 adtId, rootId, groupId;

    // no wmo found? -> outside by default
    if(!GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
        return true;

    AreaTableEntry const* atEntry = nullptr;
    WMOAreaTableEntry const* wmoEntry= GetWMOAreaTableEntryByTripple(rootId, adtId, groupId);
    if(wmoEntry)
    {
        TC_LOG_DEBUG("maps","Got WMOAreaTableEntry! flag %u, areaid %u", wmoEntry->Flags, wmoEntry->areaId);
        atEntry = sAreaTableStore.LookupEntry(wmoEntry->areaId);
    }
    return IsOutdoorWMO(mogpFlags, adtId, rootId, groupId, wmoEntry, atEntry, i_mapEntry->MapID);
}

bool Map::GetAreaInfo(float x, float y, float z, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupId) const
{
    float vmap_z = z;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    if (vmgr->getAreaInfo(GetId(), x, y, vmap_z, flags, adtId, rootId, groupId))
    {
        // check if there's terrain between player height and object height
        if(GridMap *gmap = const_cast<Map*>(this)->GetGrid(x, y))
        {
            float _mapheight = gmap->getHeight(x,y);
            // z + 2.0f condition taken from GetHeight(), not sure if it's such a great choice...
            if(z + 2.0f > _mapheight &&  _mapheight > vmap_z)
                return false;
        }
        return true;
    }
    return false;
}

uint32 Map::GetAreaId(float x, float y, float z, bool *isOutdoors) const
{
    uint32 mogpFlags;
    int32 adtId, rootId, groupId;
    WMOAreaTableEntry const* wmoEntry = nullptr;
    AreaTableEntry const* atEntry = nullptr;
    bool haveAreaInfo = false;

    if (GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
    {
        haveAreaInfo = true;
        if ((wmoEntry = GetWMOAreaTableEntryByTripple(rootId, adtId, groupId)))
            atEntry = sAreaTableStore.LookupEntry(wmoEntry->areaId);
    }

    uint16 areaId = 0;

    if (atEntry)
        areaId = atEntry->ID;
    else
    {
        if (GridMap *gmap = const_cast<Map*>(this)->GetGrid(x, y))
            areaId = gmap->getArea(x, y);
        // this used while not all *.map files generated (instances)

        if (!areaId)
            areaId = i_mapEntry->linked_zone;
    }

    if (isOutdoors) //pointer can be null
    {
        if (haveAreaInfo)
            *isOutdoors = IsOutdoorWMO(mogpFlags, adtId, rootId, groupId, wmoEntry, atEntry, i_mapEntry->MapID);
        else
            *isOutdoors = true;
    }
    return areaId;
}

uint8 Map::GetTerrainType(float x, float y) const
{
    if (GridMap *gmap = const_cast<Map*>(this)->GetGrid(x, y))
        return gmap->getTerrainType(x, y);
    else
        return 0;
}

ZLiquidStatus Map::GetLiquidStatus(float x, float y, float z, uint8 reqLiquidTypeMask, LiquidData* data) const
{
    ZLiquidStatus result = LIQUID_MAP_NO_WATER;
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float liquid_level = INVALID_HEIGHT;
    float ground_level = INVALID_HEIGHT;
    LiquidType liquid_type = LIQUID_TYPE_NO_WATER;
    if (vmgr->GetLiquidLevel(GetId(), x, y, z, reqLiquidTypeMask, liquid_level, ground_level, liquid_type))
    {
        TC_LOG_DEBUG("maps", "GetLiquidStatus(): vmap liquid level: %f ground: %f base liquid type: %u", liquid_level, ground_level, uint32(liquid_type));
        // Check water level and ground level
        if (liquid_level > ground_level && z > ground_level - 2)
        {
            // All ok in water -> store data
            if (data)
            {
#ifdef LICH_KING
                // hardcoded in client like this
                // sunstrider: What? Ocean water on Outland map is Green Lava? Does that make sense?
                if (GetId() == 530 && liquid_type == 2)
                    liquid_type = 15;
#endif

                uint32 liquidFlagType = 0;
                if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(liquid_type))
                    liquidFlagType = liq->GetType();

                if (liquid_type && liquid_type < LIQUID_TYPE_NAXXRAMAS_SLIME) //first special water? No need to check for those
                {
                    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(GetAreaId(x, y, z)))
                    {
                        LiquidType overrideLiquid = LiquidType(area->LiquidTypeOverride[liquidFlagType]);
                        if (!overrideLiquid && area->zone)
                        {
                            area = sAreaTableStore.LookupEntry(area->zone);
                            if (area)
                                overrideLiquid = LiquidType(area->LiquidTypeOverride[liquidFlagType]);
                        }

                        if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(overrideLiquid))
                        {
                            liquid_type = overrideLiquid;
                            liquidFlagType = liq->GetType();
                        }
                    }
                }

                data->level = liquid_level;
                data->depth_level = ground_level;
                data->entry = liquid_type;
                data->type_flags = GetLiquidFlagsFromType(liquidFlagType);
            }

            float delta = liquid_level - z;

            // Get position delta
            if (delta > 2.0f)                   // Under water
                return LIQUID_MAP_UNDER_WATER;
            if (delta > 0.0f)                   // In water
                return LIQUID_MAP_IN_WATER;
            if (delta > -0.1f)                   // Walk on water
                return LIQUID_MAP_WATER_WALK;
            result = LIQUID_MAP_ABOVE_WATER;
        }
    }

    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
    {
        LiquidData map_data;
        ZLiquidStatus map_result = gmap->GetLiquidStatus(x, y, z, reqLiquidTypeMask, &map_data);
        // Not override LIQUID_MAP_ABOVE_WATER with LIQUID_MAP_NO_WATER:
        if (map_result != LIQUID_MAP_NO_WATER && (map_data.level > ground_level))
        {
            if (data)
            {
                *data = map_data;
            }
            return map_result;
        }
    }
    return result;
}

void Map::GetFullTerrainStatusForPosition(float x, float y, float z, PositionFullTerrainStatus& data, uint8 reqLiquidType) const
{
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    VMAP::AreaAndLiquidData vmapData;
    vmgr->getAreaAndLiquidData(GetId(), x, y, z, reqLiquidType, vmapData);
    if (vmapData.areaInfo)
        data.areaInfo = boost::in_place(vmapData.areaInfo->adtId, vmapData.areaInfo->rootId, vmapData.areaInfo->groupId, vmapData.areaInfo->mogpFlags);

    GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y);
    float mapHeight = gmap->getHeight(x, y);

    // area lookup
    AreaTableEntry const* areaEntry = nullptr;
    if (vmapData.areaInfo && (z + 2.0f <= mapHeight || mapHeight <= vmapData.floorZ))
        if (WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTripple(vmapData.areaInfo->rootId, vmapData.areaInfo->adtId, vmapData.areaInfo->groupId))
            areaEntry = sAreaTableStore.LookupEntry(wmoEntry->areaId);

    if (areaEntry)
    {
        data.floorZ = vmapData.floorZ;
        data.areaId = areaEntry->ID;
    }
    else
    {
        data.floorZ = mapHeight;
        if (gmap)
            data.areaId = gmap->getArea(x, y);
        else
            data.areaId = 0;

        if (!data.areaId)
            data.areaId = i_mapEntry->linked_zone;

        if (data.areaId)
            areaEntry = sAreaTableStore.LookupEntry(data.areaId);
    }

    // liquid processing
    data.liquidStatus = LIQUID_MAP_NO_WATER;
    if (vmapData.liquidInfo && vmapData.liquidInfo->level > vmapData.floorZ && z + 2.0f > vmapData.floorZ)
    {
        uint32 liquidType = vmapData.liquidInfo->type;
#ifdef LICH_KING
        if (GetId() == 530 && liquidType == 2) // gotta love blizzard hacks
            liquidType = 15;
#endif

        uint32 liquidFlagType = 0;
        if (LiquidTypeEntry const* liquidData = sLiquidTypeStore.LookupEntry(liquidType))
            liquidFlagType = liquidData->GetType();

        if (liquidType && liquidType < LIQUID_TYPE_NAXXRAMAS_SLIME && areaEntry)
        {
            uint32 overrideLiquid = areaEntry->LiquidTypeOverride[liquidFlagType];
            if (!overrideLiquid && areaEntry->zone)
            {
                AreaTableEntry const* zoneEntry = sAreaTableStore.LookupEntry(areaEntry->zone);
                if (zoneEntry)
                    overrideLiquid = zoneEntry->LiquidTypeOverride[liquidFlagType];
            }

            if (LiquidTypeEntry const* overrideData = sLiquidTypeStore.LookupEntry(overrideLiquid))
            {
                liquidType = overrideLiquid;
                liquidFlagType = overrideData->GetType();
            }
        }

        data.liquidInfo = boost::in_place();
        data.liquidInfo->level = vmapData.liquidInfo->level;
        data.liquidInfo->depth_level = vmapData.floorZ;
        data.liquidInfo->entry = liquidType;
        data.liquidInfo->type_flags = GetLiquidFlagsFromType(liquidFlagType);

        float delta = vmapData.liquidInfo->level - z;
        if (delta > 2.0f)
            data.liquidStatus = LIQUID_MAP_UNDER_WATER;
        else if (delta > 0.0f)
            data.liquidStatus = LIQUID_MAP_IN_WATER;
        else if (delta > -0.1f)
            data.liquidStatus = LIQUID_MAP_WATER_WALK;
        else
            data.liquidStatus = LIQUID_MAP_ABOVE_WATER;
    }
    // look up liquid data from grid map
    if (gmap && (data.liquidStatus == LIQUID_MAP_ABOVE_WATER || data.liquidStatus == LIQUID_MAP_NO_WATER))
    {
        LiquidData gridMapLiquid;
        ZLiquidStatus gridMapStatus = gmap->GetLiquidStatus(x, y, z, reqLiquidType, &gridMapLiquid);
        if (gridMapStatus != LIQUID_MAP_NO_WATER && (gridMapLiquid.level > vmapData.floorZ))
        {
#ifdef LICH_KING
            if (GetId() == 530 && gridMapLiquid.entry == 2)
                gridMapLiquid.entry = 15;
#endif
            data.liquidInfo = gridMapLiquid;
            data.liquidStatus = gridMapStatus;
        }
    }
}


float Map::GetWaterLevel(float x, float y) const
{
    if (GridMap* gmap = const_cast<Map*>(this)->GetGrid(x, y))
        return gmap->getLiquidLevel(x, y);
    else
        return 0;
}

uint32 Map::GetAreaId(float x, float y, float z) const
{
    return GetAreaId(x, y, z, nullptr);
}

uint32 Map::GetZoneId(float x, float y, float z) const
{
    uint32 areaId = GetAreaId(x, y, z);
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId))
        if (area->zone)
            return area->zone;

    return areaId;
}

void Map::GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const
{
    areaid = zoneid = GetAreaId(x, y, z);
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaid))
        if (area->zone)
            zoneid = area->zone;
}


bool Map::isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, uint32 phasemask, VMAP::ModelIgnoreFlags ignoreFlags) const
{
    return VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(GetId(), x1, y1, z1, x2, y2, z2, ignoreFlags)
            && _dynamicTree.isInLineOfSight(x1, y1, z1, x2, y2, z2, phasemask);
}

bool Map::IsInWater(float x, float y, float pZ, LiquidData *data) const
{
    LiquidData liquid_status;
    LiquidData *liquid_ptr = data ? data : &liquid_status;
    return (GetLiquidStatus(x, y, pZ, MAP_ALL_LIQUIDS, liquid_ptr) & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)) != 0;
}

bool Map::IsUnderWater(float x, float y, float z) const
{
    return (GetLiquidStatus(x, y, z, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN) & LIQUID_MAP_UNDER_WATER) != 0;
}

bool Map::CheckGridIntegrity(Creature* c, bool moved) const
{
    Cell const& cur_cell = c->GetCurrentCell();

    CellCoord xy_val = Trinity::ComputeCellCoord(c->GetPositionX(), c->GetPositionY());
    Cell xy_cell(xy_val);
    if(xy_cell != cur_cell)
    {
        TC_LOG_DEBUG("maps","Creature (GUIDLow: %u) X: %f Y: %f (%s) in grid[%u,%u]cell[%u,%u] instead grid[%u,%u]cell[%u,%u]",
            c->GetGUIDLow(),
            c->GetPositionX(),c->GetPositionY(),(moved ? "final" : "original"),
            cur_cell.GridX(), cur_cell.GridY(), cur_cell.CellX(), cur_cell.CellY(),
            xy_cell.GridX(),  xy_cell.GridY(),  xy_cell.CellX(),  xy_cell.CellY());
        return true;                                        // not crash at error, just output error in debug mode
    }

    return true;
}

bool Map::CheckGridIntegrity(GameObject* go, bool moved) const
{
    Cell const& cur_cell = go->GetCurrentCell();

    CellCoord xy_val = Trinity::ComputeCellCoord(go->GetPositionX(), go->GetPositionY());
    Cell xy_cell(xy_val);
    if(xy_cell != cur_cell)
    {
        TC_LOG_DEBUG("maps","GameObject (GUIDLow: %u) X: %f Y: %f (%s) in grid[%u,%u]cell[%u,%u] instead grid[%u,%u]cell[%u,%u]",
            go->GetGUIDLow(),
            go->GetPositionX(),go->GetPositionY(),(moved ? "final" : "original"),
            cur_cell.GridX(), cur_cell.GridY(), cur_cell.CellX(), cur_cell.CellY(),
            xy_cell.GridX(),  xy_cell.GridY(),  xy_cell.CellX(),  xy_cell.CellY());
        return true;                                        // not crash at error, just output error in debug mode
    }

    return true;
}

const char* Map::GetMapName() const
{
    return i_mapEntry ? i_mapEntry->name[sWorld->GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";
}

void Map::SendInitSelf( Player * player)
{
    TC_LOG_DEBUG("maps","Creating player data for himself %u", player->GetGUIDLow());

    UpdateData data;

    bool hasTransport = false;

    // attach to player data current transport data
    if(Transport* transport = player->GetTransport())
    {
        hasTransport = true;
        transport->BuildCreateUpdateBlockForPlayer(&data, player);
    }

    // build data for self presence in world at own client (one time for map)
    player->BuildCreateUpdateBlockForPlayer(&data, player);

    // build other passengers at transport also (they always visible and marked as visible and will not send at visibility update at add to map
    if(Transport* transport = player->GetTransport())
        for(auto itr : transport->GetPassengers())
            if(player!=itr && player->HaveAtClient(itr))
            {
                hasTransport = true;
                itr->BuildCreateUpdateBlockForPlayer(&data, player);
            }

    WorldPacket packet;
    data.BuildPacket(&packet, player->GetSession()->GetClientBuild(), hasTransport);
    player->SendDirectMessage(&packet);
}

void Map::SendInitTransports( Player * player)
{
    // Hack to send out transports
    UpdateData transData;
    bool hasTransport = false;
    for (auto _transport : _transports)
        if (_transport != player->GetTransport())
            {
                _transport->BuildCreateUpdateBlockForPlayer(&transData, player);
                hasTransport = true;
            }

    WorldPacket packet;
    transData.BuildPacket(&packet, player->GetSession()->GetClientBuild(), hasTransport);
    player->SendDirectMessage(&packet);
}

void Map::SendRemoveTransports(Player* player)
{
    // Hack to send out transports
    UpdateData transData;
    for (auto _transport : _transports)
        if (_transport != player->GetTransport())
            _transport->BuildOutOfRangeUpdateBlock(&transData);

    // sunwell: remove static transports from client
    for (auto it = player->m_clientGUIDs.begin(); it != player->m_clientGUIDs.end(); )
    {
        if (IS_TRANSPORT(*it))
        {
            transData.AddOutOfRangeGUID(*it);
            it = player->m_clientGUIDs.erase(it);
        }
        else
            ++it;
    }

    WorldPacket packet;
    transData.BuildPacket(&packet, player->GetSession()->GetClientBuild());
    player->SendDirectMessage(&packet);
}


inline void Map::setNGrid(NGridType *grid, uint32 x, uint32 y)
{
    if(x >= MAX_NUMBER_OF_GRIDS || y >= MAX_NUMBER_OF_GRIDS)
    {
        TC_LOG_ERROR("maps","map::setNGrid() Invalid grid coordinates found: %d, %d!",x,y);
        ABORT();
    }
    i_grids[x][y] = grid;
}

void Map::SendObjectUpdates()
{
    //build updates for each objects
    UpdateDataMapType update_players; //one UpdateData object per player, containing updates for all objects
    UpdatePlayerSet player_set; //only there for performance, avoid recreating it at each BuildUpdate call

    while (!_updateObjects.empty())
    {
        Object* obj = *_updateObjects.begin();
        ASSERT(obj->IsInWorld());

        _updateObjects.erase(_updateObjects.begin());
        obj->BuildUpdate(update_players, player_set);
    }

    WorldPacket packet;                                     // here we allocate a std::vector with a size of 0x10000
    for (auto & update_player : update_players)
    {
        update_player.second.BuildPacket(&packet, update_player.first->GetSession()->GetClientBuild());
        update_player.first->GetSession()->SendPacket(&packet);
        packet.clear();                                     // clean the string
    }
}

void Map::DelayedUpdate(const uint32 t_diff)
{
    for (_transportsUpdateIter = _transports.begin(); _transportsUpdateIter != _transports.end();)
    {
        MotionTransport* transport = *_transportsUpdateIter;
        ++_transportsUpdateIter;

        if (!transport->IsInWorld())
            continue;

        transport->DelayedUpdate(t_diff);
    }

    RemoveAllObjectsInRemoveList();

    // Don't unload grids if it's battleground, since we may have manually added GOs, creatures, those doesn't load from DB at grid re-load !
    // This isn't really bother us, since as soon as we have instanced BG-s, the whole map unloads as the BG gets ended
    if (!IsBattlegroundOrArena())
    {
        for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end();)
        {
            NGridType *grid = i->GetSource();
            GridInfo* info = i->GetSource()->getGridInfoRef();
            ++i;                                                // The update might delete the map and we need the next map before the iterator gets invalid
            ASSERT(grid->GetGridState() >= 0 && grid->GetGridState() < MAX_GRID_STATE);
            si_GridStates[grid->GetGridState()]->Update(*this, *grid, *info, t_diff);
        }
    }
}

void Map::AddObjectToRemoveList(WorldObject *obj)
{
    assert(obj->GetMapId()==GetId() && obj->GetInstanceId()==GetInstanceId());

    obj->CleanupsBeforeDelete(false); 

    i_objectsToRemove.insert(obj);
    //TC_LOG_DEBUG("maps","Object (GUID: %u TypeId: %u) added to removing list.",obj->GetGUIDLow(),obj->GetTypeId());
}

void Map::AddObjectToSwitchList(WorldObject *obj, bool on)
{
    assert(obj->GetMapId()==GetId() && obj->GetInstanceId()==GetInstanceId());

    auto itr = i_objectsToSwitch.find(obj);
    if(itr == i_objectsToSwitch.end())
        i_objectsToSwitch.insert(itr, std::make_pair(obj, on));
    else if(itr->second != on)
        i_objectsToSwitch.erase(itr);
    else
        ABORT();
}

void Map::RemoveAllObjectsInRemoveList()
{
    while(!i_objectsToSwitch.empty())
    {
        auto itr = i_objectsToSwitch.begin();
        WorldObject *obj = itr->first;
        bool on = itr->second;
        i_objectsToSwitch.erase(itr);

        if (!obj->IsPermanentWorldObject())
        { 
            switch(obj->GetTypeId())
            {
            case TYPEID_UNIT:
                if(!(obj->ToCreature())->IsPet())
                    SwitchGridContainers(obj->ToCreature(), on);
                break;
            case TYPEID_GAMEOBJECT:
                SwitchGridContainers<GameObject>(obj->ToGameObject(), on);
                break;
            default:
                break;
            }
        }
    }

    //TC_LOG_DEBUG("maps","Object remover 1 check.");
    while(!i_objectsToRemove.empty())
    {
        auto itr = i_objectsToRemove.begin();
        WorldObject* obj = *itr;

        switch(obj->GetTypeId())
        {
        case TYPEID_CORPSE:
        {
            Corpse* corpse = ObjectAccessor::GetCorpse(*obj, obj->GetGUID());
            if (!corpse)
                TC_LOG_ERROR("maps","Try delete corpse/bones %u that not in map", obj->GetGUIDLow());
            else
                RemoveFromMap(corpse,true);
            break;
        }
        case TYPEID_DYNAMICOBJECT:
            RemoveFromMap((DynamicObject*)obj,true);
            break;
        case TYPEID_GAMEOBJECT:
        {
            GameObject* go = obj->ToGameObject();
            if (MotionTransport* transport = go->ToMotionTransport())
                RemoveFromMap(transport, true);
            else
                RemoveFromMap(go, true);
            break;
        }
        case TYPEID_UNIT:
            // HACK in case triggered sequence some spell can continue casting after prev CleanupsBeforeDelete call
            // make sure that like sources auras/etc removed before destructor start
            (obj->ToCreature())->CleanupsBeforeDelete();
            RemoveFromMap(obj->ToCreature(),true);
            break;
        default:
            TC_LOG_ERROR("maps","Non-grid object (TypeId: %u) in grid object removing list, ignored.",obj->GetTypeId());
            break;
        }

        i_objectsToRemove.erase(itr);
    }
    //TC_LOG_DEBUG("maps","Object remover 2 check.");
}

uint32 Map::GetPlayersCountExceptGMs() const
{
    uint32 count = 0;
    for(const auto & itr : m_mapRefManager)
        if(!itr.GetSource()->IsGameMaster())
            ++count;
    return count;
}

void Map::SendToPlayers(WorldPacket* data) const
{
    for(const auto & itr : m_mapRefManager)
        itr.GetSource()->SendDirectMessage(data);
}

bool Map::ActiveObjectsNearGrid(NGridType const& ngrid) const
{
    CellCoord cell_min(ngrid.getX() * MAX_NUMBER_OF_CELLS, ngrid.getY() * MAX_NUMBER_OF_CELLS);
    CellCoord cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord + MAX_NUMBER_OF_CELLS);

    //we must find visible range in cells so we unload only non-visible cells...
    float viewDist = GetVisibilityRange();
    int cell_range = (int)ceilf(viewDist / SIZE_OF_GRID_CELL) + 1;

    cell_min << cell_range;
    cell_min -= cell_range;
    cell_max >> cell_range;
    cell_max += cell_range;

    for(const auto & iter : m_mapRefManager)
    {
        Player* plr = iter.GetSource();

        CellCoord p = Trinity::ComputeCellCoord(plr->GetPositionX(), plr->GetPositionY());
        if( (cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
            (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    for(auto obj : m_activeForcedNonPlayers)
    {
        CellCoord p = Trinity::ComputeCellCoord(obj->GetPositionX(), obj->GetPositionY());
        if( (cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
            (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    return false;
}

void Map::AddToForceActive( Creature* c)
{
    AddToForceActiveHelper(c);

    // also not allow unloading spawn grid to prevent creating creature clone at load
    if(!c->IsPet() && c->GetSpawnId())
    {
        float x,y,z;
        c->GetRespawnPosition(x,y,z);
        GridCoord p = Trinity::ComputeGridCoord(x, y);
        if(!getNGrid(p.x_coord, p.y_coord))
        {
            GridCoord p2 = Trinity::ComputeGridCoord(c->GetPositionX(), c->GetPositionY());
            TC_LOG_ERROR("maps","Active creature (GUID: %u Entry: %u) added to grid[%u,%u] but spawn grid[%u,%u] not loaded.",
                c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
        }
    }
}

void Map::RemoveFromForceActive( Creature* c)
{
    RemoveFromForceActiveHelper(c);

    // also allow unloading spawn grid
    if(!c->IsPet() && c->GetSpawnId())
    {
        float x,y,z;
        c->GetRespawnPosition(x,y,z);
        GridCoord p = Trinity::ComputeGridCoord(x, y);
        if(!getNGrid(p.x_coord, p.y_coord))
        {
            GridCoord p2 = Trinity::ComputeGridCoord(c->GetPositionX(), c->GetPositionY());
            TC_LOG_ERROR("maps","Active creature (GUID: %u Entry: %u) removed from grid[%u,%u] but spawn grid[%u,%u] not loaded.",
                c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
        }
    }
}

Player* Map::GetPlayer(ObjectGuid const& guid)
{
    return ObjectAccessor::GetPlayer(this, guid);
}

Corpse* Map::GetCorpse(ObjectGuid const& guid)
{
    return _objectsStore.Find<Corpse>(guid);
}

Creature* Map::GetCreature(ObjectGuid guid)
{
    return _objectsStore.Find<Creature>(guid);
}

GameObject* Map::GetGameObject(ObjectGuid const& guid)
{
    return _objectsStore.Find<GameObject>(guid);
}

Pet* Map::GetPet(ObjectGuid const& guid)
{
    return _objectsStore.Find<Pet>(guid);
}

DynamicObject* Map::GetDynamicObject(ObjectGuid const& guid)
{
    return _objectsStore.Find<DynamicObject>(guid);
}

Transport* Map::GetTransport(ObjectGuid const& guid)
{
    if (!guid.IsMOTransport())
        return NULL;

    GameObject* go = GetGameObject(guid);
    return go ? go->ToTransport() : NULL;
}

void Map::UpdateIteratorBack(Player* player)
{
    if (m_mapRefIter == player->GetMapRef())
        m_mapRefIter = m_mapRefIter->nocheck_prev();
}

WorldObject* Map::GetWorldObject(ObjectGuid const& guid)
{
    switch (guid.GetHigh())
    {
    case HighGuid::Player:        return GetPlayer(guid);
    case HighGuid::Transport:
    case HighGuid::Mo_Transport:
    case HighGuid::GameObject:    return GetGameObject(guid);
    case HighGuid::Vehicle:
    case HighGuid::Unit:          return GetCreature( guid);
    case HighGuid::Pet:           return GetPet(guid);
    case HighGuid::DynamicObject: return GetDynamicObject(guid);
    case HighGuid::Corpse:        return GetCorpse( guid);
    default:                      return nullptr;
    }
}

Creature* Map::GetCreatureWithSpawnId(uint32 spawnId)
{
    auto creatureBounds = GetCreatureBySpawnIdStore().equal_range(spawnId);
    for (auto it = creatureBounds.first; it != creatureBounds.second; ++it)
    {
        auto c = it->second;

        if (c->GetMapId() != GetId())
            continue;

        if (c->GetInstanceId() != GetInstanceId())
            continue;

        if (c->GetSpawnId() != spawnId)
            continue;

        return c;
    }

    return nullptr;
}

void Map::AddCreatureToPool(Creature *cre, uint32 poolId)
{
    auto itr = m_cpmembers.find(poolId);
    if (itr == m_cpmembers.end()) {
        std::set<uint64> newSet;
        newSet.insert(cre->GetGUID());
        m_cpmembers[poolId] = newSet;
    } else {
        itr->second.insert(cre->GetGUID());
    }
}

void Map::RemoveCreatureFromPool(Creature *cre, uint32 poolId)
{
    auto itr = m_cpmembers.find(poolId);
    if (itr != m_cpmembers.end()) {
        std::set<uint64> membersSet = itr->second;
        auto itr = membersSet.find(cre->GetGUID());
        if(itr != membersSet.end())
        {
            cre->SetCreaturePoolId(0);
            membersSet.erase(itr);
            return;
        }
        TC_LOG_ERROR("maps","Creature %u could not be removed from pool %u", cre->GetSpawnId(), poolId);
    } else {
        TC_LOG_ERROR("maps","Pool %u not found for creature %u", poolId, cre->GetSpawnId());
    }
}

std::list<Creature*> Map::GetAllCreaturesFromPool(uint32 poolId)
{
    std::list<Creature*> creatureList;

    auto itr = m_cpmembers.find(poolId);
    if (itr != m_cpmembers.end())
    {
        for(auto guid : itr->second)
        {
            Creature* c = GetCreature(guid);
            if(c)
                creatureList.push_back(c);
            else
                TC_LOG_ERROR("maps", "GetAllCreaturesFromPool : couldn't get unit with guid " UI64FMTD, guid);
        }
    }

    return creatureList;
}

bool Map::AllTransportsEmpty() const
{
    for (auto _transport : _transports)
        if (!_transport->GetPassengers().empty())
            return false;

    return true;
}

void Map::AllTransportsRemovePassengers()
{
    for (auto _transport : _transports)
        while (!_transport->GetPassengers().empty())
            _transport->RemovePassenger(*(_transport->GetPassengers().begin()), true);
}

template TC_GAME_API bool Map::AddToMap(Corpse *, bool);
template TC_GAME_API bool Map::AddToMap(Creature *, bool);
template TC_GAME_API bool Map::AddToMap(GameObject *, bool);
template TC_GAME_API bool Map::AddToMap(DynamicObject *, bool);

template TC_GAME_API void Map::RemoveFromMap(Corpse *,bool);
template TC_GAME_API void Map::RemoveFromMap(Creature *,bool);
template TC_GAME_API void Map::RemoveFromMap(GameObject *, bool);
template TC_GAME_API void Map::RemoveFromMap(DynamicObject *, bool);

/* ******* Dungeon Instance Maps ******* */

InstanceMap::InstanceMap(uint32 id, time_t expiry, uint32 instanceId, uint8 spawnMode, Map* _parent)
  : Map(MAP_TYPE_INSTANCE_MAP, id, expiry, instanceId, spawnMode, _parent), i_data(nullptr),
    m_resetAfterUnload(false), m_unloadWhenEmpty(false)
{
    //lets initialize visibility distance for dungeons
    InstanceMap::InitVisibilityDistance();

    // the timer is started by default, and stopped when the first player joins
    // this make sure it gets unloaded if for some reason no player joins
    m_unloadTimer = std::max(sWorld->getConfig(CONFIG_INSTANCE_UNLOAD_DELAY), (uint32)MIN_UNLOAD_DELAY);
}

TestMap::TestMap(uint32 id, uint32 instanceId, uint8 spawnMode, Map* _parent, bool enableMapObjects)
    : InstanceMap(id, 0, instanceId, spawnMode, _parent)
{
    i_mapType = MAP_TYPE_TEST_MAP;
    m_unloadTimer = 0; //disable unload for test maps
    m_unloadWhenEmpty = true;
    m_disableMapObjects = !enableMapObjects;
}

InstanceMap::~InstanceMap()
{
    if(i_data)
    {
        delete i_data;
        i_data = nullptr;
    }
    
    // unload instance specific navigation data
    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(GetId(), GetInstanceId());
}

float InstanceMap::GetDefaultVisibilityDistance() const
{
    return World::GetMaxVisibleDistanceInInstances();
}

float InstanceMap::GetVisibilityNotifierPeriod() const
{
    return World::GetVisibilityNotifyPeriodInInstances();
}

/*
    Do map specific checks to see if the player can enter
*/
Map::EnterState InstanceMap::CannotEnter(Player* player)
{
    if(player->GetMapRef().getTarget() == this)
    {
        TC_LOG_ERROR("maps","InstanceMap::CanEnter - player %s(%u) already in map %d,%d,%d!", player->GetName().c_str(), player->GetGUIDLow(), GetId(), GetInstanceId(), GetSpawnMode());
        ABORT();
        // return CANNOT_ENTER_ALREADY_IN_MAP;
    }
    
    // cannot enter if the instance is full (player cap), GMs don't count
    InstanceTemplate const* instanceTemplate = sObjectMgr->GetInstanceTemplate(GetId());
    if (instanceTemplate && !player->IsGameMaster() && GetPlayersCountExceptGMs() >= instanceTemplate->maxPlayers)
    {
        //TC_LOG_DEBUG("maps","MAP: Instance '%u' of map '%s' cannot have more than '%u' players. Player '%s' rejected", GetInstanceId(), GetMapName(), iTemplate->maxPlayers, player->GetName());
        return CANNOT_ENTER_MAX_PLAYERS;
    }

    // cannot enter while players in the instance are in combat
    Group *pGroup = player->GetGroup();
    if(!player->IsGameMaster() && pGroup && pGroup->InCombatToInstance(GetInstanceId()) && player->GetMapId() != GetId())
        return CANNOT_ENTER_ZONE_IN_COMBAT;

    // cannot enter if player is permanent saved to a different instance id
    if (InstancePlayerBind* playerBind = player->GetBoundInstance(GetId(), GetDifficulty()))
        if (playerBind->perm && playerBind->save)
            if (playerBind->save->GetInstanceId() != GetInstanceId())
                return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

    return Map::CannotEnter(player);
}

/*
    Do map specific checks and add the player to the map if successful.
*/
bool InstanceMap::AddPlayerToMap(Player *player)
{
    // TODO: Not sure about checking player level: already done in HandleAreaTriggerOpcode
    // GMs still can teleport player in instance.
    // Is it needed?

    {
        std::lock_guard<std::mutex> lock(_mapLock);
        // Check moved to void WorldSession::HandleMoveWorldportAckOpcode()
        //if (!CanEnter(player))
            //return false;

        // get or create an instance save for the map
        InstanceSave *mapSave = sInstanceSaveMgr->GetInstanceSave(GetInstanceId());
        if(!mapSave)
        {
            TC_LOG_DEBUG("maps","InstanceMap::Add: creating instance save for map %d difficulty %d with instance id %d", GetId(), GetDifficulty(), GetInstanceId());
            mapSave = sInstanceSaveMgr->AddInstanceSave(GetId(), GetInstanceId(), GetDifficulty(), 0, true);
        }

        // check for existing instance binds
        InstancePlayerBind *playerBind = player->GetBoundInstance(GetId(), GetDifficulty());
        if(playerBind && playerBind->perm)
        {
            // cannot enter other instances if bound permanently
            if(playerBind->save != mapSave)
            {
                //TC_LOG_ERROR("maps","InstanceMap::Add: player %s(%d) is permanently bound to instance %d,%d,%d,%d,%d,%d but he is being put in instance %d,%d,%d,%d,%d,%d", player->GetName(), player->GetGUIDLow(), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficulty(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficulty(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset());
                ABORT();
            }
        }
        else
        {
            Group *pGroup = player->GetGroup();
            if(pGroup)
            {
                // solo saves should be reset when entering a group
                InstanceGroupBind *groupBind = pGroup->GetBoundInstance(GetDifficulty(), GetId());
                if(playerBind)
                {
                  //  TC_LOG_ERROR("maps","InstanceMap::Add: player %s(%d) is being put in instance %d,%d,%d,%d,%d,%d but he is in group %d and is bound to instance %d,%d,%d,%d,%d,%d!", player->GetName(), player->GetGUIDLow(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficulty(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset(), GUID_LOPART(pGroup->GetLeaderGUID()), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficulty(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset());
                    if(groupBind) 
                        TC_LOG_ERROR("maps","InstanceMap::Add: the group is bound to instance %d,%d,%d,%d,%d,%d", groupBind->save->GetMapId(), groupBind->save->GetInstanceId(), groupBind->save->GetDifficulty(), groupBind->save->GetPlayerCount(), groupBind->save->GetGroupCount(), groupBind->save->CanReset());

                    TC_LOG_ERROR("maps","InstanceMap::Add: do not let player %s enter instance otherwise crash will happen", player->GetName().c_str());
                    return false;
                    //player->UnbindInstance(GetId(), GetSpawnMode());
                    //ABORT();
                }
                // bind to the group or keep using the group save
                if(!groupBind)
                    pGroup->BindToInstance(mapSave, false);
                else
                {
                    // cannot jump to a different instance without resetting it
                    if(groupBind->save != mapSave)
                    {
//                        TC_LOG_ERROR("maps","InstanceMap::Add: player %s(%d) is being put in instance %d,%d,%d but he is in group %d which is bound to instance %d,%d,%d!", player->GetName(), player->GetGUIDLow(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficulty(), GUID_LOPART(pGroup->GetLeaderGUID()), groupBind->save->GetMapId(), groupBind->save->GetInstanceId(), groupBind->save->GetDifficulty());
                        if(mapSave)
                            TC_LOG_ERROR("maps","MapSave players: %d, group count: %d", mapSave->GetPlayerCount(), mapSave->GetGroupCount());
                        else
                            TC_LOG_ERROR("maps","MapSave NULL");
                        if(groupBind->save)
                            TC_LOG_ERROR("maps","GroupBind save players: %d, group count: %d", groupBind->save->GetPlayerCount(), groupBind->save->GetGroupCount());
                        else
                            TC_LOG_ERROR("maps","GroupBind save NULL");
                        ABORT();
                    }
                    // if the group/leader is permanently bound to the instance
                    // players also become permanently bound when they enter
                    if(groupBind->perm)
                    {
                        WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
                        data << uint32(0);
                        player->SendDirectMessage(&data);
                        player->BindToInstance(mapSave, true);
                    }
                }
            }
            else
            {
                // set up a solo bind or continue using it
                if(!playerBind)
                    player->BindToInstance(mapSave, false);
                else
                    // cannot jump to a different instance without resetting it
                    assert(playerBind->save == mapSave);
            }
        }

        if(i_data) i_data->OnPlayerEnter(player);
        // for normal instances cancel the reset schedule when the
        // first player enters (no players yet)
        SetResetSchedule(false);

        player->SendInitWorldStates();
        //TC_LOG_DEBUG("maps","MAP: Player '%s' entered the instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());
        // initialize unload state
        m_unloadTimer = 0;
        m_resetAfterUnload = false;
        m_unloadWhenEmpty = false;
    }
    
    // Remove auras that cannot be present in instance
    player->RemoveAurasWithCustomAttribute(SPELL_ATTR_CU_REMOVE_ON_INSTANCE_ENTER);

    // this will acquire the same mutex so it cannot be in the previous block
    Map::AddPlayerToMap(player);
    return true;
}

void InstanceMap::Update(const uint32& t_diff)
{
    Map::Update(t_diff);

    if(i_data)
        i_data->Update(t_diff);
}

void InstanceMap::RemovePlayerFromMap(Player *player, bool remove)
{
    //TC_LOG_DEBUG("maps","MAP: Removing player '%s' from instance '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
    //if last player set unload timer
    if(!m_unloadTimer && m_mapRefManager.getSize() == 1)
        m_unloadTimer = m_unloadWhenEmpty ? MIN_UNLOAD_DELAY : std::max(sWorld->getConfig(CONFIG_INSTANCE_UNLOAD_DELAY), (uint32)MIN_UNLOAD_DELAY);
    Map::RemovePlayerFromMap(player, remove);
    // for normal instances schedule the reset after all players have left
    SetResetSchedule(true);
}

inline GridMap *Map::GetGrid(float x, float y)
{
    // half opt method
    int gx=(int)(32-x/SIZE_OF_GRIDS);                       //grid x
    int gy=(int)(32-y/SIZE_OF_GRIDS);                       //grid y

    // ensure GridMap is loaded
    EnsureGridCreated(GridCoord(63-gx,63-gy));

    return GridMaps[gx][gy];
}

void InstanceMap::CreateInstanceScript(bool load)
{
    if(i_data != nullptr)
        return;

    InstanceTemplate const* mInstance = sObjectMgr->GetInstanceTemplate(GetId());
    if (mInstance)
    {
        i_script_id = mInstance->ScriptId;
        i_data = sScriptMgr->CreateInstanceScript(this);
    }

    if(!i_data)
        return;

    i_data->Initialize();

    if(load)
    {
        // TODO: make a global storage for this
        QueryResult result = CharacterDatabase.PQuery("SELECT data FROM instance WHERE map = '%u' AND id = '%u'", GetId(), i_InstanceId);
        if (result)
        {
            Field* fields = result->Fetch();
            const char* data = fields[0].GetCString();
            if(data)
                i_data->Load(data);
        }
    }
}

/*
    Returns true if there are no players in the instance
*/
bool InstanceMap::Reset(uint8 method)
{
    // note: since the map may not be loaded when the instance needs to be reset
    // the instance must be deleted from the DB by InstanceSaveManager

    if(HavePlayers())
    {
        if(method == INSTANCE_RESET_ALL)
        {
            // notify the players to leave the instance so it can be reset
            for(auto & itr : m_mapRefManager)
                itr.GetSource()->SendResetFailedNotify(GetId());
        }
        else
        {
            if(method == INSTANCE_RESET_GLOBAL)
            {
                // set the homebind timer for players inside (1 minute)
                for(auto & itr : m_mapRefManager)
                    itr.GetSource()->m_InstanceValid = false;
            }

            // the unload timer is not started
            // instead the map will unload immediately after the players have left
            m_unloadWhenEmpty = true;
            m_resetAfterUnload = true;
        }
    }
    else
    {
        // unloaded at next update
        m_unloadTimer = MIN_UNLOAD_DELAY;
        m_resetAfterUnload = true;
    }

    return m_mapRefManager.isEmpty();
}

std::string const& InstanceMap::GetScriptName() const
{
    return sObjectMgr->GetScriptName(i_script_id);
}

void InstanceMap::PermBindAllPlayers(Player *player)
{
    InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(GetInstanceId());
    if(!save)
    {
        TC_LOG_ERROR("maps","Cannot bind players, no instance save available for map!\n");
        return;
    }

    Group *group = player->GetGroup();
    // group members outside the instance group don't get bound
    for(auto & itr : m_mapRefManager)
    {
        Player* plr = itr.GetSource();
        // players inside an instance cannot be bound to other instances
        // some players may already be permanently bound, in this case nothing happens
        InstancePlayerBind *bind = plr->GetBoundInstance(save->GetMapId(), save->GetDifficulty());
        if(!bind || !bind->perm)
        {
            plr->BindToInstance(save, true);
            WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
            data << uint32(0);
            plr->SendDirectMessage(&data);
        }

        // if the leader is not in the instance the group will not get a perm bind
        if(group && group->GetLeaderGUID() == plr->GetGUID())
            group->BindToInstance(save, true);
    }
}

void Map::RemoveAllPlayers()
{
    if (HavePlayers())
    {
        for (auto & itr : m_mapRefManager)
        {
            Player* player = itr.GetSource();
            if (!player->IsBeingTeleportedFar())
            {
                TC_LOG_ERROR("maps", "Map::UnloadAll: player %s is still in map %u during unload, this should not happen!", player->GetName().c_str(), GetId());
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
            }
        }
    }
}

void TestMap::RemoveAllPlayers()
{
    if (HavePlayers())
        for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
            if (Player* player = itr->GetSource())
                if (!player->IsBeingTeleportedFar())
                    player->TeleportTo(player->m_recallMap, player->m_recallX, player->m_recallY, player->m_recallZ, player->m_recallO);
}

void TestMap::DisconnectAllBots()
{
#ifdef TESTS
    if (!HavePlayers())
        return;

    std::list<Player*> botsToRemove;
    for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
        if (Player* player = itr->GetSource())
            if (player->GetPlayerbotAI())
                botsToRemove.push_back(player);

    for (auto itr : botsToRemove)
        TestCase::_RemoveTestBot(itr);
#endif
}

Player* TestMap::GetFirstHumanPlayer()
{

#ifdef TESTS
    for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
        if (Player* player = itr->GetSource())
            if (!player->GetPlayerbotAI())
                return player;
#endif
    return nullptr;
}

void InstanceMap::UnloadAll()
{
    ASSERT(!HavePlayers());

    if (m_resetAfterUnload == true) 
    {
        sObjectMgr->DeleteRespawnTimeForInstance(GetInstanceId()); //DeleteRespawnTimes();
        DeleteCorpseData();
    }

    Map::UnloadAll();
}

void InstanceMap::HandleCrash()
{
    if (HavePlayers())
    {
        std::list<Player*> players;
        //dont teleport players in this loop at this will invalidate our iterator
        for (auto & itr : m_mapRefManager)
            players.push_back(itr.GetSource());

        for (auto plr : players)
        {
            bool tpResult = false;
            AreaTrigger const* at = sObjectMgr->GetGoBackTrigger(GetId());
            if (at)
                tpResult = plr->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, plr->GetOrientation());

            if(!tpResult)
                tpResult = plr->TeleportTo(plr->m_homebindMapId, plr->m_homebindX, plr->m_homebindY, plr->m_homebindZ, plr->GetOrientation());

            if (!tpResult) //just to be extra sure
                plr->m_kickatnextupdate = true;

            ChatHandler(plr).SendSysMessage(LANG_INSTANCE_CRASHED);
        }
    }

    if (m_resetAfterUnload == true)
        sObjectMgr->DeleteRespawnTimeForInstance(GetInstanceId());

}

void InstanceMap::SendResetWarnings(uint32 timeLeft) const
{
    for(const auto & itr : m_mapRefManager)
        itr.GetSource()->SendInstanceResetWarning(GetId(), timeLeft);
}

void InstanceMap::SetResetSchedule(bool on)
{
    // only for normal instances
    // the reset time is only scheduled when there are no payers inside
    // it is assumed that the reset time will rarely (if ever) change while the reset is scheduled
    if(!HavePlayers() && !IsRaid() && !IsHeroic())
    {
        InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(GetInstanceId());
        if(!save) 
            TC_LOG_ERROR("maps", "InstanceMap::SetResetSchedule: cannot turn schedule %s, no save available for instance %u of %u", on ? "on" : "off", GetInstanceId(), GetId());
        else 
            sInstanceSaveMgr->ScheduleReset(on, save->GetResetTime(), InstanceSaveManager::InstResetEvent(0, GetId(), GetDifficulty(), GetInstanceId()));
    }
}

MapDifficulty const* Map::GetMapDifficulty() const
{
    return GetMapDifficultyData(GetId(), GetDifficulty());
}

Difficulty Map::GetDifficulty() const
{ 
    return Difficulty(GetSpawnMode()); 
}

bool Map::IsRegularDifficulty() const
{ 
    return GetDifficulty() == REGULAR_DIFFICULTY; 
}

#ifdef LICH_KING
bool Map::IsHeroic() const 
{ 
    return IsRaid() ? i_spawnMode >= RAID_DIFFICULTY_10MAN_HEROIC : i_spawnMode >= DUNGEON_DIFFICULTY_HEROIC; 
}
#else
bool Map::IsHeroic() const 
{ 
    return i_spawnMode == DUNGEON_DIFFICULTY_HEROIC; 
}
#endif

/* ******* Battleground Instance Maps ******* */

BattlegroundMap::BattlegroundMap(uint32 id, time_t expiry, uint32 InstanceId, Map* parent)
  : Map(MAP_TYPE_BATTLEGROUND_MAP, id, expiry, InstanceId, REGULAR_DIFFICULTY, parent)
{
    //lets initialize visibility distance for BG/Arenas
    BattlegroundMap::InitVisibilityDistance();
}

BattlegroundMap::~BattlegroundMap()
{
    if (m_bg)
    {
        //unlink to prevent crash, always unlink all pointer reference before destruction
        m_bg->SetBgMap(NULL);
        m_bg = NULL;
    }
}

float BattlegroundMap::GetDefaultVisibilityDistance() const
{
    return World::GetMaxVisibleDistanceInBGArenas();
}

float BattlegroundMap::GetVisibilityNotifierPeriod() const
{
    return World::GetVisibilityNotifyPeriodInBGArenas();
}

Map::EnterState BattlegroundMap::CannotEnter(Player* player)
{
    if(player->GetMapRef().getTarget() == this)
    {
        TC_LOG_ERROR("maps","BGMap::CanEnter - player %u already in map!", player->GetGUIDLow());
        ABORT();
        // return CANNOT_ENTER_ALREADY_IN_MAP;
    }

    if(player->GetBattlegroundId() != GetInstanceId())
        return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

    // player number limit is checked in bgmgr, no need to do it here

    return Map::CannotEnter(player);
}

bool BattlegroundMap::AddPlayerToMap(Player * player)
{
    {
        std::lock_guard<std::mutex> lock(_mapLock);
        //Check moved to void WorldSession::HandleMoveWorldportAckOpcode()
        //if(!CanEnter(player))
          //  return false;
        // reset instance validity, battleground maps do not homebind
        player->m_InstanceValid = true;
    }
    return Map::AddPlayerToMap(player);
}

void BattlegroundMap::RemovePlayerFromMap(Player *player, bool remove)
{
    ASSERT(player);
    if (player->isSpectator() && !player->isSpectateCanceled())
    {
        if (GetBG())
            GetBG()->RemoveSpectator(player->GetGUID());

        if (player->isSpectator())
            player->SetSpectate(false);
    }

    //TC_LOG_DEBUG("maps","MAP: Removing player '%s' from bg '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
    Map::RemovePlayerFromMap(player, remove);
}

void BattlegroundMap::SetUnload()
{
    m_unloadTimer = MIN_UNLOAD_DELAY;
}

void BattlegroundMap::RemoveAllPlayers()
{
    if (HavePlayers())
        for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
            if (Player* player = itr->GetSource())
                if (!player->IsBeingTeleportedFar())
                    player->TeleportTo(player->GetBattlegroundEntryPoint());
}

void BattlegroundMap::HandleCrash()
{
    if (HavePlayers())
    {
        std::list<Player*> players;
        //dont teleport players in this loop at this will invalidate our iterator
        for (auto & itr : m_mapRefManager)
            players.push_back(itr.GetSource());

        for (auto plr : players)
        {
            bool tpResult = plr->TeleportTo(plr->GetBattlegroundEntryPoint());
            ChatHandler(plr).SendSysMessage(LANG_INSTANCE_CRASHED);
            if (!tpResult)
                plr->m_kickatnextupdate = true;
        }
    }
}

/*--------------------------TRINITY-------------------------*/

void Map::SendZoneDynamicInfo(Player* player)
{
    uint32 zoneId = GetZoneId(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
    ZoneDynamicInfoMap::const_iterator itr = _zoneDynamicInfo.find(zoneId);
    if (itr == _zoneDynamicInfo.end())
        return;

    if (itr->second.MusicId)
        player->GetSession()->SendPlayMusic(itr->second.MusicId);

    if (WeatherState weather = itr->second.WeatherId)
        player->GetSession()->SendWeather(weather, itr->second.WeatherGrade, 0);

    if (uint32 overrideLight = itr->second.OverrideLightId)
        player->GetSession()->SendOverrideLight(_defaultLight, overrideLight, itr->second.LightFadeInTime);
}

void Map::SetZoneMusic(uint32 zoneId, uint32 musicId)
{
    if (_zoneDynamicInfo.find(zoneId) == _zoneDynamicInfo.end())
        _zoneDynamicInfo.insert(ZoneDynamicInfoMap::value_type(zoneId, ZoneDynamicInfo()));

    _zoneDynamicInfo[zoneId].MusicId = musicId;

    Map::PlayerList const& players = GetPlayers();
    if (!players.isEmpty())
    {
        for (const auto & itr : players)
            if (Player* player = itr.GetSource())
                if (player->GetZoneId() == zoneId)
                    player->GetSession()->SendPlayMusic(musicId);
    }
}

void Map::SetZoneWeather(uint32 zoneId, WeatherState weatherId, float weatherGrade)
{
    if (_zoneDynamicInfo.find(zoneId) == _zoneDynamicInfo.end())
        _zoneDynamicInfo.insert(ZoneDynamicInfoMap::value_type(zoneId, ZoneDynamicInfo()));

    ZoneDynamicInfo& info = _zoneDynamicInfo[zoneId];
    info.WeatherId = weatherId;
    info.WeatherGrade = weatherGrade;
    Map::PlayerList const& players = GetPlayers();

    if (!players.isEmpty())
    {
        for (const auto & itr : players)
            if (Player* player = itr.GetSource())
                if (player->GetZoneId() == zoneId)
                    player->GetSession()->SendWeather(weatherId, weatherGrade, 0);
    }
}

void Map::SetZoneOverrideLight(uint32 zoneId, uint32 lightId, uint32 fadeInTime)
{
    if (_zoneDynamicInfo.find(zoneId) == _zoneDynamicInfo.end())
        _zoneDynamicInfo.insert(ZoneDynamicInfoMap::value_type(zoneId, ZoneDynamicInfo()));

    ZoneDynamicInfo& info = _zoneDynamicInfo[zoneId];
    info.OverrideLightId = lightId;
    info.LightFadeInTime = fadeInTime;
    Map::PlayerList const& players = GetPlayers();

    if (!players.isEmpty())
    {
        WorldPacket data(SMSG_OVERRIDE_LIGHT, 4 + 4 + 4);
        data << uint32(_defaultLight);
        data << uint32(lightId);
        data << uint32(fadeInTime);

        for (const auto & itr : players)
            if (Player* player = itr.GetSource())
                if (player->GetZoneId() == zoneId)
                    player->SendDirectMessage(&data);
    }
}

bool Map::IsGridLoaded(float x, float y) const
{
    GridCoord gp = Trinity::ComputeGridCoord(x,y);
    if((gp.x_coord >= MAX_NUMBER_OF_GRIDS) || (gp.y_coord >= MAX_NUMBER_OF_GRIDS))
        return false;

    return IsGridLoaded(gp);
}

//LK OK
void WorldSession::SendPlayMusic(uint32 musicId)
{
    WorldPacket data(SMSG_PLAY_MUSIC, 4);
    data << uint32(musicId);
    SendPacket(&data);
}

//LK OK
void WorldSession::SendWeather(WeatherState weatherId, float weatherGrade, uint8 unk)
{
    WorldPacket data(SMSG_WEATHER, 4 + 4 + 4);
    data << uint32(weatherId);
    data << float(weatherGrade);
    data << uint8(unk);
    SendPacket(&data);
}

//LK OK
void WorldSession::SendOverrideLight(uint32 defaultLightId, uint32 overrideLightId, uint32 fadeTime)
{
    WorldPacket data(SMSG_OVERRIDE_LIGHT, 4 + 4 + 4);
    data << uint32(defaultLightId);
    data << uint32(overrideLightId);
    data << uint32(fadeTime);
    SendPacket(&data);
}

bool Map::Instanceable() const { return i_mapEntry && i_mapEntry->Instanceable(); }
bool Map::IsDungeon() const { return i_mapEntry && i_mapEntry->IsDungeon(); }
bool Map::IsNonRaidDungeon() const { return i_mapEntry && i_mapEntry->IsNonRaidDungeon(); }
bool Map::IsRaid() const { return i_mapEntry && i_mapEntry->IsRaid(); }
bool Map::IsWorldMap() const { return i_mapEntry && i_mapEntry->IsWorldMap(); }

bool Map::IsBattleground() const { return i_mapEntry && i_mapEntry->IsBattleground(); }
bool Map::IsBattleArena() const { return i_mapEntry && i_mapEntry->IsBattleArena(); }
bool Map::IsBattlegroundOrArena() const { return i_mapEntry && i_mapEntry->IsBattlegroundOrArena(); }



void Map::LoadCorpseData()
{
    /*
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CORPSES);
    stmt->setUInt32(0, GetId());
    stmt->setUInt32(1, GetInstanceId());

    //        0     1     2     3            4      5          6          7       8       9        10     11        12    13          14          15         16
    // SELECT posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, guildId, flags, dynFlags, time, corpseType, instanceId, phaseMask, guid FROM corpse WHERE mapId = ? AND instanceId = ?
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        CorpseType type = CorpseType(fields[13].GetUInt8());
        ObjectGuid::LowType guid = fields[16].GetUInt32();
        if (type >= MAX_CORPSE_TYPE || type == CORPSE_BONES)
        {
            TC_LOG_ERROR("misc", "Corpse (guid: %u) have wrong corpse type (%u), not loading.", guid, type);
            continue;
        }

        Corpse* corpse = new Corpse(type);

        if (!corpse->LoadCorpseFromDB(GenerateLowGuid<HighGuid::Corpse>(), fields))
        {
            delete corpse;
            continue;
        }

        AddCorpse(corpse);

    } while (result->NextRow());
    */
}

void Map::DeleteCorpseData()
{
    // DELETE FROM corpse WHERE mapId = ? AND instanceId = ?
    /*
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CORPSES_FROM_MAP);
    stmt->setUInt32(0, GetId());
    stmt->setUInt32(1, GetInstanceId());
    CharacterDatabase.Execute(stmt);
    */
}

void Map::AddCorpse(Corpse* corpse)
{
    corpse->SetMap(this);

    _corpsesByCell[corpse->GetCellCoord().GetId()].insert(corpse);
    if (corpse->GetType() != CORPSE_BONES)
        _corpsesByPlayer[corpse->GetOwnerGUID()] = corpse;
    else
        _corpseBones.insert(corpse);
}

void Map::RemoveCorpse(Corpse* corpse)
{
    ASSERT(corpse);

    corpse->DestroyForNearbyPlayers();
    if (corpse->IsInGrid())
        RemoveFromMap(corpse, false);
    else
    {
        corpse->RemoveFromWorld();
        corpse->ResetMap();
    }

    _corpsesByCell[corpse->GetCellCoord().GetId()].erase(corpse);
    if (corpse->GetType() != CORPSE_BONES)
        _corpsesByPlayer.erase(corpse->GetOwnerGUID());
    else
        _corpseBones.erase(corpse);
}

Corpse* Map::ConvertCorpseToBones(ObjectGuid const& ownerGuid, bool insignia /*= false*/)
{
    Corpse* corpse = GetCorpseByPlayer(ownerGuid);
    if (!corpse)
        return nullptr;

    RemoveCorpse(corpse);

    // remove corpse from DB
    /*
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    corpse->DeleteFromDB(trans);
    CharacterDatabase.CommitTransaction(trans);
    */
    Corpse* bones = NULL;

    // create the bones only if the map and the grid is loaded at the corpse's location
    // ignore bones creating option in case insignia
    if ((insignia ||
        (IsBattlegroundOrArena() ? sWorld->getBoolConfig(CONFIG_DEATH_BONES_BG_OR_ARENA) : sWorld->getBoolConfig(CONFIG_DEATH_BONES_WORLD))) 
        && !IsRemovalGrid(corpse->GetPositionX(), corpse->GetPositionY())        )
    {
        // Create bones, don't change Corpse
        bones = new Corpse();
        bones->Create(ObjectGuid(corpse->GetGUID()).GetCounter());

        for (uint8 i = OBJECT_FIELD_TYPE + 1; i < CORPSE_END; ++i)                    // don't overwrite guid and object type
            bones->SetUInt32Value(i, corpse->GetUInt32Value(i));

        bones->SetCellCoord(corpse->GetCellCoord());
        bones->Relocate(corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetOrientation());
        bones->SetPhaseMask(corpse->GetPhaseMask(), false);

        bones->SetUInt32Value(CORPSE_FIELD_FLAGS, CORPSE_FLAG_UNK2 | CORPSE_FLAG_BONES);
        bones->SetGuidValue(CORPSE_FIELD_OWNER, ObjectGuid::Empty);

        for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
            if (corpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                bones->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);

        AddCorpse(bones);

        // add bones in grid store if grid loaded where corpse placed
        AddToMap(bones);
    }

    // all references to the corpse should be removed at this point
    delete corpse;

    return bones;
}

void Map::RemoveOldCorpses()
{
    time_t now = time(nullptr);

    std::vector<ObjectGuid> corpses;
    corpses.reserve(_corpsesByPlayer.size());

    for (auto const& p : _corpsesByPlayer)
        if (p.second->IsExpired(now))
            corpses.push_back(p.first);

    for (ObjectGuid const& ownerGuid : corpses)
        ConvertCorpseToBones(ownerGuid);

    std::vector<Corpse*> expiredBones;
    for (Corpse* bones : _corpseBones)
        if (bones->IsExpired(now))
            expiredBones.push_back(bones);

    for (Corpse* bones : expiredBones)
    {
        RemoveCorpse(bones);
        delete bones;
    }
}

TempSummon* Map::SummonCreature(uint32 entry, Position const& pos, SummonPropertiesEntry const* properties /*= nullptr*/, uint32 duration /*= 0*/, Unit* summoner /*= nullptr*/, uint32 spellId /*= 0*/)
{
    uint32 mask = UNIT_MASK_SUMMON;
    if (properties)
    {
        switch (properties->Category)
        {
        case SUMMON_CATEGORY_PET:
            mask = UNIT_MASK_GUARDIAN;
            break;
        case SUMMON_CATEGORY_PUPPET: //Only property 65 has this one. Spell such has : Eye of Kilrogg / Dream Vision / Using Steam Tonk Controller
            mask = UNIT_MASK_PUPPET;
            break;
        case SUMMON_CATEGORY_VEHICLE:
            mask = UNIT_MASK_MINION;
            break;
        case SUMMON_CATEGORY_WILD:
        case SUMMON_CATEGORY_ALLY:
        case SUMMON_CATEGORY_UNK:
        {
            switch (properties->Type)
            {
            case SUMMON_TYPE_MINION:
            case SUMMON_TYPE_GUARDIAN:
            case SUMMON_TYPE_GUARDIAN2:
                mask = UNIT_MASK_GUARDIAN;
                break;
            case SUMMON_TYPE_TOTEM:
            case SUMMON_TYPE_LIGHTWELL:
                mask = UNIT_MASK_TOTEM;
                break;
            case SUMMON_TYPE_VEHICLE:
            case SUMMON_TYPE_VEHICLE2:
                mask = UNIT_MASK_SUMMON;
                break;
            case SUMMON_TYPE_MINIPET:
                mask = UNIT_MASK_MINION;
                break;
            default:
#ifdef LICH_KING
                if (properties->Flags & 512) // Mirror Image, Summon Gargoyle
                    mask = UNIT_MASK_GUARDIAN;
#endif
                break;
            }
            break;
        }
        default:
            return NULL;
        }
    }

    uint32 phase = PHASEMASK_NORMAL;
    if (summoner)
        phase = summoner->GetPhaseMask();

    TempSummon* summon = NULL;
    switch (mask)
    {
    case UNIT_MASK_SUMMON:
        summon = new TempSummon(properties, summoner, false);
        break;
    case UNIT_MASK_GUARDIAN:
        summon = new Guardian(properties, summoner, false);
        break;
    case UNIT_MASK_PUPPET:
        summon = new Puppet(properties, summoner);
        break;
    case UNIT_MASK_TOTEM:
        summon = new Totem(properties, summoner);
        break;
    case UNIT_MASK_MINION:
        summon = new Minion(properties, summoner, false);
        break;
    }

    if (!summon->Create(GenerateLowGuid<HighGuid::Unit>(), this, phase, entry, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), nullptr/*, vehId*/))
    {
        delete summon;
        return NULL;
    }

    summon->SetUInt32Value(UNIT_CREATED_BY_SPELL, spellId);

    summon->SetHomePosition(pos);

    summon->InitStats(duration);
    AddToMap(summon->ToCreature());
    summon->InitSummon();

    // call MoveInLineOfSight for nearby creatures
    Trinity::AIRelocationNotifier notifier(*summon);
    Cell::VisitAllObjects(summon, notifier, GetVisibilityRange());

    return summon;
}

void Map::UnloadAll()
{
    // clear all delayed moves, useless anyway do this moves before map unload.
    _creaturesToMove.clear();
    _gameObjectsToMove.clear();

    for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end();)
    {
        NGridType &grid(*i->GetSource());
        ++i;
        UnloadGrid(grid, true);       // deletes the grid and removes it from the GridRefManager
    }

    // sunwell: crashfix, some npc can be left on transport (not a default passenger)
    if (!AllTransportsEmpty())
        AllTransportsRemovePassengers();

    _transportsUpdateIter = _transports.end();
    for (auto itr = _transports.begin(); itr != _transports.end();)
    {
        MotionTransport* transport = *itr;
        ++itr;

        RemoveFromMap(transport, true);
    }
    _transports.clear();

    for (auto& cellCorpsePair : _corpsesByCell)
    {
        for (Corpse* corpse : cellCorpsePair.second)
        {
            corpse->RemoveFromWorld();
            corpse->ResetMap();
            delete corpse;
        }
    }

    _corpsesByCell.clear();
    _corpsesByPlayer.clear();
    _corpseBones.clear();
}
