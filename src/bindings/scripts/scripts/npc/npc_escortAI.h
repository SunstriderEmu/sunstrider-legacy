/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

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

struct npc_escortAI : public ScriptedAI
{
    public:

        // Pure Virtual Functions
        virtual void WaypointReached(uint32) = 0;

        virtual void Aggro(Unit*) = 0;

        virtual void Reset() = 0;

        // CreatureAI functions
        npc_escortAI(Creature *c) : ScriptedAI(c), IsBeingEscorted(false), PlayerTimer(1000), MaxPlayerDistance(DEFAULT_MAX_PLAYER_DISTANCE), CanMelee(true), DespawnAtEnd(true), DespawnAtFar(true) {m_creature->GetPosition(LastPos.x, LastPos.y, LastPos.z);}

        bool IsVisible(Unit*) const;

        void AttackStart(Unit*);

        void MoveInLineOfSight(Unit*);

        void JustRespawned();

        void EnterEvadeMode();

        void UpdateAI(const uint32);

        void MovementInform(uint32, uint32);

        void OnPossess(bool apply);

        // EscortAI functions
        void AddWaypoint(uint32 id, float x, float y, float z, uint32 WaitTimeMs = 0);
        
        void GetWaypointListFromDB(uint32 entry);

        void Start(bool bAttack, bool bDefend, bool bRun = false, uint64 pGUID = 0, uint32 entry = 0);
        
        void SetRun(bool bRun = true);
        void SetEscortPaused(bool uPaused);
        
        bool HasEscortState(uint32 uiEscortState) { return (m_uiEscortState & uiEscortState); }
        virtual bool IsEscorted() { return (m_uiEscortState & STATE_ESCORT_ESCORTING); }

        void SetMaxPlayerDistance(float newMax) { MaxPlayerDistance = newMax; }
        float GetMaxPlayerDistance() { return MaxPlayerDistance; }

        void SetCanMelee(bool usemelee) { CanMelee = usemelee; }
        void SetDespawnAtEnd(bool despawn) { DespawnAtEnd = despawn; }
        void SetDespawnAtFar(bool despawn) { DespawnAtFar = despawn; }
        bool GetAttack() { return Attack; }//used in EnterEvadeMode override
        
        Player* GetPlayerForEscort() { return Unit::GetPlayer(PlayerGUID); }

    // EscortAI variables
    protected:
        uint64 PlayerGUID;
        bool IsBeingEscorted;
        bool IsOnHold;

    private:
        void AddEscortState(uint32 uiEscortState) { m_uiEscortState |= uiEscortState; }
        void RemoveEscortState(uint32 uiEscortState) { m_uiEscortState &= ~uiEscortState; }
    
        uint32 WaitTimer;
        uint32 PlayerTimer;
        uint32 m_uiEscortState;
        float MaxPlayerDistance;

        struct
        {
            float x;
            float y;
            float z;
        }LastPos;

        std::list<Escort_Waypoint> WaypointList;
        std::list<Escort_Waypoint>::iterator CurrentWP;

        bool Attack;
        bool Defend;
        bool Returning;
        bool ReconnectWP;
        bool Run;
        bool CanMelee;
        bool DespawnAtEnd;
        bool DespawnAtFar;
};
#endif

