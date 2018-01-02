
#ifndef __BATTLEGROUNDAA_H
#define __BATTLEGROUNDAA_H

class Battleground;

class BattlegroundAAScore : public BattlegroundScore
{
    public:
        BattlegroundAAScore() {};
        virtual ~BattlegroundAAScore() {};
        //TODO fix me
};

class BattlegroundAA : public Battleground
{
    friend class BattlegroundMgr;

    public:
        BattlegroundAA();
        ~BattlegroundAA();
        void Update(time_t diff);

        /* inherited from BattlegroundClass */
        virtual void AddPlayer(Player *plr);
        void RemovePlayer(Player *plr, ObjectGuid guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        bool SetupBattleground();
        void HandleKillPlayer(Player* player, Player *killer);
};
#endif

