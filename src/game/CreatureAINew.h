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
        CreatureAINew(Creature* creature) : me(creature) {}

        virtual ~CreatureAINew() {}

        /* At every creature update */
        virtual void update(uint32 const /*diff*/) {}
        /* When reset (spawn & evade) */
        virtual void onReset(bool /*onSpawn*/) {}
        /* When attacking by a target */
        virtual void onAttackStart(Unit* /*victim*/) {}
        /* When entering combat */
        virtual void onCombatStart(Unit* /*victim*/) {}
        /* On death */
        virtual void onDeath(Unit* /*killer*/) {}
        /* When killed a unit */
        virtual void onKill(Unit* /*victim*/) {}

    protected:
        Creature* me;
};
 
#endif
