#ifndef IRCMGR_H
#define	IRCMGR_H

#include <Common.h>
#include <libircclient.h>
#include "Policies/SingletonImp.h"

typedef struct {
    char* channel;
    char* nick;

} irc_ctx_t;

class IRCMgr : public ACE_Based::Runnable
{
public:

    IRCMgr();
    virtual ~IRCMgr();

    static void cbConnect(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count);
    void run();

private:

    irc_callbacks_t _callbacks;
    irc_ctx_t _ctx;
    unsigned short _port;
    irc_session_t* _session;

};

#define sIRCMgr Trinity::Singleton<IRCMgr>::Instance()

#endif	/* IRCMGR_H */

