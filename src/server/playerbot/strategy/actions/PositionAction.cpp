
#include "../../playerbot.h"
#include "PositionAction.h"
#include "../values/PositionValue.h"

using namespace ai;

bool PositionAction::Execute(Event event)
{
	string qualifier = event.getParam();
	if (qualifier.empty())
		return false;

    Player* master = GetMaster();
    if (!master)
        return false;

	ai::Position& pos = context->GetValue<ai::Position&>("position", qualifier)->Get();
    pos.Set( master->GetPositionX(), master->GetPositionY(), master->GetPositionZ());

    std::ostringstream out; out << "Position " << qualifier << " is set";
    ai->TellMaster(out);
    return true;
}

bool MoveToPositionAction::Execute(Event event)
{
	ai::Position& pos = context->GetValue<ai::Position&>("position", qualifier)->Get();
    if (!pos.isSet())
    {
        std::ostringstream out; out << "Position " << qualifier << " is not set";
        ai->TellMaster(out);
        return false;
    }

    return MoveTo(bot->GetMapId(), pos.x, pos.y, pos.z);
}

