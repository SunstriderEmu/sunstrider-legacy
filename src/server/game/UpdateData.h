
#ifndef __UPDATEDATA_H
#define __UPDATEDATA_H

#include "ObjectGuid.h"
class WorldPacket;
enum ClientBuild : uint32;

enum OBJECT_UPDATE_TYPE
{
    UPDATETYPE_VALUES               = 0,
    UPDATETYPE_MOVEMENT             = 1,
    UPDATETYPE_CREATE_OBJECT        = 2,
    UPDATETYPE_CREATE_OBJECT2       = 3,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
    UPDATETYPE_NEAR_OBJECTS         = 5
};

enum OBJECT_UPDATE_FLAGS
{
    UPDATEFLAG_NONE                    = 0x000,
    UPDATEFLAG_SELF                    = 0x001,
    UPDATEFLAG_TRANSPORT               = 0x002,
    UPDATEFLAG_HAS_TARGET              = 0x004,
    UPDATEFLAG_LOWGUID                 = 0x008,
    UPDATEFLAG_HIGHGUID                = 0x010,
    UPDATEFLAG_LIVING                  = 0x020,
    UPDATEFLAG_STATIONARY_POSITION     = 0x040
};

enum LK_OBJECT_UPDATE_FLAGS
{
    LK_UPDATEFLAG_NONE                 = 0x000,
    LK_UPDATEFLAG_SELF                 = 0x001,
    LK_UPDATEFLAG_TRANSPORT            = 0x002,
    LK_UPDATEFLAG_HAS_TARGET           = 0x004,
    LK_UPDATEFLAG_UNKNOWN              = 0x008,
    LK_UPDATEFLAG_LOWGUID              = 0x010,
    LK_UPDATEFLAG_LIVING               = 0x020,
    LK_UPDATEFLAG_STATIONARY_POSITION  = 0x040,
    LK_UPDATEFLAG_VEHICLE              = 0x080,
    LK_UPDATEFLAG_POSITION             = 0x100,
    LK_UPDATEFLAG_ROTATION             = 0x200
};

/**
    Contains a list of changes of an object, for a given player
*/
class UpdateData
{
    public:
        UpdateData();

        void AddOutOfRangeGUID(std::set<uint64>& guids);
        void AddOutOfRangeGUID(const uint64 &guid);
        void AddUpdateBlock(const ByteBuffer &block);
        /** Build a WorldPacket from this update data 
            @packet an unitialized WorldPacket
        */
        bool BuildPacket(WorldPacket* packet, ClientBuild build, bool hasTransport = false);
        bool HasData() { return m_blockCount > 0 || !m_outOfRangeGUIDs.empty(); }
        void Clear();

        GuidSet const& GetOutOfRangeGUIDs() const { return m_outOfRangeGUIDs; }

    protected:
        uint32 m_blockCount;  //one per object updated
        GuidSet m_outOfRangeGUIDs;
        ByteBuffer m_data;

        void Compress(void* dst, uint32 *dst_size, void* src, int src_size);
};
#endif

