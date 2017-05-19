
#include "Common.h"
#include "Player.h"
#include "BattleGroundMgr.h"
#include "BattlegroundQueue.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "BattleGroundNA.h"
#include "BattleGroundBE.h"
#include "BattleGroundAA.h"
#include "BattleGroundRL.h"
#include "SharedDefines.h"
#include "MapManager.h"
#include "Map.h"
#include "MapInstanced.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Chat.h"
#include "ArenaTeam.h"

/*********************************************************/
/***            BATTLEGROUND QUEUE SYSTEM              ***/
/*********************************************************/

BattlegroundQueue::BattlegroundQueue()
{
    m_avgTime = 0;
    m_lastTimes.clear();
}

BattlegroundQueue::~BattlegroundQueue()
{
    for (int i = 0; i < MAX_BATTLEGROUND_QUEUE_RANGES; i++)
    {
        m_QueuedPlayers[i].clear();
        for(auto & itr : m_QueuedGroups[i])
        {
            delete itr;
        }
        m_QueuedGroups[i].clear();
    }
}

void BattlegroundQueue::AddStatsForAvgTime(uint32 time)
{
    m_lastTimes.push_back(time);
    if (m_lastTimes.size() > 10)
        m_lastTimes.pop_front();
        
    uint32 totalTime = 0;
    for (std::list<uint32>::const_iterator itr = m_lastTimes.begin(); itr != m_lastTimes.end(); itr++)
        totalTime += (*itr);
        
    m_avgTime = uint32(totalTime / m_lastTimes.size());
    //TC_LOG_INFO("bg.battleground","New average time: %u", m_avgTime);
    //m_avgTime = uint32((prevTime + time) / m_playerCount);
}

// initialize eligible groups from the given source matching the given specifications
void BattlegroundQueue::EligibleGroups::Init(BattlegroundQueue::QueuedGroupsList *source, uint32 BgTypeId, uint32 side, uint32 MaxPlayers, uint8 ArenaType, bool IsRated, uint32 MinRating, uint32 MaxRating, uint32 DisregardTime, uint32 excludeTeam)
{
    // clear from prev initialization
    clear();
    BattlegroundQueue::QueuedGroupsList::iterator itr, next;
    // iterate through the source
    for(itr = source->begin(); itr!= source->end(); itr = next)
    {
        next = itr;
        ++next;
        if( (*itr)->BgTypeId == BgTypeId &&     // bg type must match
            (*itr)->ArenaType == ArenaType &&   // arena type must match
            (*itr)->IsRated == IsRated &&       // israted must match
            (*itr)->IsInvitedToBGInstanceGUID == 0 && // leave out already invited groups
            (*itr)->Team == side &&             // match side
            (*itr)->Players.size() <= MaxPlayers &&   // the group must fit in the bg
            ( !excludeTeam || (*itr)->ArenaTeamId != excludeTeam ) && // if excludeTeam is specified, leave out those arena team ids
            ( !IsRated || (*itr)->Players.size() == MaxPlayers ) &&   // if rated, then pass only if the player count is exact NEEDS TESTING! (but now this should never happen)
            ( !DisregardTime || (*itr)->JoinTime <= DisregardTime              // pass if disregard time is greater than join time
               || (*itr)->ArenaTeamRating == 0                 // pass if no rating info
               || ( (*itr)->ArenaTeamRating >= MinRating       // pass if matches the rating range
                     && (*itr)->ArenaTeamRating <= MaxRating ) ) )
        {
            // the group matches the conditions
            // using push_back for proper selecting when inviting
            push_back((*itr));
        }
    }
}

// selection pool initialization, used to clean up from prev selection
void BattlegroundQueue::SelectionPool::Init(EligibleGroups * curr)
{
    m_CurrEligGroups = curr;
    SelectedGroups.clear();
    PlayerCount = 0;
}

// remove group info from selection pool
void BattlegroundQueue::SelectionPool::RemoveGroup(GroupQueueInfo *ginfo)
{
    // find what to remove
    for(auto itr = SelectedGroups.begin(); itr != SelectedGroups.end(); ++itr)
    {
        if((*itr)==ginfo)
        {
            SelectedGroups.erase(itr);
            // decrease selected players count
            PlayerCount -= ginfo->Players.size();
            return;
        }
    }
}

// add group to selection
// used when building selection pools
void BattlegroundQueue::SelectionPool::AddGroup(GroupQueueInfo * ginfo)
{
    SelectedGroups.push_back(ginfo);
    // increase selected players count
    PlayerCount+=ginfo->Players.size();
}

