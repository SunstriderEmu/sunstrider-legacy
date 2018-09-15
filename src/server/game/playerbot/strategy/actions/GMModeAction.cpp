
#include "../../playerbot.h"
#include "GMModeAction.h"


using namespace ai;

bool GMModeAction::Execute(Event event)
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

	bool on = bot->IsGameMaster();
    if (on)
        ai->TellMaster("GM mode off");
    else
        ai->TellMaster("GM mode on");

    bot->SetGameMaster(!on);
	
    return true;
}
