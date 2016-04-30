#pragma once

#include "Trigger.h"

namespace ai
{
    class ExternalEventHelper {
    public:
        ExternalEventHelper(AiObjectContext* aiObjectContext) : aiObjectContext(aiObjectContext) {}

        bool ParseChatCommand(std::string command, Player* owner = nullptr);

        void HandlePacket(std::map<uint16, std::string> &handlers, const WorldPacket &packet, Player* owner = nullptr);

        bool HandleCommand(std::string name, std::string param, Player* owner = nullptr);

    private:
        AiObjectContext* aiObjectContext;
    };
}
