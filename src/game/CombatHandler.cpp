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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectDefines.h"

void WorldSession::HandleAttackSwingOpcode( WorldPacket & recvData )
{
    PROFILE;

    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;
    recvData >> guid;

    Unit* enemy = ObjectAccessor::GetUnit(*_player, guid);

    if (!enemy) {
        if (!IS_UNIT_GUID(guid))
            TC_LOG_ERROR("FIXME","WORLD: Object %u (TypeID: %u) isn't player, pet or creature", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));
        else
            TC_LOG_ERROR("FIXME","WORLD: Enemy %s %u not found", GetLogNameForGuid(guid), GUID_LOPART(guid));

        // stop attack state at client
        SendMeleeAttackStop(NULL);
        return;
    }

    if (!_player->CanAttack(enemy)) {
        // stop attack state at client
        SendMeleeAttackStop(enemy);
        return;
    }

    _player->Attack(enemy, true);
}

void WorldSession::HandleAttackStopOpcode( WorldPacket & /*recvData*/ )
{
    PROFILE;
    
    GetPlayer()->AttackStop();
}

void WorldSession::HandleSetSheathedOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData,4);

    uint32 sheathed;
    recvData >> sheathed;

    //TC_LOG_DEBUG("FIXME", "WORLD: Recvd CMSG_SETSHEATHED Message guidlow:%u value1:%u", GetPlayer()->GetGUIDLow(), sheathed );

    GetPlayer()->SetSheath(sheathed);
}

void WorldSession::SendMeleeAttackStop(Unit const* enemy)
{
    WorldPacket data( SMSG_ATTACKSTOP, (4+20) );            // we guess size
    data.append(GetPlayer()->GetPackGUID());
    if (enemy)
        data.append(enemy->GetPackGUID());
    else
        data << uint8(0);

    data << uint32(0);                                      // unk, can be 1 also
    SendPacket(&data);
}

