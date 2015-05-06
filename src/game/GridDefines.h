/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef TRINITY_GRIDDEFINES_H
#define TRINITY_GRIDDEFINES_H

#include "Common.h"
#include "GameSystem/NGrid.h"
#include <cmath>

// Forward class definitions
class Corpse;
class Creature;
class DynamicObject;
class GameObject;
class Pet;
class Player;

#define MAX_NUMBER_OF_GRIDS      64

#define SIZE_OF_GRIDS            533.33333f
#define CENTER_GRID_ID           (MAX_NUMBER_OF_GRIDS/2)

#define CENTER_GRID_OFFSET      (SIZE_OF_GRIDS/2)

#define MIN_GRID_DELAY          (MINUTE*IN_MILLISECONDS)
#define MIN_MAP_UPDATE_DELAY    50

#define MAX_NUMBER_OF_CELLS     8
#define SIZE_OF_GRID_CELL       (SIZE_OF_GRIDS/MAX_NUMBER_OF_CELLS)

#define CENTER_GRID_CELL_ID     (MAX_NUMBER_OF_CELLS*MAX_NUMBER_OF_GRIDS/2)
#define CENTER_GRID_CELL_OFFSET (SIZE_OF_GRID_CELL/2)

#define TOTAL_NUMBER_OF_CELLS_PER_MAP    (MAX_NUMBER_OF_GRIDS*MAX_NUMBER_OF_CELLS)

#define MAP_RESOLUTION 128

#define MAP_SIZE                (SIZE_OF_GRIDS*MAX_NUMBER_OF_GRIDS)
#define MAP_HALFSIZE            (MAP_SIZE/2)

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

#define MAX_HEIGHT            100000.0f                     // can be use for find ground height at surface
#define INVALID_HEIGHT       -100000.0f                     // for check, must be equal to VMAP_INVALID_HEIGHT, real value for unknown height is VMAP_INVALID_HEIGHT_VALUE
#define MAX_FALL_DISTANCE     250000.0f                     // "unlimited fall" to find VMap ground if it is available, just larger than MAX_HEIGHT - INVALID_HEIGHT
#define MIN_UNLOAD_DELAY      1                             // immediate unload
#define DEFAULT_HEIGHT_SEARCH 50.0f                         // default search distance to find height at nearby locations

/// Represents a map magic value of 4 bytes (used in versions)
union u_map_magic
{
    char asChar[4]; ///> Non-null terminated string
    uint32 asUInt;  ///> uint32 representation
};

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

// Creature used instead pet to simplify *::Visit templates (not required duplicate code for Creature->Pet case)
typedef TYPELIST_4(Player, Creature/*pets*/, Corpse/*resurrectable*/, DynamicObject/*farsight target*/) AllWorldObjectTypes;
typedef TYPELIST_4(GameObject, Creature/*except pets*/, DynamicObject, Corpse/*Bones*/) AllGridObjectTypes;

typedef GridRefManager<Corpse>          CorpseMapType;
typedef GridRefManager<Creature>        CreatureMapType;
typedef GridRefManager<DynamicObject>   DynamicObjectMapType;
typedef GridRefManager<GameObject>      GameObjectMapType;
typedef GridRefManager<Player>          PlayerMapType;

typedef Grid<Player, AllWorldObjectTypes,AllGridObjectTypes> GridType;
typedef NGrid<MAX_NUMBER_OF_CELLS, Player, AllWorldObjectTypes, AllGridObjectTypes> NGridType;

typedef TypeMapContainer<AllGridObjectTypes> GridTypeMapContainer;
typedef TypeMapContainer<AllWorldObjectTypes> WorldTypeMapContainer;

template<const unsigned int LIMIT>
struct CoordPair
{
    CoordPair(uint32 x=0, uint32 y=0) : x_coord(x), y_coord(y) {}
    CoordPair(const CoordPair<LIMIT> &obj) : x_coord(obj.x_coord), y_coord(obj.y_coord) {}
    bool operator==(const CoordPair<LIMIT> &obj) const { return (obj.x_coord == x_coord && obj.y_coord == y_coord); }
    bool operator!=(const CoordPair<LIMIT> &obj) const { return !operator==(obj); }
    CoordPair<LIMIT>& operator=(const CoordPair<LIMIT> &obj)
    {
        x_coord = obj.x_coord;
        y_coord = obj.y_coord;
        return *this;
    }

