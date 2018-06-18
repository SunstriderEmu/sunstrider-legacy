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

std::list<TestResult> TestResults::GetFilteredResult(bool success, std::initializer_list<TestStatus> const& statuses) const
{
    std::list<TestResult> filtered;
    auto const& iterateList = success ? _successes : _failures;
    for (auto itr : iterateList)
        for (auto status : statuses)
            if (itr.GetStatus() == status)
                filtered.push_back(itr);

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
    auto unexpectedSuccesses = GetFilteredResult(true, { STATUS_KNOWN_BUG });
    auto incompleteSuccess = GetFilteredResult(true, { STATUS_WIP });
    auto regressions = GetFilteredResult(false, { STATUS_PASSING, STATUS_PASSING_INCOMPLETE });
    auto knownBugs = GetFilteredResult(false, { STATUS_KNOWN_BUG });
    auto incompleteFailures = GetFilteredResult(false, { STATUS_WIP });

    std::stringstream ss;
    ss << "===============================================================================" << std::endl;
    ss << "Tests results for input pattern: " << (_usedPattern == "" ? "(empty pattern)" : _usedPattern) << std::endl;
    ss << " " << std::endl; //empty line are ignored by core
    if (_totalTestsRan > 0)
    {
        auto partialSuccessCount = GetFilteredResult(true, { STATUS_PASSING_INCOMPLETE }).size();

        ss << " " << _totalTestsRan << " | Total tests ran (" << _ignored << " ignored)" << std::endl;
        ss << " " << successes << " | Successes";
        if (partialSuccessCount)
            ss << " (partial: " << partialSuccessCount << ")";
        ss << std::endl;
        ss << " " << failures  << " | Failures (regressions: " << regressions.size() << ", known: " << knownBugs.size() << ")" << std::endl;
        ss << " " << std::endl;
        if(!failures)
            ss << R"( All tests passed \o/)" << std::endl;

        //show successes with each test status
        HandlePrintResults(ss, "Success (unexpected):", unexpectedSuccesses);
        HandlePrintResults(ss, "Success (incompletes):", incompleteSuccess);
        //not showing expected successes, those are implicit

        //show failures with each test status
        HandlePrintResults(ss, "Failures (regressions):", regressions);
        HandlePrintResults(ss, "Failures (known bugs):", knownBugs);
        HandlePrintResults(ss, "Failures (incomplete):", incompleteFailures);
    }
    else {
        ss << "No test found with this pattern" << std::endl;
    }
    ss << " " << std::endl; //empty line are ignored by core
    ss << "===============================================================================" << std::endl;
    return ss.str();
}
