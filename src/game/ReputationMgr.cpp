/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ReputationMgr.h"
/*
#include "DatabaseEnv.h"
#include "DBCStores.h"
#include "Player.h"
#include "WorldPacket.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
*/
const int32 ReputationMgr::PointsInRank[MAX_REPUTATION_RANK] = {36000, 3000, 3000, 3000, 6000, 12000, 21000, 1000};

const int32 ReputationMgr::Reputation_Cap = 42999;
const int32 ReputationMgr::Reputation_Bottom = -42000;
