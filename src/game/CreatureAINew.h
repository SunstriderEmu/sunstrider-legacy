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

enum SelectedTarget
{
    TARGET_RANDOM = 0,                               //Just selects a random target
    TARGET_TOPAGGRO,                                 //Selects targes from top aggro to bottom
    TARGET_BOTTOMAGGRO,                              //Selects targets from bottom aggro to top
    TARGET_NEAREST,
    TARGET_FARTHEST,
};

#define EVENT_MAX_ID    255

class CreatureAINew
{
    public:
        CreatureAINew(Creature* creature) : me(creature), inCombat(false), m_currEvent(EVENT_MAX_ID) {}

        virtual ~CreatureAINew() {}
        
        /* Events handling */
        void schedule(uint8 id, uint32 timer) { schedule(id, timer, timer); }
        void schedule(uint8 id, uint32 minTimer, uint32 maxTimer);
        void cancel(uint8 id) { m_events.erase(id); }
        void setExecuted(uint8 id) { m_events.erase(id); }
        void delay(uint8 id, uint32 delay);
        void delayAll(uint32 delay);
        bool executeEvent(uint32 const /*diff*/, uint8& /*id*/);
        void updateEvents(uint32 const /*diff*/);
        // + ensureTimerOnEvents(uint32 minTimer); -> delay events which have timer < minTimer to minTimer

        bool aiInCombat() { return inCombat; }
        void setAICombat(bool on) { inCombat = on; }
        
        /* Target selection */
        Unit* selectUnit(SelectedTarget /*target*/, uint32 /*position*/);
        
        void doCast(Unit* /*victim*/, uint32 /*spellId*/, bool triggered = false, bool interrupt = false);
        void setZoneInCombat();
        uint32 talk(uint8 groupid, uint64 targetGUID = 0);

        /* At every creature update */
        virtual void update(uint32 const /*diff*/);
        bool updateVictim(bool evade = true);
        void doMeleeAttackIfReady();
        /* In Creature::AIM_Initialize() */
        virtual void initialize() { onReset(true); }
        /* When reset (spawn & evade) */
        virtual void onReset(bool /*onSpawn*/) {}
        /* When creature respawn */
        virtual void onRespawn() { onReset(true); }
        /* When entering evade mode */
        virtual void evade();
        /* When reaching home position */
        virtual void onReachedHome() {}
        /* When attacking a new target */
        void attackStart(Unit* /*victim*/);
        /* When entering combat */
        virtual void onCombatStart(Unit* /*victim*/) {}
        /* On death */
        virtual void onDeath(Unit* /*killer*/) {}
        /* When killed a unit */
        virtual void onKill(Unit* /*victim*/) {}
        /* When another unit is moving in line of sight */
        virtual void onMoveInLoS(Unit* /*who*/);
        /* When adding some threat on another unit */
        virtual void onThreatAdd(Unit* /*who*/, float& /*threat*/) {}
        /* When removing some threat from another unit */
        virtual void onThreatRemove(Unit* /*who*/, float& /*threat*/) {}

    protected:
        Creature* me;
        
        bool inCombat;
        
        typedef std::map<uint8, uint32> EventMap;
        EventMap m_events;
        
        uint8 m_currEvent;
};
 
#endif
