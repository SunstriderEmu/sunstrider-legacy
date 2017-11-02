#include "test_utils.h"

#include "Player.h"
#include "Creature.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"

/*
void Testing::PrepareCasterAndTarget(Player*& caster, Creature*& target)
{
    //corner in testing map
    static const WorldLocation testLoc = WorldLocation(13, -128.0f, -128.0f, -144.7f, 0.4f);
    uint32 bot = sRandomPlayerbotMgr.AddRandomBot(false);
    REQUIRE(bot != 0);

    Player* playerBot = sRandomPlayerbotMgr.AddPlayerBot(bot, 0);
    REQUIRE(playerBot != nullptr);

    PlayerbotAI* ai = playerBot->GetPlayerbotAI();
    REQUIRE(ai != nullptr);

    //handle bot position
    bool teleportOK = playerBot->TeleportTo(testLoc);
    REQUIRE(teleportOK);
    ai->HandleTeleportAck(); //immediately handle teleport packet

                             //spawn target creature
    Position inFront(playerBot);
    playerBot->MovePosition(inFront, 10.0f, playerBot->GetOrientation());
    Creature* targetCreature = playerBot->SummonCreature(10, inFront); // Creature ID 10 = Training dummy
    REQUIRE(targetCreature);

    caster = playerBot;
    target = targetCreature;
}

void Testing::PrepareCasterAndTarget_cleanup(Player*& caster, Creature*& target)
{
    caster->GetSession()->KickPlayer(); //not working for bots ?
    target->DespawnOrUnsummon();
}

*/