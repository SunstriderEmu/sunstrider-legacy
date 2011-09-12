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

#ifndef WR_CREATURESCRIPT_H
#define WR_CREATURESCRIPT_H

#include "UnitScript.h"

class Creature;

class CreatureScript : public UnitScript
{
    public:
        CreatureScript(Creature* creature) : me(creature) {}
        
        virtual ScriptType getScriptType() { return SCRIPT_TYPE_CREATURE; }
        
        virtual void initializeAI() { onReset(true); }
        bool updateVictim(bool evade = true);
        void attackStart(Unit* target);
        void doMeleeAttackIfReady();
        
        /* HOOKS */
        /* On reset (spawn & evade) */
        void onReset(bool onSpawn);
        /* When entering combat */
        void onCombatStart(Unit* who);
        /* On death */
        void onDeath(Unit* killer);
        /* When killed another unit */
        void onKill(Unit* killed);
        /* When another unit moves in LoS */
        void onMoveInLoS(Unit* who);
        /* When evading */
        void evade();
    
    protected:
        CreatureScript() {}

        Creature* me;
};

#endif
