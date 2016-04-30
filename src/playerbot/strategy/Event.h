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
        Event(std::string source) : source(source) {}
        Event(std::string source, std::string param, Player* owner = NULL) : source(source), param(param), owner(owner) {}
        Event(std::string source, WorldPacket &packet, Player* owner = NULL) : source(source), packet(packet), owner(owner) {}
        virtual ~Event() {}

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
