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

class CreatureAINew
{
    public:
        CreatureAINew(Creature* creature) : me(creature), inCombat(false) {}

        virtual ~CreatureAINew() {}

        bool aiInCombat() { return inCombat; }
        void setAICombat(bool on) { inCombat = on; }

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

    protected:
        Creature* me;
        
        bool inCombat;
};
 
#endif
