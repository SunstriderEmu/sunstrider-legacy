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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include "Common.h"
#include "ByteBuffer.h"
#include "UpdateFields.h"
#include "UpdateData.h"
#include "GameSystem/GridReference.h"
#include "ObjectDefines.h"
#include "GridDefines.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "UpdateMask.h"

class CreatureAI;

#include <set>
#include <string>

#define CONTACT_DISTANCE            0.5f
#define INTERACTION_DISTANCE        5.0f
#define ATTACK_DISTANCE             5.0f
#define MAX_VISIBILITY_DISTANCE     333.0f      // max distance for visible object show, limited in 333 yards
#define DEFAULT_VISIBILITY_DISTANCE 90.0f       // default visible distance, 90 yards on continents
#define DEFAULT_VISIBILITY_INSTANCE 120.0f      // default visible distance in instances, 120 yards
#define DEFAULT_VISIBILITY_BGARENAS 180.0f      // default visible distance in BG/Arenas, 180 yards

#define DEFAULT_WORLD_OBJECT_SIZE   0.388999998569489f      // player size, also currently used (correctly?) for any non Unit world objects
#define MAX_STEALTH_DETECT_RANGE    45.0f
#define DEFAULT_COMBAT_REACH        1.5f
#define MIN_MELEE_REACH             2.0f
#define NOMINAL_MELEE_RANGE         5.0f
#define MELEE_RANGE                 (NOMINAL_MELEE_RANGE - MIN_MELEE_REACH * 2) //center to center for players

uint32 GuidHigh2TypeId(uint32 guid_hi);

enum TempSummonType
{
    TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1,             // despawns after a specified time OR when the creature disappears
    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2,             // despawns after a specified time OR when the creature dies
    TEMPSUMMON_TIMED_DESPAWN               = 3,             // despawns after a specified time
    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4,             // despawns after a specified time after the creature is out of combat
    TEMPSUMMON_CORPSE_DESPAWN              = 5,             // despawns instantly after death
    TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6,             // despawns after a specified time after death
    TEMPSUMMON_DEAD_DESPAWN                = 7,             // despawns when the creature disappears
    TEMPSUMMON_MANUAL_DESPAWN              = 8              // despawns when UnSummon() is called
};

class WorldPacket;
class UpdateData;
class WorldSession;
class Creature;
class Player;
class InstanceData;
class GameObject;
class Transport;
class WorldObject;

typedef std::unordered_map<Player*, UpdateData> UpdateDataMapType;

#define POSITION_GET_X_Y_Z(a) a->GetPositionX(), a->GetPositionY(), a->GetPositionZ()

struct Position
{
    Position(float x = 0, float y = 0, float z = 0, float o = 0)
        : m_positionX(x), m_positionY(y), m_positionZ(z), m_orientation(NormalizeOrientation(o)) { }

    Position(const WorldObject* obj);

    Position(const Position &loc) { Relocate(loc); }

    struct PositionXYZStreamer
    {
        explicit PositionXYZStreamer(Position& pos) : m_pos(&pos) {}
        Position* m_pos;
    };

    struct PositionXYZOStreamer
    {
        explicit PositionXYZOStreamer(Position& pos) : m_pos(&pos) {}
        Position* m_pos;
    };

    float m_positionX;
    float m_positionY;
    float m_positionZ;
    float m_orientation;

    void Relocate(float x, float y)
        { m_positionX = x; m_positionY = y;}
    void Relocate(float x, float y, float z)
        { m_positionX = x; m_positionY = y; m_positionZ = z; }
    void Relocate(float x, float y, float z, float orientation)
        { m_positionX = x; m_positionY = y; m_positionZ = z; m_orientation = orientation; }
    void Relocate(const Position &pos)
        { m_positionX = pos.m_positionX; m_positionY = pos.m_positionY; m_positionZ = pos.m_positionZ; m_orientation = pos.m_orientation; }
    void Relocate(const Position *pos)
        { m_positionX = pos->m_positionX; m_positionY = pos->m_positionY; m_positionZ = pos->m_positionZ; m_orientation = pos->m_orientation; }
    void RelocateOffset(const Position &offset);
    virtual void SetOrientation(float orientation)
        { m_orientation = orientation; }

