
#ifndef __BATTLEGROUND_H
#define __BATTLEGROUND_H

#include "Opcodes.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"
#include "SpectatorAddon.h"
#include "Position.h"

struct PvPDifficultyEntry;
class ChatHandler;

enum BattlegroundSounds
{
    SOUND_HORDE_WINS                = 8454,
    SOUND_ALLIANCE_WINS             = 8455,
    SOUND_BG_START                  = 3439,
    SOUND_BG_START_L70ETC           = 11803,
};

enum BattlegroundQuests
{
    SPELL_WS_QUEST_REWARD           = 43483,
    SPELL_AB_QUEST_REWARD           = 43484,
    SPELL_AV_QUEST_REWARD           = 43475,
    SPELL_AV_QUEST_KILLED_BOSS      = 23658,
    SPELL_EY_QUEST_REWARD           = 43477,
    SPELL_AB_QUEST_REWARD_4_BASES   = 24061,
    SPELL_AB_QUEST_REWARD_5_BASES   = 24064
};

enum BattlegroundMarks
{
    ITEM_AV_MARK_OF_HONOR           = 20560,
    ITEM_WS_MARK_OF_HONOR           = 20558,
    ITEM_AB_MARK_OF_HONOR           = 20559,
    ITEM_EY_MARK_OF_HONOR           = 29024
};

enum BattlegroundMarksCount
{
    ITEM_WINNER_COUNT               = 3,
    ITEM_LOSER_COUNT                = 1
};

enum BattlegroundSpells
{
    SPELL_WAITING_FOR_RESURRECT     = 2584,                 // Waiting to Resurrect
    SPELL_SPIRIT_HEAL_CHANNEL       = 22011,                // Spirit Heal Channel
    SPELL_SPIRIT_HEAL               = 22012,                // Spirit Heal
    SPELL_RESURRECTION_VISUAL       = 24171,                // Resurrection Impact Visual
    SPELL_ARENA_PREPARATION         = 32727,                // use this one, 32728 not correct
    SPELL_ALLIANCE_GOLD_FLAG        = 32724,
    SPELL_ALLIANCE_GREEN_FLAG       = 32725,
    SPELL_HORDE_GOLD_FLAG           = 35774,
    SPELL_HORDE_GREEN_FLAG          = 35775,
    SPELL_PREPARATION               = 44521,                // Preparation
    SPELL_SPIRIT_HEAL_MANA          = 44535,                // Spirit Heal
    SPELL_RECENTLY_DROPPED_FLAG     = 42792,                // Recently Dropped Flag
    SPELL_AURA_PLAYER_IDLE          = 43680,                // When reported idle by other players, removed by PvP Combat
    SPELL_AURA_PLAYER_INACTIVE      = 43681                 // After 1 min Idle
};

enum BattleGroundCreatureSpawnMode
{
    DESPAWN_FORCED   = 0,
    RESPAWN_STOP     = 1,
    RESPAWN_START    = 2,
    RESPAWN_FORCED   = 3
};
#define BG_CREATURE_SPAWN_MODE_COUNT 4

enum BattlegroundTimeIntervals
{
    CHECK_PLAYER_POSITION_INVERVAL  = 1 * SECOND * IN_MILLISECONDS,  // ms
    RESURRECTION_INTERVAL           = 30 * SECOND * IN_MILLISECONDS, // ms
    //REMIND_INTERVAL                 = 30 * SECOND * IN_MILLISECONDS,
    INVITATION_REMIND_TIME          = 20 * SECOND * IN_MILLISECONDS, // ms
    INVITE_ACCEPT_WAIT_TIME         = 80 * SECOND * IN_MILLISECONDS, // ms
    TIME_TO_AUTOREMOVE              = 2 * MINUTE * IN_MILLISECONDS,
    MAX_OFFLINE_TIME                = 2 * MINUTE * SECOND,
    START_DELAY0                    = 2 * MINUTE * IN_MILLISECONDS,
    START_DELAY1                    = 1 * MINUTE * IN_MILLISECONDS,
    START_DELAY2                    = 30 * SECOND * IN_MILLISECONDS,
    START_DELAY3                    = 15 * SECOND * IN_MILLISECONDS, //used only in arena
    RESPAWN_ONE_DAY                 = DAY,                  // secs
    RESPAWN_IMMEDIATELY             = 0,                    // secs
    BUFF_RESPAWN_TIME               = 3 * MINUTE,           // secs
    BG_HONOR_SCORE_TICKS            = 330                   // points
};

