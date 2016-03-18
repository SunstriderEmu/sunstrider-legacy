/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_WAYPOINTMOVEMENTGENERATOR_H
#define TRINITY_WAYPOINTMOVEMENTGENERATOR_H

/** @page PathMovementGenerator is used to generate movements
 * of waypoints and flight paths.  Each serves the purpose
 * of generate activities so that it generates updated
 * packets for the players.
 */

#include "MovementGenerator.h"
#include "WaypointManager.h"

#include "Player.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  3 * MINUTE * IN_MILLISECONDS           // 3 Minutes
#define TIMEDIFF_NEXT_WP      250

enum WaypointPathType
{
    WP_PATH_TYPE_LOOP           = 0,
    /* Note that for ONCE, the creature must have another movement generator if you don't want the waypoint generator to be started again. 
    So you'll get no problem with creature where the path is loaded dynamically (as these creature at least the IDLE generator beneath), 
    but when used from creature_addon the path will loop anyway */
    WP_PATH_TYPE_ONCE           = 1,
    WP_PATH_TYPE_ROUND_TRIP     = 2,

    WP_PATH_TYPE_TOTAL
};

std::string GetWaypointPathTypeName(WaypointPathType type);

enum WaypointPathDirection
{
    WP_PATH_DIRECTION_NORMAL    = 0,
    WP_PATH_DIRECTION_REVERSE   = 1, //travel waypoints decrementaly
    WP_PATH_DIRECTION_RANDOM    = 2,

    WP_PATH_DIRECTION_TOTAL
};

std::string GetWaypointPathDirectionName(WaypointPathDirection dir);

template<class T, class P>
class PathMovementBase
{
    public:
        PathMovementBase() : i_path(), i_currentNode(0) { }
        virtual ~PathMovementBase() { };

        // template pattern, not defined .. override required
        uint32 GetCurrentNode() const { return i_currentNode; }

    protected:
        P i_path;
        //The node we're currently going to. Index for i_path. /!\ not always equal to path id in db (db path may not start at 0 or all points be contiguous)
        uint32 i_currentNode;
};

template<class T>
class WaypointMovementGenerator;

