#ifndef REPLAY_RECORDER_H
#define REPLAY_RECORDER_H

#include "SharedDefines.h"

class ReplayRecorder
{
public:
    ReplayRecorder(ObjectGuid::LowType recorderGUID) :
        recorderGUID(recorderGUID),
        _pcktWriting(nullptr)
    {}
    ~ReplayRecorder();

    bool StartPacketDump(std::string const& file, WorldLocation startPosition);
    void StopPacketDump();
    void AddPacket(WorldPacket const* packet);

private:
	FILE* _pcktWriting;
    ObjectGuid::LowType recorderGUID;
};

#endif //REPLAY_RECORDER_H