// magic event-numbers
#define BG_EVENT_NONE 255
// those generic events should get a high event id
#define BG_EVENT_DOOR 254

struct BattleGroundEventIdx
{
    uint8 event1;
    uint8 event2;
};

enum BattlegroundBuffObjects
{
    BG_OBJECTID_SPEEDBUFF_ENTRY     = 179871,
    BG_OBJECTID_REGENBUFF_ENTRY     = 179904,
    BG_OBJECTID_BERSERKERBUFF_ENTRY = 179905
};

const uint32 Buff_Entries[3] = { BG_OBJECTID_SPEEDBUFF_ENTRY, BG_OBJECTID_REGENBUFF_ENTRY, BG_OBJECTID_BERSERKERBUFF_ENTRY };

enum BattlegroundStatus
{
    STATUS_NONE         = 0,
    STATUS_WAIT_QUEUE   = 1,
    STATUS_WAIT_JOIN    = 2,
    STATUS_IN_PROGRESS  = 3,
    STATUS_WAIT_LEAVE   = 4                                 // custom
};

enum BattlegroundStartingEvents
{
    BG_STARTING_EVENT_NONE  = 0x00,
    BG_STARTING_EVENT_1     = 0x01,
    //BG_STARTING_EVENT_2     = 0x02, //unused
    BG_STARTING_EVENT_3     = 0x04,
    BG_STARTING_EVENT_4     = 0x08,
    BG_STARTING_EVENT_5     = 0x10,
};

struct BattlegroundPlayer
{
    uint32  OfflineRemoveTime;                              // for tracking and removing offline players from queue after 2 minutes
    uint32  TotalOfflineTime;
    uint32  Team;                                           // Player's team
};

struct BattlegroundObjectInfo
{
    BattlegroundObjectInfo() : object(nullptr), timer(0), spellid(0) {}

    GameObject  *object;
    int32       timer;
    uint32      spellid;
};

struct PlayerLogInfo
{
    ObjectGuid::LowType guid;
    std::string ip;
    uint32 heal;
    uint32 damage;
    uint8 kills;
};

enum ScoreType
{
    SCORE_KILLING_BLOWS         = 1,
    SCORE_DEATHS                = 2,
    SCORE_HONORABLE_KILLS       = 3,
    SCORE_BONUS_HONOR           = 4,
    //EY, but in MSG_PVP_LOG_DATA opcode!
    SCORE_DAMAGE_DONE           = 5,
    SCORE_HEALING_DONE          = 6,
    //WS
    SCORE_FLAG_CAPTURES         = 7,
    SCORE_FLAG_RETURNS          = 8,
    //AB
    SCORE_BASES_ASSAULTED       = 9,
    SCORE_BASES_DEFENDED        = 10,
    //AV
    SCORE_GRAVEYARDS_ASSAULTED  = 11,
    SCORE_GRAVEYARDS_DEFENDED   = 12,
    SCORE_TOWERS_ASSAULTED      = 13,
    SCORE_TOWERS_DEFENDED       = 14,
    SCORE_MINES_CAPTURED        = 15,
    SCORE_LEADERS_KILLED        = 16,
    SCORE_SECONDARY_OBJECTIVES  = 17,
    // TODO : implement them

    SCORE_DAMAGE_TAKEN          = 20,
    SCORE_HEALING_TAKEN         = 21,
};

