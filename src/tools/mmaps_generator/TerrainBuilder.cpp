
#include "TerrainBuilder.h"

#include "PathCommon.h"
#include "MapBuilder.h"

#include "VMapManager2.h"
#include "MapTree.h"
#include "ModelInstance.h"
#include "MapDefines.h"
#include "WaterDefines.h"

// ******************************************
// Map file format defines
// ******************************************
struct map_fileheader
{
    uint32 mapMagic;
    uint32 versionMagic;
    uint32 buildMagic;
    uint32 areaMapOffset;
    uint32 areaMapSize;
    uint32 heightMapOffset;
    uint32 heightMapSize;
    uint32 liquidMapOffset;
    uint32 liquidMapSize;
    uint32 holesOffset;
    uint32 holesSize;
};

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

struct map_heightHeader
{
    uint32 fourcc;
    uint32 flags;
    float  gridHeight;
    float  gridMaxHeight;
};

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

namespace MMAP
{

    char const* MAP_VERSION_MAGIC = "v1.8";

    TerrainBuilder::TerrainBuilder(bool skipLiquid, bool quick) : m_skipLiquid (skipLiquid), m_quick(quick)
    { }

    TerrainBuilder::~TerrainBuilder() { }

    /**************************************************************************/
    void TerrainBuilder::getLoopVars(Spot portion, int &loopStart, int &loopEnd, int &loopInc)
    {
        switch (portion)
        {
            case ENTIRE:
                loopStart = 0;
                loopEnd = V8_SIZE_SQ;
                loopInc = 1;
                break;
            case TOP:
                loopStart = 0;
                loopEnd = V8_SIZE;
                loopInc = 1;
                break;
            case LEFT:
                loopStart = 0;
                loopEnd = V8_SIZE_SQ - V8_SIZE + 1;
                loopInc = V8_SIZE;
                break;
            case RIGHT:
                loopStart = V8_SIZE - 1;
                loopEnd = V8_SIZE_SQ;
                loopInc = V8_SIZE;
                break;
            case BOTTOM:
                loopStart = V8_SIZE_SQ - V8_SIZE;
                loopEnd = V8_SIZE_SQ;
                loopInc = 1;
                break;
        }
    }

    /**************************************************************************/
    void TerrainBuilder::loadMap(uint32 mapID, uint32 tileX, uint32 tileY, MeshData &meshData)
    {
        if (loadMap(mapID, tileX, tileY, meshData, ENTIRE))
        {
            loadMap(mapID, tileX+1, tileY, meshData, LEFT);
            loadMap(mapID, tileX-1, tileY, meshData, RIGHT);
            loadMap(mapID, tileX, tileY+1, meshData, TOP);
            loadMap(mapID, tileX, tileY-1, meshData, BOTTOM);
        }
    }

