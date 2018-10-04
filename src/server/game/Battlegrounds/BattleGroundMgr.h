
#ifndef __BATTLEGROUNDMGR_H
#define __BATTLEGROUNDMGR_H

#include "SharedDefines.h"
#include "BattleGround.h"
#include "BattleGroundQueue.h"


struct BattlemasterListEntry;
typedef std::map<uint32, Battleground*> BattlegroundContainer;
typedef std::set<uint32> BattlegroundClientIdsContainer;

typedef std::unordered_map<uint32, BattlegroundTypeId> BattleMastersMap;
typedef std::unordered_map<uint32, std::vector<BattleGroundEventIdx> > CreatureBattleEventIndexesMap;
typedef std::unordered_map<uint32, std::vector<BattleGroundEventIdx> > GameObjectBattleEventIndexesMap;

// handle the queue types and bg types separately to enable joining queue for different sized arenas at the same time

enum BattlegroundMisc
{
    BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY = 86400,    // seconds in a day

    BATTLEGROUND_OBJECTIVE_UPDATE_INTERVAL = 1000
};

struct BattlegroundData
{
    BattlegroundContainer m_Battlegrounds;
    BattlegroundClientIdsContainer m_ClientBattlegroundIds[MAX_BATTLEGROUND_BRACKETS];
    BGFreeSlotQueueContainer BGFreeSlotQueue;
};

struct BattlegroundTemplate
{
    BattlegroundTypeId Id;
    uint16 MinPlayersPerTeam;
    uint16 MaxPlayersPerTeam;
    uint8 MinLevel;
    uint8 MaxLevel;
    Position StartLocation[BG_TEAMS_COUNT];
    float MaxStartDistSq;
    uint8 Weight;
    uint32 ScriptId;
    BattlemasterListEntry const* BattlemasterEntry;