enum ArenaType
{
    ARENA_TYPE_2v2          = 2,
    ARENA_TYPE_3v3          = 3,
    ARENA_TYPE_5v5          = 5
};

enum BattlegroundType
{
    TYPE_BATTLEGROUND     = 3,
    TYPE_ARENA            = 4
};

enum BattlegroundWinner
{
    WINNER_HORDE            = 0,
    WINNER_ALLIANCE         = 1,
    WINNER_NONE             = 2
};

/* re enable this if GroupJoinBattlegroundResult is not usable on BC 
enum BattlegroundJoinError
{
    BG_JOIN_ERR_OK = 0,
    BG_JOIN_ERR_OFFLINE_MEMBER = 1,
    BG_JOIN_ERR_GROUP_TOO_MANY = 2,
    BG_JOIN_ERR_MIXED_FACTION = 3,
    BG_JOIN_ERR_MIXED_LEVELS = 4,
    BG_JOIN_ERR_MIXED_ARENATEAM = 5,
    BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE = 6,
    BG_JOIN_ERR_GROUP_DESERTER = 7,
    BG_JOIN_ERR_ALL_QUEUES_USED = 8,
    BG_JOIN_ERR_GROUP_NOT_ENOUGH = 9
};
*/

class BattlegroundScore
{
    public:
        BattlegroundScore() : KillingBlows(0), HonorableKills(0), Deaths(0), DamageDone(0), HealingDone(0), BonusHonor(0) {};
        virtual ~BattlegroundScore()                        //virtual destructor is used when deleting score from scores map
        = default;;
        uint32 KillingBlows;
        uint32 Deaths;
        uint32 HonorableKills;
        uint32 BonusHonor;
        uint32 DamageDone;
        uint32 HealingDone;
        uint32 DamageTaken;
        uint32 HealingTaken;
};

enum BGHonorMode
{
    BG_NORMAL = 0,
    BG_HOLIDAY,
    BG_HONOR_MODE_NUM
};

typedef std::map<uint64, BattlegroundScore*> BattlegroundScoreMap;

/*
This class is used to:
1. Add player to battleground
2. Remove player from battleground
3. some certain cases, same for all battlegrounds
4. It has properties same for all battlegrounds
*/
class TC_GAME_API Battleground
{
    friend class BattlegroundMgr;

    public:
        /* Construction */
        Battleground();
        /*Battleground(const Battleground& bg);*/
        virtual ~Battleground();
        virtual void Update(time_t diff);                   // must be implemented in BG subclass of BG specific update code, but must in begginning call parent version
        virtual bool SetupBattleground() { return true; }    // must be implemented in BG subclass
        virtual void Reset();                               // resets all common properties for battlegrounds

        // Called after event state changed (event add or remove).
        virtual void HandleCommand(Player* player, ChatHandler* handler, char const* args);
        virtual void OnEventStateChanged(uint8 event1, uint8 event2, bool actived) {}
        virtual void StartingEventCloseDoors() {}
        virtual void StartingEventOpenDoors() {}
        virtual void StartingEventDespawnDoors();

        /* Battleground */
        // Get methods:
        std::string const& GetName() const    { return m_Name; }
        BattlegroundTypeId GetTypeID() const  { return m_TypeID; }
        BattlegroundBracketId GetBracketId() const { return m_BracketId; }
        uint32 GetInstanceID() const          { return m_InstanceID; }
        uint32 GetStatus() const              { return m_Status; }
        uint32 GetClientInstanceID() const    { return m_ClientInstanceID; }
        uint32 GetLastResurrectTime() const   { return m_LastResurrectTime; }
        uint32 GetMaxPlayers() const          { return m_MaxPlayers; }
        uint32 GetMinPlayers() const          { return m_MinPlayers; }

        uint32 GetMinLevel() const            { return m_LevelMin; }
        uint32 GetMaxLevel() const            { return m_LevelMax; }

