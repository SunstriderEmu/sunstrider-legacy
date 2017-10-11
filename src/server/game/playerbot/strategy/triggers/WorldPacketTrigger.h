#pragma once

#include "../Trigger.h"

namespace ai
{
    class WorldPacketTrigger : public Trigger {
    public:
        WorldPacketTrigger(PlayerbotAI* ai, std::string command) : Trigger(ai, command), triggered(false) {}

        virtual void ExternalEvent(WorldPacket& _packet, Player* _owner = NULL)
        {
            this->packet = _packet;
            this->owner = _owner;
            triggered = true;
        }

        virtual Event Check()
        {
            if (!triggered)
                return Event();

            return Event(getName(), packet, owner);
        }

        virtual void Reset()
        {
            triggered = false;
        }

    private:
        WorldPacket packet;
        bool triggered;
        Player* owner;
    };
}
