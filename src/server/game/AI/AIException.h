
#ifndef TRINITY_AIEXCEPTION_H
#define TRINITY_AIEXCEPTION_H

#include "Define.h"
#include <exception>
#include <string>

class TC_GAME_API InvalidAIException : public std::exception
{
public:
    InvalidAIException(char const* msg) : msg_(msg) {}

    char const* what() const noexcept override { return msg_.c_str(); }

private:
    std::string const msg_;
};

#endif
