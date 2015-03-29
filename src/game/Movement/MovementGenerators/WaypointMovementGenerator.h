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
#include "Path.h"

#include "Player.h"

#include <vector>
#include <set>

#define FLIGHT_TRAVEL_UPDATE  100
#define STOP_TIME_FOR_PLAYER  3 * MINUTE * IN_MILLISECONDS           // 3 Minutes
#define TIMEDIFF_NEXT_WP      250

enum WaypointPathType
{
    WP_PATH_TYPE_LOOP           = 0,
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
        PathMovementBase() : i_path(NULL), i_currentNode(0) { }
        virtual ~PathMovementBase() { };

        // template pattern, not defined .. override required
        void LoadPath(T &);
        uint32 GetCurrentNode() const { return i_currentNode; }

    protected:
        P i_path;
        uint32 i_currentNode;
};

template<class T>
class WaypointMovementGenerator;

template<>
class WaypointMovementGenerator<Creature> : public MovementGeneratorMedium< Creature, WaypointMovementGenerator<Creature> >,
    public PathMovementBase<Creature, WaypointPathNodes const*>
{
    public:
        WaypointMovementGenerator(uint32 _path_id = 0);
        ~WaypointMovementGenerator() { i_path = NULL; }
        void DoInitialize(Creature*);
        void DoFinalize(Creature*);
        void DoReset(Creature*);
        bool DoUpdate(Creature*, uint32 diff);

        WaypointPathNodes const& GetPath() { return *i_path; }

        void MovementInform(Creature*);

        MovementGeneratorType GetMovementGeneratorType() { return WAYPOINT_MOTION_TYPE; }

        // now path movement implmementation
        void LoadPath(Creature*);
        
        WaypointPathType GetPathType() { return path_type; }
        //return true if argument is correct
        bool SetPathType(WaypointPathType type);

        WaypointPathDirection GetPathDirection() { return direction; }
        //return true if argument is correct
        bool SetDirection(WaypointPathDirection dir);

        bool GetResetPos(Creature*, float& x, float& y, float& z);

    private:
        //return if last waypoint depending on current direction
        bool IsLastNode(uint32 node);
        //set next node as current depending on direction, return false if already at last node
        bool SetNextNode();
        
        void Stop(int32 time) { i_nextMoveTime.Reset(time);}

        bool Stopped() { return !i_nextMoveTime.Passed();}

        bool CanMove(int32 diff)
        {
            i_nextMoveTime.Update(diff);
            return i_nextMoveTime.Passed();
        }

        void OnArrived(Creature*);
        //return false if path generator must expire
        bool StartMove(Creature*);

        void StartMoveNow(Creature* creature)
        {
            i_nextMoveTime.Reset(0);
            StartMove(creature);
        }

        TimeTrackerSmall i_nextMoveTime;
        bool m_isArrivalDone;
        uint32 path_id;
        WaypointPathType path_type;
        WaypointPathDirection direction;
};

/** FlightPathMovementGenerator generates movement of the player for the paths
 * and hence generates ground and activities for the player.
 */
class FlightPathMovementGenerator : public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
    public PathMovementBase<Player, TaxiPathNodeList const*>
{
    public:
        explicit FlightPathMovementGenerator(TaxiPathNodeList const& pathnodes, uint32 startNode = 0)
        {
            i_path = &pathnodes;
            i_currentNode = startNode;
            _endGridX = 0.0f;
            _endGridY = 0.0f;
            _endMapId = 0;
            _preloadTargetNode = 0;
        }
        void DoInitialize(Player*);
        void DoReset(Player*);
        void DoFinalize(Player*);
        bool DoUpdate(Player*, uint32);
        MovementGeneratorType GetMovementGeneratorType() { return FLIGHT_MOTION_TYPE; }

        TaxiPathNodeList const& GetPath() { return *i_path; }
        uint32 GetPathAtMapEnd() const;
        bool HasArrived() const { return (i_currentNode >= i_path->size()); }
        void SetCurrentNodeAfterTeleport();
        void SkipCurrentNode() { ++i_currentNode; }
        void DoEventIfAny(Player* player, TaxiPathNodeEntry const& node, bool departure);

        bool GetResetPos(Player*, float& x, float& y, float& z);

        void InitEndGridInfo();
        void PreloadEndGrid();

    private:
        float _endGridX;                //! X coord of last node location
        float _endGridY;                //! Y coord of last node location
        uint32 _endMapId;               //! map Id of last node location
        uint32 _preloadTargetNode;      //! node index where preloading starts
};
#endif
