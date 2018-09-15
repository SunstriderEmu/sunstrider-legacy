
#include "../../playerbot.h"
#include "CheatGodAction.h"


using namespace ai;

bool CheatGodAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    WorldSession* session = master->GetSession();
    if (!session)
        return false;

    if (session->GetSecurity() <= SEC_PLAYER)
    {
        ai->TellMaster("Not allowed");
        return true;
    }

	bool on = bot->GetCommandStatus(CHEAT_GOD);
    if (on)
    {
        ai->TellMaster("Cheat god off");
        bot->SetCommandStatusOff(CHEAT_GOD);
    }
    else
    {
        ai->TellMaster("Cheat god off");
        bot->SetCommandStatusOn(CHEAT_GOD);
    }
	
    return true;
}
