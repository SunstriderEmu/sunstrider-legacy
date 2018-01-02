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

inline char const* GetLogNameForGuid(ObjectGuid guid)
{
    switch (guid.GetHigh())
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

