#include "ArenaTeamMgr.h"
#include "World.h"
#include "Player.h"

ArenaTeamMgr::ArenaTeamMgr()
{
}

ArenaTeamMgr::~ArenaTeamMgr()
{
}

ArenaTeamMgr* ArenaTeamMgr::instance()
{
    static ArenaTeamMgr instance;
    return &instance;
}

void ArenaTeamMgr::DistributeArenaPoints()
{
    // used to distribute arena points based on last week's stats
    sWorld->SendGlobalText("Distributing arena points to players...", nullptr);

    //temporary structure for storing maximum points to add values for all players
    std::map<ObjectGuid::LowType, uint32> PlayerPoints;

    //at first update all points for all team members
    for (auto team_itr = sObjectMgr->GetArenaTeamMapBegin(); team_itr != sObjectMgr->GetArenaTeamMapEnd(); ++team_itr)
    {
        if (ArenaTeam * at = team_itr->second)
        {
            at->UpdateArenaPointsHelper(PlayerPoints);
        }
    }

    //cycle that gives points to all players
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for (auto & PlayerPoint : PlayerPoints)
    {
        // Update database
        trans->PAppend("UPDATE characters SET arenaPoints = arenaPoints + '%u' WHERE guid = '%u'", PlayerPoint.second, PlayerPoint.first);

        // Add points if player is online
        Player* pl = ObjectAccessor::FindConnectedPlayer(ObjectGuid(HighGuid::Player, PlayerPoint.first));
        if (pl)
            pl->ModifyArenaPoints(PlayerPoint.second);
    }
    CharacterDatabase.CommitTransaction(trans);

    PlayerPoints.clear();

    sWorld->SendGlobalText("Finished setting arena points for online players.", nullptr);

    sWorld->SendGlobalText("Modifying played count, arena points etc. for loaded arena teams, sending updated stats to online players...", nullptr);
    for (auto titr = sObjectMgr->GetArenaTeamMapBegin(); titr != sObjectMgr->GetArenaTeamMapEnd(); ++titr)
    {
        if (ArenaTeam * at = titr->second)
        {
            if (at->GetType() == ARENA_TEAM_2v2 && sWorld->getConfig(CONFIG_ARENA_DECAY_ENABLED))
                at->HandleDecay();

            at->FinishWeek();                              // set played this week etc values to 0 in memory, too
            at->SaveToDB();                                // save changes
            at->NotifyStatsChanged();                      // notify the players of the changes
        }
    }

    if (sWorld->getConfig(CONFIG_ARENA_NEW_TITLE_DISTRIB))
        sWorld->updateArenaLeadersTitles();

    sWorld->SendGlobalText("Modification done.", nullptr);

    sWorld->SendGlobalText("Done flushing Arena points.", nullptr);
}
