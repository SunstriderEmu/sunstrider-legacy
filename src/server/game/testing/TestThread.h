#ifndef TESTTHREAD_H
#define TESTTHREAD_H

class TestCase;
#include "TestResults.h"
#include <atomic>

//Custom exception. This is to have a different type than exception and be able to differenciate test and regular exceptions
class TestException : public std::exception
{
public:
    explicit TestException() :
        exception()
    { }
};

class TestThread
{
public:
    //Create thread with test, do not start immediately
    TestThread(std::shared_ptr<TestCase> test);
    //Start test!
    void Start();
    bool IsStarted() const;

    //Wake up TestMgr if it was waiting
    void WakeUp();
    //Resume execution after a previous loop
    void ResumeExecution();

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
    uint32 GetWaitTimer() const { return _waitTimer;  }
    
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
    uint32 _lastMapUpdateTime; //last update time of the map, the last time we updated the test. This is used to make sure the map has updated before updating our test.
    std::future<void> _future;
    std::atomic<ThreadState> _state; // this thread may be finished either because test finished, or because test was cancelled

    //Set wait time for the thread. (will not notify anything)
    void _SetWait(uint32 ms);
    // -- var to handle waiting
    std::condition_variable _testCV;
    std::mutex _testCVMutex;
    std::atomic<uint32>     _waitTimer;
};

#endif // TESTTHREAD_H
