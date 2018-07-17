#include "TestResults.h"
#include "TestCase.h"
#include "TestThread.h"
#include <sstream>

TestResults::TestResults() :
    _totalTestsRan(0),
    _ignored(0)
{ }

void TestResults::TestFinished(TestCase const& test)
{
    _totalTestsRan++;
    std::list<TestSectionResult> resultList = test.GetResults();

    //for tests with no section, create one fake section. If test has failed out of section, another fake section has already been created in TestCase::_FailNoException
    if (resultList.empty())
        _successes.emplace_back(test.GetName(), "<no section>", true, STATUS_PASSING, "(no error)");
    else
    {
        for (auto const& result : resultList)
            if (result.IsSuccess())
                _successes.emplace_back(result);
            else
                _failures.emplace_back(result);
    }
}

std::list<TestSectionResult> TestResults::GetFilteredResult(bool success, std::initializer_list<TestStatus> const& statuses) const
{
    std::list<TestSectionResult> filtered;
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
    ss << " " << std::endl; //empty line are ignored by core, so add space
    if (_totalTestsRan > 0)
    {
        auto partialSuccessCount = GetFilteredResult(true, { STATUS_PASSING_INCOMPLETE }).size();

        ss << " " << _totalTestsRan << " | Total tests ran (" << _ignored << " ignored)" << std::endl;
        ss << " " << successes << " | Section successes";
        if (partialSuccessCount)
            ss << " (partial: " << partialSuccessCount << ")";
        ss << std::endl;
        ss << " " << failures  << " | Section failures (regressions: " << regressions.size() << ", known: " << knownBugs.size() << ")" << std::endl;
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
