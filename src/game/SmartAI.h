//Adapted and updated from TrinityCore : 2014-01-24

#ifndef TRINITY_SMARTAI_H
#define TRINITY_SMARTAI_H

#include "CreatureAI.h"
#include "Unit.h"

#include "SmartScript.h"
#include "SmartScriptMgr.h"
#include "GameObjectAI.h"

enum SmartEscortState
{
    SMART_ESCORT_NONE       = 0x000,                        //nothing in progress
    SMART_ESCORT_ESCORTING  = 0x001,                        //escort is in progress
    SMART_ESCORT_RETURNING  = 0x002,                        //escort is returning after being in combat
    SMART_ESCORT_PAUSED     = 0x004                         //will not proceed with waypoints before state is removed
};

enum SmartEscortVars
{
    SMART_ESCORT_MAX_PLAYER_DIST        = 50,
    SMART_MAX_AID_DIST    = SMART_ESCORT_MAX_PLAYER_DIST / 2
};

class SmartAI : public CreatureAI
{
    public:
        ~SmartAI(){ }
        explicit SmartAI(Creature* c);

        // Start moving to the desired MovePoint
        void StartPath(bool run = false, uint32 path = 0, bool repeat = false, Unit* invoker = NULL);
        bool LoadPath(uint32 entry);
        void PausePath(uint32 delay, bool forced = false);
        void StopPath(uint32 DespawnTime = 0, uint32 quest = 0, bool fail = false);
        void EndPath(bool fail = false, bool died = false);
        void ResumePath();
        WayPoint* GetNextWayPoint();
        bool HasEscortState(uint32 uiEscortState) const { return (mEscortState & uiEscortState) != 0; }
        void AddEscortState(uint32 uiEscortState) { mEscortState |= uiEscortState; }
        void RemoveEscortState(uint32 uiEscortState) { mEscortState &= ~uiEscortState; }
        void SetAutoAttack(bool on) { mCanAutoAttack = on; }
        void SetCombatMove(bool on);
        bool CanCombatMove() { return mCanCombatMove; }
        void SetFollow(Unit* target, float dist = 0.0f, float angle = 0.0f, uint32 credit = 0, uint32 end = 0, uint32 creditType = 0);
        void StopFollow();

        void SetScript9(SmartScriptHolder& e, uint32 entry, Unit* invoker);
        SmartScript* GetScript() { return &mScript; }
        bool IsEscortInvokerInRange();

        // Called when creature is spawned or respawned
        void JustRespawned() override;

        // Called at reaching home after evade, InitializeAI(), EnterEvadeMode() for resetting variables
        void JustReachedHome() override;

        // Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
        void EnterCombat(Unit* enemy) override;

        // Called for reaction at stopping attack at no attackers or targets
        void EnterEvadeMode() override;

        // Called when the creature is killed
        void JustDied(Unit* killer) override;

        // Called when the creature kills a unit
        void KilledUnit(Unit* victim) override;

        // Called when the attacked unit dies (not necesserily killed by us)
        void AttackedUnitDied(Unit* /* attacked */) override;

        // Called when the creature summon successfully other creature
        void JustSummoned(Creature* creature) override;

        // Tell creature to attack and follow the victim
        void AttackStart(Unit* who) override;

        // Called if IsVisible(Unit* who) is true at each *who move, reaction at visibility zone enter
        void MoveInLineOfSight(Unit* who) override;

        // Called when hit by a spell
        void SpellHit(Unit* unit, const SpellInfo*) override;

        // Called when spell hits a target
        void SpellHitTarget(Unit* target, const SpellInfo*) override;

        // Called at any Damage from any attacker (before damage apply)
        void DamageTaken(Unit* doneBy, uint32& damage) override;

        // Called when the creature receives heal
        void HealReceived(Unit* doneBy, uint32& addhealth) override;

        // Called at World update tick
        void UpdateAI(const uint32 diff) override;

        // Called at text emote receive from player
        void ReceiveEmote(Player* player, uint32 textEmote) override;

        // Called at waypoint reached or point movement finished
        void MovementInform(uint32 MovementType, uint32 Data) override;

        // Called when creature is summoned by another unit
        void IsSummonedBy(Unit* summoner) override;

        // Called at any Damage to any victim (before damage apply)
        void DamageDealt(Unit* doneTo, uint32& damage, DamageEffectType /*damagetype*/) override;

        // Called when a summoned creature dissapears (UnSommoned)
        void SummonedCreatureDespawn(Creature* unit) override;

        // called when the corpse of this creature gets removed
        void CorpseRemoved(uint32& respawnDelay) override;

        // Called at World update tick if creature is charmed
        void UpdateAIWhileCharmed(const uint32 diff); //Not handled

        // Called when a Player/Creature enters the creature (vehicle)
        void PassengerBoarded(Unit* who, int8 seatId, bool apply) /* override */; //LK

