#include "IRCMgr.h"
#include "Database/DatabaseEnv.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "World.h"
#include "Runnable.h"
#include <boost/regex.hpp>

void IRCSession::run()
{
    /*
    irc_run loop forever until irc_cmd_quit is called, so we need to start it in a separate thread
    */
    std::thread ircRun([&](){
         if (irc_run(_server->session)) {
            TC_LOG_ERROR("IRCMgr","Could not connect or I/O error with a server (%s:%u, %susing SSL): %s", 
                    _server->host.c_str(), _server->port, (_server->ssl ? "" : "not "), irc_strerror(irc_errno(_server->session)));
            return;
        }
    });

    while(!m_stop)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    //BUT THEN
    irc_cmd_quit(_server->session, "Quit");
        
    ircRun.join();
}

IRCMgr::IRCMgr()
{
    TC_LOG_INFO("IRCMgr","IRCMgr: Initializing...");
    if (!configure()) {
        TC_LOG_ERROR("IRCMgr","IRCMgr: There are errors in your configuration.");
        return;
    }
        
    // Initialize the callbacks
    memset(&_callbacks, 0, sizeof (_callbacks));

    // Set up the callbacks we will use
    _callbacks.event_connect = onIRCConnectEvent;
    _callbacks.event_channel = onIRCChannelEvent;
    _callbacks.event_kick = onIRCPartEvent;
    _callbacks.event_quit = onIRCPartEvent;
    _callbacks.event_part = onIRCPartEvent;
    _callbacks.event_join = onIRCJoinEvent;
    
    connect();
    
    ircChatHandler = new IRCHandler();

    TC_LOG_INFO("IRCMgr","IRCMgr initialized.");
}

IRCMgr::~IRCMgr()
{
    stopSessions();
    
    delete ircChatHandler;
    ircChatHandler = nullptr;

    TC_LOG_DEBUG("IRCMgr","IRCMgr stopped.");
}

bool IRCMgr::configure()
{
    QueryResult res = CharacterDatabase.Query("SELECT `id`, `host`, `port`, `ssl`, `nick` FROM wrchat_servers ORDER BY `id`");
    if (!res) {
        TC_LOG_ERROR("IRCMgr", "IRCMgr : No server found, please set IRC.Enabled to 0 in configuration file if you don't want to use the IRC bridge.");
        return false;
    }
    
    uint32 count = 0;
    uint32 id;
    Field* fields = res->Fetch();
    QueryResult res2 = NULL;
    Field* fields2 = NULL;
    do {
        IRCServer* server = new IRCServer;
        id = fields[0].GetUInt32();
        server->session = NULL;
        server->host = fields[1].GetString();
        server->port = fields[2].GetUInt32();
        server->ssl = fields[3].GetBool();
        server->nick = fields[4].GetString();
        
        /*                                      0               1            2              3              4      */
        res2 = CharacterDatabase.PQuery("SELECT irc_channel, password, ingame_channel, channel_type, join_message FROM wrchat_channels WHERE server = %u", id);
        if (!res2) {
            TC_LOG_ERROR("IRCMgr","IRCMgr : Server %u (%s:%u, %susing SSL) has no associated channels in table wrchat_channels.",
                    id, server->host.c_str(), server->port, server->ssl ? "" : "not ");
            continue;
        }
        
        do {
            fields2 = res2->Fetch();

            IRCChan* channel = new IRCChan;
            channel->name = fields2[0].GetString();
            channel->password = fields2[1].GetString();
            channel->joinmsg = fields2[4].GetString();
            channel->server = server;
            channel->enabled = false;
            
            uint32 type = fields2[3].GetUInt32();
            switch (type) {
            case CHAN_TYPE_CHANNEL_ALLIANCE:
            {
                PublicChannel cc;
                cc.name = fields2[2].GetString();
                cc.faction = CHAN_FACTION_ALLIANCE;

                _channelToIRC_A.insert(std::make_pair(cc.name, channel));
                break;
            }
            case CHAN_TYPE_CHANNEL_HORDE:
            {
                PublicChannel cc;
                cc.name = fields2[2].GetString();
                cc.faction = CHAN_FACTION_HORDE;

                _channelToIRC_H.insert(std::make_pair(cc.name, channel));
                break;
            }
            case CHAN_TYPE_GUILD:
            {
                GuildChannel gc;
                uint32 guildId = atoi(fields2[2].GetCString());
                gc.guildId = guildId;
                channel->guilds.push_back(gc);
                
                _guildsToIRC.insert(std::make_pair(guildId, channel));
                
                break;
            }
            case CHAN_TYPE_SPAM_REPORT:
                _spamReportChans.push_back(channel);
                break;
            default:
                TC_LOG_ERROR("IRCMgr","IRCMgr : Invalid channel type %u.", type);
            }
            
            server->channels.push_back(channel);
        } while (res2->NextRow());
        
        _servers[id] = server;
        count++;
    } while (res->NextRow());
    
    TC_LOG_INFO("IRCMgr","IRCMgr : Loaded %u irc servers.", count);
    
    return true;
}

