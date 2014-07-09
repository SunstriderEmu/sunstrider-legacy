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

#include "Player.h"
#include "Battleground.h"
#include "BattlegroundAA.h"

BattlegroundAA::BattlegroundAA()
{

}

BattlegroundAA::~BattlegroundAA()
{

}

void BattlegroundAA::Update(time_t diff)
{
    Battleground::Update(diff);
}

void BattlegroundAA::AddPlayer(Player *plr)
{
    Battleground::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattlegroundAAScore* sc = new BattlegroundAAScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattlegroundAA::RemovePlayer(Player * /*plr*/, uint64 /*guid*/)
{
}

void BattlegroundAA::HandleKillPlayer(Player* player, Player* killer)
{
    Battleground::HandleKillPlayer(player, killer);
}

void BattlegroundAA::HandleAreaTrigger(Player * /*Source*/, uint32 /*Trigger*/)
{
}

bool BattlegroundAA::SetupBattleground()
{
    return true;
}

