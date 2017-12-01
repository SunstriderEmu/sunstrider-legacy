#ifndef Trinity_game_Position_h__
#define Trinity_game_Position_h__

#include "Define.h"
#include <string>
#include <cmath>

class ByteBuffer;

struct TC_GAME_API Position
{
    Position(float x = 0.0f, float y = 0.0f, float z = 0.0f, float o = 0.0f)
        : m_positionX(x), m_positionY(y), m_positionZ(z), m_orientation(NormalizeOrientation(o)) { }

    Position(const WorldObject* obj);

    Position(const Position &loc) { Relocate(loc); }

    Position(G3D::Vector3 const& vect);

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

    /*
    search using this relation: dist2d < radius && abs(dz) < height
    */
    bool IsWithinDoubleVerticalCylinder(Position const* center, float radius, float height) const;
    bool HasInArc(float arcangle, const Position *pos, float border = 2.0f) const;
    bool HasInLine(const WorldObject* target, float objSize, float width) const;
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
TC_GAME_API ByteBuffer &operator>>(ByteBuffer& buf, Position::PositionXYZOStreamer const & streamer);
TC_GAME_API ByteBuffer & operator<<(ByteBuffer& buf, Position::PositionXYZStreamer const & streamer);
TC_GAME_API ByteBuffer &operator>>(ByteBuffer& buf, Position::PositionXYZStreamer const & streamer);
TC_GAME_API ByteBuffer & operator<<(ByteBuffer& buf, Position::PositionXYZOStreamer const & streamer);

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

        void WorldRelocate(const WorldLocation &loc)
        { 
			m_mapId = loc.GetMapId(); 
			Relocate(loc); 
		}

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

#endif
