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
                _remainingTests[testId] = std::move(std::make_shared<TestThread>(testCase)); //will immediately start a new thread running the test
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

    uint32 available_threads = std::thread::hardware_concurrency();

    //prepare X first tests from _remainingTests and copy them in updatingTests 
    std::vector<std::pair<uint32, std::shared_ptr<TestThread>>> updatingTests;
    uint32 usedThreads = 0;
    auto remaining_itr = _remainingTests.begin();
    while (usedThreads < available_threads && remaining_itr != _remainingTests.end())
    {
        updatingTests.push_back(std::make_pair(remaining_itr->first, remaining_itr->second));
        remaining_itr++;
        usedThreads++;
    }

    // Tests are being run in threads and executed in parralel. Threads are actually used to keep the call stack rather than running tests in parallel.
    // This loop will iterate over every prepared tests, and wait for them to finish or to have triggered a wait time.
    #pragma omp parallel for
    for(auto itr = updatingTests.begin(); itr != updatingTests.end(); itr++)
    {
        uint32 testID = itr->first;
        auto& testThread = itr->second;
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
            size_t removed = _remainingTests.erase(testID);
            ASSERT(removed == 1);
            continue;
        }
        testThread->UpdateWaitTimer(diff);
    }
    updatingTests.clear();

    if (_remainingTests.empty()) //then we're done!
    {
        std::string results;
        if (_cancelling)
            results = "\nTests were canceled";
        else
            results = _results.ToString();

        sWorld->SendGlobalText("\nTests finished. Results:");
        TC_LOG_INFO("test.unit_test", "Test finished. Results:");
        //print it line by line, messages too long may not be displayed
        std::istringstream iss(results);
        for (std::string line; std::getline(iss, line); )
        {
            sWorld->SendGlobalText(line.c_str());
            TC_LOG_INFO("test.unit_test", "*%s", line.c_str());
        }

        _running = false;
    }
}

void TestMgr::Cancel()
{
    _cancelling = true;
    for (auto itr : _remainingTests)
        itr.second->Cancel();
}

std::string TestMgr::GetStatusString() const
{
    std::stringstream ss;
    ss << _remainingTests.size() << " test(s) left" << std::endl;
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
    auto test = _remainingTests.find(testId);
    if (test == _remainingTests.end())
        return false;

    auto testThread = (*test).second;
    auto testCase = testThread->GetTest();
    TestMap* testMap = testCase->GetMap();
    if (!testMap) //test may be spawned without a map
        return false;

    WorldLocation const& loc = testCase->GetLocation();
    
    player->SaveRecallPosition();
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
    for (auto itr : _remainingTests)
    {
        auto testId = itr.first;
        auto testThread = itr.second;
        auto test = testThread->GetTest();
        ss << testId << " - " << test->GetName() << std::endl;
        found = true;
    }
    if (!found)
        ss << "(none)" << std::endl;

    return ss.str();
}
