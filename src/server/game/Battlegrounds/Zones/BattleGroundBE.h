
#ifndef __BATTLEGROUNDBE_H
#define __BATTLEGROUNDBE_H

class Battleground;

enum BattlegroundBEObjectTypes
{
    BG_BE_OBJECT_DOOR_1         = 0,
    BG_BE_OBJECT_DOOR_2         = 1,
    BG_BE_OBJECT_DOOR_3         = 2,
    BG_BE_OBJECT_DOOR_4         = 3,
    BG_BE_OBJECT_BUFF_1         = 4,
    BG_BE_OBJECT_BUFF_2         = 5,
    BG_BE_OBJECT_MAX            = 6
};

enum BattlegroundBEObjects
{
    BG_BE_OBJECT_TYPE_DOOR_1    = 183971,
    BG_BE_OBJECT_TYPE_DOOR_2    = 183973,
    BG_BE_OBJECT_TYPE_DOOR_3    = 183970,
    BG_BE_OBJECT_TYPE_DOOR_4    = 183972,
    BG_BE_OBJECT_TYPE_BUFF_1    = 184663,
    BG_BE_OBJECT_TYPE_BUFF_2    = 184664
};

class BattlegroundBEScore : public BattlegroundScore
{
    public:
        BattlegroundBEScore() {};
        virtual ~BattlegroundBEScore() {};
};

class BattlegroundBE : public Battleground
{
    friend class BattlegroundMgr;

    public:
        BattlegroundBE();
        ~BattlegroundBE();
        void Update(time_t diff) override;

        /* inherited from BattlegroundClass */
        virtual void AddPlayer(Player *plr) override;

        void RemovePlayer(Player *plr, ObjectGuid guid) override;
        void HandleAreaTrigger(Player *Source, uint32 Trigger) override;
        bool SetupBattleground() override;
        virtual void FillInitialWorldStates(WorldPacket &d) override;
        void HandleKillPlayer(Player* player, Player *killer) override;
        bool HandlePlayerUnderMap(Player * plr) override;

        /* Scorekeeping */
        void UpdatePlayerScore(Player *Source, uint32 type, uint32 value) override;
};
#endif

