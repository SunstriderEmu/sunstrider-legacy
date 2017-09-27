
#ifndef _ARENATEAMMGR_H
#define _ARENATEAMMGR_H

#include "ArenaTeam.h"
#include <unordered_map>

class TC_GAME_API ArenaTeamMgr
{
private:
    ArenaTeamMgr();
    ~ArenaTeamMgr();

public:
    static ArenaTeamMgr* instance();

    void DistributeArenaPoints();

protected:
};

#define sArenaTeamMgr ArenaTeamMgr::instance()

#endif
