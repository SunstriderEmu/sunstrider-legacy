
#ifndef TRINITY_MAP_H
#define TRINITY_MAP_H

#include "Define.h"
#include "GridDefines.h"
#include "Cell.h"
#include "GridRefManager.h"
#include "MapRefManager.h"
#include "MPSCQueue.h"
#include "DynamicTree.h"
#include "Models/GameObjectModel.h"
#include <boost/heap/fibonacci_heap.hpp>
#include "ObjectGuid.h"
#include "SpawnData.h"
#include "Transaction.h"
#include "SharedDefines.h"

#include <bitset>
#include <list>
#include <mutex>

class Unit;
class WorldPacket;
class InstanceScript;
class WorldObject;
class CreatureGroup;
struct ScriptInfo;
class Battleground;
class GridMap;
class Transport;
class MotionTransport;
namespace Trinity { struct ObjectUpdater; }
namespace VMAP { enum class ModelIgnoreFlags : uint32; }
struct MapDifficulty;
struct MapEntry;
enum Difficulty : uint8;
class BattlegroundMap;
class InstanceMap;
class MapInstanced;
enum WeatherState : int;
class Object;
class TempSummon;
struct Position;
struct SummonPropertiesEntry;
class TestThread;

struct ScriptAction
{
    ObjectGuid sourceGUID;
    ObjectGuid targetGUID;
    ObjectGuid ownerGUID;                                       // owner of source if source is item
	ScriptInfo const* script;                               // pointer to static script data
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
    uint32 ScriptId;
    bool heroicForced = false;
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

struct PositionFullTerrainStatus
{
    struct AreaInfo
    {
        AreaInfo(int32 _adtId, int32 _rootId, int32 _groupId, uint32 _flags) : adtId(_adtId), rootId(_rootId), groupId(_groupId), mogpFlags(_flags) { }
        int32 const adtId;
        int32 const rootId;
        int32 const groupId;
        uint32 const mogpFlags;
    };

    PositionFullTerrainStatus() : areaId(0), floorZ(0.0f), outdoors(true) {}
    uint32 areaId;
    float floorZ;
    bool outdoors;
    ZLiquidStatus liquidStatus;
    Optional<AreaInfo> areaInfo;
    Optional<LiquidData> liquidInfo;
};

struct ZoneDynamicInfo
{
    ZoneDynamicInfo();

    uint32 MusicId;
    WeatherState WeatherId;
    float WeatherGrade;
    uint32 OverrideLightId;
    uint32 LightFadeInTime;
};

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

//sun: map instead of unordered, we want to be able to get max key in AddCreatureToGroup
typedef std::map<ObjectGuid::LowType /*leaderSpawnId*/, CreatureGroup*> CreatureGroupHolderType;
struct RespawnInfo; // forward declaration
struct CompareRespawnInfo
{
    bool operator()(RespawnInfo const* a, RespawnInfo const* b) const;
};
typedef std::unordered_map<uint32 /*zoneId*/, ZoneDynamicInfo> ZoneDynamicInfoMap;
typedef boost::heap::fibonacci_heap<RespawnInfo*, boost::heap::compare<CompareRespawnInfo>> RespawnListContainer;
typedef RespawnListContainer::handle_type RespawnListHandle;
typedef std::unordered_map<uint32, RespawnInfo*> RespawnInfoMap;
struct RespawnInfo
{
    SpawnObjectType type;
    ObjectGuid::LowType spawnId;
    uint32 entry;
    time_t respawnTime;
    uint32 gridId;
    uint32 zoneId;
    RespawnListHandle handle;
};
inline bool CompareRespawnInfo::operator()(RespawnInfo const* a, RespawnInfo const* b) const
{
    if (a == b)
        return false;
    if (a->respawnTime != b->respawnTime)
        return (a->respawnTime > b->respawnTime);
    if (a->spawnId != b->spawnId)
        return a->spawnId < b->spawnId;
    ASSERT(a->type != b->type, "Duplicate respawn entry for spawnId (%u,%u) found!", a->type, a->spawnId);
    return a->type < b->type;
}

enum MapType
{
    // not specialized
    MAP_TYPE_MAP, 
    // MapInstanced class
    MAP_TYPE_MAP_INSTANCED,
    // InstanceMap class
    MAP_TYPE_INSTANCE_MAP,
    // BattlegroundMap class
    MAP_TYPE_BATTLEGROUND_MAP,
    // TestMap class
    MAP_TYPE_TEST_MAP,
};

class TC_GAME_API Map : public GridRefManager<NGridType>
{
    friend class MapReference;
    public:
		explicit Map(MapType type, uint32 id, time_t expiry, uint32 InstanceId, uint8 SpawnMode, Map* parent = nullptr);
        virtual ~Map() override;

        MapEntry const* GetEntry() const { return i_mapEntry; }
        MapType GetMapType() const { return i_mapType; }

        // currently unused for normal maps
        virtual bool CanUnload(uint32 diff);