    void operator<<(const uint32 val)
    {
        if( x_coord > val )
            x_coord -= val;
        else
            x_coord = 0;
    }

    void operator>>(const uint32 val)
    {
        if( x_coord+val < LIMIT )
            x_coord += val;
        else
            x_coord = LIMIT - 1;
    }

    void operator-=(const uint32 val)
    {
        if( y_coord > val )
            y_coord -= val;
        else
            y_coord = 0;
    }

    void operator+=(const uint32 val)
    {
        if( y_coord+val < LIMIT )
            y_coord += val;
        else
            y_coord = LIMIT - 1;
    }

    uint32 x_coord;
    uint32 y_coord;
};

typedef CoordPair<MAX_NUMBER_OF_GRIDS> GridPair;
typedef CoordPair<TOTAL_NUMBER_OF_CELLS_PER_MAP> CellPair;

namespace Trinity
{
    template<class RET_TYPE, int CENTER_VAL>
        inline RET_TYPE Compute(float x, float y, float center_offset, float size)
    {
        // calculate and store temporary values in double format for having same result as same mySQL calculations
        double x_offset = (double(x) - center_offset)/size;
        double y_offset = (double(y) - center_offset)/size;

        int x_val = int(x_offset+CENTER_VAL + 0.5);
        int y_val = int(y_offset+CENTER_VAL + 0.5);
        return RET_TYPE(x_val, y_val);
    }

    inline GridPair ComputeGridPair(float x, float y)
    {
        return Compute<GridPair, CENTER_GRID_ID>(x, y, CENTER_GRID_OFFSET, SIZE_OF_GRIDS);
    }

    inline CellPair ComputeCellPair(float x, float y)
    {
        return Compute<CellPair, CENTER_GRID_CELL_ID>(x, y, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL);
    }

    inline CellPair ComputeCellPair(float x, float y, float &x_off, float &y_off)
    {
        double x_offset = (double(x) - CENTER_GRID_CELL_OFFSET)/SIZE_OF_GRID_CELL;
        double y_offset = (double(y) - CENTER_GRID_CELL_OFFSET)/SIZE_OF_GRID_CELL;

        int x_val = int(x_offset + CENTER_GRID_CELL_ID + 0.5);
        int y_val = int(y_offset + CENTER_GRID_CELL_ID + 0.5);
        x_off = (float(x_offset) - x_val + CENTER_GRID_CELL_ID) * SIZE_OF_GRID_CELL;
        y_off = (float(y_offset) - y_val + CENTER_GRID_CELL_ID) * SIZE_OF_GRID_CELL;
        return CellPair(x_val, y_val);
    }

    inline void NormalizeMapCoord(float &c)
    {
        if(c > MAP_HALFSIZE - 0.5)
            c = MAP_HALFSIZE - 0.5;
        else if(c < -(MAP_HALFSIZE - 0.5))
            c = -(MAP_HALFSIZE - 0.5);
    }
    
    // modulos a radian orientation to the range of 0..2PI
    inline float NormalizeOrientation(float o)
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

    inline bool IsValidMapCoord(float c)
    {
        return std::isfinite(c) && (std::fabs(c) <= MAP_HALFSIZE - 0.5);
    }

    inline bool IsValidMapCoord(float x, float y)
    {
        return IsValidMapCoord(x) && IsValidMapCoord(y);
    }

    inline bool IsValidMapCoord(float x, float y, float z)
    {
        return IsValidMapCoord(x,y) && std::isfinite(z);
    }

    inline bool IsValidMapCoord(float x, float y, float z, float o)
    {
        return IsValidMapCoord(x,y,z) && std::isfinite(o);
    }
}
#endif

