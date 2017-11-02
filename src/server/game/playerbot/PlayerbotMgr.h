#ifndef _PLAYERBOTMGR_H
#define _PLAYERBOTMGR_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include "ObjectGuid.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

typedef map<uint64, Player*> PlayerBotMap;

class TC_GAME_API PlayerbotHolder : public PlayerbotAIBase
{
public:
    PlayerbotHolder();
    virtual ~PlayerbotHolder();

    //Connect bot to server
    Player* AddPlayerBot(uint64 guid, uint32 masterAccountId, bool testingBot = false);
    void LogoutPlayerBot(uint64 guid);
    Player* GetPlayerBot (uint64 guid) const;
    PlayerBotMap::const_iterator GetPlayerBotsBegin() const { return playerBots.begin(); }
    PlayerBotMap::const_iterator GetPlayerBotsEnd()   const { return playerBots.end();   }

    void UpdateAIInternal(uint32 elapsed) override;
    void UpdateSessions(uint32 elapsed);

    void LogoutAllBots();
    void OnBotLogin(Player* const bot, bool testingBot = false);

    list<std::string> HandlePlayerbotCommand(char const* args, Player* master = nullptr);
    std::string ProcessBotCommand(std::string cmd, ObjectGuid guid, bool admin, uint32 masterAccountId, uint32 masterGuildId);
    uint32 GetAccountId(std::string name);

protected:
    virtual void OnBotLoginInternal(Player * const bot) = 0;

protected:
    PlayerBotMap playerBots;
};

class PlayerbotMgr : public PlayerbotHolder
{
public:
    PlayerbotMgr(Player* const master);
    ~PlayerbotMgr() override;

    static bool HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args);
    void HandleMasterIncomingPacket(const WorldPacket& packet);
    void HandleMasterOutgoingPacket(const WorldPacket& packet);
    void HandleCommand(uint32 type, const std::string& text);

    void UpdateAIInternal(uint32 elapsed) override;

    Player* GetMaster() const { return master; };

    void SaveToDB();

protected:
    void OnBotLoginInternal(Player * const bot) override;

private:
    Player* const master;
};

#endif
