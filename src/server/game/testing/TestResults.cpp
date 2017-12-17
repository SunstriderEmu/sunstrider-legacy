#include "TestResults.h"
#include "TestCase.h"
#include "TestThread.h"
#include <sstream>

TestResult::TestResult(std::string name, bool success, TestStatus status, std::string errorMsg) :
    _name(name),
    _success(success),
    _errorMsg(errorMsg),
    _testStatus(status)
{ }


std::string TestResult::ToString() const
{
    std::stringstream ss;
    ss << "  -----------------------------------------------------------------------------" << std::endl;
    ss << "  Test name: " << _name << std::endl;
    if (!_errorMsg.empty())
    {
        ss << "  Error message:" << std::endl;
        std::stringstream errorStream(_errorMsg);
        for (std::string line; std::getline(errorStream, line); ) //making sure the message is indented even if on multiple line
        {
            size_t startPos = 0;
            const uint32 maxLineLength = 75;
            while (startPos < line.size())
            {
                std::string subline = line.substr(startPos, maxLineLength); //substr include as many chars as possible
                ss << "    " + subline << std::endl;
                startPos += maxLineLength;
            }
        }
    }
    return ss.str();
}

TestStatus TestResult::GetStatus() const 
{
    return _testStatus;
}

TestResults::TestResults() :
    _totalTestsRan(0),
    _ignored(0)
{ }

void TestResults::TestFinished(TestCase const& test)
{
    _totalTestsRan++;
    TestResult result(test.GetName(), test.Failed(), test.GetTestStatus(), test.GetError());
    if(test.Failed())
        _failures.emplace_back(std::move(result));
    else
        _successes.emplace_back(std::move(result));
}

std::list<TestResult> TestResults::GetFilteredResult(bool success, TestStatus status) const
{
    std::list<TestResult> filtered;
    if (success)
    {
        for (auto itr : _successes)
            if (itr.GetStatus() == status)
                filtered.push_back(itr);
    }
    else 
    {
        for (auto itr : _failures)
            if (itr.GetStatus() == status)
                filtered.push_back(itr);
    }
    return filtered;
}

void TestResults::HandlePrintResults(std::stringstream& ss, std::string desc, TestResultList container)
{
    if (!container.empty())
    {
        ss << "(" << container.size() << ") " << desc << std::endl;
        for (auto failed : container)
            ss << failed.ToString() << std::endl;
    }
}

std::string TestResults::ToString()
{
    uint32 successes = _successes.size();
    uint32 failures = _failures.size();

    std::stringstream ss;
    ss << "===============================================================================" << std::endl;
    ss << "Tests results for input pattern: " << (_usedPattern == "" ? "(empty pattern)" : _usedPattern) << std::endl;
    ss << std::endl;
    if (_totalTestsRan > 0)
    {
        ss << " " << _totalTestsRan << " | Total tests ran (" << _ignored << " ignored)" << std::endl;
        ss << " " << successes << " | Successes" << std::endl;
        ss << " " << failures  << " | Failures" << std::endl;
        ss << " " << std::endl;
        if(!failures)
            ss << R"( All tests passed \o/)" << std::endl;

        //show successes with each test status
        auto unexpectedSuccesses = GetFilteredResult(true, STATUS_KNOWN_BUG);
        HandlePrintResults(ss, "Success (unexpected):", unexpectedSuccesses);
        auto incompleteSuccess = GetFilteredResult(true, STATUS_INCOMPLETE);
        HandlePrintResults(ss, "Success (incompletes):", incompleteSuccess);
        //not showing expected successes

        //show failures with each test status
        auto regressions = GetFilteredResult(false, STATUS_PASSING);
        HandlePrintResults(ss, "Failures (regressions):", regressions);
        auto knownBugs = GetFilteredResult(false, STATUS_KNOWN_BUG);
        HandlePrintResults(ss, "Failures (known bugs):", knownBugs);
        auto incompleteFailures = GetFilteredResult(false, STATUS_INCOMPLETE);
        HandlePrintResults(ss, "Failures (incomplete):", incompleteFailures);
    }
    else {
        ss << "No test found with this pattern" << std::endl;
    }
    ss << "===============================================================================" << std::endl;
    return ss.str();
}
