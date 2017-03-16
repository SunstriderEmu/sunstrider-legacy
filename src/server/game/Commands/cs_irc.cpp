#include "Chat.h"
#include "IRCMgr.h"

bool ChatHandler::HandleIRCReconnectCommand(const char* /* args */)
{
   PSendSysMessage("Trying to reconnect IRC...");
   sIRCMgr->connect();

   return true;
}

bool ChatHandler::HandleIRCJoinCommand(const char* /* args */)
{
    PSendSysMessage("NYI");
    return true;
}

bool ChatHandler::HandleIRCPartCommand(const char* /* args */)
{
    PSendSysMessage("NYI");
    return true;
}

bool ChatHandler::HandleIRCQuitCommand(const char* /* args */)
{
    PSendSysMessage("NYI");
    return true;
}