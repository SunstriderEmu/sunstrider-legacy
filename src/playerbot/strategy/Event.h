#pragma once

namespace ai
{
    class Event
	{
	public:
        Event(Event const& other)
        {
            source = other.source;
            param = other.param;
            packet = other.packet;
            owner = other.owner;
        }
        Event() {}
        Event(std::string source) : source(std::move(source)) {}
        Event(std::string source, std::string param, Player* owner = nullptr) : source(std::move(source)), param(std::move(param)), owner(owner) {}
        Event(std::string source, WorldPacket &packet, Player* owner = nullptr) : source(std::move(source)), packet(packet), owner(owner) {}
        virtual ~Event() = default;

	public:
        std::string getSource() { return source; }
        std::string getParam() { return param; }
        WorldPacket& getPacket() { return packet; }
        ObjectGuid getObject();
        Player* getOwner() { return owner; }
        bool operator! () const { return source.empty(); }

    protected:
        std::string source;
        std::string param;
        WorldPacket packet;
        ObjectGuid object;
        Player* owner;
	};
}
