#include "Chat.h"
#include "Management/MMapManager.h"
#include "Management/MMapFactory.h"
#include "PathGenerator.h"
#include "Transport.h"
#include "MoveSplineInit.h"
#include "WaypointManager.h"
#include "WaypointMovementGenerator.h"
#include "MapManager.h"
#include "ScriptSystem.h"
#include "SmartScriptMgr.h"
#include <fstream>

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
            { "reload",         SEC_GAMEMASTER3,     false, &HandleMmapReloadCommand,          "" },
            { "fixpath",        SEC_SUPERADMIN,      true,  &HandleFixPathCommand,             "" },
            { "fixpath_smart",  SEC_SUPERADMIN,      true,  &HandleFixPathSmartCommand,        "" },
            { "fixpath_escort", SEC_SUPERADMIN,      true,  &HandleFixPathEscortCommand,       "" },
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

    static bool HandleMmapReloadCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();

        int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
        int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

        player->GetMap()->ReloadMMap(gx, gy);
        handler->PSendSysMessage("Triggered reload from mmap at coord (%i,%i)", gx, gy);
        return true;
    }

    static void FillPathUsingMMaps(WaypointPath& path, WaypointPath const* original_path, Map const* map, WaypointPathType pathType)
    {
        bool lastPoint = false;
        for (auto itr = original_path->nodes.begin(); itr != original_path->nodes.end(); itr++)
        {
            WaypointNode newNode = *itr;
            newNode.temp = newNode.id + 1; //Mark this point as original and non removable
            path.nodes.push_back(newNode);
            auto nextItr = std::next(itr);
            if (nextItr == original_path->nodes.end())
            {
                if (pathType == WP_PATH_TYPE_LOOP)
                {
                    nextItr = original_path->nodes.begin();
                    lastPoint = true;
                }
                else
                    break;
            }

            G3D::Vector3 start = G3D::Vector3(itr->x, itr->y, itr->z);
            G3D::Vector3 end = G3D::Vector3(nextItr->x, nextItr->y, nextItr->z);
            G3D::Vector3 diff = end - start;

            PathGenerator pathGenerator(Position(start.x, start.y, start.z), map->GetId());
            pathGenerator.SetPathLengthLimit(diff.length() * 2);
            bool success = pathGenerator.CalculatePath(end.x, end.y, end.z, true, false);
            if (success)
                if (pathGenerator.GetPathType() & (PATHFIND_SHORT | PATHFIND_NOPATH | PATHFIND_INCOMPLETE | PATHFIND_NOT_USING_PATH))
                    success = false;

            if (success)
            {
                Movement::PointsArray points = pathGenerator.GetPath();
                points.erase(points.begin());
                points.pop_back();
                for (auto point : points)
                {
                    WaypointNode intermediateNode(0, point);
                    intermediateNode.moveType = newNode.moveType;
                    path.nodes.push_back(intermediateNode);
                }
            }

            if (lastPoint)
                break;
        }
    }

    static void DiscardUnnecessaryPoints(WaypointPath& path, WaypointPathType pathType)
    {
        float const MAX_GROUND_DIST = 1.5f;
        bool lastPoint = false;
        WaypointNode lastNode = *(path.nodes.begin());
        for (auto itr = ++path.nodes.begin(); itr != path.nodes.end();) //start at node 2
        {
            WaypointNode& currentNode = *itr;
            if (currentNode.temp) //point is original and non removable
                goto prepare_next_loop;

            {
                WaypointNode const& previousNode = lastNode;
                auto nextItr = std::next(itr);
                if (nextItr == path.nodes.end())
                {
                    if (pathType == WP_PATH_TYPE_LOOP)
                    {
                        nextItr = path.nodes.begin();
                        lastPoint = true;
                    }
                    else
                        break;
                }

                WaypointNode const& nextNode = *nextItr;

                G3D::Vector3 start(previousNode.x, previousNode.y, previousNode.z);
                G3D::Vector3 delta = G3D::Vector3(nextNode.x, nextNode.y, nextNode.z) - start;
                G3D::Vector3 deltaCurrent = G3D::Vector3(currentNode.x, currentNode.y, currentNode.z) - start;
                G3D::Vector3 pointCurrent = G3D::Vector3(currentNode.x, currentNode.y, currentNode.z);

                //approximative
                G3D::Vector3 pointLine = start + delta * (deltaCurrent.length() / delta.length());
                if ((pointCurrent - pointLine).length() < MAX_GROUND_DIST)
                {
                    itr = path.nodes.erase(itr);
                    //then last node is still the same
                    continue;
                }
            }

            if (lastPoint)
                break;

            //prepare next loop
        prepare_next_loop:
            lastNode = currentNode;
            itr++;
        }
    }

    /*
    Will try to insert point between path nodes whenever needed. Generate files in the working directory.
    Adapt scripts after that:
    - Check all occurence of WaypointPathStarted, WaypointStarted, WaypointReached, WaypointPathEnded
    - Check all smartai SMART_ACTION_LOAD_PATH
    */
    static bool HandleFixPathCommand(ChatHandler* handler, char const* args)
    { 
        CreatureDataContainer const& allCreatures = sObjectMgr->GetAllCreatureData();
        uint32 const DEBUG_LIMIT = 0; // stop after DEBUG_LIMIT modified if set
        uint32 const DEBUG_PATH_ID = 0; // only do this path if set 
        uint32 modified = 0;

        std::ofstream outfile("rework_path.txt", std::ofstream::out);
        std::ofstream outfileSQL("rework_path.sql", std::ofstream::out);

        std::vector<uint32> alreadyDone;

        for (auto itr : allCreatures)
        {
            if (DEBUG_LIMIT && modified >= DEBUG_LIMIT)
                break;

            uint32 spawnID = itr.first;
            CreatureData const& creData = itr.second;
            CreatureAddon const* addon = sObjectMgr->GetCreatureAddon(spawnID);
            if (!addon)
                continue;

            uint32 pathID = addon->path_id;
            if (!pathID)
                continue;

            if (DEBUG_PATH_ID && pathID != DEBUG_PATH_ID)
                continue;

            CreatureTemplate const* templatInfo = sObjectMgr->GetCreatureTemplate(creData.GetFirstSpawnEntry());
            if (!templatInfo)
            {
                ASSERT(false);
                continue;
            }

            CreatureMovementData const& movementData = templatInfo->Movement;
            if (movementData.IsFlightAllowed())
                continue;

            WaypointPath const* original_path = sWaypointMgr->GetPath(pathID);
            if (!original_path)
            {
                //unknown path, can't help that, that's a db error
                continue;
            }
            if (original_path->pathDirection == WP_PATH_DIRECTION_RANDOM)
            {
                //nothing to do with random path
                continue;
            }

            if (std::find(alreadyDone.begin(), alreadyDone.end(), pathID) != alreadyDone.end())
            {
                outfileSQL << "UPDATE `creature_addon` SET `path_id` = '" << pathID << "' WHERE `spawnID` = " << spawnID << ";" << std::endl << std::endl;
                continue;
            }

            Map const* map = sMapMgr->CreateBaseMap(creData.spawnPoint.GetMapId());
            if (!map)
                continue;

            WaypointPath path;
            if (original_path->nodes.size() == 1)
                continue;

            //step 1: Fill path with mmaps points
            FillPathUsingMMaps(path, original_path, map, WaypointPathType(original_path->pathType));
            
            //step 2: discard many many points
            DiscardUnnecessaryPoints(path, WaypointPathType(original_path->pathType));
         
            //step3: write ids
            uint32 id = original_path->nodes[0].id; //reuse same first id
            for (auto& itr : path.nodes)
                itr.id = id++;

            //step4: Write results to file
            bool changed = path.nodes.size() > original_path->nodes.size();
            bool warn = path.nodes.size() > original_path->nodes.size()*2;

            //if path was changed, write new path
            if (changed)
            {
                modified++;
                alreadyDone.push_back(pathID);

                outfileSQL << "DELETE FROM `waypoint_data` WHERE `id` = " << pathID << ";" << std::endl;
                outfile << "PATH: " << pathID << ", SPAWN: " << spawnID;
                if (warn)
                    outfile << ", WARN!";

                outfile << std::endl;
                outfileSQL << "INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_type`, `action`, `comment`) VALUES " << std::endl;
                auto itr2 = path.nodes.begin();
                for (; itr2 != path.nodes.end(); itr2++)
                {
                    outfile << pathID << ',' << itr2->id << ',' << itr2->x << ',' << itr2->y << ',' << itr2->z << ',' << itr2->orientation << ',' << itr2->delay << ',' << itr2->moveType << ',' << itr2->eventId << std::endl;

                    outfileSQL << "(" << pathID << ", " << itr2->id << ", " << itr2->x << ", " << itr2->y << ", " << itr2->z << ", " << itr2->orientation << ", " << itr2->delay << ", " << itr2->moveType << ", ";
                    if (itr2->eventId)
                        outfileSQL << itr2->eventId;
                    else
                        outfileSQL << "NULL";

                    if (itr2->temp)
                    {
                        uint32 originalID = uint32(itr2->temp - 1);
                        outfile << " (original id:" << originalID << ")";

                        auto result = WorldDatabase.PQuery("SELECT comment FROM waypoint_data WHERE id = %u AND point = %u", pathID, originalID);
                        if (!result)
                        {
                            handler->PSendSysMessage("Path %u point %u not found???????", pathID, originalID);
                            return true;
                        }
                        std::string comment;
                        if (originalID != itr2->id)
                            comment += "original id: " + std::to_string(originalID) + " / ";
                        comment += result->Fetch()[0].GetString();
                        outfileSQL << ", '" << comment.c_str() << "')";
                    }
                    else
                        outfileSQL << ", 'autogenerated with `.mmap fixpath`')";

                    if (std::next(itr2) != path.nodes.end())
                        outfileSQL << ",";
                    else
                        outfileSQL << ";";

                    outfileSQL << std::endl;
                }
                outfileSQL << "UPDATE `creature_addon` SET `path_id` = '" << pathID << "' WHERE `spawnID` = " << spawnID << ";" << std::endl << std::endl;
            }
        }
        outfile.close();
        outfileSQL.close();
        handler->PSendSysMessage("Finished %u", modified);
        return true;
    }
    
    static Optional<uint32> FindMapForCreature(uint32 npcID)
    {
        static std::unordered_map<uint32 /*npcID*/, uint32 /*map*/> staticMaps;
        if (staticMaps.empty())
        {
            staticMaps[17077] = 530;
            staticMaps[19685] = 530;
            staticMaps[20129] = 1;
            staticMaps[17393] = 530;
            staticMaps[3694]  = 1;
            staticMaps[13716] = 349;
            staticMaps[17405] = 530;
            staticMaps[14353] = 429;
            staticMaps[21768] = 530;
            staticMaps[17379] = 530;
            staticMaps[17418] = 530;
            staticMaps[17318] = 530;
            staticMaps[17391] = 530;
            staticMaps[17404] = 530;
            staticMaps[17417] = 530;
            staticMaps[18154] = 530;
            staticMaps[19604] = 530;
            staticMaps[19616] = 530;
            staticMaps[21181] = 530;
            staticMaps[20802] = 530;
            staticMaps[21409] = 530;
            staticMaps[21410] = 530;
            staticMaps[21867] = 530;
            staticMaps[22507] = 530;
            staticMaps[20438] = 530;
            staticMaps[20439] = 530;
        }

        auto itr = staticMaps.find(npcID);
        if (itr != staticMaps.end())
            return Optional<uint32>(itr->second);

        CreatureDataContainer const& allCreatures = sObjectMgr->GetAllCreatureData();
        for (auto itr : allCreatures)
        {
            CreatureData const& creData = itr.second;
            if (creData.GetFirstSpawnEntry() != npcID) //not actually correct but there are very few creatures using several entries
                continue;

            return Optional<uint32>(creData.spawnPoint.GetMapId());
        }
        return Optional<uint32>();
    }

    static bool HandleFixPathSmartCommand(ChatHandler* handler, char const* args)
    {
        uint32 modified = 0;

        std::ofstream outfileSQL("rework_path_smart.sql", std::ofstream::out);

        struct PathList
        {
            uint32 waypointID;
            uint32 entry;
            int32 entryorguid;
            uint8 sourceType;
        };
        //1 - list paths
        static std::unordered_map<uint32 /*npcID*/, uint32 /*map*/> staticMatch;
        if (staticMatch.empty())
        {
            staticMatch[57109] = 16700;
            staticMatch[57110] = 16700;
            staticMatch[57319] = 16831;
            staticMatch[57357] = 16842;
            staticMatch[57568] = 16842;
            staticMatch[57569] = 16842;
            staticMatch[62864] = 17845;
            staticMatch[68118] = 17845; //not right npc but right map
            staticMatch[68121] = 17845; //not right npc but right map
            staticMatch[68865] = 17845; //not right npc but right map
            staticMatch[68868] = 17845; //not right npc but right map
            staticMatch[143540] = 14354; 
            staticMatch[287080] = 14354; 
            staticMatch[310238] = 16700; 
            staticMatch[311588] = 17845; //not right npc but right map
            staticMatch[430620] = 14354;
            staticMatch[1366600] = 17845; //not right npc but right map
            staticMatch[10000000] = 0;
            staticMatch[10000001] = 0;
            staticMatch[10000002] = 0;
            staticMatch[10000003] = 0;  
            staticMatch[10000004] = 0;  
            staticMatch[12345678] = 0;
            staticMatch[16777215] = 0; //not right npc but right map
        }

        for (auto storePath : sSmartWaypointMgr->GetWaypointStore())
        {
            uint32 entry = storePath.first;
            uint32 creatureID = entry;
            WaypointPath const& original_path = storePath.second;

            auto itr = staticMatch.find(entry);
            if (itr != staticMatch.end())
            {
                creatureID = itr->second;
                if (!creatureID)
                    continue;
            }

            CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(creatureID);
            if (!info)
                info = sObjectMgr->GetCreatureTemplate(creatureID / 100);
            if (!info)
            {
                handler->PSendSysMessage("Could not find creature for entry %u", creatureID);
                return true;
            }

            CreatureMovementData const& movementData = info->Movement;
            if (movementData.IsFlightAllowed())
                continue;

            Optional<uint32> _mapID = FindMapForCreature(info->Entry);
            if (!_mapID.is_initialized())
            {
                //spawned via script?
                handler->PSendSysMessage("Could not find map for npc %u, need fix", info->Entry);
                return true;
            }

            Map const* map = sMapMgr->CreateBaseMap(*_mapID);
            if (!map)
                continue;

            WaypointPath path;
            if (original_path.nodes.size() == 1)
                continue;

            //step 1: Fill path with mmaps points
            FillPathUsingMMaps(path, &original_path, map, WP_PATH_TYPE_ONCE);

            //step 2: discard many many points
            DiscardUnnecessaryPoints(path, WP_PATH_TYPE_ONCE);

            //step3: write ids
            uint32 id = original_path.nodes[0].id; //reuse same first id
            for (auto& itr : path.nodes)
                itr.id = id++;

            //step4: Write results to file
            bool changed = path.nodes.size() > original_path.nodes.size();
            //if path was changed, write new path
            if (changed)
            {
                modified++;
                outfileSQL << "DELETE FROM `waypoints` WHERE `entry` = " << entry << ";" << std::endl;
                outfileSQL << "INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `point_comment`) VALUES " << std::endl;
                for (auto itr2 = path.nodes.begin(); itr2 != path.nodes.end(); itr2++)
                {
                    outfileSQL << "(" << entry << ", " << itr2->id << ", " << itr2->x << ", " << itr2->y << ", " << itr2->z << ", ";

                    if (itr2->temp)
                    {
                        uint32 originalID = uint32(itr2->temp - 1);
                        auto result = WorldDatabase.PQuery("SELECT point_comment FROM waypoints WHERE entry = %u AND pointid = %u", entry, originalID);
                        if (!result)
                        {
                            handler->PSendSysMessage("Path %u point %u not found???????", entry, originalID);
                            return true;
                        }
                        std::string comment;
                        if (originalID != itr2->id)
                            comment += "original id: " + std::to_string(originalID) + " / ";
                        comment += result->Fetch()[0].GetString();
                        outfileSQL << "'" << comment.c_str() << "')";
                    }
                    else
                        outfileSQL << "'autogenerated with `.mmap fixpath`')";

                    if (std::next(itr2) != path.nodes.end())
                        outfileSQL << ",";
                    else
                        outfileSQL << ";";

                    outfileSQL << std::endl;
                }
                //list entryorguid using this path
                struct SmartUser
                {
                    int32 entryorguid;
                    uint8 sourceType;
                };
                std::vector<SmartUser> users;
                auto result = WorldDatabase.PQuery("SELECT DISTINCT entryorguid, source_type FROM smart_scripts WHERE action_type = 53 AND action_param2 = %u", entry);
                if (result)
                {
                    do
                    {
                        Field* fields = result->Fetch();
                        SmartUser user;
                        user.entryorguid = fields[0].GetInt32();
                        user.sourceType = fields[1].GetUInt8();
                        users.push_back(user);
                    } while (result->NextRow());
                }

                for (SmartUser const& user : users)
                {
                    for (auto itr2 = path.nodes.rbegin(); itr2 != path.nodes.rend(); itr2++) //iterate in reverse, want to affect higher ids first
                    {
                        if (itr2->temp)
                        {
                            uint32 originalID = uint32(itr2->temp - 1);
                            if (originalID == itr2->id)
                                continue;

                            //SMART_EVENT_WAYPOINT_REACHED
                            outfileSQL << "UPDATE smart_scripts SET event_param1 = " << itr2->id << " WHERE entryorguid = " << user.entryorguid << " AND source_type = " << uint32(user.sourceType) << " AND event_type IN (40, 56, 57, 58) AND event_param1 = " << originalID << " AND event_param2 IN (0, " << entry << ");" << std::endl;
                            if(user.sourceType == 9)
                                outfileSQL << "UPDATE smart_scripts SET event_param1 = " << itr2->id << " WHERE entryorguid = " << user.entryorguid / 100 << " AND source_type = 0 AND event_type IN (40, 56, 57, 58) AND event_param1 = " << originalID << " AND event_param2 IN (0, " << entry << ");" << std::endl;
                        }
                    }
                }
            }
        }

        outfileSQL.close();
        handler->PSendSysMessage("Finished %u", modified);
        return true;
    }

    static bool HandleFixPathEscortCommand(ChatHandler* handler, char const* args)
    {
        uint32 modified = 0;

        std::ofstream outfileSQL("rework_path_escort.sql", std::ofstream::out);

        SystemMgr::ScriptWaypointStore store = sScriptSystemMgr->GetWaypointStore();
        for (auto itr : store)
        {
            uint32 entry = itr.first;
            WaypointPath const& original_path = itr.second;
            Optional<uint32> _mapID = FindMapForCreature(entry);
            if (!_mapID.is_initialized())
            {
                //spawned via script?
                handler->PSendSysMessage("Could not find map for npc %u, need fix", entry);
                return true;
            }

            Map const* map = sMapMgr->CreateBaseMap(*_mapID);
            if (!map)
                continue;

            WaypointPath path;
            if (original_path.nodes.size() == 1)
                continue;

            //step 1: Fill path with mmaps points
            FillPathUsingMMaps(path, &original_path, map, WP_PATH_TYPE_ONCE);

            //step 2: discard many many points
            DiscardUnnecessaryPoints(path, WP_PATH_TYPE_ONCE);

            //step3: write ids
            uint32 id = original_path.nodes[0].id; //reuse same first id
            for (auto& itr : path.nodes)
                itr.id = id++;

            //step4: Write results to file

            bool changed = path.nodes.size() > original_path.nodes.size();
            //if path was changed, write new path
            if (changed)
            {
                modified++;
                outfileSQL << "DELETE FROM `script_waypoint` WHERE `entry` = " << entry << ";" << std::endl;
                outfileSQL << "INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`, `point_comment`) VALUES " << std::endl;
                auto itr2 = path.nodes.begin();
                for (; itr2 != path.nodes.end(); itr2++)
                {
                    outfileSQL << "(" << entry << ", " << itr2->id << ", " << itr2->x << ", " << itr2->y << ", " << itr2->z << ", " << itr2->delay << ", ";

                    if (itr2->temp)
                    {
                        uint32 originalID = uint32(itr2->temp - 1);
                        auto result = WorldDatabase.PQuery("SELECT point_comment FROM script_waypoint WHERE entry = %u AND pointid = %u", entry, originalID);
                        if (!result)
                        {
                            handler->PSendSysMessage("Path %u point %u not found???????", entry, originalID);
                            return true;
                        }
                        std::string comment;
                        if(originalID != itr2->id)
                            comment += "original id: " + std::to_string(originalID) + " / ";
                        comment += result->Fetch()[0].GetString();
                        outfileSQL << "'" << comment.c_str() << "')";
                    }
                    else
                        outfileSQL << "'autogenerated with `.mmap fixpath`')";

                    if (std::next(itr2) != path.nodes.end())
                        outfileSQL << ",";
                    else
                        outfileSQL << ";";

                    outfileSQL << std::endl;
                }
            }
        }
        outfileSQL.close();
        handler->PSendSysMessage("Finished %u", modified);
        return true;
    }
};

void AddSC_mmaps_commandscript()
{
    new mmaps_commandscript();
}
