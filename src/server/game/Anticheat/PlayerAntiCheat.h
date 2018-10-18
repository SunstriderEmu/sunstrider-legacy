#ifndef PLAYER_ANTICHEAT_H
#define PLAYER_ANTICHEAT_H

#include "Opcodes.h"

class Unit;
struct MovementInfo;
class WorldSession;
class WorldPacket;

class AntiCheatModule
{
public:
    virtual void OnClientPacketReceived(WorldPacket const& packet) {}
    virtual void OnPlayerMoved(Unit const* moved, MovementInfo const& movementInfo, OpcodeClient opcode) {}
    virtual void OnPlayerSetFlying(Unit const* moved, bool set) {}
    virtual void OnPlayerKnockBack(Unit const* moved) {}
    virtual void OnPlayerTeleport(Unit const* moved) {}
    virtual void OnPlayerMoverChanged(Unit const* oldMover, Unit const* newMover) {}
    virtual void OnPlayerSpeedChanged(Unit const* moved, float oldSpeed, float newSpeed) {}
    virtual void OnPlayerWaterWalk(Unit const* moved) {}
    virtual void OnPlayerSlowfall(Unit const* moved) {}
};

class ModuleFactory
{
public:
    virtual AntiCheatModule* Create(WorldSession*) = 0;
};

class PlayerAntiCheat
{
public:
    PlayerAntiCheat(WorldSession* session);

    static void RegisterModule(const std::string& name, ModuleFactory* m);

    void OnClientPacketReceived(WorldPacket const& packet);
    //movementInfo contains original packet from client, with time untouched
    void OnPlayerMoved(Unit const* moved, MovementInfo const& movementInfo, OpcodeClient opcode);
    void OnPlayerSetFlying(Unit const* moved, bool set);
    void OnPlayerKnockBack(Unit const* moved);
    void OnPlayerTeleport(Unit const* moved);
    //oldMover, newMover may be nullptr
    void OnPlayerMoverChanged(Unit const* oldMover, Unit const* newMover);
    void OnPlayerSpeedChanged(Unit const* moved, float oldSpeed, float newSpeed);
    void OnPlayerWaterWalk(Unit const* moved);
    void OnPlayerSlowfall(Unit const* moved);

private:
    std::list<std::unique_ptr<AntiCheatModule>> modules;

    static std::list<ModuleFactory*> factories;
};

#define REGISTER_ANTICHEAT_MODULE(_class) \
    class _class##Factory : public ModuleFactory { \
    public: \
        _class##Factory() \
        { \
            PlayerAntiCheat::RegisterModule(#_class, this); \
        } \
        virtual AntiCheatModule* Create(WorldSession* session) { \
            return new _class(session); \
        } \
    }; \
    static _class##Factory global_##_class##Factory;

#endif