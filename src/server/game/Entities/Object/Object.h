
#ifndef _OBJECT_H
#define _OBJECT_H

#include "Common.h"
#include "Duration.h"
#include "EventProcessor.h"
#include "ByteBuffer.h"
#include "UpdateData.h"
#include "Map.h"
#include "ModelIgnoreFlags.h"
#include "ObjectGuid.h"
#include "UpdateMask.h"
#include "SharedDefines.h"
#include "SpellDefines.h"
#include "Position.h"
#include "ObjectDefines.h"

#include <set>
#include <string>

uint32 GuidHigh2TypeId(HighGuid guid_hi);

class WorldPacket;
class UpdateData;
class Creature;
class Player;
class InstanceScript;
class GameObject;
class MotionTransport;
class WorldObject;
class CreatureAI;
class ZoneScript;
class Spell;
class SpellCastTargets;
class SpellInfo;
struct FactionTemplateEntry;

namespace G3D
{
    class Vector3;
}

typedef std::unordered_map<Player*, UpdateData> UpdateDataMapType;
typedef std::unordered_set<uint32> UpdatePlayerSet;

float const DEFAULT_COLLISION_HEIGHT = 2.03128f; // Most common value in dbc

struct MovementInfo
{
    // common
#ifdef LICH_KING
    ObjectGuid guid;
#endif
    uint32 flags;
#ifdef LICH_KING
    uint16 flags2;
#else
    uint8 flags2; //not used on BC (but still in packet)
#endif
    Position pos;
    uint32 time;

    // transport
    struct TransportInfo
    {
        void Reset()
        {
            guid.Clear();
            pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
            time = 0;
        }

        ObjectGuid guid;
        Position pos;
        uint32 time;
    } transport;

    // swimming/flying
    float pitch;

    // falling
    uint32 fallTime;

    // jumping
    struct JumpInfo
    {
        void Reset()
        {
            zspeed = sinAngle = cosAngle = xyspeed = 0.0f;
        }

        float zspeed, sinAngle, cosAngle, xyspeed;

    } jump;

    // spline
    float splineElevation;

    MovementInfo() :
#ifdef LICH_KING
        guid(0),
#endif
        flags(0), flags2(0), time(0), pitch(0.0f), fallTime(0), splineElevation(0.0f)
    {
        pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        transport.Reset();
        jump.Reset();
    }

    uint32 GetMovementFlags() const { return flags; }
    void SetMovementFlags(uint32 flag) { flags = flag; }
    void AddMovementFlag(uint32 flag) { flags |= flag; }
    void RemoveMovementFlag(uint32 flag) { flags &= ~flag; }
    bool HasMovementFlag(uint32 flag) const { return flags & flag; }

    uint16 GetExtraMovementFlags() const { return flags2; }
    bool HasExtraMovementFlag(uint16 flag) const { return flags2 & flag; }
#ifdef LICH_KING
    void AddExtraMovementFlag(uint16 flag) { flags2 |= flag; }
#endif

    void SetFallTime(uint32 _time) { fallTime = _time; }
};

#define POSITION_GET_X_Y_Z(a)   (a)->GetPositionX(), (a)->GetPositionY(), (a)->GetPositionZ()
#define POSITION_GET_X_Y_Z_O(a) (a)->GetPositionX(), (a)->GetPositionY(), (a)->GetPositionZ(), (a)->GetOrientation()

template<class T>
class GridObject
{
public:
	virtual ~GridObject() { }

	bool IsInGrid() const { return _gridRef.isValid(); }
	void AddToGrid(GridRefManager<T>& m) { ASSERT(!IsInGrid()); _gridRef.link(&m, (T*)this); }
	void RemoveFromGrid() { ASSERT(IsInGrid()); _gridRef.unlink(); }
private:
	GridReference<T> _gridRef;
};

class TC_GAME_API Object
{
    public:
        virtual ~Object ( );
        
        inline const bool& IsInWorld() const { return m_inWorld; }
        virtual void AddToWorld()
        {
            if(m_inWorld)
                return;

            assert(m_uint32Values);

            m_inWorld = true;

            // synchronize values mirror with values array (changes will send in updatecreate opcode any way
            ClearUpdateMask(true);
        }
        virtual void RemoveFromWorld()
        {
            if(!m_inWorld)
                return;

            m_inWorld = false;

            // if we remove from world then sending changes not required
            ClearUpdateMask(true);
        }

        ObjectGuid GetGUID() const;
        PackedGuid const& GetPackGUID() const;
        uint32 GetEntry() const;
        void SetEntry(uint32 entry);

