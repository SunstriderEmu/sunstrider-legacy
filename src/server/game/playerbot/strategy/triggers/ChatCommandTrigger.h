#pragma once

#include "../Trigger.h"

namespace ai
{
    class ChatCommandTrigger : public Trigger {
    public:
        ChatCommandTrigger(PlayerbotAI* ai, std::string command) : Trigger(ai, command), triggered(false), owner(nullptr) {}

        virtual void ExternalEvent(std::string _param, Player* _owner = nullptr)
        {
            this->param = _param;
            this->owner = _owner;
            triggered = true;
        }

        virtual Event Check()
        {
            if (!triggered)
                return Event();

            return Event(getName(), param, owner);
        }

        virtual void Reset()
        {
            triggered = false;
        }

   private:
        std::string param;
        bool triggered;
        Player* owner;
    };
}
