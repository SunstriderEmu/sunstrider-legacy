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

#ifndef WR_WORLDOBJECTSCRIPT_H
#define WR_WORLDOBJECTSCRIPT_H

#include "ObjectScript.h"

class WorldObject;

class WorldObjectScript : public ObjectScript
{
    public:
        WorldObjectScript(WorldObject* worldobj) : me(worldobj) {}
    
        /* At world tick */
        virtual void update(uint32 const diff) = 0;

    protected:
        WorldObjectScript() {}

        WorldObject* me;
};

#endif