        float GetObjectScale() const;
        virtual void SetObjectScale(float scale);

        uint8 GetTypeId() const { return m_objectTypeId; }
        bool isType(uint16 mask) const { return (mask & m_objectType); }

        virtual void BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        void SendUpdateToPlayer(Player* player);

        /**
            Fill the update data with update(s) for given target (the updates are about the data of this object)
        */
        void BuildValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const;
        /**
            Mark this object for destroying at client in update data
        */
        void BuildOutOfRangeUpdateBlock(UpdateData *data) const;

        virtual void DestroyForPlayer(Player *target, bool onDeath = false) const;

        int32 GetInt32Value( uint16 index ) const;
        uint32 GetUInt32Value( uint16 index ) const;
        uint64 GetUInt64Value( uint16 index ) const;
        float GetFloatValue( uint16 index ) const;
        uint8 GetByteValue( uint16 index, uint8 offset) const;
        uint16 GetUInt16Value(uint16 index, uint8 offset) const;
        ObjectGuid GetGuidValue(uint16 index) const;

        void SetInt32Value(uint16 index, int32 value);
        void SetUInt32Value(uint16 index, uint32 value);
        void UpdateUInt32Value(uint16 index, uint32 value);
        void SetUInt64Value(uint16 index, uint64 value);
        void SetFloatValue(uint16 index, float value);
        void SetByteValue(uint16 index, uint8 offset, uint8 value);
        void SetUInt16Value(uint16 index, uint8 offset, uint16 value);
        void SetInt16Value(uint16 index, uint8 offset, int16 value) { SetUInt16Value(index, offset, (uint16)value); }
        void SetStatFloatValue(uint16 index, float value);
        void SetStatInt32Value(uint16 index, int32 value);
        void SetGuidValue(uint16 index, ObjectGuid value);
		bool AddGuidValue(uint16 index, ObjectGuid value);
		bool RemoveGuidValue(uint16 index, ObjectGuid value);

        void ApplyModUInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModPositiveFloatValue( uint16 index, float val, bool apply);
        void ApplyModSignedFloatValue( uint16 index, float val, bool apply);
        void ApplyPercentModFloatValue(uint16 index, float val, bool apply);

        void SetFlag(uint16 index, uint32 newFlag);
        void RemoveFlag(uint16 index, uint32 oldFlag);
        void ToggleFlag(uint16 index, uint32 flag);
        bool HasFlag(uint16 index, uint32 flag) const;

        void SetByteFlag(uint16 index, uint8 offset, uint8 newFlag);
        void RemoveByteFlag(uint16 index, uint8 offset, uint8 newFlag);
        void ToggleFlag(uint16 index, uint8 offset, uint8 flag );
        bool HasByteFlag(uint16 index, uint8 offset, uint8 flag) const;

        void ApplyModFlag(uint16 index, uint32 flag, bool apply);
        void SetFlag64(uint16 index, uint64 newFlag);
        void RemoveFlag64(uint16 index, uint64 oldFlag);
        void ToggleFlag64(uint16 index, uint64 flag);
        bool HasFlag64(uint16 index, uint64 flag) const;
        void ApplyModFlag64(uint16 index, uint64 flag, bool apply);

        void ClearUpdateMask(bool remove);
        //void SendUpdateObjectToAllExcept(Player* exceptPlayer);

        bool LoadValues(const char* data);

        uint16 GetValuesCount() const { return m_valuesCount; }

        virtual bool HasQuest(uint32 /* quest_id */) const { return false; }
        virtual bool HasInvolvedQuest(uint32 /* quest_id */) const { return false; }

        void SetIsNewObject(bool enable) { m_isNewObject = enable; }
        /** 
            Visits cells around the object, fill players UpdateData with updates from this object if needed
			player_set is there for performance only, since this function is called a lot, this helps avoiding recreating it at each call. Content is not important and is cleared at each call.
        */
        virtual void BuildUpdate(UpdateDataMapType&, UpdatePlayerSet& player_set) { }
        /**
           Adds the player and update data for him to the given updateData map. 
           Creates the update map for him if it doesn't exists, else exists the already existing one.
        */
        void BuildFieldsUpdate(Player*, UpdateDataMapType& data_map) const;

        /** Force notify of all update fields having this flag. Don't forget to remove it afterwards. */
        void SetFieldNotifyFlag(uint16 flag) { _fieldNotifyFlags |= flag; }
        void RemoveFieldNotifyFlag(uint16 flag) { _fieldNotifyFlags &= uint16(~flag); }

        // FG: some hacky helpers
        void ForceValuesUpdateAtIndex(uint32);
        
