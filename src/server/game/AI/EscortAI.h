
#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

#include "ScriptedCreature.h"
#include "WaypointDefines.h"

#define DEFAULT_MAX_PLAYER_DISTANCE 50

struct Escort_Waypoint
{
    Escort_Waypoint(uint32 _id, float _x, float _y, float _z, uint32 _w)
    {
        id = _id;
        x = _x;
        y = _y;
        z = _z;
        WaitTimeMs = _w;
    }

    uint32 id;
    float x;
    float y;
    float z;
    uint32 WaitTimeMs;
};

enum eEscortState
{
    STATE_ESCORT_NONE       = 0x000,                        //nothing in progress
    STATE_ESCORT_ESCORTING  = 0x001,                        //escort are in progress
    STATE_ESCORT_RETURNING  = 0x002,                        //escort is returning after being in combat
    STATE_ESCORT_PAUSED     = 0x004                         //will not proceed with waypoints before state is removed
};

struct TC_GAME_API EscortAI : public ScriptedAI
{
    public:

        // Pure Virtual Functions
        virtual void WaypointReached(uint32, uint32) override = 0;

        void JustEngagedWith(Unit*) override = 0;

        void Reset() override = 0;

        // CreatureAI functions
		EscortAI(Creature *c);

        void MoveInLineOfSight(Unit*) override;
        void JustDied(Unit*) override;
        void FailQuest();
        void JustAppeared() override;
        void ReturnToLastPoint();
        void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override;
        void UpdateAI(const uint32) override;
        void MovementInform(uint32, uint32) override;

        virtual void UpdateEscortAI(uint32 diff); // used when it's needed to add code in update (abilities, scripted events, etc)
        // EscortAI functions

        //Use manually added waypoint. When using this, SetRun is ignored and unit flags are used instead
        void AddWaypoint(uint32 id, float x, float y, float z, float orientation = 0, uint32 waitTime = 0);

        void Start(bool isActiveAttacker = true, bool run = false, ObjectGuid playerGUID = ObjectGuid::Empty, Quest const* quest = nullptr, bool instantRespawn = false, bool canLoopPath = false, bool resetWaypoints = true);
        
        //sunstrider: keeping for compat but pretty useless, wp moveType will be used instead in all case except when using AddWaypoint
        void SetRun(bool on = true);

        void SetEscortPaused(bool uPaused);
        void SetPauseTimer(uint32 Timer) { _pauseTimer = Timer; }
        
        bool HasEscortState(uint32 uiEscortState) { return (_escortState & uiEscortState); }
        bool IsEscorted() const override { return (_escortState & STATE_ESCORT_ESCORTING); }

        void SetMaxPlayerDistance(float newMax) { _maxPlayerDistance = newMax; }
        float GetMaxPlayerDistance() const { return _maxPlayerDistance; }

        void SetDespawnAtEnd(bool despawn) { _despawnAtEnd = despawn; }
        void SetDespawnAtFar(bool despawn) { _despawnAtFar = despawn; }
       
        bool IsActiveAttacker() const { return _activeAttacker; } // used in EnterEvadeMode override
        void SetActiveAttacker(bool attack) { _activeAttacker = attack; }

        ObjectGuid GetEventStarterGUID() const { return _playerGUID; }

        virtual bool IsEscortNPC(bool isEscorting) const override;

    // EscortAI variables
    protected:
        ObjectGuid _playerGUID;

        Player* GetPlayerForEscort();
    private:

        bool AssistPlayerInCombatAgainst(Unit* who) override;
        bool IsPlayerOrGroupInRange();
        void FillPointMovementListForCreature();

        void AddEscortState(uint32 uiEscortState) { _escortState |= uiEscortState; }
        void RemoveEscortState(uint32 uiEscortState) { _escortState &= ~uiEscortState; }

        uint32 _pauseTimer;
        uint32 _playerCheckTimer;
        uint32 _escortState;
        float _maxPlayerDistance;

        Quest const* _escortQuest; // generally passed in Start() when regular escort script.

        WaypointPath _path;

        bool _activeAttacker;      // obsolete, determined by faction.
        bool _running;             // all creatures are walking by default (has flag MOVEMENTFLAG_WALK)
        bool _instantRespawn;      // if creature should respawn instantly after escort over (if not, database respawntime are used)
        bool _returnToStart;       // if creature can walk same path (loop) without despawn. Not for regular escort quests.
        bool _despawnAtEnd;
        bool _despawnAtFar;
        bool _manualPath;
        bool _hasImmuneToNPCFlags;
        bool _started;
        bool _useUnitWalkFlag;
        bool _ended;
        bool _resume;
};
#endif

