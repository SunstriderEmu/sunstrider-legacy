
#include "IRCMgr.h"
#include "Log.h"

INSTANTIATE_SINGLETON_1(IRCMgr);

IRCMgr::IRCMgr()
{
    // Initialize the callbacks
    memset(&_callbacks, 0, sizeof (_callbacks));

    // Set up the callbacks we will use
    _callbacks.event_connect = cbConnect;
    _port = 6667;

    _session = irc_create_session(&_callbacks);
    if (!_session) {
        sLog.outError("IRCMgr: Could not create IRC session.");
        return;
    }

    _ctx.channel = "#test";
    _ctx.nick = "NeoW";
    irc_set_ctx(_session, &_ctx);

    irc_option_set(_session, LIBIRC_OPTION_SSL_NO_VERIFY);
    if (irc_connect(_session, "chat.wowmania.fr", _port, 0, "NeW", 0, 0)) {
        sLog.outError("IRCMgr: Could not connect: %s", irc_strerror(irc_errno(_session)));
        return;
    }
    
    sLog.outString("IRCMgr initialized.");
}

IRCMgr::~IRCMgr()
{
}

void IRCMgr::run()
{
    if (irc_run(_session)) {
        printf("IRCMgr: Could not connect or I/O error: %s\n", irc_strerror(irc_errno(_session)));
        return;
    }
}

void IRCMgr::cbConnect(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count)
{
    /*irc_ctx_t * ctx = (irc_ctx_t *) irc_get_ctx (session);
    irc_cmd_join (session, ctx->channel, 0);
    
    irc_cmd_msg (session, "#test", "OHAI");*/
}