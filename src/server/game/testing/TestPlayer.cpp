#include "TestPlayer.h"

void TestPlayer::SetMapAtCreation(PlayerInfo const* info)
{
    //harcoded position lost in the middle of nowhere... Players may cast spells at creation so they need to be on a map if I don't want to change a lot of things. Lets just spawn them in an empty zone to avoid uneccessary loading
    Relocate(0.0f, 0.0f, 0.0f);
    SetMap(sMapMgr->CreateMap(169, this)); //emerald dream
    UpdatePositionData();
}

void TestPlayer::Update(uint32 time)
{
    //since we don't handle session for test players, handle teleports here
    if (IsBeingTeleported())
        GetPlayerbotAI()->HandleTeleportAck();

    Player::Update(time);
}