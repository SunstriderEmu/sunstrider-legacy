#include "Chat.h"
#include "Management/MMapManager.h"
#include "Management/MMapFactory.h"
#include "PathGenerator.h"
#include "Transport.h"
#include "MoveSplineInit.h"

class mmaps_commandscript : public CommandScript
{
public:
    mmaps_commandscript() : CommandScript("mmaps_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> mmapCommandTable =
        {
            { "path",           SEC_GAMEMASTER3,     false, &HandleMmapPathCommand,            "" },
            { "loc",            SEC_GAMEMASTER3,     false, &HandleMmapLocCommand,             "" },
            { "loadedtiles",    SEC_GAMEMASTER3,     false, &HandleMmapLoadedTilesCommand,     "" },
            { "stats",          SEC_GAMEMASTER3,     false, &HandleMmapStatsCommand,           "" },
            { "testarea",       SEC_GAMEMASTER3,     false, &HandleMmapTestAreaCommand,        "" },
            { "reload",         SEC_GAMEMASTER3,     false, &handleMmapReloadCommand,          "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "mmap",           SEC_GAMEMASTER2,  false, nullptr,                              "", mmapCommandTable },
        };
        return commandTable;
    }

    static bool HandleMmapPathCommand(ChatHandler* handler, char const* args)
    {
        if (!MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(handler->GetSession()->GetPlayer()->GetMapId()))
        {
            handler->PSendSysMessage("NavMesh not loaded for current map.");
            return true;
        }

        handler->PSendSysMessage("mmap path:");

        // units
        Player* player = handler->GetSession()->GetPlayer();
        Unit* target = handler->GetSelectedUnit();
        if (!player || !target)
        {
            handler->PSendSysMessage("Invalid target/source selection.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* para = strtok((char*)args, " ");

        bool useStraightPath = false;
        bool excludeSteep = false;
        if (para)
        {
            if (strcmp(para, "straigth") == 0)
                useStraightPath = true;
            else if (strcmp(para, "nonsteep") == 0)
                excludeSteep = true;
        }

        // unit locations
        float x, y, z;
        player->GetPosition(x, y, z);

        // path
        PathGenerator path(target);
        path.SetUseStraightPath(useStraightPath);
        if (excludeSteep)
            path.ExcludeSteepSlopes();
        Transport* transport = player->GetTransport();
        path.SetTransport(transport);

        bool result = path.CalculatePath(x, y, z);

        Movement::PointsArray const& pointPath = path.GetPath();
        handler->PSendSysMessage("%s's path to %s:", target->GetName().c_str(), player->GetName().c_str());
        handler->PSendSysMessage("Building: %s", useStraightPath ? "StraightPath" : "SmoothPath");
        handler->PSendSysMessage("Result: %s - Length: %zu - Type: %u", (result ? "true" : "false"), pointPath.size(), path.GetPathType());

        G3D::Vector3 const &start = path.GetStartPosition();
        G3D::Vector3 const &end = path.GetEndPosition();
        G3D::Vector3 const &actualEnd = path.GetActualEndPosition();

        handler->PSendSysMessage("StartPosition     (%.3f, %.3f, %.3f)", start.x, start.y, start.z);
        handler->PSendSysMessage("EndPosition       (%.3f, %.3f, %.3f)", end.x, end.y, end.z);
        handler->PSendSysMessage("ActualEndPosition (%.3f, %.3f, %.3f)", actualEnd.x, actualEnd.y, actualEnd.z);

        if (!player->IsGameMaster())
            handler->PSendSysMessage("Enable GM mode to see the path points.");

        for (auto const & i : pointPath)
        {
            G3D::Vector3 targetPos(i);
            if (transport)
                transport->CalculatePassengerPosition(targetPos.x, targetPos.y, targetPos.z);
            if (Creature* wp = player->SummonCreature(VISUAL_WAYPOINT, targetPos.x, targetPos.y, targetPos.z, 0, TEMPSUMMON_TIMED_DESPAWN, 10000))
            {
                wp->SetDisableGravity(true);
                if (transport)
                    transport->AddPassenger(wp, true);

                wp->StopMovingOnCurrentPos(); //update position on client
            }
        }

        return true;
    }

    static bool HandleMmapLocCommand(ChatHandler* handler, char const* args)
    {
        bool playerWalkableOnly = false;
        if (args)
        {
            std::string _args(args);
            if (_args == "player")
                playerWalkableOnly = true;
        }

        handler->PSendSysMessage("mmap tileloc:");

        // grid tile location
        Player* player = handler->GetSession()->GetPlayer();

        int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
        int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

        handler->PSendSysMessage("%03u%02i%02i.mmtile", player->GetMapId(), gy, gx);
        handler->PSendSysMessage("gridloc [%i,%i]", gx, gy);

        // calculate navmesh tile location
        const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(player->GetMapId());
        const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(player->GetMapId(), player->GetInstanceId());
        if (!navmesh || !navmeshquery)
        {
            handler->PSendSysMessage("NavMesh not loaded for current map.");
            return true;
        }

        const float* min = navmesh->getParams()->orig;

        float x, y, z;
        player->GetPosition(x, y, z);
        float location[VERTEX_SIZE] = { y, z, x };
        float extents[VERTEX_SIZE] = { 2.f,4.f,2.f };

        int32 tilex = int32((y - min[0]) / SIZE_OF_GRIDS);
        int32 tiley = int32((x - min[2]) / SIZE_OF_GRIDS);

        handler->PSendSysMessage("Calc   [%02i,%02i]", tilex, tiley);

        // navmesh poly -> navmesh tile location
        dtQueryFilter filter = dtQueryFilter();
        if (playerWalkableOnly)
            filter.setExcludeFlags(NAV_STEEP_SLOPES);
        dtPolyRef polyRef = INVALID_POLYREF;
        navmeshquery->findNearestPoly(location, extents, &filter, &polyRef, nullptr);

        if (polyRef == INVALID_POLYREF)
            handler->PSendSysMessage("Dt     [??,??] (invalid poly, probably no tile loaded)");
        else
        {
            const dtMeshTile* tile;
            const dtPoly* poly;
            navmesh->getTileAndPolyByRef(polyRef, &tile, &poly);
            if (tile)
                handler->PSendSysMessage("Dt     [%02i,%02i]", tile->header->x, tile->header->y);
            else
                handler->PSendSysMessage("Dt     [??,??] (no tile loaded)");
        }

        if (player->GetMap()->IsPlayerWalkable(player->GetPosition()))
            handler->PSendSysMessage("Map::IsPlayerWalkable -> true");
        else
            handler->PSendSysMessage("Map::IsPlayerWalkable -> false");

        return true;
    }

    static bool HandleMmapLoadedTilesCommand(ChatHandler* handler, char const* /*args*/)
    {
        uint32 mapid = handler->GetSession()->GetPlayer()->GetMapId();

        const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(mapid);
        const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(mapid, handler->GetSession()->GetPlayer()->GetInstanceId());
        if (!navmesh || !navmeshquery)
        {
            handler->PSendSysMessage("NavMesh not loaded for current map.");
            return true;
        }

        handler->PSendSysMessage("mmap loadedtiles:");

        for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
        {
            const dtMeshTile* tile = navmesh->getTile(i);
            if (!tile || !tile->header)
                continue;

            handler->PSendSysMessage("[%02i,%02i]", tile->header->x, tile->header->y);
        }

        return true;
    }

    static bool HandleMmapStatsCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage("mmap stats:");

        MMAP::MMapManager *manager = MMAP::MMapFactory::createOrGetMMapManager();
        handler->PSendSysMessage(" %u maps loaded with %u tiles overall", manager->getLoadedMapsCount(), manager->getLoadedTilesCount());

        const dtNavMesh* navmesh = manager->GetNavMesh(handler->GetSession()->GetPlayer()->GetMapId());
        if (!navmesh)
        {
            handler->PSendSysMessage("NavMesh not loaded for current map.");
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

            tileCount++;
            nodeCount += tile->header->bvNodeCount;
            polyCount += tile->header->polyCount;
            vertCount += tile->header->vertCount;
            triCount += tile->header->detailTriCount;
            triVertCount += tile->header->detailVertCount;
            dataSize += tile->dataSize;
        }

        handler->PSendSysMessage("Navmesh stats on current map:");
        handler->PSendSysMessage(" %u tiles loaded", tileCount);
        handler->PSendSysMessage(" %u BVTree nodes", nodeCount);
        handler->PSendSysMessage(" %u polygons (%u vertices)", polyCount, vertCount);
        handler->PSendSysMessage(" %u triangles (%u vertices)", triCount, triVertCount);
        handler->PSendSysMessage(" %.2f MB of data (not including pointers)", ((float)dataSize / sizeof(unsigned char)) / 1048576);

        return true;
    }

    static bool HandleMmapTestAreaCommand(ChatHandler* handler, char const* /*args*/)
    {
        float radius = 40.0f;
        WorldObject* object = handler->GetSession()->GetPlayer();

        // Get Creatures
        std::list<Creature*> creatureList;
        Trinity::AnyUnitInObjectRangeCheck go_check(object, radius);
        Trinity::CreatureListSearcher<Trinity::AnyUnitInObjectRangeCheck> go_search(object, creatureList, go_check);
        Cell::VisitGridObjects(object, go_search, radius);

        if (!creatureList.empty())
        {
            handler->PSendSysMessage("Found %zu Creatures.", creatureList.size());

            uint32 paths = 0;
            uint32 uStartTime = GetMSTime();

            float gx, gy, gz;
            object->GetPosition(gx, gy, gz);
            for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
            {
                PathGenerator path(*itr);
                path.CalculatePath(gx, gy, gz);
                ++paths;
            }

            uint32 uPathLoadTime = GetMSTimeDiff(uStartTime, GetMSTime());
            handler->PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
        }
        else
            handler->PSendSysMessage("No creatures in %f yard range.", radius);

        return true;
    }

    static bool handleMmapReloadCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
        int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

        player->GetMap()->ReloadMMap(gx, gy);
        handler->PSendSysMessage("Triggered reload from mmap at coord (%i,%i)", gx, gy);
        return true;
    }
};

void AddSC_mmaps_commandscript()
{
    new mmaps_commandscript();
}
