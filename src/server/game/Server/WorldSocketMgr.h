/** \addtogroup u2w User to World Communication
*  @{
*  \file WorldSocketMgr.h
*  \author Derex <derex101@gmail.com>
*/

#ifndef __WORLDSOCKETMGR_H
#define __WORLDSOCKETMGR_H

#include "SocketMgr.h"

class WorldSocket;

/// Manages all sockets connected to peers and network threads
class TC_GAME_API WorldSocketMgr : public SocketMgr<WorldSocket>
{
    typedef SocketMgr<WorldSocket> BaseSocketMgr;
    
public:
    static WorldSocketMgr& Instance();

    /// Start network, listen at address:port .
    bool StartWorldNetwork(Trinity::Asio::IoContext& ioContext, std::string const& bindIp, uint16 port, int networkThreads);

    /// Stops all network threads, It will wait for all running threads .
    void StopNetwork() override;

    void OnSocketOpen(tcp::socket&& sock, uint32 threadIndex) override;

    std::size_t GetApplicationSendBufferSize() const { return _socketApplicationSendBufferSize; }

protected:
    WorldSocketMgr();

    NetworkThread<WorldSocket>* CreateThreads() const override;

private:
    int32 _socketSystemSendBufferSize;
    int32 _socketApplicationSendBufferSize;
    bool _tcpNoDelay;
};

#define sWorldSocketMgr WorldSocketMgr::Instance()

#endif
/// @}
