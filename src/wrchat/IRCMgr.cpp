#include "IRCMgr.h"
#include "Database/DatabaseEnv.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "World.h"

INSTANTIATE_SINGLETON_1(IRCMgr);

IRCMgr::IRCMgr()
{
    _guildsToIRC.clear();
    
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
    
    ircChatHandler = new IRCHandler();

    sLog.outString("IRCMgr initialized.");
}

IRCMgr::~IRCMgr()
{
}

#ifdef __gnu_linux__

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
        
        res2 = CharacterDatabase.PQuery("SELECT irc_channel, password, ingame_channel, channel_type, join_message FROM wrchat_channels WHERE server = %u", id);
        if (!res2) {
            sLog.outError("IRCMgr: Server %u (%s:%u, %susing SSL) has no associated channels in table wrchat_channels.",
                    id, server->host.c_str(), server->port, server->ssl ? "" : "not ");
            continue;
        }
        
        do {
            fields2 = res2->Fetch();

            IRCChan* channel = new IRCChan;
            channel->name = fields2[0].GetCppString();
            channel->password = fields2[1].GetCppString();
            channel->joinmsg = fields2[4].GetCppString();
            channel->server = server;
            
            uint32 type = fields2[3].GetUInt32();
            switch (type) {
            case CHAN_TYPE_PUBLIC_ALLIANCE:
            case CHAN_TYPE_PUBLIC_HORDE:
                ChannelChannel cc;
                cc.name = fields2[2].GetString();
                cc.type = (uint8)type;

                _channelToIRC.insert(std::make_pair(cc.name, channel));
                break;
            case CHAN_TYPE_GUILD:
            {
                GuildChannel gc;
                uint32 guildId = atoi(fields2[2].GetString());
                gc.guildId = guildId;
                channel->guilds.push_back(gc);
                
                _guildsToIRC.insert(std::make_pair(guildId, channel));
                
                break;
            }
            case CHAN_TYPE_SPAM_REPORT:
                _spamReportChans.push_back(channel);
                break;
            default:
                sLog.outError("IRCMgr: Invalid channel type %u.", type);
            }
            
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
    ACE_Based::Thread* lastSpawned;
    for (IRCServers::iterator itr = _servers.begin(); itr != _servers.end(); itr++) {
        ACE_Based::Thread th(new IRCSession(itr->second));
        lastSpawned = &th;
    }
    
    lastSpawned->wait();
    // TODO: memleaks
}

void IRCMgr::onIRCConnectEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    IRCServer* server = (IRCServer*) irc_get_ctx(session);
    for (uint32 i = 0; i < server->channels.size(); i++) {
        irc_cmd_join(session, server->channels[i]->name.c_str(), 
                (server->channels[i]->password != "" ? server->channels[i]->password.c_str() : NULL));
        irc_cmd_msg(session, server->channels[i]->name.c_str(), server->channels[i]->joinmsg.c_str());
    }
}

void IRCMgr::onIRCChannelEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    if (!params[1] || !origin) // No message sent
        return;
    
    sIRCMgr.HandleChatCommand(session,params[0],params[1]);
    
    IRCServer* server = (IRCServer*) irc_get_ctx(session);
    std::string msg = "[";
    msg += origin;
    msg = msg.substr(0, msg.find_first_of("!"));
    msg += "] ";
    msg += params[1];
    for (uint32 i = 0; i < server->channels.size(); i++) {
        IRCChan* chan = server->channels[i];
        if (strcmp(chan->name.c_str(), params[0])) // Maybe we can achieve better perfs with a map instead of iterating on a vector
            continue;
        
        // 1: Linked guild channels
        for (uint32 j = 0; j < chan->guilds.size(); j++) {
            if (Guild* guild = objmgr.GetGuildById(chan->guilds[j].guildId))
                guild->BroadcastToGuildFromIRC(msg);
        }
        // 2: Linked custom channels
        // no support yet
    }
}

void IRCMgr::HandleChatCommand(irc_session_t* session, const char* _channel, const char* params)
{
    if(params[0] != '!')
        return;

    if (!strncmp(params, "!who", 4)) {
        IRCServer* server = (IRCServer*) irc_get_ctx(session);
        if(!server) return;
        for (uint32 i = 0; i < server->channels.size(); i++) 
        {
            IRCChan* chan = server->channels[i];
            if(chan.name != _channel) continue;
            for (uint32 j = 0; j < chan->guilds.size(); j++) 
            {
                if (Guild* guild = objmgr.GetGuildById(chan->guilds[j].guildId))
                {
                    std::stringstream msg;
                    msg << "Connectés <" << guild->GetName() << ">: " << guild->GetOnlineMembersName();
                    irc_cmd_msg(session, _channel, msg.str().c_str());
                }
            }
            return;
        }
    }

    if(ircChatHandler) ircChatHandler->ParseCommands(session,_channel,params);
}

