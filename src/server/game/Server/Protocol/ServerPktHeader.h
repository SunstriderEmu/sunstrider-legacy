#ifndef __SERVERPKTHDR_H__
#define __SERVERPKTHDR_H__

#pragma pack(push, 1)

#ifdef LICH_KING
struct ServerPktHeader
{
    /**
     * size is the length of the payload _plus_ the length of the opcode
     */
    ServerPktHeader(uint32 size, uint16 cmd) : size(size)
    {
        uint8 headerIndex=0;
        if (isLargePacket())
        {
            TC_LOG_TRACE("network", "initializing large server to client packet. Size: %u, cmd: %u", size, cmd);
            header[headerIndex++] = 0x80 | (0xFF & (size >> 16));
        }
        header[headerIndex++] = 0xFF &(size >> 8);
        header[headerIndex++] = 0xFF & size;

        header[headerIndex++] = 0xFF & cmd;
        header[headerIndex++] = 0xFF & (cmd >> 8);
    }

    uint8 getHeaderLength() const
    {
        // cmd = 2 bytes, size= 2||3bytes
        return 2 + (isLargePacket() ? 3 : 2);
    }

    bool isLargePacket() const
    {
        return size > 0x7FFF;
    }

    const uint32 size;
    uint8 header[5];
};
#else
struct ServerPktHeader
{
    /**
    * size is the length of the payload _plus_ the length of the opcode
    */
    ServerPktHeader(uint32 size, uint16 cmd)
    {
        DEBUG_ASSERT(size < std::numeric_limits<uint16>::max());
        header[0] = 0xFF & (size >> 8);
        header[1] = 0xFF & size;

        header[2] = 0xFF & cmd;
        header[3] = 0xFF & (cmd >> 8);
    }

    inline uint8 getHeaderLength() const
    {
        return 2 + 2;
    }

    uint8 header[4];
};

#endif

#pragma pack(pop)

#endif
