//
#include "../../playerbot.h"
#include "ChangeStrategyAction.h"
#include "../../PlayerbotAIConfig.h"

using namespace ai;

bool ChangeCombatStrategyAction::Execute(Event event)
{
    std::string text = event.getParam();
    ai->ChangeStrategy(text.empty() ? getName() : text, BOT_STATE_COMBAT);
    return true;
}

bool ChangeNonCombatStrategyAction::Execute(Event event)
{
    std::string text = event.getParam();

    uint32 account = sObjectMgr->GetPlayerAccountIdByGUID(bot->GetGUID());
    if (sPlayerbotAIConfig.IsInRandomAccountList(account) && ai->GetMaster() && ai->GetMaster()->GetSession()->GetSecurity() < SEC_GAMEMASTER1)
    {
        if (text.find("loot") != std::string::npos || text.find("gather") != std::string::npos)
        {
            ai->TellMaster("You can change any strategy except loot and gather");
            return false;
        }
    }

    ai->ChangeStrategy(text, BOT_STATE_NON_COMBAT);
    return true;
}

bool ChangeDeadStrategyAction::Execute(Event event)
{
    std::string text = event.getParam();
    ai->ChangeStrategy(text, BOT_STATE_DEAD);
    return true;
}
