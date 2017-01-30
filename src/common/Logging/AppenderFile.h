
#ifndef APPENDERFILE_H
#define APPENDERFILE_H

#include <atomic>
#include "Appender.h"

class TC_COMMON_API AppenderFile : public Appender
{
public:
    typedef std::integral_constant<AppenderType, APPENDER_FILE>::type TypeIndex;

    AppenderFile(uint8 id, std::string const& name, LogLevel level, AppenderFlags flags, ExtraAppenderArgs extraArgs);
    ~AppenderFile();
    FILE* OpenFile(std::string const& name, std::string const& mode, bool backup);
    AppenderType getType() const override { return TypeIndex::value; }

private:
    void CloseFile();
    void _write(LogMessage const* message) override;
    FILE* logfile;
    std::string _fileName;
    std::string _logDir;
    bool _dynamicName;
    bool _backup;
    uint64 _maxFileSize;
    std::atomic<uint64> _fileSize;
};

#endif
