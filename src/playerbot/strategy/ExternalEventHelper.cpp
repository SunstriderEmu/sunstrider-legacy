#include "ExternalEventHelper.h"

bool ExternalEventHelper::ParseChatCommand(string command, Player* owner)
{
	if (HandleCommand(command, "", owner))
		return true;

	size_t i = string::npos;
	while (true)
	{
		size_t found = command.rfind(" ", i);
		if (found == string::npos || !found)
			break;

		string name = command.substr(0, found);
		string param = command.substr(found + 1);

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

void ExternalEventHelper::HandlePacket(map<uint16, string> &handlers, const WorldPacket &packet, Player* owner)
{
	uint16 opcode = packet.GetOpcode();
	string name = handlers[opcode];
	if (name.empty())
		return;

	Trigger* trigger = aiObjectContext->GetTrigger(name);
	if (!trigger)
		return;

	WorldPacket p(packet);
	trigger->ExternalEvent(p, owner);
}

bool ExternalEventHelper::HandleCommand(string name, string param, Player* owner)
{
	Trigger* trigger = aiObjectContext->GetTrigger(name);
	if (!trigger)
		return false;

	trigger->ExternalEvent(param, owner);
	return true;
}
