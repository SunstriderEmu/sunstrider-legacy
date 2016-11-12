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

#include "AOEAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "World.h"

#include <list>

int AOEAI::Permissible(const Creature *creature)
{
    return PERMIT_BASE_NO;
}

AOEAI::AOEAI(Creature *c) : CreatureAI(c)
{
    DEBUG_ASSERT(me->m_spells[0]);
    me->SetVisibility(VISIBILITY_ON);//visible to see all spell anims
    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);//can't be targeted
    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);//can't be damaged
    me->SetDisplayId(11686);//invisible model,around a size of a player
}

void AOEAI::AttackStart(Unit *who)
{
    
}

void AOEAI::UpdateAI(const uint32 diff)
{
    if(!me->HasAuraEffect(me->m_spells[0]))
        me->CastSpell(me, me->m_spells[0],false);
}
