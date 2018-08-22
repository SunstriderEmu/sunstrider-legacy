#include "Chat.h"
#include "Profiler.h"

/* .profiling start [filename] */
static bool HandleProfilingStartCommand(ChatHandler* handler, char const* args)
{
    //default filename
    std::string filename = std::to_string(time(nullptr)) + ".prof";
    char* cFileName = strtok((char*)args, " ");
    if (cFileName)
        filename = cFileName;

    std::string failureReason;
    if(sProfiler->Start(filename, failureReason))
        handler->SendSysMessage("Profiling started");
    else
        handler->PSendSysMessage("Profiling start failed with reason %s", failureReason.c_str());
    return true;
}

/* .profiling stop */
static bool HandleProfilingStopCommand(ChatHandler* handler, char const* args)
{
    std::string failureReason;
    if (sProfiler->Stop(failureReason))
        handler->SendSysMessage("Profiling stopped");
    else
        handler->PSendSysMessage("Profiling stop failed with reason %s", failureReason.c_str());

    return true;
}

/* .profiling status */
static bool HandleProfilingStatusCommand(ChatHandler* handler, char const* args)
{
    std::string infos = sProfiler->GetInfos();
    handler->PSendSysMessage("Profiling infos:\n%s", infos.c_str());
    return true;
}
