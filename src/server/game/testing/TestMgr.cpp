#include "TestMgr.h"
#include "TestResults.h"
#include "TestCase.h"
#include "TestThread.h"

#include "Log.h"
#include "ScriptMgr.h"
#include <regex>
#include <future>

TestMgr::TestMgr() :
    _running(false),
    _loading(false),
    _cancelling(false)
{ }

void TestMgr::_Load(std::string name_or_pattern)
{
    _results = {}; //reset results
    _results.SetUsedPattern(name_or_pattern);

    uint32 testId = 1;
    auto const& allTestsScripts = sScriptMgr->GetAllTests();
    // iterate through multimap's elements (by key)
    decltype(allTestsScripts.equal_range("")) range;
    for (auto i = allTestsScripts.begin(); i != allTestsScripts.end(); i = range.second)
    {
        // Get the range of the current key
        range = allTestsScripts.equal_range(i->first);

        for (auto testScript = range.first; testScript != range.second; ++testScript)
        {
            auto testCase = testScript->second->GetTest();
            testCase->_SetName(testScript->second->GetName()); //to improve: move this to ScriptMgr?
            if (_TestMatchPattern(testCase, name_or_pattern))
            {
                _tests[testId] = std::move(std::make_shared<TestThread>(testCase)); //will immediately start a new thread running the test
                testId++;
            }
            else
                _results.IncreasedIgnored();
        }
    }
}

bool TestMgr::_TestMatchPattern(std::shared_ptr<TestCase> test, std::string const& pattern) const
{
    if (pattern == "")
        return true;

    std::regex regex_pattern;
    try
    {
        regex_pattern = std::regex(pattern);
    }
    catch (std::exception)
    {
        return false;
    }

    return std::regex_match(test->GetName(), regex_pattern);
}

bool TestMgr::Run(std::string args)
{
    if (_running || _loading)
        return false;
    
    _results = {}; //reset results

    _running = true;
    _loading = true;
    _cancelling = false;
    _Load(args);
    _loading = false;

    return true;
}

void TestMgr::Update(uint32 const diff)
{
    if (!_running || _loading)
        return;

    // Tests are being run in threads but still executed one at a time. Threads are actually used to keep the call stack rather than running tests in parallel.
    // This loop will iterate over every tests, and wait for them to finish or to have triggered a wait time.
    // TODO: Actually... can't we have them execute in parallel now? since they are on separate maps
    for (decltype(_tests)::iterator itr = _tests.begin(); itr != _tests.end();)
    {
        auto testThread = (*itr).second;
        auto test = testThread->GetTest();

        if (!testThread->IsStarted())
            testThread->Start();

        testThread->WakeUp();
        testThread->WaitUntilDoneOrWaiting(test);
        ASSERT(test->IsSetup());
        //from this line only can we be sure that the test thread is not currently running
        if (testThread->IsFinished())
        {
            _results.TestFinished(*test);
            testThread->GetTest()->_Cleanup();
            itr = _tests.erase(itr);
            continue;
        }
        testThread->UpdateWaitTimer(diff);
        itr++;
    }

    if (_tests.empty()) //then we're done!
    {
        std::string results;
        if (_cancelling)
            results = "\nTests were canceled";
        else
            results = _results.ToString();

        sWorld->SendGlobalText("\nTests finished. Results:");
        sWorld->SendGlobalText(results.c_str());
        TC_LOG_INFO("test.unit_test", "Test finished. Results: %s", results.c_str());
        _running = false;
    }
}

void TestMgr::Cancel()
{
    _cancelling = true;
    for (decltype(_tests)::iterator itr = _tests.begin(); itr != _tests.end(); itr++)
        itr->second->Cancel();
}

std::string TestMgr::GetStatusString() const
{
    std::stringstream ss;
    ss << _tests.size() << " test(s) left" << std::endl;
    return ss.str();
}

std::string TestMgr::ListAvailable(std::string filter) const
{
    std::vector<std::string> foundList;

    auto const& allTestsScripts = sScriptMgr->GetAllTests();
    decltype(allTestsScripts.equal_range("")) range;
    for (auto i = allTestsScripts.begin(); i != allTestsScripts.end(); i = range.second)
    {
        range = allTestsScripts.equal_range(i->first);
        for (auto testScript = range.first; testScript != range.second; ++testScript)
        {
            auto testCase = testScript->second->GetTest();
            testCase->_SetName(testScript->second->GetName()); //to improve: move this to ScriptMgr?
            if (_TestMatchPattern(testCase, filter))
                foundList.emplace_back(testCase->GetName());
        }
    }
    
    std::stringstream ss;
    ss << "Available tests";
    if (!filter.empty())
        ss << " with filter " << filter;
    ss << ":" << std::endl;
    if (foundList.empty())
    {
        ss << "none" << std::endl;
    }
    else 
    {
        for (auto itr : foundList)
            ss << itr << std::endl;
    }
    return ss.str();
}

bool TestMgr::GoToTest(Player* player, uint32 testId) const
{
    auto test = _tests.find(testId);
    if (test == _tests.end())
        return false;

    auto testThread = (*test).second;
    auto testCase = testThread->GetTest();
    TestMap* testMap = testCase->GetMap();
    if (!testMap) //test may be spawned without a map
        return false;

    WorldLocation const& loc = testCase->GetLocation();
    
    player->UnbindInstance(testMap->GetId(), testMap->GetDifficulty());
    player->SetTeleportingToTest(testMap->GetInstanceId());
    player->TeleportTo(loc, TELE_TO_TEST_MODE);
    return true;
}

std::string TestMgr::ListRunning(std::string filter) const
{
    std::stringstream ss;
    ss << "Running tests";
    if (!filter.empty())
        ss << " with filter " << filter;
    ss << ":" << std::endl;

    std::vector<std::string> foundList;
    bool found = false;
    for (decltype(_tests)::const_iterator itr = _tests.begin(); itr != _tests.end(); itr++)
    {
        auto testId = (*itr).first;
        auto testThread = (*itr).second;
        auto test = testThread->GetTest();
        ss << testId << " - " << test->GetName() << std::endl;
        found = true;
    }
    if (!found)
        ss << "(none)" << std::endl;

    return ss.str();
}