    /**************************************************************************/
    bool TerrainBuilder::loadMap(uint32 mapID, uint32 tileX, uint32 tileY, MeshData &meshData, Spot portion)
    {
        char mapFileName[255];
        sprintf(mapFileName, "maps/%03u%02u%02u.map", mapID, tileY, tileX);

        FILE* mapFile = fopen(mapFileName, "rb");
        if (!mapFile)
            return false;

        map_fileheader fheader;
        if (fread(&fheader, sizeof(map_fileheader), 1, mapFile) != 1 ||
            fheader.versionMagic != *((uint32 const*)(MAP_VERSION_MAGIC)))
        {
            fclose(mapFile);
            printf("%s is the wrong version, please extract new .map files\n", mapFileName);
            return false;
        }

        map_heightHeader hheader;
        fseek(mapFile, fheader.heightMapOffset, SEEK_SET);

        bool haveTerrain = false;
        bool haveLiquid = false;
        if (fread(&hheader, sizeof(map_heightHeader), 1, mapFile) == 1)
        {
            haveTerrain = !(hheader.flags & MAP_HEIGHT_NO_HEIGHT);
            haveLiquid = fheader.liquidMapOffset && !m_skipLiquid;
        }

        // no data in this map file
        if (!haveTerrain && !haveLiquid)
        {
            fclose(mapFile);
            return false;
        }

        // data used later
        uint16 holes[16][16];
        memset(holes, 0, sizeof(holes));
        uint8 liquid_type[16][16];
        memset(liquid_type, 0, sizeof(liquid_type));
        G3D::Array<int> ltriangles;
        G3D::Array<int> ttriangles;

        // terrain data
        if (haveTerrain)
        {
            float heightMultiplier;
            float V9[V9_SIZE_SQ], V8[V8_SIZE_SQ];
            int expected = V9_SIZE_SQ + V8_SIZE_SQ;

            if (hheader.flags & MAP_HEIGHT_AS_INT8)
            {
                uint8 v9[V9_SIZE_SQ];
                uint8 v8[V8_SIZE_SQ];
                int count = 0;
                count += fread(v9, sizeof(uint8), V9_SIZE_SQ, mapFile);
                count += fread(v8, sizeof(uint8), V8_SIZE_SQ, mapFile);
                if (count != expected)
                    printf("TerrainBuilder::loadMap: Failed to read some data expected %d, read %d\n", expected, count);

                heightMultiplier = (hheader.gridMaxHeight - hheader.gridHeight) / 255;

                for (int i = 0; i < V9_SIZE_SQ; ++i)
                    V9[i] = (float)v9[i]*heightMultiplier + hheader.gridHeight;

                for (int i = 0; i < V8_SIZE_SQ; ++i)
                    V8[i] = (float)v8[i]*heightMultiplier + hheader.gridHeight;
            }
            else if (hheader.flags & MAP_HEIGHT_AS_INT16)
            {
                uint16 v9[V9_SIZE_SQ];
                uint16 v8[V8_SIZE_SQ];
                int count = 0;
                count += fread(v9, sizeof(uint16), V9_SIZE_SQ, mapFile);
                count += fread(v8, sizeof(uint16), V8_SIZE_SQ, mapFile);
                if (count != expected)
                    printf("TerrainBuilder::loadMap: Failed to read some data expected %d, read %d\n", expected, count);

                heightMultiplier = (hheader.gridMaxHeight - hheader.gridHeight) / 65535;

                for (int i = 0; i < V9_SIZE_SQ; ++i)
                    V9[i] = (float)v9[i]*heightMultiplier + hheader.gridHeight;

                for (int i = 0; i < V8_SIZE_SQ; ++i)
                    V8[i] = (float)v8[i]*heightMultiplier + hheader.gridHeight;
            }
            else
            {
                int count = 0;
                count += fread(V9, sizeof(float), V9_SIZE_SQ, mapFile);
                count += fread(V8, sizeof(float), V8_SIZE_SQ, mapFile);
                if (count != expected)
                    printf("TerrainBuilder::loadMap: Failed to read some data expected %d, read %d\n", expected, count);
            }

            // hole data
            if (fheader.holesSize != 0)
            {
                ASSERT(fheader.holesSize == sizeof(holes)); //sunstrider: extra assert
                memset(holes, 0, fheader.holesSize);
                fseek(mapFile, fheader.holesOffset, SEEK_SET);
                if (fread(holes, fheader.holesSize, 1, mapFile) != 1)
                    printf("TerrainBuilder::loadMap: Failed to read some data expected 1, read 0\n");
            }

            if (portion == ENTIRE)
            {
                {
                    std::unique_lock<boost::shared_mutex> lock(map_V_mutex);
                    if (map_V9.find(mapID) == map_V9.end())
                        map_V9[mapID] = new float[V9_SIZE_SQ];

                    if (map_V8.find(mapID) == map_V8.end())
                        map_V8[mapID] = new float[V8_SIZE_SQ];
                }

                memcpy(map_V8[mapID], V8, V8_SIZE_SQ * sizeof(float));
                memcpy(map_V9[mapID], V9, V9_SIZE_SQ * sizeof(float));
            }

            int count = meshData.solidVerts.size() / 3;
            float xoffset = (float(tileX)-32)*GRID_SIZE;
            float yoffset = (float(tileY)-32)*GRID_SIZE;

            float coord[3];

            for (int i = 0; i < V9_SIZE_SQ; ++i)
            {
                getHeightCoord(i, GRID_V9, xoffset, yoffset, coord, V9);
                meshData.solidVerts.append(coord[0]);
                meshData.solidVerts.append(coord[2]);
                meshData.solidVerts.append(coord[1]);
            }

            for (int i = 0; i < V8_SIZE_SQ; ++i)
            {
                getHeightCoord(i, GRID_V8, xoffset, yoffset, coord, V8);
                meshData.solidVerts.append(coord[0]);
                meshData.solidVerts.append(coord[2]);
                meshData.solidVerts.append(coord[1]);
            }

            int indices[] = { 0, 0, 0 };
            int loopStart = 0, loopEnd = 0, loopInc = 0;
            getLoopVars(portion, loopStart, loopEnd, loopInc);
            for (int i = loopStart; i < loopEnd; i+=loopInc)
                for (int j = TOP; j <= BOTTOM; j+=1)
                {
                    getHeightTriangle(i, Spot(j), indices);
                    ttriangles.append(indices[2] + count);
                    ttriangles.append(indices[1] + count);
                    ttriangles.append(indices[0] + count);
                }
        }

        // liquid data
        if (haveLiquid)
        {
            map_liquidHeader lheader;
            fseek(mapFile, fheader.liquidMapOffset, SEEK_SET);
            if (fread(&lheader, sizeof(map_liquidHeader), 1, mapFile) != 1)
                printf("TerrainBuilder::loadMap: Failed to read some data expected 1, read 0\n");


            float* liquid_map = NULL;

            if (!(lheader.flags & MAP_LIQUID_NO_TYPE))
                if (fread(liquid_type, sizeof(liquid_type), 1, mapFile) != 1)
                    printf("TerrainBuilder::loadMap: Failed to read some data expected 1, read 0\n");

            if (!(lheader.flags & MAP_LIQUID_NO_HEIGHT))
            {
                uint32 toRead = lheader.width * lheader.height;
                liquid_map = new float [toRead];
                if (fread(liquid_map, sizeof(float), toRead, mapFile) != toRead)
                    printf("TerrainBuilder::loadMap: Failed to read some data expected 1, read 0\n");
            }

            if (liquid_map)
            {
                int count = meshData.liquidVerts.size() / 3;
                float xoffset = (float(tileX)-32)*GRID_SIZE;
                float yoffset = (float(tileY)-32)*GRID_SIZE;

                float coord[3];
                int row, col;

                // generate coordinates
                if (!(lheader.flags & MAP_LIQUID_NO_HEIGHT))
                {
                    int j = 0;
                    for (int i = 0; i < V9_SIZE_SQ; ++i)
                    {
                        row = i / V9_SIZE;
                        col = i % V9_SIZE;

                        if (row < lheader.offsetY || row >= lheader.offsetY + lheader.height ||
                            col < lheader.offsetX || col >= lheader.offsetX + lheader.width)
                        {
                            // dummy vert using invalid height
                            meshData.liquidVerts.append((xoffset+col*GRID_PART_SIZE)*-1, INVALID_MAP_LIQ_HEIGHT, (yoffset+row*GRID_PART_SIZE)*-1);
                            continue;
                        }

                        getLiquidCoord(i, j, xoffset, yoffset, coord, liquid_map);
                        meshData.liquidVerts.append(coord[0]);
                        meshData.liquidVerts.append(coord[2]);
                        meshData.liquidVerts.append(coord[1]);
                        j++;
                    }
                }
                else
                {
                    for (int i = 0; i < V9_SIZE_SQ; ++i)
                    {
                        row = i / V9_SIZE;
                        col = i % V9_SIZE;
                        meshData.liquidVerts.append((xoffset+col*GRID_PART_SIZE)*-1, lheader.liquidLevel, (yoffset+row*GRID_PART_SIZE)*-1);
                    }
                }

                delete [] liquid_map;

                int indices[] = { 0, 0, 0 };
                int loopStart = 0, loopEnd = 0, loopInc = 0, triInc = BOTTOM-TOP;
                getLoopVars(portion, loopStart, loopEnd, loopInc);

                // generate triangles
                for (int i = loopStart; i < loopEnd; i+=loopInc)
                    for (int j = TOP; j <= BOTTOM; j+= triInc)
                    {
                        getHeightTriangle(i, Spot(j), indices, true);
                        ltriangles.append(indices[2] + count);
                        ltriangles.append(indices[1] + count);
                        ltriangles.append(indices[0] + count);
                    }
            }
        }

        fclose(mapFile);

        // now that we have gathered the data, we can figure out which parts to keep:
        // liquid above ground, ground above liquid
        int loopStart = 0, loopEnd = 0, loopInc = 0, tTriCount = 4;
        bool useTerrain, useLiquid;

        float* lverts = meshData.liquidVerts.getCArray();
        int* ltris = ltriangles.getCArray();

        float* tverts = meshData.solidVerts.getCArray();
        int* ttris = ttriangles.getCArray();

        if ((ltriangles.size() + ttriangles.size()) == 0)
            return false;

        // make a copy of liquid vertices
        // used to pad right-bottom frame due to lost vertex data at extraction
        float* lverts_copy = NULL;
        if (meshData.liquidVerts.size())
        {
            lverts_copy = new float[meshData.liquidVerts.size()];
            memcpy(lverts_copy, lverts, sizeof(float)*meshData.liquidVerts.size());
        }

        getLoopVars(portion, loopStart, loopEnd, loopInc);
        for (int i = loopStart; i < loopEnd; i+=loopInc)
        {
            for (int j = 0; j < 2; ++j)
            {
                // default is true, will change to false if needed
                useTerrain = true;
                useLiquid = true;
                uint8 liquidType = MAP_LIQUID_TYPE_NO_WATER;
                // FIXME: "warning: the address of ‘liquid_type’ will always evaluate as ‘true’"

                // if there is no liquid, don't use liquid
                if (!meshData.liquidVerts.size() || !ltriangles.size())
                    useLiquid = false;
                else
                {
                    liquidType = getLiquidType(i, liquid_type);
                    switch (liquidType)
                    {
                        default:
                            useLiquid = false;
                            break;
                        case MAP_LIQUID_TYPE_WATER:
                        case MAP_LIQUID_TYPE_OCEAN:
                            // merge different types of water
                            liquidType = NAV_WATER;
                            break;
                        case MAP_LIQUID_TYPE_MAGMA:
                            liquidType = NAV_MAGMA;
                            break;
                        case MAP_LIQUID_TYPE_SLIME:
                            liquidType = NAV_SLIME;
                            break;
                        case MAP_LIQUID_TYPE_DARK_WATER:
                            // players should not be here, so logically neither should creatures
                            useTerrain = false;
                            useLiquid = false;
                            break;
                    }
                }

                // if there is no terrain, don't use terrain
                if (!ttriangles.size())
                    useTerrain = false;

                // while extracting ADT data we are losing right-bottom vertices
                // this code adds fair approximation of lost data
                if (useLiquid)
                {
                    float quadHeight = 0;
                    uint32 validCount = 0;
                    for(uint32 idx = 0; idx < 3; idx++)
                    {
                        float h = lverts_copy[ltris[idx]*3 + 1];
                        if (h != INVALID_MAP_LIQ_HEIGHT && h < INVALID_MAP_LIQ_HEIGHT_MAX)
                        {
                            quadHeight += h;
                            validCount++;
                        }
                    }

                    // update vertex height data
                    if (validCount > 0 && validCount < 3)
                    {
                        quadHeight /= validCount;
                        for(uint32 idx = 0; idx < 3; idx++)
                        {
                            float h = lverts[ltris[idx]*3 + 1];
                            if (h == INVALID_MAP_LIQ_HEIGHT || h > INVALID_MAP_LIQ_HEIGHT_MAX)
                                lverts[ltris[idx]*3 + 1] = quadHeight;
                        }
                    }

                    // no valid vertexes - don't use this poly at all
                    if (validCount == 0)
                        useLiquid = false;
                }

                // if there is a hole here, don't use the terrain
                if (useTerrain && fheader.holesSize != 0)
                    useTerrain = !isHole(i, holes);

                // we use only one terrain kind per quad - pick higher one
                if (useTerrain && useLiquid)
                {
                    float minLLevel = INVALID_MAP_LIQ_HEIGHT_MAX;
                    float maxLLevel = INVALID_MAP_LIQ_HEIGHT;
                    for(uint32 x = 0; x < 3; x++)
                    {
                        float h = lverts[ltris[x]*3 + 1];
                        if (minLLevel > h)
                            minLLevel = h;

                        if (maxLLevel < h)
                            maxLLevel = h;
                    }

                    float maxTLevel = INVALID_MAP_LIQ_HEIGHT;
                    float minTLevel = INVALID_MAP_LIQ_HEIGHT_MAX;
                    for(uint32 x = 0; x < 6; x++)
                    {
                        float h = tverts[ttris[x]*3 + 1];
                        if (maxTLevel < h)
                            maxTLevel = h;

                        if (minTLevel > h)
                            minTLevel = h;
                    }

                    // terrain under the liquid?
                    if (minLLevel > maxTLevel)
                        useTerrain = false;

                    //liquid under the terrain?
                    if (minTLevel > maxLLevel)
                        useLiquid = false;
                }

                // store the result
                if (useLiquid)
                {
                    meshData.liquidType.append(liquidType);
                    for (int k = 0; k < 3; ++k)
                        meshData.liquidTris.append(ltris[k]);
                }

                if (useTerrain)
                    for (int k = 0; k < 3*tTriCount/2; ++k)
                        meshData.solidTris.append(ttris[k]);

                // advance to next set of triangles
                ltris += 3;
                ttris += 3*tTriCount/2;
            }
        }

        if (lverts_copy)
            delete [] lverts_copy;

        return meshData.solidTris.size() || meshData.liquidTris.size();
    }