// add group to bg queue with the given leader and bg specifications
GroupQueueInfo * BattlegroundQueue::AddGroup(Player *leader, uint32 BgTypeId, uint8 ArenaType, bool isRated, uint32 arenaRating, uint32 arenateamid)
{
    uint32 queue_id = leader->GetBattlegroundQueueIdFromLevel();

    // create new ginfo
    // cannot use the method like in addplayer, because that could modify an in-queue group's stats
    // (e.g. leader leaving queue then joining as individual again)
    auto  ginfo = new GroupQueueInfo;
    ginfo->BgTypeId                  = BgTypeId;
    ginfo->ArenaType                 = ArenaType;
    ginfo->ArenaTeamId               = arenateamid;
    ginfo->IsRated                   = isRated;
    ginfo->IsInvitedToBGInstanceGUID = 0;                       // maybe this should be modifiable by function arguments to enable selection of running instances?
    ginfo->JoinTime                  = GetMSTime();
    ginfo->Team                      = leader->GetTeam();
    ginfo->ArenaTeamRating           = arenaRating;
    ginfo->OpponentsTeamRating       = 0;                       //initialize it to 0

    ginfo->Players.clear();

    m_QueuedGroups[queue_id].push_back(ginfo);

    // return ginfo, because it is needed to add players to this group info
    return ginfo;
}

void BattlegroundQueue::AddPlayer(Player *plr, GroupQueueInfo *ginfo)
{
    uint32 queue_id = plr->GetBattlegroundQueueIdFromLevel();

    //if player isn't in queue, he is added, if already is, then values are overwritten, no memory leak
    PlayerQueueInfo& info = m_QueuedPlayers[queue_id][plr->GetGUID()];
    info.InviteTime                 = 0;
    info.LastInviteTime             = 0;
    info.LastOnlineTime             = GetMSTime();
    info.GroupInfo                  = ginfo;

    // add the pinfo to ginfo's list
    ginfo->Players[plr->GetGUID()]  = &info;
}

void BattlegroundQueue::RemovePlayer(uint64 guid, bool decreaseInvitedCount)
{
    Player *plr = ObjectAccessor::FindConnectedPlayer(guid);

    int32 queue_id = 0;                                     // signed for proper for-loop finish
    QueuedPlayersMap::iterator itr;
    GroupQueueInfo * group;
    QueuedGroupsList::iterator group_itr;
    bool IsSet = false;
    if(plr)
    {
        queue_id = plr->GetBattlegroundQueueIdFromLevel();

        itr = m_QueuedPlayers[queue_id].find(guid);
        if(itr != m_QueuedPlayers[queue_id].end())
            IsSet = true;
    }

    if(!IsSet)
    {
        // either player is offline, or he levelled up to another queue category
        // TC_LOG_ERROR("bg.battleground","Battleground: removing offline player from BG queue - this might not happen, but it should not cause crash");
        for (uint32 i = 0; i < MAX_BATTLEGROUND_QUEUE_RANGES; i++)
        {
            itr = m_QueuedPlayers[i].find(guid);
            if(itr != m_QueuedPlayers[i].end())
            {
                queue_id = i;
                IsSet = true;
                break;
            }
        }
    }

    // couldn't find the player in bg queue, return
    if(!IsSet)
    {
        //TC_LOG_ERROR("bg.battleground","Battleground: couldn't find player to remove.");
        return;
    }

    group = itr->second.GroupInfo;

    for(group_itr=m_QueuedGroups[queue_id].begin(); group_itr != m_QueuedGroups[queue_id].end(); ++group_itr)
    {
        if(group == (GroupQueueInfo*)(*group_itr))
            break;
    }

    // variables are set (what about leveling up when in queue????)
    // remove player from group
    // if only player there, remove group

    // remove player queue info from group queue info
    auto pitr = group->Players.find(guid);

    if(pitr != group->Players.end())
        group->Players.erase(pitr);

    // check for iterator correctness
    if (group_itr != m_QueuedGroups[queue_id].end() && itr != m_QueuedPlayers[queue_id].end())
    {
        // used when player left the queue, NOT used when porting to bg
        if (decreaseInvitedCount)
        {
            // if invited to bg, and should decrease invited count, then do it
            if(group->IsInvitedToBGInstanceGUID)
            {
                Battleground* bg = sBattlegroundMgr->GetBattleground(group->IsInvitedToBGInstanceGUID);
                if (bg)
                    bg->DecreaseInvitedCount(group->Team);
                if (bg && !bg->GetPlayersSize() && !bg->GetInvitedCount(ALLIANCE) && !bg->GetInvitedCount(HORDE))
                {
                    // no more players on battleground, set delete it
                    bg->SetDeleteThis();
                }
            }
            // update the join queue, maybe now the player's group fits in a queue!
            // not yet implemented (should store bgTypeId in group queue info?)
        }
        // remove player queue info
        m_QueuedPlayers[queue_id].erase(itr);
        // remove group queue info if needed
        if(group->Players.empty())
        {
            m_QueuedGroups[queue_id].erase(group_itr);
            delete group;
        }
        // NEEDS TESTING!
        // group wasn't empty, so it wasn't deleted, and player have left a rated queue -> everyone from the group should leave too
        // don't remove recursively if already invited to bg!
        else if(!group->IsInvitedToBGInstanceGUID && decreaseInvitedCount && group->IsRated)
        {
            // remove next player, this is recursive
            // first send removal information
            if(Player *plr2 = ObjectAccessor::FindConnectedPlayer(group->Players.begin()->first))
            {
                Battleground * bg = sBattlegroundMgr->GetBattlegroundTemplate(group->BgTypeId);
                uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(group->BgTypeId,group->ArenaType);
                uint32 queueSlot = plr2->GetBattlegroundQueueIndex(bgQueueTypeId);
                plr2->RemoveBattlegroundQueueId(bgQueueTypeId); // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, plr2->GetTeam(), queueSlot, STATUS_NONE, 0, 0);
                plr2->SendDirectMessage(&data);
            }
            // then actually delete, this may delete the group as well!
            RemovePlayer(group->Players.begin()->first,decreaseInvitedCount);
        }
    }
}

