
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
    Player* player = GetPlayer();
    if (!player->duel || player == player->duel->Initiator || player->duel->State != DUEL_STATE_CHALLENGED)
        return;

    ObjectGuid guid;
    recvPacket >> guid;

    Player* target = player->duel->Opponent;
    if (target->GetGuidValue(PLAYER_DUEL_ARBITER) != guid)
        return;

    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_DUEL_ACCEPTED");
    TC_LOG_DEBUG("network", "Player 1 is: %u (%s)", player->GetGUID().GetCounter(), player->GetName().c_str());
    TC_LOG_DEBUG("network", "Player 2 is: %u (%s)", target->GetGUID().GetCounter(), target->GetName().c_str());

    time_t now = player->GetMap()->GetGameTime();
    player->duel->StartTime = now + 3;
    target->duel->StartTime = now + 3;

    player->duel->State = DUEL_STATE_COUNTDOWN;
    target->duel->State = DUEL_STATE_COUNTDOWN;

    player->SendDuelCountdown(3000);
    target->SendDuelCountdown(3000);
}

void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network", "WORLD: Received CMSG_DUEL_CANCELLED");
    Player* player = GetPlayer();

    ObjectGuid guid;
    recvPacket >> guid;

    // no duel requested
    if (!player->duel || player->duel->State == DUEL_STATE_COMPLETED)
        return;

    // player surrendered in a duel using /forfeit
    if (GetPlayer()->duel->State == DUEL_STATE_IN_PROGRESS)
    {
        GetPlayer()->CombatStopWithPets(true);
        GetPlayer()->duel->Opponent->CombatStopWithPets(true);

        GetPlayer()->CastSpell(GetPlayer(), 7267, true);    // beg
        GetPlayer()->DuelComplete(DUEL_WON);
        return;
    }

    GetPlayer()->DuelComplete(DUEL_INTERRUPTED);
}

