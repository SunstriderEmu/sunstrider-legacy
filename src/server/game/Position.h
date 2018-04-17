#ifndef Trinity_game_Position_h__
#define Trinity_game_Position_h__

#include "Define.h"
#include <string>
#include <cmath>

class ByteBuffer;
class WorldObject;
namespace G3D
{
    class Vector3;
};

struct TC_GAME_API Position
{
    Position(float x = 0.0f, float y = 0.0f, float z = 0.0f, float o = 0.0f)
        : m_positionX(x), m_positionY(y), m_positionZ(z), m_orientation(NormalizeOrientation(o)) { }

    Position(const WorldObject* obj);

    Position(const Position &loc) { Relocate(loc); }

    Position(G3D::Vector3 const& vect);

    struct XY;
    struct XYZ;
    struct XYZO;
    struct PackedXYZ;

    template <class Tag>
    struct ConstStreamer
    {
        explicit ConstStreamer(Position const& pos) : Pos(&pos) { }
        Position const* Pos;
    };

    template <class Tag>
    struct Streamer
    {
        explicit Streamer(Position& pos) : Pos(&pos) { }
        operator ConstStreamer<Tag>() const { return ConstStreamer<Tag>(*Pos); }
        Position* Pos;
    };

    float m_positionX;
    float m_positionY;
    float m_positionZ;
    float m_orientation;
    
    //these functions only change the position at server, you need to use proper opcodes or set object to notify
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
    //use SetFacingTo to send proper update to client
    virtual void SetOrientation(float orientation)
        { m_orientation = orientation; }

	inline float GetPositionX() const { return m_positionX; }
	inline float GetPositionY() const { return m_positionY; }
	inline float GetPositionZ() const { return m_positionZ; }
	inline float GetOrientation() const { return m_orientation; }

    inline void GetPosition(float &x, float &y) const
        { x = m_positionX; y = m_positionY; }
	inline void GetPosition(float &x, float &y, float &z) const
        { x = m_positionX; y = m_positionY; z = m_positionZ; }
	inline void GetPosition(float &x, float &y, float &z, float &o) const
        { x = m_positionX; y = m_positionY; z = m_positionZ; o = m_orientation; }

    Position GetPosition() const
    {
        return *this;
    }

    Streamer<XY> PositionXYStream() { return Streamer<XY>(*this); }
    ConstStreamer<XY> PositionXYStream() const { return ConstStreamer<XY>(*this); }
    Streamer<XYZ> PositionXYZStream() { return Streamer<XYZ>(*this); }
    ConstStreamer<XYZ> PositionXYZStream() const { return ConstStreamer<XYZ>(*this); }
    Streamer<XYZO> PositionXYZOStream() { return Streamer<XYZO>(*this); }
    ConstStreamer<XYZO> PositionXYZOStream() const { return ConstStreamer<XYZO>(*this); }
    Streamer<PackedXYZ> PositionPackedXYZStream() { return Streamer<PackedXYZ>(*this); }
    ConstStreamer<PackedXYZ> PositionPackedXYZStream() const { return ConstStreamer<PackedXYZ>(*this); }

    bool IsPositionValid() const;

    float GetExactDist2dSq(float x, float y) const
    { 
        float dx = m_positionX - x; float dy = m_positionY - y; return dx*dx + dy*dy; 
    }
    float GetExactDist2d(const float x, const float y) const
    { 
        return sqrt(GetExactDist2dSq(x, y)); 
    }
    float GetExactDist2dSq(const Position *pos) const
    { 
        float dx = m_positionX - pos->m_positionX; float dy = m_positionY - pos->m_positionY; return dx*dx + dy*dy; 
    }
    float GetExactDist2d(const Position *pos) const
    { 
        return sqrt(GetExactDist2dSq(pos)); 
    }
    float GetExactDistSq(float x, float y, float z) const
    { 
        float dz = m_positionZ - z; return GetExactDist2dSq(x, y) + dz*dz; 
    }
    float GetExactDist(float x, float y, float z) const
    { 
        return sqrt(GetExactDistSq(x, y, z)); 
    }
    float GetExactDistSq(const Position *pos) const
    { 
        float dx = m_positionX - pos->m_positionX; float dy = m_positionY - pos->m_positionY; float dz = m_positionZ - pos->m_positionZ; return dx*dx + dy*dy + dz*dz; 
    }
    float GetExactDist(const Position *pos) const
     { 
        return sqrt(GetExactDistSq(pos)); 
    }
    float GetExactDistSq(Position const& pos) const
    {
        float dx = m_positionX - pos.m_positionX; float dy = m_positionY - pos.m_positionY; float dz = m_positionZ - pos.m_positionZ; return dx*dx + dy*dy + dz*dz;
    }
    float GetExactDist(Position const& pos) const
    {
        return std::sqrt(GetExactDistSq(pos));
    }

    void GetPositionOffsetTo(const Position & endPos, Position & retOffset) const;
    Position GetPositionWithOffset(Position const& offset) const;

