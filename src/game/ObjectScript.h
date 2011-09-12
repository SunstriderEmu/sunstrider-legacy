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

#ifndef WR_OBJECTSCRIPT_H
#define WR_OBJECTSCRIPT_H

#include "Common.h"
#include "SharedDefines.h"

class Object;

// TODO: Each getScript() will contain a dynamic_cast<CreatureScript*>, dynamic_cast<ObjectScript*>, etc, in each class

class ObjectScript
{
    public:
        ObjectScript(Object* obj) : me(obj) {}
        
        virtual ScriptType getScriptType() { return SCRIPT_TYPE_OBJECT; }

        /* Register self in script repository */
        void registerSelf();
        /* Reset script */
        void reset();
        /* On entering evade mode or manual script reset (at reset() call) */
        void onReset();
    
    protected:
        ObjectScript() {}
    
        Object* me;
};

#endif
