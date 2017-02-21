#ifndef REPLAY_RECORDER_H
#define REPLAY_RECORDER_H

#include "SharedDefines.h"

class ReplayRecorder
{
public:
    ReplayRecorder(uint32 recorderGUID) : 
        recorderGUID(recorderGUID),
        _pcktWriting(nullptr)
    {}
    ~ReplayRecorder();

    bool StartPacketDump(std::string const& file, WorldLocation startPosition);
    void StopPacketDump();
    void AddPacket(WorldPacket const* packet);

private:
	FILE* _pcktWriting;
    uint32 recorderGUID;
};

#endif //REPLAY_RECORDER_H