#include "IRCMgr.h"
#include "Database/DatabaseEnv.h"

INSTANTIATE_SINGLETON_1(IRCMgr);

IRCMgr::IRCMgr()
{
    sLog.outString("IRCMgr: Initializing...");
    if (!configure()) {
        sLog.outError("IRCMgr: There are errors in your configuration.");
        return;
    }
        
    // Initialize the callbacks
    memset(&_callbacks, 0, sizeof (_callbacks));

    // Set up the callbacks we will use
    _callbacks.event_connect = onIRCConnectEvent;
    _callbacks.event_channel = onIRCChannelEvent;
    
    connect();
    
    sLog.outString("IRCMgr initialized.");
}

IRCMgr::~IRCMgr()
{
}

bool IRCMgr::configure()
{
    QueryResult* res = CharacterDatabase.Query("SELECT `id`, `host`, `port`, `ssl`, `nick` FROM wrchat_servers ORDER BY `id`");
    if (!res) {
        sLog.outError("IRCMgr: No server found, please set IRC.Enabled to 0 in configuration file if you don't want to use the IRC bridge.");
        return false;
    }
    
    uint32 count = 0;
    uint32 id;
    Field* fields = res->Fetch();
    QueryResult* res2 = NULL;
    Field* fields2 = NULL;
    do {
        IRCServer* server = new IRCServer;
        id = fields[0].GetUInt32();
        server->session = NULL;
        server->host = fields[1].GetCppString();
        server->port = fields[2].GetUInt32();
        server->ssl = fields[3].GetBool();
        server->nick = fields[4].GetCppString();
        
        res2 = CharacterDatabase.PQuery("SELECT irc_channel, ingame_channel, channel_type, join_message FROM wrchat_channels WHERE server = %u", id);
        if (!res2) {
            sLog.outError("IRCMgr: Server %u (%s:%u, %susing SSL) has no associated channels in table wrchat_channels.",
                    id, server->host.c_str(), server->port, server->ssl ? "" : "not ");
            continue;
        }
        
        do {
            fields2 = res2->Fetch();

            IRCChan* channel = new IRCChan;
            channel->name = fields2[0].GetCppString();
            channel->joinmsg = fields2[3].GetCppString();
            server->channels.push_back(channel);
        } while (res2->NextRow());
        
        delete res2;
        
        _servers[id] = server;
        count++;
    } while (res->NextRow());
    
    sLog.outString("Loaded %u irc servers.", count);
    
    delete res;
    
    return true;
}

void IRCMgr::connect()
{
    for (IRCServers::iterator itr = _servers.begin(); itr != _servers.end(); itr++) {
        itr->second->session = irc_create_session(&_callbacks);
        if (!itr->second->session) {
            sLog.outError("IRCMgr: Could not create IRC session for server %u (%s:%u, %susing SSL): %s.",
                    itr->first, itr->second->host.c_str(), itr->second->port, (itr->second->ssl ? "" : "not "));
            continue;
        }

        irc_set_ctx(itr->second->session, itr->second);
        irc_option_set(itr->second->session, LIBIRC_OPTION_SSL_NO_VERIFY);
        
        if (irc_connect(itr->second->session, itr->second->host.c_str(), itr->second->port, 0, itr->second->nick.c_str(), itr->second->nick.c_str(), "Windrunner IRC Bridge")) {
            sLog.outError("IRCMgr: Could not connect to server %u (%s:%u, %susing SSL): %s",
                    itr->first, itr->second->host.c_str(), itr->second->port, (itr->second->ssl ? "" : "not "), irc_strerror(irc_errno(itr->second->session)));
        }
    }
}

void IRCMgr::run()
{
    // Start one thread per session
    for (IRCServers::iterator itr = _servers.begin(); itr != _servers.end(); itr++) {
        ACE_Based::Thread th(new IRCSession(itr->second));
    }
    
    // TODO: memleaks and wait()
}

void IRCMgr::onIRCConnectEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    IRCServer* server = (IRCServer*) irc_get_ctx(session);
    for (uint32 i = 0; i < server->channels.size(); i++) {
        irc_cmd_join(session, server->channels[i]->name.c_str(), NULL);
        irc_cmd_msg (session, server->channels[i]->name.c_str(), server->channels[i]->joinmsg.c_str());
    }
}

void IRCMgr::onIRCChannelEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    /*const char* who = (origin ?: "Someone");
    const char* channel = params[0];
    const char* message = (params[1] ?: "?");
    std::string response = who;
    response += ": ";
    response += message;
    irc_cmd_msg (session, channel, response.c_str());*/
}

void IRCMgr::onIngameGuildJoin(uint32 guildId, const char* guildName, const char* origin)
{
    
}