void IRCMgr::onIngameGuildJoin(uint32 guildId, std::string const& guildName, std::string const& origin)
{
    if (origin.empty())
        return;
    
    std::string msg = origin;
    msg += " joined guild channel <";
    msg += guildName;
    msg += ">";
    
    sendToIRCFromGuild(guildId, msg);
}

void IRCMgr::onIngameGuildLeft(uint32 guildId, std::string const& guildName, std::string const& origin)
{
    if (origin.empty())
        return;
    
    std::string msg = origin;
    msg += " left guild channel <";
    msg += guildName;
    msg += ">";
    
    sendToIRCFromGuild(guildId, msg);
}

void IRCMgr::onIngameGuildMessage(uint32 guildId, std::string const& origin, const char* message)
{
    if (origin.empty() || !message)
        return;
    
    std::string str_msg(message);

    IRCMgr::ConvertWoWColorsToIRC(str_msg);

    std::string msg = "[G][";
    msg += origin;
    msg += "] ";
    msg += str_msg;
    
    sendToIRCFromGuild(guildId, msg);
}

void IRCMgr::EnableServer(IRCServer* server, bool enable)
{
    for(auto itr = _guildsToIRC.begin(); itr != _guildsToIRC.end();itr++)
        if(itr->second->server == server)
            itr->second->enabled = enable;

    for(auto itr = _channelToIRC_A.begin(); itr != _channelToIRC_A.end();itr++)
        if(itr->second->server == server)
            itr->second->enabled = enable;

    for(auto itr = _channelToIRC_H.begin(); itr != _channelToIRC_H.end();itr++)
        if(itr->second->server == server)
            itr->second->enabled = enable;
}

void IRCMgr::ConvertWoWColorsToIRC(std::string& msg)
{
    //not working properly atm if there is multiple items or characters before the item :

    //IRC support only 16 colors, let's replace some wow known colors with close ones, or default to brown
    std::string color = boost::regex_replace(msg, boost::regex("\\|c..(......)((?!\\|r).+)\\|r"), "$1");
    color = color.erase(0,1); //FIXME regex_replace put a '32' first character, why ? hack delete here
    if(!color.compare("9d9d9d"))
        color = "DARKGRAY";
    else if(!color.compare("ffffff"))
        color = "WHITE/BLACK";
    else if (!color.compare("0070dd"))
        color = "BLUE";
    else if (!color.compare("1eff00"))
        color = "GREEN";
    else if (!color.compare("a335ee"))
        color = "PURPLE";
    else if (!color.compare("ff8000")) //orange item. No orange in irc, using red.
        color = "RED";
    else if ( (!color.compare("ffff00")) || (!color.compare("40c040"))   // yellow quest || green quest
           || (!color.compare("ff2020")) || (!color.compare("808080")) ) // || red quest || gray quest
        color = "YELLOW/BLACK";
    else
        color = "BROWN";

    //replacecolor
    std::string formatter = "[COLOR=" + color + "]$2[/COLOR]";
    msg = boost::regex_replace(msg, boost::regex("\\|c..(......)((?!\\|r).+)\\|r"), formatter); 
   // TC_LOG_DEBUG("IRCMgr",msg.c_str());

    //remove some other junk (player:, spell:, ...)
    msg = boost::regex_replace(msg, boost::regex("\\|H[^:]+:[^\\[]*([^\\|]+)\\|h"), "$1");
   // TC_LOG_DEBUG("IRCMgr",msg.c_str());

    //convert to irc format
    msg = irc_color_convert_to_mirc(msg.c_str());
}

void IRCMgr::onIngameChannelMessage(ChannelFaction faction, const char* channel, std::string const& origin, const char* message)
{
    std::stringstream msg;
    switch(faction)
    {
    case CHAN_FACTION_ALLIANCE:
        msg << "[COLOR=DARKBLUE][A]";
        break;
    case CHAN_FACTION_HORDE:
        msg << "[COLOR=RED][H]";
        break;
    default:
        return;
    }

    TC_LOG_DEBUG("IRCMgr", "%s", message);
    std::string str_message(message);
    IRCMgr::ConvertWoWColorsToIRC(str_message);

    msg << "[" << channel << "]";
    msg << "[" << origin << "] ";
    msg << "[/COLOR]";
    msg << str_message;
    
    TC_LOG_DEBUG("IRCMgr", "%s", msg.str().c_str());

    std::string finalmsg(irc_color_convert_to_mirc(msg.str().c_str()));

    sendToIRCFromChannel(channel, faction, finalmsg);
}

