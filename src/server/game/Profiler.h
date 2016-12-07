#ifndef __PROFILER_H
#define __PROFILER_H

class Profiler
{
public:
    static Profiler* instance()
    {
        static Profiler instance;
        return &instance;
    }

	bool Start(std::string filename, ::string& failureReason);
	bool Stop(std::string& failureReason);
	/* Flush any currently buffered profiling state to the profile file.
	* Has no effect if the profiler has not been started.
	*/
	void Flush();
	/* Not sure when to use this: "Routine for registering new threads with the profiler. "*/
	void RegisterThread();
	bool IsRunning() const;
	std::string GetInfos() const;

private:
	Profiler() {}
};

#define sProfiler Profiler::instance()

#endif // __PROFILER_H