bool BattlegroundQueue::InviteGroupToBG(GroupQueueInfo * ginfo, Battleground * bg, uint32 side)
{
    // set side if needed
    if(side)
        ginfo->Team = side;

    if(!ginfo->IsInvitedToBGInstanceGUID)
    {
        // not yet invited
        // set invitation
        ginfo->IsInvitedToBGInstanceGUID = bg->GetInstanceID();
        uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
        // loop through the players
        for(auto itr = ginfo->Players.begin(); itr != ginfo->Players.end(); ++itr)
        {
            // set status
            itr->second->InviteTime = GetMSTime();
            itr->second->LastInviteTime = GetMSTime();

            // get the player
            Player* plr = ObjectAccessor::FindConnectedPlayer(itr->first);
            // if offline, skip him
            if(!plr)
                continue;

            // invite the player
            sBattlegroundMgr->InvitePlayer(plr, bg->GetInstanceID(),ginfo->Team);

            WorldPacket data;

            uint32 queueSlot = plr->GetBattlegroundQueueIndex(bgQueueTypeId);

            // send status packet
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, side?side:plr->GetTeam(), queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME, 0);
            plr->SendDirectMessage(&data);
            
            // Update average wait time information
            AddStatsForAvgTime(GetMSTimeDiff(itr->second->GroupInfo->JoinTime, GetMSTime()));
        }
        return true;
    }

    return false;
}

// used to recursively select groups from eligible groups
bool BattlegroundQueue::SelectionPool::Build(uint32 MinPlayers, uint32 MaxPlayers, EligibleGroups::iterator startitr)
{
    // start from the specified start iterator
    for(auto itr1 = startitr; itr1 != m_CurrEligGroups->end(); ++itr1)
    {
        // if it fits in, select it
        if(GetPlayerCount() + (*itr1)->Players.size() <= MaxPlayers)
        {
            auto next = itr1;
            ++next;
            AddGroup((*itr1));
            if(GetPlayerCount() >= MinPlayers)
            {
                // enough players are selected
                return true;
            }
            // try building from the rest of the elig. groups
            // if that succeeds, return true
            if(Build(MinPlayers,MaxPlayers,next))
                return true;
            // the rest didn't succeed, so this group cannot be included
            RemoveGroup((*itr1));
        }
    }
    if (MinPlayers == 0)
        return true;

    // build didn't succeed
    return false;
}

// this function is responsible for the selection of queued groups when trying to create new battlegrounds
bool BattlegroundQueue::BuildSelectionPool(uint32 bgTypeId, uint32 queue_id, uint32 MinPlayers, uint32 MaxPlayers,  SelectionPoolBuildMode mode, uint8 ArenaType, bool isRated, uint32 MinRating, uint32 MaxRating, uint32 DisregardTime, uint32 excludeTeam)
{
    uint32 side;
    switch(mode)
    {
    case NORMAL_ALLIANCE:
    case ONESIDE_ALLIANCE_TEAM1:
    case ONESIDE_ALLIANCE_TEAM2:
        side = ALLIANCE;
        break;
    case NORMAL_HORDE:
    case ONESIDE_HORDE_TEAM1:
    case ONESIDE_HORDE_TEAM2:
        side = HORDE;
        break;
    default:
        //unknown mode, return false
        TC_LOG_ERROR("battleground","Battleground: unknown selection pool build mode %u, returning...", mode);
        return false;
        break;
    }

    // initiate the groups eligible to create the bg
    m_EligibleGroups.Init(&(m_QueuedGroups[queue_id]), bgTypeId, side, MaxPlayers, ArenaType, isRated, MinRating, MaxRating, DisregardTime, excludeTeam);
    // init the selected groups (clear)
    // and set m_CurrEligGroups pointer
    // we set it this way to only have one EligibleGroups object to save some memory
    m_SelectionPools[mode].Init(&m_EligibleGroups);
    // build succeeded
    if(m_SelectionPools[mode].Build(MinPlayers,MaxPlayers,m_EligibleGroups.begin()))
        return true;

    // failed to build a selection pool matching the given values
    return false;
}