void IRCMgr::onIRCPartEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    std::string fullNick(origin);                                           
    std::string nick = fullNick.substr(0, fullNick.find("!"));
    IRCServer* server = (IRCServer*) irc_get_ctx(session);
    if(strcmp(server->nick.c_str(), nick.c_str()) == 0) // if it's me !
        sIRCMgr->EnableServer(server,false);
}

void IRCMgr::onIRCJoinEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    std::string fullNick(origin);                                           
    std::string nick = fullNick.substr(0, fullNick.find("!"));
    IRCServer* server = (IRCServer*) irc_get_ctx(session);
    if(strcmp(server->nick.c_str(), nick.c_str()) == 0) // if it's me !
        sIRCMgr->EnableServer(server,true);
}

void IRCMgr::onIRCChannelEvent(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    if (!params[1] || !origin) // No message sent
        return;

    if(sWorld->IsShuttingDown())
        return;
    
    sIRCMgr->HandleChatCommand(session,params[0],params[1]);
    
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
            if (Guild* guild = sObjectMgr->GetGuildById(chan->guilds[j].guildId))
                guild->BroadcastToGuildFromIRC(msg);
        }
        // 2: Linked custom channels
        // no irc -> custom channel support yet
    }
}

void IRCMgr::stopSessions()
{
    for( IRCThreadList::iterator itr = sessionThreads.begin( ); itr != sessionThreads.end( ); itr++)
    {
        (*itr)->stop();
    }

    //Delete sessions
    for( IRCThreadList::iterator itr = sessionThreads.begin( ); itr != sessionThreads.end( ); )
    {
        delete *itr;
        itr = sessionThreads.erase(itr);
    }

}

void IRCMgr::startSessions()
{
    stopSessions();

    // Start one thread per session
    Runnable* lastSpawned;
    for (IRCServers::iterator itr = _servers.begin(); itr != _servers.end(); itr++) {
        lastSpawned = new IRCSession(itr->second);
        lastSpawned->start();
        sessionThreads.push_back(lastSpawned);
    }
}

