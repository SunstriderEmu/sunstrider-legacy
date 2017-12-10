#ifndef REPLAY_PLAYER_H
#define REPLAY_PLAYER_H

#include "SharedDefines.h"

class ReplayPlayer
{
public:
	ReplayPlayer(Player* p) :
        _player(p),
        _pcktReading(nullptr), 
        _pcktReadSpeedRate(1.0f), 
        _pcktReadTimer(0), 
        _pcktReadLastUpdate(0)
    {}
    ~ReplayPlayer();

    //return false if ended and ReplayPlayer can be deleted
    bool UpdateReplay();
    static bool OpcodeAllowedWhileReplaying(Opcodes op);

	uint32 GetRecorderGuid() const { return _recorderGuid; }
	void SkipTime(int32 delay) { _pcktReadTimer += delay; }
	void SetSpeedRate(float r) { _pcktReadSpeedRate = r; }
	bool ReadFromFile(std::string const& file, WorldLocation& startLoc);
    void StopRead();

private:
    FILE* _pcktReading;
    float  _pcktReadSpeedRate;
    uint32 _pcktReadTimer;
    uint32 _pcktReadLastUpdate;
    uint32 _recorderGuid;

    Player* _player;
};

#endif //REPLAY_PLAYER_H