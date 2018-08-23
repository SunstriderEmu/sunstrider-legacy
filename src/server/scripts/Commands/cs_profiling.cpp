#include "Chat.h"
#include "Profiler.h"

class profiling_commandscript : public CommandScript
{
public:
    profiling_commandscript() : CommandScript("profiling_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> profilingCommandTable =
        {
            { "start",     SEC_SUPERADMIN,   true,  &HandleProfilingStartCommand,             "" },
            { "stop",      SEC_SUPERADMIN,   true,  &HandleProfilingStopCommand,              "" },
            { "status",    SEC_SUPERADMIN,   true,  &HandleProfilingStatusCommand,            "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "profiling", SEC_SUPERADMIN,   false, nullptr,                                  "", profilingCommandTable },
        };
    }

    /* .profiling start [filename] */
    static bool HandleProfilingStartCommand(ChatHandler* handler, char const* args)
    {
        //default filename
        std::string filename = std::to_string(time(nullptr)) + ".prof";
        char* cFileName = strtok((char*)args, " ");
        if (cFileName)
            filename = cFileName;

        std::string failureReason;
        if (sProfiler->Start(filename, failureReason))
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
};

void AddSC_profiling_commandscript()
{
    new profiling_commandscript();
}
