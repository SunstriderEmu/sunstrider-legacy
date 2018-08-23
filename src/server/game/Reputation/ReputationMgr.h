#ifndef __TRINITY_REPUTATION_MGR_H
#define __TRINITY_REPUTATION_MGR_H

#include "Common.h"
#include "SharedDefines.h"
#include "Language.h"
#include "DBCStructure.h"
#include "QueryResult.h"
#include <map>

static uint32 const ReputationRankStrIndex[MAX_REPUTATION_RANK] =
{
    LANG_REP_HATED,    LANG_REP_HOSTILE, LANG_REP_UNFRIENDLY, LANG_REP_NEUTRAL,
    LANG_REP_FRIENDLY, LANG_REP_HONORED, LANG_REP_REVERED,    LANG_REP_EXALTED
};

enum FactionFlags
{
    FACTION_FLAG_VISIBLE            = 0x01,                 // makes visible in client (set or can be set at interaction with target of this faction)
    FACTION_FLAG_AT_WAR             = 0x02,                 // enable AtWar-button in client. player controlled (except opposition team always war state), Flag only set on initial creation
    FACTION_FLAG_HIDDEN             = 0x04,                 // hidden faction from reputation pane in client (player can gain reputation, but this update not sent to client)
    FACTION_FLAG_INVISIBLE_FORCED   = 0x08,                 // always overwrite FACTION_FLAG_VISIBLE and hide faction in rep.list, used for hide opposite team factions
    FACTION_FLAG_PEACE_FORCED       = 0x10,                 // always overwrite FACTION_FLAG_AT_WAR, used for prevent war with own team factions
    FACTION_FLAG_INACTIVE           = 0x20,                 // player controlled, state stored in characters.data ( CMSG_SET_FACTION_INACTIVE )
    FACTION_FLAG_RIVAL              = 0x40                  // flag for the two competing outland factions
};

namespace ReputationMgr
{
    const int32 PointsInRank[MAX_REPUTATION_RANK] = { 36000, 3000, 3000, 3000, 6000, 12000, 21000, 1000 };
    const int32 Reputation_Cap = 42999;
    const int32 Reputation_Bottom = -42000;
};

#endif // __TRINITY_REPUTATION_MGR_H