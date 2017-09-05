#ifndef TRINITY_OBJECTDEFINES_H
#define TRINITY_OBJECTDEFINES_H

#include "Define.h"
#include "ObjectGuid.h"

#ifndef UINT64_C
    #define UINT64_C(x)  (x ## ULL)
#endif


// used for creating values for respawn for example
inline uint64 MAKE_PAIR64(uint32 l, uint32 h);
inline uint32 PAIR64_HIPART(uint64 x);
inline uint32 PAIR64_LOPART(uint64 x);
inline uint16 MAKE_PAIR16(uint8 l, uint8 h);
inline uint32 MAKE_PAIR32(uint16 l, uint16 h);
inline uint16 PAIR32_HIPART(uint32 x);
inline uint16 PAIR32_LOPART(uint32 x);

uint64 MAKE_PAIR64(uint32 l, uint32 h)
{
    return uint64(l | (uint64(h) << 32));
}

uint32 PAIR64_HIPART(uint64 x)
{
    return (uint32)((x >> 32) & UI64LIT(0x00000000FFFFFFFF));
}

uint32 PAIR64_LOPART(uint64 x)
{
    return (uint32)(x & UI64LIT(0x00000000FFFFFFFF));
}

uint16 MAKE_PAIR16(uint8 l, uint8 h)
{
    return uint16(l | (uint16(h) << 8));
}

uint32 MAKE_PAIR32(uint16 l, uint16 h)
{
    return uint32(l | (uint32(h) << 16));
}

uint16 PAIR32_HIPART(uint32 x)
{
    return (uint16)((x >> 16) & 0x0000FFFF);
}

uint16 PAIR32_LOPART(uint32 x)
{
    return (uint16)(x & 0x0000FFFF);
}

#define IS_EMPTY_GUID(Guid)          ( Guid == 0 )

#define IS_CREATURE_GUID(Guid)       ( GUID_HIPART(Guid) == uint32(HighGuid::Unit) )
#define IS_VEHICLE_GUID(Guid)        ( GUID_HIPART(Guid) == uint32(HighGuid::Vehicle) )
#define IS_CREATURE_OR_VEHICLE_GUID(Guid) ( IS_CREATURE_GUID(Guid) || IS_VEHICLE_GUID(Guid) )
#define IS_PET_GUID(Guid)            ( GUID_HIPART(Guid) == uint32(HighGuid::Pet) )
#define IS_CREATURE_OR_PET_GUID(Guid)( IS_CREATURE_GUID(Guid) || IS_PET_GUID(Guid) )
#define IS_PLAYER_GUID(Guid)         ( GUID_HIPART(Guid) == uint32(HighGuid::Player) && Guid != 0 )  // special case for empty guid need check (since HighGuid::Player == 0)
#define IS_UNIT_GUID(Guid)           ( IS_CREATURE_OR_PET_GUID(Guid) || IS_PLAYER_GUID(Guid) )
#define IS_ITEM_GUID(Guid)           ( GUID_HIPART(Guid) == uint32(HighGuid::Item) )
#define IS_GAMEOBJECT_GUID(Guid)     ( GUID_HIPART(Guid) == uint32(HighGuid::GameObject) )
#define IS_DYNAMICOBJECT_GUID(Guid)  ( GUID_HIPART(Guid) == uint32(HighGuid::DynamicObject) )
#define IS_CORPSE_GUID(Guid)         ( GUID_HIPART(Guid) == uint32(HighGuid::Corpse) )
#define IS_TRANSPORT(Guid)           ( GUID_HIPART(Guid) == uint32(HighGuid::Transport) )
#define IS_MO_TRANSPORT(Guid)        ( GUID_HIPART(Guid) == uint32(HighGuid::Mo_Transport) )

// l - OBJECT_FIELD_GUID
// e - OBJECT_FIELD_ENTRY for GO (except GAMEOBJECT_TYPE_MO_TRANSPORT) and creatures or UNIT_FIELD_PETNUMBER for pets
// h - OBJECT_FIELD_GUID + 1
#define MAKE_NEW_GUID(l, e, h)   uint64( uint64(l) | ( uint64(e) << 24 ) | ( uint64(h) << 48 ) )

#define GUID_HIPART(x)   (uint32)((uint64(x) >> 48) & 0x0000FFFF)

// We have different low and middle part size for different guid types
#define _GUID_ENPART_2(x) 0
#define _GUID_ENPART_3(x) (uint32)((uint64(x) >> 24) & 0x0000000000FFFFFFLL)
#define _GUID_LOPART_2(x) (uint32)(uint64(x)         & 0x00000000FFFFFFFFLL)
#define _GUID_LOPART_3(x) (uint32)(uint64(x)         & 0x0000000000FFFFFFLL)

inline bool IsGuidHaveEnPart(uint64 const& guid)
{
    switch(HighGuid(GUID_HIPART(guid)))
    {
        case HighGuid::Item:
        case HighGuid::Player:
        case HighGuid::DynamicObject:
        case HighGuid::Corpse:
            return false;
        case HighGuid::GameObject:
        case HighGuid::Transport:
        case HighGuid::Unit:
        case HighGuid::Pet:
        case HighGuid::Mo_Transport:
        default:
            return true;
    }
}

#define GUID_ENPART(x) (IsGuidHaveEnPart(x) ? _GUID_ENPART_3(x) : _GUID_ENPART_2(x))
#define GUID_LOPART(x) (IsGuidHaveEnPart(x) ? _GUID_LOPART_3(x) : _GUID_LOPART_2(x))

inline char const* GetLogNameForGuid(uint64 guid)
{
    switch(HighGuid(GUID_HIPART(guid)))
    {
        case HighGuid::Item:         return "item";
        case HighGuid::Player:       return guid ? "player" : "none";
        case HighGuid::GameObject:   return "gameobject";
        case HighGuid::Transport:    return "transport";
        case HighGuid::Unit:         return "creature";
        case HighGuid::Pet:          return "pet";
        case HighGuid::DynamicObject:return "dynobject";
        case HighGuid::Corpse:       return "corpse";
        case HighGuid::Mo_Transport: return "mo_transport";
        default:
            return "<unknown>";
    }
}
#endif