// used to remove the Enter Battle window if the battle has already, but someone still has it
// (this can happen in arenas mainly, since the preparation is shorter than the timer for the bgqueueremove event
void BattlegroundQueue::BGEndedRemoveInvites(Battleground *bg)
{
    uint32 queue_id = bg->GetQueueType();
    uint32 bgInstanceId = bg->GetInstanceID();
    uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
    QueuedGroupsList::iterator itr, next;
    for(itr = m_QueuedGroups[queue_id].begin(); itr != m_QueuedGroups[queue_id].end(); itr = next)
    {
        // must do this way, because the groupinfo will be deleted when all playerinfos are removed
        GroupQueueInfo * ginfo = (*itr);
        next = itr;
        ++next;
        // if group was invited to this bg instance, then remove all references
        if(ginfo->IsInvitedToBGInstanceGUID == bgInstanceId)
        {
            // after removing this much playerinfos, the ginfo will be deleted, so we'll use a for loop
            uint32 to_remove = ginfo->Players.size();
            uint32 team = ginfo->Team;
            for(int i = 0; i < to_remove; ++i)
            {
                // always remove the first one in the group
                auto itr2 = ginfo->Players.begin();
                if(itr2 == ginfo->Players.end())
                {
                    TC_LOG_ERROR("battleground","Empty Players in ginfo, this should never happen!");
                    return;
                }

                // get the player
                Player * plr = ObjectAccessor::FindConnectedPlayer(itr2->first);
                if(!plr)
                {
                    TC_LOG_ERROR("battleground","Player offline when trying to remove from GroupQueueInfo, this should never happen.");
                    continue;
                }

                // get the queueslot
                uint32 queueSlot = plr->GetBattlegroundQueueIndex(bgQueueTypeId);
                if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
                {
                    plr->RemoveBattlegroundQueueId(bgQueueTypeId);
                    // remove player from queue, this might delete the ginfo as well! don't use that pointer after this!
                    RemovePlayer(itr2->first, true);
                    // this is probably unneeded, since this player was already invited -> does not fit when initing eligible groups
                    // but updating the queue can't hurt
                    BattlegroundQueueUpdate(bgQueueTypeId, bg->GetQueueType());
                    // send info to client
                    WorldPacket data;
                    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, team, queueSlot, STATUS_NONE, 0, 0);
                    plr->SendDirectMessage(&data);
                }
            }
        }
    }
}

