#ifndef IRCMGR_H
#define	IRCMGR_H

#include <Common.h>
#include <libircclient.h>
#include "Policies/SingletonImp.h"
#include "Log.h"

/**
 * IDEAS:
 * - warn when a player is alone in sunwell
 * - warn when a player has received too many 'report spam' and allow to perform some command
 *      (with some arbitrary number,like .see xxx) to see reported messages
 * - warn when someone fails a warden check
 */

enum ChannelType {
    CHAN_TYPE_PUBLIC_ALLIANCE   = 0,
    CHAN_TYPE_PUBLIC_HORDE      = 1,
    CHAN_TYPE_GUILD             = 2
};

typedef struct {
    uint32 guildId;
} GuildChannel;

typedef std::vector<GuildChannel> GuildChannels;

typedef struct {
    std::string name;
    std::string joinmsg;
    GuildChannels guilds;
    void* server; // Forward declaration isn't working in this case
} IRCChan;

typedef std::vector<IRCChan*> IRCChans;

typedef std::multimap<uint32, IRCChan*> GuildToIRCMap;

typedef struct {
    irc_session_t* session;
    std::string host;
    uint32 port;
    bool ssl;
    std::string nick;
    IRCChans channels;
} IRCServer;

typedef std::map<uint32, IRCServer*> IRCServers;

class IRCSession : public ACE_Based::Runnable
{
public:
    
    IRCSession(IRCServer* server) : _server(server)
    {
    }
    
    void run()
    {
        if (irc_run(_server->session)) {
            sLog.outError("IRCMgr: Could not connect or I/O error with a server (%s:%u, %susing SSL): %s", 
                    _server->host.c_str(), _server->port, (_server->ssl ? "" : "not "), irc_strerror(irc_errno(_server->session)));
            return;
        }
    }
    
private:
    IRCServer* _server;
};

class IRCMgr : public ACE_Based::Runnable
{
public:

    friend class Trinity::Singleton<IRCMgr>;
    friend class Trinity::OperatorNew<IRCMgr>;
    
    virtual ~IRCMgr();

    // IRC callbacks
    static void onIRCConnectEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);
    static void onIRCChannelEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);
    
    // Ingame callbacks
    void onIngameGuildJoin(uint32 guildId, const char* guildName, const char* origin);
    void onIngameGuildLeft(uint32 guildId, const char* guildName, const char* origin);
    void onIngameGuildMessage(uint32 guildId, const char* origin, const char* message);
    
    void run();

private:

    IRCMgr();
    bool configure();
    void connect();
    
    void sendToIRCFromGuild(uint32 guildId, std::string msg);
    
    irc_callbacks_t _callbacks;
    IRCServers _servers;
    
    GuildToIRCMap _guildsToIRC;
};

#define sIRCMgr Trinity::Singleton<IRCMgr>::Instance()

#endif	/* IRCMGR_H */

