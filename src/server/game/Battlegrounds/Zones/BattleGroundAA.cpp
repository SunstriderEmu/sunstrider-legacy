
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundAA.h"

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
    auto  sc = new BattlegroundAAScore;

    PlayerScores[plr->GetGUID()] = sc;
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

