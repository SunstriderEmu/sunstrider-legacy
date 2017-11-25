#ifndef TESTTHREAD_H
#define TESTTHREAD_H

class TestCase;
#include "TestResults.h"
#include <atomic>

class TestThread
{
public:
    TestThread(std::shared_ptr<TestCase> test);
    void Run();
    //update test Wait Timer and notify main thread to carry on if needed
    void UpdateWaitTimer(uint32 const diff);
    std::shared_ptr<TestCase> GetTest() const { return _testCase; };
    bool IsFinished() const { return _finished; }

    // Sleep caller execution until ... (this does not sleep the test thread)
    void SleepUntilDoneOrWaiting(std::shared_ptr<TestCase> test);
    // Sleep caller execution for given ms (Must be called from the TestCase). False if test is cancelling
    bool Wait(uint32 ms);    
    
    //stop and fail tests as soon as possible
    void Cancel();

private:
    std::shared_ptr<TestCase> _testCase;
    std::future<void> _future;
    std::atomic<bool> _finished; // this thread may be finished either because test finished, or because test was cancelled

    void _SetWait(uint32 ms);
    // -- var to handle waiting
    std::condition_variable _cv;
    std::mutex _m;
    std::atomic<uint32>     _waitTimer;
    /* When waiting, we need some way to ignore time that has already passed in this update. This is particularly needed if we're using the debugger. This is very approximative...*/
    milliseconds _thisUpdateStartTimeMS;
    // --
};

#endif // TESTTHREAD_H