    float GetPositionX() const { return m_positionX; }
    float GetPositionY() const { return m_positionY; }
    float GetPositionZ() const { return m_positionZ; }
    float GetOrientation() const { return m_orientation; }

    void GetPosition(float &x, float &y) const
        { x = m_positionX; y = m_positionY; }
    void GetPosition(float &x, float &y, float &z) const
        { x = m_positionX; y = m_positionY; z = m_positionZ; }
    void GetPosition(float &x, float &y, float &z, float &o) const
        { x = m_positionX; y = m_positionY; z = m_positionZ; o = m_orientation; }

    Position GetPosition() const
    {
        return *this;
    }

    Position::PositionXYZStreamer PositionXYZStream()
    {
        return Position::PositionXYZStreamer(*this);
    }
    Position::PositionXYZOStreamer PositionXYZOStream()
    {
        return Position::PositionXYZOStreamer(*this);
    }

    bool IsPositionValid() const;

    float GetExactDist2dSq(float x, float y) const
        { float dx = m_positionX - x; float dy = m_positionY - y; return dx*dx + dy*dy; }
    float GetExactDist2d(const float x, const float y) const
        { return sqrt(GetExactDist2dSq(x, y)); }
    float GetExactDist2dSq(const Position *pos) const
        { float dx = m_positionX - pos->m_positionX; float dy = m_positionY - pos->m_positionY; return dx*dx + dy*dy; }
    float GetExactDist2d(const Position *pos) const
        { return sqrt(GetExactDist2dSq(pos)); }
    float GetExactDistSq(float x, float y, float z) const
        { float dz = m_positionZ - z; return GetExactDist2dSq(x, y) + dz*dz; }
    float GetExactDist(float x, float y, float z) const
        { return sqrt(GetExactDistSq(x, y, z)); }
    float GetExactDistSq(const Position *pos) const
        { float dx = m_positionX - pos->m_positionX; float dy = m_positionY - pos->m_positionY; float dz = m_positionZ - pos->m_positionZ; return dx*dx + dy*dy + dz*dz; }
    float GetExactDist(const Position *pos) const
        { return sqrt(GetExactDistSq(pos)); }

    void GetPositionOffsetTo(const Position & endPos, Position & retOffset) const;

    float GetAngle(const Position *pos) const;
    float GetAngle(float x, float y) const;
    float GetRelativeAngle(const Position *pos) const
        { return GetAngle(pos) - m_orientation; }
    float GetRelativeAngle(float x, float y) const { return GetAngle(x, y) - m_orientation; }
    void GetSinCos(float x, float y, float &vsin, float &vcos) const;

    bool IsInDist2d(float x, float y, float dist) const
        { return GetExactDist2dSq(x, y) < dist * dist; }
    bool IsInDist2d(const Position *pos, float dist) const
        { return GetExactDist2dSq(pos) < dist * dist; }
    bool IsInDist(float x, float y, float z, float dist) const
        { return GetExactDistSq(x, y, z) < dist * dist; }
    bool IsInDist(const Position *pos, float dist) const
        { return GetExactDistSq(pos) < dist * dist; }
    bool HasInArc(float arcangle, const Position *pos, float border = 2.0f) const;
    bool HasInLine(const Unit* target, float width) const;
    std::string ToString() const;

    // modulos a radian orientation to the range of 0..2PI
    static float NormalizeOrientation(float o)
    {
        // fmod only supports positive numbers. Thus we have
        // to emulate negative numbers
        if (o < 0)
        {
            float mod = o *-1;
            mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
            mod = -mod + 2.0f * static_cast<float>(M_PI);
            return mod;
        }
        return fmod(o, 2.0f * static_cast<float>(M_PI));
    }
};
ByteBuffer &operator>>(ByteBuffer& buf, Position::PositionXYZOStreamer const & streamer);
ByteBuffer & operator<<(ByteBuffer& buf, Position::PositionXYZStreamer const & streamer);
ByteBuffer &operator>>(ByteBuffer& buf, Position::PositionXYZStreamer const & streamer);
ByteBuffer & operator<<(ByteBuffer& buf, Position::PositionXYZOStreamer const & streamer);


struct MovementInfo
{
    // common
    uint32 flags;
    uint8 flags2; //not used on BC
    Position pos;
    uint32 time;

    // transport
    struct TransportInfo
    {
        void Reset()
        {
            guid = 0;
            pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
            time = 0;
        }

