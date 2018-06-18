#ifndef TESTRESULTS_H
#define TESTRESULTS_H

#include "TestCase.h"
#include "Optional.h"

class TestCase;

class TestResult
{
public:
    TestResult(std::string name, bool success, TestStatus status, std::string errorMsg = "");

    std::string ToString() const;
    TestStatus GetStatus() const;

private:
    bool _success;
    std::string _name;
    std::string _errorMsg;
    TestStatus _testStatus;
};

class TestResults
{
public:
    typedef std::list<TestResult> TestResultList;

    TestResults();

    std::string ToString();

    void TestFinished(TestCase const& test);
    void IncreasedIgnored() { _ignored++; }
    void SetUsedPattern(std::string const& pattern) { _usedPattern = pattern; }

private:
    uint32 _totalTestsRan;
    uint32 _ignored;
    std::string _usedPattern;

    TestResultList&& GetFilteredResult(bool success, std::initializer_list<TestStatus> const& statuses) const;
    static void HandlePrintResults(std::stringstream& ss, std::string desc, TestResultList container);

    TestResultList _successes;
    TestResultList _failures;
};

#endif // TESTRESULTS_H