    bool IsArena() const;
};

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
        void BuildPlayerLeftBattlegroundPacket(WorldPacket *data, ObjectGuid guid);
        //fromWhere is LK only
        void BuildBattlegroundListPacket(WorldPacket* data, ObjectGuid guid, Player* plr, BattlegroundTypeId bgTypeId, uint8 fromWhere = 0);
        void BuildGroupJoinedBattlegroundPacket(WorldPacket* data, GroupJoinBattlegroundResult result);
        void BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value);
        //void BuildPvPLogDataPacket(WorldPacket *data, Battleground *bg);
        void BuildBattlegroundStatusPacket(WorldPacket* data, Battleground* bg, uint8 queueSlot, uint8 statusId, uint32 time1, uint32 time2, uint8 arenaType, uint32 arenaFaction);
        void BuildPlaySoundPacket(WorldPacket *data, uint32 soundid);

        /* Player invitation */
        // called from Queue update, or from Addplayer to queue
        //void InvitePlayer(Player* plr, uint32 bgInstanceGUID, uint32 team);

        /* Battlegrounds */
        Battleground* GetBattlegroundThroughClientInstance(uint32 instanceId, BattlegroundTypeId bgTypeId);
        Battleground* GetBattleground(uint32 InstanceID, BattlegroundTypeId bgTypeId);
        Battleground * GetBattlegroundTemplate(BattlegroundTypeId bgTypeId);
        Battleground* CreateNewBattleground(BattlegroundTypeId bgTypeId, PvPDifficultyEntry const* bracketEntry, uint8 arenaType, bool isRated);

        void AddBattleground(Battleground* BG);
        void RemoveBattleground(BattlegroundTypeId bgTypeId, uint32 instanceId);
        void AddToBGFreeSlotQueue(BattlegroundTypeId bgTypeId, Battleground* bg);
        void RemoveFromBGFreeSlotQueue(BattlegroundTypeId bgTypeId, uint32 instanceId);
        BGFreeSlotQueueContainer& GetBGFreeSlotQueueStore(BattlegroundTypeId bgTypeId);

        void LoadBattlegroundTemplates();
        void DeleteAllBattlegrounds();
        void SendToBattleground(Player* player, uint32 InstanceID, BattlegroundTypeId bgTypeId);

        /* Battleground queues */
        BattlegroundQueue& GetBattlegroundQueue(BattlegroundQueueTypeId bgQueueTypeId) { return m_BattlegroundQueues[bgQueueTypeId]; }
        void ScheduleQueueUpdate(uint32 arenaMatchmakerRating, uint8 arenaType, BattlegroundQueueTypeId bgQueueTypeId, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id);
        uint32 GetPrematureFinishTime() const;
        uint32 GetAverageQueueWaitTimeForMaxLevels(BattlegroundQueueTypeId type) const;

        bool ToggleArenaTesting();
        bool ToggleBattleGroundTesting();
        void SetHolidayWeekends(uint32 mask);

        const bool IsArenaTesting() const { return m_ArenaTesting; }
        const bool IsBattleGroundTesting() const { return m_Testing; }

        void SendAreaSpiritHealerQueryOpcode(Player *pl, Battleground *bg, ObjectGuid guid);

        static BattlegroundQueueTypeId BGQueueTypeId(BattlegroundTypeId bgTypeId, uint8 arenaType);
        static BattlegroundTypeId BGTemplateId(BattlegroundQueueTypeId bgQueueTypeId);
        static uint8 BGArenaType(BattlegroundQueueTypeId bgQueueTypeId);

        uint32 GetMaxRatingDifference() const;
        uint32 GetRatingDiscardTimer()  const;

        void LoadBattleEventIndexes();
        const BattleGroundEventIdx GetCreatureEventIndex(uint32 spawnID) const
        {
            CreatureBattleEventIndexesMap::const_iterator itr = m_CreatureBattleEventIndexMap.find(spawnID);
            if (itr != m_CreatureBattleEventIndexMap.end())
                return itr->second[0];
            return m_CreatureBattleEventIndexMap.find(-1)->second[0];
        }
        const BattleGroundEventIdx GetGameObjectEventIndex(uint32 spawnID) const
        {
            GameObjectBattleEventIndexesMap::const_iterator itr = m_GameObjectBattleEventIndexMap.find(spawnID);
            if (itr != m_GameObjectBattleEventIndexMap.end())
                return itr->second[0];
            return m_GameObjectBattleEventIndexMap.find(-1)->second[0];
        }
        // Nostalrius: allow multiple events per creature ... Avoid when possible.
        std::vector<BattleGroundEventIdx> const& GetCreatureEventsVector(uint32 spawnID) const
        {
            CreatureBattleEventIndexesMap::const_iterator itr = m_CreatureBattleEventIndexMap.find(spawnID);
            if (itr != m_CreatureBattleEventIndexMap.end())
                return itr->second;
            return m_CreatureBattleEventIndexMap.find(-1)->second;
        }
        std::vector<BattleGroundEventIdx> const& GetGameObjectEventsVector(uint32 spawnID) const
        {
            GameObjectBattleEventIndexesMap::const_iterator itr = m_GameObjectBattleEventIndexMap.find(spawnID);
            if (itr != m_GameObjectBattleEventIndexMap.end())
                return itr->second;
            return m_GameObjectBattleEventIndexMap.find(-1)->second;
        }

        void InitAutomaticArenaPointDistribution();
        //TC compat
        BattlegroundTypeId GetBattleMasterBG(uint32 entry) const { return sObjectMgr->GetBattleMasterBG(entry); }

    private:

        bool CreateBattleground(BattlegroundTemplate const* bgTemplate);
        uint32 CreateClientVisibleInstanceId(BattlegroundTypeId bgTypeId, BattlegroundBracketId bracket_id);
        static bool IsArenaType(BattlegroundTypeId bgTypeId);
        BattlegroundTypeId GetRandomBG(BattlegroundTypeId id);

        /* Battlegrounds */
        typedef std::map<BattlegroundTypeId, BattlegroundData> BattlegroundDataContainer;
        BattlegroundDataContainer bgDataStore;

        BattlegroundQueue m_BattlegroundQueues[MAX_BATTLEGROUND_QUEUE_TYPES];

        std::mutex m_QueueUpdateSchedulerLock;
        std::vector<uint64> m_QueueUpdateScheduler;
        uint32 m_NextRatingDiscardUpdate;
        uint32 m_NextRatedArenaUpdate;
        //bool   m_AutoDistributePoints;
        uint64 m_NextAutoDistributionTime;
        uint32 m_AutoDistributionTimeChecker;
        uint32 m_UpdateTimer;
        bool   m_ArenaTesting;
        bool   m_Testing;
        CreatureBattleEventIndexesMap m_CreatureBattleEventIndexMap;
        GameObjectBattleEventIndexesMap m_GameObjectBattleEventIndexMap;

        BattlegroundTemplate const* GetBattlegroundTemplateByTypeId(BattlegroundTypeId id)
        {
            BattlegroundTemplateMap::const_iterator itr = _battlegroundTemplates.find(id);
            if (itr != _battlegroundTemplates.end())
                return &itr->second;
            return nullptr;
        }

        BattlegroundTemplate const* GetBattlegroundTemplateByMapId(uint32 mapId)
        {
            BattlegroundMapTemplateContainer::const_iterator itr = _battlegroundMapTemplates.find(mapId);
            if (itr != _battlegroundMapTemplates.end())
                return itr->second;
            return nullptr;
        }

        typedef std::map<BattlegroundTypeId, BattlegroundTemplate> BattlegroundTemplateMap;
        typedef std::map<uint32 /*mapId*/, BattlegroundTemplate*> BattlegroundMapTemplateContainer;
        BattlegroundTemplateMap _battlegroundTemplates;
        BattlegroundMapTemplateContainer _battlegroundMapTemplates;
};

#define sBattlegroundMgr BattlegroundMgr::instance()
#endif

