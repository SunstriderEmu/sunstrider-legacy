
#ifndef TRINITYCORE_WATER_DEFINES_H
#define TRINITYCORE_WATER_DEFINES_H

/*
Some data around Liquid types

LiquidType has the following entries which are currently never used:
    NAXXRAMAS_SLIME = 21,
    COILFANG_RAID_WATER = 41,
    HYJAL_PAST_WATER = 61, 

The WMO liquid type field has :
Stormwind water = 5
Orgrimmar water = 5
slaves pen water = 5
azshara building water = 5
ironforge water = 5
ironforge lava = 7
blackrock lava (outside instance) = 7
molten core = 7
naxxramas slime = 8
undercity slime = 4 (this one is not supposed to make damage so I guess it's supposed to be different)

The maps have the following values :
booty bay water = 2
Dark water = 18
Hyjal water under the tree = 1
Rivers/lakes in strangletorn = 1

So we have the following problems with current implementations :
- The special LiquidType entries are never used. Water from Hyjal or Naxx don't seems to have any special values so I don't know how we're supposed to used them.
- The dark water with 18 seems to indicate another problem, there is a hack around this atm

There are also some research here that may give some ideas but it doesn't seem to apply to BC : http://www.pxr.dk/wowdev/wiki/index.php?title=WMO/v17#MLIQ_chunk
Or maybe it's just plain wrong.
Also there are flags in the MCLQ we currently don't use, I've not searched much on this side yet (I can just tell you that in the hyjal water it's still = 1, so I've no great hope).
*/

enum BaseLiquidType
{
    BASE_LIQUID_TYPE_NO_WATER = 0,
    BASE_LIQUID_TYPE_WATER = 1,
    BASE_LIQUID_TYPE_OCEAN = 2,
    BASE_LIQUID_TYPE_MAGMA = 3,
    BASE_LIQUID_TYPE_SLIME = 4,
    BASE_LIQUID_TYPE_DARK_WATER = 5,
};

enum BaseLiquidTypeMask
{
    BASE_LIQUID_TYPE_MASK_NONE  = 0x0,
    BASE_LIQUID_TYPE_MASK_WATER = 0x1,
    BASE_LIQUID_TYPE_MASK_OCEAN = 0x2,
    BASE_LIQUID_TYPE_MASK_MAGMA = 0x4,
    BASE_LIQUID_TYPE_MASK_SLIME = 0x8,
    BASE_LIQUID_TYPE_MASK_DARK_WATER = 0x10,

    BASE_LIQUID_TYPE_MASK_ALL = (BASE_LIQUID_TYPE_MASK_NONE | BASE_LIQUID_TYPE_MASK_WATER | BASE_LIQUID_TYPE_MASK_OCEAN | BASE_LIQUID_TYPE_MASK_MAGMA | BASE_LIQUID_TYPE_MASK_SLIME | BASE_LIQUID_TYPE_MASK_DARK_WATER),
};

enum MapLiquidType
{
    MAP_LIQUID_TYPE_NO_WATER    = 0,
    MAP_LIQUID_TYPE_WATER       = 1,
    MAP_LIQUID_TYPE_OCEAN       = 2,
    MAP_LIQUID_TYPE_MAGMA       = 3,
    MAP_LIQUID_TYPE_SLIME       = 4,
    MAP_LIQUID_TYPE_DARK_WATER  = 18, //this is STRANGE
};

enum WmoLiquidType
{
    WMO_LIQUID_TYPE_NO_WATER = 4,
    WMO_LIQUID_TYPE_WATER = 5,
    WMO_LIQUID_TYPE_OCEAN = 6, //not sure, this is a guess
    WMO_LIQUID_TYPE_LAVA = 7,
    WMO_LIQUID_TYPE_SLIME = 8,
};

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
    BaseLiquidType baseLiquidType;
    float  level;
    float  depth_level;
};

#endif