//
#include "../../playerbot.h"
#include "ChangeTalentsAction.h"

using namespace ai;

bool ChangeTalentsAction::Execute(Event event)
{
#ifdef LICH_KING
    std::string text = event.getParam();
    if (text == "1" || text == "primary")
    {
        bot->ActivateSpec(0);
        ai->TellMaster("Primary talents activated");
    }
    else if (text == "2" || text == "secondary")
    {
        bot->ActivateSpec(1);
        ai->TellMaster("Secondary talents activated");
    }
    else
    {
        std::ostringstream out;
        switch (bot->GetActiveSpec())
        {
        case 0: out << "Primary";
            break;
        default: out << "Secondary";
            break;
        }
        out << " talents are active";
        ai->TellMaster(out);
    }
#endif
    return true;
}
