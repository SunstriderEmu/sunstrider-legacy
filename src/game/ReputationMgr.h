
#ifndef __TRINITY_REPUTATION_MGR_H
#define __TRINITY_REPUTATION_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "Language.h"
#include "DBCStructure.h"
#include "QueryResult.h"
#include <map>

class Player;

class ReputationMgr
{
    public:                                                 // constructors and global modifiers
  
    public:                                                 // statics
        static const int32 PointsInRank[MAX_REPUTATION_RANK];
        static const int32 Reputation_Cap;
        static const int32 Reputation_Bottom;

};

#endif
