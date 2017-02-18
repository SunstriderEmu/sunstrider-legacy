#include "Chat.h"
#include "Management/MMapManager.h"
#include "Management/MMapFactory.h"

bool ChatHandler::HandleMmapTestArea(const char* args)
{
    return true;
}


bool ChatHandler::HandleMmapPathCommand(const char* args)
{
    if (!MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(GetSession()->GetPlayer()->GetMapId()))
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    PSendSysMessage("mmap path:");

    // units
    Player* player = GetSession()->GetPlayer();
    Unit* target = GetSelectedUnit();
    if (!player || !target)
    {
       PSendSysMessage("Invalid target/source selection.");
       SetSentErrorMessage(true);
       return false;
    }

    char* para = strtok((char*)args, " ");

    bool useStraightPath = false;
    if (para && strcmp(para, "true") == 0)
        useStraightPath = true;

    // unit locations
    float x, y, z;
    player->GetPosition(x, y, z);

    // path
    PathGenerator path(target);
    path.SetUseStraightPath(useStraightPath);
    bool result = path.CalculatePath(x, y, z);

    Movement::PointsArray const& pointPath = path.GetPath();
    PSendSysMessage("%s's path to %s:", target->GetName().c_str(), player->GetName().c_str());
    PSendSysMessage("Building: %s", useStraightPath ? "StraightPath" : "SmoothPath");
    PSendSysMessage("Result: %s - Length: %zu - Type: %u", (result ? "true" : "false"), pointPath.size(), path.GetPathType());

    G3D::Vector3 const &start = path.GetStartPosition();
    G3D::Vector3 const &end = path.GetEndPosition();
    G3D::Vector3 const &actualEnd = path.GetActualEndPosition();

    PSendSysMessage("StartPosition     (%.3f, %.3f, %.3f)", start.x, start.y, start.z);
    PSendSysMessage("EndPosition       (%.3f, %.3f, %.3f)", end.x, end.y, end.z);
    PSendSysMessage("ActualEndPosition (%.3f, %.3f, %.3f)", actualEnd.x, actualEnd.y, actualEnd.z);

    if (!player->IsGameMaster())
        PSendSysMessage("Enable GM mode to see the path points.");

    for (const auto & i : pointPath)
        player->SummonCreature(VISUAL_WAYPOINT, i.x, i.y, i.z, 0, TEMPSUMMON_TIMED_DESPAWN, 9000);

    return true;
}

bool ChatHandler::HandleMmapLocCommand(const char* /*args*/)
{
    PSendSysMessage("mmap tileloc:");

    // grid tile location
    Player* player = m_session->GetPlayer();

    int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
    int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

    PSendSysMessage("%03u%02i%02i.mmtile", player->GetMapId(), gy, gx);
    PSendSysMessage("gridloc [%i,%i]", gx, gy);

    // calculate navmesh tile location
    const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(player->GetMapId());
    const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(player->GetMapId(), player->GetInstanceId());
    if (!navmesh || !navmeshquery)
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    const float* min = navmesh->getParams()->orig;

    float x, y, z;
    player->GetPosition(x, y, z);
    float location[VERTEX_SIZE] = {y, z, x};
    float extents[VERTEX_SIZE] = {2.f,4.f,2.f};

    int32 tilex = int32((y - min[0]) / SIZE_OF_GRIDS);
    int32 tiley = int32((x - min[2]) / SIZE_OF_GRIDS);

    PSendSysMessage("Calc   [%02i,%02i]", tilex, tiley);

    // navmesh poly -> navmesh tile location
    dtQueryFilter filter = dtQueryFilter();
    dtPolyRef polyRef = INVALID_POLYREF;
    navmeshquery->findNearestPoly(location, extents, &filter, &polyRef, nullptr);

    if (polyRef == INVALID_POLYREF)
        PSendSysMessage("Dt     [??,??] (invalid poly, probably no tile loaded)");
    else
    {
        const dtMeshTile* tile;
        const dtPoly* poly;
        navmesh->getTileAndPolyByRef(polyRef, &tile, &poly);
        if (tile)
            PSendSysMessage("Dt     [%02i,%02i]", tile->header->x, tile->header->y);
        else
            PSendSysMessage("Dt     [??,??] (no tile loaded)");
    }

    return true;
}

bool ChatHandler::HandleMmapLoadedTilesCommand(const char* /*args*/)
{
    

    uint32 mapid = m_session->GetPlayer()->GetMapId();

    const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(mapid);
    const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(mapid, m_session->GetPlayer()->GetInstanceId());
    if (!navmesh || !navmeshquery)
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    PSendSysMessage("mmap loadedtiles:");

    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        PSendSysMessage("[%02i,%02i]", tile->header->x, tile->header->y);
    }

    return true;
}

bool ChatHandler::HandleMmapStatsCommand(const char* /*args*/)
{
    PSendSysMessage("mmap stats:");

    MMAP::MMapManager *manager = MMAP::MMapFactory::createOrGetMMapManager();
    PSendSysMessage(" %u maps loaded with %u tiles overall", manager->getLoadedMapsCount(), manager->getLoadedTilesCount());

    const dtNavMesh* navmesh = manager->GetNavMesh(m_session->GetPlayer()->GetMapId());
    if (!navmesh)
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    uint32 tileCount = 0;
    uint32 nodeCount = 0;
    uint32 polyCount = 0;
    uint32 vertCount = 0;
    uint32 triCount = 0;
    uint32 triVertCount = 0;
    uint32 dataSize = 0;
    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        tileCount ++;
        nodeCount += tile->header->bvNodeCount;
        polyCount += tile->header->polyCount;
        vertCount += tile->header->vertCount;
        triCount += tile->header->detailTriCount;
        triVertCount += tile->header->detailVertCount;
        dataSize += tile->dataSize;
    }

    PSendSysMessage("Navmesh stats on current map:");
    PSendSysMessage(" %u tiles loaded", tileCount);
    PSendSysMessage(" %u BVTree nodes", nodeCount);
    PSendSysMessage(" %u polygons (%u vertices)", polyCount, vertCount);
    PSendSysMessage(" %u triangles (%u vertices)", triCount, triVertCount);
    PSendSysMessage(" %.2f MB of data (not including pointers)", ((float)dataSize / sizeof(unsigned char)) / 1048576);

    return true;
}
