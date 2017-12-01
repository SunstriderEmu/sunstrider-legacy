
#ifndef TRINITY_M2STORES_H
#define TRINITY_M2STORES_H

#include "Define.h"
#include "Position.h"
#include <vector>

struct FlyByCamera
{
    uint32 timeStamp;
    Position locations;
};

TC_GAME_API void LoadM2Cameras(std::string const& dataPath);

TC_GAME_API std::vector<FlyByCamera> const* GetFlyByCameras(uint32 cinematicCameraId);

#endif
