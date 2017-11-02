#include "TestResults.h"
#include "TestCase.h"
#include "TestThread.h"
#include <sstream>

FailedTestResult::FailedTestResult(std::string name, std::string errorMsg) :
    _name(name),
    _errorMsg(errorMsg)
{ }


std::string FailedTestResult::ToString()
{
    std::stringstream ss;
    ss << "    -------------------------------------------------------------------" << std::endl;
    ss << "    Test name: " << _name << std::endl;
    ss << "    Error message:" << std::endl;
    std::stringstream errorStream(_errorMsg);
    for (std::string line; std::getline(errorStream, line); ) //making sure the message is indented even if on multiple line
        ss << "        " + line << std::endl;
    ss << "    -------------------------------------------------------------------" << std::endl;
    return ss.str();
}

TestResults::TestResults() :
    _totalTestsRan(0),
    _ignored(0)
{ }

void TestResults::TestFinished(TestCase const& test)
{
    _totalTestsRan++;
    if (test.Failed())
        _failedResults.emplace_back(test.GetName(), test.GetError());
}

std::string TestResults::ToString()
{
    uint32 failures = _failedResults.size();
    uint32 succeses = _totalTestsRan - failures;

    std::stringstream ss;
    ss << "===========================================" << std::endl;
    ss << "Input regex pattern: " << (_usedPattern == "" ? "(empty pattern)" : _usedPattern) << std::endl;
    if (_totalTestsRan > 0)
    {
        ss << "Total tests ran:" << std::endl << _totalTestsRan << " (" << _ignored << " ignored)" << std::endl;
        ss << "Successes:" << std::endl << succeses << std::endl;
        ss << "Failures:" << std::endl << failures << std::endl;
        ss << std::endl;
        if (_failedResults.size() > 0)
        {
            ss << "Failed tests:" << std::endl;
            for (auto failed : _failedResults)
                ss << failed.ToString() << std::endl;
        }
        else {
            ss << R"(All tests passed \o/)" << std::endl;
        }
    }
    else {
        ss << "No test found with this pattern" << std::endl;
    }
    ss << "============================================" << std::endl;
    return ss.str();
}
