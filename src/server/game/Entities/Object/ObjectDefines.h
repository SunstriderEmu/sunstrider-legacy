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

enum PhaseMasks : uint32
{
	PHASEMASK_NORMAL = 0x00000001,
	PHASEMASK_ANYWHERE = 0xFFFFFFFF
};

enum NotifyFlags
{
    NOTIFY_NONE                     = 0x00,
    NOTIFY_AI_RELOCATION            = 0x01,
    NOTIFY_VISIBILITY_CHANGED       = 0x02,
    NOTIFY_ALL                      = 0xFF
};

enum GOSummonType
{
    GO_SUMMON_TIMED_OR_CORPSE_DESPAWN      = 0,    // despawns after a specified time OR when the summoner dies
    GO_SUMMON_TIMED_DESPAWN                = 1     // despawns after a specified time
};

enum TempSummonType
{
    TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1,             // despawns after a specified time OR when the creature corpse disappears
    TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2,             // despawns after a specified time OR at creature death
    TEMPSUMMON_TIMED_DESPAWN               = 3,             // despawns after a specified time
    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4,             // despawns after a specified time after the creature is out of combat
    TEMPSUMMON_CORPSE_DESPAWN              = 5,             // despawns instantly after death
    TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6,             // despawns after a specified time after death
    TEMPSUMMON_DEAD_DESPAWN                = 7,             // despawns when the creature corpse disappears
    TEMPSUMMON_MANUAL_DESPAWN              = 8              // despawns when UnSummon() is called
};

#define CONTACT_DISTANCE            0.5f
#define INTERACTION_DISTANCE        5.0f
#define ATTACK_DISTANCE             5.0f
#define MAX_SEARCHER_DISTANCE       150.0f // sunwell: replace the use of MAX_VISIBILITY_DISTANCE in searchers, because MAX_VISIBILITY_DISTANCE is quite too big for this purpos
#define MAX_VISIBILITY_DISTANCE     250.0f      // max distance for visible object show, limited in 333 yards
#define VISIBILITY_INC_FOR_GOBJECTS 30.0f // sunwell
#define VISIBILITY_COMPENSATION     15.0f // increase searchers
#define SPELL_SEARCHER_COMPENSATION 30.0f // increase searchers size in case we have large npc near cell border
#define VISIBILITY_DIST_WINTERGRASP 175.0f // LK
#define SIGHT_RANGE_UNIT            50.0f

#define DEFAULT_VISIBILITY_DISTANCE 90.0f       // default visible distance, 90 yards on continents
#define DEFAULT_VISIBILITY_INSTANCE 120.0f      // default visible distance in instances, 120 yards
#define DEFAULT_VISIBILITY_BGARENAS 180.0f      // default visible distance in BG/Arenas, 180 yards

#define DEFAULT_PLAYER_BOUNDING_RADIUS   0.388999998569489f      // player size, also currently used (correctly?) for any non Unit world objects
#define MAX_STEALTH_DETECT_RANGE    45.0f
#define DEFAULT_PLAYER_COMBAT_REACH 1.5f
#define MIN_MELEE_REACH             2.0f
#define NOMINAL_MELEE_RANGE         5.0f
#define MELEE_RANGE                 (NOMINAL_MELEE_RANGE - MIN_MELEE_REACH * 2) //center to center for players

#endif