        inline Player* ToPlayer() { if (GetTypeId() == TYPEID_PLAYER) return reinterpret_cast<Player*>(this); else return nullptr; }
        inline Player const* ToPlayer() const { if (GetTypeId() == TYPEID_PLAYER) return reinterpret_cast<Player const*>(this); else return nullptr; }

        inline Creature* ToCreature() { if (GetTypeId() == TYPEID_UNIT) return reinterpret_cast<Creature*>(this); else return nullptr; }
        inline Creature const* ToCreature() const { if (GetTypeId() == TYPEID_UNIT) return reinterpret_cast<Creature const*>(this); else return nullptr; }

        inline Unit* ToUnit() { if (isType(TYPEMASK_UNIT)) return reinterpret_cast<Unit*>(this); else return nullptr; }
        inline Unit const* ToUnit() const { if (isType(TYPEMASK_UNIT)) return reinterpret_cast<Unit const*>(this); else return nullptr; }

        inline GameObject* ToGameObject() { if (GetTypeId() == TYPEID_GAMEOBJECT) return reinterpret_cast<GameObject*>(this); else return nullptr; }
        inline GameObject const* ToGameObject() const { if (GetTypeId() == TYPEID_GAMEOBJECT) return reinterpret_cast<GameObject const*>(this); else return nullptr; }

        inline Corpse* ToCorpse() { if (GetTypeId() == TYPEID_CORPSE) return reinterpret_cast<Corpse*>(this); else return nullptr; }
        inline Corpse const* ToCorpse() const { if (GetTypeId() == TYPEID_CORPSE) return reinterpret_cast<Corpse const*>(this); else return nullptr; }

        inline DynamicObject* ToDynObject() { if (GetTypeId() == TYPEID_DYNAMICOBJECT) return reinterpret_cast<DynamicObject*>(this); else return nullptr; }
        inline DynamicObject const* ToDynObject() const { if (GetTypeId() == TYPEID_DYNAMICOBJECT) return reinterpret_cast<DynamicObject const*>(this); else return nullptr; }

    protected:

        Object();

        void _InitValues();
        void _Create(ObjectGuid::LowType guidlow, uint32 entry, HighGuid guidhigh);
        std::string _ConcatFields(uint16 startIndex, uint16 size) const;
        void _LoadIntoDataField(std::string const& data, uint32 startOffset, uint32 count);

        uint32 GetUpdateFieldData(Player const* target, uint32*& flags) const;

        void BuildMovementUpdate(ByteBuffer* data, uint16 flags) const;
        /**
            Second step of filling updateData ByteBuffer with data from this object, for given target
        */
        virtual void BuildValuesUpdate(uint8 updatetype, ByteBuffer* updateData, Player* target) const;

        uint16 m_objectType;

        uint8 m_objectTypeId;
        uint8 m_updateFlag;

        union
        {
            int32  *m_int32Values;
            uint32 *m_uint32Values;
            float  *m_floatValues;
        };

        UpdateMask _changesMask;

        uint16 m_valuesCount;

        uint16 _fieldNotifyFlags;

		virtual void AddToObjectUpdate() = 0;
		virtual void RemoveFromObjectUpdate() = 0;
		void AddToObjectUpdateIfNeeded();

        bool m_objectUpdated;

    private:
        bool m_inWorld;
        bool m_isNewObject;

        PackedGuid m_PackGUID;

        // for output helpfull error messages from asserts
        bool PrintIndexError(uint32 index, bool set) const;
        Object(const Object&);                              // prevent generation copy constructor
        Object& operator=(Object const&);                   // prevent generation assigment operator
};


template <class T_VALUES, class T_FLAGS, class FLAG_TYPE, uint8 ARRAY_SIZE>
class FlaggedValuesArray32
{
public:
	FlaggedValuesArray32()
	{
		memset(&m_values, 0x00, sizeof(T_VALUES) * ARRAY_SIZE);
		m_flags = 0;
	}

	T_FLAGS  GetFlags() const { return m_flags; }
	bool     HasFlag(FLAG_TYPE flag) const { return m_flags & (1 << flag); }
	void     AddFlag(FLAG_TYPE flag) { m_flags |= (1 << flag); }
	void     DelFlag(FLAG_TYPE flag) { m_flags &= ~(1 << flag); }

	T_VALUES GetValue(FLAG_TYPE flag) const { return m_values[flag]; }
	void     SetValue(FLAG_TYPE flag, T_VALUES value) { m_values[flag] = value; }
	void     AddValue(FLAG_TYPE flag, T_VALUES value) { m_values[flag] += value; }

private:
	T_VALUES m_values[ARRAY_SIZE];
	T_FLAGS m_flags;
};

