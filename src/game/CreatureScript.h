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

#include "SharedDefines.h"

#include <string>

class Creature;
class CreatureAINew;

class CreatureScript
{
    public:
        CreatureScript(std::string name) : m_name(name) {}
        
        std::string getName() { return m_name; }
        
        virtual ScriptType getScriptType() { return SCRIPT_TYPE_CREATURE; }
        
        virtual CreatureAINew* getAI(Creature* creature) { return NULL; }
    
    protected:
        std::string m_name;
};

#endif