void IRCMgr::connect()
{
    for (IRCServers::iterator itr = _servers.begin(); itr != _servers.end(); itr++) {
        if (itr->second->session && irc_is_connected(itr->second->session))
        {
            TC_LOG_WARN("IRCMgr", "IRCMgr: Could not create IRC session for server %u (%s:%u), session is already connected.", itr->first, itr->second->host.c_str(), itr->second->port);
            continue;
        }

        itr->second->session = irc_create_session(&_callbacks);
        if (!itr->second->session) {
            TC_LOG_ERROR("IRCMgr","IRCMgr: Could not create IRC session for server %u (%s:%u, %susing SSL)",
                    itr->first, itr->second->host.c_str(), itr->second->port, (itr->second->ssl ? "" : "not "));
            continue;
        }

        irc_set_ctx(itr->second->session, itr->second);
        irc_option_set(itr->second->session, LIBIRC_OPTION_SSL_NO_VERIFY);
        
        if (irc_connect(itr->second->session, itr->second->host.c_str(), itr->second->port, 0, itr->second->nick.c_str(), itr->second->nick.c_str(), "Sunstrider irc bridge")) {
            TC_LOG_ERROR("IRCMgr","IRCMgr: Could not connect to server %u (%s:%u, %susing SSL): %s",
                    itr->first, itr->second->host.c_str(), itr->second->port, (itr->second->ssl ? "" : "not "), irc_strerror(irc_errno(itr->second->session)));
        }
    }
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

void IRCMgr::HandleChatCommand(irc_session_t* session, const char* _channel, const char* params)
{
    if(params[0] != '!')
        return;

    if (strncmp(params, "!who", 4) == 0) {
        IRCServer* server = (IRCServer*) irc_get_ctx(session);
        if(!server) return;
        for (uint32 i = 0; i < server->channels.size(); i++) 
        {
            IRCChan* chan = server->channels[i];
            if(chan->name != _channel || !chan->enabled) continue;
            for (uint32 j = 0; j < chan->guilds.size(); j++) 
            {
                if (Guild* guild = sObjectMgr->GetGuildById(chan->guilds[j].guildId))
                {
                    std::stringstream msg;
                    msg << "Connected <" << guild->GetName() << ">: " << guild->GetOnlineMembersName();
                    irc_cmd_msg(session, _channel, msg.str().c_str());
                }
            }
            return;
        }
    }

    if(ircChatHandler) ircChatHandler->ParseCommands(session,_channel,params);
}

void IRCMgr::sendToIRCFromGuild(uint32 guildId, std::string msg)
{
    mtx.lock();
    std::pair <GuildToIRCMap::iterator, GuildToIRCMap::iterator> range;
    range = _guildsToIRC.equal_range(guildId);
  
    for( GuildToIRCMap::iterator itr = range.first; itr != range.second; ++itr) {
        if(!itr->second->enabled)
            continue;

        irc_cmd_msg(((IRCServer*)itr->second->server)->session, itr->second->name.c_str(), msg.c_str());
    }
    mtx.unlock();
}

void IRCMgr::sendGlobalMsgToIRC(std::string msg)
{
    for(auto itr : _guildsToIRC)
    {
        if(!itr.second->enabled)
            continue;

        irc_cmd_msg(((IRCServer*)itr.second->server)->session, itr.second->name.c_str(), msg.c_str());
    }

    //useless lambda for pleasure
    auto glambda = [&](ChannelToIRCMap ircmap) 
    {

        for(auto itr : ircmap)
        {
            if(!itr.second->enabled)
                continue;

            irc_cmd_msg(((IRCServer*)itr.second->server)->session, itr.second->name.c_str(), msg.c_str());
        }
    };

    glambda(_channelToIRC_A);
    glambda(_channelToIRC_H);
}

void IRCMgr::sendToIRCFromChannel(const char* channel, ChannelFaction faction, std::string msg)
{
    std::pair <ChannelToIRCMap::iterator, ChannelToIRCMap::iterator> range;
    if(faction == CHAN_FACTION_ALLIANCE)
        range = _channelToIRC_A.equal_range(channel);
    else //CHAN_FACTION_HORDE
        range = _channelToIRC_H.equal_range(channel);
  
    for( ChannelToIRCMap::iterator itr = range.first; itr != range.second; ++itr) 
    {
        if(!itr->second->enabled)
            continue;

        irc_cmd_msg(((IRCServer*)itr->second->server)->session, itr->second->name.c_str(), msg.c_str());
    }
}

void IRCMgr::onReportSpam(std::string const& spammerName, uint32 spammerGUID)
{
    if (spammerName.empty())
        return;

    std::ostringstream msg;
    msg << "[SPAM] " << sWorld->getConfig(CONFIG_SPAM_REPORT_THRESHOLD) << " players reported spam from ";
    msg << spammerName << " (GUID: " << spammerGUID << ") in lass than " << secsToTimeString(sWorld->getConfig(CONFIG_SPAM_REPORT_PERIOD)) << "."; // TODO: suggest a command to see reported messages
    for (IRCChans::const_iterator itr = _spamReportChans.begin(); itr != _spamReportChans.end(); itr++)
        irc_cmd_msg(((IRCServer*)(*itr)->server)->session, (*itr)->name.c_str(), msg.str().c_str());
}

void IRCHandler::SendSysMessage(const char *str, bool /* escapeCharacters */)
{
    if(ircSession && channel)
    {
        //decompose string into multiple lines
        std::string s(str);
        size_t startPos = 0;
        size_t lastPos = 0;
        std::string subStr;

        lastPos = s.find("\r");
        if(lastPos == std::string::npos)
            lastPos = s.find("\n");
        while(lastPos != std::string::npos) {
           subStr = s.substr(startPos, lastPos - startPos);
           IRCMgr::ConvertWoWColorsToIRC(subStr);
           irc_cmd_msg(ircSession, channel, subStr.c_str());
           startPos = lastPos + 1;
           //find next end of line or string end
           lastPos = s.find("\r", startPos);
           if(lastPos == std::string::npos)
               lastPos = s.find("\n", startPos);
        }
        //send last part (from last line return to to string end)
        subStr = s.substr(startPos, std::string::npos);
        IRCMgr::ConvertWoWColorsToIRC(subStr);
        irc_cmd_msg(ircSession, channel, subStr.c_str());
    }
}
    
const char *IRCHandler::GetTrinityString(int32 entry) const
{
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

bool IRCHandler::isAvailable(ChatCommand const& cmd) const
{
    return cmd.noSessionNeeded && cmd.AllowIRC;
}

std::string const IRCHandler::GetName() const
{
    return GetTrinityString(172); //LANG_CONSOLE_COMMAND = 172
}

bool IRCHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

int IRCHandler::ParseCommands(irc_session_t* session,const char* _channel, const char* params)
{
    if(!sWorld->getConfig(CONFIG_IRC_COMMANDS))
        return false;

    ircSession = session;
    channel = _channel;
    return ChatHandler::ParseCommands(params);
}