/*
*Completely rewritten by Kelno*
You can set this path as repeatable or not with SetPathType.
Default type is WP_PATH_TYPE_LOOP.
Creature will have UNIT_STATE_ROAMING_MOVE and UNIT_STATE_ROAMING when currently moving.
This generator does NOT used mmaps to create path between points

TODO:
- mmaps
- cyclic choz (check if position equals then skip
*/
template<>
class WaypointMovementGenerator<Creature> : public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
    public PathMovementBase<Creature, WaypointPath const*>
{
    public:
        WaypointMovementGenerator(Movement::PointsArray& points);
        // If path_id is left at 0, will try to get path id from Creature::GetWaypointPathId()
        WaypointMovementGenerator(uint32 _path_id = 0);
        ~WaypointMovementGenerator();
        void DoInitialize(Creature*);
        void DoFinalize(Creature*);
        void DoReset(Creature*);
        bool DoUpdate(Creature*, uint32 diff);

        void UnitSpeedChanged() override { i_recalculatePath = true; }

        void MovementInform(Creature*, uint32 DBNodeId);

        MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

        uint32 GetSplineId() const override { return _splineId; }

        // Load path (from Creature::GetWaypointPathId) and start it
        bool LoadPath(Creature*);
        
        WaypointPathType GetPathType() { return path_type; }
        //return true if argument is correct
        bool SetPathType(WaypointPathType type);

        WaypointPathDirection GetPathDirection() { return direction; }
        //return true if argument is correct
        bool SetDirection(WaypointPathDirection dir);

        bool GetResetPos(Creature*, float& x, float& y, float& z) const;

        void SplineFinished(Creature* creature, uint32 splineId);

        bool GetCurrentDestinationPoint(Creature* creature, Position& pos) const;

    private:
        // Return if node is last waypoint depending on current direction
        bool IsLastMemoryNode(uint32 node);

        // Get next node as current depending on direction, return false if already at last node
        //Enable allowReverseDirection to allow generator direction to revert if type is WP_PATH_TYPE_ROUND_TRIP and we're at path end
        bool GetNextMemoryNode(uint32 fromNode, uint32& nextNode, bool allowReverseDirection = true);
        
        // Get first node in path (depending on direction)
        uint32 GetFirstMemoryNode();

        // Pause path execution for given time. This does not stop current spline movement.
        void Pause(int32 time);

        /* Handle point relative stuff (memory inform, script, delay)
        arrivedNodeIndex = index in i_path
        */
        void OnArrived(Creature*, uint32 arrivedNodeIndex);

        /* Fill m_precomputedPath with data from i_path according to current node (that is, points until next stop), then start spline path
        nextNode = skip current node
        */
        bool StartSplinePath(Creature* c, bool nextNode = false);
        //meant to be used by StartSplinePath only. Return false if should break in loop
        bool GeneratePathToNextPoint(Position const& from, Creature* creature, WaypointData* nextNode, uint32& splineId);

        bool IsPaused();

        // Update pause timer if any and return wheter we can continue. Return false if not pausing at the moment.
        bool UpdatePause(int32 diff);

        // Remove pause if any and <insert StartSplinePath comment here>
        bool StartMoveNow(Creature* creature, bool nextNode = false)
        {
            i_nextMoveTime.Reset(0);
            return StartSplinePath(creature, nextNode);
        }
        
        //create a new customPath object with given array
        bool CreateCustomPath(Movement::PointsArray&);

        TimeTrackerSmall i_nextMoveTime; //timer for pauses
        uint32 path_id;
        //this movement generator can be constructed with either a path id or with given points, stored in customPath in this second case
        WaypointPath* customPath;
        WaypointPathType path_type;
        WaypointPathDirection direction;

        Movement::PointsArray m_precomputedPath;
        bool i_recalculatePath;

        uint32 _splineId;
        //true when creature has reached the start node in path (it has to travel from its current position first)
        uint32 reachedFirstNode;

        typedef std::unordered_map<uint32 /*splineId*/, uint32 /*pathNodeId*/> SplineToPathIdMapping;
        //filled at spline path generation. Used to determine which node spline system reached. When spline id is finished, it means we've reached path id.
        SplineToPathIdMapping splineToPathIds;

        typedef std::unordered_map<uint32 /*splineId*/, uint32 /*pathNodeId*/> PathIdToPathIndexMapping;
        //filled at initial path loading. Used to update i_currentNode when a new spline node is reached.
        PathIdToPathIndexMapping pathIdsToPathIndexes;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class FlightPathMovementGenerator : public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
    public PathMovementBase<Player, TaxiPathNodeList>
{
    public:
        explicit FlightPathMovementGenerator(uint32 startNode = 0)
        {
            i_currentNode = startNode;
            _endGridX = 0.0f;
            _endGridY = 0.0f;
            _endMapId = 0;
            _preloadTargetNode = 0;
        }
        void LoadPath(Player* player);
        void DoInitialize(Player*);
        void DoReset(Player*);
        void DoFinalize(Player*);
        bool DoUpdate(Player*, uint32);
        MovementGeneratorType GetMovementGeneratorType() { return FLIGHT_MOTION_TYPE; }

        TaxiPathNodeList const& GetPath() { return i_path; }
        uint32 GetPathAtMapEnd() const;
        bool HasArrived() const { return (i_currentNode >= i_path.size()); }
        void SetCurrentNodeAfterTeleport();
        void SkipCurrentNode() { ++i_currentNode; }
        void DoEventIfAny(Player* player, TaxiPathNodeEntry const* node, bool departure);

        bool GetResetPos(Player*, float& x, float& y, float& z) const;

        void InitEndGridInfo();
        void PreloadEndGrid();

    private:
        float _endGridX;                //! X coord of last node location
        float _endGridY;                //! Y coord of last node location
        uint32 _endMapId;               //! map Id of last node location
        uint32 _preloadTargetNode;      //! node index where preloading starts

        struct TaxiNodeChangeInfo
        {
            uint32 PathIndex;
            int32 Cost;
        };

        std::deque<TaxiNodeChangeInfo> _pointsForPathSwitch;    //! node indexes and costs where TaxiPath changes
};
#endif