enum MapObjectCellMoveState
{
	MAP_OBJECT_CELL_MOVE_NONE, //not in move list
	MAP_OBJECT_CELL_MOVE_ACTIVE, //in move list
	MAP_OBJECT_CELL_MOVE_INACTIVE, //in move list but should not move
};

class TC_GAME_API MapObject
{
	friend class Map; //map for moving creatures
	friend class ObjectGridLoader; //grid loader for loading creatures

protected:
	MapObject() : _moveState(MAP_OBJECT_CELL_MOVE_NONE)
	{
		_newPosition.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
	}

private:
	Cell _currentCell;
	Cell const& GetCurrentCell() const { return _currentCell; }
	void SetCurrentCell(Cell const& cell) { _currentCell = cell; }

	MapObjectCellMoveState _moveState;
	Position _newPosition;
	void SetNewCellPosition(float x, float y, float z, float o)
	{
		_moveState = MAP_OBJECT_CELL_MOVE_ACTIVE;
		_newPosition.Relocate(x, y, z, o);
	}
};

class TC_GAME_API WorldObject : public Object, public WorldLocation
{
    public:
		~WorldObject() override;

        virtual void Update ( uint32 /*time_diff*/ ) { }

        void _Create(ObjectGuid::LowType guidlow, HighGuid guidhigh, uint32 phaseMask);
        virtual void AddToWorld() override;
		virtual void RemoveFromWorld() override;