        virtual bool AddPlayerToMap(Player *);
        virtual void RemovePlayerFromMap(Player *, bool);
        template<class T> bool AddToMap(T *, bool checkTransport = false);
        template<class T> void RemoveFromMap(T *, bool);

        void VisitNearbyCellsOf(WorldObject* obj, TypeContainerVisitor<Trinity::ObjectUpdater, GridTypeMapContainer> &gridVisitor, TypeContainerVisitor<Trinity::ObjectUpdater, WorldTypeMapContainer> &worldVisitor);
        //this wrap map udpates and call it with diff since last updates. If minimumTimeSinceLastUpdate, the thread will sleep until minimumTimeSinceLastUpdate is reached
        void DoUpdate(uint32 maxDiff, uint32 minimumTimeSinceLastUpdate = 0);
        virtual void Update(const uint32&);

		virtual float GetDefaultVisibilityDistance() const;
		virtual float GetVisibilityNotifierPeriod() const;
        float GetVisibilityRange() const { return m_VisibleDistance; }
        //function for setting up visibility distance for maps on per-type/per-Id basis
        virtual void InitVisibilityDistance();
		void SetVisibilityDistance(float dist);

        void PlayerRelocation(Player* player, float x, float y, float z, float angle);
        void CreatureRelocation(Creature* creature, float x, float y, float z, float angle);
        void GameObjectRelocation(GameObject* gob, float x, float y, float z, float angle);
        void DynamicObjectRelocation(DynamicObject* dob, float x, float y, float z, float angle);

        template<class T, class CONTAINER> void Visit(const Cell &cell, TypeContainerVisitor<T, CONTAINER> &visitor);

        void LoadGrid(float x, float y);
		bool UnloadGrid(NGridType& ngrid, bool pForce);
        virtual void UnloadAll();

		void ResetGridExpiry(NGridType &grid, float factor = 1) const
		{
			grid.ResetTimeTracker(time_t(float(i_gridExpiry)*factor));
		}

		time_t GetGridExpiry(void) const { return i_gridExpiry; }
		uint32 GetId(void) const { return i_id; }

		bool IsRemovalGrid(float x, float y) const
		{
			GridCoord p = Trinity::ComputeGridCoord(x, y);
			return !getNGrid(p.x_coord, p.y_coord) /* || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL*/; //removed state is disabled on sunstrider
		}
        bool IsRemovalGrid(Position const& pos) const { return IsRemovalGrid(pos.GetPositionX(), pos.GetPositionY()); }

        bool IsGridLoaded(uint32 gridId) const { return IsGridLoaded(GridCoord(gridId % MAX_NUMBER_OF_GRIDS, gridId / MAX_NUMBER_OF_GRIDS)); }
        bool IsGridLoaded(float x, float y) const { return IsGridLoaded(Trinity::ComputeGridCoord(x, y)); }
        bool IsGridLoaded(Position const& pos) const { return IsGridLoaded(pos.GetPositionX(), pos.GetPositionY()); }


		static void InitStateMachine();
		static void DeleteStateMachine();

        Map const* GetParent() const { return m_parentMap; }

		void AddUpdateObject(Object* obj)
		{
			_updateObjects.insert(obj);
		}

		void RemoveUpdateObject(Object* obj)
		{
			_updateObjects.erase(obj);
		}

