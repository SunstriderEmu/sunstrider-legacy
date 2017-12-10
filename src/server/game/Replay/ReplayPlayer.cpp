#include "ReplayPlayer.h"
#include "Chat.h"

ReplayPlayer::~ReplayPlayer()
{
    StopRead();
}

void ReplayPlayer::StopRead()
{
    if (_pcktReading)
        fclose(_pcktReading);
    _pcktReading = nullptr;
}

bool ReplayPlayer::UpdateReplay()
{
    if (!_pcktReading)
        return false;

    while (true)
    {
        long int pos = ftell(_pcktReading);
        uint32 nextTime = 0;
        fscanf(_pcktReading, "%u", &nextTime);
        if (!nextTime)
        {
            StopRead();
            break;
        }
        fseek(_pcktReading, pos, SEEK_SET);
        uint32 now = GetMSTime();
        uint32 diff = GetMSTimeDiff(_pcktReadLastUpdate, now);
        _pcktReadLastUpdate = now;
        _pcktReadTimer += diff * _pcktReadSpeedRate;
        if (nextTime > _pcktReadTimer) // Stop
            break;
        // else, send another packet
        uint32 size = 0;
        uint32 opcode = 0;
        int32 readValue = 0;
        if (fscanf(_pcktReading, "%u:%u:%u|%u", &nextTime, &opcode, &size, &readValue) != 4)
            return false;
        else
        {
            WorldPacket data(opcode, size);
            while (true)
            {
                if (data.size() >= size && readValue != 256)
                {
                    if (_player)
                        ChatHandler(_player).PSendSysMessage("[Replay] Invalid packet size [opcode %s|size %u|time %u]", GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), size, nextTime);
                    return false;
                }
                if (readValue == 256)
                    break;
                data << uint8(readValue);
                int readCount = fscanf(_pcktReading, " %u", &readValue);
                if (!readCount)
                {
                    if (_player)
                        ChatHandler(_player).PSendSysMessage("[Replay] Invalid packet (truncated) [opcode %s|size %u|time %u]", GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), size, nextTime);
                    return false;
                }
            }
            _player->GetSession()->SendPacket(&data);
            fscanf(_pcktReading, "\n");
        }
    }
    return true;
}

bool ReplayPlayer::ReadFromFile(std::string const& file, WorldLocation& startLoc)
{
    StopRead(); // Clean
    _pcktReading = fopen(file.c_str(), "r");
    if (_pcktReading)
    {
        uint32 fileTime = 0;
        uint32 recorderGuidLow = 0;
        if (!fscanf(_pcktReading, "BEGIN_TIME=%u\n", &fileTime))
        {
            StopRead();
            return false;
        }
        _pcktReadTimer = fileTime;
        _pcktReadLastUpdate = GetMSTime();
        if (fscanf(_pcktReading, "RECORDER_LOWGUID=%u\n", &recorderGuidLow))
            _recorderGuid = recorderGuidLow;
        else
            _recorderGuid = 0;

        uint32 mapId = 0;
        float x, y, z;
        if (fscanf(_pcktReading, "START_LOC_MAP=%u\n", &mapId))
            startLoc.m_mapId = mapId;
        if (fscanf(_pcktReading, "START_LOC_X=%f\n", &x))
            startLoc.m_positionX = x;
        if (fscanf(_pcktReading, "START_LOC_Y=%f\n", &y))
            startLoc.m_positionY = y;
        if (fscanf(_pcktReading, "START_LOC_Z=%f\n", &z))
            startLoc.m_positionZ = z;
    }
    else
        return false;

    return true;
}

bool ReplayPlayer::OpcodeAllowedWhileReplaying(Opcodes op)
{
    switch (op)
    {
    case CMSG_MESSAGECHAT:
    case MSG_MOVE_STOP:
    case MSG_MOVE_HEARTBEAT:
    case CMSG_ITEM_TEXT_QUERY:
    case CMSG_ITEM_NAME_QUERY:
    case CMSG_NAME_QUERY:
    case CMSG_PET_NAME_QUERY:
    case CMSG_ITEM_QUERY_SINGLE:
    case CMSG_ITEM_QUERY_MULTIPLE:
        return true;
    default:
        // Otherwise we simply ignore
        return false;
    }
}