        uint32 GetMaxPlayersPerTeam() const   { return m_MaxPlayersPerTeam; }
        uint32 GetMinPlayersPerTeam() const   { return m_MinPlayersPerTeam; }

        int GetStartDelayTime() const         { return m_StartDelayTime; }
        uint32 GetTimeLimit() const           { return m_timeLimit; }
        time_t GetStartTimestamp() const      { return time(nullptr) - GetStartTime(); }
        uint8 GetArenaType() const            { return m_ArenaType; }
        uint8 GetWinner() const               { return m_Winner; }
        uint32 GetScriptId() const            { return ScriptId; }
        uint32 GetBattlemasterEntry() const;
        uint32 GetStartTime() const           { return m_StartTime; }
        uint32 GetEndTime() const             { return m_EndTime; }

        // Set methods:
        void SetName(std::string Name)        { m_Name = Name; }
        void SetTypeID(BattlegroundTypeId TypeID) { m_TypeID = TypeID; }
        //here we can count minlevel and maxlevel for players
        void SetBracket(PvPDifficultyEntry const* bracketEntry);
        void SetInstanceID(uint32 InstanceID) { m_InstanceID = InstanceID; }
        void SetStatus(BattlegroundStatus Status);
        void SetClientInstanceID(uint32 InstanceID) { m_ClientInstanceID = InstanceID; }
        void SetStartTime(uint32 Time)      { m_StartTime = Time; }
        void SetEndTime(uint32 Time)        { m_EndTime = Time; }
        void SetLastResurrectTime(uint32 Time) { m_LastResurrectTime = Time; }
        void SetMaxPlayers(uint32 MaxPlayers) { m_MaxPlayers = MaxPlayers; }
        void SetMinPlayers(uint32 MinPlayers) { m_MinPlayers = MinPlayers; }
        void SetLevelRange(uint32 min, uint32 max) { m_LevelMin = min; m_LevelMax = max; }
        void SetRated(bool state)           { m_IsRated = state; }
        void SetArenaType(uint8 type)       { m_ArenaType = type; }
        void SetArenaorBGType(bool _isArena) { m_IsArena = _isArena; }
        void SetWinner(uint8 winner)        { m_Winner = winner; }
        void SetScriptId(uint32 scriptId)   { ScriptId = scriptId; }

        //Set a time limit before closing
        void SetTimeLimit(uint32 limit)    { m_timeLimit = limit; }
        //Decrease timer before gates opening
        void ModifyStartDelayTime(int diff) { m_StartDelayTime -= diff; }
        //Set timer before gates opening
        void SetStartDelayTime(int Time);

        void SetMaxPlayersPerTeam(uint32 MaxPlayers) { m_MaxPlayersPerTeam = MaxPlayers; }
        void SetMinPlayersPerTeam(uint32 MinPlayers) { m_MinPlayersPerTeam = MinPlayers; }

        void AddToBGFreeSlotQueue();                        //this queue will be useful when more battlegrounds instances will be available
        void RemoveFromBGFreeSlotQueue();                   //this method could delete whole BG instance, if another free is available

        void DecreaseInvitedCount(uint32 team)      { (team == ALLIANCE) ? --m_InvitedAlliance : --m_InvitedHorde; }
        void IncreaseInvitedCount(uint32 team)      { (team == ALLIANCE) ? ++m_InvitedAlliance : ++m_InvitedHorde; }
        void PlayerInvitedInRatedArena(Player* player, uint32 team);
        uint32 GetInvitedCount(uint32 team) const
        {
            if( team == ALLIANCE )
                return m_InvitedAlliance;
            else
                return m_InvitedHorde;
        }
        bool HasFreeSlots() const;
        uint32 GetFreeSlotsForTeam(uint32 Team) const;

        bool IsArena() const        { return m_IsArena; }
        bool isBattleground() const { return !m_IsArena; }
        bool isRated() const        { return m_IsRated; }

