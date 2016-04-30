#include "ExternalEventHelper.h"
#include "ChatHelper.h"
#include "AiObjectContext.h"

namespace ai
{
    bool ExternalEventHelper::ParseChatCommand(std::string command, Player* owner)
    {
        if (HandleCommand(command, "", owner))
            return true;

        size_t i = std::string::npos;
        while (true)
        {
            size_t found = command.rfind(" ", i);
            if (found == std::string::npos || !found)
                break;

            std::string name = command.substr(0, found);
            std::string param = command.substr(found + 1);

            i = found - 1;

            if (HandleCommand(name, param, owner))
                return true;
        }

        if (!ChatHelper::parseable(command))
            return false;

        HandleCommand("q", command, owner);
        HandleCommand("c", command, owner);
        HandleCommand("t", command, owner);
        return true;
    }

    void ExternalEventHelper::HandlePacket(std::map<uint16, std::string> &handlers, const WorldPacket &packet, Player* owner)
    {
        uint16 opcode = packet.GetOpcode();
        std::string name = handlers[opcode];
        if (name.empty())
            return;

        Trigger* trigger = aiObjectContext->GetTrigger(name);
        if (!trigger)
            return;

        WorldPacket p(packet);
        trigger->ExternalEvent(p, owner);
    }

    bool ExternalEventHelper::HandleCommand(std::string name, std::string param, Player* owner)
    {
        Trigger* trigger = aiObjectContext->GetTrigger(name);
        if (!trigger)
            return false;

        trigger->ExternalEvent(param, owner);
        return true;
    }
}