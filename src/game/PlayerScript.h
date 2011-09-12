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

#ifndef WR_PLAYERSCRIPT_H
#define WR_PLAYERSCRIPT_H

#include "UnitScript.h"

class Player;

class PlayerScript : public UnitScript
{
    public:
        PlayerScript(Player* player) : me(player) {}
        
        virtual ScriptType getScriptType() { return SCRIPT_TYPE_PLAYER; }
    
    protected:
        PlayerScript() {}

        Player* me;
};

#endif
