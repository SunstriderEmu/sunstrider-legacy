#include "Chat.h"
#include "Language.h"
#include "CharacterCache.h"
#ifdef TESTS
#include "TestMgr.h"
#endif

class test_commandscript : public CommandScript
{
public:
    test_commandscript() : CommandScript("test_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> testCommandTable =
        {
            { "start",          SEC_ADMINISTRATOR, true,  &HandleTestsStartCommand,                 "" },
            { "list",           SEC_ADMINISTRATOR, true,  &HandleTestsListCommand,                  "" },
            { "running",        SEC_ADMINISTRATOR, true,  &HandleTestsRunningCommand,               "" },
            { "cancel",         SEC_ADMINISTRATOR, true,  &HandleTestsCancelCommand,                "" },
            { "go",             SEC_ADMINISTRATOR, false, &HandleTestsGoCommand,                    "" },
            { "join",           SEC_ADMINISTRATOR, false, &HandleTestsJoinCommand,                  "" },
            { "loop",           SEC_ADMINISTRATOR, true,  &HandleTestsLoopCommand,                  "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "tests",          SEC_SUPERADMIN,   true,  nullptr,                                        "", testCommandTable },
        };
        return commandTable;
    }

#ifdef TESTS
    static bool HandleTestsStartCommand(ChatHandler* handler, char const* args)
    {
        bool ok = sTestMgr->Run(args);
        if (ok)
        {
            handler->SendSysMessage("Tests started. Results will be dumped to all players.");
        }
        else {
            std::string testStatus = sTestMgr->GetStatusString();
            handler->SendSysMessage("Tests currently running, failed to start or failed to join. Current status:");
            handler->PSendSysMessage("%s", testStatus.c_str());
        }
        return true;
    }

    static bool HandleTestsListCommand(ChatHandler* handler, char const* args)
    {
        std::string list_str = sTestMgr->ListAvailable(args);
        handler->PSendSysMessage("%s", list_str.c_str());
        return true;
    }

    static bool HandleTestsRunningCommand(ChatHandler* handler, char const* args)
    {
        std::string list_str = sTestMgr->ListRunning(args);
        handler->PSendSysMessage("%s", list_str.c_str());
        return true;
    }

    static bool HandleTestsCancelCommand(ChatHandler* handler, char const* args)
    {
        if (!sTestMgr->IsRunning())
        {
            handler->SendSysMessage("Tests are not running");
            return true;
        }

        sTestMgr->Cancel();
        return true;
    }

    static bool HandleTestsGoCommand(ChatHandler* handler, char const* args)
    {
        uint32 instanceId = atoi(args);
        if (instanceId == 0)
            return false;

        bool ok = sTestMgr->GoToTest(handler->GetSession()->GetPlayer(), instanceId);
        if (ok)
            handler->PSendSysMessage("Teleporting player to test");
        else
            handler->PSendSysMessage("Failed to teleport player to test");
        return true;
    }


    static bool HandleTestsJoinCommand(ChatHandler* handler, const char* testName)
    {
        bool ok = sTestMgr->Run(testName, handler->GetSession()->GetPlayer());
        if (ok)
        {
            handler->SendSysMessage("Test (join) started. Results will be dumped to all players.");
        }
        else {
            std::string testStatus = sTestMgr->GetStatusString();
            handler->SendSysMessage("Test currently running or failed to start. Current status:");
            handler->PSendSysMessage("%s", testStatus.c_str());
        }
        return true;
    }

    static bool HandleTestsLoopCommand(ChatHandler* handler, char const* args)
    {
        //TODO
        return true;
    }

#else
    static bool HandleTestsStartCommand(ChatHandler* handler, char const* args) { handler->SendSysMessage("Core has not been compiled with tests"); return true; }
    static bool HandleTestsListCommand(ChatHandler* handler, char const* args) { return HandleTestsStartCommand(handler, args); }
    static bool HandleTestsRunningCommand(ChatHandler* handler, char const* args) { return HandleTestsStartCommand(handler, args); }
    static bool HandleTestsGoCommand(ChatHandler* handler, char const* args) { return HandleTestsStartCommand(handler, args); }
    static bool HandleTestsCancelCommand(ChatHandler* handler, char const* args) { return HandleTestsStartCommand(handler, args); }
    static bool HandleTestsJoinCommand(ChatHandler* handler, char const* args) { return HandleTestsStartCommand(handler, args); }
    static bool HandleTestsLoopCommand(ChatHandler* handler, char const* args) { return HandleTestsStartCommand(handler, args); }
#endif
};

void AddSC_test_commandscript()
{
    new test_commandscript();
}
