#ifndef TESTSECTIONRESULT_H
#define TESTSECTIONRESULT_H

#include "TestDefines.h"

class TestSectionResult
{
public:
    TestSectionResult(std::string testName, std::string sectionName, bool success, TestStatus status, std::string errorMsg = "");

    std::string ToString() const;
    TestStatus GetStatus() const;
    bool IsSuccess() const { return _success; }
    void AppendToError(std::string message);

private:
    bool _success;
    std::string _name;
    std::string _sectionName;
    std::string _errorMsg;
    TestStatus _testStatus;
};

#endif