        // some calls like isInWater should not use vmaps due to processor power
        // can return INVALID_HEIGHT if under z+2 z coord not found height
        float _GetHeight(float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const;
        /* Returns closest height for given position, search in map height.
        @checkVMap search in vmap height as well. If both map and vmap heights were found, the closest one will be returned
        walkableOnly NYI
        Returns INVALID_HEIGHT if no height found at position or if height is further than maxSearchDist
        */
        float GetHeight(float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f, bool walkableOnly = false) const;
        float GetMinHeight(float x, float y) const;
        float GetGridMapHeight(float x, float y) const;
        float GetVMapFloor(float x, float y, float z, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const;
        /* Get map level (checking vmaps) or liquid level at given point */
        float GetWaterOrGroundLevel(uint32 phasemask, float x, float y, float z, float* ground = nullptr, bool swim = false, float collisionHeight = 2.03128f, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const; // 2.03128f = DEFAULT_COLLISION_HEIGHT in Object.h
        bool IsPlayerWalkable(Position pos) const;
        //Returns INVALID_HEIGHT if nothing found. walkableOnly NYI
        float GetHeight(uint32 phasemask, float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f, bool walkableOnly = false) const;
        float GetHeight(uint32 phasemask, Position const& pos, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const { return GetHeight(phasemask, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), vmap, maxSearchDist, collisionHeight); }
        float GetCeil(uint32 phasemask, float x, float y, float z, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const;
        float GetCeil(uint32 phasemask, Position const& pos, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const;
        float GetCeil(Position const& pos, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const;
        float GetCeil(float x, float y, float z, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const;
        float GetGameObjectCeil(uint32 phasemask, float x, float y, float z, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const;

        void RemoveGameObjectModel(GameObjectModel const& model);
        void InsertGameObjectModel(GameObjectModel const& model);
        bool ContainsGameObjectModel(GameObjectModel const& model) const;
        float GetGameObjectFloor(uint32 phasemask, float x, float y, float z, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, float collisionHeight = 0.0f) const
        {
            return _dynamicTree.getHeight(x, y, z, maxSearchDist + collisionHeight, phasemask);
        }
        Transport* GetTransportForPos(uint32 phase, float x, float y, float z, WorldObject* worldobject = nullptr);

        bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, uint32 phasemask, LineOfSightChecks checks, VMAP::ModelIgnoreFlags ignoreFlags) const;
        void Balance() { _dynamicTree.balance(); }
        //get dynamic collision (gameobjects only ?)
        bool getObjectHitPos(uint32 phasemask, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float &ry, float& rz, float modifyDist);

        time_t GetCreatureRespawnTime(ObjectGuid::LowType dbGuid) const
        {
            RespawnInfoMap::const_iterator itr = _creatureRespawnTimesBySpawnId.find(dbGuid);
            return itr != _creatureRespawnTimesBySpawnId.end() ? itr->second->respawnTime : 0;
        }

        time_t GetGORespawnTime(ObjectGuid::LowType dbGuid) const
        {
            RespawnInfoMap::const_iterator itr = _gameObjectRespawnTimesBySpawnId.find(dbGuid);
            return itr != _gameObjectRespawnTimesBySpawnId.end() ? itr->second->respawnTime : 0;
        }

        time_t GetRespawnTime(SpawnObjectType type, uint32 spawnId) const { return (type == SPAWN_TYPE_GAMEOBJECT) ? GetGORespawnTime(spawnId) : GetCreatureRespawnTime(spawnId); }

        void UpdatePlayerZoneStats(uint32 oldZone, uint32 newZone);

        void SaveRespawnTime(SpawnObjectType type, ObjectGuid::LowType spawnId, uint32 entry, time_t respawnTime, uint32 zoneId, uint32 gridId = 0, bool writeDB = true, bool replace = false, SQLTransaction dbTrans = nullptr);
        void SaveRespawnTimeDB(SpawnObjectType type, ObjectGuid::LowType spawnId, time_t respawnTime, SQLTransaction dbTrans = nullptr);
        void LoadRespawnTimes();
        void DeleteRespawnTimes() { DeleteRespawnInfo(); DeleteRespawnTimesInDB(GetId(), GetInstanceId()); }

        static void DeleteRespawnTimesInDB(uint16 mapId, uint32 instanceId);

        ZLiquidStatus GetLiquidStatus(float x, float y, float z, uint8 reqLiquidTypeMask, LiquidData *data = nullptr, float collisionHeight = 2.03128f) const;  // DEFAULT_COLLISION_HEIGHT in Object.h
        void GetFullTerrainStatusForPosition(float x, float y, float z, PositionFullTerrainStatus& data, uint8 reqLiquidType = MAP_ALL_LIQUIDS, float collisionHeight = 2.03128f) const; // DEFAULT_COLLISION_HEIGHT in Object.h

        float GetWaterLevel(float x, float y) const;
        //IsUnderWater is implied by this
        bool IsInWater(float x, float y, float z, LiquidData *data = nullptr) const;
        bool IsUnderWater(float x, float y, float z) const;

        uint32 GetAreaId(float x, float y, float z) const;
        uint32 GetAreaId(Position const& pos) const { return GetAreaId(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()); }
        uint32 GetZoneId(float x, float y, float z) const;
        uint32 GetZoneId(Position const& pos) const { return GetZoneId(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ()); }
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const;

        void MoveAllCreaturesInMoveList();
        void MoveAllGameObjectsInMoveList();
		void MoveAllDynamicObjectsInMoveList();
        void RemoveAllObjectsInRemoveList();
        virtual void RemoveAllPlayers();

        bool CreatureRespawnRelocation(Creature *c, bool diffGridOnly);        // used only in MoveAllCreaturesInMoveList and ObjectGridUnloader
        bool GameObjectRespawnRelocation(GameObject* go, bool diffGridOnly);

        // assert print helper
        bool CheckGridIntegrity(Creature* c, bool moved) const;
        bool CheckGridIntegrity(GameObject* c, bool moved) const;

        uint32 GetInstanceId() const { return i_InstanceId; }
        uint8 GetSpawnMode() const { return (i_spawnMode); }

        enum EnterState
        {
            CAN_ENTER = 0,
            CANNOT_ENTER_ALREADY_IN_MAP = 1, // Player is already in the map
            CANNOT_ENTER_NO_ENTRY, // No map entry was found for the target map ID
            CANNOT_ENTER_UNINSTANCED_DUNGEON, // No instance template was found for dungeon map
            CANNOT_ENTER_DIFFICULTY_UNAVAILABLE, // Requested instance difficulty is not available for target map
            CANNOT_ENTER_NOT_IN_RAID, // Target instance is a raid instance and the player is not in a raid group
            CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE, // Player is dead and their corpse is not in target instance
            CANNOT_ENTER_INSTANCE_BIND_MISMATCH, // Player's permanent instance save is not compatible with their group's current instance bind
            CANNOT_ENTER_TOO_MANY_INSTANCES, // Player has entered too many instances recently
            CANNOT_ENTER_MAX_PLAYERS, // Target map already has the maximum number of players allowed
            CANNOT_ENTER_ZONE_IN_COMBAT, // A boss encounter is currently in progress on the target map
            CANNOT_ENTER_UNSPECIFIED_REASON
        };
        virtual EnterState CannotEnter(Player* /*player*/) { return CAN_ENTER; }
        const char* GetMapName() const;

        // have meaning only for instanced map (that have set real difficulty)
        Difficulty GetDifficulty() const;
        bool IsRegularDifficulty() const;
        MapDifficulty const* GetMapDifficulty() const;

        bool Instanceable() const;
        bool IsDungeon() const;
        bool IsNonRaidDungeon() const;
        bool IsRaid() const;
        bool IsRaidOrHeroicDungeon() const;
        bool IsWorldMap() const;
        bool IsHeroic() const;

        bool IsBattleground() const;
        bool IsBattleArena() const;
        bool IsBattlegroundOrArena() const;
   
        void AddObjectToRemoveList(WorldObject *obj);
        void AddObjectToSwitchList(WorldObject *obj, bool on);
        virtual void DelayedUpdate(const uint32 diff);

		void LoadCorpseData();
		void DeleteCorpseData();
		void AddCorpse(Corpse* corpse);
		void RemoveCorpse(Corpse* corpse);
		Corpse* ConvertCorpseToBones(ObjectGuid const& ownerGuid, bool insignia = false);
		void RemoveOldCorpses();

        void resetMarkedCells() { marked_cells.reset(); }
        bool isCellMarked(uint32 pCellId) { return marked_cells.test(pCellId); }
        void markCell(uint32 pCellId) { marked_cells.set(pCellId); }

		TempSummon* SummonCreature(uint32 entry, Position const& pos, SummonPropertiesEntry const* properties = nullptr, uint32 duration = 0, Unit* summoner = nullptr, uint32 spellId = 0);
        void SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list = nullptr);
        Player* GetPlayer(ObjectGuid const& guid);
		Corpse* GetCorpse(ObjectGuid const& guid);
        Creature* GetCreature(ObjectGuid guid);
        GameObject* GetGameObject(ObjectGuid const& guid);
        Transport* GetTransport(ObjectGuid const& guid);
        DynamicObject* GetDynamicObject(ObjectGuid const& guid);
		Pet* GetPet(ObjectGuid const& guid);
        Creature* GetCreatureBySpawnId(ObjectGuid::LowType spawnId) const;
        GameObject* GetGameObjectBySpawnId(ObjectGuid::LowType spawnId) const;
        WorldObject* GetWorldObjectBySpawnId(SpawnObjectType type, ObjectGuid::LowType spawnId) const { return (type == SPAWN_TYPE_GAMEOBJECT) ? reinterpret_cast<WorldObject*>(GetGameObjectBySpawnId(spawnId)) : reinterpret_cast<WorldObject*>(GetCreatureBySpawnId(spawnId)); }
        WorldObject* GetWorldObject(ObjectGuid const& guid);

		MapStoredObjectTypesContainer& GetObjectsStore() { return _objectsStore; }

		typedef std::unordered_multimap<ObjectGuid::LowType, Creature*> CreatureBySpawnIdContainer;
		CreatureBySpawnIdContainer& GetCreatureBySpawnIdStore() { return _creatureBySpawnIdStore; }
        CreatureBySpawnIdContainer const& GetCreatureBySpawnIdStore() const { return _creatureBySpawnIdStore; }

		typedef std::unordered_multimap<ObjectGuid::LowType, GameObject*> GameObjectBySpawnIdContainer;
		GameObjectBySpawnIdContainer& GetGameObjectBySpawnIdStore() { return _gameobjectBySpawnIdStore; }
        GameObjectBySpawnIdContainer const& GetGameObjectBySpawnIdStore() const { return _gameobjectBySpawnIdStore; }

		std::unordered_set<Corpse*> const* GetCorpsesInCell(uint32 cellId) const
		{
			auto itr = _corpsesByCell.find(cellId);
			if (itr != _corpsesByCell.end())
				return &itr->second;

			return nullptr;
		}

		Corpse* GetCorpseByPlayer(ObjectGuid const& ownerGuid) const
		{
			auto itr = _corpsesByPlayer.find(ownerGuid);
			if (itr != _corpsesByPlayer.end())
				return itr->second;

			return nullptr;
		}

        MapInstanced* ToMapInstanced() { if (Instanceable())  return reinterpret_cast<MapInstanced*>(this); else return nullptr; }
        const MapInstanced* ToMapInstanced() const { if (Instanceable())  return (const MapInstanced*)((MapInstanced*)this); else return nullptr; }

        InstanceMap* ToInstanceMap() { if (IsDungeon())  return reinterpret_cast<InstanceMap*>(this); else return nullptr; }
        const InstanceMap* ToInstanceMap() const { if (IsDungeon())  return (const InstanceMap*)((InstanceMap*)this); else return nullptr; }

        BattlegroundMap* ToBattlegroundMap() { if (IsBattlegroundOrArena()) return reinterpret_cast<BattlegroundMap*>(this); else return nullptr; }
        const BattlegroundMap* ToBattlegroundMap() const { if (IsBattlegroundOrArena()) return reinterpret_cast<BattlegroundMap const*>(this); return nullptr; }


        bool HavePlayers() const { return !m_mapRefManager.isEmpty(); }
        uint32 GetPlayersCountExceptGMs() const;
		bool ActiveObjectsNearGrid(NGridType const& ngrid) const;

		void AddWorldObject(WorldObject* obj) { i_worldObjects.insert(obj); }
		void RemoveWorldObject(WorldObject* obj) { i_worldObjects.erase(obj); }

		/*
        void AddUnitToNotify(Unit* unit);
        void RelocationNotify();
		*/

        void SendToPlayers(WorldPacket* data) const;

        typedef MapRefManager PlayerList;
        PlayerList const& GetPlayers() const { return m_mapRefManager; }

		//per-map script storage
		void ScriptsStart(std::map<uint32, std::multimap<uint32, ScriptInfo> > const& scripts, uint32 id, Object* source, Object* target, bool start = true);
		void ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target);

        // must called with AddToWorld
        template<class T>
        void AddToForceActive(T* obj) { AddToForceActiveHelper(obj); }

        void AddToForceActive(Creature* obj);

        // must called with RemoveFromWorld
        template<class T>
        void RemoveFromForceActive(T* obj) { RemoveFromForceActiveHelper(obj); }

        void RemoveFromForceActive(Creature* obj);

		void UpdateIteratorBack(Player* player);

        template<class T> void SwitchGridContainers(T* obj, bool active);
        CreatureGroupHolderType CreatureGroupHolder;

        CreatureGroup* GetCreatureGroup(uint32 groupID) const;

        /*
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
        */
        
        void AddCreatureToPool(Creature*, uint32);
        void RemoveCreatureFromPool(Creature*, uint32);
        std::list<Creature*> GetAllCreaturesFromPool(uint32);

        // Objects that must update even in inactive grids without activating them
        typedef std::set<MotionTransport*> TransportsContainer;
        TransportsContainer _transports;
        TransportsContainer::iterator _transportsUpdateIter;

        //this function is overrided by InstanceMap and BattlegroundMap to handle crash recovery
        virtual void HandleCrash() { ASSERT(false); }

        void SetZoneMusic(uint32 zoneId, uint32 musicId);
        void SetZoneWeather(uint32 zoneId, WeatherState weatherId, float weatherGrade);
        void SetZoneOverrideLight(uint32 zoneId, uint32 lightId, uint32 fadeInTime);

		template<HighGuid high>
		inline ObjectGuid::LowType GenerateLowGuid()
		{
			static_assert(ObjectGuidTraits<high>::MapSpecific, "Only map specific guid can be generated in Map context");
			return GetGuidSequenceGenerator<high>().Generate();
		}

		template<HighGuid high>
		inline ObjectGuid::LowType GetMaxLowGuid()
		{
			static_assert(ObjectGuidTraits<high>::MapSpecific, "Only map specific guid can be retrieved in Map context");
			return GetGuidSequenceGenerator<high>().GetNextAfterMaxUsed();
		}

		uint32 GetLastMapUpdateTime() const { return _lastMapUpdate; }

        void ReloadMMap(int gx, int gy);

    private:

        void LoadMapAndVMap(int gx, int gy);
        void LoadVMap(int pX, int pY);
        void LoadMap(int gx, int gy, bool reload = false);
        void LoadMMap(int gx, int gy);
        GridMap* GetGrid(float x, float y);

		void SetTimer(uint32 t) { i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t; }

        void SendInitSelf( Player * player );

        void SendInitTransports( Player * player );
        void SendRemoveTransports( Player * player );

        void SendZoneDynamicInfo(Player* player);

        bool CreatureCellRelocation(Creature* creature, Cell new_cell);
        bool GameObjectCellRelocation(GameObject* gob, Cell new_cell);
		bool DynamicObjectCellRelocation(DynamicObject* go, Cell new_cell);

		template<class T> void InitializeObject(T* obj);
        void AddCreatureToMoveList(Creature* c, float x, float y, float z, float ang);
		void RemoveCreatureFromMoveList(Creature* c);
        void AddGameObjectToMoveList(GameObject* go, float x, float y, float z, float ang);
		void RemoveGameObjectFromMoveList(GameObject* go);
		void AddDynamicObjectToMoveList(DynamicObject* go, float x, float y, float z, float ang);
		void RemoveDynamicObjectFromMoveList(DynamicObject* go);
		
		bool _creatureToMoveLock;
        std::vector<Creature*> _creaturesToMove;

		bool _gameObjectsToMoveLock;
		std::vector<GameObject*> _gameObjectsToMove;

		bool _dynamicObjectsToMoveLock;
		std::vector<DynamicObject*> _dynamicObjectsToMove;

        bool IsGridLoaded(const GridCoord &) const;
        void EnsureGridLoaded(const Cell&);
		void EnsureGridLoadedForActiveObject(Cell const&, WorldObject* object);
        void EnsureGridCreated(const GridCoord &);
        void EnsureGridCreated_i(const GridCoord &);

        void buildNGridLinkage(NGridType* pNGridType) { pNGridType->link(this); }

		/*
        template<class T> void AddType(T *obj);
        template<class T> void RemoveType(T *obj, bool);
		*/

        NGridType* getNGrid(uint32 x, uint32 y) const
        {
            assert(x < MAX_NUMBER_OF_GRIDS);
            assert(y < MAX_NUMBER_OF_GRIDS);
            return i_grids[x][y];
        }
        bool isGridObjectDataLoaded(uint32 x, uint32 y) const { return getNGrid(x,y)->isGridObjectDataLoaded(); }
        void setGridObjectDataLoaded(bool pLoaded, uint32 x, uint32 y) { getNGrid(x,y)->setGridObjectDataLoaded(pLoaded); }

        void setNGrid(NGridType* grid, uint32 x, uint32 y);
		void ScriptsProcess();

		void SendObjectUpdates();

        bool AllTransportsEmpty() const; // sunwell
        void AllTransportsRemovePassengers(); // sunwell
        TransportsContainer const& GetAllTransports() const { return _transports; }

    protected:
        void SetUnloadReferenceLock(GridCoord const& p, bool on) { getNGrid(p.x_coord, p.y_coord)->setUnloadReferenceLock(on); }

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

		int32 m_VisibilityNotifyPeriod;

        /** The objects in m_activeForcedNonPlayers are always kept active and makes everything around them also active, just like players
        */
        typedef std::set<WorldObject*> ActiveForcedNonPlayers;
        ActiveForcedNonPlayers m_activeForcedNonPlayers;
        ActiveForcedNonPlayers::iterator m_activeForcedNonPlayersIter;

        MapType i_mapType;
        bool m_disableMapObjects; //do not load creatures and gameobjects. For testing purpose.
    private:
        inline Player* _GetScriptPlayerSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo) const;
        inline Creature* _GetScriptCreatureSourceOrTarget(Object* source, Object* target, ScriptInfo const* scriptInfo, bool bReverse = false) const;
        inline Unit* _GetScriptUnit(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        inline Player* _GetScriptPlayer(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        inline Creature* _GetScriptCreature(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        inline WorldObject* _GetScriptWorldObject(Object* obj, bool isSource, ScriptInfo const* scriptInfo) const;
        inline void _ScriptProcessDoor(Object* source, Object* target, ScriptInfo const* scriptInfo) const;
        inline GameObject* _FindGameObject(WorldObject* pWorldObject, ObjectGuid::LowType guid) const;

        NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        GridMap* GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        uint16 GridMapReference[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cells;

		//these functions used to process player/mob aggro reactions and
		//visibility calculations. Highly optimized for massive calculations
		void ProcessRelocationNotifies(const uint32 diff);

		bool i_scriptLock;
        std::set<WorldObject *> i_objectsToRemove;
        std::map<WorldObject*, bool> i_objectsToSwitch;
		std::set<WorldObject*> i_worldObjects;

		typedef std::multimap<time_t, ScriptAction> ScriptScheduleMap;
		ScriptScheduleMap m_scriptSchedule;

    public:
        void ProcessRespawns();
        void ApplyDynamicModeRespawnScaling(WorldObject const* obj, ObjectGuid::LowType spawnId, uint32& respawnDelay, uint32 mode) const;

    private:
        // if return value is true, we can respawn
        // if return value is false, reschedule the respawn to new value of info->respawnTime iff nonzero, delete otherwise
        // if return value is false and info->respawnTime is nonzero, it is guaranteed to be greater than time(NULL)
        bool CheckRespawn(RespawnInfo* info);
        void DoRespawn(SpawnObjectType type, ObjectGuid::LowType spawnId, uint32 gridId);
        void Respawn(RespawnInfo* info, bool force = false, SQLTransaction dbTrans = nullptr);
        void Respawn(std::vector<RespawnInfo*>& respawnData, bool force = false, SQLTransaction dbTrans = nullptr);
        void AddRespawnInfo(RespawnInfo& info, bool replace = false);
        void DeleteRespawnInfo();
        void DeleteRespawnInfo(RespawnInfo* info);
        void DeleteRespawnInfo(std::vector<RespawnInfo*>& toDelete)
        {
            for (RespawnInfo* info : toDelete)
                DeleteRespawnInfo(info);
            toDelete.clear();
        }
        void DeleteRespawnInfo(SpawnObjectTypeMask types, uint32 zoneId = 0)
        {
            std::vector<RespawnInfo*> v;
            GetRespawnInfo(v, types, zoneId);
            if (!v.empty())
                DeleteRespawnInfo(v);
        }
        void DeleteRespawnInfo(SpawnObjectType type, ObjectGuid::LowType spawnId)
        {
            if (RespawnInfo* info = GetRespawnInfo(type, spawnId))
                DeleteRespawnInfo(info);
        }

    public:
        void GetRespawnInfo(std::vector<RespawnInfo*>& respawnData, SpawnObjectTypeMask types, uint32 zoneId = 0) const;
        RespawnInfo* GetRespawnInfo(SpawnObjectType type, ObjectGuid::LowType spawnId) const;
        void ForceRespawn(SpawnObjectType type, ObjectGuid::LowType spawnId)
        {
            if (RespawnInfo* info = GetRespawnInfo(type, spawnId))
                Respawn(info, true);
        }
        void RemoveRespawnTime(RespawnInfo* info, bool doRespawn = false, SQLTransaction dbTrans = nullptr);
        void RemoveRespawnTime(std::vector<RespawnInfo*>& respawnData, bool doRespawn = false, SQLTransaction dbTrans = nullptr);
        void RemoveRespawnTime(SpawnObjectTypeMask types = SPAWN_TYPEMASK_ALL, uint32 zoneId = 0, bool doRespawn = false, SQLTransaction dbTrans = nullptr)
        {
            std::vector<RespawnInfo*> v;
            GetRespawnInfo(v, types, zoneId);
            if (!v.empty())
                RemoveRespawnTime(v, doRespawn, dbTrans);
        }
        void RemoveRespawnTime(SpawnObjectType type, ObjectGuid::LowType spawnId, bool doRespawn = false, SQLTransaction dbTrans = nullptr)
        {
            if (RespawnInfo* info = GetRespawnInfo(type, spawnId))
                RemoveRespawnTime(info, doRespawn, dbTrans);
        }

        SpawnGroupTemplateData const* GetSpawnGroupData(uint32 groupId) const;

        bool IsSpawnGroupActive(uint32 groupId) const;

        // Enable the spawn group, which causes all creatures in it to respawn (unless they have a respawn timer)
        // The force flag can be used to force spawning additional copies even if old copies are still around from a previous spawn
        bool SpawnGroupSpawn(uint32 groupId, bool ignoreRespawn = false, bool force = false, std::vector<WorldObject*>* spawnedObjects = nullptr);
        // Despawn all creatures in the spawn group if spawned, optionally delete their respawn timer, and disable the group
        bool SpawnGroupDespawn(uint32 groupId, bool deleteRespawnTimes = false, size_t* count = nullptr);

        // Disable the spawn group, which prevents any creatures in the group from respawning until re-enabled
        // This will not affect any already-present creatures in the group
        void SetSpawnGroupInactive(uint32 groupId) { SetSpawnGroupActive(groupId, false); }

        typedef std::function<void(Map*)> FarSpellCallback;
        void AddFarSpellCallback(FarSpellCallback&& callback);

        // Type specific code for add/remove to/from grid
        template<class T>
            void AddToGrid(T*, Cell const&);

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
                auto itr = m_activeForcedNonPlayers.find(obj);
                if(itr == m_activeForcedNonPlayers.end())
                    return;
                if(itr==m_activeForcedNonPlayersIter)
                    ++m_activeForcedNonPlayersIter;
                m_activeForcedNonPlayers.erase(itr);
            }
            else
                m_activeForcedNonPlayers.erase(obj);
        }

        typedef std::map<uint32, std::set<ObjectGuid> > CreaturePoolMember;
        CreaturePoolMember m_cpmembers;

        RespawnListContainer _respawnTimes;
        RespawnInfoMap       _creatureRespawnTimesBySpawnId;
        RespawnInfoMap       _gameObjectRespawnTimesBySpawnId;
        RespawnInfoMap& GetRespawnMapForType(SpawnObjectType type) { return (type == SPAWN_TYPE_GAMEOBJECT) ? _gameObjectRespawnTimesBySpawnId : _creatureRespawnTimesBySpawnId; }
        RespawnInfoMap const& GetRespawnMapForType(SpawnObjectType type) const { return (type == SPAWN_TYPE_GAMEOBJECT) ? _gameObjectRespawnTimesBySpawnId : _creatureRespawnTimesBySpawnId; }
        
        void SetSpawnGroupActive(uint32 groupId, bool state);
        std::unordered_set<uint32> _toggledSpawnGroupIds;

        uint32 _respawnCheckTimer;
        std::unordered_map<uint32, uint32> _zonePlayerCountMap;

        ZoneDynamicInfoMap _zoneDynamicInfo;
        uint32 _defaultLight;

		template<HighGuid high>
		inline ObjectGuidGeneratorBase& GetGuidSequenceGenerator()
		{
			auto itr = _guidGenerators.find(high);
			if (itr == _guidGenerators.end())
				itr = _guidGenerators.insert(std::make_pair(high, std::unique_ptr<ObjectGuidGenerator<high>>(new ObjectGuidGenerator<high>()))).first;

			return *itr->second;
		}
		std::map<HighGuid, std::unique_ptr<ObjectGuidGeneratorBase>> _guidGenerators;
        
		MapStoredObjectTypesContainer _objectsStore;
        CreatureBySpawnIdContainer _creatureBySpawnIdStore;
        GameObjectBySpawnIdContainer _gameobjectBySpawnIdStore;
		std::unordered_map<uint32/*cellId*/, std::unordered_set<Corpse*>> _corpsesByCell;
		std::unordered_map<ObjectGuid, Corpse*> _corpsesByPlayer;
		std::unordered_set<Corpse*> _corpseBones;

		std::unordered_set<Object*> _updateObjects;
        uint32 _lastMapUpdate;

        MPSCQueue<FarSpellCallback> _farSpellCallbacks;

		time_t i_gridExpiry;

		//used for fast base_map (e.g. MapInstanced class object) search for
		//InstanceMaps and BattlegroundMaps...
		Map* m_parentMap;

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

class TC_GAME_API InstanceMap : public Map
{
    public:
        InstanceMap(uint32 id, time_t expiry, uint32 InstanceId, uint8 SpawnMode, Map* parent);
        ~InstanceMap() override;
        bool AddPlayerToMap(Player *) override;
        void RemovePlayerFromMap(Player *, bool) override;
        void Update(const uint32&) override;
        void CreateInstanceData(bool load);
        bool Reset(uint8 method);
        uint32 GetScriptId() { return i_script_id; }
        std::string const& GetScriptName() const;
        InstanceScript* GetInstanceScript() { return i_data; }
        void PermBindAllPlayers();
        void UnloadAll() override;
        void HandleCrash() override;
        EnterState CannotEnter(Player* player) override;
        void SendResetWarnings(uint32 timeLeft) const;
        void SetResetSchedule(bool on);

        float GetDefaultVisibilityDistance() const override;
		float GetVisibilityNotifierPeriod() const override;
    protected:
        bool m_unloadWhenEmpty;
    private:
        bool m_resetAfterUnload;
        InstanceScript* i_data;
        uint32 i_script_id;
};

class TC_GAME_API TestMap : public InstanceMap
{
public:
    TestMap(std::weak_ptr<TestThread>& testThread, uint32 id, uint32 InstanceId, uint8 spawnMode, Map* parent, bool enableMapObjects);
    ~TestMap();
    void Update(const uint32&) override;
    void RemoveAllPlayers() override;
    bool AddPlayerToMap(Player *) override;
    bool CanUnload(uint32 diff) override;

    void DisconnectAllBots();
    Player* GetFirstHumanPlayer();

private:
    std::weak_ptr<TestThread> _testThread; //TestMap will use the TestThread for some time sync with the test waits
};

class TC_GAME_API BattlegroundMap : public Map
{
    public:
		BattlegroundMap(uint32 id, time_t expiry, uint32 InstanceId, Map* parent); //expiry arg is fake here, battlegrounds grids are never unloaded
        ~BattlegroundMap() override;

        bool AddPlayerToMap(Player *) override;
        void RemovePlayerFromMap(Player *, bool) override;
        EnterState CannotEnter(Player* player) override;
        void SetUnload();
        //void UnloadAll();
        void RemoveAllPlayers() override;

        void HandleCrash() override;

		float GetDefaultVisibilityDistance() const override;
		float GetVisibilityNotifierPeriod() const override;

        Battleground* GetBG() { return m_bg; }
        void SetBG(Battleground* bg) { m_bg = bg; }
    private:
        Battleground* m_bg;
};

template<class T, class CONTAINER>
inline void Map::Visit(Cell const& cell, TypeContainerVisitor<T, CONTAINER>& visitor)
{
    const uint32 x = cell.GridX();
    const uint32 y = cell.GridY();
    const uint32 cell_x = cell.CellX();
    const uint32 cell_y = cell.CellY();

    if (!cell.NoCreate() || IsGridLoaded(GridCoord(x, y)))
    {
        EnsureGridLoaded(cell);
        getNGrid(x, y)->VisitGrid(cell_x, cell_y, visitor);
    }
}

#endif

