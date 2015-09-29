#pragma once

#include "Trigger.h"

namespace ai
{
    class ExternalEventHelper {
    public:
        ExternalEventHelper(AiObjectContext* aiObjectContext) : aiObjectContext(aiObjectContext) {}

        bool ParseChatCommand(string command, Player* owner = nullptr);

        void HandlePacket(map<uint16, string> &handlers, const WorldPacket &packet, Player* owner = nullptr);

        bool HandleCommand(string name, string param, Player* owner = nullptr);

    private:
        AiObjectContext* aiObjectContext;
    };
}
