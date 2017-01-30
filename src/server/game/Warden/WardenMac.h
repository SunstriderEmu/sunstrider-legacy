
#ifndef _WARDEN_MAC_H
#define _WARDEN_MAC_H

#include <map>
#include "WardenBase.h"

class ByteBuffer;
class WorldSession;
class WardenBase;
class BigNumber;

class WardenMac : WardenBase
{
    public:
        WardenMac();
        ~WardenMac();

        void Init(WorldSession *pClient, BigNumber *K);
        ClientWardenModule *GetModuleForClient(WorldSession *session);
        void InitializeModule();
        void RequestHash();
        void HandleHashResult(ByteBuffer &buff);
        void RequestData();
        void HandleData(ByteBuffer &buff);
};

#endif
