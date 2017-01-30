
#ifndef LOGGER_H
#define LOGGER_H

#include "Appender.h"

class TC_COMMON_API Logger
{
    public:
        Logger();

        void Create(std::string const& name, LogLevel level);
        void addAppender(uint8 type, Appender *);
        void delAppender(uint8 type);

        std::string const& getName() const;
        LogLevel getLogLevel() const;
        void setLogLevel(LogLevel level);
        void write(LogMessage* message) const;

    private:
        std::string name;
        LogLevel level;
        AppenderMap appenders;
};

#endif