void IRCMgr::onIngameGuildJoin(uint32 guildId, const char* guildName, const char* origin)
{
    if (!origin)
        return;
    
    std::string msg = origin;
    msg += " a rejoint le canal de la guilde <";
    msg += guildName;
    msg += ">";
    
    sendToIRCFromGuild(guildId, msg);
}

void IRCMgr::onIngameGuildLeft(uint32 guildId, const char* guildName, const char* origin)
{
    if (!origin)
        return;
    
    std::string msg = origin;
    msg += " a quitté le canal de la guilde <";
    msg += guildName;
    msg += ">";
    
    sendToIRCFromGuild(guildId, msg);
}

void IRCMgr::onIngameGuildMessage(uint32 guildId, const char* origin, const char* message)
{
    if (!origin || !message)
        return;

    std::string msg = "[G][";
    msg += origin;
    msg += "] ";
    msg += message;
    
    sendToIRCFromGuild(guildId, msg);
}

void IRCMgr::onIngameChannelMessage(ChannelType type, const char* channel, const char* origin, const char* message)
{
    std::stringstream msg;
    switch(type)
    {
    case CHAN_TYPE_PUBLIC_ALLIANCE:
        msg << "[A]";
        break;
    case CHAN_TYPE_PUBLIC_HORDE:
        msg << "[H]";
        break;
    default:
        return;
    }

    msg << "[" << channel << "]";
    msg << "[" << origin << "] ";
    msg << message;

    for (ChannelToIRCMap::const_iterator itr = _channelToIRC.lower_bound(channel);itr != _channelToIRC.upper_bound(channel); itr++) {
        if(itr.first->type == type)
            irc_cmd_msg(((IRCServer*)itr.second->server)->session, itr.second->name.c_str(), msg.str().c_str());
        return;
    }
}

void IRCMgr::sendToIRCFromGuild(uint32 guildId, std::string msg)
{
    for (GuildToIRCMap::const_iterator itr = _guildsToIRC.lower_bound(guildId);
            itr != _guildsToIRC.upper_bound(guildId); itr++) {
        irc_cmd_msg(((IRCServer*)itr->second->server)->session, itr->second->name.c_str(), msg.c_str());
    }
}

void IRCMgr::onReportSpam(const char* spammer, uint32 spammerGUID)
{
    if (!spammer)
        return;

    std::ostringstream msg;
    msg << "[SPAM] " << sWorld.getConfig(CONFIG_SPAM_REPORT_THRESHOLD) << " joueurs ont signalé un spam de ";
    msg << spammer << " (GUID: " << spammerGUID << ") en moins de " << secsToTimeString(sWorld.getConfig(CONFIG_SPAM_REPORT_PERIOD)) << "."; // TODO: suggest a command to see reported messages
    for (IRCChans::const_iterator itr = _spamReportChans.begin(); itr != _spamReportChans.end(); itr++)
        irc_cmd_msg(((IRCServer*)(*itr)->server)->session, (*itr)->name.c_str(), msg.str().c_str());
}
void IRCHandler::SendSysMessage(const char *str)
{
    if(ircSession && channel)
        irc_cmd_msg(ircSession, channel, str);
}

#endif

#ifdef _WIN32
    bool IRCMgr::configure() {return true;}
    void IRCMgr::connect() {}
    void IRCMgr::run() {}
    void IRCMgr::onIRCConnectEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {}
    void IRCMgr::onIRCChannelEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {}
    void IRCMgr::onIngameGuildJoin(uint32 guildId, const char* guildName, const char* origin) {}
    void IRCMgr::onIngameGuildLeft(uint32 guildId, const char* guildName, const char* origin) {}
    void IRCMgr::onIngameGuildMessage(uint32 guildId, const char* origin, const char* message) {}
    void IRCMgr::onIngameChannelMessage(ChannelType type, const char* channel, const char* origin, const char* message) {}
    void IRCMgr::onReportSpam(const char* spammer, uint32 spammerGUID) {}

    void IRCMgr::sendToIRCFromGuild(uint32 guildId, std::string msg) {}

    void IRCHandler::SendSysMessage(const char *str) {}
#endif

    
const char *IRCHandler::GetTrinityString(int32 entry) const
{
    return objmgr.GetTrinityStringForDBCLocale(entry);
}

bool IRCHandler::isAvailable(ChatCommand const& cmd) const
{
     return cmd.AllowIRC;
}

const char *IRCHandler::GetName() const
{
    return GetTrinityString(172); //LANG_CONSOLE_COMMAND = 172
}

bool IRCHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

int IRCHandler::ParseCommands(irc_session_t* session,const char* _channel, const char* params)
{
    if(!sWorld.getConfig(CONFIG_IRC_COMMANDS))
        return false;

    ircSession = session;
    channel = _channel;
    return ChatHandler::ParseCommands(params);
}