        // Called when gets initialized, when creature is added to world
        void InitializeAI() override;

        // Called when creature gets charmed by another unit
        void OnCharmed(Unit* charmer, bool apply) override;

        // Used in scripts to share variables
        void DoAction(const int32 param = 0) override;

        // Used in scripts to share variables
        uint32 GetData(uint32 id = 0) const override;

        // Used in scripts to share variables
        void SetData(uint32 id, uint32 value) override;

        // Used in scripts to share variables
        void SetGUID(uint64 guid, int32 id = 0) override;

        // Used in scripts to share variables
        uint64 GetGUID(int32 id = 0) const override;

        //core related
        static int Permissible(const Creature*);  //FIXME ?

        // Called at movepoint reached
        void MovepointReached(uint32 id);

        // Makes the creature run/walk
        void SetRun(bool run = true);

        void SetFly(bool fly = true);

        void SetSwim(bool swim = true);

        void SetInvincibilityHpLevel(uint32 level) { mInvincibilityHpLevel = level; }

        void sOnGossipHello(Player* player) override;
        void sGossipSelect(Player* player, uint32 sender, uint32 action) override;
        void sOnGossipSelectCode(Player* player, uint32 sender, uint32 action, const char* code) override;
        void sQuestAccept(Player* player, Quest const* quest) override;
        //void sQuestSelect(Player* player, Quest const* quest);
        //void sQuestComplete(Player* player, Quest const* quest);
        void sQuestReward(Player* player, Quest const* quest, uint32 opt) override;
        bool sOnDummyEffect(Unit* caster, uint32 spellId, uint32 effIndex) override;
        //FIXME void sOnGameEvent(bool start, uint16 eventId) override;

        uint32 mEscortQuestID;

        void SetDespawnTime(uint32 t)
        {
            mDespawnTime = t;
            mDespawnState = t ? 1 : 0;
        }
        void StartDespawn() { mDespawnState = 2; }

        void RemoveAuras();

        void OnSpellClick(Unit* clicker, bool& result) override;

        void FriendlyKilled(Creature const* c, float range) override;

        void SetPreventMoveHome(bool prevent) { m_preventMoveHome = prevent; }

    private:
        uint32 mFollowCreditType;
        uint32 mFollowArrivedTimer;
        uint32 mFollowCredit;
        uint32 mFollowArrivedEntry;
        uint64 mFollowGuid;
        float mFollowDist;
        float mFollowAngle;

        void ReturnToLastOOCPos();
        void UpdatePath(const uint32 diff);
        SmartScript mScript;
        WPPath* mWayPoints;
        uint32 mEscortState;
        uint32 mCurrentWPID;
        uint32 mLastWPIDReached;
        bool mOOCReached; //just reached last SMART_ESCORT_LAST_OOC_POINT
        bool mWPReached; //just reached any point
        uint32 mWPPauseTimer;
        WayPoint* mLastWP;
        Position mLastOOCPos;//set on enter combat
        uint32 GetWPCount() const { return mWayPoints ? mWayPoints->size() : 0; }
        bool mCanRepeatPath;
        bool mRun;
        bool mCanAutoAttack;
        bool mCanCombatMove;
        bool mForcedPaused;
        uint32 mInvincibilityHpLevel;

        uint32 mDespawnTime;
        /**
         // 0 - no despawn scheduled
            1 -- proceed despawn at path end
            2 -- set invisible when mDespawnTime reached 0, then set to 3
            3 -- despawn when mDespawnTime reached 0
        */
        uint32 mDespawnState;
        void UpdateDespawn(const uint32 diff);
        uint32 mEscortInvokerCheckTimer;
        bool mJustReset;
        bool m_preventMoveHome;
};

class SmartGameObjectAI : public GameObjectAI
{
    public:
        SmartGameObjectAI(GameObject* g) : GameObjectAI(g) { }
        ~SmartGameObjectAI() { }

        void UpdateAI(const uint32 diff);
        void InitializeAI();
        void Reset();
        SmartScript* GetScript() { return &mScript; }
        static int Permissible(const GameObject* g);

        bool OnGossipHello(Player* player) override;
        bool OnGossipSelect(Player* player, uint32 sender, uint32 action) override;
        bool OnGossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) override;
        bool OnQuestAccept(Player* player, Quest const* quest) override;
        bool QuestReward(Player* player, Quest const* quest, uint32 opt) override;
        void Destroyed(Player* player, uint32 eventId) override;
        void SetData(uint32 id, uint32 value) override;
        void SetScript9(SmartScriptHolder& e, uint32 entry, Unit* invoker); //FIXME
        void OnGameEvent(bool start, uint16 eventId); //FIXME
        void OnStateChanged(GOState state, Unit* unit) override;
        void OnLootStateChanged(LootState state, Unit* unit) override;
        void EventInform(uint32 eventId); //FIXME

    private:
        SmartScript mScript;
};
#endif