        void GetNearPoint2D(WorldObject const* searcher, float &x, float &y, float distance, float absAngle) const;
        void GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float distance2d, float absAngle) const;
        void GetClosePoint(float &x, float &y, float &z, float size, float distance2d = 0, float relAngle = 0) const;
        void MovePosition(Position &pos, float dist, float angle);
        void GetGroundPoint(float &x, float &y, float &z, float dist, float angle);
        void GetGroundPointAroundUnit(float &x, float &y, float &z, float dist, float angle)
        {
            GetPosition(x, y, z);
            GetGroundPoint(x, y, z, dist, angle);
        }
        void GetContactPoint(const WorldObject* obj, float &x, float &y, float &z, float distance2d = CONTACT_DISTANCE) const;
        
        /* Get first collision. Checks static & dynamic positions.
        ignoreContactDistance = false : move back CONTACT_DISTANCE at collision
        resultPos is set at collision or at destination if no collision happened
        Returns wheter a collision happened
        */
        bool GetCollisionPosition(Position from, float x, float y, float z, Position& resultPos, float modifyDist = -CONTACT_DISTANCE);
        /* get first collision position with ground or valid terrain under it. Trinity name : GetFirstCollisionPosition
           angle = relative angle from current orientation */
        Position GetFirstWalkableCollisionPosition(float dist, float angle);
        /** move to first collision position with ground or valid terrain under it
            angle = relative angle from current orientation
        */
        void MovePositionToFirstWalkableCollision(Position &pos, float dist, float angle);
        inline void MovePositionToFirstCollision(Position &pos, float dist, float angle) { return MovePositionToFirstWalkableCollision(pos, dist, angle); } //TC compat
        bool ComputeCollisionPosition(Position const& startPosition, Position const& endPosition, float& x, float& y, float& z) const;

        Position GetRandomNearPosition(float radius);
        Position GetNearPosition(float dist, float angle);

        virtual float GetCombatReach() const { return 0.0f; } // overridden (only) in Unit
        bool IsPositionValid() const;
        //Set Z to ground position for given x and z
        void UpdateGroundPositionZ(float x, float y, float &z) const;
        //Set Z to closest allowed position, depending on fly/swim/waterwalk ability of object
        void UpdateAllowedPositionZ(float x, float y, float &z, float maxDist = 50.0f) const;
        //Set Z to closest allowed position, depending on given fly/swim/waterwalk abilities given
        static void UpdateAllowedPositionZ(uint32 phaseMask, uint32 mapId, float x, float y, float &z, bool canSwim, bool canFly, bool waterWalk, float collisionHeight, float maxDist = 50.0f);
        float SelectBestZForDestination(float x, float y, float z, bool excludeCollisionHeight) const;

        void GetRandomPoint(Position const& pos, float distance, float &rand_x, float &rand_y, float &rand_z) const;
        Position GetRandomPoint(Position const &srcPos, float distance) const;

		virtual void SetMap(Map* map);
		virtual void ResetMap();

		void SetZoneScript();
		void ClearZoneScript();
		ZoneScript* GetZoneScript() const { return m_zoneScript; }

        uint32 GetInstanceId() const { return m_InstanceId; }

        uint32 GetZoneId() const { return m_zoneId; }
        uint32 GetAreaId() const { return m_areaId; }
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const { zoneid = m_zoneId, areaid = m_areaId; }
        bool IsOutdoors() const { return m_outdoors; }

        //for trinitycore compatibility
        uint32 GetPhaseMask() const { return m_phaseMask; }
		bool InSamePhase(WorldObject const* obj) const;
		bool InSamePhase(uint32 phasemask) const { return (GetPhaseMask() & phasemask) != 0; }
        static bool InSamePhase(WorldObject const* a, WorldObject const* b) { return a && a->InSamePhase(b); }
		virtual void SetPhaseMask(uint32 newPhaseMask, bool update);

        InstanceScript* GetInstanceScript();

        std::string const& GetName() const { return m_name; }
        void SetName(const std::string& newname) { m_name=newname; }

        // override WorldObject function for proper name localization
        virtual std::string const& GetNameForLocaleIdx(LocaleConstant locale_idx) const { return GetName(); }

        float GetDistance( const WorldObject* obj ) const;
        float GetDistance(const float x, const float y, const float z) const;
        float GetDistance(Position const &pos) const;
        float GetExactDistance(const float x, const float y, const float z) const;
        float GetDistanceSqr(float x, float y, float z) const;
        float GetDistanceSq(const float &x, const float &y, const float &z) const;
        float GetDistance2d(const WorldObject* obj) const;
        float GetDistance2d(const float x, const float y) const;
        float GetExactDistance2d(const WorldObject* obj) const;
        float GetExactDistance2d(const float x, const float y) const;
        float GetDistanceZ(const WorldObject* obj) const;

        bool IsSelfOrInSameMap(WorldObject const* obj) const;
        inline bool IsInMap(const WorldObject* obj) const { return GetMapId()==obj->GetMapId() && GetInstanceId()==obj->GetInstanceId(); }
        bool IsWithinDist3d(float x, float y, float z, float dist) const;
        bool IsWithinDist3d(Position const* pos, float dist) const;
        bool IsWithinDist2d(float x, float y, float dist) const;
        bool IsWithinDist2d(Position const* pos, float dist) const;
        // No check if same map. Use only if you will sure about placing both object at same map
        bool IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D = true) const;
        bool IsWithinDistInMap(WorldObject const* obj, float dist2compare, bool is3D = true, bool incOwnRadius = true, bool incTargetRadius = true) const;
        bool IsWithinLOS(float x, float y, float z, LineOfSightChecks checks = LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags ignoreFlags = VMAP::ModelIgnoreFlags::Nothing) const;
        bool IsWithinLOSInMap(WorldObject const* obj, LineOfSightChecks checks = LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags ignoreFlags = VMAP::ModelIgnoreFlags::Nothing) const;
        Position GetHitSpherePointFor(Position const& dest) const;
        void GetHitSpherePointFor(Position const& dest, float& x, float& y, float& z) const;
        bool isInFront(WorldObject const* target, float arc = M_PI) const;
        bool isInBack(WorldObject const* target, float arc = M_PI) const;
        
        bool GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D = true) const;
        bool IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D = true) const;

        virtual void CleanupsBeforeDelete(bool finalCleanup = true);  // used in destructor or explicitly before mass creature delete to remove cross-references to already deleted units

        virtual void SendMessageToSet(WorldPacket const* data, bool self);
        virtual void SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool includeMargin = false, Player const* skipped_rcvr = nullptr);
        virtual void SendMessageToSet(WorldPacket const* data, Player* skipped_rcvr);
        void BuildHeartBeatMsg(WorldPacket* data) const;

		virtual uint8 GetLevelForTarget(WorldObject const* /*target*/) const { return 1; }

        void SendObjectDeSpawnAnim(ObjectGuid guid);

        virtual void SaveRespawnTime(uint32 /*forceDelay*/ = 0, bool /*saveToDB*/ = true) { }
        void AddObjectToRemoveList();

		float GetGridActivationRange() const;
		float GetVisibilityRange() const;
		float GetSightRange(WorldObject const* target = nullptr) const;
        // @tolerance: Absolute distance added to detect range
		bool CanSeeOrDetect(WorldObject const* obj, bool ignoreStealth = false, bool distanceCheck = false, bool checkAlert = false, float tolerance = 0.0f) const;

		FlaggedValuesArray32<int32, uint32, StealthType, TOTAL_STEALTH_TYPES> m_stealth;
		FlaggedValuesArray32<int32, uint32, StealthType, TOTAL_STEALTH_TYPES> m_stealthDetect;

		FlaggedValuesArray32<int32, uint32, InvisibilityType, TOTAL_INVISIBILITY_TYPES> m_invisibility;
		FlaggedValuesArray32<int32, uint32, InvisibilityType, TOTAL_INVISIBILITY_TYPES> m_invisibilityDetect;

		FlaggedValuesArray32<int32, uint32, ServerSideVisibilityType, TOTAL_SERVERSIDE_VISIBILITY_TYPES> m_serverSideVisibility;
		FlaggedValuesArray32<int32, uint32, ServerSideVisibilityType, TOTAL_SERVERSIDE_VISIBILITY_TYPES> m_serverSideVisibilityDetect;

        // Low Level Packets
        void PlayDistanceSound(uint32 sound_id, Player* target = nullptr);
        void PlayDirectSound(uint32 Sound, Player* target = nullptr);
        
        template <typename Container>
        void GetGameObjectListWithEntryInGrid(Container& gameObjectContainer, uint32 entry, float maxSearchRange = 250.0f) const;

        template <typename Container>
        void GetCreatureListWithEntryInGrid(Container& creatureContainer, uint32 entry, float maxSearchRange = 250.0f) const;

        template <typename Container>
        void GetPlayerListInGrid(Container& playerContainer, float maxSearchRange) const;

        Map const* GetBaseMap() const;
		Map* GetMap() const { ASSERT(m_currMap); return m_currMap; }
		Map* FindMap() const { return m_currMap; }

		TempSummon* SummonCreature(uint32 id, Position const& pos, TempSummonType spwtype = TEMPSUMMON_MANUAL_DESPAWN, uint32 despwtime = 0) const;
		TempSummon* SummonCreature(uint32 id, float x, float y, float z, float ang, TempSummonType spwtype = TEMPSUMMON_MANUAL_DESPAWN, uint32 despwtime = 0) const;
        GameObject* SummonGameObject(uint32 entry, Position const& pos, G3D::Quat const& rot, uint32 respawnTime, GOSummonType summonType = GO_SUMMON_TIMED_OR_CORPSE_DESPAWN) const;
        GameObject* SummonGameObject(uint32 entry, float x, float y, float z, float ang, G3D::Quat const& rot, uint32 respawnTime /* s */);
        Creature*   SummonTrigger(float x, float y, float z, float ang, uint32 dur, CreatureAI* (*GetAI)(Creature*) = nullptr);
        void        SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list = nullptr);

        Creature*   FindNearestCreature(uint32 entry, float range, bool alive = true) const;
        GameObject* FindNearestGameObject(uint32 entry, float range) const;
        GameObject* FindNearestGameObjectOfType(GameobjectTypes type, float range) const;
        Player* SelectNearestPlayer(float distance, bool alive = true) const;

        virtual ObjectGuid GetOwnerGUID() const = 0;
        virtual ObjectGuid GetCharmerOrOwnerGUID() const { return GetOwnerGUID(); }
        ObjectGuid GetCharmerOrOwnerOrOwnGUID() const;

        Unit* GetOwner() const;
        Unit* GetCharmerOrOwner() const;
        Unit* GetCharmerOrOwnerOrSelf() const;
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
        Player* GetAffectingPlayer() const;

        Player* GetSpellModOwner() const;
        int32 CalculateSpellDamage(SpellInfo const* spellInfo, uint8 effIndex, int32 const* basePoints = nullptr) const;

        // target dependent range checks
        float GetSpellMaxRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;
        float GetSpellMinRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;

        float ApplyEffectModifiers(SpellInfo const* spellInfo, uint8 effIndex, float value) const;
        int32 CalcSpellDuration(SpellInfo const* spellInfo) const;
        int32 ModSpellDuration(SpellInfo const* spellInfo, WorldObject const* target, int32 duration, bool positive, uint32 effectMask) const;
        void ModSpellCastTime(SpellInfo const* spellInfo, int32& castTime, Spell* spell = nullptr) const;
        void ModSpellDurationTime(SpellInfo const* spellInfo, int32& durationTime, Spell* spell = nullptr) const;

        virtual float MeleeSpellMissChance(Unit const* victim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const;
        virtual SpellMissInfo MeleeSpellHitResult(Unit* victim, SpellInfo const* spellInfo) const;
        SpellMissInfo MagicSpellHitResult(Unit* victim, SpellInfo const* spellInfo, Item* castItem = nullptr) const;
        SpellMissInfo SpellHitResult(Unit* victim, SpellInfo const* spellInfo, bool canReflect = false, Item* castItem = nullptr) const;
        void SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo);

        //Force ALL spells to use hit result missInfo. You should use CastSpell arguments whenever possible
        void ForceSpellHitResultOverride(SpellMissInfo missInfo) { _forceHitResultOverride = missInfo; }
        SpellMissInfo GetForceSpellHitResultOverride() const { return _forceHitResultOverride; }
        void ResetForceSpellHitResultOverride() { _forceHitResultOverride = SPELL_FORCE_HIT_DEFAULT; }

        virtual uint32 GetFaction() const = 0;
        virtual void SetFaction(uint32 /*faction*/) { }
        FactionTemplateEntry const* GetFactionTemplateEntry() const;

        ReputationRank GetReactionTo(WorldObject const* target) const;
        //sun: removed static for an added check inside
        /*static*/ ReputationRank GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, WorldObject const* target) const;

        bool IsHostileTo(WorldObject const* target) const;
        bool IsHostileToPlayers() const;
        bool IsFriendlyTo(WorldObject const* target) const;
        bool IsNeutralToAll() const;

        // CastSpell's third arg can be a variety of things - check out CastSpellExtraArgs' constructors!
        uint32 CastSpell(SpellCastTargets const& targets, uint32 spellId, CastSpellExtraArgs const& args = { });
        uint32 CastSpell(WorldObject* target, uint32 spellId, CastSpellExtraArgs const& args = { });
        uint32 CastSpell(Position const& dest, uint32 spellId, CastSpellExtraArgs const& args = { });

        bool IsValidAttackTarget(WorldObject const* target, SpellInfo const* bySpell = nullptr) const;
        bool IsValidAssistTarget(WorldObject const* target, SpellInfo const* bySpell = nullptr) const;

        Unit* GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo);

        bool isActiveObject() const { return m_isActive; }
        /** Old setActive. Force an object to be considered as active. An active object will keep a grid loaded an make every other objects around in grid being updated as well (= cause VisitAllObjects).
        So when using this, don't forget to set it as false as soon as you don't need it anymore.
        */
        void SetKeepActive(bool isActiveObject);
        bool IsFarVisible() const { return m_isFarVisible; }
        void SetFarVisible(bool on);
        void SetWorldObject(bool apply);
		bool IsPermanentWorldObject() const { return m_isWorldObject; }
		bool IsWorldObject() const;

        bool m_isTempWorldObject;

		void DestroyForNearbyPlayers();
		virtual void UpdateObjectVisibility(bool forced = true);
        virtual void UpdateObjectVisibilityOnCreate() { UpdateObjectVisibility(true); }
        
        MovementInfo m_movementInfo;
        
        uint32  LastUsedScriptID;

        void SetFallTime(uint32 time) { m_movementInfo.SetFallTime(time); }
        void AddUnitMovementFlag(uint32 f) { m_movementInfo.flags |= f; }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.flags &= ~f; }
        bool HasUnitMovementFlag(uint32 f) const { return (m_movementInfo.flags & f) == f; }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.flags; }
        void SetUnitMovementFlags(uint32 f) { m_movementInfo.flags = f; }

        void AddExtraUnitMovementFlag(uint16 f) { m_movementInfo.flags2 |= f; }
        void RemoveExtraUnitMovementFlag(uint16 f) { m_movementInfo.flags2 &= ~f; }
        bool HasExtraUnitMovementFlag(uint16 f) const { return (m_movementInfo.flags2 & f) == f; }
        uint16 GetExtraUnitMovementFlags() const { return m_movementInfo.flags2; }
        void SetExtraUnitMovementFlags(uint16 f) { m_movementInfo.flags2 = f; }

        // Transports
        Transport* GetTransport() const { return m_transport; }
        float GetTransOffsetX() const { return m_movementInfo.transport.pos.GetPositionX(); }
        float GetTransOffsetY() const { return m_movementInfo.transport.pos.GetPositionY(); }
        float GetTransOffsetZ() const { return m_movementInfo.transport.pos.GetPositionZ(); }
        float GetTransOffsetO() const { return m_movementInfo.transport.pos.GetOrientation(); }
        uint32 GetTransTime()   const { return m_movementInfo.transport.time; }
        virtual ObjectGuid GetTransGUID() const;
        void SetTransport(Transport* t);
        void SetTransOffset(float x, float y, float z, float o = 0.0f) { m_movementInfo.transport.pos.Relocate(x, y, z, o); }
        void SetTransTime(uint32 time) { m_movementInfo.transport.time = time; }
