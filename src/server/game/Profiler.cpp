#include "Profiler.h"
#include <sstream>
#ifdef USE_GPERFTOOLS
	#include <gperftools/profiler.h>
#endif

bool Profiler::Start(std::string filename, std::string& failureReason)
{
#ifdef USE_GPERFTOOLS
	if (IsRunning())
	{
		failureReason = "Already running";
		return false;
	}

	bool success = ProfilerStart(filename.c_str());
	if (!success)
	{
		failureReason = "ProfilerStart returned an error";
		return false;
	}

	return true;
#else
	failureReason = "Not compiled with gperftools";
	return false;
#endif
}

bool Profiler::Stop(std::string& failureReason)
{
#ifdef USE_GPERFTOOLS

	if (!IsRunning())
	{
		failureReason = "Not running";
		return false;
	}
	ProfilerStop();
	return true;
#else
	failureReason = "Not compiled with gperftools";
	return false;
#endif
}

bool Profiler::IsRunning() const 
{
#ifdef USE_GPERFTOOLS
	ProfilerState state;
	ProfilerGetCurrentState(&state);
	return state.enabled;
#else
	return false;
#endif
}

void Profiler::RegisterThread()
{
#ifdef USE_GPERFTOOLS
	ProfilerRegisterThread();
#endif
}

void Profiler::Flush()
{
#ifdef USE_GPERFTOOLS
	ProfilerFlush();
#endif
}

std::string Profiler::GetInfos() const
{
#ifdef USE_GPERFTOOLS
	bool enabledForAllThreads = ProfilingIsEnabledForAllThreads();
	ProfilerState state;
	ProfilerGetCurrentState(&state);

	std::stringstream infos;
	infos << "Profiling is " << std::endl;
	infos << "- " << (state.enabled ? "enabled" : "disabled") << std::endl;
	if (!state.enabled)
		return infos.str();

	infos << "- " << (enabledForAllThreads ? "" : "not ") << "enabled for all threads" << std::endl;
	infos << "- Started at " << std::to_string(state.start_time) << " / " << TimeToTimestampStr(state.start_time) << std::endl;
	infos << "- Writing to " << state.profile_name << std::endl;
	infos << "- Has currently gathered " << std::to_string(state.samples_gathered) << " samples" << std::endl;
	
	return infos.str();
#else
	return "Not compiled with gperftools";
#endif

}

