
#ifndef TRINITY_GRIDDEFINES_H
#define TRINITY_GRIDDEFINES_H

#include "Common.h"
#include "WaterDefines.h"
#include "NGrid.h"
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
#define MAP_INVALID_ZONE      0xFFFFFFFF
#define DEFAULT_HEIGHT_SEARCH 50.0f                         // default search distance to find height at nearby locations

/// Represents a map magic value of 4 bytes (used in versions)
union u_map_magic
{
    char asChar[4]; ///> Non-null terminated string
    uint32 asUInt;  ///> uint32 representation
};

// Creature used instead pet to simplify *::Visit templates (not required duplicate code for Creature->Pet case)
typedef TYPELIST_4(Player, Creature/*pets*/, Corpse/*resurrectable*/, DynamicObject/*farsight target*/) AllWorldObjectTypes;
typedef TYPELIST_4(GameObject, Creature/*except pets*/, DynamicObject, Corpse/*Bones*/) AllGridObjectTypes;
typedef TYPELIST_5(Creature, GameObject, DynamicObject, Pet, Corpse) AllMapStoredObjectTypes;

typedef GridRefManager<Corpse>          CorpseMapType;
typedef GridRefManager<Creature>        CreatureMapType;
typedef GridRefManager<DynamicObject>   DynamicObjectMapType;
typedef GridRefManager<GameObject>      GameObjectMapType;
typedef GridRefManager<Player>          PlayerMapType;

enum GridMapTypeMask
{
    GRID_MAP_TYPE_MASK_CORPSE           = 0x01,
    GRID_MAP_TYPE_MASK_CREATURE         = 0x02,
    GRID_MAP_TYPE_MASK_DYNAMICOBJECT    = 0x04,
    GRID_MAP_TYPE_MASK_GAMEOBJECT       = 0x08,
    GRID_MAP_TYPE_MASK_PLAYER           = 0x10,
    GRID_MAP_TYPE_MASK_ALL              = 0x1F
};

typedef Grid<Player, AllWorldObjectTypes,AllGridObjectTypes> GridType;
typedef NGrid<MAX_NUMBER_OF_CELLS, Player, AllWorldObjectTypes, AllGridObjectTypes> NGridType;

typedef TypeMapContainer<AllGridObjectTypes> GridTypeMapContainer;
typedef TypeMapContainer<AllWorldObjectTypes> WorldTypeMapContainer;
typedef TypeUnorderedMapContainer<AllMapStoredObjectTypes, uint64> MapStoredObjectTypesContainer;

template<const uint32 LIMIT>
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

	void dec_x(uint32 val)
	{
		if (x_coord > val)
			x_coord -= val;
		else
			x_coord = 0;
	}

	void inc_x(uint32 val)
	{
		if (x_coord + val < LIMIT)
			x_coord += val;
		else
			x_coord = LIMIT - 1;
	}

	void dec_y(uint32 val)
	{
		if (y_coord > val)
			y_coord -= val;
		else
			y_coord = 0;
	}

	void inc_y(uint32 val)
	{
		if (y_coord + val < LIMIT)
			y_coord += val;
		else
			y_coord = LIMIT - 1;
	}

    bool IsCoordValid() const
    {
        return x_coord < LIMIT && y_coord < LIMIT;
    }

	CoordPair& normalize()
	{
		x_coord = std::min(x_coord, LIMIT - 1);
		y_coord = std::min(y_coord, LIMIT - 1);
		return *this;
	}

	uint32 GetId() const
	{
		return y_coord * LIMIT + x_coord;
	}

    uint32 x_coord;
    uint32 y_coord;
};

typedef CoordPair<MAX_NUMBER_OF_GRIDS> GridCoord;
typedef CoordPair<TOTAL_NUMBER_OF_CELLS_PER_MAP> CellCoord;

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

    inline GridCoord ComputeGridCoord(float x, float y)
    {
        return Compute<GridCoord, CENTER_GRID_ID>(x, y, CENTER_GRID_OFFSET, SIZE_OF_GRIDS);
    }

    inline CellCoord ComputeCellCoord(float x, float y)
    {
        return Compute<CellCoord, CENTER_GRID_CELL_ID>(x, y, CENTER_GRID_CELL_OFFSET, SIZE_OF_GRID_CELL);
    }

    inline CellCoord ComputeCellCoord(float x, float y, float &x_off, float &y_off)
    {
        double x_offset = (double(x) - CENTER_GRID_CELL_OFFSET)/SIZE_OF_GRID_CELL;
        double y_offset = (double(y) - CENTER_GRID_CELL_OFFSET)/SIZE_OF_GRID_CELL;

        int x_val = int(x_offset + CENTER_GRID_CELL_ID + 0.5);
        int y_val = int(y_offset + CENTER_GRID_CELL_ID + 0.5);
        x_off = (float(x_offset) - x_val + CENTER_GRID_CELL_ID) * SIZE_OF_GRID_CELL;
        y_off = (float(y_offset) - y_val + CENTER_GRID_CELL_ID) * SIZE_OF_GRID_CELL;
        return CellCoord(x_val, y_val);
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
        return IsValidMapCoord(x,y) && IsValidMapCoord(z);
    }

    inline bool IsValidMapCoord(float x, float y, float z, float o)
    {
        return IsValidMapCoord(x,y,z) && std::isfinite(o);
    }
}
#endif

