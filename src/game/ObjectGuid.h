/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ObjectGuid_h__
#define ObjectGuid_h__

#include "Common.h"
#include "ByteBuffer.h"

#include <functional>
#include <unordered_set>

enum TypeID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7
};

#define NUM_CLIENT_OBJECT_TYPES             8

enum TypeMask
{
    TYPEMASK_OBJECT         = 0x0001,
    TYPEMASK_ITEM           = 0x0002,
    TYPEMASK_CONTAINER      = 0x0006,                       // TYPEMASK_ITEM | 0x0004
    TYPEMASK_UNIT           = 0x0008,                       // creature
    TYPEMASK_PLAYER         = 0x0010,
    TYPEMASK_GAMEOBJECT     = 0x0020,
    TYPEMASK_DYNAMICOBJECT  = 0x0040,
    TYPEMASK_CORPSE         = 0x0080,
    TYPEMASK_SEER           = TYPEMASK_PLAYER | TYPEMASK_UNIT | TYPEMASK_DYNAMICOBJECT
};

enum HighGuid
{
    HIGHGUID_ITEM           = 0x4000,                      // blizz 4000
    HIGHGUID_CONTAINER      = 0x4000,                      // blizz 4000
    HIGHGUID_PLAYER         = 0x0000,                      // blizz 0000
    HIGHGUID_GAMEOBJECT     = 0xF110,                      // blizz F110
    HIGHGUID_TRANSPORT      = 0xF120,                      // blizz F120 (for GAMEOBJECT_TYPE_TRANSPORT)
    HIGHGUID_UNIT           = 0xF130,                      // blizz F130
    HIGHGUID_PET            = 0xF140,                      // blizz F140
    HIGHGUID_VEHICLE        = 0xF150,                      // blizz F550
    HIGHGUID_DYNAMICOBJECT  = 0xF100,                      // blizz F100
    HIGHGUID_CORPSE         = 0xF101,                      // blizz F100
    HIGHGUID_MO_TRANSPORT   = 0x1FC0,                      // blizz 1FC0 (for GAMEOBJECT_TYPE_MO_TRANSPORT)
    HIGHGUID_INSTANCE       = 0x1F40,                      // blizz 1F40
    HIGHGUID_GROUP          = 0x1F50
};

class PackedGuid;

struct PackedGuidReader
{
    explicit PackedGuidReader(uint64& guid) : GuidPtr(&guid) { }
    uint64* GuidPtr;
};

// Some Shared defines
typedef std::set<uint64> GuidSet;
typedef std::list<uint64> GuidList;
typedef std::deque<uint64> GuidDeque;
typedef std::vector<uint64> GuidVector;
typedef std::unordered_set<uint64> GuidUnorderedSet;

// minimum buffer size for packed guid is 9 bytes
#define PACKED_GUID_MIN_BUFFER_SIZE 9

class PackedGuid
{
        friend ByteBuffer& operator<<(ByteBuffer& buf, PackedGuid const& guid);

    public:
        explicit PackedGuid() : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(0); }
        explicit PackedGuid(uint64 guid) : _packedGuid(PACKED_GUID_MIN_BUFFER_SIZE) { _packedGuid.appendPackGUID(guid); }

        void Set(uint64 guid) { _packedGuid.wpos(0); _packedGuid.appendPackGUID(guid); }

        size_t size() const { return _packedGuid.size(); }

    private:
        ByteBuffer _packedGuid;
};

template<HighGuid high>
class ObjectGuidGenerator
{
    public:
        explicit ObjectGuidGenerator(uint32 start = 1) : _nextGuid(start) { }

        void Set(uint32 val) { _nextGuid = val; }
        uint32 Generate();
        uint32 GetNextAfterMaxUsed() const { return _nextGuid; }

    private:
        uint32 _nextGuid;
};

ByteBuffer& operator<<(ByteBuffer& buf, PackedGuid const& guid);
ByteBuffer& operator>>(ByteBuffer& buf, PackedGuidReader const& guid);

#endif // ObjectGuid_h__