/*
this method is called when group is inserted, or player / group is removed from BG Queue - there is only one player's status changed, so we don't use while(true) cycles to invite whole queue
it must be called after fully adding the members of a group to ensure group joining
should be called after removeplayer functions in some cases
*/
void BattlegroundQueue::BattlegroundQueueUpdate(uint32 bgTypeId, uint32 queue_id, uint8 arenatype, bool isRated, uint32 arenaRating)
{
    if (queue_id >= MAX_BATTLEGROUND_QUEUE_RANGES)
    {
        //this is error, that caused crashes (not in , but now it shouldn't)
        TC_LOG_ERROR("battleground","BattlegroundQueue::BattlegroundQueueUpdate() called for non existing queue type - this can cause crash, pls report problem, if this is the last line of error log before crash");
        return;
    }

    //if no players in queue ... do nothing
    if (m_QueuedGroups[queue_id].empty())
        return;

    //battleground with free slot for player should be always the last in this queue
    BGFreeSlotQueueType::iterator itr, next;
    for (itr = sBattlegroundMgr->BGFreeSlotQueue[bgTypeId].begin(); itr != sBattlegroundMgr->BGFreeSlotQueue[bgTypeId].end(); itr = next)
    {
        next = itr;
        ++next;
        // battleground is running, so if:
        // DO NOT allow queue manager to invite new player to running arena
        if ((*itr)->isBattleground() && (*itr)->GetTypeID() == bgTypeId && (*itr)->GetQueueType() == queue_id && (*itr)->GetStatus() > STATUS_WAIT_QUEUE && (*itr)->GetStatus() < STATUS_WAIT_LEAVE)
        {
            //we must check both teams
            Battleground* bg = *itr; //we have to store battleground pointer here, because when battleground is full, it is removed from free queue (not yet implemented!!)
            // and iterator is invalid

            for(auto & itr : m_QueuedGroups[queue_id])
            {
                // did the group join for this bg type?
                if(itr->BgTypeId != bgTypeId)
                    continue;
                // if so, check if fits in
                if(bg->GetFreeSlotsForTeam(itr->Team) >= itr->Players.size())
                {
                    // if group fits in, invite it
                    InviteGroupToBG(itr,bg,itr->Team);
                }
                // invite group if it has enough free slots in absolute (not with GetFreeSlotsForTeam)
                if (itr->Players.size() > 1 && (bg->GetInvitedCount(itr->Team) + itr->Players.size()) < bg->GetMaxPlayersPerTeam()) {
                    InviteGroupToBG(itr,bg,itr->Team);
                }
            }

            if (!bg->HasFreeSlots())
            {
                //remove BG from BGFreeSlotQueue
                bg->RemoveFromBGFreeSlotQueue();
            }
        }
    }

    // finished iterating through the bgs with free slots, maybe we need to create a new bg

    Battleground * bg_template = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if(!bg_template)
    {
        TC_LOG_ERROR("bg.battleground","Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }

    // get the min. players per team, properly for larger arenas as well. (must have full teams for arena matches!)
    uint32 MinPlayersPerTeam = bg_template->GetMinPlayersPerTeam();
    uint32 MaxPlayersPerTeam = bg_template->GetMaxPlayersPerTeam();
    if(bg_template->isBattleground())
    {
        if(sBattlegroundMgr->IsBattleGroundTesting())
            MinPlayersPerTeam = 0;
    }
    if(bg_template->IsArena())
    {
        
        //is this really needed ?
        switch(arenatype)
        {
        case ARENA_TYPE_2v2:
            MaxPlayersPerTeam = 2;
            MinPlayersPerTeam = 2;
            break;
        case ARENA_TYPE_3v3:
            MaxPlayersPerTeam = 3;
            MinPlayersPerTeam = 3;
            break;
        case ARENA_TYPE_5v5:
            MaxPlayersPerTeam = 5;
            MinPlayersPerTeam = 5;
            break;
        }

        if (sBattlegroundMgr->IsArenaTesting())
            MinPlayersPerTeam = 1;
    }

    // found out the minimum and maximum ratings the newly added team should battle against
    // arenaRating is the rating of the latest joined team
    uint32 arenaMinRating = (arenaRating <= sBattlegroundMgr->GetMaxRatingDifference()) ? 0 : arenaRating - sBattlegroundMgr->GetMaxRatingDifference();
    // if no rating is specified, set maxrating to 0
    uint32 arenaMaxRating = (arenaRating == 0)? 0 : arenaRating + sBattlegroundMgr->GetMaxRatingDifference();
    uint32 discardTime = 0;
    // if max rating difference is set and the time past since server startup is greater than the rating discard time
    // (after what time the ratings aren't taken into account when making teams) then
    // the discard time is current_time - time_to_discard, teams that joined after that, will have their ratings taken into account
    // else leave the discard time on 0, this way all ratings will be discarded
    if(sBattlegroundMgr->GetMaxRatingDifference() && GetMSTime() >= sBattlegroundMgr->GetRatingDiscardTimer() && arenatype == ARENA_TYPE_2v2)
        discardTime = GetMSTime() - sBattlegroundMgr->GetRatingDiscardTimer();

    // try to build the selection pools
    bool bAllyOK = BuildSelectionPool(bgTypeId, queue_id, MinPlayersPerTeam, MaxPlayersPerTeam, NORMAL_ALLIANCE, arenatype, isRated, arenaMinRating, arenaMaxRating, discardTime);
    bool bHordeOK = BuildSelectionPool(bgTypeId, queue_id, MinPlayersPerTeam, MaxPlayersPerTeam, NORMAL_HORDE, arenatype, isRated, arenaMinRating, arenaMaxRating, discardTime);

    // if selection pools are ready, create the new bg
    if (bAllyOK && bHordeOK)
    {
        Battleground * bg2 = nullptr;
        // special handling for arenas
        if(bg_template->IsArena())
        {
            // Find a random arena, that can be created
            uint8 arenas[] = {BATTLEGROUND_NA, BATTLEGROUND_BE, BATTLEGROUND_RL};
            uint32 arena_num = urand(0,2);
            if( !(bg2 = sBattlegroundMgr->CreateNewBattleground(arenas[arena_num%3])) &&
                !(bg2 = sBattlegroundMgr->CreateNewBattleground(arenas[(arena_num+1)%3])) &&
                !(bg2 = sBattlegroundMgr->CreateNewBattleground(arenas[(arena_num+2)%3])) )
            {
                TC_LOG_ERROR("bg.battleground","Battleground: couldn't create any arena instance!");
                return;
            }

            // set the MaxPlayersPerTeam values based on arenatype
            // setting the min player values isn't needed, since we won't be using that value later on.
            if(sBattlegroundMgr->IsArenaTesting())
            {
                bg2->SetMaxPlayersPerTeam(1);
                bg2->SetMaxPlayers(2);
            }
            else
            {
                switch(arenatype)
                {
                case ARENA_TYPE_2v2:
                    bg2->SetMaxPlayersPerTeam(2);
                    bg2->SetMaxPlayers(4);
                    break;
                case ARENA_TYPE_3v3:
                    bg2->SetMaxPlayersPerTeam(3);
                    bg2->SetMaxPlayers(6);
                    break;
                case ARENA_TYPE_5v5:
                    bg2->SetMaxPlayersPerTeam(5);
                    bg2->SetMaxPlayers(10);
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            // create new battleground
            bg2 = sBattlegroundMgr->CreateNewBattleground(bgTypeId);
            if(sBattlegroundMgr->IsBattleGroundTesting())
            {
                bg2->SetMaxPlayersPerTeam(1);
                bg2->SetMaxPlayers(2);
            }
        }

        if(!bg2)
        {
            TC_LOG_ERROR("bg.battleground","Battleground: couldn't create bg %u",bgTypeId);
            return;
        }

        // start the joining of the bg
        bg2->SetStatus(STATUS_WAIT_JOIN);
        bg2->SetQueueType(queue_id);
        // initialize arena / rating info
        bg2->SetArenaType(arenatype);
        // set rating
        bg2->SetRated(isRated);

        std::list<GroupQueueInfo* >::iterator itr;

        // Send amount of invites based on the difference between the sizes of the two faction's queues
        uint32 QUEUED_HORDE = m_SelectionPools[NORMAL_HORDE].SelectedGroups.size();
        uint32 QUEUED_ALLIANCE = m_SelectionPools[NORMAL_ALLIANCE].SelectedGroups.size();
        uint16 maxbginvites = 0;

        if(QUEUED_ALLIANCE <= QUEUED_HORDE)
            maxbginvites = QUEUED_ALLIANCE;
        else
            maxbginvites = QUEUED_HORDE;

        // invite groups from horde selection pool
        uint16 invitecounter = 0;
        for(itr = m_SelectionPools[NORMAL_HORDE].SelectedGroups.begin(); itr != m_SelectionPools[NORMAL_HORDE].SelectedGroups.end(); ++itr)
        {
            if (invitecounter >= maxbginvites)
                return;
            InviteGroupToBG((*itr),bg2,HORDE);
            ++invitecounter;
        }

        // invite groups from ally selection pool
        invitecounter = 0;
        for(itr = m_SelectionPools[NORMAL_ALLIANCE].SelectedGroups.begin(); itr != m_SelectionPools[NORMAL_ALLIANCE].SelectedGroups.end(); ++itr)
        {
            if (invitecounter >= maxbginvites)
                return;
            InviteGroupToBG((*itr),bg2,ALLIANCE);
            ++invitecounter;
        }

        if (isRated)
        {
            auto itr_alliance = m_SelectionPools[NORMAL_ALLIANCE].SelectedGroups.begin();
            auto itr_horde = m_SelectionPools[NORMAL_HORDE].SelectedGroups.begin();
            (*itr_alliance)->OpponentsTeamRating = (*itr_horde)->ArenaTeamRating;
            (*itr_horde)->OpponentsTeamRating = (*itr_alliance)->ArenaTeamRating;
        }

        bg2->StartBattleground();
    }

    // there weren't enough players for a "normal" match
    // if arena, enable horde versus horde or alliance versus alliance teams here

    else if(bg_template->IsArena())
    {
        bool bOneSideHordeTeam1 = false, bOneSideHordeTeam2 = false;
        bool bOneSideAllyTeam1 = false, bOneSideAllyTeam2 = false;
        bOneSideHordeTeam1 = BuildSelectionPool(bgTypeId, queue_id,MaxPlayersPerTeam,MaxPlayersPerTeam,ONESIDE_HORDE_TEAM1,arenatype, isRated, arenaMinRating, arenaMaxRating, discardTime);
        if(bOneSideHordeTeam1)
        {
            // one team has been selected, find out if other can be selected too
            std::list<GroupQueueInfo* >::iterator itr;
            // temporarily change the team side to enable building the next pool excluding the already selected groups
            for(itr = m_SelectionPools[ONESIDE_HORDE_TEAM1].SelectedGroups.begin(); itr != m_SelectionPools[ONESIDE_HORDE_TEAM1].SelectedGroups.end(); ++itr)
                (*itr)->Team=ALLIANCE;

            bOneSideHordeTeam2 = BuildSelectionPool(bgTypeId, queue_id,MaxPlayersPerTeam,MaxPlayersPerTeam,ONESIDE_HORDE_TEAM2,arenatype, isRated, arenaMinRating, arenaMaxRating, discardTime, (*(m_SelectionPools[ONESIDE_HORDE_TEAM1].SelectedGroups.begin()))->ArenaTeamId);

            // change back the team to horde
            for(itr = m_SelectionPools[ONESIDE_HORDE_TEAM1].SelectedGroups.begin(); itr != m_SelectionPools[ONESIDE_HORDE_TEAM1].SelectedGroups.end(); ++itr)
                (*itr)->Team=HORDE;

            if(!bOneSideHordeTeam2)
                bOneSideHordeTeam1 = false;
        }
        if(!bOneSideHordeTeam1)
        {
            // check for one sided ally
            bOneSideAllyTeam1 = BuildSelectionPool(bgTypeId, queue_id,MaxPlayersPerTeam,MaxPlayersPerTeam,ONESIDE_ALLIANCE_TEAM1,arenatype, isRated, arenaMinRating, arenaMaxRating, discardTime);
            if(bOneSideAllyTeam1)
            {
                // one team has been selected, find out if other can be selected too
                std::list<GroupQueueInfo* >::iterator itr;
                // temporarily change the team side to enable building the next pool excluding the already selected groups
                for(itr = m_SelectionPools[ONESIDE_ALLIANCE_TEAM1].SelectedGroups.begin(); itr != m_SelectionPools[ONESIDE_ALLIANCE_TEAM1].SelectedGroups.end(); ++itr)
                    (*itr)->Team=HORDE;

                bOneSideAllyTeam2 = BuildSelectionPool(bgTypeId, queue_id,MaxPlayersPerTeam,MaxPlayersPerTeam,ONESIDE_ALLIANCE_TEAM2,arenatype, isRated, arenaMinRating, arenaMaxRating, discardTime,(*(m_SelectionPools[ONESIDE_ALLIANCE_TEAM1].SelectedGroups.begin()))->ArenaTeamId);

                // change back the team to ally
                for(itr = m_SelectionPools[ONESIDE_ALLIANCE_TEAM1].SelectedGroups.begin(); itr != m_SelectionPools[ONESIDE_ALLIANCE_TEAM1].SelectedGroups.end(); ++itr)
                    (*itr)->Team=ALLIANCE;
            }

            if(!bOneSideAllyTeam2)
                bOneSideAllyTeam1 = false;
        }
        // 1-sided BuildSelectionPool() will work, because the MinPlayersPerTeam == MaxPlayersPerTeam in every arena!!!!
        if( (bOneSideHordeTeam1 && bOneSideHordeTeam2) ||
            (bOneSideAllyTeam1 && bOneSideAllyTeam2) )
        {
            // which side has enough players?
            uint32 side = 0;
            SelectionPoolBuildMode mode1, mode2;
            // find out what pools are we using
            if(bOneSideAllyTeam1 && bOneSideAllyTeam2)
            {
                side = ALLIANCE;
                mode1 = ONESIDE_ALLIANCE_TEAM1;
                mode2 = ONESIDE_ALLIANCE_TEAM2;
            }
            else
            {
                side = HORDE;
                mode1 = ONESIDE_HORDE_TEAM1;
                mode2 = ONESIDE_HORDE_TEAM2;
            }

            // create random arena
            uint8 arenas[] = {BATTLEGROUND_NA, BATTLEGROUND_BE, BATTLEGROUND_RL};
            uint32 arena_num = urand(0,2);
            Battleground* bg2 = nullptr;
            if( !(bg2 = sBattlegroundMgr->CreateNewBattleground(arenas[arena_num%3])) &&
                !(bg2 = sBattlegroundMgr->CreateNewBattleground(arenas[(arena_num+1)%3])) &&
                !(bg2 = sBattlegroundMgr->CreateNewBattleground(arenas[(arena_num+2)%3])) )
            {
                TC_LOG_ERROR("bg.battleground","Could not create arena.");
                return;
            }

            // init stats
            if(sBattlegroundMgr->IsArenaTesting())
            {
                bg2->SetMaxPlayersPerTeam(1);
                bg2->SetMaxPlayers(2);
            }
            else
            {
                switch(arenatype)
                {
                case ARENA_TYPE_2v2:
                    bg2->SetMaxPlayersPerTeam(2);
                    bg2->SetMaxPlayers(4);
                    break;
                case ARENA_TYPE_3v3:
                    bg2->SetMaxPlayersPerTeam(3);
                    bg2->SetMaxPlayers(6);
                    break;
                case ARENA_TYPE_5v5:
                    bg2->SetMaxPlayersPerTeam(5);
                    bg2->SetMaxPlayers(10);
                    break;
                default:
                    break;
                }
            }

            bg2->SetRated(isRated);

            // assigned team of the other group
            uint32 other_side;
            if(side == ALLIANCE)
                other_side = HORDE;
            else
                other_side = ALLIANCE;

            // start the joining of the bg
            bg2->SetStatus(STATUS_WAIT_JOIN);
            bg2->SetQueueType(queue_id);
            // initialize arena / rating info
            bg2->SetArenaType(arenatype);

            std::list<GroupQueueInfo* >::iterator itr;

            // invite players from the first group as horde players (actually green team)
            for(itr = m_SelectionPools[mode1].SelectedGroups.begin(); itr != m_SelectionPools[mode1].SelectedGroups.end(); ++itr)
            {
                InviteGroupToBG((*itr),bg2,HORDE);
            }

            // invite players from the second group as ally players (actually gold team)
            for(itr = m_SelectionPools[mode2].SelectedGroups.begin(); itr != m_SelectionPools[mode2].SelectedGroups.end(); ++itr)
            {
                InviteGroupToBG((*itr),bg2,ALLIANCE);
            }

            if (isRated)
            {
                auto itr_alliance = m_SelectionPools[mode1].SelectedGroups.begin();
                auto itr_horde = m_SelectionPools[mode2].SelectedGroups.begin();
                (*itr_alliance)->OpponentsTeamRating = (*itr_horde)->ArenaTeamRating;
                (*itr_horde)->OpponentsTeamRating = (*itr_alliance)->ArenaTeamRating;
            }

            bg2->StartBattleground();
        }
    }
}

/*********************************************************/
/***            BATTLEGROUND QUEUE EVENTS              ***/
/*********************************************************/

bool BGQueueInviteEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = ObjectAccessor::FindConnectedPlayer( m_PlayerGuid );

    // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
    if (!plr)
        return true;

    // Player can be in another BG queue and must be removed in normal way in any case
    // // player is already in battleground ... do nothing (battleground queue status is deleted when player is teleported to BG)
    // if (plr->GetBattlegroundId() > 0)
    //    return true;

    Battleground* bg = sBattlegroundMgr->GetBattleground(m_BgInstanceGUID);
    if (!bg)
        return true;

    uint32 queueSlot = plr->GetBattlegroundQueueIndex(bg->GetTypeID());
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue
    {
        uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
        uint32 queueSlot = plr->GetBattlegroundQueueIndex(bgQueueTypeId);
        if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
        {
            // check if player is invited to this bg ... this check must be here, because when player leaves queue and joins another, it would cause a problems
            BattlegroundQueue::QueuedPlayersMap const& qpMap = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[plr->GetBattlegroundQueueIdFromLevel()];
            auto qItr = qpMap.find(m_PlayerGuid);
            if (qItr != qpMap.end() && qItr->second.GroupInfo->IsInvitedToBGInstanceGUID == m_BgInstanceGUID)
            {
                WorldPacket data;
                sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, qItr->second.GroupInfo->Team, queueSlot, STATUS_WAIT_JOIN, INVITE_ACCEPT_WAIT_TIME/2, 0);
                plr->SendDirectMessage(&data);
            }
        }
    }
    return true;                                            //event will be deleted
}

void BGQueueInviteEvent::Abort(uint64 /*e_time*/)
{
    //this should not be called
    TC_LOG_ERROR("bg.battleground","Battleground invite event ABORTED!");
}

bool BGQueueRemoveEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = ObjectAccessor::FindConnectedPlayer( m_PlayerGuid );
    if (!plr)
        // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
        return true;

    Battleground* bg = sBattlegroundMgr->GetBattleground(m_BgInstanceGUID);
    if (!bg)
        return true;

    uint32 bgQueueTypeId = sBattlegroundMgr->BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
    uint32 queueSlot = plr->GetBattlegroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
    {
        // check if player is invited to this bg ... this check must be here, because when player leaves queue and joins another, it would cause a problems
        auto qMapItr = sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[plr->GetBattlegroundQueueIdFromLevel()].find(m_PlayerGuid);
        if (qMapItr != sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].m_QueuedPlayers[plr->GetBattlegroundQueueIdFromLevel()].end() && qMapItr->second.GroupInfo && qMapItr->second.GroupInfo->IsInvitedToBGInstanceGUID == m_BgInstanceGUID)
        {
            if (qMapItr->second.GroupInfo->IsRated)
            {
                ArenaTeam * at = sObjectMgr->GetArenaTeamById(qMapItr->second.GroupInfo->ArenaTeamId);
                if (at)
                {
                    at->MemberLost(plr, qMapItr->second.GroupInfo->OpponentsTeamRating);
                    at->SaveToDB();
                }
            }
            plr->RemoveBattlegroundQueueId(bgQueueTypeId);
            sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].RemovePlayer(m_PlayerGuid, true);
            sBattlegroundMgr->m_BattlegroundQueues[bgQueueTypeId].BattlegroundQueueUpdate(bgQueueTypeId, bg->GetQueueType());
            WorldPacket data;
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, m_PlayersTeam, queueSlot, STATUS_NONE, 0, 0);
            plr->SendDirectMessage(&data);
        }
    }

    //event will be deleted
    return true;
}

void BGQueueRemoveEvent::Abort(uint64 /*e_time*/)
{
    //this should not be called
    TC_LOG_ERROR("bg.battleground","Battleground remove event ABORTED!");
}
