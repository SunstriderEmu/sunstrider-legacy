
#ifndef SUNSTRIDER_WATER_DEFINES_H
#define SUNSTRIDER_WATER_DEFINES_H

/*
Some data around Liquid types

On BC Map water and WMO waters use the same types. Values are indexes off LiquidType DBC. 99,9% of places use values between 1 and 4, but there are some special waters with spells, listed in LiquidType

Example values on BC :
1 (water) : Shattrath
2 (ocean) : any ?
3 (lava) : Iron Forge and Lava Dunegon
4 (slime) : Undercity and "World\\wmo\\Outland\\Shadowmoon\\Slagpit\\Shadowmoon_Slagpit01.wmo"

There are also some infos here: http://www.pxr.dk/wowdev/wiki/index.php?title=WMO/v17#MLIQ_chunk
Most of it isn't for BC though

MAP_LIQUID_VALUES are not client files values but values stored by the extractors
*/

enum LiquidType
{
    LIQUID_TYPE_NO_WATER = 0,
    LIQUID_TYPE_WATER = 1,
    LIQUID_TYPE_OCEAN = 2,
    LIQUID_TYPE_MAGMA = 3,
    LIQUID_TYPE_SLIME = 4,

    //those next ones are only present in area table liquid overrides and are associated with spells
    LIQUID_TYPE_NAXXRAMAS_SLIME  = 21,
    LIQUID_TYPE_COILFANG_WATER   = 41,
    LIQUID_TYPE_HYJAL_PAST_WATER = 61,
};

#define MAP_LIQUID_STATUS_SWIMMING (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)
#define MAP_LIQUID_STATUS_IN_CONTACT (MAP_LIQUID_STATUS_SWIMMING | LIQUID_MAP_WATER_WALK)

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_ALL_LIQUIDS   (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME)

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#ifdef LICH_KING
#define MAP_LIQUID_TYPE_WMO_WATER   0x20
#endif

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

enum ZLiquidStatus
{
    LIQUID_MAP_NO_WATER     = 0,
    LIQUID_MAP_ABOVE_WATER  = 1,
    LIQUID_MAP_WATER_WALK   = 2,
    LIQUID_MAP_IN_WATER     = 3,
    LIQUID_MAP_UNDER_WATER  = 4,
};

struct LiquidData
{
    uint32 type_flags;
    uint32 entry;
    float  level;
    float  depth_level;
};

#endif // SUNSTRIDER_WATER_DEFINES_H