        uint64 guid;
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
        flags(0), flags2(0), time(0), pitch(0.0f), fallTime(0), splineElevation(0.0f)
    {
        pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        transport.Reset();
        jump.Reset();
    }

    // Read/Write methods
    void Read(ByteBuffer& data);
    void Write(ByteBuffer& data) const;

    uint32 GetMovementFlags() const { return flags; }
    void SetMovementFlags(uint32 flag) { flags = flag; }
    void AddMovementFlag(uint32 flag) { flags |= flag; }
    void RemoveMovementFlag(uint32 flag) { flags &= ~flag; }
    bool HasMovementFlag(uint32 flag) const { return flags & flag; }

    void SetFallTime(uint32 time) { fallTime = time; }
};

inline ByteBuffer& operator<< (ByteBuffer& buf, MovementInfo const& mi)
{
    mi.Write(buf);
    return buf;
}

inline ByteBuffer& operator>> (ByteBuffer& buf, MovementInfo& mi)
{
    mi.Read(buf);
    return buf;
}

#define MAPID_INVALID 0xFFFFFFFF

class WorldLocation : public Position
{
    public:
        explicit WorldLocation(uint32 _mapid = MAPID_INVALID, float _x = 0, float _y = 0, float _z = 0, float _o = 0)
            : m_mapId(_mapid) { Relocate(_x, _y, _z, _o); }
        WorldLocation(const WorldLocation &loc) { WorldRelocate(loc); }

        void WorldRelocate(const WorldLocation &loc)
            { m_mapId = loc.GetMapId(); Relocate(loc); }
        uint32 GetMapId() const { return m_mapId; }

        uint32 m_mapId;
};


class Object
{
    public:
        virtual ~Object ( );
        
        const bool& IsInWorld() const { return m_inWorld; }
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

        uint64 GetGUID() const { return GetUInt64Value(OBJECT_FIELD_GUID); }
        uint32 GetGUIDLow() const { return GUID_LOPART(GetUInt64Value(OBJECT_FIELD_GUID)); }
        uint32 GetGUIDMid() const { return GUID_ENPART(GetUInt64Value(OBJECT_FIELD_GUID)); }
        uint32 GetGUIDHigh() const { return GUID_HIPART(GetUInt64Value(OBJECT_FIELD_GUID)); }
        PackedGuid const& GetPackGUID() const { return m_PackGUID; }
        uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
        void SetEntry(uint32 entry) { SetUInt32Value(OBJECT_FIELD_ENTRY, entry); }

