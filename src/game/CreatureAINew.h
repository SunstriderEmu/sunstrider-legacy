/*
 * Copyright (C) 2009 - 2011 Windrunner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef WR_CREATUREAI_H
#define WR_CREATUREAI_H

#define EVENT_MAX_ID    255

typedef struct aiEvent
{
    uint8  id;
    uint32 timer;
    uint32 flags;
    uint32 flagsByDefault;
    bool   active;
    bool   activeByDefault;
    uint32 phaseMask;
    
    aiEvent(uint32 _id, uint32 _minTimer, uint32 _maxTimer, uint32 _flags, bool _activeByDefault, uint32 _phaseMask) :
        id(_id), flags(_flags), flagsByDefault(_flags), activeByDefault(_activeByDefault), active(_activeByDefault), phaseMask(_phaseMask)
    {
        if (_minTimer > _maxTimer) {
            TC_LOG_ERROR("AIEvent","AIEvent::AIEVent: event %u has minTimer > maxTimer, swapping timers.", id);
            std::swap(_minTimer, _maxTimer);
        }
        
        if (_minTimer != _maxTimer)
            timer = _minTimer + rand()%(_maxTimer - _minTimer);
        else
            timer = _minTimer;
    }
    
    void calcTimer(uint32 minTimer)
    {
        calcTimer(minTimer, minTimer);
    }
    
    void calcTimer(uint32 minTimer, uint32 maxTimer)
    {
        if (minTimer > maxTimer)
            std::swap(minTimer, maxTimer);
            
        if (minTimer != maxTimer)
            timer = minTimer + rand()%(maxTimer - minTimer);
        else
            timer = minTimer;
    }
    
    bool isActiveInPhase(uint32 phase)
    {
        if (phaseMask == 0)
            return true;

        return ((phaseMask & (1 << phase)) != 0);
    }
} AIEvent;

typedef enum eventFlag
{
    EVENT_FLAG_NONE             = 0x0,
    EVENT_FLAG_DELAY_IF_CASTING = 0x1
} EventFlag;

class CreatureAINew
{
    public:
        CreatureAINew(Creature* creature) : me(creature), inCombat(false), m_currEvent(EVENT_MAX_ID), m_phase(0), m_lastEvent(EVENT_MAX_ID) {}

        ~CreatureAINew();
        
        /* Events handling */
        void addEvent(uint8 id, uint32 minTimer, uint32 maxTimer, uint32 flags = EVENT_FLAG_NONE, bool activeByDefault = true, uint32 phaseMask = 0);
        void resetEvent(uint8 id, uint32 minTimer, uint32 maxTimer);
        void resetEvent(uint8 id, uint32 timer) { resetEvent(id, timer, timer); }
        void scheduleEvent(uint8 id, uint32 minTimer, uint32 maxTimer);
        void scheduleEvent(uint8 id, uint32 timer) { scheduleEvent(id, timer, timer); }
        void disableEvent(uint8 id);
        void enableEvent(uint8 id);
        bool isActive(uint8 id);
        void setFlag(uint8 id, uint32 flags);
        void delayEvent(uint8 id, uint32 delay);
        void delayAllEvents(uint32 delay);
        bool executeEvent(uint32 const diff, uint8& id);
        void updateEvents(uint32 const diff, uint32 mask = 0);
        // + ensureTimerOnEvents(uint32 minTimer); -> delay events which have timer < minTimer to minTimer
        
        /* Phases handling */
        void setPhase(uint8 phase, bool force = false);
        void incrPhase();
        void decrPhase();
        uint8 getPhase() { return m_phase; }
        uint32 phaseMaskForPhase(uint8 phase) { if (phase > 0) return (1 << phase); else return 0; }

        bool aiInCombat() { return inCombat; }
        void setAICombat(bool on) { inCombat = on; }
        
        /* Target selection */
        bool checkTarget(Unit* target, bool playersOnly, float radius, bool noTank = false);
        Unit* selectUnit(SelectAggroTarget target, uint32 position);
        Unit* selectUnit(SelectAggroTarget target, uint32 position, float radius, bool playersOnly);
        Unit* selectUnit(SelectAggroTarget target, uint32 position, float radius, bool playersOnly, bool noTank);
        void selectUnitList(std::list<Unit*>& targetList, uint32 maxTargets, SelectAggroTarget targetType, float radius, bool playerOnly);
        void getAllPlayersInRange(std::list<Player*>& players, float range);
        
        uint32 doCast(Unit* victim, uint32 spellId, bool triggered = false, bool interrupt = false);
        void doTeleportTo(float x, float y, float z, uint32 time = 0);
        void doResetThreat();
        float doGetThreat(Unit* unit);
        void doModifyThreatPercent(Unit* unit, int32 pct);
        void doModifyThreat(Unit* unit, float threat);
        void setZoneInCombat(bool force = false);
        uint32 talk(uint8 groupid, uint64 targetGUID = 0);
        void deleteFromThreatList(uint64 guid);
        void deleteFromThreatList(Unit* target);
        bool isInMeleeRange();

        //Returns friendly unit with the most amount of hp missing from max hp
        Unit* doSelectLowestHpFriendly(float range, uint32 minHPDiff = 1);

        //Returns a list of friendly CC'd units within range
        std::list<Creature*> doFindFriendlyCC(float range);

        //Returns a list of all friendly units missing a specific buff within range
        std::list<Creature*> doFindFriendlyMissingBuff(float range, uint32 spellid);
        
        /* Script interaction */
        virtual void message(uint32 id, uint64 data) {}
        virtual bool getMessage(uint32 id, uint64 data) { return false;}

        /* At every creature update */
        virtual void update(uint32 const diff);
        /* At every creature update in evade mode*/
        virtual void updateEM(uint32 const diff) {}
        bool updateVictim(bool evade = true);
        bool updateCombat(bool evade = true);
        virtual void doMeleeAttackIfReady();
        /* In Creature::AIM_Initialize() */
        virtual void initialize() { onReset(true); }
        /* When reset (spawn & evade) */
        virtual void onReset(bool onSpawn) {}
        /* When creature respawn */
        virtual void onRespawn() { onReset(true); }
        /* When entering evade mode */
        virtual void evade();
        /* When reaching home position */
        virtual void onReachedHome() {}
        /* When attacking a new target */
        virtual void attackStart(Unit* victim);
        /* When entering combat */
        virtual void onCombatStart(Unit* victim) {}
        /* On death */
        virtual void onDeath(Unit* killer) {}
        /* When killed a unit */
        virtual void onKill(Unit* victim) {}
        /* When another unit is moving in line of sight */
        virtual void onMoveInLoS(Unit* who);
        /* When adding some threat on another unit */
        virtual void onThreatAdd(Unit* who, float& threat) {}
        /* When removing some threat from another unit */
        virtual void onThreatRemove(Unit* who, float& threat) {}
        /* When changing phase */
        virtual void onEnterPhase(uint32 newPhase) {}
        /* When taking damage */
        virtual void onDamageTaken(Unit* attacker, uint32& damage) {}
        /* When taking heal */
        virtual void onHealingTaken(Unit* healer, uint32& heal) {}
        /* When summoning an add */
        virtual void onSummon(Creature* summoned) {}
        /* When summoned add despawns */
        virtual void onSummonDespawn(Creature* summoned) {}
        /* Hooks for spell */
        virtual void onSpellPrepare(SpellEntry const* spell, Unit* target) {}
        virtual void onHitBySpell(Unit* caster, SpellEntry const* spell) {}
        virtual void onSpellFinish(Unit* caster, uint32 spellId, Unit* target, bool ok) {}

        //Called at waypoint reached or PointMovement end
        virtual void onMovementInform(uint32, uint32) {}
        virtual void summonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId) {}
        
        virtual void onRemove() {}

        virtual void sGossipHello(Player* player) {}
        virtual void sGossipSelect(Player* player, uint32 sender, uint32 action) {}
        virtual void sGossipSelectCode(Player* player, uint32 sender, uint32 action, const char* code) {}
        virtual void sQuestAccept(Player* player, Quest const* quest) {}
        virtual void sQuestSelect(Player* player, Quest const* quest) {}
        virtual void sQuestComplete(Player* player, Quest const* quest) {}
        virtual void sQuestReward(Player* player, Quest const* quest, uint32 opt) {}

    protected:
        Creature* me;
        
        bool inCombat;
        
        typedef std::map<uint8, AIEvent*> EventMap;
        EventMap m_events;
        
        uint8 m_currEvent;
        uint8 m_phase;
        uint8 m_lastEvent;
};

class Creature_NoMovementAINew : public CreatureAINew
{
    public:
        Creature_NoMovementAINew(Creature* creature) : CreatureAINew(creature) {}
    
        virtual void attackStart(Unit* /*victim*/);
};
 
#endif
