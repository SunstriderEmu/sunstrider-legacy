#ifndef TESTMGR_H
#define TESTMGR_H

class TestCase;
class TestThread;
#include "TestResults.h"
#include <atomic>

class TestMgr
{
public:
    static uint32 const STARTING_TEST_ID = 1;

    static TestMgr* instance()
    {
        static TestMgr instance;
        return &instance;
    }

    TestMgr();

    bool IsRunning() const { return _running; }
    //true on success start
    //if joiner: start an unique test and teleport player to it immediately
    bool Run(std::string args, Player* joiner = nullptr);
    std::string ListAvailable(std::string filter) const;
    std::string ListRunning(std::string filter) const;
    bool GoToTest(Player*, uint32 testId) const;
    void Update();
    //cancel all tests, print to all players when stopped. This is not instantaneous since test are stopped on next Wait or completion
    void Cancel();
    uint32 GetRemainingTestsCount() const;

    std::string GetStatusString() const;

private:
    //defined in TestLoader.cpp, all tests are listed there
    void _Load(std::string name_or_pattern, Player* joiner = nullptr);
    bool _TestMatchPattern(TestCase* test, std::string const& pattern) const;

    std::map<uint32 /*testId*/, std::shared_ptr<TestThread>> _remainingTests; //all remaining tests, tests finished are removed from it
    TestResults _results;
    std::atomic<bool> _running;
    std::atomic<bool> _loading;
    std::atomic<bool> _canceling;
}; 

//extra ifdef to make sure we don't include this by error
#ifdef TESTS
#define sTestMgr TestMgr::instance()
#endif

#endif // TESTMGR_H