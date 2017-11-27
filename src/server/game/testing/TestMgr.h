#ifndef TESTMGR_H
#define TESTMGR_H

class TestCase;
class TestThread;
#include "TestResults.h"
#include <atomic>

class TestMgr
{
public:
    static TestMgr* instance()
    {
        static TestMgr instance;
        return &instance;
    }

    TestMgr();

    bool IsRunning() const { return _running; }
    //true on success start
    bool Run(std::string args);
    std::string ListAvailable(std::string filter) const;
    std::string ListRunning(std::string filter) const;
    bool GoToTest(Player*, uint32 testId) const;
    void Update(uint32 const diff);
    //cancel all tests, print to all players when stopped. This is not instantaneous since test are stopped on next Wait or completion
    void Cancel();

    std::string GetStatusString() const;

private:
    //defined in TestLoader.cpp, all tests are listed there
    void _Load(std::string name_or_pattern);
    bool _TestMatchPattern(std::shared_ptr<TestCase> test, std::string const& pattern) const;

    std::map<uint32 /*testId*/, std::shared_ptr<TestThread>> _remainingTests; //all remaining tests, tests finished are removed from it
    TestResults _results;
    std::atomic<bool> _running;
    std::atomic<bool> _loading;
    std::atomic<bool> _cancelling;
}; 

//extra ifdef to make sure we don't include this by error
#ifdef TESTS
#define sTestMgr TestMgr::instance()
#endif

#endif // TESTMGR_H