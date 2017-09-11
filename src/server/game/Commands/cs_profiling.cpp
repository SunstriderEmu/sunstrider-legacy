#include "Chat.h"
#include "Profiler.h"

/* .profiling start [filename] */
bool ChatHandler::HandleProfilingStartCommand(const char* args)
{
    //default filename
    std::string filename = std::to_string(time(nullptr)) + ".prof";
    char* cFileName = strtok((char*)args, " ");
    if (cFileName)
        filename = cFileName;

    std::string failureReason;
    if(sProfiler->Start(filename, failureReason))
        SendSysMessage("Profiling started");
    else
        PSendSysMessage("Profiling start failed with reason %s", failureReason.c_str());
    return true;
}

/* .profiling stop */
bool ChatHandler::HandleProfilingStopCommand(const char* args)
{
    std::string failureReason;
    if (sProfiler->Stop(failureReason))
        SendSysMessage("Profiling stopped");
    else
        PSendSysMessage("Profiling stop failed with reason %s", failureReason.c_str());

    return true;
}

/* .profiling status */
bool ChatHandler::HandleProfilingStatusCommand(const char* args)
{
    std::string infos = sProfiler->GetInfos();
    PSendSysMessage("Profiling infos:\n%s", infos.c_str());
    return true;
}
