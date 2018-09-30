#include "PlayerAntiCheat.h"

std::list<ModuleFactory*> PlayerAntiCheat::factories;

void PlayerAntiCheat::RegisterModule(const std::string& /*name*/, ModuleFactory* m)
{
    factories.push_back(m);
}

PlayerAntiCheat::PlayerAntiCheat(WorldSession* session)
{
    for (auto& f : factories)
        modules.push_back(std::unique_ptr<AntiCheatModule>(f->Create(session)));
}

void PlayerAntiCheat::OnClientPacketReceived(WorldPacket const& packet)
{
    for(auto& m : modules)
        m->OnClientPacketReceived(packet);
}

void PlayerAntiCheat::OnPlayerMoved(Unit const* moved, MovementInfo const& movementInfo, OpcodeClient opcode)
{
    for (auto& m : modules)
        m->OnPlayerMoved(moved, movementInfo, opcode);
}

void PlayerAntiCheat::OnPlayerSetFlying(Unit const* moved, bool set)
{
    for (auto& m : modules)
        m->OnPlayerSetFlying(moved, set);
}

void PlayerAntiCheat::OnPlayerKnockBack(Unit const* moved)
{
    for (auto& m : modules)
        m->OnPlayerKnockBack(moved);
}

void PlayerAntiCheat::OnPlayerTeleport(Unit const* moved)
{
    for (auto& m : modules)
        m->OnPlayerTeleport(moved);
}

void PlayerAntiCheat::OnPlayerMoverChanged(Unit const* oldMover, Unit const* newMover)
{
    for (auto& m : modules)
        m->OnPlayerMoverChanged(oldMover, newMover);
}

void PlayerAntiCheat::OnPlayerSpeedChanged(Unit const* moved, float oldSpeed, float newSpeed)
{
    for (auto& m : modules)
        m->OnPlayerSpeedChanged(moved, oldSpeed, newSpeed);
}

void PlayerAntiCheat::OnPlayerWaterWalk(Unit const* moved)
{
    for (auto& m : modules)
        m->OnPlayerWaterWalk(moved);
}

void PlayerAntiCheat::OnPlayerSlowfall(Unit const* moved)
{
    for (auto& m : modules)
        m->OnPlayerSlowfall(moved);
}