    /**************************************************************************/
    void TerrainBuilder::getHeightCoord(int index, Grid grid, float xOffset, float yOffset, float* coord, float* v)
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        switch (grid)
        {
        case GRID_V9:
            coord[0] = (xOffset + index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
            coord[1] = (yOffset + (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
            coord[2] = v[index];
            break;
        case GRID_V8:
            coord[0] = (xOffset + index%(V8_SIZE)*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
            coord[1] = (yOffset + (int)(index/(V8_SIZE))*GRID_PART_SIZE + GRID_PART_SIZE/2.f) * -1.f;
            coord[2] = v[index];
            break;
        }
    }

    /**************************************************************************/
    void TerrainBuilder::getHeightTriangle(int square, Spot triangle, int* indices, bool liquid/* = false*/)
    {
        int rowOffset = square/V8_SIZE;
        if (!liquid)
            switch (triangle)
        {
            case TOP:
                indices[0] = square+rowOffset;                  //           0-----1 .... 128
                indices[1] = square+1+rowOffset;                //           |\ T /|
                indices[2] = (V9_SIZE_SQ)+square;               //           | \ / |
                break;                                          //           |L 0 R| .. 127
            case LEFT:                                          //           | / \ |
                indices[0] = square+rowOffset;                  //           |/ B \|
                indices[1] = (V9_SIZE_SQ)+square;               //          129---130 ... 386
                indices[2] = square+V9_SIZE+rowOffset;          //           |\   /|
                break;                                          //           | \ / |
            case RIGHT:                                         //           | 128 | .. 255
                indices[0] = square+1+rowOffset;                //           | / \ |
                indices[1] = square+V9_SIZE+1+rowOffset;        //           |/   \|
                indices[2] = (V9_SIZE_SQ)+square;               //          258---259 ... 515
                break;
            case BOTTOM:
                indices[0] = (V9_SIZE_SQ)+square;
                indices[1] = square+V9_SIZE+1+rowOffset;
                indices[2] = square+V9_SIZE+rowOffset;
                break;
            default: break;
        }
        else
            switch (triangle)
        {                                                           //           0-----1 .... 128
            case TOP:                                               //           |\    |
                indices[0] = square+rowOffset;                      //           | \ T |
                indices[1] = square+1+rowOffset;                    //           |  \  |
                indices[2] = square+V9_SIZE+1+rowOffset;            //           | B \ |
                break;                                              //           |    \|
            case BOTTOM:                                            //          129---130 ... 386
                indices[0] = square+rowOffset;                      //           |\    |
                indices[1] = square+V9_SIZE+1+rowOffset;            //           | \   |
                indices[2] = square+V9_SIZE+rowOffset;              //           |  \  |
                break;                                              //           |   \ |
            default: break;                                         //           |    \|
        }                                                           //          258---259 ... 515

    }

    /**************************************************************************/
    void TerrainBuilder::getLiquidCoord(int index, int index2, float xOffset, float yOffset, float* coord, float* v)
    {
        // wow coords: x, y, height
        // coord is mirroed about the horizontal axes
        coord[0] = (xOffset + index%(V9_SIZE)*GRID_PART_SIZE) * -1.f;
        coord[1] = (yOffset + (int)(index/(V9_SIZE))*GRID_PART_SIZE) * -1.f;
        coord[2] = v[index2];
    }

    static uint16 holetab_h[4] = {0x1111, 0x2222, 0x4444, 0x8888};
    static uint16 holetab_v[4] = {0x000F, 0x00F0, 0x0F00, 0xF000};

    /**************************************************************************/
    bool TerrainBuilder::isHole(int square, const uint16 holes[16][16])
    {
        int row = square / 128;
        int col = square % 128;
        int cellRow = row / 8;     // 8 squares per cell
        int cellCol = col / 8;
        int holeRow = row % 8 / 2;
        int holeCol = (square - (row * 128 + cellCol * 8)) / 2;

        uint32 hole = holes[cellRow][cellCol];

        return (hole & holetab_h[holeCol] & holetab_v[holeRow]) != 0;
    }

    /**************************************************************************/
    uint8 TerrainBuilder::getLiquidType(int square, const uint8 liquid_type[16][16])
    {
        int row = square / 128;
        int col = square % 128;
        int cellRow = row / 8;     // 8 squares per cell
        int cellCol = col / 8;

        return liquid_type[cellRow][cellCol];
    }

    /**************************************************************************/
    bool TerrainBuilder::loadVMap(uint32 mapID, uint32 tileX, uint32 tileY, MeshData &meshData)
    {
        int result = vmapManager.loadMap("vmaps", mapID, tileX, tileY);
        bool retval = false;

        do
        {
            InstanceTreeMap instanceTrees;
            vmapManager.getInstanceMapTree(instanceTrees);

            ModelInstance* models = NULL;
            //model instances count
            uint32 count = 0;
            instanceTrees[mapID]->getModelInstances(models, count);

            if (result == VMAP_LOAD_RESULT_ERROR || !instanceTrees[mapID] || !models)
            {
                vmapManager.unloadMap(mapID, tileX, tileY);
                break;
            }

            //map geometry is already loaded at this point, noting down the last map vert and tri counts (anything after those will be models geometry)
            int mapVertsCount = meshData.solidVerts.size();
            int mapTrisCount = meshData.solidTris.size();
            meshData.vmapFirstTriangle = meshData.solidTris.size() / 3;

            for (uint32 i = 0; i < count; ++i)
            {
                ModelInstance instance = models[i];

                // model instances exist in tree even though there are instances of that model in this tile
                WorldModel* worldModel = instance.getWorldModel();
                if (!worldModel)
                    continue;

                // now we have a model to add to the meshdata
                retval = true;

                std::vector<GroupModel> groupModels;
                worldModel->getGroupModels(groupModels);

                // all M2s need to have triangle indices reversed
                bool isM2 = instance.name.find(".m2") != std::string::npos || instance.name.find(".M2") != std::string::npos;

                // transform data
                float scale = instance.iScale;
                G3D::Matrix3 rotation = G3D::Matrix3::fromEulerAnglesXYZ(G3D::pi()*instance.iRot.z/-180.f, G3D::pi()*instance.iRot.x/-180.f, G3D::pi()*instance.iRot.y/-180.f);
                G3D::Vector3 position = instance.iPos;
                position.x -= 32*GRID_SIZE;
                position.y -= 32*GRID_SIZE;

                for (std::vector<GroupModel>::iterator it = groupModels.begin(); it != groupModels.end(); ++it)
                {
                    std::vector<G3D::Vector3> tempVertices;
                    std::vector<G3D::Vector3> transformedVertices;
                    std::vector<MeshTriangle> tempTriangles;
                    WmoLiquid* liquid = nullptr;

                    it->getMeshData(tempVertices, tempTriangles, liquid);

                    // first handle collision mesh
                    transform(tempVertices, transformedVertices, scale, rotation, position);

                    int offset = meshData.solidVerts.size() / 3;

                    copyVertices(transformedVertices, meshData.solidVerts); // (x, y, z) -> (y, z, x)
                    copyIndices(tempTriangles, meshData.solidTris, offset, isM2);

                    // now handle liquid data
                    if (liquid)
                    {
                        std::vector<G3D::Vector3> liqVerts;
                        std::vector<int> liqTris;
                        uint32 tilesX, tilesY, vertsX, vertsY;
                        G3D::Vector3 corner;
                        liquid->getPosInfo(tilesX, tilesY, corner);
                        vertsX = tilesX + 1;
                        vertsY = tilesY + 1;
                        uint8* flags = liquid->GetFlagsStorage();
                        float* data = liquid->GetHeightStorage();
                        uint8 type = NAV_EMPTY;

                        // convert liquid type to NavTerrain
#ifdef LICH_KING
                        switch (liquid->GetType() & 3) 
                        //this is needed on LK because wmo get liquid type such as : 13 14 17 19 20. On BC we got classic water types (1 to 4)
#else
                        switch (liquid->GetType())
#endif
                        {
                        case LIQUID_TYPE_WATER:
                        case LIQUID_TYPE_OCEAN:
                            type = NAV_WATER;
                            break;
                        case LIQUID_TYPE_MAGMA:
                            type = NAV_MAGMA;
                            break;
                        case LIQUID_TYPE_SLIME:
                            type = NAV_SLIME;
                            break;
                        }

                        // indexing is weird...
                        // after a lot of trial and error, this is what works:
                        // vertex = y*vertsX+x
                        // tile   = x*tilesY+y
                        // flag   = y*tilesY+x

                        G3D::Vector3 vert;
                        for (uint32 x = 0; x < vertsX; ++x)
                            for (uint32 y = 0; y < vertsY; ++y)
                            {
                                vert = G3D::Vector3(corner.x + x * GRID_PART_SIZE, corner.y + y * GRID_PART_SIZE, data[y*vertsX + x]);
                                vert = vert * rotation * scale + position;
                                vert.x *= -1.f;
                                vert.y *= -1.f;
                                liqVerts.push_back(vert);
                            }

                            int idx1, idx2, idx3, idx4;
                            uint32 square;
                            for (uint32 x = 0; x < tilesX; ++x)
                                for (uint32 y = 0; y < tilesY; ++y)
                                    if ((flags[x+y*tilesX] & 0x0f) != 0x0f)
                                    {
                                        square = x * tilesY + y;
                                        idx1 = square+x;
                                        idx2 = square+1+x;
                                        idx3 = square+tilesY+1+1+x;
                                        idx4 = square+tilesY+1+x;

                                        // top triangle
                                        liqTris.push_back(idx3);
                                        liqTris.push_back(idx2);
                                        liqTris.push_back(idx1);
                                        // bottom triangle
                                        liqTris.push_back(idx4);
                                        liqTris.push_back(idx3);
                                        liqTris.push_back(idx1);
                                    }

                                    uint32 liqOffset = meshData.liquidVerts.size() / 3;
                                    for (uint32 j = 0; j < liqVerts.size(); ++j)
                                        meshData.liquidVerts.append(liqVerts[j].y, liqVerts[j].z, liqVerts[j].x);

                                    for (uint32 j = 0; j < liqTris.size() / 3; ++j)
                                    {
                                        meshData.liquidTris.append(liqTris[j *3+1] + liqOffset, liqTris[j *3+2] + liqOffset, liqTris[j *3] + liqOffset);
                                        meshData.liquidType.append(type);
                                    }
                    }
                }
            }

            //from here, code adapted from Nost! This will remove terrain (adt) geometry inside m2/wmx and wmo objects
            meshData.vmapLastTriangle = meshData.solidTris.size() / 3;
            if (m_quick)
                break;

            /// remove from terrain all triangles inside any model.
            if (!mapVertsCount || !mapTrisCount)  //sun addition
                break;

            float* terrainInsideModelsVerts = new float[mapVertsCount]; //terrain vertices inside any model. Positives values mean inside model.
            for (int i = 0; i < mapVertsCount; ++i)
                terrainInsideModelsVerts[i] = -1.0f;

            //compute vertex under models (fill terrainInsideModelsVerts)
            for (uint32 i = 0; i < count; ++i) //for each models instances
            {
                ModelInstance instance = models[i];

                // model instances exist in tree even though there are instances of that model in this tile
                WorldModel* worldModel = instance.getWorldModel();
                if (!worldModel)
                    continue;

                std::vector<GroupModel> groupModels;
                worldModel->getGroupModels(groupModels);

                // all M2s need to have triangle indices reversed
                //sun: added mdx... probably wrong but this somehow fixes the logic :/ probably indirectly
                bool isM2 = instance.name.find(".mdx") != instance.name.npos || instance.name.find(".m2") != instance.name.npos || instance.name.find(".M2") != instance.name.npos;

                // transform data
                float scale = instance.iScale;
                G3D::Matrix3 rotation = G3D::Matrix3::fromEulerAnglesXYZ(G3D::pi() * instance.iRot.z / -180.f, G3D::pi() * instance.iRot.x / -180.f, G3D::pi() * instance.iRot.y / -180.f);
                G3D::Vector3 position = instance.iPos; //object position in world
                position.x -= 32 * GRID_SIZE;
                position.y -= 32 * GRID_SIZE;

                /// Check every map vertice
                /// @TODO, alread excludes the really far ones?
                // x, y * -1
                G3D::Vector3 up(0, 0, 1);
                up.x *= -1.0f;
                up.y *= -1.0f;
                up = up * rotation.inverse(); //sun: removed scale here, makes no sense for a direction
                for (std::vector<GroupModel>::iterator it = groupModels.begin(); it != groupModels.end(); ++it)
                {
                    for (int t = 0; t < mapVertsCount / 3; ++t)
                    {
                        // y, z, x
                        G3D::Vector3 v(meshData.solidVerts[3 * t + 2], meshData.solidVerts[3 * t], meshData.solidVerts[3 * t + 1] + 0.2f);
                        // v = map vertex position relative to world
                        v.x *= -1.f;
                        v.y *= -1.f;
                        v -= position;
                        v = v * rotation.inverse() / scale;
                        // v = map vertex transformed back to a position relative to object ?

                        //sun: if point is further (horizontally) than 600 to the center of the object, skip. (added this to speed up calc)
                        if ((v.x * v.x + v.y * v.y) > 600.0f * 600.0f)
                            continue;

                        float outDist = -1.0f;
                        float inDist = -1.0f;
                        if (it->IsUnderObject(v, up, isM2, &outDist, &inDist)) // inDist < outDist
                            terrainInsideModelsVerts[t] = inDist;
                    }
                }
            }
            /// Correct triangles partially under models
            //sun : next line cause crash. 
            //for (int i = 0; i < meshData.solidTris.size() / 3; ++i) //sun: check each triangles. map + vmap? why? It seems to me that the intent here is to check only terrain triangles and not the model triangle we just added?
            for (int i = 0; i < mapTrisCount / 3; ++i) //sun, changed to only check map triangles
            {
                G3D::Vector3 tri[3]; //triangle position  (? y, z, x ?)
                uint32 vertIdx[3];
                bool insideModel[3] = { false }; // triangle vertex inside model
                for (int j = 0; j < 3; ++j)
                {
                    vertIdx[j] = meshData.solidTris[i * 3 + j];
                    float insideDistance = terrainInsideModelsVerts[vertIdx[j]];
                    if (vertIdx[j] < mapVertsCount)
                        insideModel[j] = (insideDistance >= 0.1f);
                    //sun: get triangle position from vertex idx
                    tri[j] = G3D::Vector3(meshData.solidVerts[3 * vertIdx[j] + 2], meshData.solidVerts[3 * vertIdx[j]], meshData.solidVerts[3 * vertIdx[j] + 1]);
                }

                // First case: nothing to do =) (sun: skip if no vertex of triangle is sticking out enough out of the model)
                if (terrainInsideModelsVerts[vertIdx[0]] < 1.0f && terrainInsideModelsVerts[vertIdx[1]] < 1.0f && terrainInsideModelsVerts[vertIdx[2]] < 1.0f)
                    continue;
                if (insideModel[0] == insideModel[1] == insideModel[2]) // all vertexes are in, or all vertexes are out
                    continue;

                // There is one vertex outside
                uint32 outsideIdx; //first vertex outside
                for (outsideIdx = 0; outsideIdx < 3; ++outsideIdx)
                    if (!insideModel[outsideIdx])
                        break;

                // Intersection to vertexes
                // - vert 1 & 2 are inside.
                if (insideModel[(outsideIdx + 1) % 3] && insideModel[(outsideIdx + 2) % 3])
                {
                    G3D::Vector3 intersectTo1;
                    G3D::Vector3 intersectTo2;
                    bool inters1 = (vmapManager.getObjectHitPos(mapID, tri[outsideIdx].x, tri[outsideIdx].y, tri[outsideIdx].z,
                        tri[(outsideIdx + 1) % 3].x, tri[(outsideIdx + 1) % 3].y, tri[(outsideIdx + 1) % 3].z,
                        intersectTo1.x, intersectTo1.y, intersectTo1.z, 0.0f));
                    bool inters2 = (vmapManager.getObjectHitPos(mapID, tri[outsideIdx].x, tri[outsideIdx].y, tri[outsideIdx].z,
                        tri[(outsideIdx + 2) % 3].x, tri[(outsideIdx + 2) % 3].y, tri[(outsideIdx + 2) % 3].z,
                        intersectTo2.x, intersectTo2.y, intersectTo2.z, 0.0f));
                    if (!inters1)
                        intersectTo1 = (tri[(outsideIdx + 1) % 3] * 4 + tri[outsideIdx]) / 5;
                    if (!inters2)
                        intersectTo2 = (tri[(outsideIdx + 2) % 3] * 4 + tri[outsideIdx]) / 5;

                    // Insert new vertices
                    int offset = meshData.solidVerts.size() / 3;
                    meshData.solidVerts.append(intersectTo1.y);
                    meshData.solidVerts.append(intersectTo1.z);
                    meshData.solidVerts.append(intersectTo1.x);
                    meshData.solidVerts.append(intersectTo2.y);
                    meshData.solidVerts.append(intersectTo2.z);
                    meshData.solidVerts.append(intersectTo2.x);
                    vertIdx[(outsideIdx + 1) % 3] = offset;
                    vertIdx[(outsideIdx + 2) % 3] = offset + 1;
                }
                if (insideModel[(outsideIdx + 1) % 3] != insideModel[(outsideIdx + 2) % 3])
                {
                    // Only one vert is inside
                    for (uint32 insideIdx = 0; insideIdx < 3; ++insideIdx)
                        if (insideIdx != outsideIdx && insideModel[insideIdx])
                        {
                            uint32 outsideIdx2 = (outsideIdx + 1) % 3;
                            if (outsideIdx2 == insideIdx)
                                outsideIdx2 = (outsideIdx2 + 1) % 3;

                            // 2 vertexes outside: outsideIdx/outsideIdx2. One inside: insideIdx
                            G3D::Vector3 intersectFrom1;
                            G3D::Vector3 intersectFrom2;
                            bool inters1 = (vmapManager.getObjectHitPos(mapID, tri[outsideIdx].x, tri[outsideIdx].y, tri[outsideIdx].z,
                                tri[insideIdx].x, tri[insideIdx].y, tri[insideIdx].z,
                                intersectFrom1.x, intersectFrom1.y, intersectFrom1.z, -0.01f));
                            bool inters2 = (vmapManager.getObjectHitPos(mapID, tri[outsideIdx2].x, tri[outsideIdx2].y, tri[outsideIdx2].z,
                                tri[insideIdx].x, tri[insideIdx].y, tri[insideIdx].z,
                                intersectFrom2.x, intersectFrom2.y, intersectFrom2.z, -0.01f));
                            if (!inters1 && !inters2)
                            {
                                intersectFrom1 = (tri[outsideIdx] + tri[insideIdx] * 3) / 4;
                                intersectFrom2 = (tri[outsideIdx2] + tri[insideIdx] * 3) / 4;
                            }
                            int offsetVerts = meshData.solidVerts.size() / 3;
                            meshData.solidVerts.append(intersectFrom1.y);
                            meshData.solidVerts.append(intersectFrom1.z);
                            meshData.solidVerts.append(intersectFrom1.x);
                            meshData.solidVerts.append(intersectFrom2.y);
                            meshData.solidVerts.append(intersectFrom2.z);
                            meshData.solidVerts.append(intersectFrom2.x);
                            // Triangles verts are oriented surfaces ("inside" / "outside")
                            meshData.solidTris.append(vertIdx[outsideIdx2]);
                            meshData.solidTris.append(offsetVerts);
                            meshData.solidTris.append(offsetVerts + 1);
                            meshData.solidTris.append(offsetVerts + 1);
                            meshData.solidTris.append(offsetVerts);
                            meshData.solidTris.append(vertIdx[outsideIdx2]);
                            vertIdx[insideIdx] = offsetVerts;
                            break;
                        }
                }
                for (int j = 0; j < 3; ++j)
                    meshData.solidTris[i * 3 + j] = vertIdx[j];
            }
         
            /// sun: Clear map triangles completely under models
            for (int i = 0; i < mapTrisCount / 3; ++i)
            {
                bool insideModel[3] = { false }; // triangle vertex inside model
                bool allInBorder = true; //true = all points are close to the model
                for (int j = 0; j < 3; ++j)
                {
                    if (meshData.solidTris[i * 3 + j] < mapVertsCount)
                    {
                        float insideValue = terrainInsideModelsVerts[meshData.solidTris[i * 3 + j]];
                        insideModel[j] = (insideValue >= 1.0f && insideValue);
                        if (insideValue >= 1.5f) //sun, moved this condition inside previous if. This was causing crash with triangles added in previous steps (triangle idx outside of terrainInsideModelsVerts range), and we dont need to process those
                            allInBorder = false;
                    }
                }

                if (insideModel[0] && insideModel[1] && insideModel[2] && !allInBorder)
                    for (int j = 0; j < 3; ++j)  //clear triangle
                        meshData.solidTris[i * 3 + j] = 0;
            }

            //cleanup
            delete[] terrainInsideModelsVerts;
        }
        while (false);

        vmapManager.unloadMap(mapID, tileX, tileY);

        return retval;
    }

    void TerrainBuilder::unloadVMap(uint32 mapID, uint32 tileX, uint32 tileY)
    {
        vmapManager.unloadMap(mapID, tileX, tileY);
    }

    /**************************************************************************/
    void TerrainBuilder::transform(std::vector<G3D::Vector3> &source, std::vector<G3D::Vector3> &transformedVertices, float scale, G3D::Matrix3 &rotation, G3D::Vector3 &position)
    {
        for (std::vector<G3D::Vector3>::iterator it = source.begin(); it != source.end(); ++it)
        {
            // apply tranform, then mirror along the horizontal axes
            G3D::Vector3 v((*it) * rotation * scale + position);
            v.x *= -1.f;
            v.y *= -1.f;
            transformedVertices.push_back(v);
        }
    }

    /**************************************************************************/
    void TerrainBuilder::copyVertices(std::vector<G3D::Vector3> &source, G3D::Array<float> &dest)
    {
        for (std::vector<G3D::Vector3>::iterator it = source.begin(); it != source.end(); ++it)
        {
            dest.push_back((*it).y);
            dest.push_back((*it).z);
            dest.push_back((*it).x);
        }
    }

    /**************************************************************************/
    void TerrainBuilder::copyIndices(std::vector<MeshTriangle> &source, G3D::Array<int> &dest, int offset, bool flip)
    {
        if (flip)
        {
            for (std::vector<MeshTriangle>::iterator it = source.begin(); it != source.end(); ++it)
            {
                dest.push_back((*it).idx2+offset);
                dest.push_back((*it).idx1+offset);
                dest.push_back((*it).idx0+offset);
            }
        }
        else
        {
            for (std::vector<MeshTriangle>::iterator it = source.begin(); it != source.end(); ++it)
            {
                dest.push_back((*it).idx0+offset);
                dest.push_back((*it).idx1+offset);
                dest.push_back((*it).idx2+offset);
            }
        }
    }

    /**************************************************************************/
    void TerrainBuilder::copyIndices(G3D::Array<int> &source, G3D::Array<int> &dest, int offset)
    {
        int* src = source.getCArray();
        for (int32 i = 0; i < source.size(); ++i)
            dest.append(src[i] + offset);
    }

    /**************************************************************************/
    void TerrainBuilder::cleanVertices(G3D::Array<float> &verts, G3D::Array<int> &tris)
    {
        std::map<int, int> vertMap;

        int* t = tris.getCArray();
        float* v = verts.getCArray();

        G3D::Array<float> cleanVerts;
        int index, count = 0;
        // collect all the vertex indices from triangle
        for (int i = 0; i < tris.size(); ++i)
        {
            if (vertMap.find(t[i]) != vertMap.end())
                continue;
            std::pair<int, int> val;
            val.first = t[i];

            index = val.first;
            val.second = count;

            vertMap.insert(val);
            cleanVerts.append(v[index * 3], v[index * 3 + 1], v[index * 3 + 2]);
            count++;
        }

        verts.fastClear();
        verts.append(cleanVerts);
        cleanVerts.clear();

        // update triangles to use new indices
        for (int i = 0; i < tris.size(); ++i)
        {
            std::map<int, int>::iterator it;
            if ((it = vertMap.find(t[i])) == vertMap.end())
                continue;

            t[i] = (*it).second;
        }

        vertMap.clear();
    }

    /**************************************************************************/
    void TerrainBuilder::loadOffMeshConnections(uint32 mapID, uint32 tileX, uint32 tileY, MeshData &meshData, const char* offMeshFilePath)
    {
        // no meshfile input given?
        if (offMeshFilePath == NULL)
            return;

        FILE* fp = fopen(offMeshFilePath, "rb");
        if (!fp)
        {
            printf(" loadOffMeshConnections:: input file %s not found!\n", offMeshFilePath);
            return;
        }

        // pretty silly thing, as we parse entire file and load only the tile we need
        // but we don't expect this file to be too large
        char* buf = new char[512];
        while(fgets(buf, 512, fp))
        {
            float p0[3], p1[3];
            uint32 mid, tx, ty;
            float size;
            if (sscanf(buf, "%u %u,%u (%f %f %f) (%f %f %f) %f", &mid, &tx, &ty,
                &p0[0], &p0[1], &p0[2], &p1[0], &p1[1], &p1[2], &size) != 10)
                continue;

            if (mapID == mid && tileX == tx && tileY == ty)
            {
                meshData.offMeshConnections.append(p0[1]);
                meshData.offMeshConnections.append(p0[2]);
                meshData.offMeshConnections.append(p0[0]);

                meshData.offMeshConnections.append(p1[1]);
                meshData.offMeshConnections.append(p1[2]);
                meshData.offMeshConnections.append(p1[0]);

                meshData.offMeshConnectionDirs.append(1);          // 1 - both direction, 0 - one sided
                meshData.offMeshConnectionRads.append(size);       // agent size equivalent
                // can be used same way as polygon flags
                meshData.offMeshConnectionsAreas.append((unsigned char)0xFF);
                meshData.offMeshConnectionsFlags.append((unsigned short)0xFF);  // all movement masks can make this path
            }

        }

        delete [] buf;
        fclose(fp);
    }

    float TerrainBuilder::getHeight(uint32 mapID, float x, float y)
    {
        float* m_V8 = nullptr;
        float* m_V9 = nullptr;

        {
            boost::shared_lock_guard<boost::shared_mutex> lock(map_V_mutex);
            auto v8_itr = map_V8.find(mapID);
            auto v9_itr = map_V9.find(mapID);
            if (v8_itr == map_V8.end() || v9_itr == map_V9.end())
                return -50000;

            m_V8 = (*v8_itr).second;
            m_V9 = (*v9_itr).second;
        }

        x = MAP_RESOLUTION * (32 - x / GRID_SIZE);
        y = MAP_RESOLUTION * (32 - y / GRID_SIZE);

        int x_int = (int)x;
        int y_int = (int)y;
        x -= x_int;
        y -= y_int;
        x_int &= (MAP_RESOLUTION - 1);
        y_int &= (MAP_RESOLUTION - 1);

        // Height stored as: h5 - its v8 grid, h1-h4 - its v9 grid
        // +--------------> X
        // | h1-------h2     Coordinates is:
        // | | \  1  / |     h1 0,0
        // | |  \   /  |     h2 0,1
        // | | 2  h5 3 |     h3 1,0
        // | |  /   \  |     h4 1,1
        // | | /  4  \ |     h5 1/2,1/2
        // | h3-------h4
        // V Y
        // For find height need
        // 1 - detect triangle
        // 2 - solve linear equation from triangle points
        // Calculate coefficients for solve h = a*x + b*y + c

        float a, b, c;
        // Select triangle:
        if (x + y < 1)
        {
            if (x > y)
            {
                // 1 triangle (h1, h2, h5 points)
                float h1 = m_V9[(x_int) * 129 + y_int];
                float h2 = m_V9[(x_int + 1) * 129 + y_int];
                float h5 = 2 * m_V8[x_int * 128 + y_int];
                a = h2 - h1;
                b = h5 - h1 - h2;
                c = h1;
            }
            else
            {
                // 2 triangle (h1, h3, h5 points)
                float h1 = m_V9[x_int * 129 + y_int];
                float h3 = m_V9[x_int * 129 + y_int + 1];
                float h5 = 2 * m_V8[x_int * 128 + y_int];
                a = h5 - h1 - h3;
                b = h3 - h1;
                c = h1;
            }
        }
        else
        {
            if (x > y)
            {
                // 3 triangle (h2, h4, h5 points)
                float h2 = m_V9[(x_int + 1) * 129 + y_int];
                float h4 = m_V9[(x_int + 1) * 129 + y_int + 1];
                float h5 = 2 * m_V8[x_int * 128 + y_int];
                a = h2 + h4 - h5;
                b = h4 - h2;
                c = h5 - h4;
            }
            else
            {
                // 4 triangle (h3, h4, h5 points)
                float h3 = m_V9[(x_int) * 129 + y_int + 1];
                float h4 = m_V9[(x_int + 1) * 129 + y_int + 1];
                float h5 = 2 * m_V8[x_int * 128 + y_int];
                a = h4 - h3;
                b = h3 + h4 - h5;
                c = h5 - h4;
            }
        }
        // Calculate height
        return a * x + b * y + c;
    }

    /**
    * A triangle is undermap if there is no ground under
    * the notion of 'ground' is determined by the triangles normal orientations.
    * Note: Pos = {y, z, x}
    */
    bool TerrainBuilder::IsUnderMap(uint32 mapID, float* pos)
    {
        float terrainHeight = getHeight(mapID, pos[2], pos[0]);
        float z = pos[1];
        if (z + 0.2f > terrainHeight) // Over terrain
            return false;
        return vmapManager.isUnderModel(mapID, pos[2], pos[0], z + 0.2f);
    }
}
