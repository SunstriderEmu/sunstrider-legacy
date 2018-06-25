#ifndef TESTRESULTS_H
#define TESTRESULTS_H

#include "TestSectionResult.h"
#include "TestCase.h"
#include "Optional.h"

class TestCase;

class TestResults
{
public:
    typedef std::list<TestSectionResult> TestResultList;

    TestResults();

    std::string ToString();

    void TestFinished(TestCase const& test);
    void IncreasedIgnored() { _ignored++; }
    void SetUsedPattern(std::string const& pattern) { _usedPattern = pattern; }

private:
    uint32 _totalTestsRan;
    uint32 _ignored;
    std::string _usedPattern;

    TestResultList GetFilteredResult(bool success, std::initializer_list<TestStatus> const& statuses) const;
    static void HandlePrintResults(std::stringstream& ss, std::string desc, TestResultList container);

    TestResultList _successes;
    TestResultList _failures;
};

#endif // TESTRESULTS_H
