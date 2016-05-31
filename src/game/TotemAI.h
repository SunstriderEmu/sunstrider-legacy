/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
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

#ifndef TRINITY_TOTEMAI_H
#define TRINITY_TOTEMAI_H

#include "CreatureAI.h"
#include "Timer.h"

class Creature;
class Totem;

class TotemAI : public CreatureAI
{
    public:

        TotemAI(Creature *c);
        ~TotemAI();

        void MoveInLineOfSight(Unit *) override;
        void AttackStart(Unit *) override;
        void EnterEvadeMode(EvadeReason /* why */) override;

        void UpdateAI(const uint32) override;
        static int Permissible(const Creature *);

    private:
        Totem &i_totem;
        uint64 i_victimGuid;
};

//simply kill self
class KillMagnetEvent : public BasicEvent
{
public:
    KillMagnetEvent(Unit& self) : _self(self) { }
    bool Execute(uint64 e_time, uint32 p_time)
    {
        _self.SetDeathState(JUST_DIED);
        return true;
    }

protected:
    Unit& _self;
};

#endif

