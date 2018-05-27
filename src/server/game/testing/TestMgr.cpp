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
    _canceling(false)
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
        return false;
    else if (pattern == "all")
        return true;

    //special case, only run incomplete tests if directly called
    if (test->GetTestStatus() == STATUS_WIP)
        return test->GetName() == pattern;

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
    _canceling = false;
    ASSERT(_remainingTests.empty());
    _Load(args);
    _loading = false;

    return true;
}

void TestMgr::Update()
{
    if (!_running || _loading)
        return;

    const uint32 MAX_PARALLEL_TESTS = sWorld->getConfig(CONFIG_TESTING_MAX_PARALLEL_TESTS);
    uint32 loops = 0;
    //For every running test, start if needed and check if it's finished
    for (decltype(_remainingTests)::iterator itr = _remainingTests.begin(); itr != _remainingTests.end() && loops < MAX_PARALLEL_TESTS;)
    {
        //uint32 testID = itr->first;
        auto& testThread = itr->second;

        if (!testThread->IsStarted() && !_canceling)
        {
            testThread->Start();
        }
        else if (testThread->IsFinished())
        {
            auto test = testThread->GetTest();
            _results.TestFinished(*test);
            itr = _remainingTests.erase(itr);
            continue;
        }
        itr++;
        loops++;
    }

    if (_remainingTests.empty()) //then we're done!
    {
        std::string results;
        if (_canceling)
            results = "\nTests were canceled";
        else
            results = _results.ToString();

        //print it line by line, messages too long may not be displayed
        std::istringstream iss(results);
        for (std::string line; std::getline(iss, line); )
        {
            sWorld->SendGlobalText(line.c_str());
            TC_LOG_INFO("test.unit_test", "%s", line.c_str());
        }

        _running = false;
    }
}

void TestMgr::Cancel()
{
    _canceling = true;
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
    //special case, list all test when no pattern is given
    if (filter == "")
        filter = ".*";

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
        ss << " with regex filter '" << filter << "'";
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

    WorldLocation const& loc = testCase->GetLocation();
    
    player->SaveRecallPosition();
    player->RemoveFromGroup();
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

    if (_running && _canceling)
        ss << "(canceling)" << std::endl;

    return ss.str();
}