#ifdef LICH_KING
        void SetTransSeat(int8 seat) { m_movementInfo.transport.seat = seat; }
#endif
        void SetTransGUID(ObjectGuid guid) { m_movementInfo.transport.guid = guid; }

        virtual float GetStationaryX() const { return GetPositionX(); }
        virtual float GetStationaryY() const { return GetPositionY(); }
        virtual float GetStationaryZ() const { return GetPositionZ(); }
        virtual float GetStationaryO() const { return GetOrientation(); }

        void UpdatePositionData(bool updateCreatureLiquid = false);
        float GetFloorZ() const;
        virtual float GetCollisionHeight() const { return 0.0f; }
        float GetMapWaterOrGroundLevel(float x, float y, float z, float* ground = nullptr) const;
        float GetMapHeight(float x, float y, float z, bool vmap = true, float distanceToSearch = 50.0f) const; // DEFAULT_HEIGHT_SEARCH in map.h

        // Event handler
        EventProcessor m_Events;

        void BuildUpdate(UpdateDataMapType&, UpdatePlayerSet& player_set) override;

		void AddToObjectUpdate() override;
		void RemoveFromObjectUpdate() override;

		//relocation and visibility system functions
		void AddToNotify(uint16 f) { m_notifyflags |= f; }
		bool isNeedNotify(uint16 f) const { return (m_notifyflags & f) != 0; }
		uint16 GetNotifyFlags() const { return m_notifyflags; }
		bool NotifyExecuted(uint16 f) const { return (m_executed_notifies & f) != 0; }
		void SetNotified(uint16 f) { m_executed_notifies |= f; }
		void ResetAllNotifies() { m_notifyflags = 0; m_executed_notifies = 0; }

    protected:
        explicit WorldObject(bool isWorldObject); //note: here it means if it is in grid object list or world object list
        std::string m_name;
		bool const m_isWorldObject;
		ZoneScript* m_zoneScript;
        bool m_isActive;
        bool m_isFarVisible;

		//these functions are used mostly for Relocate() and Corpse/Player specific stuff...
		//use them ONLY in LoadFromDB()/Create() funcs and nowhere else!
		//mapId/instanceId should be set in SetMap() function!
		void SetLocationMapId(uint32 _mapId) { m_mapId = _mapId; }
		void SetLocationInstanceId(uint32 _instanceId) { m_InstanceId = _instanceId; }

		virtual bool IsNeverVisible() const { return !IsInWorld(); }
		virtual bool IsAlwaysVisibleFor(WorldObject const* /*seer*/) const { return false; }
		virtual bool IsInvisibleDueToDespawn() const { return false; }
		//difference from IsAlwaysVisibleFor: 1. after distance check; 2. use owner or charmer as seer
		virtual bool IsAlwaysDetectableFor(WorldObject const* /*seer*/) const { return false; }

        // transports
        Transport* m_transport;

        virtual void ProcessPositionDataChanged(PositionFullTerrainStatus const& data, bool updateCreatureLiquid = false);
        uint32 m_zoneId;
        uint32 m_areaId;
        float m_staticFloorZ;
        bool m_outdoors;

        virtual bool CanNeverSee(WorldObject const* obj) const;
        virtual bool CanAlwaysSee(WorldObject const* /*obj*/) const { return false; }

    private:
        Map*   m_currMap;                                   //current object's Map location
		uint32 m_InstanceId;                                // in map copy with instance id
        uint32 m_phaseMask;                                 // in area phase state

		uint16 m_notifyflags;
		uint16 m_executed_notifies;
        virtual bool _IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D, bool incOwnRadius = true, bool incTargetRadius = true) const;

        bool mSemaphoreTeleport;

		bool CanDetect(WorldObject const* obj, bool ignoreStealth, bool checkAlert = false, float tolerance = 0.0f) const;
		bool CanDetectInvisibilityOf(WorldObject const* obj) const;
		bool CanDetectStealthOf(WorldObject const* obj, bool checkAlert = false, float tolerance = 0.0f) const;

        SpellMissInfo _forceHitResultOverride;
};

namespace Trinity
{
    // Binary predicate to sort WorldObjects based on the distance to a reference WorldObject
    class ObjectDistanceOrderPred
    {
        public:
            ObjectDistanceOrderPred(const WorldObject* pRefObj, bool ascending = true) : m_refObj(pRefObj), m_ascending(ascending) {}
            bool operator()(const WorldObject* pLeft, const WorldObject* pRight) const
            {
                return m_ascending ? m_refObj->GetDistanceOrder(pLeft, pRight) : !m_refObj->GetDistanceOrder(pLeft, pRight);
            }
        private:
            const WorldObject* m_refObj;
            const bool m_ascending;
    };
}

#endif