    float GetAbsoluteAngle(float x, float y) const;
    float GetAbsoluteAngle(Position const& pos) const { return GetAbsoluteAngle(pos.m_positionX, pos.m_positionY); }
    float GetAbsoluteAngle(Position const* pos) const { return GetAbsoluteAngle(*pos); }
    float ToAbsoluteAngle(float relAngle) const { return NormalizeOrientation(relAngle + m_orientation); }

    float GetRelativeAngle(float x, float y) const { return GetAbsoluteAngle(x, y) - m_orientation; }
    float GetRelativeAngle(Position const& pos) const { return ToRelativeAngle(GetAbsoluteAngle(pos)); }
    float GetRelativeAngle(Position const* pos) const { return ToRelativeAngle(GetAbsoluteAngle(pos)); }
    float ToRelativeAngle(float absAngle) const { return NormalizeOrientation(absAngle - m_orientation); }

    void GetSinCos(float x, float y, float &vsin, float &vcos) const;

    bool IsInDist2d(float x, float y, float dist) const { return GetExactDist2dSq(x, y) < dist * dist; }
    bool IsInDist2d(Position const* pos, float dist) const { return GetExactDist2dSq(pos) < dist * dist; }

    bool IsInDist(float x, float y, float z, float dist) const { return GetExactDistSq(x, y, z) < dist * dist; }
    bool IsInDist(Position const& pos, float dist) const { return GetExactDistSq(pos) < dist * dist; }
    bool IsInDist(Position const* pos, float dist) const { return GetExactDistSq(pos) < dist * dist; }

    /*
    search using this relation: dist2d < radius && abs(dz) < height
    */
    bool IsWithinDoubleVerticalCylinder(Position const* center, float radius, float height) const;
    bool HasInArc(float arcangle, const Position *pos, float border = 2.0f) const;
    bool HasInLine(Position const* pos, float objSize, float width) const;
    std::string ToString() const;

    // constrain arbitrary radian orientation to interval [0,2*PI)
    static float NormalizeOrientation(float o);

    bool operator==(Position const &a);

    inline bool operator!=(Position const &a)
    {
        return !(operator==(a));
    }

    //Use negative dist to move behind
    void MoveInFront(Position frontOf, float dist)
    {
        m_positionX = frontOf.m_positionX + dist * std::cos(frontOf.m_orientation);
        m_positionY = frontOf.m_positionY + dist * std::sin(frontOf.m_orientation);
        m_positionZ = frontOf.m_positionZ;
    }
};

#define MAPID_INVALID 0xFFFFFFFF

class TC_GAME_API WorldLocation : public Position
{
    public:
        explicit WorldLocation(uint32 _mapid = MAPID_INVALID, float _x = 0, float _y = 0, float _z = 0, float _o = 0)
            : Position(_x, _y, _z, _o), m_mapId(_mapid) { }

		WorldLocation(uint32 mapId, Position const& position)
			: Position(position), m_mapId(mapId) { }

		WorldLocation(WorldLocation const& loc)
			: Position(loc), m_mapId(loc.GetMapId()) { }

        void WorldRelocate(WorldLocation const& loc) { m_mapId = loc.GetMapId(); Relocate(loc); }
        void WorldRelocate(WorldLocation const* loc) { m_mapId = loc->GetMapId(); Relocate(loc); }

		void WorldRelocate(uint32 _mapId = MAPID_INVALID, float _x = 0.f, float _y = 0.f, float _z = 0.f, float _o = 0.f)
		{
			m_mapId = _mapId;
			Relocate(_x, _y, _z, _o);
		}

		WorldLocation GetWorldLocation() const
		{
			return *this;
		}

        uint32 GetMapId() const { return m_mapId; }

        uint32 m_mapId;
};

TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XY> const& streamer);
TC_GAME_API ByteBuffer& operator>>(ByteBuffer& buf, Position::Streamer<Position::XY> const& streamer);
TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XYZ> const& streamer);
TC_GAME_API ByteBuffer& operator>>(ByteBuffer& buf, Position::Streamer<Position::XYZ> const& streamer);
TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XYZO> const& streamer);
TC_GAME_API ByteBuffer& operator>>(ByteBuffer& buf, Position::Streamer<Position::XYZO> const& streamer);
TC_GAME_API ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::PackedXYZ> const& streamer);

template <class Tag>
struct TaggedPosition
{
    TaggedPosition(float x = 0.0f, float y = 0.0f, float z = 0.0f, float o = 0.0f) : Pos(x, y, z, o) { }
    TaggedPosition(Position const& pos) : Pos(pos) { }

    TaggedPosition& operator=(Position const& pos)
    {
        Pos.Relocate(pos);
        return *this;
    }

    operator Position() const { return Pos; }

    friend ByteBuffer& operator<<(ByteBuffer& buf, TaggedPosition const& tagged) { return buf << Position::ConstStreamer<Tag>(tagged.Pos); }
    friend ByteBuffer& operator>>(ByteBuffer& buf, TaggedPosition& tagged) { return buf >> Position::Streamer<Tag>(tagged.Pos); }

    Position Pos;
};

#endif
