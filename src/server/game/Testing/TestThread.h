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
    friend class TestCase; //to allow calling Wait and HandleThreadPause
public:
    static const uint32 MAX_UPDATE_TIME_MS = 200;

    enum ThreadState : uint8
    {
        STATE_NOT_STARTED, //not yet setup
        STATE_STARTED, //started but not yet setup
        STATE_WAITING_FOR_JOIN, //started, setup but waiting for join
        STATE_READY, //started and setup but not yet running
        STATE_RUNNING, //currently updating
        STATE_WAITING, //update is suspended because of waiting time
        STATE_PAUSED, //almost the same as waiting, but map will skip updates when we're paused. A pause is always removed at the next update.
        STATE_CANCELING, //currently in the process of canceling
        STATE_FINISHED,
    };

    //Create thread with test, do not start immediately
    TestThread(std::unique_ptr<TestCase> test);
    ~TestThread();

    //Start test!
    void Start();
    bool IsStarted() const;

    //Wake up TestMgr if it was waiting
    void WakeUp();
    //Resume execution after a previous loop
    void ResumeExecution();
    void HandlePlayerJoined(Player* player); 

    //Main function for thread
    void Run();
    //update test Wait Timer but do not notify anything
    void UpdateWaitTimer(uint32 const mapDiff);
    TestCase* GetTest() const { return _testCase.get(); };

    // Sleep caller execution until ... (this does not sleep the test thread)
    void WaitUntilDoneOrWaiting(TestCase* test);
    uint32 GetWaitTimer() const { return _waitTimer;  }
    bool IsPaused() const;
    bool IsFinished() const;
    bool IsCanceling() const;
    bool CanUnloadMap() const;
    ThreadState GetState() const { return _state; }

    //stop and fail tests as soon as possible
    void Cancel();
    //test will wait for a player to start running after setup
    void SetJoiner(ObjectGuid playerJoiner);

private:
    std::unique_ptr<TestCase> _testCase;
    std::future<void> _future; //the actual thread containing the test
    std::atomic<ThreadState> _state; // this thread may be finished either because test finished, or because test was cancelled
    ObjectGuid _joinerGUID;

    //Set wait time for the thread. (will not notify anything)
    void _SetWait(uint32 ms);
    // -- var to handle waiting
    std::condition_variable _testCV;
    std::mutex _testCVMutex;
    std::atomic<uint32>     _waitTimer;
    // --
    milliseconds _thisUpdateStartTimeMS;


    // Sleep caller execution for given ms (MUST BE called from the TestCase only)
    void Wait(uint32 ms);
    //This will make the thread pause itself and resume in the next udpate if it has been running too long (MUST BE called from the TestCase only)
    //return true if thread was paused
    bool HandleThreadPause();
};

#endif // TESTTHREAD_H
