
#ifndef _MAPTREE_H
#define _MAPTREE_H

#include "Define.h"
#include "BoundingIntervalHierarchy.h"
#include <unordered_map>

namespace VMAP
{
    class ModelInstance;
    class GroupModel;
    class VMapManager2;
    enum class LoadResult : uint8;
    enum class ModelIgnoreFlags : uint32;

    struct LocationInfo
    {
        LocationInfo(): hitInstance(0), hitModel(0), ground_Z(-G3D::inf()), rootId(0) { }
        int32 rootId;
        const ModelInstance* hitInstance;
        const GroupModel* hitModel;
        float ground_Z;
    };

    class TC_COMMON_API StaticMapTree
    {
        typedef std::unordered_map<uint32, bool> loadedTileMap;
        typedef std::unordered_map<uint32, uint32> loadedSpawnMap;
        private:
            uint32 iMapID;
            bool iIsTiled;
            BIH iTree;
            ModelInstance* iTreeValues; // the tree entries
            uint32 iNTreeValues;

            // Store all the map tile idents that are loaded for that map
            // some maps are not splitted into tiles and we have to make sure, not removing the map before all tiles are removed
            // empty tiles have no tile file, hence map with bool instead of just a set (consistency check)
            loadedTileMap iLoadedTiles;
            // stores <tree_index, reference_count> to invalidate tree values, unload map, and to be able to report errors
            loadedSpawnMap iLoadedSpawns;
            std::string iBasePath;

        private:
            /**
            If intersection is found within pMaxDist, sets pMaxDist to intersection distance and returns true.
            Else, pMaxDist is not modified and returns false;
            */
            bool getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit, ModelIgnoreFlags ignoreFlags) const;
            //bool containsLoadedMapTile(unsigned int pTileIdent) const { return(iLoadedMapTiles.containsKey(pTileIdent)); }
        public:
            static std::string getTileFileName(uint32 mapID, uint32 tileX, uint32 tileY);
            static uint32 packTileID(uint32 tileX, uint32 tileY) { return tileX<<16 | tileY; }
            static void unpackTileID(uint32 ID, uint32 &tileX, uint32 &tileY) { tileX = ID>>16; tileY = ID&0xFF; }
            static LoadResult CanLoadMap(const std::string &basePath, uint32 mapID, uint32 tileX, uint32 tileY);

            StaticMapTree(uint32 mapID, std::string basePath);
            ~StaticMapTree();

            bool isInLineOfSight(const G3D::Vector3& pos1, const G3D::Vector3& pos2, ModelIgnoreFlags ignoreFlags) const;
            /**
            When moving from pos1 to pos2 check if we hit an object. Return true and the position if we hit one
            Return the hit pos or the original dest pos
            */
            bool getObjectHitPos(const G3D::Vector3& pos1, const G3D::Vector3& pos2, G3D::Vector3& pResultHitPos, float pModifyDist) const;
            /** Return closest z position DOWNWARDS for given position within maxSearchDist (search up and down)
            Returns G3D::inf() if no z found
            */
            float getHeight(const G3D::Vector3& pPos, float maxSearchDist) const;
            bool getAreaInfo(G3D::Vector3 &pos, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupId) const;
            bool GetLocationInfo(const G3D::Vector3 &pos, LocationInfo &info) const;

            bool InitMap(const std::string &fname, VMapManager2* vm);
            void UnloadMap(VMapManager2* vm);
            bool LoadMapTile(uint32 tileX, uint32 tileY, VMapManager2* vm);
            void UnloadMapTile(uint32 tileX, uint32 tileY, VMapManager2* vm);
            bool isTiled() const { return iIsTiled; }
            uint32 numLoadedTiles() const { return iLoadedTiles.size(); }
            void getModelInstances(ModelInstance* &models, uint32 &count);

        private:
            StaticMapTree(StaticMapTree const& right) = delete;
            StaticMapTree& operator=(StaticMapTree const& right) = delete;
    };

    struct AreaInfo
    {
        AreaInfo(): result(false), ground_Z(-G3D::inf()), flags(0), adtId(0),
            rootId(0), groupId(0) { }
        bool result;
        float ground_Z;
        uint32 flags;
        int32 adtId;
        int32 rootId;
        int32 groupId;
    };
}                                                           // VMAP

#endif // _MAPTREE_H
