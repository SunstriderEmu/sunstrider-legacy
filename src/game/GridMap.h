#ifndef TRINITY_GRIDMAP_H
#define TRINITY_GRIDMAP_H

#include "Define.h"
#include "GridDefines.h"


// ******************************************
// Map file format defines
// ******************************************
struct map_fileheader
{
    u_map_magic mapMagic;
    u_map_magic versionMagic;
    u_map_magic buildMagic;
    uint32 areaMapOffset;
    uint32 areaMapSize;
    uint32 heightMapOffset;
    uint32 heightMapSize;
    uint32 liquidMapOffset;
    uint32 liquidMapSize;
    uint32 holesOffset;
    uint32 holesSize;
};

#define MAP_AREA_NO_AREA      0x0001

struct map_areaHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 gridArea;
};


#define MAP_HEIGHT_NO_HEIGHT            0x0001
#define MAP_HEIGHT_AS_INT16             0x0002
#define MAP_HEIGHT_AS_INT8              0x0004
#define MAP_HEIGHT_HAS_FLIGHT_BOUNDS    0x0008

struct map_heightHeader
{
    uint32 fourcc;
    uint32 flags;
    float  gridHeight;
    float  gridMaxHeight;
};

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

struct map_liquidHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 liquidType;
    uint8  offsetX;
    uint8  offsetY;
    uint8  width;
    uint8  height;
    float  liquidLevel;
};

class GridMap
{
    uint32  _flags;
    union{
        float* m_V9;
        uint16* m_uint16_V9;
        uint8* m_uint8_V9;
    };
    union{
        float* m_V8;
        uint16* m_uint16_V8;
        uint8* m_uint8_V8;
    };
    int16* _maxHeight;
    int16* _minHeight;

    // Height level data
    float _gridHeight;
    float _gridIntHeightMultiplier;

    // Area data
    uint16* _areaMap;

    // Liquid data
    float _liquidLevel;
    uint16* _liquidEntry;
    uint8* _liquidFlags;
    float* _liquidMap;
    uint16 _gridArea;
    uint16 _liquidType;
    uint8 _liquidOffX;
    uint8 _liquidOffY;
    uint8 _liquidWidth;
    uint8 _liquidHeight;


    bool loadAreaData(FILE* in, uint32 offset, uint32 size);
    bool loadHeightData(FILE* in, uint32 offset, uint32 size);
    bool loadLiquidData(FILE* in, uint32 offset, uint32 size);

    // Get height functions and pointers. walkableOnly NYI
    typedef float (GridMap::*GetHeightPtr) (float x, float y, bool walkableOnly) const;
    GetHeightPtr _gridGetHeight;
    float getHeightFromFloat(float x, float y, bool walkableOnly = false) const;
    float getHeightFromUint16(float x, float y, bool walkableOnly = false) const;
    float getHeightFromUint8(float x, float y, bool walkableOnly = false) const;
    float getHeightFromFlat(float x, float y, bool walkableOnly = false) const;
    
public:
    GridMap();
    ~GridMap();
    bool loadData(char* filaname);
    void unloadData();

    uint16 getArea(float x, float y) const;
    inline float getHeight(float x, float y, bool walkableOnly = false) const {return (this->*_gridGetHeight)(x, y, walkableOnly);}
    float getMinHeight(float x, float y) const;
    float getLiquidLevel(float x, float y) const;
    uint8 getTerrainType(float x, float y) const;
    ZLiquidStatus getLiquidStatus(float x, float y, float z, BaseLiquidTypeMask ReqLiquidTypeMask, LiquidData* data = 0);
    
    static bool ExistMap(uint32 mapid, int x, int y);
    static bool ExistVMap(uint32 mapid, int x, int y);
};

#endif // TRINITY_GRIDMAP_H