        typedef std::map<ObjectGuid, BattlegroundPlayer> BattlegroundPlayerMap;
        BattlegroundPlayerMap const& GetPlayers() const { return m_Players; }
        uint32 GetPlayersSize() const { return m_Players.size(); }

        std::map<ObjectGuid, BattlegroundScore*>::const_iterator GetPlayerScoresBegin() const { return PlayerScores.begin(); }
        std::map<ObjectGuid, BattlegroundScore*>::const_iterator GetPlayerScoresEnd() const { return PlayerScores.end(); }
        uint32 GetPlayerScoresSize() const { return PlayerScores.size(); }

        uint32 GetReviveQueueSize() const { return m_ReviveQueue.size(); }

        void AddPlayerToResurrectQueue(ObjectGuid npc_guid, ObjectGuid player_guid);
        void RemovePlayerFromResurrectQueue(ObjectGuid player_guid);

        void StartBattleground();

        GameObject* GetBGObject(uint32 type, bool logError = true);
        Creature* GetBGCreature(uint32 type, bool logError = true);
        /* Location */
        void SetMapId(uint32 MapID) { m_MapId = MapID; }
        uint32 GetMapId() const { return m_MapId; }

		// Map pointers
		void SetBgMap(BattlegroundMap* map) { m_Map = map; }
		BattlegroundMap* GetBgMap() const { ASSERT(m_Map); return m_Map; }
		BattlegroundMap* FindBgMap() const { return m_Map; }

        void SetTeamStartPosition(TeamId TeamID, Position const& pos);
        Position const* GetTeamStartPosition(TeamId teamId) const;

        void SetStartMaxDist(float startMaxDist) { m_StartMaxDist = startMaxDist; }
        float GetStartMaxDist() const { return m_StartMaxDist; }