        float GetObjectScale() const { return GetFloatValue(OBJECT_FIELD_SCALE_X); }
        virtual void SetObjectScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); }

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
        /**
            Adds the movement update block data to update data
            (not sure of how this should be used, update this comment if you know)
        */
        void BuildMovementUpdateBlock(UpdateData* data, uint32 flags = 0) const;

        virtual void DestroyForPlayer(Player *target, bool onDeath = false) const;

        int32 GetInt32Value( uint16 index ) const;
        uint32 GetUInt32Value( uint16 index ) const;
        uint64 GetUInt64Value( uint16 index ) const;
        float GetFloatValue( uint16 index ) const;
        uint8 GetByteValue( uint16 index, uint8 offset) const;
        uint16 GetUInt16Value(uint16 index, uint8 offset) const;
        uint64 GetGuidValue(uint16 index) const; //for TC compat

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
        void SetGuidValue(uint16 index, uint64 value); //for TC compat

        void ApplyModUInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModInt32Value(uint16 index, int32 val, bool apply);
        void ApplyModUInt64Value(uint16 index, int32 val, bool apply);
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

        virtual bool hasQuest(uint32 /* quest_id */) const { return false; }
        virtual bool hasInvolvedQuest(uint32 /* quest_id */) const { return false; }

        /** 
            Visits cells around the object, fill players UpdateData with updates from this object if needed
        */
        virtual void BuildUpdate(UpdateDataMapType&) { }
        /**
           Adds the player and update data for him to the given updateData map. 
           Creates the update map for him if it doesn't exists, else exists the already existing one.
        */
        void BuildFieldsUpdate(Player*, UpdateDataMapType& data_map) const;

        void SetFieldNotifyFlag(uint16 flag) { _fieldNotifyFlags |= flag; }
        void RemoveFieldNotifyFlag(uint16 flag) { _fieldNotifyFlags &= uint16(~flag); }

        // FG: some hacky helpers
        void ForceValuesUpdateAtIndex(uint32);
        
        Player* ToPlayer() { if (GetTypeId() == TYPEID_PLAYER) return reinterpret_cast<Player*>(this); else return NULL; }
        Player const* ToPlayer() const { if (GetTypeId() == TYPEID_PLAYER) return reinterpret_cast<Player const*>(this); else return NULL; }

        Creature* ToCreature() { if (GetTypeId() == TYPEID_UNIT) return reinterpret_cast<Creature*>(this); else return NULL; }
        Creature const* ToCreature() const { if (GetTypeId() == TYPEID_UNIT) return reinterpret_cast<Creature const*>(this); else return NULL; }

        Unit* ToUnit() { if (isType(TYPEMASK_UNIT)) return reinterpret_cast<Unit*>(this); else return NULL; }
        Unit const* ToUnit() const { if (isType(TYPEMASK_UNIT)) return reinterpret_cast<Unit const*>(this); else return NULL; }

        GameObject* ToGameObject() { if (GetTypeId() == TYPEID_GAMEOBJECT) return reinterpret_cast<GameObject*>(this); else return NULL; }
        GameObject const* ToGameObject() const { if (GetTypeId() == TYPEID_GAMEOBJECT) return reinterpret_cast<GameObject const*>(this); else return NULL; }

        Corpse* ToCorpse() { if (GetTypeId() == TYPEID_CORPSE) return reinterpret_cast<Corpse*>(this); else return NULL; }
        Corpse const* ToCorpse() const { if (GetTypeId() == TYPEID_CORPSE) return reinterpret_cast<Corpse const*>(this); else return NULL; }

        DynamicObject* ToDynObject() { if (GetTypeId() == TYPEID_DYNAMICOBJECT) return reinterpret_cast<DynamicObject*>(this); else return NULL; }
        DynamicObject const* ToDynObject() const { if (GetTypeId() == TYPEID_DYNAMICOBJECT) return reinterpret_cast<DynamicObject const*>(this); else return NULL; }

        //dont use, used by map only
        Cell const& GetCurrentCell() const { return _currentCell; }
        void SetCurrentCell(Cell const& cell) { _currentCell = cell; }
    protected:

        Object();

        void _InitValues();
        void _Create(uint32 guidlow, uint32 entry, HighGuid guidhigh);
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

        bool m_objectUpdated;

    private:
        //only used for creatures & gobject for now
        Cell _currentCell;

        bool m_inWorld;

        PackedGuid m_PackGUID;

        // for output helpfull error messages from asserts
        bool PrintIndexError(uint32 index, bool set) const;
        Object(const Object&);                              // prevent generation copy constructor
        Object& operator=(Object const&);                   // prevent generation assigment operator
};

class WorldObject : public Object, public WorldLocation
{
    public:
        virtual ~WorldObject ( ) {}

        virtual void Update ( uint32 /*time_diff*/ ) { }

        void _Create( uint32 guidlow, HighGuid guidhigh, uint32 mapid );

        void GetNearPoint2D( float &x, float &y, float distance, float absAngle) const;
        void GetNearPoint( WorldObject const* searcher, float &x, float &y, float &z, float searcher_size, float distance2d,float absAngle) const;
        void GetClosePoint(float &x, float &y, float &z, float searcherSize, float distance2d = 0, float angle = 0) const
        {
            // angle calculated from current orientation
            GetNearPoint(NULL,x,y,z,searcherSize,distance2d,GetOrientation() + angle);
        }
        void MovePosition(Position &pos, float dist, float angle);
        void GetGroundPoint(float &x, float &y, float &z, float dist, float angle);
        void GetGroundPointAroundUnit(float &x, float &y, float &z, float dist, float angle)
        {
            GetPosition(x, y, z);
            GetGroundPoint(x, y, z, dist, angle);
        }
        void GetContactPoint( const WorldObject* obj, float &x, float &y, float &z, float distance2d = CONTACT_DISTANCE) const
        {
            // angle to face `obj` to `this` using distance includes size of `obj`
            GetNearPoint(obj,x,y,z,obj->GetObjectSize(),distance2d,GetAngle( obj ));
        }
        Position GetFirstCollisionPosition(float dist, float angle, bool keepZ = false);
        void MovePositionToFirstCollision(Position &pos, float dist, float angle, bool keepZ = false);
        Position GetRandomNearPosition(float radius);
        Position GetNearPosition(float dist, float angle);

