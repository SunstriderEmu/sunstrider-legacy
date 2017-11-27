#ifndef TESTTHREAD_H
#define TESTTHREAD_H

class TestCase;
#include "TestResults.h"
#include <atomic>

class TestThread
{
public:
    //Create thread with test, do not start immediately
    TestThread(std::shared_ptr<TestCase> test);
    //Start test!
    void Start();
    bool IsStarted() const;

    //Resume execution if thread was waiting
    void WakeUp();

    //Main function for thread
    void Run();
    //update test Wait Timer but do not notify anything
    void UpdateWaitTimer(uint32 const diff);
    std::shared_ptr<TestCase> GetTest() const { return _testCase; };
    bool IsFinished() const;

    // Sleep caller execution until ... (this does not sleep the test thread)
    void WaitUntilDoneOrWaiting(std::shared_ptr<TestCase> test);
    // Sleep caller execution for given ms (MUST BE called from the TestCase only). False if test is cancelling
    bool Wait(uint32 ms);    
    
    //stop and fail tests as soon as possible
    void Cancel();

    enum ThreadState : uint8
    {
        STATE_NOT_STARTED, //not yet running
        STATE_RUNNING, 
        STATE_WAITING,
        STATE_FINISHED,
    };

private:
    std::shared_ptr<TestCase> _testCase;
    std::future<void> _future;
    std::atomic<ThreadState> _state; // this thread may be finished either because test finished, or because test was cancelled

    //Set wait time for the thread. (will not notify anything)
    void _SetWait(uint32 ms);
    // -- var to handle waiting
    std::condition_variable _testCV;
    std::mutex _testCVMutex;
    std::atomic<uint32>     _waitTimer;
    /* When waiting, we need some way to ignore time that has already passed in this update. This is particularly needed if we're using the debugger. This is very approximative...*/
    milliseconds _thisUpdateStartTimeMS;
    // --
};

#endif // TESTTHREAD_H
