
#ifndef _TCSOAP_H
#define _TCSOAP_H

#include "Define.h"
#include <mutex>
#include <future>
#include <string>

void process_message(struct soap* soap_message);
void TCSoapThread(const std::string& host, uint16 port);

class SOAPCommand
{
    public:
        SOAPCommand():
            m_success(false)
        {
        }

        ~SOAPCommand()
        {
        }

        void appendToPrintBuffer(const char* msg)
        {
            m_printBuffer += msg;
        }

        void setCommandSuccess(bool val)
        {
            m_success = val;
            finishedPromise.set_value();
        }

        bool hasCommandSucceeded() const
        {
            return m_success;
        }

        static void print(void* callbackArg, const char* msg)
        {
            ((SOAPCommand*)callbackArg)->appendToPrintBuffer(msg);
        }

        static void commandFinished(void* callbackArg, bool success);

        bool m_success;
        std::string m_printBuffer;
        std::promise<void> finishedPromise;
};

#endif
