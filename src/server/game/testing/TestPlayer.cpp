#include "TestPlayer.h"

void TestPlayer::SetMapAtCreation(PlayerInfo const* info)
{
    //do nothing here, we set map earlier in TestCase
}

void TestPlayer::Update(uint32 time)
{
    //since we don't handle session for test players, handle teleports here
    if (IsBeingTeleported())
        GetPlayerbotAI()->HandleTeleportAck();

    Player::Update(time);
}