        float GetObjectSize() const;
        bool IsPositionValid() const;
        void UpdateGroundPositionZ(float x, float y, float &z) const;
        void UpdateAllowedPositionZ(float x, float y, float &z, float maxDist = 50.0f) const;
        static void UpdateAllowedPositionZ(uint32 mapId, float x, float y, float &z, bool canSwim, bool canFly, bool waterWalk, float maxDist = 50.0f, Position* collisionFrom = nullptr);

        void GetRandomPoint( const Position &pos, float distance, float &rand_x, float &rand_y, float &rand_z ) const;

        void SetMapId(uint32 newMap) { m_mapId = newMap; m_map = NULL; }

        void SetInstanceId(uint32 val) { m_InstanceId = val; m_map = NULL; }
        uint32 GetInstanceId() const { return m_InstanceId; }

        uint32 GetZoneId() const;
        uint32 GetAreaId() const;

        //for trinitycore compatibility
        PhaseMask GetPhaseMask() const { return PhaseMask(1); }

        InstanceData* GetInstanceData();

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
        bool IsInMap(const WorldObject* obj) const { return GetMapId()==obj->GetMapId() && GetInstanceId()==obj->GetInstanceId(); }
        bool IsWithinDist3d(float x, float y, float z, float dist) const;
        bool IsWithinDist3d(Position const* pos, float dist) const;
        bool IsWithinDist2d(float x, float y, float dist) const;
        bool IsWithinDist2d(Position const* pos, float dist) const;
        bool IsWithinDistInMap(const WorldObject* obj, const float dist2compare, const bool is3D = true) const;
        bool IsWithinLOS(const float x, const float y, const float z ) const;
        bool IsWithinLOSInMap(const WorldObject* obj) const;
        bool isInFront(WorldObject const* target, float arc = M_PI) const;
        bool isInBack(WorldObject const* target, float arc = M_PI) const;
        
