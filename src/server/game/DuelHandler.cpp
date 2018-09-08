
#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Log.h"
#include "Opcodes.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "Player.h"
#include "GameTime.h"

void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guid;
    Player *pl;
    Player *plTarget;

    if(!GetPlayer()->duel)                                  // ignore accept from duel-sender
        return;

    recvPacket >> guid;

    pl       = GetPlayer();
    plTarget = pl->duel->opponent;

    if(pl == pl->duel->initiator || !plTarget || pl == plTarget || pl->duel->startTime != 0 || plTarget->duel->startTime != 0)
        return;

    TC_LOG_DEBUG("network.opcodes", "WORLD: received CMSG_DUEL_ACCEPTED. P1 %u vs P2 %u.", pl->GetGUID().GetCounter(), plTarget->GetGUID().GetCounter());

    //reset cooldown in duel area
    if(pl->IsInDuelArea())
    {
        pl->RemoveArenaSpellCooldowns();
        plTarget->RemoveArenaSpellCooldowns();
    }

    time_t now = GameTime::GetGameTime();
    pl->duel->startTimer = now;
    plTarget->duel->startTimer = now;

    WorldPacket data(SMSG_DUEL_COUNTDOWN, 4);
    data << (uint32)3000;                                   // 3 seconds
    pl->SendDirectMessage(&data);
    plTarget->SendDirectMessage(&data);
}

void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{
    //TC_LOG_DEBUG("network.opcode", "WORLD: received CMSG_DUEL_CANCELLED" );

    // no duel requested
    if(!GetPlayer()->duel)
        return;

    // player surrendered in a duel using /forfeit
    if(GetPlayer()->duel->startTime != 0)
    {
        GetPlayer()->CombatStopWithPets(true);
        if(GetPlayer()->duel->opponent)
            GetPlayer()->duel->opponent->CombatStopWithPets(true);

        GetPlayer()->CastSpell(GetPlayer(), 7267, TRIGGERED_FULL_MASK);    // beg
        GetPlayer()->DuelComplete(DUEL_WON);
        return;
    }

    // player either discarded the duel using the "discard button"
    // or used "/forfeit" before countdown reached 0
    ObjectGuid guid;
    recvPacket >> guid;

    GetPlayer()->DuelComplete(DUEL_INTERRUPTED);
}

