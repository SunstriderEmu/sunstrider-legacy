#ifndef TRINITYCORE_WORLDPACKET_H
#define TRINITYCORE_WORLDPACKET_H

#include "Common.h"
#include "ByteBuffer.h"

class WorldPacket : public ByteBuffer
{
    public:
                      // just container for later use
        WorldPacket() : ByteBuffer(0), m_opcode(0)
        {
        }

        explicit WorldPacket(uint16 opcode, size_t res=200) : ByteBuffer(res), m_opcode(opcode) { }

        WorldPacket(WorldPacket&& packet) : ByteBuffer(std::move(packet)), m_opcode(packet.m_opcode)
        {
        }

        WorldPacket(const WorldPacket& right) : ByteBuffer(right), m_opcode(right.m_opcode)
        {
        }

        WorldPacket& operator=(WorldPacket const& right)
        {
            if (this != &right)
            {
                m_opcode = right.m_opcode;
                ByteBuffer::operator =(right);
            }

            return *this;
        }

        WorldPacket(uint16 opcode, MessageBuffer&& buffer) : ByteBuffer(std::move(buffer)), m_opcode(opcode) { }

        void Initialize(uint16 opcode, size_t newres=200)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
        }

        uint16 GetOpcode() const { return m_opcode; }
        void SetOpcode(uint16 opcode) { m_opcode = opcode; }

    protected:
        uint16 m_opcode;
};
#endif

