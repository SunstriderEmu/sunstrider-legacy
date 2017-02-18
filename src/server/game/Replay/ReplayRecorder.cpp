#include "ReplayRecorder.h"

ReplayRecorder::~ReplayRecorder()
{
    StartPacketDump("");
}

bool ReplayRecorder::StartPacketDump(std::string const& file)
{
    if (file == "")
    {
        if (_pcktWriting)
            fclose(_pcktWriting);
        _pcktWriting = nullptr;
        return false;
    }
    StartPacketDump(""); // Clean
    _pcktWriting = fopen(file.c_str(), "w+");
    if (!_pcktWriting)
        return false;

    fprintf(_pcktWriting, "BEGIN_TIME=%u\n", GetMSTime());
    fprintf(_pcktWriting, "RECORDER_LOWGUID=%u\n", recorderGUID);

    return true;
}

void ReplayRecorder::AddPacket(WorldPacket const* packet)
{
    std::stringstream oss;
    oss << GetMSTime() << ":" << packet->GetOpcode() << ":" << packet->size() << "|";
    for (size_t i = 0; i < packet->size(); ++i)
        oss << uint32(packet->read<uint8>(i)) << " ";
    oss << "256\n";
    fprintf(_pcktWriting, oss.str().c_str());
}