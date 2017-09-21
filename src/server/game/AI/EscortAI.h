/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

#include "ScriptedCreature.h"

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

struct TC_GAME_API npc_escortAI : public ScriptedAI
{
    public:

        // Pure Virtual Functions
        virtual void WaypointReached(uint32) = 0;

        void EnterCombat(Unit*) override = 0;

        void Reset() override = 0;

        // CreatureAI functions
		npc_escortAI(Creature *c);

        void AttackStart(Unit * who) override;

        void MoveInLineOfSight(Unit*) override;

        void JustRespawned() override;

        void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override;

        void UpdateAI(const uint32) override;

        void MovementInform(uint32, uint32) override;

        void OnPossess(Unit* charmer, bool apply) override;

        // EscortAI functions
        void AddWaypoint(uint32 id, float x, float y, float z, uint32 WaitTimeMs = 0);
        
        void GetWaypointListFromDB(uint32 entry);

        void Start(bool bAttack, bool bDefend, bool bRun = false, uint64 pGUID = 0, uint32 entry = 0);
        
        void SetRun(bool bRun = true);
        void SetEscortPaused(bool uPaused);
        
        bool HasEscortState(uint32 uiEscortState) { return (m_uiEscortState & uiEscortState); }
        bool IsEscorted() const override { return (m_uiEscortState & STATE_ESCORT_ESCORTING); }

        void SetMaxPlayerDistance(float newMax) { MaxPlayerDistance = newMax; }
        float GetMaxPlayerDistance() const { return MaxPlayerDistance; }

        void SetCanMelee(bool usemelee) { CanMelee = usemelee; }
        void SetCanDefend(bool canDef) { Defend = canDef; }
        void SetDespawnAtEnd(bool despawn) { DespawnAtEnd = despawn; }
        void SetDespawnAtFar(bool despawn) { DespawnAtFar = despawn; }
        bool GetAttack() const { return Attack; }//used in EnterEvadeMode override
        
        Player* GetPlayerForEscort();
        
        bool AssistPlayerInCombat(Unit* who);
        
        void SetLastPos(float x, float y, float z) { LastPos.x = x; LastPos.y = y; LastPos.z = z; }

        void AddEscortState(uint32 uiEscortState) { m_uiEscortState |= uiEscortState; }
        void RemoveEscortState(uint32 uiEscortState) { m_uiEscortState &= ~uiEscortState; }

    // EscortAI variables
    protected:
        uint64 PlayerGUID;
        bool IsOnHold;

    private:
    
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
        bool ReconnectWP;
        bool Run;
        bool CanMelee;
        bool DespawnAtEnd;
        bool DespawnAtFar;
};
#endif

