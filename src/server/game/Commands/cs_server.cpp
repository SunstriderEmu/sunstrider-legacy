#include "Chat.h"
#include "Language.h"
#include "GlobalEvents.h"
#include "Monitor.h"
#include "GitRevision.h"
#include "GameTime.h"

/// Triggering corpses expire check in world
bool ChatHandler::HandleServerCorpsesCommand(const char* /*args*/)
{
    SendSysMessage("NYI");
    return true;
}

/// Exit the realm
bool ChatHandler::HandleServerExitCommand(const char* args)
{
    SendSysMessage(LANG_COMMAND_EXIT);
    World::StopNow(SHUTDOWN_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleServerInfoCommand(const char* /*args*/)
{
    uint32 activeClientsNum = sWorld->GetActiveSessionCount();
    uint32 queuedClientsNum = sWorld->GetQueuedSessionCount();
    uint32 maxActiveClientsNum = sWorld->GetMaxActiveSessionCount();
    std::string str = secsToTimeString(GameTime::GetUptime());
    uint32 currentMapTimeDiff = 0;
    if (GetSession())
        if (Player const* p = GetSession()->GetPlayer())
            if (Map const* m = p->FindMap())
                currentMapTimeDiff = sMonitor->GetLastDiffForMap(*m);

    PSendSysMessage(GitRevision::GetFullVersion());
    PSendSysMessage("Players online: %u (Max: %u, Queued: %u)", activeClientsNum, maxActiveClientsNum, queuedClientsNum);
    PSendSysMessage(LANG_UPTIME, str.c_str());
    PSendSysMessage("Smoothed update time diff: %u.", sMonitor->GetSmoothTimeDiff());
    PSendSysMessage("Instant update time diff: %u.", sWorld->GetUpdateTime());
    PSendSysMessage("Current map update time diff: %u.", currentMapTimeDiff);
    if (sWorld->IsShuttingDown())
        PSendSysMessage("Server restart in %s", secsToTimeString(sWorld->GetShutDownTimeLeft()).c_str());

    return true;
}

/// Display the 'Message of the day' for the realm
bool ChatHandler::HandleServerMotdCommand(const char* /*args*/)
{
    PSendSysMessage(LANG_MOTD_CURRENT, sWorld->GetMotd());
    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(const char* args)
{
    sWorld->ShutdownCancel();
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    //if (exitcode_str)
    if (reason)
    {
        /*int32 exitcode = atoi (exitcode_str);

        // Handle atoi() errors
        if (exitcode == 0 && (exitcode_str[0] != '0' || exitcode_str[1] != '\0'))
            return false;

        // Exit code should be in range of 0-125, 126-255 is used
        // in many shells for their own return codes and code > 255
        // is not supported in many others
        if (exitcode < 0 || exitcode > 125)
            return false;

        sWorld->ShutdownServ (time, 0, exitcode);*/
        
        sWorld->ShutdownServ(time, 0, SHUTDOWN_EXIT_CODE, reason);
    }
    else
        sWorld->ShutdownServ(time, 0, SHUTDOWN_EXIT_CODE, "");
    return true;
}

bool ChatHandler::HandleServerRestartCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, reason);
    else
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, "");
        
    return true;
}

bool ChatHandler::HandleServerIdleRestartCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, reason);
    else
        sWorld->ShutdownServ(time,SHUTDOWN_MASK_RESTART|SHUTDOWN_MASK_IDLE, RESTART_EXIT_CODE, "");
        
    return true;
}

bool ChatHandler::HandleServerIdleShutDownCommand(const char* args)
{
    ARGS_CHECK

    char* time_str = strtok ((char*) args, " ");
    char* reason = strtok (nullptr, "");

    int32 time = atoi (time_str);

    ///- Prevent interpret wrong arg value as 0 secs shutdown time
    if((time == 0 && (time_str[0]!='0' || time_str[1]!='\0')) || time < 0)
        return false;

    if (reason)
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_IDLE, SHUTDOWN_EXIT_CODE, reason);
    else
        sWorld->ShutdownServ(time, SHUTDOWN_MASK_IDLE, SHUTDOWN_EXIT_CODE, "");
        
    return true;
}

/// Define the 'Message of the day' for the realm
bool ChatHandler::HandleServerSetMotdCommand(const char* args)
{
    sWorld->SetMotd(args);
    PSendSysMessage(LANG_MOTD_NEW, args);
    return true;
}

bool ChatHandler::HandleServerSetConfigCommand(const char* args)
{
    ARGS_CHECK

    char* cConfigIndex = strtok((char*)args, " ");
    char* cConfigValue = strtok(nullptr, " ");

    if (!cConfigIndex || !cConfigValue)
        return false;

    uint32 configIndex = (uint32)atoi(cConfigIndex);
    if(configIndex > CONFIG_VALUE_COUNT)
    {
        PSendSysMessage("Wrong index");
        return false;
    }

    uint32 configValue = (uint32)atoi(cConfigValue);
    sWorld->setConfig(configIndex,configValue);

    PSendSysMessage("Config %i set to %i",configIndex,configValue);

    return true;
}
