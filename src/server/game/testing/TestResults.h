#ifndef TESTRESULTS_H
#define TESTRESULTS_H

class TestCase;

class FailedTestResult
{
public:
    FailedTestResult(std::string name, std::string errorMsg);

    std::string ToString();
private:
    std::string _name;
    std::string _errorMsg;
};

class TestResults
{
public:
    TestResults();

    std::string ToString();

    void TestFinished(TestCase const& test);
    void IncreasedIgnored() { _ignored++; }
    void SetUsedPattern(std::string const& pattern) { _usedPattern = pattern; }

private:
    uint32 _totalTestsRan;
    uint32 _ignored;
    std::string _usedPattern;
    std::vector<FailedTestResult> _failedResults;
};

#endif // TESTRESULTS_H