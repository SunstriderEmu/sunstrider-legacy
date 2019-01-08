
#ifndef SUNSTRIDER_WATER_DEFINES_H
#define SUNSTRIDER_WATER_DEFINES_H

/*
Sun: Some data and observations around Liquid types:

On TBC the WMO files references either the first 4 LiquidTypes, or the next 4 one non existing, or sometimes both, this doesn't seem consistent.
Those next 4 are the fake types in the enum below.
On TLK, WMO files use (others) correctly existing liquid types.

Also note that Liquid Type entries reference a base liquid type which is different between TBC and TLK. The TBC types are enumerated in BCLiquidTypeType.
For TLK, they are the same as for map (AdtLiquidType)

There are also some infos on the file structure here: https://web.archive.org/web/20130114171623/http://www.pxr.dk/wowdev/wiki/index.php?title=WMO/v17
Although I didn't use any for this implementation.

MAP_LIQUID_* are not client files values but values stored by the extractors

Other note, dark water isn't actually a liquid type, but any liquid type 
*/

enum LiquidType : uint32
{
    LIQUID_TYPE_NO_WATER  = 0,
    LIQUID_TYPE_WATER     = 1,
    LIQUID_TYPE_OCEAN     = 2,
    LIQUID_TYPE_MAGMA     = 3,
    LIQUID_TYPE_SLIME     = 4,

#ifndef LICH_KING
    //those "fake types" are 4 referenced in WMOs but not in LiquidType.dbc
    LIQUID_TYPE_WMO_WATER = 5, //ex: Stormwind
    LIQUID_TYPE_WMO_OCEAN = 6, //Guessed: there is no wmo with this type on tbc
    LIQUID_TYPE_WMO_LAVA  = 7, //ex: Blackrock, Onyxia's lair
    LIQUID_TYPE_WMO_SLIME = 8, //ex: Stratholme, Undeadziggurat, Naxx
#endif

    //those next ones are existing in LiquidType.dbc but not referenced in any wmo/map. They are however present in area table liquid overrides and are associated with spells
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

enum ZLiquidStatus : uint32
{
    LIQUID_MAP_NO_WATER     = 0x0,
    LIQUID_MAP_ABOVE_WATER  = 0x1,
    LIQUID_MAP_WATER_WALK   = 0x2,
    LIQUID_MAP_IN_WATER     = 0x4,
    LIQUID_MAP_UNDER_WATER  = 0x8,
};

struct LiquidData
{
    uint32 type_flags;
    uint32 entry;
    float  level;
    float  depth_level;
};

#endif // SUNSTRIDER_WATER_DEFINES_H