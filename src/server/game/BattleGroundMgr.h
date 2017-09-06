
#ifndef __BATTLEGROUNDMGR_H
#define __BATTLEGROUNDMGR_H

#include "SharedDefines.h"
#include "BattleGround.h"
#include "BattleGroundQueue.h"

//TODO it is not possible to have this structure, because we should have BattlegroundSet for each queue
//so i propose to change this type to array 1..MAX_BATTLEGROUND_TYPE_ID of sets or maps..
typedef std::map<uint32, Battleground*> BattlegroundSet;

// handle the queue types and bg types separately to enable joining queue for different sized arenas at the same time

#define BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY    86400     // seconds in a day

class TC_GAME_API BattlegroundMgr
{
    private:
        BattlegroundMgr();
        ~BattlegroundMgr();
    public:
        static BattlegroundMgr* instance()
        {
            static BattlegroundMgr instance;
            return &instance;
        }

        /* Construction */
        void Update(time_t diff);

        /* Packet Building */
        void BuildPlayerJoinedBattlegroundPacket(WorldPacket *data, Player *plr);
        void BuildPlayerLeftBattlegroundPacket(WorldPacket *data, Player *plr);
        void BuildBattlegroundListPacket(WorldPacket *data, uint64 guid, Player *plr, uint32 bgTypeId);
        void BuildGroupJoinedBattlegroundPacket(WorldPacket *data, uint32 bgTypeId);
        void BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value);
        void BuildPvpLogDataPacket(WorldPacket *data, Battleground *bg);
        void BuildBattlegroundStatusPacket(WorldPacket *data, Battleground *bg, uint32 team, uint8 QueueSlot, uint8 StatusID, uint32 Time1, uint32 Time2, uint32 arenatype = 0, uint8 israted = 0);
        void BuildPlaySoundPacket(WorldPacket *data, uint32 soundid);

        /* Player invitation */
        // called from Queue update, or from Addplayer to queue
        void InvitePlayer(Player* plr, uint32 bgInstanceGUID, uint32 team);

        /* Battlegrounds */
        BattlegroundSet::iterator GetBattlegroundsBegin() { return m_Battlegrounds.begin(); };
        BattlegroundSet::iterator GetBattlegroundsEnd() { return m_Battlegrounds.end(); };

        Battleground* GetBattleground(uint32 ID)
        {
            auto i = m_Battlegrounds.find(ID);
            if(i != m_Battlegrounds.end())
                return i->second;
            else
                return nullptr;
        };

        BattlegroundSet GetBattlegroundByType(uint32 bgTypeId);

        Battleground * GetBattlegroundTemplate(uint32 bgTypeId);
        Battleground * CreateNewBattleground(uint32 bgTypeId);

        uint32 CreateBattleground(uint32 bgTypeId, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, char* BattlegroundName, uint32 MapID, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StartLocO);

        inline void AddBattleground(uint32 ID, Battleground* BG) { m_Battlegrounds[ID] = BG; };
        void RemoveBattleground(uint32 instanceID);

        void CreateInitialBattlegrounds();
        void DeleteAllBattlegrounds();

        void SendToBattleground(Player *pl, uint32 bgTypeId);

        /* Battleground queues */
        //these queues are instantiated when creating BattlegroundMrg
        BattlegroundQueue m_BattlegroundQueues[BATTLEGROUND_QUEUE_TYPES_TOTAL]; // public, because we need to access them in BG handler code

        BGFreeSlotQueueType BGFreeSlotQueue[MAX_BATTLEGROUND_TYPE_ID];

        void SendAreaSpiritHealerQueryOpcode(Player *pl, Battleground *bg, uint64 guid);

        bool IsArenaType(uint32 bgTypeId) const;
        bool IsBattlegroundType(uint32 bgTypeId) const;
        static uint32 BGQueueTypeId(uint32 bgTypeId, uint8 arenaType);
        uint32 BGTemplateId(uint32 bgQueueTypeId) const;
        uint8 BGArenaType(uint32 bgQueueTypeId) const;

        uint32 GetMaxRatingDifference() const {return m_MaxRatingDifference;}
        uint32 GetMaxRatingDifferenceFromTimer(uint32 time) { return (m_MaxRatingDifference + 150 * (time % 30)); }
        uint32 GetRatingDiscardTimer() const {return m_RatingDiscardTimer;}

        void InitAutomaticArenaPointDistribution();
        void DistributeArenaPoints();
        uint32 GetPrematureFinishTime() const {return m_PrematureFinishTimer;}
        bool ToggleArenaTesting();
        bool ToggleBattleGroundTesting();
        const bool IsArenaTesting() const { return m_ArenaTesting; }
        const bool IsBattleGroundTesting() const { return m_BattleGroundTesting; }

        //force updating all queues
        void UpdateAllQueues();

        void SetHolidayWeekends(uint32 mask);
    private:

        /* Battlegrounds */
        BattlegroundSet m_Battlegrounds;
        uint32 m_MaxRatingDifference;
        uint32 m_RatingDiscardTimer;
        uint32 m_NextRatingDiscardUpdate;
        bool   m_AutoDistributePoints;
        uint64 m_NextAutoDistributionTime;
        uint32 m_AutoDistributionTimeChecker;
        uint32 m_PrematureFinishTimer;
        bool   m_ArenaTesting;
        bool   m_BattleGroundTesting;
};

#define sBattlegroundMgr BattlegroundMgr::instance()
#endif

