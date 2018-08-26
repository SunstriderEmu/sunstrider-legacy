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
#include "PathMovementBase.h"
#include "Player.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  3 * MINUTE * IN_MILLISECONDS           // 3 Minutes
#define TIMEDIFF_NEXT_WP      250

enum WaypointPathType : uint32
{
    WP_PATH_TYPE_LOOP           = 0,
    /* Note that for ONCE, the creature must have another movement generator if you don't want the waypoint generator to be started again. 
    So you'll get no problem with creature where the path is loaded dynamically (as these creature at least the IDLE generator beneath), 
    but when used from creature_addon the path will loop anyway */
    WP_PATH_TYPE_ONCE           = 1,
    WP_PATH_TYPE_ROUND_TRIP     = 2,

    WP_PATH_TYPE_UNSPECIFIED    = 100000, //special value

    WP_PATH_TYPE_TOTAL
};

TC_GAME_API std::string GetWaypointPathTypeName(WaypointPathType type);

enum WaypointPathDirection
{
    WP_PATH_DIRECTION_NORMAL    = 0,
    WP_PATH_DIRECTION_REVERSE   = 1, //travel waypoints decrementaly
    WP_PATH_DIRECTION_RANDOM    = 2,

    WP_PATH_DIRECTION_TOTAL
};

TC_GAME_API std::string GetWaypointPathDirectionName(WaypointPathDirection dir);

template<class T>
class WaypointMovementGenerator;

/*
Completely rewritten for sunstrider
You can set this path as repeatable or not with SetPathType.
Default type is WP_PATH_TYPE_LOOP.
Creature will have UNIT_STATE_ROAMING_MOVE and UNIT_STATE_ROAMING when currently moving.
*/
template<>
class TC_GAME_API WaypointMovementGenerator<Creature> : public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
    public PathMovementBase<Creature, WaypointPath const*>
{
    public:
        /* 
        repeating: path will use its default value, either WP_PATH_TYPE_LOOP or any value specified in waypoint_info table. Using this argument will override the default value.
        smoothSpline: will calculate path to further points to allow using smooth splines. This has better visuals (for flying creatures only) but can lead to more imprecise positions, plus it has bad visual when pausing the waypoint */
        explicit WaypointMovementGenerator(Movement::PointsArray& points, Optional<bool> repeating = {}, bool smoothSpline = false);
        explicit WaypointMovementGenerator(WaypointPath& path, Optional<bool> repeating = {}, bool smoothSpline = false);
        // If path_id is left at 0, will try to get path id from Creature::GetWaypointPathId()
        explicit WaypointMovementGenerator(uint32 _path_id = 0, Optional<bool> repeating = {}, bool smoothSpline = false);
        ~WaypointMovementGenerator() override;

        MovementGeneratorType GetMovementGeneratorType() const override;

        void UnitSpeedChanged() override { AddFlag(MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING); }

        void Pause(uint32 timer = 0) override;
        void Resume(uint32 overrideTimer = 0) override;

        bool DoInitialize(Creature*);
        void DoFinalize(Creature* owner, bool active, bool movementInform);
        void DoReset(Creature*);
        bool DoUpdate(Creature*, uint32 diff);
        void DoDeactivate(Creature*);

        uint32 GetSplineId() const override { return _splineId; }

        // Load path (from Creature::GetWaypointPathId) and start it
        bool LoadPath(Creature*);
        
        WaypointPathType GetPathType() const { return path_type; }
        //return true if argument is correct
        bool SetPathType(WaypointPathType type);

        WaypointPathDirection GetPathDirection() const { return direction; }
        //return true if argument is correct
        bool SetDirection(WaypointPathDirection dir);

        bool GetResetPosition(Unit*, float& x, float& y, float& z) override;

        void SplineFinished(Creature* creature, uint32 splineId);

        bool GetCurrentDestinationPoint(Creature* creature, Position& pos) const;

    private:
        void MovementInform(Creature*, uint32 DBNodeId);

        // Return if node is last waypoint depending on current direction
        bool IsLastMemoryNode(uint32 node);

        // Get next node as current depending on direction, return false if already at last node
        //Enable allowReverseDirection to allow generator direction to revert if type is WP_PATH_TYPE_ROUND_TRIP and we're at path end
        bool HasNextMemoryNode(uint32 fromNode, bool allowReverseDirection = true);
        bool GetNextMemoryNode(uint32 fromNode, uint32& nextNode, bool allowReverseDirection = true);
        
        // Get first node in path (depending on direction)
        uint32 GetFirstMemoryNode();

        // Pause path execution for given time. This does not stop current spline movement.
        void Pause(int32 time);

        /* Handle point relative stuff (memory inform, script, delay)
        arrivedNodeIndex = index in _path
        */
        void OnArrived(Creature*, uint32 arrivedNodeIndex);

        /* Fill _precomputedPath with data from _path according to current node (that is, points until next stop), then start spline path
        nextNode = skip current node
        Returns false on error getting new point
        */
        bool StartMove(Creature* c);
        //meant to be used by StartSplinePath only. Return false if should break in loop
        bool GeneratePathToNextPoint(Position const& from, Creature* creature, WaypointNode const& nextNode, uint32& splineId);

        bool IsPaused();

        // Update pause timer if any and return wheter we can continue. Return false if not pausing at the moment.
        bool UpdatePause(int32 diff);
        
        //create a new customPath object with given array
        bool CreateCustomPath(Movement::PointsArray&);

        TimeTrackerSmall _nextMoveTime; //timer for pauses
        uint32 path_id;
        //this movement generator can be constructed with either a path id or with given points, stored in customPath in this second case
        WaypointPath* customPath;
        bool erasePathAtEnd; //
        WaypointPathType path_type;
        WaypointPathDirection direction;

        Movement::PointsArray _precomputedPath;
        uint32 _useSmoothSpline; //calculate path to further points to allow using smooth splines
        bool _recalculateTravel;

        Position _originalHome; //original home position before it was altered by this movegen. Some scripts are currently using the home AFTER the waypoint path. (such as 18970)

        uint32 _splineId;
        //true when creature has reached the start node in path (it has to travel from its current position first)
        uint32 _reachedFirstNode;
        bool _done;

        typedef std::unordered_map<uint32 /*splineId*/, uint32 /*pathNodeId*/> SplineToPathIdMapping;
        //filled at spline path generation. Used to determine which node spline system reached. When spline id is finished, it means we've reached path id.
        SplineToPathIdMapping splineToPathIds;

        typedef std::unordered_map<uint32 /*splineId*/, uint32 /*pathNodeId*/> PathIdToPathIndexMapping;
        //filled at initial path loading. Used to update _currentNode when a new spline node is reached.
        PathIdToPathIndexMapping pathIdsToPathIndexes;
};

#endif
