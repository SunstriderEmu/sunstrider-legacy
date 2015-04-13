#ifndef IRCMGR_H
#define    IRCMGR_H

#include <Common.h>
#include <libircclient.h>
#include "Policies/SingletonImp.h"
#include "Log.h"

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include <mutex>
#include "Runnable.h"

class Player;

/**
 * IDEAS:
 * - warn when a player is alone in sunwell
 * - warn when a player has received too many 'report spam' and allow to perform some command
 *      (with some arbitrary number,like .see xxx) to see reported messages
 * - warn when someone fails a warden check
 */

enum ChannelType {
    CHAN_TYPE_CHANNEL_ALLIANCE  = 0,
    CHAN_TYPE_CHANNEL_HORDE     = 1,
    CHAN_TYPE_GUILD             = 2,
    CHAN_TYPE_SPAM_REPORT       = 3
};

enum ChannelFaction {
    CHAN_FACTION_ALLIANCE,
    CHAN_FACTION_HORDE,
};

typedef struct {
    uint32 guildId;
} GuildChannel;

//custom players created channels
typedef struct {
    std::string name;
    ChannelFaction faction;
} PublicChannel;

typedef std::vector<GuildChannel> GuildChannels;

typedef struct {
    std::string name;
    std::string password;
    std::string joinmsg;
    GuildChannels guilds;
    void* server; // Forward declaration isn't working in this case
    bool enabled;
} IRCChan;

typedef std::vector<IRCChan*> IRCChans;

typedef std::multimap<uint32, IRCChan*> GuildToIRCMap;
typedef std::multimap<std::string, IRCChan*> ChannelToIRCMap;

typedef struct {
    irc_session_t* session;
    std::string host;
    uint32 port;
    bool ssl;
    std::string nick;
    IRCChans channels;
} IRCServer;

typedef std::map<uint32, IRCServer*> IRCServers;

class IRCSession : public Runnable  
{
public:
    
    IRCSession(IRCServer* server) : _server(server)
    {
    }
    
    void run();
    
private:
    IRCServer* _server;
};

//Command handler
class IRCHandler : public ChatHandler
{
public:
    explicit IRCHandler() {}

    // overwrite functions
    const char *GetTrinityString(int32 entry) const;
    bool isAvailable(ChatCommand const& cmd) const;
    void SendSysMessage(const char *str);
    std::string const GetName() const override;
    bool needReportToTarget(Player* chr) const;

    int ParseCommands(irc_session_t* session, const char* origin, const char* params);
private:
    //last session & channel. Dirty !
    irc_session_t* ircSession;
    const char* channel;
};

class IRCMgr
{
public:
    static IRCMgr* instance()
    {
        static IRCMgr instance;
        return &instance;
    }

    ~IRCMgr();

    // IRC callbacks
    static void onIRCConnectEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);
    static void onIRCChannelEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);
    static void onIRCPartEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);
    static void onIRCJoinEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);    
    void HandleChatCommand(irc_session_t* session, const char* _channel, const char* params);

    // Ingame callbacks
    void onIngameGuildJoin(uint32 guildId, std::string const& guildName, std::string const& origin);
    void onIngameGuildLeft(uint32 guildId, std::string const& guildName, std::string const& origin);
    void onIngameGuildMessage(uint32 guildId, std::string const& origin, const char* message);
    void onReportSpam(std::string const& spammerName, uint32 spammerGUID);
    void onIngameChannelMessage(ChannelFaction faction, const char* channel, std::string const& origin, const char* message);

    void sendToIRCFromGuild(uint32 guildId, std::string msg);
    void sendToIRCFromChannel(const char* channel, ChannelFaction faction, std::string msg);
    void sendGlobalMsgToIRC(std::string msg);

    void EnableServer(IRCServer* server, bool enable);

    void static ConvertWoWColorsToIRC(std::string& msg);
    
    void startSessions();
    void stopSessions();

    /*
    Try to connect to configured servers, if not already connected.
    */
    void connect();
private:

    IRCMgr();
    bool configure();
        
    irc_callbacks_t _callbacks;
    IRCServers _servers;
    
    GuildToIRCMap _guildsToIRC;
    ChannelToIRCMap _channelToIRC_A; //Alliance channels
    ChannelToIRCMap _channelToIRC_H; //Horde channels
    IRCChans _spamReportChans;

    //console command handler
    IRCHandler* ircChatHandler;

    typedef std::vector<Runnable*> IRCThreadList;
    IRCThreadList sessionThreads;

    std::mutex mtx;
};

#define sIRCMgr IRCMgr::instance()

#endif    /* IRCMGR_H */