        /* Packet Transfer */
        // method that should fill worldpacket with actual world states (not yet implemented for all battlegrounds!)
        virtual void FillInitialWorldStates(WorldPacket& /*data*/) {}
        void SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender = nullptr, bool self = true);
        void SendPacketToAll(WorldPacket *packet);
        void PlaySoundToTeam(uint32 SoundID, uint32 TeamID);
        void PlaySoundToAll(uint32 SoundID);

        template<class Do>
        void BroadcastWorker(Do& _do);

        void CastSpellOnTeam(uint32 SpellID, uint32 TeamID);
        void RewardHonorToTeam(uint32 Honor, uint32 TeamID);
        void RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID);
        void RewardMark(Player *plr,uint32 count);
        void SendRewardMarkByMail(Player *plr,uint32 mark, uint32 count);
        void RewardQuest(Player *plr);
        void UpdateWorldState(uint32 Field, uint32 Value);
        void UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player *Source);
        void EndBattleground(uint32 winner);
        void BlockMovement(Player *plr);

        void SendWarningToAll(int32 entry, ...);
        void SendMessageToAll(int32 entry, ChatMsg type, Player const* source = nullptr);
        void PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...);

        // specialized version with 2 string id args
        void SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 strId1 = 0, int32 strId2 = 0);

        void SendMessageToAll(char const* text);
        void SendMessageToAll(int32 entry);

        /* Raid Group */
		Group *GetBgRaid(uint32 TeamID) const;
		void SetBgRaid(uint32 TeamID, Group *bg_raid);

        virtual void UpdatePlayerScore(Player *Source, uint32 type, uint32 value);
        void BuildPvPLogDataPacket(WorldPacket& data);

        static TeamId GetTeamIndexByTeamId(uint32 Team) { return Team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
        uint32 GetPlayersCountByTeam(uint32 Team) const { return m_PlayersCount[GetTeamIndexByTeamId(Team)]; }
        uint32 GetAlivePlayersCountByTeam(uint32 Team) const;   // used in arenas to correctly handle death in spirit of redemption / last stand etc. (killer = killed) cases
        void UpdatePlayersCountByTeam(uint32 Team, bool remove)
        {
            if(remove)
                --m_PlayersCount[GetTeamIndexByTeamId(Team)];
            else
                ++m_PlayersCount[GetTeamIndexByTeamId(Team)];
        }

        uint32 GetArenaTeamIdForIndex(uint32 index) { return _arenaTeamIds[index]; }

        // used for rated arena battles
        void SetArenaTeamIdForTeam(uint32 Team, uint32 ArenaTeamId) { _arenaTeamIds[GetTeamIndexByTeamId(Team)] = ArenaTeamId; }
        uint32 GetArenaTeamIdForTeam(uint32 Team) const { return _arenaTeamIds[GetTeamIndexByTeamId(Team)]; }
        void SetArenaTeamRatingChangeForTeam(uint32 Team, int32 RatingChange) { m_ArenaTeamRatingChanges[GetTeamIndexByTeamId(Team)] = RatingChange; }
        int32 GetArenaTeamRatingChangeForTeam(uint32 Team) const { return m_ArenaTeamRatingChanges[GetTeamIndexByTeamId(Team)]; }
        void SetArenaMatchmakerRating(uint32 Team, uint32 MMR) { m_ArenaTeamMMR[GetTeamIndexByTeamId(Team)] = MMR; }
        uint32 GetArenaMatchmakerRating(uint32 Team) const { return m_ArenaTeamMMR[GetTeamIndexByTeamId(Team)]; }

        /* Triggers handle */
        // must be implemented in BG subclass
        virtual void HandleAreaTrigger(Player* /*Source*/, uint32 /*Trigger*/) {}
        // must be implemented in BG subclass if need AND call base class generic code
        virtual void HandleKillPlayer(Player *player, Player *killer);
        virtual void HandleKillUnit(Creature* /*unit*/, Player* /*killer*/);

        /* Battleground events */
        /* these functions will return true event is possible, but false if player is bugger */
        virtual void EventPlayerDroppedFlag(Player* /*player*/) {}
        virtual void EventPlayerClickedOnFlag(Player* /*player*/, GameObject* /*target_obj*/) {}
        virtual void EventPlayerCapturedFlag(Player* /*player*/, uint32 /* BgObjectType */) {}
        void EventPlayerLoggedIn(Player* player);
        void EventPlayerLoggedOut(Player* player);

        /* Death related */
        virtual WorldSafeLocsEntry const* GetClosestGraveYard(float /*x*/, float /*y*/, float /*z*/, uint32 /*team*/)  { return nullptr; }

        virtual void AddPlayer(Player *plr);                // must be implemented in BG subclass

        void AddOrSetPlayerToCorrectBgGroup(Player* player, uint32 team);

        virtual void RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket);
                                                            // can be extended in in BG subclass
                                                            /* event related */
                                                            // called when a creature gets added to map (NOTE: only triggered if
                                                            // a player activates the cell of the creature)
        void OnObjectDBLoad(Creature* creature);
        //Returns true if obj was added to map
        bool OnObjectDBLoad(GameObject* obj);
        // (de-)spawns creatures and gameobjects from an event
        void SpawnEvent(uint8 event1, uint8 event2, bool spawn, bool forced_despawn, uint32 delay = 0, Player* invoker = nullptr);
        void SetSpawnEventMode(uint8 event1, uint8 event2, BattleGroundCreatureSpawnMode mode);
        bool IsActiveEvent(uint8 event1, uint8 event2)
        {
            if (m_ActiveEvents.find(event1) == m_ActiveEvents.end())
                return false;
            return m_ActiveEvents[event1] == event2;
        }
        void ActivateEventWithoutSpawn(uint8 event1, uint8 event2)
        {
            m_ActiveEvents[event1] = event2;
        }
        ObjectGuid GetSingleCreatureGuid(uint8 event1, uint8 event2);

        void OpenDoorEvent(uint8 event1, uint8 event2 = 0);
        bool IsDoor(uint8 event1, uint8 event2);

        void HandleTriggerBuff(ObjectGuid const& go_guid);
        void SetHoliday(bool is_holiday);

        // TODO: make this protected:
        GuidVector BgObjects;
        GuidVector BgCreatures;
        //Return true if object was added to map
        bool SpawnBGObject(GameObject* object, uint32 respawntime, Player* invoker = nullptr);
        bool SpawnBGObject(ObjectGuid guid, uint32 respawntime, Player* invoker = nullptr);
        bool SpawnBGObject(uint32 type, uint32 respawntime, Player* invoker = nullptr); //for compat
        bool AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime = 0, bool inactive = false);
        void SpawnBGCreature(ObjectGuid guid, BattleGroundCreatureSpawnMode mode) = delete;
        void SpawnBGCreature(uint32 spawnID, BattleGroundCreatureSpawnMode mode);
        void SpawnBGCreature(Creature* c, BattleGroundCreatureSpawnMode mode);
        Creature* AddCreature(uint32 entry, uint32 type, float x, float y, float z, float o, uint32 respawntime = 0);
        bool DelCreature(uint32 type);
        bool DelObject(uint32 type);
        bool AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team);
        int32 GetObjectType(ObjectGuid const& guid);

        void DoorOpen(GameObject* gob);
        void DoorOpen(ObjectGuid guid);
        void DoorOpen(uint32 type); //old compat
        void DoorClose(ObjectGuid guid);
        void DoorClose(uint32 type);  //old compat
        static const char* GetTrinityString(int32 entry);

        // since arenas can be AvA or Hvh, we have to get the "temporary" team of a player
        uint32 GetPlayerTeam(ObjectGuid guid);
        bool IsPlayerInBattleground(ObjectGuid guid);

        bool ToBeDeleted() const { return m_SetDeleteThis; }
        void SetDeleteThis() {m_SetDeleteThis = true;}

        typedef std::set<ObjectGuid> SpectatorList;
        void AddSpectator (ObjectGuid playerGuid) {m_Spectators.insert(playerGuid); }
        void onAddSpectator (Player *spectator);
        void RemoveSpectator(ObjectGuid playerGuid) { m_Spectators.erase(playerGuid); }
        bool HaveSpectators() { return (m_Spectators.size() > 0); }
        void SendSpectateAddonsMsg(SpectatorAddonMsg msg);
        bool isSpectator(ObjectGuid guid);
        bool canEnterSpectator(Player *spectator);

        virtual ObjectGuid GetFlagPickerGUID(int32 /*team*/ = -1) const { return ObjectGuid::Empty; }
        virtual void SetDroppedFlagGUID(ObjectGuid /*guid*/, int32 /*team*/ = -1) { }
        virtual void HandleQuestComplete(uint32 /*questid*/, Player* /*player*/) { }
        virtual bool CanActivateGO(int32 /*entry*/, uint32 /*team*/) const { return true; }
        virtual bool IsSpellAllowed(uint32 /*spellId*/, Player const* /*player*/) const { return true; }

        // because BattleGrounds with different types and same level range has different m_BracketId
        uint8 GetUniqueBracketId() const;

    protected:
        //this method is called, when BG cannot spawn its own spirit guide, or something is wrong, It correctly ends Battleground
        void EndNow();
        void PlayerAddedToBGCheckIfBGIsRunning(Player* player);

        void _ProcessOfflineQueue(uint32 diff);
        void _CheckSafePositions(uint32 diff);
        void _ProcessLeave(uint32 diff);

        /* Scorekeeping */
        std::map<ObjectGuid, BattlegroundScore*>    PlayerScores; // Player scores
        // must be implemented in BG subclass
        virtual void RemovePlayer(Player * /*player*/, ObjectGuid /*guid*/) {}

        /* Player lists, those need to be accessible by inherited classes */
        BattlegroundPlayerMap  m_Players;
                                                            // Spirit Guide guid + Player list GUIDS
        std::map<ObjectGuid, std::vector<ObjectGuid> >  m_ReviveQueue;

        /*
        this is important variable used for invitation messages
        */
        uint8 m_Events;

        bool   m_BuffChange;
        uint32 m_score[2];                    //array that keeps general team scores, used to determine who gets most marks when bg ends prematurely

        BGHonorMode m_HonorMode;

        // this must be filled first in BattleGroundXY::Reset().. else
        // creatures will get added wrong
        // door-events are automaticly added - but _ALL_ other must be in this vector
        std::map<uint8, uint8> m_ActiveEvents;

    private:
        /* Battleground */
        BattlegroundTypeId m_TypeID;                        //Battleground type
        uint32 m_InstanceID;                                //Battleground Instance's GUID!
        uint32 m_Status;
        uint32 m_ClientInstanceID;                          // the instance-id which is sent to the client and without any other internal use
        uint32 m_StartTime;                                 //elapsed time since the very beginning
        int32  m_EndTime;                                   // it is set to 120000 when bg is ending and it decreases itself
        uint32 m_ValidStartPositionTimer;
        uint32 m_LastResurrectTime;
        BattlegroundBracketId m_BracketId;
        uint8  m_ArenaType;                                 // 2=2v2, 3=3v3, 5=5v5
        bool   m_InBGFreeSlotQueue;                         // used to make sure that BG is only once inserted into the BattlegroundMgr.BGFreeSlotQueue[bgTypeId] deque
        bool   m_SetDeleteThis;                             // used for safe deletion of the bg after end / all players leave
        // this variable is not used .... it can be found in many other ways... but to store it in BG object instance is useless
        //uint8  m_BattlegroundType;                        // 3=BG, 4=arena
        //instead of uint8 (in previous line) is bool used
        bool   m_IsArena;
        uint8  m_Winner;                                    // 0=alliance, 1=horde, 2=none
        int32  m_StartDelayTime;
        bool   m_IsRated;                                   // is this battle rated?
        bool   m_PrematureCountDown;
        uint32 m_PrematureCountDownTimer;
        uint32 m_timeLimit;
        std::string m_Name;
                    

        /* Player lists */
        std::vector<ObjectGuid> m_ResurrectQueue;               // Player GUID
        GuidDeque m_OfflineQueue;                           // Player GUID, contain all disconected players not yet kicked out

        /* Invited counters are useful for player invitation to BG - do not allow, if BG is started to one faction to have 2 more players than another faction */
        /* Invited counters will be changed only when removing already invited player from queue, removing player from battleground and inviting player to BG */
        /* Invited players counters*/
        uint32 m_InvitedAlliance;
        uint32 m_InvitedHorde;

        /* Raid Group */
        Group *m_BgRaids[2];                                // 0 - alliance, 1 - horde

        /* Players count by team */
        uint32 m_PlayersCount[2];

        /* Arena team ids by team */
        uint32 _arenaTeamIds[2];

        int32 m_ArenaTeamRatingChanges[2];

        uint32 m_ArenaTeamMMR[BG_TEAMS_COUNT];

        /* Limits */
        uint32 m_LevelMin;
        uint32 m_LevelMax;
        uint32 m_MaxPlayersPerTeam;
        uint32 m_MaxPlayers;
        uint32 m_MinPlayersPerTeam;
        uint32 m_MinPlayers;

        /* Location */
        uint32 m_MapId;
		BattlegroundMap* m_Map;
        Position StartPosition[BG_TEAMS_COUNT];
        float m_StartMaxDist;
        uint32 ScriptId;
        
        std::map<ObjectGuid, PlayerLogInfo*> m_team1LogInfo, m_team2LogInfo;

        struct EventObjects
        {
            GuidVector gameobjects;
            GuidVector creatures;
        };

        // cause we create it dynamicly i use a map - to avoid resizing when
        // using vector - also it contains 2*events concatenated with PAIR32
        // this is needed to avoid overhead of a 2dimensional std::map
        std::map<uint32, EventObjects> m_EventObjects;

        SpectatorList m_Spectators;
};
#endif

