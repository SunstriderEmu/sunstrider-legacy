#ifndef __TRINITY_REPUTATION_MGR_H
#define __TRINITY_REPUTATION_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "Define.h"

class Player;

namespace ReputationMgr
{
    const int32 PointsInRank[MAX_REPUTATION_RANK] = {36000, 3000, 3000, 3000, 6000, 12000, 21000, 1000};
    const int32 Reputation_Cap = 42999;
    const int32 Reputation_Bottom = -42000;
};

/*
class ReputationMgr
{
};
*/

#endif
