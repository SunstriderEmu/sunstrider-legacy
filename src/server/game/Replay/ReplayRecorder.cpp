#include "ReplayRecorder.h"

ReplayRecorder::~ReplayRecorder()
{
    StopPacketDump();
}

bool ReplayRecorder::StartPacketDump(std::string const& file, WorldLocation startPosition)
{
    StopPacketDump(); // Clean
    _pcktWriting = fopen(file.c_str(), "w+");
    if (!_pcktWriting)
        return false;

    fprintf(_pcktWriting, "BEGIN_TIME=%u\n", GetMSTime());
    fprintf(_pcktWriting, "RECORDER_LOWGUID=%u\n", recorderGUID);
    fprintf(_pcktWriting, "START_LOC_MAP=%u\n", startPosition.GetMapId());
    fprintf(_pcktWriting, "START_LOC_X=%f\n", startPosition.GetPositionX());
    fprintf(_pcktWriting, "START_LOC_Y=%f\n", startPosition.GetPositionY());
    fprintf(_pcktWriting, "START_LOC_Z=%f\n", startPosition.GetPositionZ());

    return true;
}

void ReplayRecorder::StopPacketDump()
{
    if (_pcktWriting)
        fclose(_pcktWriting);
    _pcktWriting = nullptr;
}

void ReplayRecorder::AddPacket(WorldPacket const* packet)
{
    std::stringstream oss;
    oss << GetMSTime() << ":" << packet->GetOpcode() << ":" << packet->size() << "|";
    for (size_t i = 0; i < packet->size(); ++i)
        oss << uint32(packet->read<uint8>(i)) << " ";
    oss << "256\n";
    fprintf("%s", _pcktWriting, oss.str().c_str());
}