        bool GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D = true) const;
        bool IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D = true) const;

        virtual void CleanupsBeforeDelete(bool finalCleanup = true);  // used in destructor or explicitly before mass creature delete to remove cross-references to already deleted units

        virtual void SendMessageToSet(WorldPacket *data, bool self, bool to_possessor = true);
        virtual void SendMessageToSetInRange(WorldPacket *data, float dist, bool self, bool to_possessor = true);
        virtual void SendMessageToSet(WorldPacket* data, Player* skipped_rcvr);
        void BuildHeartBeatMsg( WorldPacket *data ) const;

        void MonsterSay(const char* text, uint32 language, uint64 TargetGuid);
        void MonsterYell(const char* text, uint32 language, uint64 TargetGuid);
        void MonsterTextEmote(const char* text, uint64 TargetGuid, bool IsBossEmote = false, float dist = 0, bool IsServerEmote = false);
        void MonsterWhisper(const char* text, uint64 receiver, bool IsBossWhisper = false);
        void MonsterSay(int32 textId, uint32 language, uint64 TargetGuid);
        void MonsterYell(int32 textId, uint32 language, uint64 TargetGuid);
        void MonsterTextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote = false);
        void MonsterWhisper(int32 textId, uint64 receiver, bool IsBossWhisper = false);
        void BuildMonsterChat(WorldPacket *data, uint8 msgtype, char const* text, uint32 language, char const* name, uint64 TargetGuid) const;

        void SendObjectDeSpawnAnim(uint64 guid);

        virtual void SaveRespawnTime() {}

        void AddObjectToRemoveList();

        // main visibility check function in normal case (ignore grey zone distance check)
        bool isVisibleFor(Player const* u) const { return IsVisibleForInState(u,false); }

        // low level function for visibility change code, must be define in all main world object subclasses
        virtual bool IsVisibleForInState(Player const* u, bool inVisibleList) const = 0;

        // Low Level Packets
        void SendPlaySound(uint32 Sound, bool OnlySelf);
        
        void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList, uint32 uiEntry, float fMaxSearchRange);
        void GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange);

        Map      * GetMap() const   { return m_map ? m_map : const_cast<WorldObject*>(this)->_getMap(); }
        Map      * FindMap() const  { return m_map ? m_map : const_cast<WorldObject*>(this)->FindBaseMap(); }
        Map const* GetBaseMap() const;
        Creature* SummonCreature(uint32 id, float x, float y, float z, float ang,TempSummonType spwtype,uint32 despwtime);
        GameObject* SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime);
        Creature* SummonTrigger(float x, float y, float z, float ang, uint32 dur, CreatureAI* (*GetAI)(Creature*) = NULL);
        Creature*   FindNearestCreature(uint32 entry, float range, bool alive = true);
        GameObject* FindNearestGameObject(uint32 entry, float range);
        Player* FindNearestPlayer(float range);
        bool isActiveObject() const { return m_isActive; }
        /** Old setActive. Force an object to be considered as active. An active object will keep a grid loaded an make every other objects around in grid being updated as well (= cause VisitNearbyObject).
        So when using this, don't forget to set it as false as soon as you don't need it anymore.
        */
        void SetKeepActive(bool isActiveObject);
        void SetWorldObject(bool apply);
        template<class NOTIFIER> void VisitNearbyObject(const float &radius, NOTIFIER &notifier) const { GetMap()->VisitAll(GetPositionX(), GetPositionY(), radius, notifier); }
        template<class NOTIFIER> void VisitNearbyGridObject(const float &radius, NOTIFIER &notifier) const { GetMap()->VisitGrid(GetPositionX(), GetPositionY(), radius, notifier); }
        template<class NOTIFIER> void VisitNearbyWorldObject(const float &radius, NOTIFIER &notifier) const { GetMap()->VisitWorld(GetPositionX(), GetPositionY(), radius, notifier); }
        bool IsTempWorldObject;

        uint32 m_groupLootTimer;                            // (msecs)timer used for group loot
        uint64 lootingGroupLeaderGUID;                      // used to find group which is looting corpse

        
        MovementInfo m_movementInfo;
        
        uint32  LastUsedScriptID;

        // Transports
        Transport* GetTransport() const { return m_transport; }
        float GetTransOffsetX() const { return m_movementInfo.transport.pos.GetPositionX(); }
        float GetTransOffsetY() const { return m_movementInfo.transport.pos.GetPositionY(); }
        float GetTransOffsetZ() const { return m_movementInfo.transport.pos.GetPositionZ(); }
        float GetTransOffsetO() const { return m_movementInfo.transport.pos.GetOrientation(); }
        uint32 GetTransTime()   const { return m_movementInfo.transport.time; }
        virtual uint64 GetTransGUID()   const;
        void SetTransport(Transport* t) { m_transport = t; }

        virtual float GetStationaryX() const { return GetPositionX(); }
        virtual float GetStationaryY() const { return GetPositionY(); }
        virtual float GetStationaryZ() const { return GetPositionZ(); }
        virtual float GetStationaryO() const { return GetOrientation(); }

        void BuildUpdate(UpdateDataMapType&) override;
    protected:
        explicit WorldObject();
        std::string m_name;
        bool m_isActive;

        // transports
        Transport* m_transport;

    private:
        uint32 m_InstanceId;
        Map    *m_map;

        Map* _getMap();
        Map* FindBaseMap();

        bool mSemaphoreTeleport;
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
    class MessageChatLocaleCacheDo
    {
        public:
            MessageChatLocaleCacheDo(WorldObject const& obj, ChatMsg msgtype, int32 textId, uint32 language, uint64 targetGUID, float dist);
            ~MessageChatLocaleCacheDo();
            void operator()(Player* p);
        private:
            WorldObject const& i_object;
            ChatMsg i_msgtype;
            int32 i_textId;
            uint32 i_language;
            uint64 i_targetGUID;
            float i_dist;
            std::vector<WorldPacket*> i_data_cache;             // 0 = default, i => i-1 locale index
    };

    class CreatureTextLocaleDo
    {
        public:
            CreatureTextLocaleDo(WorldObject& source, WorldPacket* data_en, WorldPacket* data_fr, float dist);
            void operator()(Player* p);
        private:
            WorldObject& i_source;
            float i_dist;
            WorldPacket* i_data_en;
            WorldPacket* i_data_fr;
    };

}

#endif
