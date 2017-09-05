
#ifndef __BATTLEGROUNDQUEUE_H
#define __BATTLEGROUNDQUEUE_H

#include "SharedDefines.h"
#include "BattleGround.h"

//typedef std::map<uint32, BattlegroundQueue*> BattlegroundQueueSet;
typedef std::deque<Battleground*> BGFreeSlotQueueType;

struct GroupQueueInfo;                                      // type predefinition
struct PlayerQueueInfo                                      // stores information for players in queue
{
    uint32  InviteTime;                                     // first invite time
    uint32  LastInviteTime;                                 // last invite time
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    GroupQueueInfo * GroupInfo;                             // pointer to the associated groupqueueinfo
};

struct GroupQueueInfo                                       // stores information about the group in queue (also used when joined as solo!)
{
    std::map<uint64, PlayerQueueInfo*> Players;             // player queue info map
    uint32  Team;                                           // Player team (ALLIANCE/HORDE)
    uint32  BgTypeId;                                       // battleground type id
    bool    IsRated;                                        // rated
    uint8   ArenaType;                                      // 2v2, 3v3, 5v5 or 0 when BG
    uint32  ArenaTeamId;                                    // team id if rated match
    uint32  JoinTime;                                       // time when group was added
    uint32  IsInvitedToBGInstanceGUID;                      // was invited to certain BG
    uint32  ArenaTeamRating;                                // if rated match, inited to the rating of the team
    uint32  OpponentsTeamRating;                            // for rated arena matches
};

class Battleground;
class TC_GAME_API BattlegroundQueue
{
    public:
        BattlegroundQueue();
        ~BattlegroundQueue();

        void BattlegroundQueueUpdate(uint32 bgTypeId, uint32 queue_id, uint8 arenatype = 0, bool isRated = false, uint32 minRating = 0);

        GroupQueueInfo* AddGroup(Player * leader, uint32 BgTypeId, uint8 ArenaType, bool isRated, uint32 ArenaRating, uint32 ArenaTeamId = 0);
        void AddPlayer(Player *plr, GroupQueueInfo *ginfo);
        void RemovePlayer(uint64 guid, bool decreaseInvitedCount);
        void BGEndedRemoveInvites(Battleground * bg);

        typedef std::map<uint64, PlayerQueueInfo> QueuedPlayersMap;
        QueuedPlayersMap m_QueuedPlayers[MAX_BATTLEGROUND_QUEUE_RANGES];

        typedef std::list<GroupQueueInfo*> QueuedGroupsList;
        QueuedGroupsList m_QueuedGroups[MAX_BATTLEGROUND_QUEUE_RANGES];
        
        uint32 GetAvgTime() { return m_avgTime; }
        void AddStatsForAvgTime(uint32 time);

        // class to hold pointers to the groups eligible for a specific selection pool building mode
        class EligibleGroups : public std::list<GroupQueueInfo *>
        {
        public:
            void Init(QueuedGroupsList * source, uint32 BgTypeId, uint32 side, uint32 MaxPlayers, uint8 ArenaType = 0, bool IsRated = false, uint32 MinRating = 0, uint32 MaxRating = 0, uint32 DisregardTime = 0, uint32 excludeTeam = 0);
        };

        EligibleGroups m_EligibleGroups;

        // class to select and invite groups to bg
        class SelectionPool
        {
        public:
            void Init(EligibleGroups * curr);
            void AddGroup(GroupQueueInfo * group);
            void RemoveGroup(GroupQueueInfo * group);
            uint32 GetPlayerCount() const {return PlayerCount;}
            bool Build(uint32 MinPlayers, uint32 MaxPlayers, EligibleGroups::iterator startitr);
        public:
            std::list<GroupQueueInfo *> SelectedGroups;
        private:
            uint32 PlayerCount;
            EligibleGroups * m_CurrEligGroups;
        };

        enum SelectionPoolBuildMode
        {
            NORMAL_ALLIANCE,
            NORMAL_HORDE,
            ONESIDE_ALLIANCE_TEAM1,
            ONESIDE_ALLIANCE_TEAM2,
            ONESIDE_HORDE_TEAM1,
            ONESIDE_HORDE_TEAM2,

            NUM_SELECTION_POOL_TYPES
        };

        SelectionPool m_SelectionPools[NUM_SELECTION_POOL_TYPES];

        bool BuildSelectionPool(uint32 bgTypeId, uint32 queue_id, uint32 MinPlayers, uint32 MaxPlayers, SelectionPoolBuildMode mode, uint8 ArenaType = 0, bool isRated = false, uint32 MinRating = 0, uint32 MaxRating = 0, uint32 DisregardTime = 0, uint32 excludeTeam = 0);

    private:

        std::list<uint32> m_lastTimes;
        uint32 m_avgTime;
        bool InviteGroupToBG(GroupQueueInfo * ginfo, Battleground * bg, uint32 side);
};

/*
    This class is used to invite player to BG again, when minute lasts from his first invitation
    it is capable to solve all possibilities
*/
class BGQueueInviteEvent : public BasicEvent
{
    public:
        BGQueueInviteEvent(uint64 pl_guid, uint32 BgInstanceGUID) : m_PlayerGuid(pl_guid), m_BgInstanceGUID(BgInstanceGUID) {};
        ~BGQueueInviteEvent() override = default;;

        bool Execute(uint64 e_time, uint32 p_time) override;
        void Abort(uint64 e_time) override;
    private:
        uint64 m_PlayerGuid;
        uint32 m_BgInstanceGUID;
};

/*
    This class is used to remove player from BG queue after 2 minutes from first invitation
*/
class BGQueueRemoveEvent : public BasicEvent
{
    public:
        BGQueueRemoveEvent(uint64 pl_guid, uint32 bgInstanceGUID, uint32 playersTeam) : m_PlayerGuid(pl_guid), m_BgInstanceGUID(bgInstanceGUID), m_PlayersTeam(playersTeam) {};
        ~BGQueueRemoveEvent() override = default;;

        bool Execute(uint64 e_time, uint32 p_time) override;
        void Abort(uint64 e_time) override;
    private:
        uint64 m_PlayerGuid;
        uint32 m_BgInstanceGUID;
        uint32 m_PlayersTeam;
};

#endif

