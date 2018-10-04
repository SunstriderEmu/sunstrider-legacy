
#ifndef __BATTLEGROUNDRL_H
#define __BATTLEGROUNDRL_H

class Battleground;

enum BattlegroundRLObjectTypes
{
    BG_RL_OBJECT_DOOR_1         = 0,
    BG_RL_OBJECT_DOOR_2         = 1,
    BG_RL_OBJECT_BUFF_1         = 2,
    BG_RL_OBJECT_BUFF_2         = 3,
    BG_RL_OBJECT_MAX            = 4
};

enum BattlegroundRLObjects
{
    BG_RL_OBJECT_TYPE_DOOR_1    = 185918,
    BG_RL_OBJECT_TYPE_DOOR_2    = 185917,
    BG_RL_OBJECT_TYPE_BUFF_1    = 184663,
    BG_RL_OBJECT_TYPE_BUFF_2    = 184664
};

class BattlegroundRLScore : public BattlegroundScore
{
    public:
        BattlegroundRLScore() {};
        virtual ~BattlegroundRLScore() {};
        //TODO fix me
};

class BattlegroundRL : public Battleground
{
    friend class BattlegroundMgr;

    public:
        BattlegroundRL();
        ~BattlegroundRL();
        void Update(time_t diff) override;

        /* inherited from BattlegroundClass */
        void AddPlayer(Player *plr) override;

        void RemovePlayer(Player *plr, ObjectGuid guid) override;
        void HandleAreaTrigger(Player *Source, uint32 Trigger) override;
        bool SetupBattleground() override;
        void FillInitialWorldStates(WorldPacket &d) override;
        void HandleKillPlayer(Player* player, Player *killer) override;
        bool HandlePlayerUnderMap(Player * plr) override;
};
#endif

