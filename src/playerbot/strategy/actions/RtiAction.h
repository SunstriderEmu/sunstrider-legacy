#pragma once

#include "../Action.h"

namespace ai
{
    class RtiAction : public Action
    {
    public:
        RtiAction(PlayerbotAI* ai) : Action(ai, "rti")
        {}

        virtual bool Execute(Event event)
        {
            std::string text = event.getParam();
            if (text.empty() || text == "?")
            {
                std::ostringstream out; out << "RTI: ";
                AppendRti(out);
                ai->TellMaster(out);
                return true;
            }

            context->GetValue<std::string>("rti")->Set(text);
            std::ostringstream out; out << "RTI set to: ";
            AppendRti(out);
            ai->TellMaster(out);
            return true;
        }

    private:
        void AppendRti(std::ostringstream & out)
        {
            out << AI_VALUE(string, "rti");

            Unit* target = AI_VALUE(Unit*, "rti target");
            if(target)
                out << " (" << target->GetName() << ")";

        }

    };

}
