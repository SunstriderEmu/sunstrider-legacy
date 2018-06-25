#include "TestSectionResult.h" 
#include "TestCase.h"

TestSectionResult::TestSectionResult(std::string testName, std::string sectionName, bool success, TestStatus status, std::string errorMsg) :
    _name(testName),
    _sectionName(sectionName),
    _success(success),
    _errorMsg(errorMsg),
    _testStatus(status)
{ }


std::string TestSectionResult::ToString() const
{
    std::stringstream ss;
    ss << "  -----------------------------------------------------------------------------" << std::endl;
    ss << "  Test: " << _name << " / " << _sectionName << std::endl;
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

TestStatus TestSectionResult::GetStatus() const 
{
    return _testStatus;
}

void TestSectionResult::AppendToError(std::string message)
{
    _errorMsg += '\n' + message;
}